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

#include <vcl/salbtype.hxx>
#include <vcl/pngwrite.hxx>

#include "opengl/framebuffer.hxx"
#include "opengl/texture.hxx"

// texture with allocated size
ImplOpenGLTexture::ImplOpenGLTexture( int nWidth, int nHeight, bool bAllocate ) :
    mnRefCount( 1 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST ),
    mnFreeSlots(-1)
{
    glGenTextures( 1, &mnTexture );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    if( bAllocate )
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
        CHECK_GL_ERROR();
    }
    glBindTexture( GL_TEXTURE_2D, 0 );
    CHECK_GL_ERROR();

    VCL_GL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " allocate" );
}

// texture with content retrieved from FBO
ImplOpenGLTexture::ImplOpenGLTexture( int nX, int nY, int nWidth, int nHeight ) :
    mnRefCount( 1 ),
    mnTexture( 0 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST ),
    mnFreeSlots(-1)
{
    // FIXME We need the window height here
    // nY = GetHeight() - nHeight - nY;

    glGenTextures( 1, &mnTexture );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nX, nY, nWidth, nHeight, 0 );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, 0 );
    CHECK_GL_ERROR();

    VCL_GL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " from x" << nX << ", y" << nY );
}

// texture from buffer data
ImplOpenGLTexture::ImplOpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, void const * pData ) :
    mnRefCount( 1 ),
    mnTexture( 0 ),
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFilter( GL_NEAREST ),
    mnFreeSlots(-1)
{
    if( !mnTexture )
    {
        glGenTextures( 1, &mnTexture );
        CHECK_GL_ERROR();
    }
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    CHECK_GL_ERROR();
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    CHECK_GL_ERROR();
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mnWidth, mnHeight, 0, nFormat, nType, pData );
    CHECK_GL_ERROR();
    glBindTexture( GL_TEXTURE_2D, 0 );
    CHECK_GL_ERROR();

    VCL_GL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " " << nWidth << "x" << nHeight << " from data" );
}

ImplOpenGLTexture::~ImplOpenGLTexture()
{
    VCL_GL_INFO( "vcl.opengl", "~OpenGLTexture " << mnTexture );
    if( mnTexture != 0 )
    {
        // FIXME: this is really not optimal performance-wise.

        // Check we have been correctly un-bound from all framebuffers.
        ImplSVData* pSVData = ImplGetSVData();
        rtl::Reference<OpenGLContext> pContext = pSVData->maGDIData.mpLastContext;
        if (pContext.is())
            pContext->UnbindTextureFromFramebuffers( mnTexture );

        glDeleteTextures( 1, &mnTexture );
    }
}

bool ImplOpenGLTexture::InsertBuffer(int nX, int nY, int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData)
{
    if (!pData || mnTexture == 0)
        return false;
    glBindTexture(GL_TEXTURE_2D, mnTexture);
    CHECK_GL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_GL_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, nX, mnHeight - nY - nHeight, nWidth, nHeight, nFormat, nType, pData);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR();

    VCL_GL_INFO( "vcl.opengl", "OpenGLTexture " << mnTexture << " Insert buff. to " << nX << " " << nY
                                             << " size " << nWidth << "x" << nHeight << " from data" );

    return true;
}

bool ImplOpenGLTexture::InitializeSlots(int nSlotSize)
{
    if (mpSlotReferences)
        return false;

    mpSlotReferences.reset(new std::vector<int>(nSlotSize, 0));
    mnFreeSlots = nSlotSize;

    return true;
}

int ImplOpenGLTexture::FindFreeSlot()
{
    if (mnFreeSlots > 0 && mpSlotReferences)
    {
        for (size_t i = 0; i < mpSlotReferences->size(); i++)
        {
            if (mpSlotReferences->at(i) <= 0)
            {
                return i;
            }
        }
    }
    return -1;
}

OpenGLTexture::OpenGLTexture() :
    maRect( 0, 0, 0, 0 ),
    mpImpl(nullptr),
    mnSlotNumber(-1)
{
}

OpenGLTexture::OpenGLTexture(ImplOpenGLTexture* pImpl, Rectangle aRectangle, int nSlotNumber)
    : maRect(aRectangle)
    , mpImpl(pImpl)
    , mnSlotNumber(nSlotNumber)
{
    if (mpImpl)
        mpImpl->IncreaseRefCount(nSlotNumber);
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, bool bAllocate )
    : maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
    , mnSlotNumber(-1)
{
    mpImpl = new ImplOpenGLTexture( nWidth, nHeight, bAllocate );
}

OpenGLTexture::OpenGLTexture( int nX, int nY, int nWidth, int nHeight )
    : maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
    , mnSlotNumber(-1)
{
    mpImpl = new ImplOpenGLTexture( nX, nY, nWidth, nHeight );
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, void const * pData )
    : maRect( Point( 0, 0 ), Size( nWidth, nHeight ) )
    , mnSlotNumber(-1)
{
    mpImpl = new ImplOpenGLTexture( nWidth, nHeight, nFormat, nType, pData );
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture )
{
    maRect = rTexture.maRect;
    mpImpl = rTexture.mpImpl;
    mnSlotNumber = rTexture.mnSlotNumber;

    if (mpImpl)
        mpImpl->IncreaseRefCount(mnSlotNumber);
}

OpenGLTexture::OpenGLTexture( const OpenGLTexture& rTexture,
                              int nX, int nY, int nWidth, int nHeight )
{
    maRect = Rectangle( Point( rTexture.maRect.Left() + nX, rTexture.maRect.Top() + nY ),
                        Size( nWidth, nHeight ) );
    mpImpl = rTexture.mpImpl;
    mnSlotNumber = rTexture.mnSlotNumber;
    if (mpImpl)
        mpImpl->IncreaseRefCount(mnSlotNumber);
    VCL_GL_INFO( "vcl.opengl", "Copying texture " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );
}

OpenGLTexture::~OpenGLTexture()
{
    if (mpImpl)
    {
        mpImpl->DecreaseRefCount(mnSlotNumber);
        if (!mpImpl->ExistRefs())
            delete mpImpl;
    }
}

bool OpenGLTexture::IsUnique() const
{
    return ( mpImpl == nullptr || mpImpl->mnRefCount == 1 );
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
    VCL_GL_INFO( "vcl.opengl", "Getting coord " << Id() << " [" << maRect.Left() << "," << maRect.Top() << "] " << GetWidth() << "x" << GetHeight() );

    if( mpImpl == nullptr )
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
        CHECK_GL_ERROR();
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nFilter );
        CHECK_GL_ERROR();
    }
}

void OpenGLTexture::Bind()
{
    if( mpImpl )
    {
        glBindTexture( GL_TEXTURE_2D, mpImpl->mnTexture );
        CHECK_GL_ERROR();
    }
}

void OpenGLTexture::Unbind()
{
    if( mpImpl )
    {
        glBindTexture( GL_TEXTURE_2D, 0 );
        CHECK_GL_ERROR();
    }
}

void OpenGLTexture::SaveToFile(const OUString& rFileName)
{
    std::vector<sal_uInt8> pBuffer(GetWidth() * GetHeight() * 4);
    Read(GL_BGRA, GL_UNSIGNED_BYTE, pBuffer.data());
    BitmapEx aBitmap = OpenGLHelper::ConvertBGRABufferToBitmapEx(pBuffer.data(), GetWidth(), GetHeight());
    try
    {
        vcl::PNGWriter aWriter(aBitmap);
        SvFileStream sOutput(rFileName, StreamMode::WRITE);
        aWriter.Write(sOutput);
        sOutput.Close();
    }
    catch (...)
    {
        SAL_WARN("vcl.opengl", "Error writing png to " << rFileName);
    }
}

void OpenGLTexture::Read( GLenum nFormat, GLenum nType, sal_uInt8* pData )
{
    if( mpImpl == nullptr )
    {
        SAL_WARN( "vcl.opengl", "Can't read invalid texture" );
        return;
    }

    VCL_GL_INFO( "vcl.opengl", "Reading texture " << Id() << " " << GetWidth() << "x" << GetHeight() );

    if( GetWidth() == mpImpl->mnWidth && GetHeight() == mpImpl->mnHeight )
    {
        Bind();
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        CHECK_GL_ERROR();
        // XXX: Call not available with GLES 2.0
        glGetTexImage( GL_TEXTURE_2D, 0, nFormat, nType, pData );
        CHECK_GL_ERROR();
        Unbind();
    }
    else
    {
        long nWidth = maRect.GetWidth();
        long nHeight = maRect.GetHeight();
        long nX = maRect.Left();
        long nY = mpImpl->mnHeight - maRect.Top() - nHeight;

        // Retrieve current context
        ImplSVData* pSVData = ImplGetSVData();
        rtl::Reference<OpenGLContext> pContext = pSVData->maGDIData.mpLastContext;
        OpenGLFramebuffer* pFramebuffer = pContext->AcquireFramebuffer(*this);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        CHECK_GL_ERROR();
        glReadPixels(nX, nY, nWidth, nHeight, nFormat, nType, pData);
        CHECK_GL_ERROR();
        OpenGLContext::ReleaseFramebuffer(pFramebuffer);
    }
}

OpenGLTexture::operator bool() const
{
    return ( mpImpl != nullptr );
}

OpenGLTexture&  OpenGLTexture::operator=( const OpenGLTexture& rTexture )
{
    if (rTexture.mpImpl)
    {
        rTexture.mpImpl->IncreaseRefCount(rTexture.mnSlotNumber);
    }

    if (mpImpl)
    {
        mpImpl->DecreaseRefCount(mnSlotNumber);
        if (!mpImpl->ExistRefs())
            delete mpImpl;
    }

    maRect = rTexture.maRect;
    mpImpl = rTexture.mpImpl;
    mnSlotNumber = rTexture.mnSlotNumber;

    return *this;
}

bool OpenGLTexture::operator==( const OpenGLTexture& rTexture ) const
{
    return (mpImpl == rTexture.mpImpl
         && maRect == rTexture.maRect
         && mnSlotNumber == rTexture.mnSlotNumber);
}

bool OpenGLTexture::operator!=( const OpenGLTexture& rTexture ) const
{
    return !( *this == rTexture );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
