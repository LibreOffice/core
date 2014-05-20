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
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 ************************************************************************/

#include <config_lgpl.h>

#include <osl/diagnose.hxx>

#include "OGLTrans_Shaders.hxx"

#ifdef _WIN32

#elif defined(MACOSX)

#else // UNX == X11

#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>

#endif

namespace
{

typedef void (*GLFunction_t)();

#ifdef _WIN32

GLFunction_t lcl_glGetProcAddress(const char *const name)
{
    return wglGetProcAddress(name);
}

#elif defined(MACOSX)

GLFunction_t lcl_glGetProcAddress(const char *const name)
{
    return 0;
}

#else // UNX == X11

GLFunction_t lcl_glGetProcAddress(const char *const name)
{
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}

#endif

}

bool OGLShaders::cbInitialized = false;

#ifdef GL_VERSION_2_0

PFNGLACTIVETEXTUREPROC OGLShaders::glActiveTexture = NULL;
PFNGLCREATESHADERPROC OGLShaders::glCreateShader = NULL;
PFNGLSHADERSOURCEPROC OGLShaders::glShaderSource = NULL;
PFNGLCOMPILESHADERPROC OGLShaders::glCompileShader = NULL;
PFNGLGETSHADERIVPROC OGLShaders::glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC OGLShaders::glGetShaderInfoLog = NULL;
PFNGLDELETESHADERPROC OGLShaders::glDeleteShader = NULL;
PFNGLCREATEPROGRAMPROC OGLShaders::glCreateProgram = NULL;
PFNGLATTACHSHADERPROC OGLShaders::glAttachShader = NULL;
PFNGLLINKPROGRAMPROC OGLShaders::glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC OGLShaders::glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC OGLShaders::glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAMPROC OGLShaders::glUseProgram = NULL;
PFNGLDELETEPROGRAMPROC OGLShaders::glDeleteProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC OGLShaders::glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC OGLShaders::glUniform1i = NULL;
PFNGLUNIFORM1FPROC OGLShaders::glUniform1f = NULL;
#endif

bool OGLShaders::Initialize()
{
#ifdef GL_VERSION_2_0
    if( !cbInitialized ) {
        glActiveTexture = (PFNGLACTIVETEXTUREPROC) lcl_glGetProcAddress( "glActiveTexture" );
        glCreateShader = (PFNGLCREATESHADERPROC) lcl_glGetProcAddress( "glCreateShader" );
        glShaderSource = (PFNGLSHADERSOURCEPROC) lcl_glGetProcAddress( "glShaderSource" );
        glCompileShader = (PFNGLCOMPILESHADERPROC) lcl_glGetProcAddress( "glCompileShader" );
        glGetShaderiv = (PFNGLGETSHADERIVPROC) lcl_glGetProcAddress( "glGetShaderiv" );
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) lcl_glGetProcAddress( "glGetShaderInfoLog" );
        glDeleteShader = (PFNGLDELETESHADERPROC) lcl_glGetProcAddress( "glDeleteShader" );
        glCreateProgram = (PFNGLCREATEPROGRAMPROC) lcl_glGetProcAddress( "glCreateProgram" );
        glAttachShader = (PFNGLATTACHSHADERPROC) lcl_glGetProcAddress( "glAttachShader" );
        glLinkProgram = (PFNGLLINKPROGRAMPROC) lcl_glGetProcAddress( "glLinkProgram" );
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC) lcl_glGetProcAddress( "glGetProgramiv" );
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) lcl_glGetProcAddress( "glGetProgramInfoLog" );
        glUseProgram = (PFNGLUSEPROGRAMPROC) lcl_glGetProcAddress( "glUseProgram" );
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC) lcl_glGetProcAddress( "glDeleteProgram" );
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) lcl_glGetProcAddress( "glGetUniformLocation" );
        glUniform1i = (PFNGLUNIFORM1IPROC) lcl_glGetProcAddress( "glUniform1i" );
        glUniform1f = (PFNGLUNIFORM1FPROC) lcl_glGetProcAddress( "glUniform1f" );
        cbInitialized = true;
    }

    return glCreateShader != NULL;
#else
    return false;
#endif
}

GLuint OGLShaders::LinkProgram( const char *vertexShader, const char *fragmentShader )
{
#ifdef GL_VERSION_2_0
    if( !Initialize() )
        return 0;

    GLhandleARB vertexObject, fragmentObject, programObject;
    GLint vertexCompiled, fragmentCompiled, programLinked;
    char log[1024];

    vertexObject = glCreateShader( GL_VERTEX_SHADER );
    fragmentObject = glCreateShader( GL_FRAGMENT_SHADER );
    SAL_INFO("slideshow.opengl", "checkpoint 1: shaders created (" << (glGetError() == GL_NO_ERROR) << ") vertex: " << vertexObject << " fragment: " << fragmentObject);


    glShaderSource( vertexObject, 1, &vertexShader, NULL );
    glShaderSource( fragmentObject, 1, &fragmentShader, NULL );

    glCompileShader( vertexObject );
    glGetShaderInfoLog( vertexObject, sizeof( log ), NULL, log );
    SAL_INFO("slideshow.opengl", "vertex compile log: " << log);
    glGetShaderiv( vertexObject, GL_COMPILE_STATUS, &vertexCompiled );
    glCompileShader( fragmentObject );
    glGetShaderInfoLog( fragmentObject, sizeof( log ), NULL, log );
    SAL_INFO("slideshow.opengl", "fragment compile log: " << log);
    glGetShaderiv( fragmentObject, GL_COMPILE_STATUS, &fragmentCompiled );

    if( !vertexCompiled || !fragmentCompiled )
        return 0;

    SAL_INFO("slideshow.opengl", "checkpoint 2: shaders compiled (" << (glGetError() == GL_NO_ERROR) << ')');

    programObject = glCreateProgram();
    glAttachShader( programObject, vertexObject );
    glAttachShader( programObject, fragmentObject );

    glLinkProgram( programObject );
    glGetProgramInfoLog( programObject, sizeof( log ), NULL, log );
    SAL_INFO("slideshow.opengl", "program link log: " << log);
    glGetProgramiv( programObject, GL_LINK_STATUS, &programLinked );

    if( !programLinked )
        return 0;

    SAL_INFO("slideshow.opengl", "checkpoint 3: program linked (" << (glGetError() == GL_NO_ERROR) << ')');

    return programObject;
#else
    return 0;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
