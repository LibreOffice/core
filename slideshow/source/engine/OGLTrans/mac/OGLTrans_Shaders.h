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

#ifndef INCLUDED_OGLTRANS_SHADERS_HXX_
#define INCLUDED_OGLTRANS_SHADERS_HXX_

/* FIXME : Really needed ? */
#define GL_GLEXT_PROTOTYPES 1

/* Required Includes */
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>
#import <GLUT/glut.h>

/* Parameter */
typedef struct _Parameter {
    float current [4];
    float min     [4];
    float max     [4];
    float delta   [4];

} Parameter;

/* Macros */
#define PARAMETER_CURRENT(p)    (p.current)
#define PARAMETER_ANIMATE(p)    ({ int i; for (i = 0; i < 4; i ++) { \
                                            p.current[i] += p.delta[i]; \
                                            if ((p.current[i] < p.min[i]) || (p.current[i] > p.max[i])) \
                                                p.delta[i] = -p.delta[i]; } } )

/* OGLShaders base class */
@interface OGLShaders : NSObject
{
    BOOL initialised;
    GLhandleARB vertex_shader;
    GLhandleARB fragment_shader;
    GLhandleARB program_object;
    GLUquadric *quadric;
    BOOL gpuProcessingInit;
    BOOL gpuProcessing;
}
- (id) init;
- (void) initLazy;
- (void) dealloc;
- (NSString *) name;
- (NSString *) descriptionFilename;
- (unsigned int) loadVertexShader: (NSString *) vertexString fragmentShader: (NSString *) fragmentString;

 /* if!initialized -> initLazy */
- (void) renderFrame;

/* Check whether software rasterization or software vertex processing will be used */
- (BOOL) reflect;

@end


// Utility Functions

int NextHighestPowerOf2(int n);
void CopyFramebufferToTexture(GLuint texture);
NSBitmapImageRep *LoadImage(NSString *path, int shouldFlipVertical);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
