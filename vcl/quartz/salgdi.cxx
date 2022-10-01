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
#include <sal/log.hxx>
#include <config_folders.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/long.hxx>
#include <comphelper/lok.hxx>

#include <vcl/metric.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#include <quartz/ctfonts.hxx>
#include <fontsubset.hxx>
#include <impfont.hxx>
#include <impfontmetricdata.hxx>
#include <font/fontsubstitution.hxx>
#include <font/PhysicalFontCollection.hxx>

#ifdef MACOSX
#include <osx/salframe.h>
#endif
#include <quartz/utils.h>
#ifdef IOS
#include <svdata.hxx>
#endif
#include <sallayout.hxx>

#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#if HAVE_FEATURE_SKIA
#include <skia/osx/gdiimpl.hxx>
#endif

using namespace vcl;

namespace {

class CoreTextGlyphFallbackSubstititution
:    public vcl::font::GlyphFallbackFontSubstitution
{
public:
    bool FindFontSubstitute(vcl::font::FontSelectPattern&, LogicalFontInstance* pLogicalFont, OUString&) const override;
};

bool FontHasCharacter(CTFontRef pFont, const OUString& rString, sal_Int32 nIndex, sal_Int32 nLen)
{
    CGGlyph glyphs[nLen];
    return CTFontGetGlyphsForCharacters(pFont, reinterpret_cast<const UniChar*>(rString.getStr() + nIndex), glyphs, nLen);
}

}

bool CoreTextGlyphFallbackSubstititution::FindFontSubstitute(vcl::font::FontSelectPattern& rPattern, LogicalFontInstance* pLogicalFont,
    OUString& rMissingChars) const
{
    bool bFound = false;
    CoreTextStyle* pStyle = static_cast<CoreTextStyle*>(pLogicalFont);
    CTFontRef pFont = static_cast<CTFontRef>(CFDictionaryGetValue(pStyle->GetStyleDict(), kCTFontAttributeName));
    CFStringRef pStr = CreateCFString(rMissingChars);
    if (pStr)
    {
        CTFontRef pFallback = CTFontCreateForString(pFont, pStr, CFRangeMake(0, CFStringGetLength(pStr)));
        if (pFallback)
        {
            bFound = true;

            // tdf#148470 remove the resolved chars from rMissing to flag which ones are still missing
            // for an attempt with another font
            OUStringBuffer aStillMissingChars;
            for (sal_Int32 nStrIndex = 0; nStrIndex < rMissingChars.getLength();)
            {
                sal_Int32 nOldStrIndex = nStrIndex;
                rMissingChars.iterateCodePoints(&nStrIndex);
                sal_Int32 nCharLength = nStrIndex - nOldStrIndex;
                if (!FontHasCharacter(pFallback, rMissingChars, nOldStrIndex, nCharLength))
                    aStillMissingChars.append(rMissingChars.getStr() + nOldStrIndex, nCharLength);
            }
            rMissingChars = aStillMissingChars.toString();

            CTFontDescriptorRef pDesc = CTFontCopyFontDescriptor(pFallback);
            FontAttributes rAttr = DevFontFromCTFontDescriptor(pDesc, nullptr);

            rPattern.maSearchName = rAttr.GetFamilyName();

            rPattern.SetWeight(rAttr.GetWeight());
            rPattern.SetItalic(rAttr.GetItalic());
            rPattern.SetPitch(rAttr.GetPitch());
            rPattern.SetWidthType(rAttr.GetWidthType());

            CFRelease(pFallback);
            CFRelease(pDesc);
        }
        CFRelease(pStr);
    }

    return bFound;
}

CoreTextFontFace::CoreTextFontFace( const FontAttributes& rDFA, sal_IntPtr nFontId )
  : vcl::font::PhysicalFontFace( rDFA )
  , mnFontId( nFontId )
{
}

CoreTextFontFace::~CoreTextFontFace()
{
}

sal_IntPtr CoreTextFontFace::GetFontId() const
{
    return mnFontId;
}

AquaSalGraphics::AquaSalGraphics()
    : mnRealDPIX( 0 )
    , mnRealDPIY( 0 )
{
    SAL_INFO( "vcl.quartz", "AquaSalGraphics::AquaSalGraphics() this=" << this );

#if HAVE_FEATURE_SKIA
    if(SkiaHelper::isVCLSkiaEnabled())
        mpBackend.reset(new AquaSkiaSalGraphicsImpl(*this, maShared));
#else
    if(false)
        ;
#endif
    else
        mpBackend.reset(new AquaGraphicsBackend(maShared));

    for (int i = 0; i < MAX_FALLBACK; ++i)
        mpTextStyle[i] = nullptr;

    if (comphelper::LibreOfficeKit::isActive())
        initWidgetDrawBackends(true);
}

AquaSalGraphics::~AquaSalGraphics()
{
    SAL_INFO( "vcl.quartz", "AquaSalGraphics::~AquaSalGraphics() this=" << this );

    maShared.unsetClipPath();

    ReleaseFonts();

    maShared.mpXorEmulation.reset();

#ifdef IOS
    if (maShared.mbForeignContext)
        return;
#endif
    if (maShared.maLayer.isSet())
    {
        CGLayerRelease(maShared.maLayer.get());
    }
    else if (maShared.maContextHolder.isSet()
#ifdef MACOSX
             && maShared.mbWindow
#endif
             )
    {
        // destroy backbuffer bitmap context that we created ourself
        CGContextRelease(maShared.maContextHolder.get());
        maShared.maContextHolder.set(nullptr);
    }
}

SalGraphicsImpl* AquaSalGraphics::GetImpl() const
{
    return mpBackend->GetImpl();
}

void AquaSalGraphics::SetTextColor( Color nColor )
{
    maShared.maTextColor = nColor;
}

void AquaSalGraphics::GetFontMetric(ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel)
{
    if (nFallbackLevel < MAX_FALLBACK && mpTextStyle[nFallbackLevel])
    {
        mpTextStyle[nFallbackLevel]->GetFontMetric(rxFontMetric);
    }
}

static bool AddTempDevFont(const OUString& rFontFileURL)
{
    OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSystemPath ) );
    OString aCFileName = OUStringToOString( aUSystemPath, RTL_TEXTENCODING_UTF8 );

    CFStringRef rFontPath = CFStringCreateWithCString(nullptr, aCFileName.getStr(), kCFStringEncodingUTF8);
    CFURLRef rFontURL = CFURLCreateWithFileSystemPath(nullptr, rFontPath, kCFURLPOSIXPathStyle, true);

    CFErrorRef error;
    bool success = CTFontManagerRegisterFontsForURL(rFontURL, kCTFontManagerScopeProcess, &error);
    if (!success)
    {
        CFRelease(error);
    }
    CFRelease(rFontPath);
    CFRelease(rFontURL);

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

    // internal font resources, required for normal operation, like OpenSymbol
    AddTempFontDir( aBrandStr + "/" LIBO_SHARE_RESOURCE_FOLDER "/common/fonts/" );

    AddTempFontDir( aBrandStr + "/" LIBO_SHARE_FOLDER "/fonts/truetype/" );
}

void AquaSalGraphics::GetDevFontList(vcl::font::PhysicalFontCollection* pFontCollection)
{
    SAL_WARN_IF( !pFontCollection, "vcl", "AquaSalGraphics::GetDevFontList(NULL) !");

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

    static CoreTextGlyphFallbackSubstititution aSubstFallback;
    pFontCollection->SetFallbackHook(&aSubstFallback);
}

void AquaSalGraphics::ClearDevFontCache()
{
    SalData* pSalData = GetSalData();
    delete pSalData->mpFontList;
    pSalData->mpFontList = nullptr;
}

bool AquaSalGraphics::AddTempDevFont(vcl::font::PhysicalFontCollection*,
    const OUString& rFontFileURL, const OUString& /*rFontName*/)
{
    return ::AddTempDevFont(rFontFileURL);
}

void AquaSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    mpBackend->drawTextLayout(rLayout, rLayout.GetTextRenderModeForResolutionIndependentLayout());
}

void AquaGraphicsBackend::drawTextLayout(const GenericSalLayout& rLayout, bool bTextRenderModeForResolutionIndependentLayout)
{
#ifdef IOS
    if (!mrShared.checkContext())
    {
        SAL_WARN("vcl.quartz", "AquaSalGraphics::DrawTextLayout() without context");
        return;
    }
#endif

    const CoreTextStyle& rStyle = *static_cast<const CoreTextStyle*>(&rLayout.GetFont());
    const vcl::font::FontSelectPattern& rFontSelect = rStyle.GetFontSelectPattern();
    if (rFontSelect.mnHeight == 0)
    {
        SAL_WARN("vcl.quartz", "AquaSalGraphics::DrawTextLayout(): rFontSelect.mnHeight is zero!?");
        return;
    }

    CTFontRef pFont = static_cast<CTFontRef>(CFDictionaryGetValue(rStyle.GetStyleDict(), kCTFontAttributeName));
    CGAffineTransform aRotMatrix = CGAffineTransformMakeRotation(-rStyle.mfFontRotation);

    DevicePoint aPos;
    const GlyphItem* pGlyph;
    std::vector<CGGlyph> aGlyphIds;
    std::vector<CGPoint> aGlyphPos;
    std::vector<bool> aGlyphOrientation;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        CGPoint aGCPos = CGPointMake(aPos.getX(), -aPos.getY());

        // Whether the glyph should be upright in vertical mode or not
        bool bUprightGlyph = false;

        if (rStyle.mfFontRotation)
        {
            if (pGlyph->IsVertical())
            {
                bUprightGlyph = true;
                // Adjust the position of upright (vertical) glyphs.
                aGCPos.y -= CTFontGetAscent(pFont) - CTFontGetDescent(pFont);
            }
            else
            {
                // Transform the position of rotated glyphs.
                aGCPos = CGPointApplyAffineTransform(aGCPos, aRotMatrix);
            }
        }

        aGlyphIds.push_back(pGlyph->glyphId());
        aGlyphPos.push_back(aGCPos);
        aGlyphOrientation.push_back(bUprightGlyph);
    }

    if (aGlyphIds.empty())
        return;

    assert(aGlyphIds.size() == aGlyphPos.size());
#if 0
    std::cerr << "aGlyphIds:[";
    for (unsigned i = 0; i < aGlyphIds.size(); i++)
    {
        if (i > 0)
            std::cerr << ",";
        std::cerr << aGlyphIds[i];
    }
    std::cerr << "]\n";
    std::cerr << "aGlyphPos:[";
    for (unsigned i = 0; i < aGlyphPos.size(); i++)
    {
        if (i > 0)
            std::cerr << ",";
        std::cerr << aGlyphPos[i];
    }
    std::cerr << "]\n";
#endif

    mrShared.maContextHolder.saveState();
    RGBAColor textColor( mrShared.maTextColor );

    // The view is vertically flipped (no idea why), flip it back.
    CGContextScaleCTM(mrShared.maContextHolder.get(), 1.0, -1.0);
    CGContextSetShouldAntialias(mrShared.maContextHolder.get(), !mrShared.mbNonAntialiasedText);
    CGContextSetFillColor(mrShared.maContextHolder.get(), textColor.AsArray());

    if (rStyle.mbFauxBold)
    {

        float fSize = rFontSelect.mnHeight / 23.0f;
        CGContextSetStrokeColor(mrShared.maContextHolder.get(), textColor.AsArray());
        CGContextSetLineWidth(mrShared.maContextHolder.get(), fSize);
        CGContextSetTextDrawingMode(mrShared.maContextHolder.get(), kCGTextFillStroke);
    }

    if (bTextRenderModeForResolutionIndependentLayout)
    {
        CGContextSetAllowsFontSubpixelQuantization(mrShared.maContextHolder.get(), false);
        CGContextSetShouldSubpixelQuantizeFonts(mrShared.maContextHolder.get(), false);
        CGContextSetAllowsFontSubpixelPositioning(mrShared.maContextHolder.get(), true);
        CGContextSetShouldSubpixelPositionFonts(mrShared.maContextHolder.get(), true);
    }

    auto aIt = aGlyphOrientation.cbegin();
    while (aIt != aGlyphOrientation.cend())
    {
        bool bUprightGlyph = *aIt;
        // Find the boundary of the run of glyphs with the same rotation, to be
        // drawn together.
        auto aNext = std::find(aIt, aGlyphOrientation.cend(), !bUprightGlyph);
        size_t nStartIndex = std::distance(aGlyphOrientation.cbegin(), aIt);
        size_t nLen = std::distance(aIt, aNext);

        mrShared.maContextHolder.saveState();
        if (rStyle.mfFontRotation && !bUprightGlyph)
        {
            CGContextRotateCTM(mrShared.maContextHolder.get(), rStyle.mfFontRotation);
        }
        CTFontDrawGlyphs(pFont, &aGlyphIds[nStartIndex], &aGlyphPos[nStartIndex], nLen, mrShared.maContextHolder.get());
        mrShared.maContextHolder.restoreState();

        aIt = aNext;
    }

    mrShared.maContextHolder.restoreState();
}

void AquaSalGraphics::SetFont(LogicalFontInstance* pReqFont, int nFallbackLevel)
{
    // release the text style
    for (int i = nFallbackLevel; i < MAX_FALLBACK; ++i)
    {
        if (!mpTextStyle[i])
            break;
        mpTextStyle[i].clear();
    }

    if (!pReqFont)
        return;

    // update the text style
    mpTextStyle[nFallbackLevel] = static_cast<CoreTextStyle*>(pReqFont);
}

std::unique_ptr<GenericSalLayout> AquaSalGraphics::GetTextLayout(int nFallbackLevel)
{
    assert(mpTextStyle[nFallbackLevel]);
    if (!mpTextStyle[nFallbackLevel])
        return nullptr;
    return std::make_unique<GenericSalLayout>(*mpTextStyle[nFallbackLevel]);
}

FontCharMapRef AquaSalGraphics::GetFontCharMap() const
{
    if (!mpTextStyle[0])
    {
        return FontCharMapRef( new FontCharMap() );
    }

    return mpTextStyle[0]->GetFontFace()->GetFontCharMap();
}

bool AquaSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!mpTextStyle[0])
        return false;

    return mpTextStyle[0]->GetFontFace()->GetFontCapabilities(rFontCapabilities);
}

void AquaSalGraphics::Flush()
{
    mpBackend->Flush();
}

void AquaSalGraphics::Flush( const tools::Rectangle& rRect )
{
    mpBackend->Flush( rRect );
}

#ifdef IOS

bool AquaSharedAttributes::checkContext()
{
    if (mbForeignContext)
    {
        SAL_INFO("vcl.ios", "CheckContext() this=" << this << ", mbForeignContext, return true");
        return true;
    }

    SAL_INFO( "vcl.ios", "CheckContext() this=" << this << ",  not foreign, return false");
    return false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
