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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/checksum.hxx>
#include <vcl/outdev.hxx>
#include <svdata.hxx>
#include <salgdi.hxx>
#include <vcleventlisteners.hxx>
#include <vcl/lazydelete.hxx>
#include <scanlinewriter.hxx>

#include <o3tl/make_shared.hxx>

#include <opengl/zone.hxx>
#include <opengl/program.hxx>
#include <opengl/salbmp.hxx>
#include <opengl/RenderState.hxx>
#include <opengl/FixedTextureAtlas.hxx>

#if OSL_DEBUG_LEVEL > 0
#  define CANARY "tex-canary"
#endif

namespace
{

bool determineTextureFormat(sal_uInt16 nBits, GLenum& nFormat, GLenum& nType)
{
    switch(nBits)
    {
    case 8:
        nFormat = GL_LUMINANCE;
        nType = GL_UNSIGNED_BYTE;
        return true;
    case 24:
        nFormat = GL_RGB;
        nType = GL_UNSIGNED_BYTE;
        return true;
    case 32:
        nFormat = GL_RGBA;
        nType = GL_UNSIGNED_BYTE;
        return true;
    default:
        break;
    }
    SAL_WARN("vcl.opengl", "Could not determine the appropriate texture format for input bits '" << nBits << "'");
    return false;
}

bool isValidBitCount( sal_uInt16 nBitCount )
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 24) || (nBitCount == 32);
}

sal_uInt32 lclBytesPerRow(sal_uInt16 nBits, int nWidth)
{
    switch(nBits)
    {
    case 1:  return (nWidth + 7) >> 3;
    case 4:  return (nWidth + 1) >> 1;
    case 8:  return  nWidth;
    case 24: return  nWidth * 3;
    case 32: return  nWidth * 4;
    default:
        OSL_FAIL("vcl::OpenGLSalBitmap::AllocateUserData(), illegal bitcount!");
    }
    return 0;
}
}

OpenGLSalBitmap::OpenGLSalBitmap()
: mbDirtyTexture(true)
, mnBits(0)
, mnBytesPerRow(0)
, mnWidth(0)
, mnHeight(0)
{
}

OpenGLSalBitmap::~OpenGLSalBitmap()
{
    Destroy();
    VCL_GL_INFO( "~OpenGLSalBitmap" );
}

void OpenGLSalBitmap::Create( const OpenGLTexture& rTex, tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    DBG_TESTSOLARMUTEX();
    static const BitmapPalette aEmptyPalette;
    OpenGLVCLContextZone aContextZone;

    Destroy();
    VCL_GL_INFO( "OpenGLSalBitmap::Create from FBO: ["
                  << nX << ", " << nY << "] " << nWidth << "x" << nHeight );

    GLint nMaxTextureSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &nMaxTextureSize );
    if ( nWidth > nMaxTextureSize )
    {
        nWidth = nMaxTextureSize;
        VCL_GL_INFO( "Width limited to " << nMaxTextureSize );
    }

    if ( nHeight > nMaxTextureSize )
    {
        nHeight = nMaxTextureSize;
        VCL_GL_INFO( "Height limited to " << nMaxTextureSize );
    }

    mnWidth = nWidth;
    mnHeight = nHeight;

    // TODO Check the framebuffer configuration
    mnBits = 32;
    maPalette = aEmptyPalette;

    if( rTex )
        maTexture = OpenGLTexture( rTex, nX, nY, nWidth, nHeight );
    else
        maTexture = OpenGLTexture( nX, nY, nWidth, nHeight );
    mbDirtyTexture = false;
    VCL_GL_INFO( "Created texture " << maTexture.Id() );

    assert(mnWidth == maTexture.GetWidth() &&
           mnHeight == maTexture.GetHeight());
}

bool OpenGLSalBitmap::Create( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rBitmapPalette )
{
    DBG_TESTSOLARMUTEX();
    OpenGLVCLContextZone aContextZone;

    Destroy();
    VCL_GL_INFO( "OpenGLSalBitmap::Create with size: " << rSize );

    if( !isValidBitCount( nBits ) )
        return false;
    maPalette = rBitmapPalette;
    mnBits = nBits;
    mnWidth = rSize.Width();
    mnHeight = rSize.Height();

    // Limit size to what GL allows, so later glTexImage2D() won't fail.
    GLint nMaxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nMaxTextureSize);
    if (mnWidth > nMaxTextureSize)
        mnWidth = nMaxTextureSize;
    if (mnHeight > nMaxTextureSize)
        mnHeight = nMaxTextureSize;

    return false;
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp )
{
    DBG_TESTSOLARMUTEX();
    return Create( rSalBmp, rSalBmp.GetBitCount() );
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    DBG_TESTSOLARMUTEX();
    return Create( rSalBmp, pGraphics ? pGraphics->GetBitCount() : rSalBmp.GetBitCount() );
}

bool OpenGLSalBitmap::Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount )
{
    DBG_TESTSOLARMUTEX();
    OpenGLZone aZone;

    // check that carefully only in the debug mode
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBmp));

    const OpenGLSalBitmap& rSourceBitmap = static_cast<const OpenGLSalBitmap&>(rSalBmp);

    VCL_GL_INFO("OpenGLSalBitmap::Create from BMP: "
                << rSourceBitmap.mnWidth << "x" << rSourceBitmap.mnHeight
                << " Bits old: " << mnBits << " new:" << nNewBitCount );

    if( isValidBitCount( nNewBitCount ) )
    {
        // TODO: lfrb: What about the pending operations?!
        mnBits = nNewBitCount;
        mnBytesPerRow = rSourceBitmap.mnBytesPerRow;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        maPalette = rSourceBitmap.maPalette;
        // execute any pending operations on the source bitmap
        maTexture = rSourceBitmap.GetTexture();
        mbDirtyTexture = false;

        // be careful here, we are share & reference-count the
        // mpUserBuffer, BUT this Create() is called from
        // Bitmap::ImplMakeUnique().
        // Consequently, there might be cases when this needs to be made
        // unique later (when we don't do that right away here), like when
        // using the BitmapWriteAccess.
        mpUserBuffer = rSourceBitmap.mpUserBuffer;

        return true;
    }
    return false;
}

bool OpenGLSalBitmap::Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    DBG_TESTSOLARMUTEX();
    // TODO Is this method needed?
    return false;
}

OpenGLTexture& OpenGLSalBitmap::GetTexture() const
{
    OpenGLSalBitmap* pThis = const_cast<OpenGLSalBitmap*>(this);
    if( !maTexture || mbDirtyTexture )
        pThis->CreateTexture();
    VCL_GL_INFO( "Got texture " << maTexture.Id() );
    return pThis->maTexture;
}

void OpenGLSalBitmap::Destroy()
{
    OpenGLZone aZone;

    VCL_GL_INFO("Destroy OpenGLSalBitmap texture:" << maTexture.Id());
    maTexture = OpenGLTexture();
    DeallocateUserData();
}

bool OpenGLSalBitmap::AllocateUserData()
{
    VCL_GL_INFO( "OpenGLSalBitmap::AllocateUserData" );

    if( mnWidth && mnHeight )
    {
        mnBytesPerRow = lclBytesPerRow(mnBits, mnWidth);
    }

    bool alloc = false;
    if (mnBytesPerRow != 0 && mnHeight &&
        mnBytesPerRow <= std::numeric_limits<sal_uInt32>::max() / mnHeight)
    {
        try
        {
            size_t nToAllocate = mnBytesPerRow * mnHeight;
#if OSL_DEBUG_LEVEL > 0
            nToAllocate += sizeof(CANARY);
#endif
            mpUserBuffer = o3tl::make_shared_array<sal_uInt8>(nToAllocate);
#if OSL_DEBUG_LEVEL > 0
            memcpy(mpUserBuffer.get() + nToAllocate - sizeof(CANARY),
                   CANARY, sizeof(CANARY));
#endif
            alloc = true;
        }
        catch (const std::bad_alloc &) {}
    }
    if (!alloc)
    {
        SAL_WARN("vcl.opengl", "bad alloc " << mnBytesPerRow << "x" << mnHeight);
        DeallocateUserData();
    }
#ifdef DBG_UTIL
    else
    {
        for (size_t i = 0; i < size_t(mnBytesPerRow * mnHeight); i++)
            mpUserBuffer.get()[i] = (i & 0xFF);
    }
#endif

    return mpUserBuffer != nullptr;
}

void OpenGLSalBitmap::DeallocateUserData()
{
    mpUserBuffer.reset();
    mnBytesPerRow = 0;
}

namespace {

void lclInstantiateTexture(OpenGLTexture& rTexture, const int nWidth, const int nHeight,
                           const GLenum nFormat, const GLenum nType, sal_uInt8 const * pData)
{
    if (nWidth == nHeight)
    {
        typedef std::vector<std::unique_ptr<FixedTextureAtlasManager>> TextureAtlasVector;
        static vcl::DeleteOnDeinit<TextureAtlasVector> aTextureAtlases([]() {
            TextureAtlasVector* p = new TextureAtlasVector;
            p->reserve(5);
            p->push_back(std::make_unique<FixedTextureAtlasManager>(8, 8, 16));
            p->push_back(std::make_unique<FixedTextureAtlasManager>(8, 8, 24));
            p->push_back(std::make_unique<FixedTextureAtlasManager>(8, 8, 32));
            p->push_back(std::make_unique<FixedTextureAtlasManager>(8, 8, 48));
            p->push_back(std::make_unique<FixedTextureAtlasManager>(8, 8, 64));
            return p;
        }());
        for (std::unique_ptr<FixedTextureAtlasManager>& pTextureAtlas : *aTextureAtlases.get())
        {
            if (nWidth == pTextureAtlas->GetSubtextureSize())
            {
                rTexture = pTextureAtlas->InsertBuffer(nWidth, nHeight, nFormat, nType, pData);
                return;
            }
        }
    }
    rTexture = OpenGLTexture (nWidth, nHeight, nFormat, nType, pData);
}

} // end anonymous namespace

Size OpenGLSalBitmap::GetSize() const
{
    return Size(mnWidth, mnHeight);
}

GLuint OpenGLSalBitmap::CreateTexture()
{
    VCL_GL_INFO( "::CreateTexture bits: " << mnBits);
    GLenum nFormat = GL_RGBA;
    GLenum nType = GL_UNSIGNED_BYTE;
    sal_uInt8* pData( nullptr );
    bool bAllocated( false );

    if (mpUserBuffer != nullptr)
    {
        if( mnBits == 24 || mnBits == 32 )
        {
            // no conversion needed for truecolor
            pData = mpUserBuffer.get();

            determineTextureFormat(mnBits, nFormat, nType);
        }
        else if( mnBits == 8 && maPalette.IsGreyPalette8Bit() )
        {
            // no conversion needed for 8bit grayscale
            pData = mpUserBuffer.get();
            nFormat = GL_LUMINANCE;
            nType = GL_UNSIGNED_BYTE;
        }
        else
        {
            VCL_GL_INFO( "::CreateTexture - convert from " << mnBits << " to 24 bits" );
            // convert to 24 bits RGB using palette
            determineTextureFormat(24, nFormat, nType);
            pData = convertDataBitCount( mpUserBuffer.get(), mnWidth, mnHeight,
                mnBits, mnBytesPerRow, maPalette,
                nFormat == GL_BGR ? BitConvert::BGR : BitConvert::RGB ).release();
            bAllocated = true;
        }
    }

    OpenGLVCLContextZone aContextZone;

    lclInstantiateTexture(maTexture, mnWidth, mnHeight, nFormat, nType, pData);

    VCL_GL_INFO("Created texture " << maTexture.Id() << " bits: " << mnBits);

    if( bAllocated )
        delete[] pData;

    mbDirtyTexture = false;

    CHECK_GL_ERROR();
    return maTexture.Id();
}

bool OpenGLSalBitmap::ReadTexture()
{
    sal_uInt8* pData = mpUserBuffer.get();

    GLenum nFormat = GL_RGBA;
    GLenum nType = GL_UNSIGNED_BYTE;

    VCL_GL_INFO( "::ReadTexture " << mnWidth << "x" << mnHeight << " bits: " << mnBits);

    if( pData == nullptr )
        return false;

    OpenGLVCLContextZone aContextZone;

    rtl::Reference<OpenGLContext> xContext = OpenGLContext::getVCLContext();
    xContext->state().scissor().disable();
    xContext->state().stencil().disable();

    if ((mnBits == 8 && maPalette.IsGreyPalette8Bit()) || mnBits == 24 || mnBits == 32)
    {
        determineTextureFormat(mnBits, nFormat, nType);

#if OSL_DEBUG_LEVEL > 0
        // help valgrind & drmemory rescue us - touch last and first bits.
        pData[0] = 0;
        pData[mnBits/8*mnWidth*mnHeight-1] = 0;
        // if this fails we can read too much into pData
        assert(mnWidth == maTexture.GetWidth() &&
               mnHeight == maTexture.GetHeight());
#endif

        maTexture.Read(nFormat, nType, pData);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
        // If we read over the end of pData we have a real hidden memory
        // corruption problem !
        size_t nCanary = mnBytesPerRow * mnHeight;
        assert(!memcmp(pData + nCanary, CANARY, sizeof (CANARY)));
#endif
        return true;
    }
    else if (mnBits == 1 || mnBits == 4 || mnBits == 8)
    {   // convert buffers from 24-bit RGB to 1,4 or 8-bit buffer
        std::vector<sal_uInt8> aBuffer(mnWidth * mnHeight * 3);

        sal_uInt8* pBuffer = aBuffer.data();
        determineTextureFormat(24, nFormat, nType);
        maTexture.Read(nFormat, nType, pBuffer);
        sal_uInt32 nSourceBytesPerRow = lclBytesPerRow(24, mnWidth);

        std::unique_ptr<vcl::ScanlineWriter> pWriter = vcl::ScanlineWriter::Create(mnBits, maPalette);
        for (int y = 0; y < mnHeight; ++y)
        {
            sal_uInt8* pSource = &pBuffer[y * nSourceBytesPerRow];
            sal_uInt8* pDestination = &pData[y * mnBytesPerRow];

            pWriter->nextLine(pDestination);

            for (int x = 0; x < mnWidth; ++x)
            {
                // read source
                sal_uInt8 nR = *pSource++;
                sal_uInt8 nG = *pSource++;
                sal_uInt8 nB = *pSource++;

                pWriter->writeRGB(nR, nG, nB);
            }
        }
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

bool OpenGLSalBitmap::calcChecksumGL(OpenGLTexture& rInputTexture, BitmapChecksum& rChecksum) const
{
    OUString FragShader("areaHashCRC64TFragmentShader");

    rtl::Reference< OpenGLContext > xContext = OpenGLContext::getVCLContext();
    xContext->state().scissor().disable();
    xContext->state().stencil().disable();

    static vcl::DeleteOnDeinit<OpenGLTexture> gCRCTableTexture(
        new OpenGLTexture(512, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                           vcl_get_crc64_table()));
    OpenGLTexture &rCRCTableTexture = *gCRCTableTexture.get();

    // First Pass

    int nWidth = rInputTexture.GetWidth();
    int nHeight = rInputTexture.GetHeight();

    OpenGLProgram* pProgram = xContext->UseProgram("textureVertexShader", FragShader);
    if (pProgram == nullptr)
        return false;

    int nNewWidth = ceil( nWidth / 4.0 );
    int nNewHeight = ceil( nHeight / 4.0 );

    OpenGLTexture aFirstPassTexture(nNewWidth, nNewHeight);
    OpenGLFramebuffer* pFramebuffer = xContext->AcquireFramebuffer(aFirstPassTexture);

    pProgram->SetUniform1f( "xstep", 1.0 / mnWidth );
    pProgram->SetUniform1f( "ystep", 1.0 / mnHeight );

    pProgram->SetTexture("crc_table", rCRCTableTexture);
    pProgram->SetTexture("sampler", rInputTexture);
    pProgram->DrawTexture(rInputTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();

    // Second Pass

    nWidth = aFirstPassTexture.GetWidth();
    nHeight = aFirstPassTexture.GetHeight();

    pProgram = xContext->UseProgram("textureVertexShader", FragShader);
    if (pProgram == nullptr)
        return false;

    nNewWidth = ceil( nWidth / 4.0 );
    nNewHeight = ceil( nHeight / 4.0 );

    OpenGLTexture aSecondPassTexture(nNewWidth, nNewHeight);
    pFramebuffer = xContext->AcquireFramebuffer(aSecondPassTexture);

    pProgram->SetUniform1f( "xstep", 1.0 / mnWidth );
    pProgram->SetUniform1f( "ystep", 1.0 / mnHeight );

    pProgram->SetTexture("crc_table", rCRCTableTexture);
    pProgram->SetTexture("sampler", aFirstPassTexture);
    pProgram->DrawTexture(aFirstPassTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();

    // Final CRC on CPU
    OpenGLTexture& aFinalTexture = aSecondPassTexture;
    std::vector<sal_uInt8> aBuf( aFinalTexture.GetWidth() * aFinalTexture.GetHeight() * 4 );
    aFinalTexture.Read(GL_RGBA, GL_UNSIGNED_BYTE, aBuf.data());

    BitmapChecksum nCrc = vcl_get_checksum(0, aBuf.data(), aBuf.size());

    rChecksum = nCrc;
    return true;
}

void OpenGLSalBitmap::updateChecksum() const
{
    if (mbChecksumValid)
        return;

    if( (mnWidth * mnHeight) < (1024*768) || mnWidth < 128 || mnHeight < 128 )
    {
        SalBitmap::updateChecksum();
        return;
    }

    OpenGLSalBitmap* pThis = const_cast<OpenGLSalBitmap*>(this);

    OpenGLVCLContextZone aContextZone;
    OpenGLTexture& rInputTexture = GetTexture();
    pThis->mbChecksumValid = calcChecksumGL(rInputTexture, pThis->mnChecksum);
    if (!pThis->mbChecksumValid)
        SalBitmap::updateChecksum();
}

BitmapBuffer* OpenGLSalBitmap::AcquireBuffer( BitmapAccessMode nMode )
{
    OpenGLVCLContextZone aContextZone;

    if( nMode != BitmapAccessMode::Info )
    {
        if (!mpUserBuffer)
        {
            if( !AllocateUserData() )
                return nullptr;
            if( maTexture && !ReadTexture() )
            {
                DeallocateUserData();
                return nullptr;
            }
        }
    }

    // mpUserBuffer must be unique when we are doing the write access
    if (nMode == BitmapAccessMode::Write && mpUserBuffer && mpUserBuffer.use_count() > 1)
    {
        std::shared_ptr<sal_uInt8> aBuffer(mpUserBuffer);

        mpUserBuffer.reset();
        AllocateUserData();
        memcpy(mpUserBuffer.get(), aBuffer.get(), mnBytesPerRow * mnHeight);
    }

    BitmapBuffer* pBuffer = new BitmapBuffer;
    pBuffer->mnWidth = mnWidth;
    pBuffer->mnHeight = mnHeight;
    pBuffer->maPalette = maPalette;
    pBuffer->mnScanlineSize = mnBytesPerRow;
    pBuffer->mpBits = mpUserBuffer.get();
    pBuffer->mnBitCount = mnBits;

    switch (mnBits)
    {
        case 1:
            pBuffer->mnFormat = ScanlineFormat::N1BitMsbPal;
            break;
        case 4:
            pBuffer->mnFormat = ScanlineFormat::N4BitMsnPal;
            break;
        case 8:
            pBuffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 24:
        {
            pBuffer->mnFormat = ScanlineFormat::N24BitTcRgb;
            break;
        }
        case 32:
        {
            pBuffer->mnFormat = ScanlineFormat::N32BitTcRgba;
            ColorMaskElement aRedMask(0xff000000);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x00ff0000);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x0000ff00);
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask  = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
        default: assert(false);
    }

    return pBuffer;
}

void OpenGLSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    OpenGLVCLContextZone aContextZone;

    if( nMode == BitmapAccessMode::Write )
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

    if (!mpUserBuffer.get())
    {
        if( !AllocateUserData() || !ReadTexture() )
        {
            rBitmap.ReleaseBuffer( pBuffer, false );
            DeallocateUserData();
            return false;
        }
    }

    // TODO Might be more efficient to add a static method to SalBitmap
    //      to get system data from a buffer
    memcpy( pBuffer->mpBits, mpUserBuffer.get(), mnBytesPerRow * mnHeight );

    rBitmap.ReleaseBuffer( pBuffer, false );
    return rBitmap.GetSystemData( rData );
#else
    return false;
#endif
}

bool OpenGLSalBitmap::Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol )
{
    VCL_GL_INFO("::Replace");

    OpenGLZone aZone;
    rtl::Reference<OpenGLContext> xContext = OpenGLContext::getVCLContext();
    xContext->state().scissor().disable();
    xContext->state().stencil().disable();

    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;

    GetTexture();
    pProgram = xContext->UseProgram( "textureVertexShader",
                                     "replaceColorFragmentShader" );
    if( !pProgram )
        return false;

    OpenGLTexture aNewTex( mnWidth, mnHeight );
    pFramebuffer = xContext->AcquireFramebuffer( aNewTex );

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

// Convert texture to greyscale and adjust bitmap metadata
bool OpenGLSalBitmap::ConvertToGreyscale()
{
    VCL_GL_INFO("::ConvertToGreyscale");

    // avoid re-converting to 8bits.
    if ( mnBits == 8 && maPalette.IsGreyPalette8Bit())
        return true;

    OpenGLZone aZone;
    rtl::Reference<OpenGLContext> xContext = OpenGLContext::getVCLContext();
    xContext->state().scissor().disable();
    xContext->state().stencil().disable();

    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;

    GetTexture();
    pProgram = xContext->UseProgram("textureVertexShader", "greyscaleFragmentShader");

    if (!pProgram)
        return false;

    OpenGLTexture aNewTex(mnWidth, mnHeight);
    pFramebuffer = xContext->AcquireFramebuffer(aNewTex);
    pProgram->SetTexture("sampler", maTexture);
    pProgram->DrawTexture(maTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer( pFramebuffer );
    maTexture = aNewTex;
    mnBits = 8;
    maPalette = Bitmap::GetGreyPalette(256);

    // AllocateUserData will handle the rest.
    DeallocateUserData();
    mbDirtyTexture = false;

    CHECK_GL_ERROR();
    return true;
}

// This is needed to just make the bitmap usable as an alpha channel.
// Converting to 8bit grey will do.
bool OpenGLSalBitmap::InterpretAs8Bit()
{
    return ConvertToGreyscale();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
