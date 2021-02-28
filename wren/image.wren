import "buffer" for Buffer

class Image {

  width { _size[0] }
  height { _size[1] }
  isDisposed { _buffer.isDisposed }
  buffer { _buffer }

  construct fromFile(path){
    _buffer = Buffer.new(0)
    _size = fromFile_(_buffer, path)
  }
  construct fromBuffer(buffer){
    _buffer = Buffer.new(0)
    _size = fromBuffer_(_buffer, buffer)
  }
  construct fromRgba(buffer, w, h){
    _buffer = buffer
    _size = [w,h]
  }
  dispose(){
    _buffer.dispose()
  }

  foreign fromFile_(buffer, path)
  foreign fromBuffer_(bufferDst, bufferSrc)
  foreign resize_(oldBuffer, newBuffer, ow, oh, nw, nh)
  foreign save_(buffer, filename, format, w, h)

  resize(nw, nh){
    var buffer = Buffer.new(nw*nh*4)
    resize_(_buffer, buffer, width, height, nw, nh)
    return Image.fromRgba(buffer, nw, nh)
  }
  save(filename){
    if(filename.endsWith("png")) return save_(_buffer, filename, 0, width, height)
    if(filename.endsWith("jpg")) return save_(_buffer, filename, 1, width, height)
    if(filename.endsWith("jpeg")) return save_(_buffer, filename, 1, width, height)
    if(filename.endsWith("bmp")) return save_(_buffer, filename, 2, width, height)
    if(filename.endsWith("tga")) return save_(_buffer, filename, 3, width, height)
  }
}