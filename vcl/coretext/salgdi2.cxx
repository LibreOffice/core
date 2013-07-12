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

#include "sal/config.h"

#include "osl/file.hxx"
#include "osl/process.h"

#include "osl/mutex.hxx"

#include "rtl/bootstrap.h"
#include "rtl/strbuf.hxx"

#include "basegfx/range/b2drectangle.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/matrix/b2dhommatrixtools.hxx"

#include "vcl/sysdata.hxx"
#include "vcl/svapp.hxx"

#include "coretext/salgdi2.h"

#ifdef MACOSX
#include "aqua/salframe.h"
#endif

#ifdef IOS
#include "saldatabasic.hxx"
#include <basebmp/scanlineformats.hxx>
#endif

#include "ctfonts.hxx"

#include "fontsubset.hxx"
#include "impfont.hxx"
#include "sallayout.hxx"
#include "sft.hxx"


using namespace vcl;

// =======================================================================

SystemFontList::~SystemFontList( void )
{}


// =======================================================================

ImplMacTextStyle::ImplMacTextStyle( const FontSelectPattern& rReqFont )
:   mpFontData( (ImplMacFontData*)rReqFont.mpFontData )
,   mfFontStretch( 1.0 )
,   mfFontRotation( 0.0 )
{}

// -----------------------------------------------------------------------

ImplMacTextStyle::~ImplMacTextStyle( void )
{}

// =======================================================================

ImplMacFontData::ImplMacFontData( const ImplMacFontData& rSrc )
:   PhysicalFontFace( rSrc )
,   mnFontId( rSrc.mnFontId )
,   mpCharMap( rSrc.mpCharMap )
,   mbOs2Read( rSrc.mbOs2Read )
,   mbHasOs2Table( rSrc.mbHasOs2Table )
,   mbCmapEncodingRead( rSrc.mbCmapEncodingRead )
{
    if( mpCharMap )
        mpCharMap->AddReference();
}

// -----------------------------------------------------------------------

ImplMacFontData::ImplMacFontData( const ImplDevFontAttributes& rDFA, sal_IntPtr nFontId )
:   PhysicalFontFace( rDFA, 0 )
,   mnFontId( nFontId )
,   mpCharMap( NULL )
,   mbOs2Read( false )
,   mbHasOs2Table( false )
,   mbCmapEncodingRead( false )
,   mbFontCapabilitiesRead( false )
{}

// -----------------------------------------------------------------------

ImplMacFontData::~ImplMacFontData()
{
    if( mpCharMap )
        mpCharMap->DeReference();
}

// -----------------------------------------------------------------------

sal_IntPtr ImplMacFontData::GetFontId() const
{
    return (sal_IntPtr)mnFontId;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplMacFontData::CreateFontInstance(FontSelectPattern& rFSD) const
{
    return new ImplFontEntry(rFSD);
}

// -----------------------------------------------------------------------

static unsigned GetUShort( const unsigned char* p ){return((p[0]<<8)+p[1]);}

const ImplFontCharMap* ImplMacFontData::GetImplFontCharMap() const
{
    // return the cached charmap
    if( mpCharMap )
        return mpCharMap;

    // set the default charmap
    mpCharMap = ImplFontCharMap::GetDefaultMap();
    mpCharMap->AddReference();

    // get the CMAP byte size
    // allocate a buffer for the CMAP raw data
    const int nBufSize = GetFontTable( "cmap", NULL );
    DBG_ASSERT( (nBufSize > 0), "ImplMacFontData::GetImplFontCharMap : GetFontTable1 failed!\n");
    if( nBufSize <= 0 )
        return mpCharMap;

    // get the CMAP raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    DBG_ASSERT( (nRawLength > 0), "ImplMacFontData::GetImplFontCharMap : GetFontTable2 failed!\n");
    if( nRawLength <= 0 )
        return mpCharMap;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::GetImplFontCharMap : ByteCount mismatch!\n");

    // parse the CMAP
    CmapResult aCmapResult;
    if( ParseCMAP( &aBuffer[0], nRawLength, aCmapResult ) )
    {
        // create the matching charmap
        mpCharMap->DeReference();
        mpCharMap = new ImplFontCharMap( aCmapResult );
        mpCharMap->AddReference();
    }

    return mpCharMap;
}

bool ImplMacFontData::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
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
    nBufSize = GetFontTable( "GSUB", NULL );
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
    nBufSize = GetFontTable( "OS/2", NULL );
    if( nBufSize > 0 )
    {
        // allocate a buffer for the OS/2 raw data
        ByteVector aBuffer( nBufSize );
        // get the OS/2 raw data
        const int nRawLength = GetFontTable( "OS/2", &aBuffer[0] );
        if( nRawLength > 0 )
        {
            const unsigned char* pOS2Table = &aBuffer[0];
            vcl::getTTCoverage(
                maFontCapabilities.maUnicodeRange,
                maFontCapabilities.maCodePageRange,
                pOS2Table, nRawLength);
        }
    }
    rFontCapabilities = maFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}

// -----------------------------------------------------------------------

void ImplMacFontData::ReadOs2Table( void ) const
{
    // read this only once per font
    if( mbOs2Read )
        return;
    mbOs2Read = true;
    mbHasOs2Table = false;

    // prepare to get the OS/2 table raw data
    const int nBufSize = GetFontTable( "OS/2", NULL );
    DBG_ASSERT( (nBufSize > 0), "ImplMacFontData::ReadOs2Table : GetFontTable1 failed!\n");
    if( nBufSize <= 0 )
        return;

    // get the OS/2 raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    DBG_ASSERT( (nRawLength > 0), "ImplMacFontData::ReadOs2Table : GetFontTable2 failed!\n");
    if( nRawLength <= 0 )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::ReadOs2Table : ByteCount mismatch!\n");
    mbHasOs2Table = true;

    // parse the OS/2 raw data
    // TODO: also analyze panose info, etc.
}

void ImplMacFontData::ReadMacCmapEncoding( void ) const
{
    // read this only once per font
    if( mbCmapEncodingRead )
        return;
    mbCmapEncodingRead = true;

    const int nBufSize = GetFontTable( "cmap", NULL );
    if( nBufSize <= 0 )
        return;

    // get the CMAP raw data
    ByteVector aBuffer( nBufSize );
    const int nRawLength = GetFontTable( "cmap", &aBuffer[0] );
    if( nRawLength < 24 )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::ReadMacCmapEncoding : ByteCount mismatch!\n");

    const unsigned char* pCmap = &aBuffer[0];
    if( GetUShort( pCmap ) != 0x0000 )
        return;
}

// -----------------------------------------------------------------------

AquaSalGraphics::AquaSalGraphics()
#ifdef MACOSX
    : mpFrame( NULL )
    , mxLayer( NULL )
    , mrContext( NULL )
    , mpXorEmulation( NULL )
    , mnXorMode( 0 )
    , mnWidth( 0 )
    , mnHeight( 0 )
    , mnBitmapDepth( 0 )
    , mnRealDPIX( 0 )
    , mnRealDPIY( 0 )
    , mfFakeDPIScale( 1.0 )
    , mxClipPath( NULL )
    , maLineColor( COL_WHITE )
    , maFillColor( COL_BLACK )
    , mpMacFontData( NULL )
    , mpMacTextStyle( NULL )
    , maTextColor( COL_BLACK )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
    , mbWindow( false )
#else
    : mrContext( NULL )
    , mfFakeDPIScale( 1.0 )
    , mpMacFontData( NULL )
    , mpMacTextStyle( NULL )
    , maTextColor( COL_BLACK )
    , mbNonAntialiasedText( false )
#endif
{}

// -----------------------------------------------------------------------

AquaSalGraphics::~AquaSalGraphics()
{
#ifdef MAXOSX
    CGPathRelease( mxClipPath );
    delete mpMacTextStyle;

    if( mpXorEmulation )
        delete mpXorEmulation;

    if( mxLayer )
        CGLayerRelease( mxLayer );
    else if( mrContext && mbWindow )
    {
        // destroy backbuffer bitmap context that we created ourself
        CGContextRelease( mrContext );
        mrContext = NULL;
        // memory is freed automatically by maOwnContextMemory
    }
#endif
}

// =======================================================================

void AquaSalGraphics::SetTextColor( SalColor nSalColor )
{
    maTextColor = RGBAColor( nSalColor );
    if( mpMacTextStyle)
        mpMacTextStyle->SetTextColor( maTextColor );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int /*nFallbackLevel*/ )
{
    mpMacTextStyle->GetFontMetric( mfFakeDPIScale, *pMetric );
}

// -----------------------------------------------------------------------

sal_uLong AquaSalGraphics::GetKernPairs( sal_uLong, ImplKernPairData* )
{
    return 0;
}

// -----------------------------------------------------------------------

static bool AddTempFontDir( const char* pDir )
{
    CFStringRef rDir = CFStringCreateWithCString(NULL, pDir, kCFStringEncodingUTF8);
    CFURLRef rDirURL = CFURLCreateWithFileSystemPath(NULL, rDir, kCFURLPOSIXPathStyle, true);

    CFErrorRef error;
    bool success = CTFontManagerRegisterFontsForURL(rDirURL, kCTFontManagerScopeProcess, &error);

    if (!success)
    {
        CFRelease(error);
        return false;
    }

    return true;
}

static bool AddLocalTempFontDirs( void )
{
    static bool bFirst = true;
    if( !bFirst )
        return false;
    bFirst = false;

    // add private font files

    OUString aBrandStr( "$BRAND_BASE_DIR" );
    rtl_bootstrap_expandMacros( &aBrandStr.pData );
    OUString aBrandSysPath;
    OSL_VERIFY( osl_getSystemPathFromFileURL( aBrandStr.pData, &aBrandSysPath.pData ) == osl_File_E_None );

    OStringBuffer aBrandFontDir( aBrandSysPath.getLength()*2 );
    aBrandFontDir.append( OUStringToOString( aBrandSysPath, RTL_TEXTENCODING_UTF8 ) );
    aBrandFontDir.append( "/share/fonts/truetype/" );
    return AddTempFontDir( aBrandFontDir.getStr() );
}

void AquaSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( pFontList, "AquaSalGraphics::GetDevFontList(NULL) !");

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
    pSalData->mpFontList->AnnounceFonts( *pFontList );
}

void AquaSalGraphics::ClearDevFontCache()
{
    SalData* pSalData = GetSalData();
    delete pSalData->mpFontList;
    pSalData->mpFontList = NULL;
}

// -----------------------------------------------------------------------

bool AquaSalGraphics::AddTempDevFont( ImplDevFontList*,
    const OUString& rFontFileURL, const OUString& /*rFontName*/ )
{
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );
    OString aCFileName = OUStringToOString( aUSytemPath, RTL_TEXTENCODING_UTF8 );

    CFStringRef rDir = CFStringCreateWithCString(NULL, aCFileName.getStr(), kCFStringEncodingUTF8);
    CFURLRef rDirURL = CFURLCreateWithFileSystemPath(NULL, rDir, kCFURLPOSIXPathStyle, true);

    CFErrorRef error;
    bool success = CTFontManagerRegisterFontsForURL(rDirURL, kCTFontManagerScopeProcess, &error);

    if (!success)
    {
        CFRelease(error);
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::GetGlyphOutline( sal_GlyphId nGlyphId, basegfx::B2DPolyPolygon& rPolyPoly )
{
    const bool bRC = mpMacTextStyle->GetGlyphOutline( nGlyphId, rPolyPoly );
    return bRC;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::GetGlyphBoundRect( sal_GlyphId nGlyphId, Rectangle& rRect )
{
    const bool bRC = mpMacTextStyle->GetGlyphBoundRect( nGlyphId, rRect );
    return bRC;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // nothing to do since there are no device-specific fonts on Aqua
}

// -----------------------------------------------------------------------

void AquaSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

// -----------------------------------------------------------------------

sal_uInt16 AquaSalGraphics::SetFont( FontSelectPattern* pReqFont, int /*nFallbackLevel*/ )
{
    // release the text style
    delete mpMacTextStyle;
    mpMacTextStyle = NULL;

    // handle NULL request meaning: release-font-resources request
    if( !pReqFont )
    {
        mpMacFontData = NULL;
        return 0;
    }

    // update the text style
    mpMacFontData = static_cast<const ImplMacFontData*>( pReqFont->mpFontData );
    mpMacTextStyle = mpMacFontData->CreateMacTextStyle( *pReqFont );
    mpMacTextStyle->SetTextColor( maTextColor );

    SAL_INFO("vcl.coretext",
            "SetFont"
            << " to "     << mpMacFontData->GetFamilyName()
            << ", "       << mpMacFontData->GetStyleName()
            << " fontid=" << mpMacFontData->GetFontId()
            << " for "    << pReqFont->GetFamilyName()
            << ", "       << pReqFont->GetStyleName()
            << " weight=" << pReqFont->GetWeight()
            << " slant="  << pReqFont->GetSlant()
            << " size="   << pReqFont->mnHeight << "x" << pReqFont->mnWidth
            << " orientation=" << pReqFont->mnOrientation
            );

    return 0;
}

// -----------------------------------------------------------------------

SalLayout* AquaSalGraphics::GetTextLayout( ImplLayoutArgs& /*rArgs*/, int /*nFallbackLevel*/ )
{
    SalLayout* pSalLayout = mpMacTextStyle->GetTextLayout();
    return pSalLayout;
}

// -----------------------------------------------------------------------

const ImplFontCharMap* AquaSalGraphics::GetImplFontCharMap() const
{
    if( !mpMacFontData )
        return ImplFontCharMap::GetDefaultMap();

    return mpMacFontData->GetImplFontCharMap();
}

bool AquaSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !mpMacFontData )
        return false;

    return mpMacFontData->GetImplFontCapabilities(rFontCapabilities);
}

// -----------------------------------------------------------------------

// fake a SFNT font directory entry for a font table
// see http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6.html#Directory
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
    const ImplMacFontData* pMacFont = static_cast<const ImplMacFontData*>(pFontData);

    // short circuit for CFF-only fonts
    const int nCffSize = pMacFont->GetFontTable( "CFF ", NULL);
    if( pJustCFF != NULL )
    {
        *pJustCFF = (nCffSize > 0);
        if( *pJustCFF)
        {
            rBuffer.resize( nCffSize);
            const int nCffRead = pMacFont->GetFontTable( "CFF ", &rBuffer[0]);
            if( nCffRead != nCffSize)
                return false;
            return true;
        }
    }

    // get font table availability and size in bytes
    const int nHeadSize = pMacFont->GetFontTable( "head", NULL);
    if( nHeadSize <= 0)
        return false;
    const int nMaxpSize = pMacFont->GetFontTable( "maxp", NULL);
    if( nMaxpSize <= 0)
        return false;
    const int nCmapSize = pMacFont->GetFontTable( "cmap", NULL);
    if( nCmapSize <= 0)
        return false;
    const int nNameSize = pMacFont->GetFontTable( "name", NULL);
    if( nNameSize <= 0)
        return false;
    const int nHheaSize = pMacFont->GetFontTable( "hhea", NULL);
    if( nHheaSize <= 0)
        return false;
    const int nHmtxSize = pMacFont->GetFontTable( "hmtx", NULL);
    if( nHmtxSize <= 0)
        return false;

    // get the ttf-glyf outline tables
    int nLocaSize = 0;
    int nGlyfSize = 0;
    if( nCffSize <= 0)
    {
        nLocaSize = pMacFont->GetFontTable( "loca", NULL);
        if( nLocaSize <= 0)
            return false;
        nGlyfSize = pMacFont->GetFontTable( "glyf", NULL);
        if( nGlyfSize <= 0)
            return false;
    }

    int nPrepSize = 0, nCvtSize = 0, nFpgmSize = 0;
    if( nGlyfSize) // TODO: reduce PDF size by making hint subsetting optional
    {
        nPrepSize = pMacFont->GetFontTable( "prep", NULL);
        nCvtSize  = pMacFont->GetFontTable( "cvt ", NULL);
        nFpgmSize = pMacFont->GetFontTable( "fpgm", NULL);
    }

    // prepare a byte buffer for a fake font
    int nTableCount = 7;
    nTableCount += (nPrepSize>0) + (nCvtSize>0) + (nFpgmSize>0) + (nGlyfSize>0);
    const ByteCount nFdirSize = 12 + 16*nTableCount;
    ByteCount nTotalSize = nFdirSize;
    nTotalSize += nHeadSize + nMaxpSize + nNameSize + nCmapSize;
    if( nGlyfSize )
        nTotalSize += nLocaSize + nGlyfSize;
    else
        nTotalSize += nCffSize;
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
    if( nCvtSize ) {
        if( nCvtSize != pMacFont->GetFontTable( "cvt ", &rBuffer[nOfs]))
            return false;
        FakeDirEntry( "cvt ", nOfs, nCvtSize, &rBuffer[0], pFakeEntry );
        nOfs += nCvtSize;
    }
    if( nFpgmSize ) {
        if( nFpgmSize != pMacFont->GetFontTable( "fpgm", &rBuffer[nOfs]))
            return false;
        FakeDirEntry( "fpgm", nOfs, nFpgmSize, &rBuffer[0], pFakeEntry );
        nOfs += nFpgmSize;
    }
    if( nCffSize ) {
        if( nCffSize != pMacFont->GetFontTable( "CFF ", &rBuffer[nOfs]))
            return false;
        FakeDirEntry( "CFF ", nOfs, nCffSize, &rBuffer[0], pFakeEntry );
        nOfs += nGlyfSize;
    } else {
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
    if( nPrepSize ) {
        if( nPrepSize != pMacFont->GetFontTable( "prep", &rBuffer[nOfs]))
            return false;
        FakeDirEntry( "prep", nOfs, nPrepSize, &rBuffer[0], pFakeEntry );
        nOfs += nPrepSize;
    }

    DBG_ASSERT( (nOfs==nTotalSize), "AquaSalGraphics::CreateFontSubset (nOfs!=nTotalSize)");

    return true;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFontData, bool bVertical,
    Int32Vector& rGlyphWidths, Ucs2UIntMap& rUnicodeEnc )
{
    rGlyphWidths.clear();
    rUnicodeEnc.clear();

    if( pFontData->IsSubsettable() )
    {
        ByteVector aBuffer;
        if( !GetRawFontData( pFontData, aBuffer, NULL ) )
            return;

        // TODO: modernize psprint's horrible fontsubset C-API
        // this probably only makes sense after the switch to another SCM
        // that can preserve change history after file renames

        // use the font subsetter to get the widths
        TrueTypeFont* pSftFont = NULL;
        int nRC = ::OpenTTFontBuffer( (void*)&aBuffer[0], aBuffer.size(), 0, &pSftFont);
        if( nRC != SF_OK )
            return;

        const int nGlyphCount = ::GetTTGlyphCount( pSftFont );
        if( nGlyphCount > 0 )
        {
            // get glyph metrics
            rGlyphWidths.resize(nGlyphCount);
            std::vector<sal_uInt16> aGlyphIds(nGlyphCount);
            for( int i = 0; i < nGlyphCount; i++ )
                aGlyphIds[i] = static_cast<sal_uInt16>(i);
            const TTSimpleGlyphMetrics* pGlyphMetrics = ::GetTTSimpleGlyphMetrics(
                pSftFont, &aGlyphIds[0], nGlyphCount, bVertical );
            if( pGlyphMetrics )
            {
                for( int i = 0; i < nGlyphCount; ++i )
                    rGlyphWidths[i] = pGlyphMetrics[i].adv;
                free( (void*)pGlyphMetrics );
            }

            const ImplFontCharMap* pMap = mpMacFontData->GetImplFontCharMap();
            DBG_ASSERT( pMap && pMap->GetCharCount(), "no charmap" );
            pMap->AddReference(); // TODO: add and use RAII object instead

            // get unicode<->glyph encoding
            // TODO? avoid sft mapping by using the pMap itself
            int nCharCount = pMap->GetCharCount();
            sal_uInt32 nChar = pMap->GetFirstChar();
            for(; --nCharCount >= 0; nChar = pMap->GetNextChar( nChar ) )
            {
                if( nChar > 0xFFFF ) // TODO: allow UTF-32 chars
                    break;
                sal_Ucs nUcsChar = static_cast<sal_Ucs>(nChar);
                sal_uInt32 nGlyph = ::MapChar( pSftFont, nUcsChar, bVertical );
                if( nGlyph > 0 )
                    rUnicodeEnc[ nUcsChar ] = nGlyph;
            }

            pMap->DeReference(); // TODO: add and use RAII object instead
        }

        ::CloseTTFont( pSftFont );
    }
    else if( pFontData->IsEmbeddable() )
    {
        // get individual character widths
        OSL_FAIL("not implemented for non-subsettable fonts!\n");
    }
}

// -----------------------------------------------------------------------

const Ucs2SIntMap* AquaSalGraphics::GetFontEncodingVector(
    const PhysicalFontFace*, const Ucs2OStrMap** /*ppNonEncoded*/ )
{
    return NULL;
}

// -----------------------------------------------------------------------

const void* AquaSalGraphics::GetEmbedFontData( const PhysicalFontFace*,
                              const sal_Ucs* /*pUnicodes*/,
                              sal_Int32* /*pWidths*/,
                              FontSubsetInfo&,
                              long* /*pDataLen*/ )
{
    return NULL;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::FreeEmbedFontData( const void* pData, long /*nDataLen*/ )
{
    // TODO: implementing this only makes sense when the implementation of
    //      AquaSalGraphics::GetEmbedFontData() returns non-NULL
    (void)pData;
    DBG_ASSERT( (pData!=NULL), "AquaSalGraphics::FreeEmbedFontData() is not implemented\n");
}

// -----------------------------------------------------------------------

SystemFontData AquaSalGraphics::GetSysFontData( int /* nFallbacklevel */ ) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );

    aSysFontData.bAntialias = !mbNonAntialiasedText;

    return aSysFontData;
}

#ifdef IOS

// Note that "SvpSalGraphics" is actually called AquaSalGraphics for iOS

bool SvpSalGraphics::CheckContext()
{
    const basegfx::B2IVector size = m_aDevice->getSize();
    const basegfx::B2IVector bufferSize = m_aDevice->getBufferSize();
    const sal_Int32 scanlineStride = m_aDevice->getScanlineStride();
    basebmp::RawMemorySharedArray pixelBuffer = m_aDevice->getBuffer();

    SAL_INFO( "vcl.ios",
              "CheckContext: device=" << m_aDevice.get() <<
              " size=" << size.getX() << "x" << size.getY() <<
              (m_aDevice->isTopDown() ? " top-down" : " bottom-up") <<
              " stride=" << scanlineStride <<
              " bufferSize=(" << bufferSize.getX() << "," << bufferSize.getY() << ")" );

    switch( m_aDevice->getScanlineFormat() ) {
    case basebmp::FORMAT_EIGHT_BIT_PAL:
        mrContext = CGBitmapContextCreate(pixelBuffer.get(),
                                          bufferSize.getX(), bufferSize.getY(),
                                          8, scanlineStride,
                                          CGColorSpaceCreateDeviceGray(),
                                          kCGImageAlphaNone);
        break;
    case basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
        mrContext = CGBitmapContextCreate(pixelBuffer.get(),
                                          bufferSize.getX(), bufferSize.getY(),
                                          8, scanlineStride,
                                          CGColorSpaceCreateDeviceRGB(),
                                          kCGImageAlphaNoneSkipLast);
        break;
    default:
        SAL_INFO( "vcl.ios", "CheckContext: unsupported color format " << basebmp::formatName( m_aDevice->getScanlineFormat() ) );
    }

    SAL_WARN_IF( mrContext == NULL, "vcl.ios", "CheckContext() failed" );

    // Should we also clip the context? (Then we need to add a
    // getBounds() function to BitmapDevice.)

    if( mrContext != NULL && m_aDevice->isTopDown() )
    {
        CGContextTranslateCTM( mrContext, 0, bufferSize.getY() );
        CGContextScaleCTM( mrContext, 1, -1 );
    }

    SAL_INFO( "vcl.ios", "CheckContext: context=" << mrContext );

    return ( mrContext != NULL );
}

CGContextRef SvpSalGraphics::GetContext()
{
    if ( !mrContext )
        CheckContext();

    return mrContext;
}

#endif
 
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
