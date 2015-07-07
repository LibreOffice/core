/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
 */

#ifndef INCLUDED_VCL_INC_OPENGL_TEXTURE_H
#define INCLUDED_VCL_INC_OPENGL_TEXTURE_H

#include <GL/glew.h>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>

#include <tools/gen.hxx>

class ImplOpenGLTexture
{
public:
    int    mnRefCount;
    GLuint mnTexture;
    int    mnWidth;
    int    mnHeight;
    GLenum mnFilter;

    ImplOpenGLTexture( int nWidth, int nHeight, bool bAllocate );
    ImplOpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData );
    ImplOpenGLTexture( int nX, int nY, int nWidth, int nHeight );
    ~ImplOpenGLTexture();
};

class VCL_PLUGIN_PUBLIC OpenGLTexture
{
private:
    // if the rect size doesn't match the mpImpl one, this instance
    // is a sub-area from the real OpenGL texture
    Rectangle          maRect;

    ImplOpenGLTexture* mpImpl;

public:
                    OpenGLTexture();
                    OpenGLTexture( int nWidth, int nHeight, bool bAllocate = true );
                    OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData );
                    OpenGLTexture( int nX, int nY, int nWidth, int nHeight );
                    OpenGLTexture( const OpenGLTexture& rTexture );
                    OpenGLTexture( const OpenGLTexture& rTexture, int nX, int nY, int nWidth, int nHeight );
    virtual         ~OpenGLTexture();

    bool            IsUnique() const;

    GLuint          Id() const;
    int             GetWidth() const;
    int             GetHeight() const;
    void            GetCoord( GLfloat* pCoord, const SalTwoRect& rPosAry, bool bInverted=false ) const;
    void            GetWholeCoord( GLfloat* pCoord ) const;

    void            Bind();
    void            Unbind();
    void            Read( GLenum nFormat, GLenum nType, sal_uInt8* pData );

    GLenum          GetFilter() const;
    void            SetFilter( GLenum nFilter );

                    operator bool() const;
    OpenGLTexture&  operator=( const OpenGLTexture& rTexture );
    bool            operator==( const OpenGLTexture& rTexture ) const;
    bool            operator!=( const OpenGLTexture& rTexture ) const;
};

#endif // INCLUDED_VCL_INC_OPENGL_TEXTURE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
