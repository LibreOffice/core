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

#include <sal/config.h>
#include <vcl/opengl/OpenGLHelper.hxx>

#include "vcl/salbtype.hxx"

#include "opengl/texture.hxx"

// texture with allocated size
ImplOpenGLTexture::ImplOpenGLTexture( int nWidth, int nHeight, bool bAllocate ) :
    mnRefCount( 1 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST )
{
    glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    if( bAllocate )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glBindTexture( GL_TEXTURE_2D, 0 );
}

// texture with content retrieved from FBO
ImplOpenGLTexture::ImplOpenGLTexture( int nX, int nY, int nWidth, int nHeight ) :
    mnRefCount( 1 ),
    mnTexture( 0 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST )
{
    glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nX, nY, nWidth, nHeight, 0 );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, 0 );
    CHECK_GL_ERROR();
}

// texture from buffer data
ImplOpenGLTexture::ImplOpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData ) :
    mnRefCount( 1 ),
    mnTexture( 0 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST )
{
    if( !mnTexture )
        glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mnWidth, mnHeight, 0, nFormat, nType, pData );
    glBindTexture( GL_TEXTURE_2D, 0 );
}

ImplOpenGLTexture::~ImplOpenGLTexture()
{
    SAL_INFO( "vcl.opengl", "~OpenGLTexture " << mnTexture );
    if( mnTexture != 0 )
        glDeleteTextures( 1, &mnTexture );
}

OpenGLTexture::OpenGLTexture() :
    maRect( 0, 0, 0, 0 ),
    mpImpl( NULL )
{
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, bool bAllocate ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
{
    mpImpl = new ImplOpenGLTexture( nWidth, nHeight, bAllocate );
}

OpenGLTexture::OpenGLTexture( int nX, int nY, int nWidth, int nHeight ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
{
    mpImpl = new ImplOpenGLTexture( nX, nY, nWidth, nHeight );
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
{
    mpImpl = new ImplOpenGLTexture( nWidth, nHeight, nFormat, nType, pData );
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture )
{
    maRect = rTexture.maRect;
    mpImpl = rTexture.mpImpl;
    if( mpImpl )
        mpImpl->mnRefCount++;
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture,
                              int nX, int nY, int nWidth, int nHeight )
{
    maRect = Rectangle( Point( rTexture.maRect.Left() + nX, rTexture.maRect.Top() + nY ),
                        Size( nWidth, nHeight ) );
    mpImpl = rTexture.mpImpl;
    if( mpImpl )
        mpImpl->mnRefCount++;
    SAL_INFO( "vcl.opengl", "Copying texture " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );
}

OpenGLTexture::~OpenGLTexture()
{
    if( mpImpl )
    {
        if( mpImpl->mnRefCount == 1 )
            delete mpImpl;
        else
            mpImpl->mnRefCount--;
    }
}

bool OpenGLTexture::IsUnique() const
{
    return ( mpImpl == NULL || mpImpl->mnRefCount == 1 );
}

GLuint OpenGLTexture::Id() const
{
    if( mpImpl )
        return mpImpl->mnTexture;
    return 0;
}

int OpenGLTexture::GetWidth() const
{
    return maRect.GetWidth();
}

int OpenGLTexture::GetHeight() const
{
    return maRect.GetHeight();
}

void OpenGLTexture::GetCoord( GLfloat* pCoord, const SalTwoRect& rPosAry, bool bInverted ) const
{
    SAL_INFO( "vcl.opengl", "Getting coord " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );
    pCoord[0] = pCoord[2] = (maRect.Left() + rPosAry.mnSrcX) / (double) mpImpl->mnWidth;
    pCoord[4] = pCoord[6] = (maRect.Left() + rPosAry.mnSrcX + rPosAry.mnSrcWidth) / (double) mpImpl->mnWidth;

    if( !bInverted )
    {
        pCoord[3] = pCoord[5] = 1.0f - (maRect.Top() + rPosAry.mnSrcY) / (double) mpImpl->mnHeight;
        pCoord[1] = pCoord[7] = 1.0f - (maRect.Top() + rPosAry.mnSrcY + rPosAry.mnSrcHeight) / (double) mpImpl->mnHeight;
    }
    else
    {
        pCoord[1] = pCoord[7] = 1.0f - (maRect.Top() + rPosAry.mnSrcY) / (double) mpImpl->mnHeight;
        pCoord[3] = pCoord[5] = 1.0f - (maRect.Top() + rPosAry.mnSrcY + rPosAry.mnSrcHeight) / (double) mpImpl->mnHeight;
    }
}

GLenum OpenGLTexture::GetFilter() const
{
    if( mpImpl )
        return mpImpl->mnFilter;
    return GL_NEAREST;
}

void OpenGLTexture::SetFilter( GLenum nFilter )
{
    if( mpImpl )
    {
        mpImpl->mnFilter = nFilter;
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nFilter );
    }
}

void OpenGLTexture::Bind()
{
    if( mpImpl )
        glBindTexture( GL_TEXTURE_2D, mpImpl->mnTexture );
}

void OpenGLTexture::Unbind()
{
    if( mpImpl )
        glBindTexture( GL_TEXTURE_2D, 0 );
}

bool OpenGLTexture::Draw()
{
    GLfloat aPosition[8] = { -1, -1, -1, 1, 1, 1, 1, -1 };
    GLfloat aTexCoord[8] = {  0,  0,  0, 1, 1, 1, 1,  0 };

    if( mpImpl == NULL )
    {
        SAL_WARN( "vcl.opengl", "Can't draw invalid texture" );
        return false;
    }

    SAL_INFO( "vcl.opengl", "Drawing texture " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );
    if( GetWidth() != mpImpl->mnWidth || GetHeight() != mpImpl->mnHeight )
    {
        // FIXME: lfrb: check math
        aTexCoord[0] = aTexCoord[2] = maRect.Left() / (double) mpImpl->mnWidth;
        aTexCoord[4] = aTexCoord[6] = maRect.Right() / (double) mpImpl->mnWidth;
        aTexCoord[1] = aTexCoord[7] = maRect.Top() / (double) mpImpl->mnHeight;
        aTexCoord[3] = aTexCoord[5] = maRect.Bottom() / (double) mpImpl->mnHeight;
    }

    glBindTexture( GL_TEXTURE_2D, mpImpl->mnTexture );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, aPosition );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, aTexCoord );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return true;
}

void OpenGLTexture::Read( GLenum nFormat, GLenum nType, sal_uInt8* pData )
{
    if( mpImpl == NULL )
    {
        SAL_WARN( "vcl.opengl", "Can't read invalid texture" );
        return;
    }

    Bind();
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );

    SAL_INFO( "vcl.opengl", "Reading texture " << Id() << " " << GetWidth() << "x" << GetHeight() );

    if( GetWidth() == mpImpl->mnWidth && GetHeight() == mpImpl->mnHeight )
    {
        // XXX: Call not available with GLES 2.0
        glGetTexImage( GL_TEXTURE_2D, 0, nFormat, nType, pData );
    }
    else
    {
        GLuint nFramebufferId;
        glGenFramebuffers( 1, &nFramebufferId );
        glBindFramebuffer( GL_FRAMEBUFFER, nFramebufferId );
    CHECK_GL_ERROR();

        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Id(), 0 );
    CHECK_GL_ERROR();
        glReadPixels( maRect.Left(), mpImpl->mnHeight - maRect.Top(), GetWidth(), GetHeight(), nFormat, nType, pData );
    CHECK_GL_ERROR();

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glDeleteFramebuffers( 1, &nFramebufferId );

        int bpp = (nFormat == GL_RGB) ? 3 : 4;
        memset( pData, 255, GetWidth() * GetHeight() * bpp );
    }

    Unbind();
    CHECK_GL_ERROR();
}

OpenGLTexture::operator bool() const
{
    return ( mpImpl != NULL );
}

OpenGLTexture&  OpenGLTexture::operator=( const OpenGLTexture& rTexture )
{
    if( rTexture.mpImpl )
        rTexture.mpImpl->mnRefCount++;
    if( mpImpl )
    {
        if( mpImpl->mnRefCount == 1 )
            delete mpImpl;
        else
            mpImpl->mnRefCount--;
    }

    maRect = rTexture.maRect;
    mpImpl = rTexture.mpImpl;

    return *this;
}

bool OpenGLTexture::operator==( const OpenGLTexture& rTexture ) const
{
    return (mpImpl == rTexture.mpImpl && maRect == rTexture.maRect );
}

bool OpenGLTexture::operator!=( const OpenGLTexture& rTexture ) const
{
    return !( *this == rTexture );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
