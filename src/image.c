#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <wren.h>
#include <wren_runtime.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

int plugin_handle;

typedef struct {
  size_t size;
  char* data;
}  Buffer;

static void wren_runtime_error(WrenVM* vm, const char * error){
  wrenSetSlotString(vm, 0, error); 
  wrenAbortFiber(vm, 0);
}

static void push_size(WrenVM* vm, int w, int h){
  wrenSetSlotNewList(vm, 0);
  wrenSetSlotDouble(vm, 1, w);
  wrenInsertInList(vm, 0, -1, 1);
  wrenSetSlotDouble(vm, 2, h);
  wrenInsertInList(vm, 0, -1, 2);
}

static void image_fromFile_2(WrenVM* vm){
  Buffer* buffer = (Buffer*)wrenGetSlotForeign(vm, 1);
  const char* filename = wrenGetSlotString(vm, 2);
  int x,y,n;
  buffer->data = (char*)stbi_load(filename, &x, &y, &n, 4);
  if(buffer->data == NULL){
    wren_runtime_error(vm, stbi_failure_reason());
    return;
  }
  buffer->size = x*y*n;
  push_size(vm, x, y);
}

static void image_fromBuffer_2(WrenVM* vm){
  Buffer* dst = (Buffer*)wrenGetSlotForeign(vm, 1);
  Buffer* src = (Buffer*)wrenGetSlotForeign(vm, 2);
  int x,y,n;
  dst->data = (char*)stbi_load_from_memory((const unsigned char*)src->data, (int)src->size, &x, &y, &n, 4);
  if(dst->data == NULL){
    wren_runtime_error(vm, stbi_failure_reason());
    return;
  }
  dst->size = x*y*n;
  push_size(vm, x, y);
}

static void image_resize_6(WrenVM* vm){
  Buffer* source = (Buffer*)wrenGetSlotForeign(vm, 1);
  Buffer* target = (Buffer*)wrenGetSlotForeign(vm, 2);
  int ow = wrenGetSlotDouble(vm, 3);
  int oh = wrenGetSlotDouble(vm, 4);
  int nw = wrenGetSlotDouble(vm, 5);
  int nh = wrenGetSlotDouble(vm, 6);
  int err = stbir_resize_uint8((const unsigned char*)source->data, ow, oh, 0, (unsigned char*)target->data, nw, nh, 0, 4);
  if(err == 0){
    wren_runtime_error(vm, "Error resizing");
  }
}

typedef enum {
  FORMAT_PNG = 0,
  FORMAT_JPG = 1,
  FORMAT_BMP = 2,
  FORMAT_TGA = 3
} ImageFormat;

static void image_save_5(WrenVM* vm){
  Buffer* source = (Buffer*)wrenGetSlotForeign(vm, 1);
  const char* filename = wrenGetSlotString(vm, 2);
  ImageFormat format = wrenGetSlotDouble(vm, 3);
  int w = wrenGetSlotDouble(vm, 4);
  int h = wrenGetSlotDouble(vm, 5);
  int error;
  switch (format)
  {
  case FORMAT_PNG:
    error = stbi_write_png(filename, w, h, 4, source->data, w*4);
    break;
  case FORMAT_JPG:
    error = stbi_write_jpg(filename, w, h, 4, source->data, 80);
    break;
  case FORMAT_BMP:
    error = stbi_write_bmp(filename, w, h, 4, source->data);
    break;
  case FORMAT_TGA:
    error = stbi_write_tga(filename, w, h, 4, source->data);
    break;
  default:
    wren_runtime_error(vm, "Unknown image format");
    return;
  }
  if(error == 0) wren_runtime_error(vm, "Error on save");
}

WrenForeignMethodFn wrt_plugin_init_image(int handle){
  plugin_handle = handle;
  wrt_bind_method("image.Image.fromFile_(_,_)", image_fromFile_2);
  wrt_bind_method("image.Image.fromBuffer_(_,_)", image_fromBuffer_2);
  wrt_bind_method("image.Image.resize_(_,_,_,_,_,_)", image_resize_6);
  wrt_bind_method("image.Image.save_(_,_,_,_,_)", image_save_5);
  return NULL;
}