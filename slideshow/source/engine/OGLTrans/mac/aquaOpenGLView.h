/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  aquaOpenGLView.h
 */

@class NSOpenGLContext, NSOpenGLPixelFormat;

// the most interesting is there : in OGLTrans, we use an NSView*
// and subclass the NSView to create an OpenGLView does the trick

@interface AquaOpenGLView : NSView
{
  @private
    NSOpenGLContext*     _openGLContext;
    NSOpenGLPixelFormat* _pixelFormat;
}

+ (NSOpenGLPixelFormat*)defaultPixelFormat;

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format;
- (void)dealloc;
- (void)setOpenGLContext:(NSOpenGLContext*)context;
- (NSOpenGLContext*)openGLContext;
- (void)clearGLContext;
- (void)prepareOpenGL;
- (BOOL)isOpaque;
- (void)drawRect;
- (void)lockFocus;
- (void)update;     // moved or resized
// reshape is not supported, update bounds in drawRect
- (void) _surfaceNeedsUpdate:(NSNotification*)notification;
- (void)setPixelFormat:(NSOpenGLPixelFormat*)pixelFormat;
- (NSOpenGLPixelFormat*)pixelFormat;
- (void)encodeWithCoder:(NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
