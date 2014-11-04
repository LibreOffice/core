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

#include "vcl/bitmap.hxx"
#include "vcl/salbtype.hxx"
#include "salgdi.hxx"

#include "opengl/salbmp.hxx"
#include "unx/salbmp.h"

static bool isValidBitCount( sal_uInt16 nBitCount )
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 16) || (nBitCount == 24) || (nBitCount == 32);
}

OpenGLSalBitmap::OpenGLSalBitmap()
: mnTexture(0)
, mbDirtyTexture(true)
, mnBits(0)
, mnBytesPerRow(0)
, mnWidth(0)
, mnHeight(0)
, mnTexWidth(0)
, mnTexHeight(0)
{
}

OpenGLSalBitmap::~OpenGLSalBitmap()
{
    Destroy();
}

bool OpenGLSalBitmap::Create( OpenGLContext& rContext, long nX, long nY, long nWidth, long nHeight )
{
    static const BitmapPalette aEmptyPalette;

    Destroy();

    mpContext = &rContext;
    mpContext->makeCurrent();
    mnWidth = mnTexWidth = nWidth;
    mnHeight = mnTexHeight = nHeight;

    // TODO Check the framebuffer configuration
    mnBits = 32;
    maPalette = aEmptyPalette;

    glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nX, nY, nWidth, nHeight, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return true;
}

bool OpenGLSalBitmap::Create( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rBitmapPalette )
{
    Destroy();

    if( !isValidBitCount( nBits ) )
        return false;
    maPalette = rBitmapPalette;
    mnBits = nBits;
    mnWidth = mnTexWidth = rSize.Width();
    mnHeight = mnTexHeight = rSize.Height();
    return false;
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp )
{
    return Create( rSalBmp, rSalBmp.GetBitCount() );
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    return Create( rSalBmp, pGraphics ? pGraphics->GetBitCount() : rSalBmp.GetBitCount() );
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount )
{
    const OpenGLSalBitmap& rSourceBitmap = static_cast<const OpenGLSalBitmap&>(rSalBmp);

    if( isValidBitCount( nNewBitCount ) )
    {
        mnBits = nNewBitCount;
        mnTexWidth = rSourceBitmap.mnTexWidth;
        mnTexHeight = rSourceBitmap.mnTexHeight;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        maPalette = rSourceBitmap.maPalette;
        mnTexture = rSourceBitmap.mnTexture;

        // TODO Copy buffer data if the bitcount and palette are the same
        return true;
    }
    return false;
}

bool OpenGLSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    // TODO Is this method needed?
    return false;
}

bool OpenGLSalBitmap::Draw( OpenGLContext& rContext, const SalTwoRect& rPosAry )
{
    if( !mpContext )
        mpContext = &rContext;

    if( !mnTexture || mbDirtyTexture )
    {
        if( !CreateTexture() )
            return false;
    }

    DrawTexture( mnTexture, rPosAry );
    return true;
}

GLuint OpenGLSalBitmap::GetTexture( OpenGLContext& rContext ) const
{
    if( !mpContext )
        const_cast<OpenGLSalBitmap*>(this)->mpContext = &rContext;
    if( !mnTexture || mbDirtyTexture )
        const_cast<OpenGLSalBitmap*>(this)->CreateTexture();
    return mnTexture;
}

void OpenGLSalBitmap::Destroy()
{
    DeleteTexture();
    maUserBuffer.reset();
}

bool OpenGLSalBitmap::AllocateUserData()
{
    Destroy();

    if( mnWidth && mnHeight )
    {
        mnBytesPerRow =  0;

        switch( mnBits )
        {
        case 1:     mnBytesPerRow = (mnWidth + 7) >> 3; break;
        case 4:     mnBytesPerRow = (mnWidth + 1) >> 1; break;
        case 8:     mnBytesPerRow = mnWidth; break;
        case 16:    mnBytesPerRow = mnWidth << 1; break;
        case 24:    mnBytesPerRow = (mnWidth << 1) + mnWidth; break;
        case 32:    mnBytesPerRow = mnWidth << 2; break;
        default:
            OSL_FAIL("vcl::OpenGLSalBitmap::AllocateUserData(), illegal bitcount!");
        }
    }

    bool alloc = false;
    if (mnBytesPerRow != 0
        && mnBytesPerRow <= std::numeric_limits<sal_uInt32>::max() / mnHeight)
    {
        try
        {
            maUserBuffer.reset( new sal_uInt8[mnBytesPerRow * mnHeight] );
            alloc = true;
        }
        catch (std::bad_alloc &) {}
    }
    if (!alloc)
    {
        SAL_WARN(
            "vcl.opengl", "bad alloc " << mnBytesPerRow << "x" << mnHeight);
        maUserBuffer.reset( static_cast<sal_uInt8*>(NULL) );
        mnBytesPerRow = 0;
    }
#ifdef DBG_UTIL
    else
    {
        for (size_t i = 0; i < size_t(mnBytesPerRow * mnHeight); i++)
            maUserBuffer.get()[i] = (i & 0xFF);
    }
#endif

    return maUserBuffer.get() != 0;
}

class ImplPixelFormat
{
protected:
    sal_uInt8* mpData;
public:
    static ImplPixelFormat* GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette );

    virtual void StartLine( sal_uInt8* pLine ) { mpData = pLine; }
    virtual const BitmapColor& ReadPixel() = 0;
    virtual ~ImplPixelFormat() { }
};

class ImplPixelFormat8 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;

public:
    ImplPixelFormat8( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual const BitmapColor& ReadPixel() SAL_OVERRIDE
    {
        return mrPalette[ *mpData++ ];
    }
};

class ImplPixelFormat4 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    ImplPixelFormat4( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual void StartLine( sal_uInt8* pLine ) SAL_OVERRIDE
    {
        mpData = pLine;
        mnX = 0;
        mnShift = 4;
    }
    virtual const BitmapColor& ReadPixel() SAL_OVERRIDE
    {
        const BitmapColor& rColor = mrPalette[( mpData[mnX >> 1] >> mnShift) & 0x0f];
        mnX++;
        mnShift ^= 4;
        return rColor;
    }
};

class ImplPixelFormat1 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;

public:
    ImplPixelFormat1( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual void StartLine( sal_uInt8* pLine ) SAL_OVERRIDE
    {
        mpData = pLine;
        mnX = 0;
    }
    virtual const BitmapColor& ReadPixel() SAL_OVERRIDE
    {
        const BitmapColor& rColor = mrPalette[ (mpData[mnX >> 3 ] >> ( 7 - ( mnX & 7 ) )) & 1];
        mnX++;
        return rColor;
    }
};

ImplPixelFormat* ImplPixelFormat::GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette )
{
    switch( nBits )
    {
    case 1: return new ImplPixelFormat1( rPalette );
    case 4: return new ImplPixelFormat4( rPalette );
    case 8: return new ImplPixelFormat8( rPalette );
    }

    return 0;
}

Size OpenGLSalBitmap::GetSize() const
{
    return Size( mnWidth, mnHeight );
}

GLuint OpenGLSalBitmap::CreateTexture()
{
    GLenum nFormat, nType;
    sal_uInt8* pData( NULL );
    bool bAllocated( false );

    if( maUserBuffer.get() != 0 )
    {
        if( mnBits == 16 || mnBits == 24 || mnBits == 32 )
        {
            // no conversion needed for truecolor
            pData = maUserBuffer.get();

            switch( mnBits )
            {
            case 16:    nFormat = GL_RGB;
                        nType = GL_UNSIGNED_SHORT_5_6_5;
                        break;
            case 24:    nFormat = GL_RGB;
                        nType = GL_UNSIGNED_BYTE;
                        break;
            case 32:    nFormat = GL_RGBA;
                        nType = GL_UNSIGNED_BYTE;
                        break;
            }
        }
        else if( mnBits == 8 && maPalette.IsGreyPalette() )
        {
            // no conversion needed for grayscale
            pData = maUserBuffer.get();
            nFormat = GL_LUMINANCE;
            nType = GL_UNSIGNED_BYTE;
        }
        else
        {
            // convert to 32 bits RGBA using palette
            pData = new sal_uInt8[ mnTexHeight * (mnTexWidth << 2) ];
            bAllocated = true;
            nFormat = GL_RGBA;
            nType = GL_UNSIGNED_BYTE;

            ImplPixelFormat* pSrcFormat = ImplPixelFormat::GetFormat( mnBits, maPalette );
            sal_uInt8* pSrcData = maUserBuffer.get();
            sal_uInt8* pDstData = pData;

            sal_uInt32 nY = mnTexHeight;
            while( nY-- )
            {
                pSrcFormat->StartLine( pSrcData );

                sal_uInt32 nX = mnTexWidth;
                while( nX-- )
                {
                    const BitmapColor& c = pSrcFormat->ReadPixel();

                    *pDstData++ = c.GetRed();
                    *pDstData++ = c.GetGreen();
                    *pDstData++ = c.GetBlue();
                    *pDstData++ = 255;
                }

                pSrcData += mnBytesPerRow;
            }
        }
    }

    mpContext->makeCurrent();
    if( !mnTexture )
        glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mnWidth, mnHeight, 0, nFormat, nType, pData );
    glBindTexture( GL_TEXTURE_2D, 0 );

    if( bAllocated )
        delete pData;

    mbDirtyTexture = false;
    return mnTexture;
}

void OpenGLSalBitmap::DeleteTexture()
{
    if( mnTexture )
    {
        mpContext->makeCurrent();
        glDeleteTextures( 1, &mnTexture );
        mnTexture = 0;
    }
}

void OpenGLSalBitmap::DrawTexture( GLuint nTexture, const SalTwoRect& /*rPosAry*/ )
{
    GLushort aTexCoord[8];
    GLushort aVertices[8];

    /*if( mnTextureProgram == 0 )
    {
        if( !CreateTextureProgram() )
            return;
    }*/

    //glUseProgram( mnTextureProgram );
    //glUniform1i( mnSamplerUniform, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, nTexture );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 8, GL_UNSIGNED_SHORT, GL_FALSE, 0, aTexCoord );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 8, GL_UNSIGNED_SHORT, GL_FALSE, 0, aVertices );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glUseProgram( 0 );
}

bool OpenGLSalBitmap::ReadTexture()
{
    SalTwoRect aPosAry;
    GLuint nFramebufferId, nRenderbufferDepthId, nRenderbufferColorId;
    sal_uInt8* pData = maUserBuffer.get();

    // TODO Check mnTexWidth and mnTexHeight

    mpContext->makeCurrent();
    OpenGLHelper::createFramebuffer( mnTexWidth, mnTexHeight, nFramebufferId,
        nRenderbufferDepthId, nRenderbufferColorId, true );
    glBindFramebuffer( GL_FRAMEBUFFER, nFramebufferId );

    aPosAry.mnSrcX = aPosAry.mnDestX = 0;
    aPosAry.mnSrcY = aPosAry.mnDestY = 0;
    aPosAry.mnSrcWidth = aPosAry.mnDestWidth = mnTexWidth;
    aPosAry.mnSrcHeight = aPosAry.mnDestHeight = mnTexHeight;

    DrawTexture( mnTexture, aPosAry );
    glReadPixels( 0, 0, mnTexWidth, mnTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, pData );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDeleteFramebuffers( 1, &nFramebufferId );
    glDeleteRenderbuffers( 1, &nRenderbufferDepthId );
    glDeleteRenderbuffers( 1, &nRenderbufferColorId );

    return true;
}

sal_uInt16 OpenGLSalBitmap::GetBitCount() const
{
    return mnBits;
}

BitmapBuffer* OpenGLSalBitmap::AcquireBuffer( bool /*bReadOnly*/ )
{
    if( !maUserBuffer.get() )
    {
        if( !AllocateUserData() )
            return NULL;
        if( mnTexture && !ReadTexture() )
            return NULL;
    }

    BitmapBuffer* pBuffer = new BitmapBuffer;
    pBuffer->mnWidth = mnWidth;
    pBuffer->mnHeight = mnHeight;
    pBuffer->maPalette = maPalette;
    pBuffer->mnScanlineSize = mnBytesPerRow;
    pBuffer->mpBits = maUserBuffer.get();
    pBuffer->mnBitCount = mnBits;
    switch( mnBits )
    {
    case 1:     pBuffer->mnFormat = BMP_FORMAT_1BIT_MSB_PAL; break;
    case 4:     pBuffer->mnFormat = BMP_FORMAT_4BIT_MSN_PAL; break;
    case 8:     pBuffer->mnFormat = BMP_FORMAT_8BIT_PAL; break;
    case 16:    pBuffer->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
                pBuffer->maColorMask  = ColorMask( 0xf800, 0x07e0, 0x001f );
                break;
    case 24:    pBuffer->mnFormat = BMP_FORMAT_24BIT_TC_BGR; break;
    case 32:    pBuffer->mnFormat = BMP_FORMAT_32BIT_TC_ARGB;
                pBuffer->maColorMask  = ColorMask( 0x00ff0000, 0x0000ff00, 0x000000ff );
                break;
    }
    // FIXME pBuffer->mnFormat |= BMP_FORMAT_BOTTOM_UP;

    return pBuffer;
}

void OpenGLSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    if( !bReadOnly )
    {
        mbDirtyTexture = true;
    }
    delete pBuffer;
}

bool OpenGLSalBitmap::GetSystemData( BitmapSystemData& /*rData*/ )
{
#if 0
    // TODO Implement for ANDROID/OSX/IOS/WIN32
    X11SalBitmap rBitmap;
    BitmapBuffer* pBuffer;

    rBitmap.Create( GetSize(), mnBits, maPalette );
    pBuffer = rBitmap.AcquireBuffer( false );
    if( pBuffer == NULL )
        return false;

    if( !maUserBuffer.get() )
    {
        if( !AllocateUserData() || !ReadTexture() )
        {
            rBitmap.ReleaseBuffer( pBuffer, false );
            return false;
        }
    }

    // TODO Might be more efficient to add a static method to SalBitmap
    //      to get system data from a buffer
    memcpy( pBuffer->mpBits, maUserBuffer.get(), mnBytesPerRow * mnHeight );

    rBitmap.ReleaseBuffer( pBuffer, false );
    return rBitmap.GetSystemData( rData );
#else
    return false;
#endif
}

bool OpenGLSalBitmap::Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
    SAL_INFO( "vcl.opengl", "::Scale" );
    mnWidth *= rScaleX;
    mnHeight *= rScaleY;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
