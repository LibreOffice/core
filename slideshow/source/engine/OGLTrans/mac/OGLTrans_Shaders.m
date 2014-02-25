/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#import "OGLTrans_Shaders.h"

@implementation OGLShaders

- (id) init
{
  [super init];
  gpuProcessingInit = NO;
  return self;
}

- (void) initLazy
{
  /* Subclass should put initialisation code that can be performaned
     lazily (on first frame render) here */
  initialised = TRUE;

  /* Create a GLU quadric, used for rendering certain geometry */
  quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_FILL);
  gluQuadricNormals(quadric, GL_SMOOTH);
  gluQuadricTexture(quadric, GL_TRUE);

}

- (void) dealloc
{
  /* Free the GLU quadric */
  if (quadric)
    gluDeleteQuadric(quadric);

  [super dealloc];
}

- (NSString *) name
{
  return @"Unnamed OGLShaders";
}

- (NSString *) descriptionFilename
{
  return NULL;
}

- (unsigned int) loadVertexShader: (NSString *) vertexString fragmentShader: (NSString *) fragmentString
{
  const GLcharARB *vertex_string;
  const GLcharARB *fragment_string;
  GLint vertex_compiled;
  GLint fragment_compiled;
  GLint linked;

  /* Delete any existing program object */
  if (program_object) {
    glDeleteObjectARB(program_object);
    program_object = NULL;
  }

  /* Load and compile both shaders */
  if (vertexString) {
    vertex_shader   = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    vertex_string   = (GLcharARB *) [vertexString cStringUsingEncoding:NSASCIIStringEncoding];
    glShaderSourceARB(vertex_shader, 1, &vertex_string, NULL);
    glCompileShaderARB(vertex_shader);
    glGetObjectParameterivARB(vertex_shader, GL_OBJECT_COMPILE_STATUS_ARB, &vertex_compiled);
    /* TODO - Get info log */
  } else {
    vertex_shader   = NULL;
    vertex_compiled = 1;
  }

  if (fragmentString) {
    fragment_shader   = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    fragment_string   = [fragmentString cStringUsingEncoding:NSASCIIStringEncoding];
    glShaderSourceARB(fragment_shader, 1, &fragment_string, NULL);
    glCompileShaderARB(fragment_shader);
    glGetObjectParameterivARB(fragment_shader, GL_OBJECT_COMPILE_STATUS_ARB, &fragment_compiled);
    /* TODO - Get info log */
  } else {
    fragment_shader   = NULL;
    fragment_compiled = 1;
  }

  /* Ensure both shaders compiled */
  if (!vertex_compiled || !fragment_compiled) {
    if (vertex_shader) {
      glDeleteObjectARB(vertex_shader);
      vertex_shader   = NULL;
    }
    if (fragment_shader) {
      glDeleteObjectARB(fragment_shader);
      fragment_shader = NULL;
    }
    return 1;
  }

  /* Create a program object and link both shaders */
  program_object = glCreateProgramObjectARB();
  if (vertex_shader != NULL)
  {
    glAttachObjectARB(program_object, vertex_shader);
    glDeleteObjectARB(vertex_shader);   /* Release */
  }
  if (fragment_shader != NULL)
  {
    glAttachObjectARB(program_object, fragment_shader);
    glDeleteObjectARB(fragment_shader); /* Release */
  }
  glLinkProgramARB(program_object);
  glGetObjectParameterivARB(program_object, GL_OBJECT_LINK_STATUS_ARB, &linked);
  /* TODO - Get info log */

  if (!linked) {
    glDeleteObjectARB(program_object);
    program_object = NULL;
    return 1;
  }

  return 0;
}

- (void) renderFrame
{
  if (!initialised)
    [self initLazy];
}

- (BOOL) reflect
{
  if(!gpuProcessingInit)
  {
    /* Check if this will fall back to software rasterization or
       software vertex processing and don't reflect if it is. */

    GLint fragmentGPUProcessing, vertexGPUProcessing;
    gpuProcessingInit = YES;

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0,0,0,0);
    glPushMatrix();
    [self renderFrame];
    glPopMatrix();
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUFragmentProcessing, &fragmentGPUProcessing);
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUVertexProcessing, &vertexGPUProcessing);
    gpuProcessing = (fragmentGPUProcessing && vertexGPUProcessing) ? YES : NO;
    glPopAttrib();
  }

  return gpuProcessing;
}

@end


// Utility Functions

int NextHighestPowerOf2(int n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}

void CopyFramebufferToTexture(GLuint texture)
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glBindTexture(GL_TEXTURE_2D, texture);
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport[0], viewport[1], NextHighestPowerOf2(viewport[2]), NextHighestPowerOf2(viewport[3]), 0);
}

NSBitmapImageRep *LoadImage(NSString *path, int shouldFlipVertical)
{
  NSBitmapImageRep *bitmapimagerep;
  NSImage *image;
  image = [[[NSImage alloc] initWithContentsOfFile: path] autorelease];
  bitmapimagerep = [[NSBitmapImageRep alloc] initWithData:[image TIFFRepresentation]];

  if (shouldFlipVertical)
  {
    int bytesPerRow, lowRow, highRow;
    unsigned char *pixelData, *swapRow;

    bytesPerRow = [bitmapimagerep bytesPerRow];
    pixelData = [bitmapimagerep bitmapData];

    swapRow = (unsigned char *)malloc(bytesPerRow);
    for (lowRow = 0, highRow = [bitmapimagerep pixelsHigh]-1; lowRow < highRow; lowRow++, highRow--)
    {
      memcpy(swapRow, &pixelData[lowRow*bytesPerRow], bytesPerRow);
      memcpy(&pixelData[lowRow*bytesPerRow], &pixelData[highRow*bytesPerRow], bytesPerRow);
      memcpy(&pixelData[highRow*bytesPerRow], swapRow, bytesPerRow);
    }
    free(swapRow);
  }

  return bitmapimagerep;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
