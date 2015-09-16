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
#include "vcl/checksum.hxx"
#include "vcl/outdev.hxx"
#include "vcl/salbtype.hxx"
#include "svdata.hxx"
#include "salgdi.hxx"
#include "vcleventlisteners.hxx"

#include "opengl/zone.hxx"
#include "opengl/program.hxx"
#include "opengl/salbmp.hxx"

#include "opengl/FixedTextureAtlas.hxx"

namespace
{

static bool isValidBitCount( sal_uInt16 nBitCount )
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 16) || (nBitCount == 24) || (nBitCount == 32);
}

static std::vector<std::unique_ptr<FixedTextureAtlasManager>> sTextureAtlases;

}

OpenGLSalBitmap::OpenGLSalBitmap()
: mpContext(NULL)
, mbDirtyTexture(true)
, mnBits(0)
, mnBytesPerRow(0)
, mnWidth(0)
, mnHeight(0)
, mnBufWidth(0)
, mnBufHeight(0)
{
}

OpenGLSalBitmap::~OpenGLSalBitmap()
{
    Destroy();
    SAL_INFO( "vcl.opengl", "~OpenGLSalBitmap" );
}

bool OpenGLSalBitmap::Create( const OpenGLTexture& rTex, long nX, long nY, long nWidth, long nHeight )
{
    static const BitmapPalette aEmptyPalette;
    OpenGLZone aZone;

    Destroy();
    SAL_INFO( "vcl.opengl", "OpenGLSalBitmap::Create from FBO: [" << nX << ", " << nY << "] " << nWidth << "x" << nHeight );

    mnWidth = nWidth;
    mnHeight = nHeight;
    mnBufWidth = 0;
    mnBufHeight = 0;

    // TODO Check the framebuffer configuration
    mnBits = 32;
    maPalette = aEmptyPalette;

    if( rTex )
        maTexture = OpenGLTexture( rTex, nX, nY, nWidth, nHeight );
    else
        maTexture = OpenGLTexture( nX, nY, nWidth, nHeight );
    mbDirtyTexture = false;
    SAL_INFO( "vcl.opengl", "Created texture " << maTexture.Id() );

    return true;
}

bool OpenGLSalBitmap::Create( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rBitmapPalette )
{
    OpenGLZone aZone;

    Destroy();
    SAL_INFO( "vcl.opengl", "OpenGLSalBitmap::Create with size: " << rSize );

    if( !isValidBitCount( nBits ) )
        return false;
    maPalette = rBitmapPalette;
    mnBits = nBits;
    mnWidth = mnBufWidth = rSize.Width();
    mnHeight = mnBufHeight = rSize.Height();
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
    OpenGLZone aZone;

    // check that carefully only in the debug mode
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBmp));

    const OpenGLSalBitmap& rSourceBitmap = static_cast<const OpenGLSalBitmap&>(rSalBmp);

    SAL_INFO( "vcl.opengl", "OpenGLSalBitmap::Create from BMP: " << rSourceBitmap.mnWidth << "x" << rSourceBitmap.mnHeight );

    if( isValidBitCount( nNewBitCount ) )
    {
        // TODO: lfrb: What about the pending operations?!
        mnBits = nNewBitCount;
        mnBytesPerRow = rSourceBitmap.mnBytesPerRow;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        mnBufWidth = rSourceBitmap.mnBufWidth;
        mnBufHeight = rSourceBitmap.mnBufHeight;
        maPalette = rSourceBitmap.maPalette;
        // execute any pending operations on the source bitmap
        maTexture = rSourceBitmap.GetTexture();
        mbDirtyTexture = false;

        // be careful here, we are share & reference-count the
        // maUserBuffer, BUT this Create() is called from
        // Bitmap::ImplMakeUnique().
        // Consequently, there might be cases when this needs to be made
        // unique later (when we don't do that right away here), like when
        // using the BitmapWriteAccess.
        maUserBuffer = rSourceBitmap.maUserBuffer;

        return true;
    }
    return false;
}

bool OpenGLSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >& /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    // TODO Is this method needed?
    return false;
}

OpenGLTexture& OpenGLSalBitmap::GetTexture() const
{
    OpenGLSalBitmap* pThis = const_cast<OpenGLSalBitmap*>(this);
    if( !maTexture || mbDirtyTexture )
        pThis->CreateTexture();
    else if( !maPendingOps.empty() )
        pThis->ExecuteOperations();
    SAL_INFO( "vcl.opengl", "Got texture " << maTexture.Id() );
    return pThis->maTexture;
}

void OpenGLSalBitmap::Destroy()
{
    OpenGLZone aZone;

    SAL_INFO( "vcl.opengl", "Destroy OpenGLSalBitmap" );
    maPendingOps.clear();
    maTexture = OpenGLTexture();
    maUserBuffer.reset();
}

bool OpenGLSalBitmap::AllocateUserData()
{
    SAL_INFO( "vcl.opengl", "OpenGLSalBitmap::AllocateUserData" );

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
    if (mnBytesPerRow != 0 && mnHeight &&
        mnBytesPerRow <= std::numeric_limits<sal_uInt32>::max() / mnHeight)
    {
        try
        {
            maUserBuffer.reset( new sal_uInt8[static_cast<sal_uInt32>(mnBytesPerRow) * mnHeight] );
            alloc = true;
        }
        catch (const std::bad_alloc &) {}
    }
    if (!alloc)
    {
        SAL_WARN("vcl.opengl", "bad alloc " << mnBytesPerRow << "x" << mnHeight);
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

namespace {

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
    explicit ImplPixelFormat8( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual const BitmapColor& ReadPixel() SAL_OVERRIDE
    {
        assert( mrPalette.GetEntryCount() > *mpData );
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
    explicit ImplPixelFormat4( const BitmapPalette& rPalette )
        : mrPalette( rPalette )
        , mnX(0)
        , mnShift(4)
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
        sal_uInt32 nIdx = ( mpData[mnX >> 1] >> mnShift) & 0x0f;
        assert( mrPalette.GetEntryCount() > nIdx );
        const BitmapColor& rColor = mrPalette[nIdx];
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
    explicit ImplPixelFormat1( const BitmapPalette& rPalette )
        : mrPalette(rPalette)
        , mnX(0)
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

void lclInstantiateTexture(OpenGLTexture& rTexture, const int nWidth, const int nHeight,
                           const GLenum nFormat, const GLenum nType, sal_uInt8* pData)
{
    if (nWidth == nHeight)
    {
        if (sTextureAtlases.empty())
        {
            sTextureAtlases.push_back(std::unique_ptr<FixedTextureAtlasManager>(new FixedTextureAtlasManager(8, 8, 16)));
            sTextureAtlases.push_back(std::unique_ptr<FixedTextureAtlasManager>(new FixedTextureAtlasManager(8, 8, 24)));
            sTextureAtlases.push_back(std::unique_ptr<FixedTextureAtlasManager>(new FixedTextureAtlasManager(8, 8, 32)));
            sTextureAtlases.push_back(std::unique_ptr<FixedTextureAtlasManager>(new FixedTextureAtlasManager(8, 8, 48)));
            sTextureAtlases.push_back(std::unique_ptr<FixedTextureAtlasManager>(new FixedTextureAtlasManager(8, 8, 64)));
        }
        for (size_t i = 0; i < sTextureAtlases.size(); i++)
        {
            if (nWidth == sTextureAtlases[i]->GetSubtextureSize())
            {
                rTexture = sTextureAtlases[i]->InsertBuffer(nWidth, nHeight, nFormat, nType, pData);
                return;
            }
        }
    }
    rTexture = OpenGLTexture (nWidth, nHeight, nFormat, nType, pData);
}

}

Size OpenGLSalBitmap::GetSize() const
{
    OpenGLZone aZone;

    std::deque< OpenGLSalBitmapOp* >::const_iterator it = maPendingOps.begin();
    Size aSize( mnWidth, mnHeight );

    while( it != maPendingOps.end() )
        (*it++)->GetSize( aSize );

    return aSize;
}

void OpenGLSalBitmap::ExecuteOperations()
{
    makeCurrent();
    while( !maPendingOps.empty() )
    {
        OpenGLSalBitmapOp* pOp = maPendingOps.front();
        pOp->Execute();
        maPendingOps.pop_front();
    }
}

GLuint OpenGLSalBitmap::CreateTexture()
{
    SAL_INFO( "vcl.opengl", "::CreateTexture" );
    GLenum nFormat = GL_RGBA;
    GLenum nType = GL_UNSIGNED_BYTE;
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
            pData = new sal_uInt8[mnBufHeight * mnBufWidth * 4];
            bAllocated = true;
            nFormat = GL_RGBA;
            nType = GL_UNSIGNED_BYTE;

            ImplPixelFormat* pSrcFormat = ImplPixelFormat::GetFormat( mnBits, maPalette );
            sal_uInt8* pSrcData = maUserBuffer.get();
            sal_uInt8* pDstData = pData;

            sal_uInt32 nY = mnBufHeight;
            while( nY-- )
            {
                pSrcFormat->StartLine( pSrcData );

                sal_uInt32 nX = mnBufWidth;
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

    makeCurrent();

    lclInstantiateTexture(maTexture, mnBufWidth, mnBufHeight, nFormat, nType, pData);

    SAL_INFO( "vcl.opengl", "Created texture " << maTexture.Id() );

    if( bAllocated )
        delete[] pData;

    ExecuteOperations();
    mbDirtyTexture = false;

    CHECK_GL_ERROR();
    return maTexture.Id();
}

bool OpenGLSalBitmap::ReadTexture()
{
    sal_uInt8* pData = maUserBuffer.get();

    SAL_INFO( "vcl.opengl", "::ReadTexture " << mnWidth << "x" << mnHeight );

    if( pData == NULL )
        return false;

    if (mnBits == 8 || mnBits == 16 || mnBits == 24 || mnBits == 32)
    {
        GLenum nFormat = GL_RGBA;
        GLenum nType = GL_UNSIGNED_BYTE;

        switch( mnBits )
        {
        case 8:     nFormat = GL_LUMINANCE;
                    nType = GL_UNSIGNED_BYTE;
                    break;
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

        makeCurrent();
        maTexture.Read(nFormat, nType, pData);
        mnBufWidth = mnWidth;
        mnBufHeight = mnHeight;
        return true;
    }
    else if (mnBits == 1)
    {   // convert buffers from 24-bit RGB to 1-bit Mask
        std::vector<sal_uInt8> aBuffer(mnWidth * mnHeight * 3);
        makeCurrent();
        sal_uInt8* pBuffer = aBuffer.data();
        maTexture.Read(GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

        int nShift = 7;
        size_t nIndex = 0;

        sal_uInt8* pCurrent = pBuffer;

        for (size_t i = 0; i < aBuffer.size(); i += 3)
        {
            sal_uInt8 nR = *pCurrent++;
            sal_uInt8 nG = *pCurrent++;
            sal_uInt8 nB = *pCurrent++;

            if (nR > 0 && nG > 0 && nB > 0)
            {
                pData[nIndex] |= (1 << nShift);
            }
            nShift--;
            if (nShift < 0)
            {
                nShift = 7;
                nIndex++;
                pData[nIndex] = 0;
            }
        }

        mnBufWidth = mnWidth;
        mnBufHeight = mnHeight;
        return true;
    }

    SAL_WARN("vcl.opengl", "::ReadTexture - tx:" << maTexture.Id() << " @ "
             << mnWidth << "x" << mnHeight << "- unimplemented bit depth: "
             << mnBits);
    return false;

}

sal_uInt16 OpenGLSalBitmap::GetBitCount() const
{
    return mnBits;
}

bool OpenGLSalBitmap::calcChecksumGL(OpenGLTexture& rInputTexture, ChecksumType& rChecksum) const
{
    OUString FragShader("areaHashCRC64TFragmentShader");

    static const OpenGLTexture aCRCTableTexture(512, 1, GL_RGBA, GL_UNSIGNED_BYTE,
            vcl_get_crc64_table());

    // First Pass

    int nWidth = rInputTexture.GetWidth();
    int nHeight = rInputTexture.GetHeight();

    OpenGLProgram* pProgram = mpContext->UseProgram("textureVertexShader", FragShader);
    if (pProgram == 0)
        return false;

    int nNewWidth = ceil( nWidth / 4.0 );
    int nNewHeight = ceil( nHeight / 4.0 );

    OpenGLTexture aFirstPassTexture = OpenGLTexture(nNewWidth, nNewHeight);
    OpenGLFramebuffer* pFramebuffer = mpContext->AcquireFramebuffer(aFirstPassTexture);

    pProgram->SetUniform1f( "xstep", 1.0 / mnWidth );
    pProgram->SetUniform1f( "ystep", 1.0 / mnHeight );

    pProgram->SetTexture("crc_table", (OpenGLTexture&)(aCRCTableTexture));
    pProgram->SetTexture("sampler", rInputTexture);
    pProgram->DrawTexture(rInputTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();


    // Second Pass

    nWidth = aFirstPassTexture.GetWidth();
    nHeight = aFirstPassTexture.GetHeight();

    pProgram = mpContext->UseProgram("textureVertexShader", FragShader);
    if (pProgram == 0)
        return false;

    nNewWidth = ceil( nWidth / 4.0 );
    nNewHeight = ceil( nHeight / 4.0 );

    OpenGLTexture aSecondPassTexture = OpenGLTexture(nNewWidth, nNewHeight);
    pFramebuffer = mpContext->AcquireFramebuffer(aSecondPassTexture);

    pProgram->SetUniform1f( "xstep", 1.0 / mnWidth );
    pProgram->SetUniform1f( "ystep", 1.0 / mnHeight );

    pProgram->SetTexture("crc_table", (OpenGLTexture&)(aCRCTableTexture));
    pProgram->SetTexture("sampler", aFirstPassTexture);
    pProgram->DrawTexture(aFirstPassTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();

    // Final CRC on CPU
    OpenGLTexture& aFinalTexture = aSecondPassTexture;
    std::vector<sal_uInt8> aBuf( aFinalTexture.GetWidth() * aFinalTexture.GetHeight() * 4 );
    aFinalTexture.Read(GL_RGBA, GL_UNSIGNED_BYTE, aBuf.data());

    ChecksumType nCrc = vcl_get_checksum(0, aBuf.data(), aBuf.size());

    rChecksum = nCrc;
    return true;
}

void OpenGLSalBitmap::updateChecksum() const
{
    if (mbChecksumValid)
        return;

    OpenGLSalBitmap* pThis = const_cast<OpenGLSalBitmap*>(this);

    if (!mpContext.is())
    {
        pThis->CreateTexture();
    }

    OpenGLTexture& rInputTexture = pThis->maTexture;
    int nWidth = rInputTexture.GetWidth();
    int nHeight = rInputTexture.GetHeight();

    if( (nWidth * nHeight) < (1024*768) || nWidth < 128 || nHeight < 128 )
    {
        SalBitmap::updateChecksum();
    }
    else
    {
        pThis->mbChecksumValid = calcChecksumGL(rInputTexture, pThis->mnChecksum);
    }
}

rtl::Reference<OpenGLContext> OpenGLSalBitmap::GetBitmapContext()
{
    return ImplGetDefaultWindow()->GetGraphics()->GetOpenGLContext();
}

void OpenGLSalBitmap::makeCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    // TODO: make sure we can really use the last used context
    OpenGLContext *pContext = pSVData->maGDIData.mpLastContext;
    while( pContext && !pContext->isInitialized() )
        pContext = pContext->mpPrevContext;
    if( pContext )
        mpContext = pContext;
    else
        mpContext = GetBitmapContext();
    assert(mpContext.is() && "Couldn't get an OpenGL context");
    mpContext->makeCurrent();
}

BitmapBuffer* OpenGLSalBitmap::AcquireBuffer( BitmapAccessMode nMode )
{
    OpenGLZone aZone;

    if( nMode != BITMAP_INFO_ACCESS )
    {
        if( !maUserBuffer.get() )
        {
            if( !AllocateUserData() )
                return NULL;
            if( maTexture && !ReadTexture() )
                return NULL;
        }

        if( !maPendingOps.empty() )
        {
            SAL_INFO( "vcl.opengl", "** Creating texture and reading it back immediately" );
            if( !CreateTexture() || !AllocateUserData() || !ReadTexture() )
                return NULL;
        }
    }

    // maUserBuffer must be unique when we are doing the write access
    if (nMode == BITMAP_WRITE_ACCESS && maUserBuffer && !maUserBuffer.unique())
    {
        basebmp::RawMemorySharedArray aBuffer(maUserBuffer);

        maUserBuffer.reset();
        AllocateUserData();
        memcpy(maUserBuffer.get(), aBuffer.get(), static_cast<sal_uInt32>(mnBytesPerRow) * mnHeight);
    }

    BitmapBuffer* pBuffer = new BitmapBuffer;
    pBuffer->mnWidth = mnWidth;
    pBuffer->mnHeight = mnHeight;
    pBuffer->maPalette = maPalette;
    pBuffer->mnScanlineSize = mnBytesPerRow;
    pBuffer->mpBits = maUserBuffer.get();
    pBuffer->mnBitCount = mnBits;
    switch (mnBits)
    {
        case 1:
            pBuffer->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
            break;
        case 4:
            pBuffer->mnFormat = BMP_FORMAT_4BIT_MSN_PAL;
            break;
        case 8:
            pBuffer->mnFormat = BMP_FORMAT_8BIT_PAL;
            break;
        case 16:
        {
            pBuffer->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
            ColorMaskElement aRedMask(0xf800);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x07e0);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x001f);
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask  = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
        case 24:
            pBuffer->mnFormat = BMP_FORMAT_24BIT_TC_RGB;
            break;
        case 32:
        {
            pBuffer->mnFormat = BMP_FORMAT_32BIT_TC_RGBA;
            ColorMaskElement aRedMask(0xff000000);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x00ff0000);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x0000ff00);
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask  = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
    }

    return pBuffer;
}

void OpenGLSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    OpenGLZone aZone;

    if( nMode == BITMAP_WRITE_ACCESS )
    {
        maTexture = OpenGLTexture();
        mbDirtyTexture = true;
        mbChecksumValid = false;
    }
    // The palette is modified on read during the BitmapWriteAccess,
    // but of course, often it is not modified; interesting.
    maPalette = pBuffer->maPalette;

    // Are there any more ground movements underneath us ?
    assert( pBuffer->mnWidth == mnWidth );
    assert( pBuffer->mnHeight == mnHeight );
    assert( pBuffer->mnBitCount == mnBits );

    delete pBuffer;
}

bool OpenGLSalBitmap::GetSystemData( BitmapSystemData& /*rData*/ )
{
    SAL_WARN( "vcl.opengl", "*** NOT IMPLEMENTED *** GetSystemData" );
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

bool OpenGLSalBitmap::Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol )
{
    OpenGLZone aZone;

    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;

    GetTexture();
    makeCurrent();
    pProgram = mpContext->UseProgram( "textureVertexShader",
                                      "replaceColorFragmentShader" );
    if( !pProgram )
        return false;

    OpenGLTexture aNewTex = OpenGLTexture( mnWidth, mnHeight );
    pFramebuffer = mpContext->AcquireFramebuffer( aNewTex );

    pProgram->SetTexture( "sampler", maTexture );
    pProgram->SetColor( "search_color", rSearchColor );
    pProgram->SetColor( "replace_color", rReplaceColor );
    pProgram->SetUniform1f( "epsilon", nTol / 255.0f );
    pProgram->DrawTexture( maTexture );
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer( pFramebuffer );
    maTexture = aNewTex;

    CHECK_GL_ERROR();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
