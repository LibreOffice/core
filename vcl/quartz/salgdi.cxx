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
#include <config_folders.h>

#include <basebmp/scanlineformats.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.h>
#include <rtl/strbuf.hxx>

#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#include "ctfonts.hxx"
#include "fontsubset.hxx"
#include "impfont.hxx"
#ifdef MACOSX
#include "osx/salframe.h"
#endif
#include "quartz/salgdi.h"
#include "quartz/utils.h"
#ifdef IOS
#include "saldatabasic.hxx"
#endif
#include "sallayout.hxx"
#include "sft.hxx"

using namespace vcl;

CoreTextFontData::CoreTextFontData( const CoreTextFontData& rSrc )
  : PhysicalFontFace( rSrc )
  , mnFontId( rSrc.mnFontId )
  , mbOs2Read( rSrc.mbOs2Read )
  , mbHasOs2Table( rSrc.mbHasOs2Table )
  , mbCmapEncodingRead( rSrc.mbCmapEncodingRead )
{
    if( rSrc.mpCharMap )
        mpCharMap = rSrc.mpCharMap;
}

CoreTextFontData::CoreTextFontData( const ImplDevFontAttributes& rDFA, sal_IntPtr nFontId )
  : PhysicalFontFace( rDFA, 0 )
  , mnFontId( nFontId )
  , mbOs2Read( false )
  , mbHasOs2Table( false )
  , mbCmapEncodingRead( false )
  , mbFontCapabilitiesRead( false )
{
}

CoreTextFontData::~CoreTextFontData()
{
    if( mpCharMap )
    {
        mpCharMap = nullptr;
    }
}

sal_IntPtr CoreTextFontData::GetFontId() const
{
    return (sal_IntPtr)mnFontId;
}

static unsigned GetUShort( const unsigned char* p ){return((p[0]<<8)+p[1]);}

const FontCharMapPtr CoreTextFontData::GetFontCharMap() const
{
    // return the cached charmap
    if( mpCharMap )
        return mpCharMap;

    // set the default charmap
    FontCharMapPtr pCharMap( new FontCharMap() );
    mpCharMap = pCharMap;

    // get the CMAP byte size
    // allocate a buffer for the CMAP raw data
    const int nBufSize = GetFontTable( "cmap", nullptr );
    DBG_ASSERT( (nBufSize > 0), "CoreTextFontData::GetFontCharMap : GetFontTable1 failed!\n");
    if( nBufSize <= 0 )
        return mpCharMap;

    // get the CMAP raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    DBG_ASSERT( (nRawLength > 0), "CoreTextFontData::GetFontCharMap : GetFontTable2 failed!\n");
    if( nRawLength <= 0 )
        return mpCharMap;

    DBG_ASSERT( (nBufSize==nRawLength), "CoreTextFontData::GetFontCharMap : ByteCount mismatch!\n");

    // parse the CMAP
    CmapResult aCmapResult;
    if( ParseCMAP( &aBuffer[0], nRawLength, aCmapResult ) )
    {
        FontCharMapPtr pDefFontCharMap( new FontCharMap(aCmapResult) );
        // create the matching charmap
        mpCharMap = pDefFontCharMap;
    }

    return mpCharMap;
}

bool CoreTextFontData::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    // read this only once per font
    if( mbFontCapabilitiesRead )
    {
        rFontCapabilities = maFontCapabilities;
        return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
    }
    mbFontCapabilitiesRead = true;

    int nBufSize = 0;
    // prepare to get the GSUB table raw data
    nBufSize = GetFontTable( "GSUB", nullptr );
    if( nBufSize > 0 )
    {
        // allocate a buffer for the GSUB raw data
        ByteVector aBuffer( nBufSize );
        // get the GSUB raw data
        const int nRawLength = GetFontTable( "GSUB", &aBuffer[0] );
        if( nRawLength > 0 )
        {
            const unsigned char* pGSUBTable = &aBuffer[0];
            vcl::getTTScripts(maFontCapabilities.maGSUBScriptTags, pGSUBTable, nRawLength);
        }
    }
    nBufSize = GetFontTable( "OS/2", nullptr );
    if( nBufSize > 0 )
    {
        // allocate a buffer for the OS/2 raw data
        ByteVector aBuffer( nBufSize );
        // get the OS/2 raw data
        const int nRawLength = GetFontTable( "OS/2", &aBuffer[0] );
        if( nRawLength > 0 )
        {
            const unsigned char* pOS2Table = &aBuffer[0];
            vcl::getTTCoverage( maFontCapabilities.maUnicodeRange,
                                maFontCapabilities.maCodePageRange,
                                pOS2Table, nRawLength);
        }
    }
    rFontCapabilities = maFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}

void CoreTextFontData::ReadOs2Table() const
{
    // read this only once per font
    if( mbOs2Read )
        return;

    mbOs2Read = true;
    mbHasOs2Table = false;

    // prepare to get the OS/2 table raw data
    const int nBufSize = GetFontTable( "OS/2", nullptr );
    DBG_ASSERT( (nBufSize > 0), "CoreTextFontData::ReadOs2Table : GetFontTable1 failed!\n");
    if( nBufSize <= 0 )
        return;

    // get the OS/2 raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    DBG_ASSERT( (nRawLength > 0), "CoreTextFontData::ReadOs2Table : GetFontTable2 failed!\n");
    if( nRawLength <= 0 )
        return;

    DBG_ASSERT( (nBufSize==nRawLength), "CoreTextFontData::ReadOs2Table : ByteCount mismatch!\n");
    mbHasOs2Table = true;

    // parse the OS/2 raw data
    // TODO: also analyze panose info, etc.
}

void CoreTextFontData::ReadMacCmapEncoding() const
{
    // read this only once per font
    if( mbCmapEncodingRead )
        return;

    mbCmapEncodingRead = true;

    const int nBufSize = GetFontTable( "cmap", nullptr );
    if( nBufSize <= 0 )
        return;

    // get the CMAP raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    if( nRawLength < 24 )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "CoreTextFontData::ReadMacCmapEncoding : ByteCount mismatch!\n");

    const unsigned char* pCmap = &aBuffer[0];
    if( GetUShort( pCmap ) != 0x0000 )
        return;
}

AquaSalGraphics::AquaSalGraphics()
    : mxLayer( nullptr )
    , mrContext( nullptr )
#ifdef MACOSX
    , mpFrame( nullptr )
#endif
#if OSL_DEBUG_LEVEL > 0
    , mnContextStackDepth( 0 )
#endif
    , mpXorEmulation( nullptr )
    , mnXorMode( 0 )
    , mnWidth( 0 )
    , mnHeight( 0 )
    , mnBitmapDepth( 0 )
    , mnRealDPIX( 0 )
    , mnRealDPIY( 0 )
    , mxClipPath( nullptr )
    , maLineColor( COL_WHITE )
    , maFillColor( COL_BLACK )
    , mpFontData( nullptr )
    , mpTextStyle( nullptr )
    , maTextColor( COL_BLACK )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
#ifdef MACOSX
    , mbWindow( false )
#else
    , mbForeignContext( false )
#endif
{
    SAL_INFO( "vcl.quartz", "AquaSalGraphics::AquaSalGraphics() this=" << this );
}

AquaSalGraphics::~AquaSalGraphics()
{
    SAL_INFO( "vcl.quartz", "AquaSalGraphics::~AquaSalGraphics() this=" << this );

    if( mxClipPath )
    {
        SAL_INFO("vcl.cg", "CGPathRelease(" << mxClipPath << ")" );
        CGPathRelease( mxClipPath );
    }

    delete mpTextStyle;

    if( mpXorEmulation )
        delete mpXorEmulation;

#ifdef IOS
    if (mbForeignContext)
        return;
#endif
    if( mxLayer )
    {
        SAL_INFO("vcl.cg", "CGLayerRelease(" << mxLayer << ")" );
        CGLayerRelease( mxLayer );
    }
    else if( mrContext
#ifdef MACOSX
             && mbWindow
#endif
             )
    {
        // destroy backbuffer bitmap context that we created ourself
        SAL_INFO("vcl.cg", "CGContextRelease(" << mrContext << ")" );
        CGContextRelease( mrContext );
        mrContext = nullptr;
    }
}

SalGraphicsImpl* AquaSalGraphics::GetImpl() const
{
    return nullptr;
}

void AquaSalGraphics::SetTextColor( SalColor nSalColor )
{
    maTextColor = RGBAColor( nSalColor );
    // SAL_ DEBUG(std::hex << nSalColor << std::dec << "={" << maTextColor.GetRed() << ", " << maTextColor.GetGreen() << ", " << maTextColor.GetBlue() << ", " << maTextColor.GetAlpha() << "}");
}

void AquaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int /*nFallbackLevel*/ )
{
    mpTextStyle->GetFontMetric( *pMetric );
}

static bool AddTempDevFont(const OUString& rFontFileURL)
{
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );
    OString aCFileName = OUStringToOString( aUSytemPath, RTL_TEXTENCODING_UTF8 );

    CFStringRef rFontPath = CFStringCreateWithCString(nullptr, aCFileName.getStr(), kCFStringEncodingUTF8);
    CFURLRef rFontURL = CFURLCreateWithFileSystemPath(nullptr, rFontPath, kCFURLPOSIXPathStyle, true);

    CFErrorRef error;
    bool success = CTFontManagerRegisterFontsForURL(rFontURL, kCTFontManagerScopeProcess, &error);
    if (!success)
    {
        CFRelease(error);
    }

    return success;
}

static void AddTempFontDir( const OUString &rFontDirUrl )
{
    osl::Directory aFontDir( rFontDirUrl );
    osl::FileBase::RC rcOSL = aFontDir.open();
    if( rcOSL == osl::FileBase::E_None )
    {
        osl::DirectoryItem aDirItem;

        while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
        {
            osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
            rcOSL = aDirItem.getFileStatus( aFileStatus );
            if ( rcOSL == osl::FileBase::E_None )
            {
                AddTempDevFont(aFileStatus.getFileURL());
            }
        }
    }
}

static void AddLocalTempFontDirs()
{
    static bool bFirst = true;
    if( !bFirst )
        return;

    bFirst = false;

    // add private font files

    OUString aBrandStr( "$BRAND_BASE_DIR" );
    rtl_bootstrap_expandMacros( &aBrandStr.pData );
    AddTempFontDir( aBrandStr + "/" LIBO_SHARE_FOLDER "/fonts/truetype/" );
}

void AquaSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    DBG_ASSERT( pFontCollection, "AquaSalGraphics::GetDevFontList(NULL) !");

    AddLocalTempFontDirs();

    // The idea is to cache the list of system fonts once it has been generated.
    // SalData seems to be a good place for this caching. However we have to
    // carefully make the access to the font list thread-safe. If we register
    // a font-change event handler to update the font list in case fonts have
    // changed on the system we have to lock access to the list. The right
    // way to do that is the solar mutex since GetDevFontList is protected
    // through it as should be all event handlers

    SalData* pSalData = GetSalData();
    if( !pSalData->mpFontList )
        pSalData->mpFontList = GetCoretextFontList();

    // Copy all PhysicalFontFace objects contained in the SystemFontList
    pSalData->mpFontList->AnnounceFonts( *pFontCollection );
}

void AquaSalGraphics::ClearDevFontCache()
{
    SalData* pSalData = GetSalData();
    delete pSalData->mpFontList;
    pSalData->mpFontList = nullptr;
}

bool AquaSalGraphics::AddTempDevFont( PhysicalFontCollection*,
    const OUString& rFontFileURL, const OUString& /*rFontName*/ )
{
    return ::AddTempDevFont(rFontFileURL);
}

bool AquaSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId, basegfx::B2DPolyPolygon& rPolyPoly )
{
    const bool bRC = mpTextStyle->GetGlyphOutline( aGlyphId, rPolyPoly );
    return bRC;
}

bool AquaSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    const bool bRC = mpTextStyle->GetGlyphBoundRect( aGlyphId, rRect );
    return bRC;
}

void AquaSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

sal_uInt16 AquaSalGraphics::SetFont( FontSelectPattern* pReqFont, int /*nFallbackLevel*/ )
{
    // release the text style
    delete mpTextStyle;
    mpTextStyle = nullptr;

    // handle NULL request meaning: release-font-resources request
    if( !pReqFont )
    {
        mpFontData = nullptr;
        return 0;
    }

    // update the text style
    mpFontData = static_cast<const CoreTextFontData*>( pReqFont->mpFontData );
    mpTextStyle = new CoreTextStyle( *pReqFont );

    SAL_INFO("vcl.ct",
            "SetFont"
               " to "     << mpFontData->GetFamilyName()
            << ", "       << mpFontData->GetStyleName()
            << " fontid=" << mpFontData->GetFontId()
            << " for "    << pReqFont->GetFamilyName()
            << ", "       << pReqFont->GetStyleName()
            << " weight=" << pReqFont->GetWeight()
            << " slant="  << pReqFont->GetSlant()
            << " size="   << pReqFont->mnHeight << "x" << pReqFont->mnWidth
            << " orientation=" << pReqFont->mnOrientation
            );

    return 0;
}

SalLayout* AquaSalGraphics::GetTextLayout( ImplLayoutArgs& /*rArgs*/, int /*nFallbackLevel*/ )
{
    SalLayout* pSalLayout = mpTextStyle->GetTextLayout();
    return pSalLayout;
}

const FontCharMapPtr AquaSalGraphics::GetFontCharMap() const
{
    if( !mpFontData )
    {
        FontCharMapPtr pFontCharMap( new FontCharMap() );
        return pFontCharMap;
    }

    return mpFontData->GetFontCharMap();
}

bool AquaSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !mpFontData )
        return false;

    return mpFontData->GetFontCapabilities(rFontCapabilities);
}

// fake a SFNT font directory entry for a font table
// see http://developer.apple.com/fonts/TTRefMan/RM06/Chap6.html#Directory
static void FakeDirEntry( const char aTag[5], ByteCount nOfs, ByteCount nLen,
                          const unsigned char* /*pData*/, unsigned char*& rpDest )
{
    // write entry tag
    rpDest[ 0] = aTag[0];
    rpDest[ 1] = aTag[1];
    rpDest[ 2] = aTag[2];
    rpDest[ 3] = aTag[3];
    // TODO: get entry checksum and write it
    //      not too important since the subsetter doesn't care currently
    //      for( pData+nOfs ... pData+nOfs+nLen )
    // write entry offset
    rpDest[ 8] = (char)(nOfs >> 24);
    rpDest[ 9] = (char)(nOfs >> 16);
    rpDest[10] = (char)(nOfs >>  8);
    rpDest[11] = (char)(nOfs >>  0);
    // write entry length
    rpDest[12] = (char)(nLen >> 24);
    rpDest[13] = (char)(nLen >> 16);
    rpDest[14] = (char)(nLen >>  8);
    rpDest[15] = (char)(nLen >>  0);
    // advance to next entry
    rpDest += 16;
}

// fake a TTF or CFF font as directly accessing font file is not possible
// when only the fontid is known. This approach also handles *.dfont fonts.
bool AquaSalGraphics::GetRawFontData( const PhysicalFontFace* pFontData,
                                      ByteVector& rBuffer, bool* pJustCFF )
{
    const CoreTextFontData* pMacFont = static_cast<const CoreTextFontData*>(pFontData);

    // short circuit for CFF-only fonts
    const int nCffSize = pMacFont->GetFontTable( "CFF ", nullptr);
    if( pJustCFF != nullptr )
    {
        *pJustCFF = (nCffSize > 0);
        if( *pJustCFF)
        {
            rBuffer.resize( nCffSize);
            const int nCffRead = pMacFont->GetFontTable( "CFF ", &rBuffer[0]);
            if( nCffRead != nCffSize)
            {
                return false;
            }
            return true;
        }
    }

    // get font table availability and size in bytes
    const int nHeadSize = pMacFont->GetFontTable( "head", nullptr);
    if( nHeadSize <= 0)
        return false;

    const int nMaxpSize = pMacFont->GetFontTable( "maxp", nullptr);
    if( nMaxpSize <= 0)
        return false;

    const int nCmapSize = pMacFont->GetFontTable( "cmap", nullptr);
    if( nCmapSize <= 0)
        return false;

    const int nNameSize = pMacFont->GetFontTable( "name", nullptr);
    if( nNameSize <= 0)
        return false;

    const int nHheaSize = pMacFont->GetFontTable( "hhea", nullptr);
    if( nHheaSize <= 0)
        return false;

    const int nHmtxSize = pMacFont->GetFontTable( "hmtx", nullptr);
    if( nHmtxSize <= 0)
        return false;

    // get the ttf-glyf outline tables
    int nLocaSize = 0;
    int nGlyfSize = 0;
    if( nCffSize <= 0)
    {
        nLocaSize = pMacFont->GetFontTable( "loca", nullptr);
        if( nLocaSize <= 0)
            return false;

        nGlyfSize = pMacFont->GetFontTable( "glyf", nullptr);
        if( nGlyfSize <= 0)
            return false;
    }

    int nPrepSize = 0, nCvtSize = 0, nFpgmSize = 0;
    if( nGlyfSize) // TODO: reduce PDF size by making hint subsetting optional
    {
        nPrepSize = pMacFont->GetFontTable( "prep", nullptr);
        nCvtSize  = pMacFont->GetFontTable( "cvt ", nullptr);
        nFpgmSize = pMacFont->GetFontTable( "fpgm", nullptr);
    }

    // prepare a byte buffer for a fake font
    int nTableCount = 7;
    nTableCount += (nPrepSize>0?1:0) + (nCvtSize>0?1:0) + (nFpgmSize>0?1:0) + (nGlyfSize>0?1:0);
    const ByteCount nFdirSize = 12 + 16*nTableCount;
    ByteCount nTotalSize = nFdirSize;
    nTotalSize += nHeadSize + nMaxpSize + nNameSize + nCmapSize;

    if( nGlyfSize )
    {
        nTotalSize += nLocaSize + nGlyfSize;
    }
    else
    {
        nTotalSize += nCffSize;
    }
    nTotalSize += nHheaSize + nHmtxSize;
    nTotalSize += nPrepSize + nCvtSize + nFpgmSize;
    rBuffer.resize( nTotalSize );

    // fake a SFNT font directory header
    if( nTableCount < 16 )
    {
        int nLog2 = 0;
        while( (nTableCount >> nLog2) > 1 ) ++nLog2;
        rBuffer[ 1] = 1;                        // Win-TTF style scaler
        rBuffer[ 5] = nTableCount;              // table count
        rBuffer[ 7] = nLog2*16;                 // searchRange
        rBuffer[ 9] = nLog2;                    // entrySelector
        rBuffer[11] = (nTableCount-nLog2)*16;   // rangeShift
    }

    // get font table raw data and update the fake directory entries
    ByteCount nOfs = nFdirSize;
    unsigned char* pFakeEntry = &rBuffer[12];
    if( nCmapSize != pMacFont->GetFontTable( "cmap", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "cmap", nOfs, nCmapSize, &rBuffer[0], pFakeEntry );
    nOfs += nCmapSize;
    if( nCvtSize )
    {
        if( nCvtSize != pMacFont->GetFontTable( "cvt ", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "cvt ", nOfs, nCvtSize, &rBuffer[0], pFakeEntry );
        nOfs += nCvtSize;
    }
    if( nFpgmSize )
    {
        if( nFpgmSize != pMacFont->GetFontTable( "fpgm", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "fpgm", nOfs, nFpgmSize, &rBuffer[0], pFakeEntry );
        nOfs += nFpgmSize;
    }
    if( nCffSize )
    {
        if( nCffSize != pMacFont->GetFontTable( "CFF ", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "CFF ", nOfs, nCffSize, &rBuffer[0], pFakeEntry );
        nOfs += nGlyfSize;
    }
    else
    {
        if( nGlyfSize != pMacFont->GetFontTable( "glyf", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "glyf", nOfs, nGlyfSize, &rBuffer[0], pFakeEntry );
        nOfs += nGlyfSize;

        if( nLocaSize != pMacFont->GetFontTable( "loca", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "loca", nOfs, nLocaSize, &rBuffer[0], pFakeEntry );
        nOfs += nLocaSize;
    }
    if( nHeadSize != pMacFont->GetFontTable( "head", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "head", nOfs, nHeadSize, &rBuffer[0], pFakeEntry );
    nOfs += nHeadSize;

    if( nHheaSize != pMacFont->GetFontTable( "hhea", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "hhea", nOfs, nHheaSize, &rBuffer[0], pFakeEntry );
    nOfs += nHheaSize;
    if( nHmtxSize != pMacFont->GetFontTable( "hmtx", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "hmtx", nOfs, nHmtxSize, &rBuffer[0], pFakeEntry );
    nOfs += nHmtxSize;
    if( nMaxpSize != pMacFont->GetFontTable( "maxp", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "maxp", nOfs, nMaxpSize, &rBuffer[0], pFakeEntry );
    nOfs += nMaxpSize;
    if( nNameSize != pMacFont->GetFontTable( "name", &rBuffer[nOfs]))
        return false;

    FakeDirEntry( "name", nOfs, nNameSize, &rBuffer[0], pFakeEntry );
    nOfs += nNameSize;
    if( nPrepSize )
    {
        if( nPrepSize != pMacFont->GetFontTable( "prep", &rBuffer[nOfs]))
            return false;

        FakeDirEntry( "prep", nOfs, nPrepSize, &rBuffer[0], pFakeEntry );
        nOfs += nPrepSize;
    }

    DBG_ASSERT( (nOfs==nTotalSize), "AquaSalGraphics::CreateFontSubset (nOfs!=nTotalSize)");

    return true;
}

void AquaSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFontData, bool bVertical,
    Int32Vector& rGlyphWidths, Ucs2UIntMap& rUnicodeEnc )
{
    rGlyphWidths.clear();
    rUnicodeEnc.clear();

    if( !pFontData->IsSubsettable() )
    {
        if( pFontData->IsEmbeddable() )
        {
            // get individual character widths
            OSL_FAIL("not implemented for non-subsettable fonts!\n");
        }
        return;
    }

    ByteVector aBuffer;
    if( !GetRawFontData( pFontData, aBuffer, nullptr ) )
        return;

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // use the font subsetter to get the widths
    TrueTypeFont* pSftFont = nullptr;
    int nRC = ::OpenTTFontBuffer( static_cast<void*>(&aBuffer[0]), aBuffer.size(), 0, &pSftFont);
    if( nRC != SF_OK )
        return;

    const int nGlyphCount = ::GetTTGlyphCount( pSftFont );
    if( nGlyphCount > 0 )
    {
        // get glyph metrics
        rGlyphWidths.resize(nGlyphCount);
        std::vector<sal_uInt16> aGlyphIds(nGlyphCount);
        for( int i = 0; i < nGlyphCount; i++ )
        {
            aGlyphIds[i] = static_cast<sal_uInt16>(i);
        }

        const TTSimpleGlyphMetrics* pGlyphMetrics = ::GetTTSimpleGlyphMetrics( pSftFont, &aGlyphIds[0],
                                                                               nGlyphCount, bVertical );
        if( pGlyphMetrics )
        {
            for( int i = 0; i < nGlyphCount; ++i )
            {
                rGlyphWidths[i] = pGlyphMetrics[i].adv;
            }
            free( const_cast<TTSimpleGlyphMetrics *>(pGlyphMetrics) );
        }

        FontCharMapPtr pMap = mpFontData->GetFontCharMap();
        DBG_ASSERT( pMap && pMap->GetCharCount(), "no charmap" );

        // get unicode<->glyph encoding
        // TODO? avoid sft mapping by using the pMap itself
        int nCharCount = pMap->GetCharCount();
        sal_uInt32 nChar = pMap->GetFirstChar();
        for( ; --nCharCount >= 0; nChar = pMap->GetNextChar( nChar ) )
        {
            if( nChar > 0xFFFF ) // TODO: allow UTF-32 chars
                break;

            sal_Ucs nUcsChar = static_cast<sal_Ucs>(nChar);
            sal_uInt32 nGlyph = ::MapChar( pSftFont, nUcsChar, bVertical );
            if( nGlyph > 0 )
            {
                rUnicodeEnc[ nUcsChar ] = nGlyph;
            }
        }

        pMap = nullptr;
    }

    ::CloseTTFont( pSftFont );
}

const Ucs2SIntMap* AquaSalGraphics::GetFontEncodingVector( const PhysicalFontFace*,
                                                           const Ucs2OStrMap** /*ppNonEncoded*/,
                                                           std::set<sal_Unicode> const** )
{
    return nullptr;
}

const void* AquaSalGraphics::GetEmbedFontData( const PhysicalFontFace*,
                                               const sal_Ucs* /*pUnicodes*/,
                                               sal_Int32* /*pWidths*/,
                                               size_t /*nLen*/,
                                               FontSubsetInfo&,
                                               long* /*pDataLen*/ )
{
    return nullptr;
}

void AquaSalGraphics::FreeEmbedFontData( const void* pData, long /*nDataLen*/ )
{
    // TODO: implementing this only makes sense when the implementation of
    //      AquaSalGraphics::GetEmbedFontData() returns non-NULL
    (void)pData;
    DBG_ASSERT( (pData!=nullptr), "AquaSalGraphics::FreeEmbedFontData() is not implemented\n");
}

bool AquaSalGraphics::IsFlipped() const
{
#ifdef MACOSX
    return mbWindow;
#else
    return false;
#endif
}

void AquaSalGraphics::RefreshRect(float lX, float lY, float lWidth, float lHeight)
{
#ifdef MACOSX
    if( ! mbWindow ) // view only on Window graphics
        return;

    if( mpFrame )
    {
        // update a little more around the designated rectangle
        // this helps with antialiased rendering
        // Rounding down x and width can accumulate a rounding error of up to 2
        // The decrementing of x, the rounding error and the antialiasing border
        // require that the width and the height need to be increased by four
        const Rectangle aVclRect(Point(static_cast<long int>(lX-1),
                                       static_cast<long int>(lY-1) ),
                                 Size(  static_cast<long int>(lWidth+4),
                                        static_cast<long int>(lHeight+4) ) );
        mpFrame->maInvalidRect.Union( aVclRect );
    }
#else
    (void) lX;
    (void) lY;
    (void) lWidth;
    (void) lHeight;
    return;
#endif
}

#ifdef IOS

bool AquaSalGraphics::CheckContext()
{
    if (mbForeignContext)
    {
        SAL_INFO("vcl.ios", "CheckContext() this=" << this << ", mbForeignContext, return true");
        return true;
    }

    SAL_INFO( "vcl.ios", "CheckContext() this=" << this << ",  not foreign, return false");
    return false;
}

CGContextRef AquaSalGraphics::GetContext()
{
    if ( !mrContext )
        CheckContext();

    return mrContext;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
