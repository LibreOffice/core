/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
