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

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

class OGLShaders {
    static bool Initialize();
    static bool cbInitialized;

public:

    static GLuint LinkProgram( const char *vertexShader, const char *fragmentShader );

    /** GL shader functions
     */
#ifdef GL_VERSION_2_0

    static PFNGLCREATESHADERPROC glCreateShader;
    static PFNGLSHADERSOURCEPROC glShaderSource;
    static PFNGLCOMPILESHADERPROC glCompileShader;
    static PFNGLGETSHADERIVPROC glGetShaderiv;
    static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    static PFNGLDELETESHADERPROC glDeleteShader;

    static PFNGLCREATEPROGRAMPROC glCreateProgram;
    static PFNGLATTACHSHADERPROC glAttachShader;
    static PFNGLLINKPROGRAMPROC glLinkProgram;
    static PFNGLGETPROGRAMIVPROC glGetProgramiv;
    static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    static PFNGLUSEPROGRAMPROC glUseProgram;
    static PFNGLDELETEPROGRAMPROC glDeleteProgram;

    static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    static PFNGLUNIFORM1IPROC glUniform1i;
    static PFNGLUNIFORM1FPROC glUniform1f;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
