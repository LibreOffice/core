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
#include <algorithm>

#include "svdata.hxx"

#include "vcl/salbtype.hxx"

#include "opengl/framebuffer.hxx"
#include "opengl/texture.hxx"

// default texture
ImplOpenGLTexture::ImplOpenGLTexture() :
    mnTexture( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnFilter( GL_NEAREST )
{
}

// texture with allocated size
ImplOpenGLTexture::ImplOpenGLTexture( int nWidth, int nHeight, bool bAllocate ) :
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

// copy texture impl
ImplOpenGLTexture::ImplOpenGLTexture( const ImplOpenGLTexture& rSrc ) :
    mnTexture( rSrc.mnTexture ),
    mnWidth( rSrc.mnWidth ),
    mnHeight( rSrc.mnHeight ),
    mnFilter( rSrc.mnFilter )
{
}

ImplOpenGLTexture::~ImplOpenGLTexture()
{
    SAL_INFO( "vcl.opengl", "~OpenGLTexture " << mnTexture );
    if( mnTexture != 0 )
        glDeleteTextures( 1, &mnTexture );
}

OpenGLTexture::OpenGLTexture() :
    maRect( 0, 0, 0, 0 ),
    maImpl()
{
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, bool bAllocate ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) ),
    maImpl( ImplOpenGLTexture( nWidth, nHeight, bAllocate ) )
{
}

OpenGLTexture::OpenGLTexture( int nX, int nY, int nWidth, int nHeight ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) ),
    maImpl( ImplOpenGLTexture( nX, nY, nWidth, nHeight ) )
{
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData ) :
    maRect( Point( 0, 0 ), Size( nWidth, nHeight ) ),
    maImpl( ImplOpenGLTexture( nWidth, nHeight, nFormat, nType, pData ) )
{
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture ) :
    maRect( rTexture.maRect ),
    maImpl( rTexture.maImpl )
{
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture,
                              int nX, int nY, int nWidth, int nHeight )
{
    maRect = Rectangle( Point( rTexture.maRect.Left() + nX, rTexture.maRect.Top() + nY ),
                        Size( nWidth, nHeight ) );
    maImpl = rTexture.maImpl;
    SAL_INFO( "vcl.opengl", "Copying texture " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );
}

OpenGLTexture::~OpenGLTexture()
{
}

bool OpenGLTexture::IsUnique() const
{
    return ( maImpl.is_unique() );
}

GLuint OpenGLTexture::Id() const
{
    // default is 0
    return maImpl->mnTexture;
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

    if( maImpl->mnWidth < 1 )
    {
        pCoord[0] = pCoord[1] = pCoord[2] = pCoord[3] = 0.0f;
        pCoord[4] = pCoord[5] = pCoord[6] = pCoord[7] = 0.0f;
        return;
    }
    pCoord[0] = pCoord[2] = (maRect.Left() + rPosAry.mnSrcX) / (double) maImpl->mnWidth;
    pCoord[4] = pCoord[6] = (maRect.Left() + rPosAry.mnSrcX + rPosAry.mnSrcWidth) / (double) maImpl->mnWidth;

    if( !bInverted )
    {
        pCoord[3] = pCoord[5] = 1.0f - (maRect.Top() + rPosAry.mnSrcY) / (double) maImpl->mnHeight;
        pCoord[1] = pCoord[7] = 1.0f - (maRect.Top() + rPosAry.mnSrcY + rPosAry.mnSrcHeight) / (double) maImpl->mnHeight;
    }
    else
    {
        pCoord[1] = pCoord[7] = 1.0f - (maRect.Top() + rPosAry.mnSrcY) / (double) maImpl->mnHeight;
        pCoord[3] = pCoord[5] = 1.0f - (maRect.Top() + rPosAry.mnSrcY + rPosAry.mnSrcHeight) / (double) maImpl->mnHeight;
    }
}

void OpenGLTexture::GetWholeCoord( GLfloat* pCoord ) const
{
    if( GetWidth() != maImpl->mnWidth || GetHeight() != maImpl->mnHeight )
    {
        pCoord[0] = pCoord[2] = maRect.Left() / (double) maImpl->mnWidth;
        pCoord[4] = pCoord[6] = maRect.Right() / (double) maImpl->mnWidth;
        pCoord[3] = pCoord[5] = 1.0f - maRect.Top() / (double) maImpl->mnHeight;
        pCoord[1] = pCoord[7] = 1.0f - maRect.Bottom() / (double) maImpl->mnHeight;
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
    // default is GL_NEAREST
    return maImpl->mnFilter;
}

void OpenGLTexture::SetFilter( GLenum nFilter )
{
    maImpl->mnFilter = nFilter;
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nFilter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nFilter );

    CHECK_GL_ERROR();
}

void OpenGLTexture::Bind()
{
    glBindTexture( GL_TEXTURE_2D, maImpl->mnTexture );

    CHECK_GL_ERROR();
}

void OpenGLTexture::Unbind()
{
    glBindTexture( GL_TEXTURE_2D, 0 );

    CHECK_GL_ERROR();
}

void OpenGLTexture::Read( GLenum nFormat, GLenum nType, sal_uInt8* pData )
{
    if( maImpl->mnWidth < 1 && maImpl->mnHeight < 1 )
    {
        SAL_WARN( "vcl.opengl", "Can't read invalid texture" );
        return;
    }

    Bind();
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );

    SAL_INFO( "vcl.opengl", "Reading texture " << Id() << " " << GetWidth() << "x" << GetHeight() );

    if( GetWidth() == maImpl->mnWidth && GetHeight() == maImpl->mnHeight )
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
        glReadPixels( maRect.Left(), maImpl->mnHeight - maRect.Top(), GetWidth(), GetHeight(), nFormat, nType, pData );
        OpenGLContext::ReleaseFramebuffer( pFramebuffer );
        CHECK_GL_ERROR();
    }

    Unbind();
    CHECK_GL_ERROR();
}

OpenGLTexture::operator bool() const
{
    return ( maImpl->mnWidth != 0 && maImpl->mnHeight != 0 );
}

OpenGLTexture&  OpenGLTexture::operator=( const OpenGLTexture& rTexture )
{
    maRect = rTexture.maRect;
    maImpl = rTexture.maImpl;

    return *this;
}

bool OpenGLTexture::operator==( const OpenGLTexture& rTexture ) const
{
    return ( maImpl.same_object( rTexture.maImpl ) && maRect == rTexture.maRect );
}

bool OpenGLTexture::operator!=( const OpenGLTexture& rTexture ) const
{
    return !( *this == rTexture );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
