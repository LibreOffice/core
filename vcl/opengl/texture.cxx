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
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include "svdata.hxx"

#include "vcl/salbtype.hxx"

#include "opengl/framebuffer.hxx"
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

    SAL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " allocate" );

    CHECK_GL_ERROR();
}

// texture with content retrieved from FBO
ImplOpenGLTexture::ImplOpenGLTexture( int nX, int nY, int nWidth, int nHeight ) :
    mnRefCount( 1 ),
    mnTexture( 0 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST )
{
    // FIXME We need the window height here
    // nY = GetHeight() - nHeight - nY;

    glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nX, nY, nWidth, nHeight, 0 );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, 0 );

    SAL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " from x" << nX << ", y" << nY );

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

    SAL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " from data" );

    CHECK_GL_ERROR();
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

    if( mpImpl == NULL )
    {
        pCoord[0] = pCoord[1] = pCoord[2] = pCoord[3] = 0.0f;
        pCoord[4] = pCoord[5] = pCoord[6] = pCoord[7] = 0.0f;
        return;
    }

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

void OpenGLTexture::GetWholeCoord( GLfloat* pCoord ) const
{
    if( GetWidth() != mpImpl->mnWidth || GetHeight() != mpImpl->mnHeight )
    {
        pCoord[0] = pCoord[2] = maRect.Left() / (double) mpImpl->mnWidth;
        pCoord[4] = pCoord[6] = maRect.Right() / (double) mpImpl->mnWidth;
        pCoord[3] = pCoord[5] = 1.0f - maRect.Top() / (double) mpImpl->mnHeight;
        pCoord[1] = pCoord[7] = 1.0f - maRect.Bottom() / (double) mpImpl->mnHeight;
    }
    else
    {
        pCoord[0] = pCoord[2] = 0;
        pCoord[4] = pCoord[6] = 1;
        pCoord[1] = pCoord[7] = 0;
        pCoord[3] = pCoord[5] = 1;
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

    CHECK_GL_ERROR();
}

void OpenGLTexture::Bind()
{
    if( mpImpl )
        glBindTexture( GL_TEXTURE_2D, mpImpl->mnTexture );

    CHECK_GL_ERROR();
}

void OpenGLTexture::Unbind()
{
    if( mpImpl )
        glBindTexture( GL_TEXTURE_2D, 0 );

    CHECK_GL_ERROR();
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
        // Retrieve current context
        ImplSVData* pSVData = ImplGetSVData();
        OpenGLContext* pContext = pSVData->maGDIData.mpLastContext;
        OpenGLFramebuffer* pFramebuffer;

        pFramebuffer = pContext->AcquireFramebuffer( *this );
        glReadPixels( maRect.Left(), mpImpl->mnHeight - maRect.Top(), GetWidth(), GetHeight(), nFormat, nType, pData );
        OpenGLContext::ReleaseFramebuffer( pFramebuffer );
        CHECK_GL_ERROR();
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
