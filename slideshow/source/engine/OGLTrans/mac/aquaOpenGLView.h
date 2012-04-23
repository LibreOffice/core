/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  aquaOpenGLView.h
 */

@class NSOpenGLContext, NSOpenGLPixelFormat;

// the most interesting is there : in OGLTrans, we use an NSView*
// and subclass the NSView to create an OpenGLView does the trick

// I can't really parse the above

// This is weird code. AquaOpenGLView objects are actually allocated
// as NSOpenGLView objetcs, and the _openGLContext and _pixelFormat
// fields are copied from the NSOpenGLView.h header so that it has the
// same structure as NSOpenGLView . Surely this could be done in a
// better way? Is this because of some pre-10.4 SDK compatibility,
// which is irrelevant now?

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
