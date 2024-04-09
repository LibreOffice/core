/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <sal/log.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/long.hxx>
#include <vcl/settings.hxx>


#include <quartz/ctfonts.hxx>
#include <impfont.hxx>
#ifdef MACOSX
#include <osx/saldata.hxx>
#include <osx/salinst.h>
#endif
#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>
#include <impglyphitem.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <quartz/salgdi.h>
#include <quartz/utils.h>
#include <sallayout.hxx>
#include <hb-coretext.h>

CoreTextFont::CoreTextFont(const CoreTextFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , mfFontStretch( 1.0 )
    , mfFontRotation( 0.0 )
    , mpCTFont(nullptr)
{
    double fScaledFontHeight = rFSP.mfExactHeight;

    // convert font rotation to radian
    mfFontRotation = toRadians(rFSP.mnOrientation);

    // dummy matrix so we can use CGAffineTransformConcat() below
    CGAffineTransform aMatrix = CGAffineTransformMakeTranslation(0, 0);

    // handle font stretching if any
    if( (rFSP.mnWidth != 0) && (rFSP.mnWidth != rFSP.mnHeight) )
    {
        mfFontStretch = float(rFSP.mnWidth) / rFSP.mnHeight;
        aMatrix = CGAffineTransformConcat(aMatrix, CGAffineTransformMakeScale(mfFontStretch, 1.0F));
    }

    // artificial italic
    if (NeedsArtificialItalic())
        aMatrix = CGAffineTransformConcat(aMatrix, CGAffineTransformMake(1, 0, ARTIFICIAL_ITALIC_SKEW, 1, 0, 0));

    CTFontDescriptorRef pFontDesc = rPFF.GetFontDescriptorRef();
    mpCTFont = CTFontCreateWithFontDescriptor( pFontDesc, fScaledFontHeight, &aMatrix );
}

CoreTextFont::~CoreTextFont()
{
    if (mpCTFont)
        CFRelease(mpCTFont);
}

void CoreTextFont::GetFontMetric( ImplFontMetricDataRef const & rxFontMetric )
{
    rxFontMetric->ImplCalcLineSpacing(this);
    rxFontMetric->ImplInitBaselines(this);

    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    rxFontMetric->SetWidth( lrint( CTFontGetSize(mpCTFont) * mfFontStretch) );

    rxFontMetric->SetMinKashida(GetKashidaWidth());
}

bool CoreTextFont::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool bVertical) const
{
    CGGlyph nCGGlyph = nId;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.11 kCTFontDefaultOrientation
    const CTFontOrientation aFontOrientation = kCTFontDefaultOrientation; // TODO: horz/vert
    SAL_WNODEPRECATED_DECLARATIONS_POP
    CGRect aCGRect = CTFontGetBoundingRectsForGlyphs(mpCTFont, aFontOrientation, &nCGGlyph, nullptr, 1);

    // Apply font rotation to non-vertical glyphs.
    if (mfFontRotation && !bVertical)
        aCGRect = CGRectApplyAffineTransform(aCGRect, CGAffineTransformMakeRotation(mfFontRotation));

    tools::Long xMin = floor(aCGRect.origin.x);
    tools::Long yMin = floor(aCGRect.origin.y);
    tools::Long xMax = ceil(aCGRect.origin.x + aCGRect.size.width);
    tools::Long yMax = ceil(aCGRect.origin.y + aCGRect.size.height);
    rRect = tools::Rectangle(xMin, -yMax, xMax, -yMin);
    return true;
}

namespace {

// callbacks from CTFontCreatePathForGlyph+CGPathApply for GetGlyphOutline()
struct GgoData { basegfx::B2DPolygon maPolygon; basegfx::B2DPolyPolygon* mpPolyPoly; };

}

static void MyCGPathApplierFunc( void* pData, const CGPathElement* pElement )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const int nPointCount = rPolygon.count();

    switch( pElement->type )
    {
    case kCGPathElementCloseSubpath:
    case kCGPathElementMoveToPoint:
        if( nPointCount > 0 )
        {
            static_cast<GgoData*>(pData)->mpPolyPoly->append( rPolygon );
            rPolygon.clear();
        }
        // fall through for kCGPathElementMoveToPoint:
        if( pElement->type != kCGPathElementMoveToPoint )
        {
            break;
        }
        [[fallthrough]];
    case kCGPathElementAddLineToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[0].x, -pElement->points[0].y ) );
        break;

    case kCGPathElementAddCurveToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[2].x, -pElement->points[2].y ) );
        rPolygon.setNextControlPoint( nPointCount - 1,
                                      basegfx::B2DPoint( pElement->points[0].x,
                                                         -pElement->points[0].y ) );
        rPolygon.setPrevControlPoint( nPointCount + 0,
                                      basegfx::B2DPoint( pElement->points[1].x,
                                                         -pElement->points[1].y ) );
        break;

    case kCGPathElementAddQuadCurveToPoint:
        {
            const basegfx::B2DPoint aStartPt = rPolygon.getB2DPoint( nPointCount-1 );
            const basegfx::B2DPoint aCtrPt1( (aStartPt.getX() + 2 * pElement->points[0].x) / 3.0,
                                             (aStartPt.getY() - 2 * pElement->points[0].y) / 3.0 );
            const basegfx::B2DPoint aCtrPt2( (+2 * pElement->points[0].x + pElement->points[1].x) / 3.0,
                                             (-2 * pElement->points[0].y - pElement->points[1].y) / 3.0 );
            rPolygon.append( basegfx::B2DPoint( +pElement->points[1].x, -pElement->points[1].y ) );
            rPolygon.setNextControlPoint( nPointCount-1, aCtrPt1 );
            rPolygon.setPrevControlPoint( nPointCount+0, aCtrPt2 );
        }
        break;
    }
}

bool CoreTextFont::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rResult, bool) const
{
    rResult.clear();

    CGGlyph nCGGlyph = nId;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const CTFontOrientation aFontOrientation = kCTFontDefaultOrientation;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    CGRect aCGRect = CTFontGetBoundingRectsForGlyphs(mpCTFont, aFontOrientation, &nCGGlyph, nullptr, 1);

    if (!CGRectIsNull(aCGRect) && CGRectIsEmpty(aCGRect))
    {
        // CTFontCreatePathForGlyph returns NULL for blank glyphs, but we want
        // to return true for them.
        return true;
    }

    CGPathRef xPath = CTFontCreatePathForGlyph(mpCTFont, nCGGlyph, nullptr);
    if (!xPath)
    {
        return false;
    }

    GgoData aGgoData;
    aGgoData.mpPolyPoly = &rResult;
    CGPathApply( xPath, static_cast<void*>(&aGgoData), MyCGPathApplierFunc );
#if 0 // TODO: does OSX ensure that the last polygon is always closed?
    const CGPathElement aClosingElement = { kCGPathElementCloseSubpath, NULL };
    MyCGPathApplierFunc( (void*)&aGgoData, &aClosingElement );
#endif
    CFRelease( xPath );

    return true;
}

hb_blob_t* CoreTextFontFace::GetHbTable(hb_tag_t nTag) const
{
    hb_blob_t* pBlob = nullptr;
    CTFontRef pFont = CTFontCreateWithFontDescriptor(mxFontDescriptor, 0.0, nullptr);

    if (!nTag)
    {
        // If nTag is 0, the whole font data is requested. CoreText does not
        // give us that, so we will construct an HarfBuzz face from CoreText
        // table data and return the blob of that face.

        auto pTags = CTFontCopyAvailableTables(pFont, kCTFontTableOptionNoOptions);
        CFIndex nTags = pTags ? CFArrayGetCount(pTags) : 0;
        if (nTags > 0)
        {
            hb_face_t* pHbFace = hb_face_builder_create();
            for (CFIndex i = 0; i < nTags; i++)
            {
                auto nTable = reinterpret_cast<intptr_t>(CFArrayGetValueAtIndex(pTags, i));
                assert(nTable);
                auto pTable = GetHbTable(nTable);
                assert(pTable);
                hb_face_builder_add_table(pHbFace, nTable, pTable);
            }
            pBlob = hb_face_reference_blob(pHbFace);

            hb_face_destroy(pHbFace);
        }
        if (pTags)
            CFRelease(pTags);
    }
    else
    {
        CFDataRef pData = CTFontCopyTable(pFont, nTag, kCTFontTableOptionNoOptions);
        const CFIndex nLength = pData ? CFDataGetLength(pData) : 0;
        if (nLength > 0)
        {
            auto pBuffer = new UInt8[nLength];
            const CFRange aRange = CFRangeMake(0, nLength);
            CFDataGetBytes(pData, aRange, pBuffer);

            pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength,
                                   HB_MEMORY_MODE_READONLY, pBuffer,
                                   [](void* data) { delete[] static_cast<UInt8*>(data); });
        }
        if (pData)
            CFRelease(pData);
    }

    CFRelease(pFont);
    return pBlob;
}

const std::vector<hb_variation_t>& CoreTextFontFace::GetVariations(const LogicalFontInstance&) const
{
    CTFontRef pFont = CTFontCreateWithFontDescriptor(mxFontDescriptor, 0.0, nullptr);

    if (!mxVariations)
    {
        mxVariations.emplace();
        CFArrayRef pAxes = CTFontCopyVariationAxes(pFont);
        if (pAxes)
        {
            CFDictionaryRef pVariations = CTFontCopyVariation(pFont);
            if (pVariations)
            {
                CFIndex nAxes = CFArrayGetCount(pAxes);
                for (CFIndex i = 0; i < nAxes; ++i)
                {
                    auto pAxis = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(pAxes, i));
                    if (pAxis)
                    {
                        hb_tag_t nTag;
                        auto pTag = static_cast<CFNumberRef>(
                            CFDictionaryGetValue(pAxis, kCTFontVariationAxisIdentifierKey));
                        if (!pTag)
                            continue;
                        CFNumberGetValue(pTag, kCFNumberIntType, &nTag);

                        float fValue;
                        auto pValue
                            = static_cast<CFNumberRef>(CFDictionaryGetValue(pVariations, pTag));
                        if (!pValue)
                            continue;
                        CFNumberGetValue(pValue, kCFNumberFloatType, &fValue);

                        mxVariations->push_back({ nTag, fValue });
                    }
                }
                CFRelease(pVariations);
            }
            CFRelease(pAxes);
        }
    }

    return *mxVariations;
}

rtl::Reference<LogicalFontInstance> CoreTextFontFace::CreateFontInstance(const vcl::font::FontSelectPattern& rFSD) const
{
    return new CoreTextFont(*this, rFSD);
}

FontAttributes DevFontFromCTFontDescriptor( CTFontDescriptorRef pFD, bool* bFontEnabled )
{
    // all CoreText fonts are device fonts that can rotate just fine
    FontAttributes rDFA;
    rDFA.SetQuality( 0 );

    // reset the font attributes
    rDFA.SetFamilyType( FAMILY_DONTKNOW );
    rDFA.SetPitch( PITCH_VARIABLE );
    rDFA.SetWidthType( WIDTH_NORMAL );
    rDFA.SetWeight( WEIGHT_NORMAL );
    rDFA.SetItalic( ITALIC_NONE );
    rDFA.SetMicrosoftSymbolEncoded( false );

    // get font name
#ifdef MACOSX
    CFStringRef pLang = nullptr;
    CFStringRef pFamilyName = static_cast<CFStringRef>(
            CTFontDescriptorCopyLocalizedAttribute( pFD, kCTFontFamilyNameAttribute, &pLang ));

    if ( !pLang )
    {
        if( pFamilyName )
        {
            CFRelease( pFamilyName );
        }
        pFamilyName = static_cast<CFStringRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute ));
    }
#else
    // No "Application" on iOS. And it is unclear whether this code
    // snippet will actually ever get invoked on iOS anyway. So just
    // use the old code that uses a non-localized font name.
    CFStringRef pFamilyName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute );
#endif

    rDFA.SetFamilyName( GetOUString( pFamilyName ) );

    // get font style
    CFStringRef pStyleName = static_cast<CFStringRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontStyleNameAttribute ));
    rDFA.SetStyleName( GetOUString( pStyleName ) );

    // get font-enabled status
    if( bFontEnabled )
    {
        int bEnabled = TRUE; // by default (and when we're on macOS < 10.6) it's "enabled"
        CFNumberRef pEnabled = static_cast<CFNumberRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontEnabledAttribute ));
        CFNumberGetValue( pEnabled, kCFNumberIntType, &bEnabled );
        *bFontEnabled = bEnabled;
    }

    // get font attributes
    CFDictionaryRef pAttrDict = static_cast<CFDictionaryRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontTraitsAttribute ));

    if (bFontEnabled && *bFontEnabled)
    {
        // Ignore font formats not supported.
        int nFormat;
        CFNumberRef pFormat = static_cast<CFNumberRef>(CTFontDescriptorCopyAttribute(pFD, kCTFontFormatAttribute));
        CFNumberGetValue(pFormat, kCFNumberIntType, &nFormat);
        if (nFormat == kCTFontFormatUnrecognized || nFormat == kCTFontFormatPostScript || nFormat == kCTFontFormatBitmap)
        {
            SAL_INFO("vcl.fonts", "Ignoring font with unsupported format: " << rDFA.GetFamilyName());
            *bFontEnabled = false;
        }
        CFRelease(pFormat);
    }

    // get symbolic trait
    // TODO: use other traits such as MonoSpace/Condensed/Expanded or Vertical too
    SInt64 nSymbolTrait = 0;
    CFNumberRef pSymbolNum = nullptr;
    if( CFDictionaryGetValueIfPresent( pAttrDict, kCTFontSymbolicTrait, reinterpret_cast<const void**>(&pSymbolNum) ) )
    {
        CFNumberGetValue( pSymbolNum, kCFNumberSInt64Type, &nSymbolTrait );
        if (nSymbolTrait & kCTFontMonoSpaceTrait)
            rDFA.SetPitch(PITCH_FIXED);
    }

    // get the font weight
    double fWeight = 0;
    CFNumberRef pWeightNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontWeightTrait ));
    CFNumberGetValue( pWeightNum, kCFNumberDoubleType, &fWeight );
    int nInt = WEIGHT_NORMAL;

    // Special case fixes

    // tdf#67744: Courier Std Medium is always bold. We get a kCTFontWeightTrait of 0.23 which
    // surely must be wrong.
    if (rDFA.GetFamilyName() == "Courier Std" &&
        (rDFA.GetStyleName() == "Medium" || rDFA.GetStyleName() == "Medium Oblique") &&
        fWeight > 0.2)
    {
        fWeight = 0;
    }

    // tdf#68889: Ditto for Gill Sans MT Pro. Here I can kinda understand it, maybe the
    // kCTFontWeightTrait is intended to give a subjective "optical" impression of how the font
    // looks, and Gill Sans MT Pro Medium is kinda heavy. But with the way LibreOffice uses fonts,
    // we still should think of it as being "medium" weight.
    if (rDFA.GetFamilyName() == "Gill Sans MT Pro" &&
        (rDFA.GetStyleName() == "Medium" || rDFA.GetStyleName() == "Medium Italic") &&
        fWeight > 0.2)
    {
        fWeight = 0;
    }

    if( fWeight > 0 )
    {
        nInt = rint(int(WEIGHT_NORMAL) + fWeight * ((WEIGHT_BLACK - WEIGHT_NORMAL)/0.68));
        if( nInt > WEIGHT_BLACK )
        {
            nInt = WEIGHT_BLACK;
        }
    }
    else if( fWeight < 0 )
    {
        nInt = rint(int(WEIGHT_NORMAL) + fWeight * ((WEIGHT_NORMAL - WEIGHT_THIN)/0.8));
        if( nInt < WEIGHT_THIN )
        {
            nInt = WEIGHT_THIN;
        }
    }
    rDFA.SetWeight( static_cast<FontWeight>(nInt) );

    // get the font slant
    double fSlant = 0;
    CFNumberRef pSlantNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontSlantTrait ));
    CFNumberGetValue( pSlantNum, kCFNumberDoubleType, &fSlant );
    if( fSlant >= 0.035 )
    {
        rDFA.SetItalic( ITALIC_NORMAL );
    }
    // get width trait
    double fWidth = 0;
    CFNumberRef pWidthNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontWidthTrait ));
    CFNumberGetValue( pWidthNum, kCFNumberDoubleType, &fWidth );
    nInt = WIDTH_NORMAL;

    if( fWidth > 0 )
    {
        nInt = rint( int(WIDTH_NORMAL) + fWidth * ((WIDTH_ULTRA_EXPANDED - WIDTH_NORMAL)/0.4));
        if( nInt > WIDTH_ULTRA_EXPANDED )
        {
            nInt = WIDTH_ULTRA_EXPANDED;
        }
    }
    else if( fWidth < 0 )
    {
        nInt = rint( int(WIDTH_NORMAL) + fWidth * ((WIDTH_NORMAL - WIDTH_ULTRA_CONDENSED)/0.5));
        if( nInt < WIDTH_ULTRA_CONDENSED )
        {
            nInt = WIDTH_ULTRA_CONDENSED;
        }
    }
    rDFA.SetWidthType( static_cast<FontWidth>(nInt) );

    // release the attribute dict that we had copied
    CFRelease( pAttrDict );

    // TODO? also use the HEAD table if available to get more attributes
//  CFDataRef CTFontCopyTable( CTFontRef, kCTFontTableHead, /*kCTFontTableOptionNoOptions*/kCTFontTableOptionExcludeSynthetic );

    return rDFA;
}

static void fontEnumCallBack( const void* pValue, void* pContext )
{
    CTFontDescriptorRef pFD = static_cast<CTFontDescriptorRef>(pValue);

    bool bFontEnabled;
    FontAttributes rDFA = DevFontFromCTFontDescriptor( pFD, &bFontEnabled );

    if( bFontEnabled)
    {
        rtl::Reference<CoreTextFontFace> pFontData = new CoreTextFontFace( rDFA, pFD );
        SystemFontList* pFontList = static_cast<SystemFontList*>(pContext);
        pFontList->AddFont( pFontData.get() );
    }
}

SystemFontList::SystemFontList()
  : mpCTFontCollection( nullptr )
  , mpCTFontArray( nullptr )
{}

SystemFontList::~SystemFontList()
{
    maFontContainer.clear();

    if( mpCTFontArray )
    {
        CFRelease( mpCTFontArray );
    }
    if( mpCTFontCollection )
    {
        CFRelease( mpCTFontCollection );
    }
}

void SystemFontList::AddFont( CoreTextFontFace* pFontData )
{
    sal_IntPtr nFontId = pFontData->GetFontId();
    maFontContainer[ nFontId ] = pFontData;
}

void SystemFontList::AnnounceFonts( vcl::font::PhysicalFontCollection& rFontCollection ) const
{
    for(const auto& rEntry : maFontContainer )
    {
        rFontCollection.Add( rEntry.second.get() );
    }
}

CoreTextFontFace* SystemFontList::GetFontDataFromId( sal_IntPtr nFontId ) const
{
    auto it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
    {
        return nullptr;
    }
    return (*it).second.get();
}

bool SystemFontList::Init()
{
    // enumerate available system fonts
    static const int nMaxDictEntries = 8;
    CFMutableDictionaryRef pCFDict = CFDictionaryCreateMutable( nullptr,
                                                                nMaxDictEntries,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks );

    CFDictionaryAddValue( pCFDict, kCTFontCollectionRemoveDuplicatesOption, kCFBooleanTrue );
    mpCTFontCollection = CTFontCollectionCreateFromAvailableFonts( pCFDict );
    CFRelease( pCFDict );
    mpCTFontArray = CTFontCollectionCreateMatchingFontDescriptors( mpCTFontCollection );

    const int nFontCount = CFArrayGetCount( mpCTFontArray );
    const CFRange aFullRange = CFRangeMake( 0, nFontCount );
    CFArrayApplyFunction( mpCTFontArray, aFullRange, fontEnumCallBack, this );

    return true;
}

std::unique_ptr<SystemFontList> GetCoretextFontList()
{
    std::unique_ptr<SystemFontList> pList(new SystemFontList());
    if( !pList->Init() )
    {
        return nullptr;
    }

    return pList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
