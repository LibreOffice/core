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

#include <boost/unordered_map.hpp>

#include "impfont.hxx"
#include "outfont.hxx"
#include "sallayout.hxx"

#ifdef MACOSX
#include "osx/salinst.h"
#include "osx/saldata.hxx"
#endif
#include "quartz/salgdi.h"
#include "quartz/utils.h"
#include "ctfonts.hxx"

#include <vcl/settings.hxx>

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"



inline double toRadian(int nDegree)
{
    return nDegree * (M_PI / 1800.0);
}

CoreTextStyle::CoreTextStyle( const FontSelectPattern& rFSD )
:   mpFontData( (CoreTextFontData*)rFSD.mpFontData )
,   mfFontStretch( 1.0 )
,   mfFontRotation( 0.0 )
,   mpStyleDict( NULL )
{
    mpFontData = (CoreTextFontData*)rFSD.mpFontData;
    const FontSelectPattern* const pReqFont = &rFSD;

    double fScaledFontHeight = pReqFont->mfExactHeight;

    // convert font rotation to radian
    mfFontRotation = toRadian(pReqFont->mnOrientation);

    // dummy matrix so we can use CGAffineTransformConcat() below
    CGAffineTransform aMatrix = CGAffineTransformMakeTranslation(0, 0);

    // handle font stretching if any
    if( (pReqFont->mnWidth != 0) && (pReqFont->mnWidth != pReqFont->mnHeight) )
    {
        mfFontStretch = (float)pReqFont->mnWidth / pReqFont->mnHeight;
        aMatrix = CGAffineTransformConcat(aMatrix, CGAffineTransformMakeScale(mfFontStretch, 1.0F));
    }

    // create the style object for CoreText font attributes
    static const CFIndex nMaxDictSize = 16; // TODO: does this really suffice?
    mpStyleDict = CFDictionaryCreateMutable( NULL, nMaxDictSize,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

    CFBooleanRef pCFVertBool = pReqFont->mbVertical ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue( mpStyleDict, kCTVerticalFormsAttributeName, pCFVertBool );

    // fake bold
    if ( (pReqFont->GetWeight() >= WEIGHT_BOLD) &&
         ((mpFontData->GetWeight() < WEIGHT_SEMIBOLD) &&
          (mpFontData->GetWeight() != WEIGHT_DONTKNOW)) )
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
        int nStroke = -10.0;
        CFNumberRef rStroke = CFNumberCreate(NULL, kCFNumberSInt32Type, &nStroke);
        CFDictionarySetValue(mpStyleDict, kCTStrokeWidthAttributeName, rStroke);
#else /* kCTStrokeWidthAttributeName is not available */
        /* do we really need "fake" bold? */
#endif
    }

    // fake italic
    if (((pReqFont->GetSlant() == ITALIC_NORMAL) || (pReqFont->GetSlant() == ITALIC_OBLIQUE))
    && (mpFontData->GetSlant() == ITALIC_NONE))
    {
        aMatrix = CGAffineTransformConcat(aMatrix, CGAffineTransformMake(1, 0, toRadian(120), 1, 0, 0));
    }

    CTFontDescriptorRef pFontDesc = (CTFontDescriptorRef)mpFontData->GetFontId();
    CTFontRef pNewCTFont = CTFontCreateWithFontDescriptor( pFontDesc, fScaledFontHeight, &aMatrix );
    CFDictionarySetValue( mpStyleDict, kCTFontAttributeName, pNewCTFont );
    CFRelease( pNewCTFont);

#if 0 // LastResort is implicit in CoreText's font cascading
    const void* aGFBDescriptors[] = { CTFontDescriptorCreateWithNameAndSize( CFSTR("LastResort"), 0) }; // TODO: use the full GFB list
    const int nGfbCount = sizeof(aGFBDescriptors) / sizeof(*aGFBDescriptors);
    CFArrayRef pGfbList = CFArrayCreate( NULL, aGFBDescriptors, nGfbCount, &kCFTypeArrayCallBacks);
    CFDictionaryAddValue( mpStyleDict, kCTFontCascadeListAttribute, pGfbList);
    CFRelease( pGfbList);
#endif
}



CoreTextStyle::~CoreTextStyle( void )
{
    if( mpStyleDict )
        CFRelease( mpStyleDict );
}



void CoreTextStyle::GetFontMetric( ImplFontMetricData& rMetric ) const
{
    // get the matching CoreText font handle
    // TODO: is it worth it to cache the CTFontRef in SetFont() and reuse it here?
    CTFontRef aCTFontRef = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );

    rMetric.mnAscent       = CTFontGetAscent( aCTFontRef );
    rMetric.mnDescent      = CTFontGetDescent( aCTFontRef );
    rMetric.mnExtLeading   = CTFontGetLeading( aCTFontRef );
    rMetric.mnIntLeading   = 0;
    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    rMetric.mnWidth        = lrint( CTFontGetSize( aCTFontRef ) * mfFontStretch);

    // all CoreText fonts are scalable
    rMetric.mbScalableFont = true;
    rMetric.mbKernableFont = true;
}



bool CoreTextStyle::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect ) const
{
    CGGlyph nCGGlyph = aGlyphId & GF_IDXMASK;
    // XXX: this is broken if the glyph came from fallback font
    CTFontRef aCTFontRef = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );

    const CTFontOrientation aFontOrientation = kCTFontDefaultOrientation; // TODO: horz/vert
    const CGRect aCGRect = CTFontGetBoundingRectsForGlyphs( aCTFontRef, aFontOrientation, &nCGGlyph, NULL, 1 );

    rRect.Left()   = lrint( aCGRect.origin.x );
    rRect.Top()    = lrint( aCGRect.origin.y );
    rRect.Right()  = lrint( aCGRect.origin.x + aCGRect.size.width );
    rRect.Bottom() = lrint( aCGRect.origin.y + aCGRect.size.height );
    return true;
}



// callbacks from CTFontCreatePathForGlyph+CGPathApply for GetGlyphOutline()
struct GgoData { basegfx::B2DPolygon maPolygon; basegfx::B2DPolyPolygon* mpPolyPoly; };

static void MyCGPathApplierFunc( void* pData, const CGPathElement* pElement )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const int nPointCount = rPolygon.count();

    switch( pElement->type )
    {
    case kCGPathElementCloseSubpath:
    case kCGPathElementMoveToPoint:
        if( nPointCount > 0 ) {
            static_cast<GgoData*>(pData)->mpPolyPoly->append( rPolygon );
            rPolygon.clear();
        }
        // fall through for kCGPathElementMoveToPoint:
        if( pElement->type != kCGPathElementMoveToPoint )
            break;
    case kCGPathElementAddLineToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[0].x, -pElement->points[0].y ) );
        break;
    case kCGPathElementAddCurveToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[2].x, -pElement->points[2].y ) );
        rPolygon.setNextControlPoint( nPointCount-1, basegfx::B2DPoint( pElement->points[0].x, -pElement->points[0].y ) );
        rPolygon.setPrevControlPoint( nPointCount+0, basegfx::B2DPoint( pElement->points[1].x, -pElement->points[1].y ) );
        break;
    case kCGPathElementAddQuadCurveToPoint: {
        const basegfx::B2DPoint aStartPt = rPolygon.getB2DPoint( nPointCount-1 );
        const basegfx::B2DPoint aCtrPt1( (aStartPt.getX() + 2* pElement->points[0].x) / 3.0,
                    (aStartPt.getY() - 2 * pElement->points[0].y) / 3.0 );
        const basegfx::B2DPoint aCtrPt2( (+2 * +pElement->points[0].x + pElement->points[1].x) / 3.0,
                (-2 * pElement->points[0].y - pElement->points[1].y) / 3.0 );
        rPolygon.append( basegfx::B2DPoint( +pElement->points[1].x, -pElement->points[1].y ) );
        rPolygon.setNextControlPoint( nPointCount-1, aCtrPt1 );
        rPolygon.setPrevControlPoint( nPointCount+0, aCtrPt2 );
        } break;
    }
}

bool CoreTextStyle::GetGlyphOutline( sal_GlyphId aGlyphId, basegfx::B2DPolyPolygon& rResult ) const
{
    rResult.clear();

    CGGlyph nCGGlyph = aGlyphId & GF_IDXMASK;
    // XXX: this is broken if the glyph came from fallback font
    CTFontRef pCTFont = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );
    CGPathRef xPath = CTFontCreatePathForGlyph( pCTFont, nCGGlyph, NULL );

    GgoData aGgoData;
    aGgoData.mpPolyPoly = &rResult;
    CGPathApply( xPath, (void*)&aGgoData, MyCGPathApplierFunc );
#if 0 // TODO: does OSX ensure that the last polygon is always closed?
    const CGPathElement aClosingElement = { kCGPathElementCloseSubpath, NULL };
    MyCGPathApplierFunc( (void*)&aGgoData, &aClosingElement );
#endif

    return true;
}



void CoreTextStyle::SetTextColor( const RGBAColor& rColor )
{
    CGFloat aColor[] = { rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue(), rColor.GetAlpha() };
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGColorRef pCGColor = CGColorCreate( cs, aColor );
    CGColorSpaceRelease( cs );
    CFDictionarySetValue( mpStyleDict, kCTForegroundColorAttributeName, pCGColor );
    CFRelease( pCGColor);
}



PhysicalFontFace* CoreTextFontData::Clone( void ) const
{
    return new CoreTextFontData( *this);
}



CoreTextStyle* CoreTextFontData::CreateTextStyle( const FontSelectPattern& rFSD ) const
{
    return new CoreTextStyle( rFSD);
}



ImplFontEntry* CoreTextFontData::CreateFontInstance( /*const*/ FontSelectPattern& rFSD ) const
{
    return new ImplFontEntry( rFSD);
}



int CoreTextFontData::GetFontTable( const char pTagName[5], unsigned char* pResultBuf ) const
{
    DBG_ASSERT( pTagName[4]=='\0', "CoreTextFontData::GetFontTable with invalid tagname!\n" );

    const CTFontTableTag nTagCode = (pTagName[0]<<24) + (pTagName[1]<<16) + (pTagName[2]<<8) + (pTagName[3]<<0);

    // get the raw table length
    CTFontDescriptorRef pFontDesc = reinterpret_cast<CTFontDescriptorRef>( GetFontId());
    CTFontRef rCTFont = CTFontCreateWithFontDescriptor( pFontDesc, 0.0, NULL);
#if defined(MACOSX) && MACOSX_SDK_VERSION < 1080
    const uint32_t opts( kCTFontTableOptionExcludeSynthetic );
#else
    const uint32_t opts( kCTFontTableOptionNoOptions );
#endif
    CFDataRef pDataRef = CTFontCopyTable( rCTFont, nTagCode, opts);
    CFRelease( rCTFont);
    if( !pDataRef)
        return 0;

    const CFIndex nByteLength = CFDataGetLength( pDataRef);

    // get the raw table data if requested
    if( pResultBuf && (nByteLength > 0))
    {
        const CFRange aFullRange = CFRangeMake( 0, nByteLength);
        CFDataGetBytes( pDataRef, aFullRange, (UInt8*)pResultBuf);
    }

    CFRelease( pDataRef);

    return (int)nByteLength;
}



ImplDevFontAttributes DevFontFromCTFontDescriptor( CTFontDescriptorRef pFD, bool* bFontEnabled )
{
    // all CoreText fonts are device fonts that can rotate just fine
    ImplDevFontAttributes rDFA;
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

    // reset the font attributes
    rDFA.SetFamilyType( FAMILY_DONTKNOW );
    rDFA.SetPitch( PITCH_VARIABLE );
    rDFA.SetWidthType( WIDTH_NORMAL );
    rDFA.SetWeight( WEIGHT_NORMAL );
    rDFA.SetItalic( ITALIC_NONE );
    rDFA.SetSymbolFlag( false );

    // all scalable fonts on this platform are subsettable
    rDFA.mbEmbeddable = false;
    rDFA.mbSubsettable = true;

    // get font name
#ifdef MACOSX
    const OUString aUILang = Application::GetSettings().GetUILanguageTag().getLanguage();
    CFStringRef pUILang = CFStringCreateWithCharacters( kCFAllocatorDefault, aUILang.getStr(), aUILang.getLength() );
    CFStringRef pLang = NULL;
    CFStringRef pFamilyName = (CFStringRef)CTFontDescriptorCopyLocalizedAttribute( pFD, kCTFontFamilyNameAttribute, &pLang );
    if ( !pLang || ( CFStringCompare( pUILang, pLang, 0 ) != kCFCompareEqualTo ))
    {
        if(pFamilyName)
        {
            CFRelease( pFamilyName );
        }
        pFamilyName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute );
    }
#else
    // No "Application" on iOS. And it is unclear whether this code
    // snippet will actually ever get invoked on iOS anyway. So just
    // use the old code that uses a non-localized font name.
    CFStringRef pFamilyName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute );
#endif

    rDFA.SetFamilyName( GetOUString( pFamilyName ) );

    // get font style
    CFStringRef pStyleName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontStyleNameAttribute );
    rDFA.SetStyleName( GetOUString( pStyleName ) );

    // get font-enabled status
    if( bFontEnabled ) {
        int bEnabled = TRUE; // by default (and when we're on OS X < 10.6) it's "enabled"
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
        CFNumberRef pEnabled = (CFNumberRef)CTFontDescriptorCopyAttribute( pFD, kCTFontEnabledAttribute );
        CFNumberGetValue( pEnabled, kCFNumberIntType, &bEnabled );
#endif
        *bFontEnabled = bEnabled;
    }

    // get font attributes
    CFDictionaryRef pAttrDict = (CFDictionaryRef)CTFontDescriptorCopyAttribute( pFD, kCTFontTraitsAttribute );

    // get symbolic trait
    // TODO: use other traits such as MonoSpace/Condensed/Expanded or Vertical too
    SInt64 nSymbolTrait = 0;
    CFNumberRef pSymbolNum = NULL;
    if( CFDictionaryGetValueIfPresent( pAttrDict, kCTFontSymbolicTrait, (const void**)&pSymbolNum ) ) {
        CFNumberGetValue( pSymbolNum, kCFNumberSInt64Type, &nSymbolTrait );
        rDFA.SetSymbolFlag( ((nSymbolTrait & kCTFontClassMaskTrait) == kCTFontSymbolicClass) );
    }

    // get the font weight
    double fWeight = 0;
    CFNumberRef pWeightNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontWeightTrait );
    CFNumberGetValue( pWeightNum, kCFNumberDoubleType, &fWeight );
    int nInt = WEIGHT_NORMAL;
    if( fWeight > 0 ) {
        nInt = rint(WEIGHT_NORMAL + fWeight * ((WEIGHT_BLACK - WEIGHT_NORMAL)/0.68));
        if( nInt > WEIGHT_BLACK )
            nInt = WEIGHT_BLACK;
    } else if( fWeight < 0 ) {
        nInt = rint(WEIGHT_NORMAL + fWeight * ((WEIGHT_NORMAL - WEIGHT_THIN)/0.9));
        if( nInt < WEIGHT_THIN )
            nInt = WEIGHT_THIN;
    }
    rDFA.SetWeight( (FontWeight)nInt );

    // get the font slant
    double fSlant = 0;
    CFNumberRef pSlantNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontSlantTrait );
    CFNumberGetValue( pSlantNum, kCFNumberDoubleType, &fSlant );
    if( fSlant >= 0.035 )
        rDFA.SetItalic( ITALIC_NORMAL );

    // get width trait
    double fWidth = 0;
    CFNumberRef pWidthNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontWidthTrait );
    CFNumberGetValue( pWidthNum, kCFNumberDoubleType, &fWidth );
    nInt = WIDTH_NORMAL;
    if( fWidth > 0 ) {
        nInt = rint( WIDTH_NORMAL + fWidth * ((WIDTH_ULTRA_EXPANDED - WIDTH_NORMAL)/0.4));
        if( nInt > WIDTH_ULTRA_EXPANDED )
            nInt = WIDTH_ULTRA_EXPANDED;
    } else if( fWidth < 0 ) {
        nInt = rint( WIDTH_NORMAL + fWidth * ((WIDTH_NORMAL - WIDTH_ULTRA_CONDENSED)/0.5));
        if( nInt < WIDTH_ULTRA_CONDENSED )
            nInt = WIDTH_ULTRA_CONDENSED;
    }
    rDFA.SetWidthType( (FontWidth)nInt );

    // release the attribute dict that we had copied
    CFRelease( pAttrDict );

    // TODO? also use the HEAD table if available to get more attributes
//  CFDataRef CTFontCopyTable( CTFontRef, kCTFontTableHead, /*kCTFontTableOptionNoOptions*/kCTFontTableOptionExcludeSynthetic );

    return rDFA;
}

static void CTFontEnumCallBack( const void* pValue, void* pContext )
{
    CTFontDescriptorRef pFD = static_cast<CTFontDescriptorRef>(pValue);

    bool bFontEnabled;
    ImplDevFontAttributes rDFA = DevFontFromCTFontDescriptor( pFD, &bFontEnabled );

    if( bFontEnabled)
    {
        const sal_IntPtr nFontId = (sal_IntPtr)pValue;
        CoreTextFontData* pFontData = new CoreTextFontData( rDFA, nFontId );
        SystemFontList* pFontList = (SystemFontList*)pContext;
        pFontList->AddFont( pFontData );
    }
}



SystemFontList::SystemFontList()
:   mpCTFontCollection( NULL )
,   mpCTFontArray( NULL )
{}



SystemFontList::~SystemFontList()
{
    CTFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        delete (*it).second;
    maFontContainer.clear();

    if( mpCTFontArray )
        CFRelease( mpCTFontArray );
    if( mpCTFontCollection )
        CFRelease( mpCTFontCollection );
}



void SystemFontList::AddFont( CoreTextFontData* pFontData )
{
    sal_IntPtr nFontId = pFontData->GetFontId();
    maFontContainer[ nFontId ] = pFontData;
}



void SystemFontList::AnnounceFonts( PhysicalFontCollection& rFontCollection ) const
{
    CTFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        rFontCollection.Add( (*it).second->Clone() );
}



CoreTextFontData* SystemFontList::GetFontDataFromId( sal_IntPtr nFontId ) const
{
    CTFontContainer::const_iterator it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
        return NULL;
    return (*it).second;
}



bool SystemFontList::Init( void )
{
    // enumerate available system fonts
    static const int nMaxDictEntries = 8;
    CFMutableDictionaryRef pCFDict = CFDictionaryCreateMutable( NULL,
        nMaxDictEntries, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    CFDictionaryAddValue( pCFDict, kCTFontCollectionRemoveDuplicatesOption, kCFBooleanTrue );
    mpCTFontCollection = CTFontCollectionCreateFromAvailableFonts( pCFDict );
    CFRelease( pCFDict );

    mpCTFontArray = CTFontCollectionCreateMatchingFontDescriptors( mpCTFontCollection );
    const int nFontCount = CFArrayGetCount( mpCTFontArray );
    const CFRange aFullRange = CFRangeMake( 0, nFontCount );
    CFArrayApplyFunction( mpCTFontArray, aFullRange, CTFontEnumCallBack, this );

    return true;
}



SystemFontList* GetCoretextFontList( void )
{
    SystemFontList* pList = new SystemFontList();
    if( !pList->Init() ) {
        delete pList;
        return NULL;
    }

    return pList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
