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

#include <config_folders.h>

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

#include "aqua/atsui/salgdi.h"
#include "aqua/salframe.h"
#include "aqua/atsui/salatsuifontutils.hxx"

#include "fontsubset.hxx"
#include "impfont.hxx"
#include "sallayout.hxx"
#include "sft.hxx"


using namespace vcl;

//typedef unsigned char Boolean; // copied from MacTypes.h, should be properly included
typedef std::vector<unsigned char> ByteVector;


// ATSUI is deprecated in 10.6 (or already 10.5?)
#if HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif

// =======================================================================

ImplMacFontData::ImplMacFontData( const ImplDevFontAttributes& rDFA, ATSUFontID nFontId )
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

PhysicalFontFace* ImplMacFontData::Clone() const
{
    ImplMacFontData* pClone = new ImplMacFontData(*this);
    if( mpCharMap )
        mpCharMap->AddReference();
    return pClone;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplMacFontData::CreateFontInstance(FontSelectPattern& rFSD) const
{
    return new ImplFontEntry(rFSD);
}

// -----------------------------------------------------------------------

inline FourCharCode GetTag(const char aTagName[5])
{
    return (aTagName[0]<<24)+(aTagName[1]<<16)+(aTagName[2]<<8)+(aTagName[3]);
}

static unsigned GetUShort( const unsigned char* p ){return((p[0]<<8)+p[1]);}

#if MACOSX_SDK_VERSION >= 1070
extern "C" {
extern ATSFontRef FMGetATSFontRefFromFont(FMFont iFont);
}
#endif

const ImplFontCharMap* ImplMacFontData::GetImplFontCharMap() const
{
    // return the cached charmap
    if( mpCharMap )
        return mpCharMap;

    // set the default charmap
    mpCharMap = ImplFontCharMap::GetDefaultMap();
    mpCharMap->AddReference();

    // get the CMAP byte size
    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::GetImplFontCharMap : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return mpCharMap;

    // allocate a buffer for the CMAP raw data
    ByteVector aBuffer( nBufSize );

    // get the CMAP raw data
    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::GetImplFontCharMap : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
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

    // prepare to get the GSUB table raw data
    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus;
    eStatus = ATSFontGetTable( rFont, GetTag("GSUB"), 0, 0, NULL, &nBufSize );
    if( eStatus == noErr )
    {
        // allocate a buffer for the GSUB raw data
        ByteVector aBuffer( nBufSize );
        // get the GSUB raw data
        ByteCount nRawLength = 0;
        eStatus = ATSFontGetTable( rFont, GetTag("GSUB"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
        if( eStatus == noErr )
        {
            const unsigned char* pGSUBTable = &aBuffer[0];
            vcl::getTTScripts(maFontCapabilities.maGSUBScriptTags, pGSUBTable, nRawLength);
        }
    }
    eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, 0, NULL, &nBufSize );
    if( eStatus == noErr )
    {
        // allocate a buffer for the GSUB raw data
        ByteVector aBuffer( nBufSize );
        // get the OS/2 raw data
        ByteCount nRawLength = 0;
        eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
        if( eStatus == noErr )
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

    // prepare to get the OS/2 table raw data
    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadOs2Table : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return;

    // allocate a buffer for the OS/2 raw data
    ByteVector aBuffer( nBufSize );

    // get the OS/2 raw data
    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadOs2Table : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
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

    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadMacCmapEncoding : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return;

    ByteVector aBuffer( nBufSize );

    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadMacCmapEncoding : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::ReadMacCmapEncoding : ByteCount mismatch!\n");

    const unsigned char* pCmap = &aBuffer[0];

    if (nRawLength < 24 )
        return;
    if( GetUShort( pCmap ) != 0x0000 )
        return;
}

// =======================================================================

AquaSalGraphics::AquaSalGraphics()
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
    , mnATSUIRotation( 0 )
    , mfFontScale( 1.0 )
    , mfFontStretch( 1.0 )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
    , mbWindow( false )
{
    // create the style object for font attributes
    ATSUCreateStyle( &maATSUStyle );
}

// -----------------------------------------------------------------------

AquaSalGraphics::~AquaSalGraphics()
{
    CGPathRelease( mxClipPath );
    ATSUDisposeStyle( maATSUStyle );

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
}

// =======================================================================

void AquaSalGraphics::SetTextColor( SalColor nSalColor )
{
    RGBColor color;
    color.red     = (unsigned short) ( SALCOLOR_RED(nSalColor)   * 65535.0 / 255.0 );
    color.green   = (unsigned short) ( SALCOLOR_GREEN(nSalColor) * 65535.0 / 255.0 );
    color.blue    = (unsigned short) ( SALCOLOR_BLUE(nSalColor)  * 65535.0 / 255.0 );

    ATSUAttributeTag aTag = kATSUColorTag;
    ByteCount aValueSize = sizeof( color );
    ATSUAttributeValuePtr aValue = &color;

    OSStatus err = ATSUSetAttributes( maATSUStyle, 1, &aTag, &aValueSize, &aValue );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::SetTextColor() : Could not set font attributes!\n");
    if( err != noErr )
        return;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    (void)nFallbackLevel; // glyph-fallback on ATSU is done differently -> no fallback level

    // get the ATSU font metrics (in point units)
    // of the font that has eventually been size-limited

    ATSUFontID fontId;
    OSStatus err = ATSUGetAttribute( maATSUStyle, kATSUFontTag, sizeof(ATSUFontID), &fontId, 0 );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font id\n");

    ATSFontMetrics aMetrics;
    ATSFontRef rFont = FMGetATSFontRefFromFont( fontId );
    err = ATSFontGetHorizontalMetrics ( rFont, kATSOptionFlagsDefault, &aMetrics );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font metrics\n");
    if( err != noErr )
        return;

    // all ATS fonts are scalable fonts
    pMetric->mbScalableFont = true;
    // TODO: check if any kerning is possible
    pMetric->mbKernableFont = true;

    // convert into VCL font metrics (in unscaled pixel units)

    Fixed ptSize;
    err = ATSUGetAttribute( maATSUStyle, kATSUSizeTag, sizeof(Fixed), &ptSize, 0);
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font size\n");
    const double fPointSize = Fix2X( ptSize );

    // convert quartz units to pixel units
    // please see the comment in AquaSalGraphics::SetFont() for details
    const double fPixelSize = (mfFontScale * mfFakeDPIScale * fPointSize);
    pMetric->mnAscent       = static_cast<long>(+aMetrics.ascent  * fPixelSize + 0.5);
    pMetric->mnDescent      = static_cast<long>(-aMetrics.descent * fPixelSize + 0.5);
    const long nExtDescent  = static_cast<long>((-aMetrics.descent + aMetrics.leading) * fPixelSize + 0.5);
    pMetric->mnExtLeading   = nExtDescent - pMetric->mnDescent;
    pMetric->mnIntLeading   = 0;
    // ATSFontMetrics.avgAdvanceWidth is obsolete, so it is usually set to zero
    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    pMetric->mnWidth        = static_cast<long>(mfFontStretch * fPixelSize + 0.5);
}

// -----------------------------------------------------------------------

sal_uLong AquaSalGraphics::GetKernPairs( sal_uLong, ImplKernPairData* )
{
    return 0;
}

// -----------------------------------------------------------------------

static bool AddTempFontDir( const char* pDir )
{
    FSRef aPathFSRef;
    Boolean bIsDirectory = true;
    OSStatus eStatus = FSPathMakeRef( reinterpret_cast<const UInt8*>(pDir), &aPathFSRef, &bIsDirectory );
    DBG_ASSERTWARNING( (eStatus==noErr) && bIsDirectory, "vcl AddTempFontDir() with invalid directory name!" );
    if( eStatus != noErr )
        return false;

    // TODO: deactivate ATSFontContainerRef when closing app
    ATSFontContainerRef aATSFontContainer;

    const ATSFontContext eContext = kATSFontContextLocal; // TODO: *Global???
    eStatus = ::ATSFontActivateFromFileReference( &aPathFSRef,
        eContext, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault,
        &aATSFontContainer );
    if( eStatus != noErr )
        return false;

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
    aBrandFontDir.append( "/" LIBO_SHARE_FOLDER "/fonts/truetype/" );
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
    if (pSalData->mpFontList == NULL)
        pSalData->mpFontList = new SystemFontList();

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

    FSRef aNewRef;
    Boolean bIsDirectory = true;
    OString aCFileName = OUStringToOString( aUSytemPath, RTL_TEXTENCODING_UTF8 );
    OSStatus eStatus = FSPathMakeRef( (UInt8*)aCFileName.getStr(), &aNewRef, &bIsDirectory );
    DBG_ASSERT( (eStatus==noErr) && !bIsDirectory, "vcl AddTempDevFont() with invalid fontfile name!" );
    if( eStatus != noErr )
        return false;

    ATSFontContainerRef oContainer;

    const ATSFontContext eContext = kATSFontContextLocal; // TODO: *Global???
    eStatus = ::ATSFontActivateFromFileReference( &aNewRef,
        eContext, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault,
        &oContainer );
    if( eStatus != noErr )
        return false;

    // TODO: ATSFontDeactivate( oContainer ) when fonts are no longer needed
    // TODO: register new ImplMacFontdata in pFontList
    return true;
}

// -----------------------------------------------------------------------

// callbacks from ATSUGlyphGetCubicPaths() fore GetGlyphOutline()
struct GgoData { basegfx::B2DPolygon maPolygon; basegfx::B2DPolyPolygon* mpPolyPoly; };

static OSStatus GgoLineToProc( const Float32Point* pPoint, void* pData )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const basegfx::B2DPoint aB2DPoint( pPoint->x, pPoint->y );
    rPolygon.append( aB2DPoint );
    return noErr;
}

static OSStatus GgoCurveToProc( const Float32Point* pCP1, const Float32Point* pCP2,
    const Float32Point* pPoint, void* pData )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const sal_uInt32 nPointCount = rPolygon.count();
    const basegfx::B2DPoint aB2DControlPoint1( pCP1->x, pCP1->y );
    rPolygon.setNextControlPoint( nPointCount-1, aB2DControlPoint1 );
    const basegfx::B2DPoint aB2DEndPoint( pPoint->x, pPoint->y );
    rPolygon.append( aB2DEndPoint );
    const basegfx::B2DPoint aB2DControlPoint2( pCP2->x, pCP2->y );
    rPolygon.setPrevControlPoint( nPointCount, aB2DControlPoint2 );
    return noErr;
}

static OSStatus GgoClosePathProc( void* pData )
{
    GgoData* pGgoData = static_cast<GgoData*>(pData);
    basegfx::B2DPolygon& rPolygon = pGgoData->maPolygon;
    if( rPolygon.count() > 0 )
        pGgoData->mpPolyPoly->append( rPolygon );
    rPolygon.clear();
    return noErr;
}

static OSStatus GgoMoveToProc( const Float32Point* pPoint, void* pData )
{
    GgoClosePathProc( pData );
    OSStatus eStatus = GgoLineToProc( pPoint, pData );
    return eStatus;
}

sal_Bool AquaSalGraphics::GetGlyphOutline( sal_GlyphId nGlyphId, basegfx::B2DPolyPolygon& rPolyPoly )
{
    GgoData aGgoData;
    aGgoData.mpPolyPoly = &rPolyPoly;
    rPolyPoly.clear();

    ATSUStyle rATSUStyle = maATSUStyle; // TODO: handle glyph fallback when CWS pdffix02 is integrated
    GlyphID aGlyphId = nGlyphId & GF_IDXMASK;
    OSStatus eGgoStatus = noErr;
    OSStatus eStatus = ATSUGlyphGetCubicPaths( rATSUStyle, aGlyphId,
        GgoMoveToProc, GgoLineToProc, GgoCurveToProc, GgoClosePathProc,
        &aGgoData, &eGgoStatus );
    if( (eStatus != noErr) ) // TODO: why is (eGgoStatus!=noErr) when curves are involved?
        return false;

    GgoClosePathProc( &aGgoData );
    if( mfFontScale != 1.0 ) {
        rPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix(+mfFontScale, +mfFontScale));
    }
    return true;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::GetGlyphBoundRect( sal_GlyphId nGlyphId, Rectangle& rRect )
{
    ATSUStyle rATSUStyle = maATSUStyle; // TODO: handle glyph fallback
    GlyphID aGlyphId = nGlyphId & GF_IDXMASK;
    ATSGlyphScreenMetrics aGlyphMetrics;
    OSStatus eStatus = ATSUGlyphGetScreenMetrics( rATSUStyle,
        1, &aGlyphId, 0, FALSE, !mbNonAntialiasedText, &aGlyphMetrics );
    if( eStatus != noErr )
        return false;

    const long nMinX = (long)(+aGlyphMetrics.topLeft.x * mfFontScale + 0.5);
    const long nMinY = (long)(-aGlyphMetrics.topLeft.y * mfFontScale + 0.5);
    const long nWidth  = (long)(aGlyphMetrics.width * mfFontScale + 0.5);
    const long nHeight = (long)(aGlyphMetrics.height * mfFontScale + 0.5);
    Rectangle aRect(Point(nMinX, nMinY), Size(nWidth, nHeight));

    if ( mnATSUIRotation == 0 )
        rRect = aRect;
    else
    {
        const double fRadians = mnATSUIRotation * (M_PI/0xB40000);
        const double nSin = sin( fRadians );
        const double nCos = cos( fRadians );

        rRect.Left() =  nCos*aRect.Left() + nSin*aRect.Top();
        rRect.Top()  = -nSin*aRect.Left() - nCos*aRect.Top();

        rRect.Right()  =  nCos*aRect.Right() + nSin*aRect.Bottom();
        rRect.Bottom() = -nSin*aRect.Right() - nCos*aRect.Bottom();
    }

    return true;
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
    if( !pReqFont )
    {
        ATSUClearStyle( maATSUStyle );
        mpMacFontData = NULL;
        return 0;
    }

    // store the requested device font entry
    const ImplMacFontData* pMacFont = static_cast<const ImplMacFontData*>( pReqFont->mpFontData );
    mpMacFontData = pMacFont;

    // convert pixel units (as seen by upper layers) to typographic point units
    double fScaledAtsHeight = pReqFont->mfExactHeight;
    // avoid Fixed16.16 overflows by limiting the ATS font size
    static const float fMaxAtsHeight = 144.0;
    if( fScaledAtsHeight <= fMaxAtsHeight )
        mfFontScale = 1.0;
    else
    {
        mfFontScale = fScaledAtsHeight / fMaxAtsHeight;
        fScaledAtsHeight = fMaxAtsHeight;
    }
    Fixed fFixedSize = FloatToFixed( fScaledAtsHeight );
    // enable bold-emulation if needed
    Boolean bFakeBold = FALSE;
    if( (pReqFont->GetWeight() >= WEIGHT_BOLD)
    &&  (pMacFont->GetWeight() < WEIGHT_SEMIBOLD) )
        bFakeBold = TRUE;
    // enable italic-emulation if needed
    Boolean bFakeItalic = FALSE;
    if( ((pReqFont->GetSlant() == ITALIC_NORMAL) || (pReqFont->GetSlant() == ITALIC_OBLIQUE))
    && !((pMacFont->GetSlant() == ITALIC_NORMAL) || (pMacFont->GetSlant() == ITALIC_OBLIQUE)) )
        bFakeItalic = TRUE;

    // enable/disable antialiased text
    mbNonAntialiasedText = pReqFont->mbNonAntialiased;
    UInt32 nStyleRenderingOptions = kATSStyleNoOptions;
    if( pReqFont->mbNonAntialiased )
        nStyleRenderingOptions |= kATSStyleNoAntiAliasing;

    // set horizontal/vertical mode
    ATSUVerticalCharacterType aVerticalCharacterType = kATSUStronglyHorizontal;
    if( pReqFont->mbVertical )
        aVerticalCharacterType = kATSUStronglyVertical;

    // prepare ATS-fontid as type matching to the kATSUFontTag request
    ATSUFontID nFontID = static_cast<ATSUFontID>(pMacFont->GetFontId());

    // update ATSU style attributes with requested font parameters
    // TODO: no need to set styles which are already defaulted

    const ATSUAttributeTag aTag[] =
    {
        kATSUFontTag,
        kATSUSizeTag,
        kATSUQDBoldfaceTag,
        kATSUQDItalicTag,
        kATSUStyleRenderingOptionsTag,
        kATSUVerticalCharacterTag
    };

    const ByteCount aValueSize[] =
    {
        sizeof(ATSUFontID),
        sizeof(fFixedSize),
        sizeof(bFakeBold),
        sizeof(bFakeItalic),
        sizeof(nStyleRenderingOptions),
        sizeof(aVerticalCharacterType)
    };

    const ATSUAttributeValuePtr aValue[] =
    {
        &nFontID,
        &fFixedSize,
        &bFakeBold,
        &bFakeItalic,
        &nStyleRenderingOptions,
        &aVerticalCharacterType
    };

    static const int nTagCount = SAL_N_ELEMENTS(aTag);
    OSStatus eStatus = ATSUSetAttributes( maATSUStyle, nTagCount,
                             aTag, aValueSize, aValue );
    // reset ATSUstyle if there was an error
    if( eStatus != noErr )
    {
        DBG_WARNING( "AquaSalGraphics::SetFont() : Could not set font attributes!\n");
        ATSUClearStyle( maATSUStyle );
        mpMacFontData = NULL;
        return 0;
    }

    // prepare font stretching
    const ATSUAttributeTag aMatrixTag = kATSUFontMatrixTag;
    if( (pReqFont->mnWidth == 0) || (pReqFont->mnWidth == pReqFont->mnHeight) )
    {
        mfFontStretch = 1.0;
        ATSUClearAttributes( maATSUStyle, 1, &aMatrixTag );
    }
    else
    {
        mfFontStretch = (float)pReqFont->mnWidth / pReqFont->mnHeight;
        CGAffineTransform aMatrix = CGAffineTransformMakeScale( mfFontStretch, 1.0F );
        const ATSUAttributeValuePtr aAttr = &aMatrix;
        const ByteCount aMatrixBytes = sizeof(aMatrix);
        eStatus = ATSUSetAttributes( maATSUStyle, 1, &aMatrixTag, &aMatrixBytes, &aAttr );
        DBG_ASSERT( (eStatus==noErr), "AquaSalGraphics::SetFont() : Could not set font matrix\n");
    }

    // prepare font rotation
    mnATSUIRotation = FloatToFixed( pReqFont->mnOrientation / 10.0 );

#if OSL_DEBUG_LEVEL > 3
    fprintf( stderr, "SetFont to (\"%s\", \"%s\", fontid=%d) for (\"%s\" \"%s\" weight=%d, slant=%d size=%dx%d orientation=%d)\n",
             OUStringToOString( pMacFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             OUStringToOString( pMacFont->GetStyleName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             (int)nFontID,
             OUStringToOString( pReqFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             OUStringToOString( pReqFont->GetStyleName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             pReqFont->GetWeight(),
             pReqFont->GetSlant(),
             pReqFont->mnHeight,
             pReqFont->mnWidth,
             pReqFont->mnOrientation);
#endif

    return 0;
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
static void FakeDirEntry( FourCharCode eFCC, ByteCount nOfs, ByteCount nLen,
    const unsigned char* /*pData*/, unsigned char*& rpDest )
{
    // write entry tag
    rpDest[ 0] = (char)(eFCC >> 24);
    rpDest[ 1] = (char)(eFCC >> 16);
    rpDest[ 2] = (char)(eFCC >>  8);
    rpDest[ 3] = (char)(eFCC >>  0);
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

bool AquaSalGraphics::GetRawFontData( const PhysicalFontFace* pFontData,
                                      ByteVector& rBuffer, bool* pJustCFF )
{
    const ImplMacFontData* pMacFont = static_cast<const ImplMacFontData*>(pFontData);
    const ATSUFontID nFontId = static_cast<ATSUFontID>(pMacFont->GetFontId());
    ATSFontRef rFont = FMGetATSFontRefFromFont( nFontId );

    ByteCount nCffLen = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("CFF "), 0, 0, NULL, &nCffLen);
    if( pJustCFF != NULL )
    {
        *pJustCFF = (eStatus == noErr) && (nCffLen > 0);
        if( *pJustCFF )
        {
            rBuffer.resize( nCffLen );
            eStatus = ATSFontGetTable( rFont, GetTag("CFF "), 0, nCffLen, (void*)&rBuffer[0], &nCffLen);
            if( (eStatus != noErr) || (nCffLen <= 0) )
                return false;
            return true;
        }
    }

    // get font table availability and size in bytes
    ByteCount nHeadLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("head"), 0, 0, NULL, &nHeadLen);
    if( (eStatus != noErr) || (nHeadLen <= 0) )
        return false;
    ByteCount nMaxpLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("maxp"), 0, 0, NULL, &nMaxpLen);
    if( (eStatus != noErr) || (nMaxpLen <= 0) )
        return false;
    ByteCount nCmapLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nCmapLen);
    if( (eStatus != noErr) || (nCmapLen <= 0) )
        return false;
    ByteCount nNameLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("name"), 0, 0, NULL, &nNameLen);
    if( (eStatus != noErr) || (nNameLen <= 0) )
        return false;
    ByteCount nHheaLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("hhea"), 0, 0, NULL, &nHheaLen);
    if( (eStatus != noErr) || (nHheaLen <= 0) )
        return false;
    ByteCount nHmtxLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("hmtx"), 0, 0, NULL, &nHmtxLen);
    if( (eStatus != noErr) || (nHmtxLen <= 0) )
        return false;

    // get the glyph outline tables
    ByteCount nLocaLen  = 0;
    ByteCount nGlyfLen  = 0;
    if( (eStatus != noErr) || (nCffLen <= 0) )
    {
        eStatus = ATSFontGetTable( rFont, GetTag("loca"), 0, 0, NULL, &nLocaLen);
        if( (eStatus != noErr) || (nLocaLen <= 0) )
            return false;
        eStatus = ATSFontGetTable( rFont, GetTag("glyf"), 0, 0, NULL, &nGlyfLen);
        if( (eStatus != noErr) || (nGlyfLen <= 0) )
            return false;
    }

    ByteCount nPrepLen=0, nCvtLen=0, nFpgmLen=0;
    if( nGlyfLen )  // TODO: reduce PDF size by making hint subsetting optional
    {
        eStatus = ATSFontGetTable( rFont, GetTag("prep"), 0, 0, NULL, &nPrepLen);
        eStatus = ATSFontGetTable( rFont, GetTag("cvt "), 0, 0, NULL, &nCvtLen);
        eStatus = ATSFontGetTable( rFont, GetTag("fpgm"), 0, 0, NULL, &nFpgmLen);
    }

    // prepare a byte buffer for a fake font
    int nTableCount = 7;
    nTableCount += (nPrepLen>0) + (nCvtLen>0) + (nFpgmLen>0) + (nGlyfLen>0);
    const ByteCount nFdirLen = 12 + 16*nTableCount;
    ByteCount nTotalLen = nFdirLen;
    nTotalLen += nHeadLen + nMaxpLen + nNameLen + nCmapLen;
    if( nGlyfLen )
        nTotalLen += nLocaLen + nGlyfLen;
    else
        nTotalLen += nCffLen;
    nTotalLen += nHheaLen + nHmtxLen;
    nTotalLen += nPrepLen + nCvtLen + nFpgmLen;
    rBuffer.resize( nTotalLen );

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
    ByteCount nOfs = nFdirLen;
    unsigned char* pFakeEntry = &rBuffer[12];
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nCmapLen, (void*)&rBuffer[nOfs], &nCmapLen);
    FakeDirEntry( GetTag("cmap"), nOfs, nCmapLen, &rBuffer[0], pFakeEntry );
    nOfs += nCmapLen;
    if( nCvtLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("cvt "), 0, nCvtLen, (void*)&rBuffer[nOfs], &nCvtLen);
        FakeDirEntry( GetTag("cvt "), nOfs, nCvtLen, &rBuffer[0], pFakeEntry );
        nOfs += nCvtLen;
    }
    if( nFpgmLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("fpgm"), 0, nFpgmLen, (void*)&rBuffer[nOfs], &nFpgmLen);
        FakeDirEntry( GetTag("fpgm"), nOfs, nFpgmLen, &rBuffer[0], pFakeEntry );
        nOfs += nFpgmLen;
    }
    if( nCffLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("CFF "), 0, nCffLen, (void*)&rBuffer[nOfs], &nCffLen);
        FakeDirEntry( GetTag("CFF "), nOfs, nCffLen, &rBuffer[0], pFakeEntry );
        nOfs += nGlyfLen;
    } else {
        eStatus = ATSFontGetTable( rFont, GetTag("glyf"), 0, nGlyfLen, (void*)&rBuffer[nOfs], &nGlyfLen);
        FakeDirEntry( GetTag("glyf"), nOfs, nGlyfLen, &rBuffer[0], pFakeEntry );
        nOfs += nGlyfLen;
        eStatus = ATSFontGetTable( rFont, GetTag("loca"), 0, nLocaLen, (void*)&rBuffer[nOfs], &nLocaLen);
        FakeDirEntry( GetTag("loca"), nOfs, nLocaLen, &rBuffer[0], pFakeEntry );
        nOfs += nLocaLen;
    }
    eStatus = ATSFontGetTable( rFont, GetTag("head"), 0, nHeadLen, (void*)&rBuffer[nOfs], &nHeadLen);
    FakeDirEntry( GetTag("head"), nOfs, nHeadLen, &rBuffer[0], pFakeEntry );
    nOfs += nHeadLen;
    eStatus = ATSFontGetTable( rFont, GetTag("hhea"), 0, nHheaLen, (void*)&rBuffer[nOfs], &nHheaLen);
    FakeDirEntry( GetTag("hhea"), nOfs, nHheaLen, &rBuffer[0], pFakeEntry );
    nOfs += nHheaLen;
    eStatus = ATSFontGetTable( rFont, GetTag("hmtx"), 0, nHmtxLen, (void*)&rBuffer[nOfs], &nHmtxLen);
    FakeDirEntry( GetTag("hmtx"), nOfs, nHmtxLen, &rBuffer[0], pFakeEntry );
    nOfs += nHmtxLen;
    eStatus = ATSFontGetTable( rFont, GetTag("maxp"), 0, nMaxpLen, (void*)&rBuffer[nOfs], &nMaxpLen);
    FakeDirEntry( GetTag("maxp"), nOfs, nMaxpLen, &rBuffer[0], pFakeEntry );
    nOfs += nMaxpLen;
    eStatus = ATSFontGetTable( rFont, GetTag("name"), 0, nNameLen, (void*)&rBuffer[nOfs], &nNameLen);
    FakeDirEntry( GetTag("name"), nOfs, nNameLen, &rBuffer[0], pFakeEntry );
    nOfs += nNameLen;
    if( nPrepLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("prep"), 0, nPrepLen, (void*)&rBuffer[nOfs], &nPrepLen);
        FakeDirEntry( GetTag("prep"), nOfs, nPrepLen, &rBuffer[0], pFakeEntry );
        nOfs += nPrepLen;
    }

    DBG_ASSERT( (nOfs==nTotalLen), "AquaSalGraphics::CreateFontSubset (nOfs!=nTotalLen)");

    return sal_True;
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
    OSStatus err;
    aSysFontData.nSize = sizeof( SystemFontData );

    // NOTE: Native ATSU font fallbacks are used, not the VCL fallbacks.
    ATSUFontID fontId;
    err = ATSUGetAttribute( maATSUStyle, kATSUFontTag, sizeof(fontId), &fontId, 0 );
    if (err) fontId = 0;
    aSysFontData.aATSUFontID = (void *) fontId;

    Boolean bFbold;
    err = ATSUGetAttribute( maATSUStyle, kATSUQDBoldfaceTag, sizeof(bFbold), &bFbold, 0 );
    if (err) bFbold = FALSE;
    aSysFontData.bFakeBold = (bool) bFbold;

    Boolean bFItalic;
    err = ATSUGetAttribute( maATSUStyle, kATSUQDItalicTag, sizeof(bFItalic), &bFItalic, 0 );
    if (err) bFItalic = FALSE;
    aSysFontData.bFakeItalic = (bool) bFItalic;

    ATSUVerticalCharacterType aVerticalCharacterType;
    err = ATSUGetAttribute( maATSUStyle, kATSUVerticalCharacterTag, sizeof(aVerticalCharacterType), &aVerticalCharacterType, 0 );
    if (!err && aVerticalCharacterType == kATSUStronglyVertical) {
        aSysFontData.bVerticalCharacterType = true;
    } else {
        aSysFontData.bVerticalCharacterType = false;
    }

    aSysFontData.bAntialias = !mbNonAntialiasedText;

    return aSysFontData;
}

// -----------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
