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

#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>

#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/LogicalFontInstance.hxx>
#include <font/FontMetricData.hxx>
#include <sft.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>
#include <hb-ot.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

FontMetric::FontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnBulletOffset( 0 ),
    mnHangingBaseline( 0 ),
    mbFullstopCentered( false )
{}

FontMetric::FontMetric( const FontMetric& rFontMetric ) = default;

FontMetric::FontMetric(vcl::font::PhysicalFontFace const& rFace)
    : FontMetric()
{
    SetFamilyName(rFace.GetFamilyName());
    SetStyleName(rFace.GetStyleName());
    SetCharSet(rFace.IsMicrosoftSymbolEncoded() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
    SetFamily(rFace.GetFamilyType());
    SetPitch(rFace.GetPitch());
    SetWeight(rFace.GetWeight());
    SetItalic(rFace.GetItalic());
    SetAlignment(TextAlign::ALIGN_TOP);
    SetWidthType(rFace.GetWidthType());
    SetQuality(rFace.GetQuality() );
}

FontMetric::~FontMetric()
{
}

FontMetric& FontMetric::operator=(const FontMetric& rFontMetric) = default;

FontMetric& FontMetric::operator=(FontMetric&& rFontMetric) = default;

bool FontMetric::EqualNoBase( const FontMetric& r ) const
{
    if (mbFullstopCentered != r.mbFullstopCentered)
        return false;
    if( mnAscent     != r.mnAscent )
        return false;
    if( mnDescent    != r.mnDescent )
        return false;
    if( mnIntLeading != r.mnIntLeading )
        return false;
    if( mnExtLeading != r.mnExtLeading )
        return false;
    if( mnSlant      != r.mnSlant )
        return false;

    return true;
}

bool FontMetric::operator==( const FontMetric& r ) const
{
    if( Font::operator!=(r) )
        return false;
    return EqualNoBase(r);
}

bool FontMetric::EqualIgnoreColor( const FontMetric& r ) const
{
    if( !Font::EqualIgnoreColor(r) )
        return false;
    return EqualNoBase(r);
}

size_t FontMetric::GetHashValueNoBase() const
{
    size_t hash = 0;
    o3tl::hash_combine( hash, mbFullstopCentered );
    o3tl::hash_combine( hash, mnAscent );
    o3tl::hash_combine( hash, mnDescent );
    o3tl::hash_combine( hash, mnIntLeading );
    o3tl::hash_combine( hash, mnExtLeading );
    o3tl::hash_combine( hash, mnSlant );
    return hash;
}

size_t FontMetric::GetHashValueIgnoreColor() const
{
    size_t hash = GetHashValueNoBase();
    o3tl::hash_combine( hash, Font::GetHashValueIgnoreColor());
    return hash;
}

FontMetricData::FontMetricData( const vcl::font::FontSelectPattern& rFontSelData )
    : FontAttributes( rFontSelData )
    , mnHeight ( rFontSelData.mnHeight )
    , mnWidth ( rFontSelData.mnWidth )
    , mnOrientation( static_cast<short>(rFontSelData.mnOrientation) )
    , mnAscent( 0 )
    , mnDescent( 0 )
    , mnIntLeading( 0 )
    , mnExtLeading( 0 )
    , mnSlant( 0 )
    , mnMinKashida( 0 )
    , mnHangingBaseline( 0 )
    , mbFullstopCentered( false )
    , mnBulletOffset( 0 )
    , mnUnderlineSize( 0 )
    , mnUnderlineOffset( 0 )
    , mnBUnderlineSize( 0 )
    , mnBUnderlineOffset( 0 )
    , mnDUnderlineSize( 0 )
    , mnDUnderlineOffset1( 0 )
    , mnDUnderlineOffset2( 0 )
    , mnWUnderlineSize( 0 )
    , mnWUnderlineOffset( 0 )
    , mnAboveUnderlineSize( 0 )
    , mnAboveUnderlineOffset( 0 )
    , mnAboveBUnderlineSize( 0 )
    , mnAboveBUnderlineOffset( 0 )
    , mnAboveDUnderlineSize( 0 )
    , mnAboveDUnderlineOffset1( 0 )
    , mnAboveDUnderlineOffset2( 0 )
    , mnAboveWUnderlineSize( 0 )
    , mnAboveWUnderlineOffset( 0 )
    , mnStrikeoutSize( 0 )
    , mnStrikeoutOffset( 0 )
    , mnBStrikeoutSize( 0 )
    , mnBStrikeoutOffset( 0 )
    , mnDStrikeoutSize( 0 )
    , mnDStrikeoutOffset1( 0 )
    , mnDStrikeoutOffset2( 0 )
{
    // initialize the used font name
    sal_Int32 nTokenPos = 0;
    SetFamilyName( OUString(GetNextFontToken( rFontSelData.GetFamilyName(), nTokenPos )) );
    SetStyleName( rFontSelData.GetStyleName() );
}

bool FontMetricData::ShouldNotUseUnderlineMetrics() const
{
    if (comphelper::IsFuzzing())
        return false;

    css::uno::Sequence<OUString> rNoUnderlineMetricsList(
        officecfg::Office::Common::Misc::FontsDontUseUnderlineMetrics::get());
    if (comphelper::findValue(rNoUnderlineMetricsList, GetFamilyName()) != -1)
    {
        SAL_INFO("vcl.gdi.fontmetric", "Not using underline metrics for: " << GetFamilyName());
        return true;
    }
    return false;
}

bool FontMetricData::ImplInitTextLineSizeHarfBuzz(LogicalFontInstance* pFont)
{
    if (ShouldNotUseUnderlineMetrics())
        return false;

    auto* pHbFont = pFont->GetHbFont();

    hb_position_t nUnderlineSize;
    if (!hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_UNDERLINE_SIZE, &nUnderlineSize))
        return false;
    hb_position_t nUnderlineOffset;
    if (!hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &nUnderlineOffset))
        return false;
    hb_position_t nStrikeoutSize;
    if (!hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_STRIKEOUT_SIZE, &nStrikeoutSize))
        return false;
    hb_position_t nStrikeoutOffset;
    if (!hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_STRIKEOUT_OFFSET, &nStrikeoutOffset))
        return false;

    double fScale = 0;
    pFont->GetScale(nullptr, &fScale);

    double nOffset = -nUnderlineOffset * fScale;
    double nSize = nUnderlineSize * fScale;
    double nSize2 = nSize / 2.;
    double nBSize = nSize * 2.;
    double n2Size = nBSize / 3.;

    mnUnderlineSize = std::ceil(nSize);
    mnUnderlineOffset = std::ceil(nOffset);

    mnBUnderlineSize = std::ceil(nBSize);
    mnBUnderlineOffset = std::ceil(nOffset - nSize2);

    mnDUnderlineSize = std::ceil(n2Size);
    mnDUnderlineOffset1 = mnBUnderlineOffset;
    mnDUnderlineOffset2 = mnBUnderlineOffset + mnDUnderlineSize * 2;

    mnWUnderlineSize = mnBUnderlineSize;
    mnWUnderlineOffset = std::ceil(nOffset + nSize);

    nOffset = -nStrikeoutOffset * fScale;
    nSize = nStrikeoutSize * fScale;
    nSize2 = nSize / 2.;
    nBSize = nSize * 2.;
    n2Size = nBSize / 3.;

    mnStrikeoutSize = std::ceil(nSize);
    mnStrikeoutOffset = std::ceil(nOffset);

    mnBStrikeoutSize = std::ceil(nBSize);
    mnBStrikeoutOffset = std::round(nOffset - nSize2);

    mnDStrikeoutSize = std::ceil(n2Size);
    mnDStrikeoutOffset1 = mnBStrikeoutOffset;
    mnDStrikeoutOffset2 = mnBStrikeoutOffset + mnDStrikeoutSize * 2;

    return true;
}

void FontMetricData::ImplInitTextLineSize( const OutputDevice* pDev )
{
    mnBulletOffset = ( pDev->GetTextWidth( OUString( u' ' ) ) - pDev->GetTextWidth( OUString( u'\x00b7' ) ) ) >> 1 ;

    if (ImplInitTextLineSizeHarfBuzz(const_cast<LogicalFontInstance*>(pDev->GetFontInstance())))
        return;

    tools::Long nDescent = mnDescent;
    if ( nDescent <= 0 )
    {
        nDescent = mnAscent / 10;
        if ( !nDescent )
            nDescent = 1;
    }

    // #i55341# for some fonts it is not a good idea to calculate
    // their text line metrics from the real font descent
    // => work around this problem just for these fonts
    if( 3*nDescent > mnAscent )
        nDescent = mnAscent / 3;

    tools::Long nLineHeight = ((nDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    tools::Long nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    tools::Long nBLineHeight = ((nDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    tools::Long nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    tools::Long n2LineHeight = ((nDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    tools::Long n2LineDY = n2LineHeight;
     /* #117909#
      * add some pixels to minimum double line distance on higher resolution devices
      */
    tools::Long nMin2LineDY = 1 + pDev->GetDPIY()/150;
    if ( n2LineDY < nMin2LineDY )
        n2LineDY = nMin2LineDY;
    tools::Long n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    const vcl::Font& rFont ( pDev->GetFont() );
    bool bCJKVertical = MsLangId::isCJK(rFont.GetLanguage()) && rFont.IsVertical();
    tools::Long nUnderlineOffset = bCJKVertical ? mnDescent : (mnDescent/2 + 1);
    tools::Long nStrikeoutOffset = rFont.IsVertical() ? -((mnAscent - mnDescent) / 2) : -((mnAscent - mnIntLeading) / 3);

    mnUnderlineSize        = nLineHeight;
    mnUnderlineOffset      = nUnderlineOffset - nLineHeight2;

    mnBUnderlineSize       = nBLineHeight;
    mnBUnderlineOffset     = nUnderlineOffset - nBLineHeight2;

    mnDUnderlineSize       = n2LineHeight;
    mnDUnderlineOffset1    = nUnderlineOffset - n2LineDY2 - n2LineHeight;
    mnDUnderlineOffset2    = mnDUnderlineOffset1 + n2LineDY + n2LineHeight;

    tools::Long nWCalcSize = mnDescent;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnWUnderlineSize = nWCalcSize;
        else
            mnWUnderlineSize = 3;
    }
    else
        mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;


    // Don't assume that wavelines are never placed below the descent, because for most fonts the waveline
    // is drawn into the text
    mnWUnderlineOffset     = nUnderlineOffset;

    mnStrikeoutSize        = nLineHeight;
    mnStrikeoutOffset      = nStrikeoutOffset - nLineHeight2;

    mnBStrikeoutSize       = nBLineHeight;
    mnBStrikeoutOffset     = nStrikeoutOffset - nBLineHeight2;

    mnDStrikeoutSize       = n2LineHeight;
    mnDStrikeoutOffset1    = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
    mnDStrikeoutOffset2    = mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;

}


void FontMetricData::ImplInitAboveTextLineSize( const OutputDevice* pDev )
{
    ImplInitTextLineSize(pDev);

    tools::Long nIntLeading = mnIntLeading;
    // TODO: assess usage of nLeading below (changed in extleading CWS)
    // if no leading is available, we assume 15% of the ascent
    if ( nIntLeading <= 0 )
    {
        nIntLeading = mnAscent*15/100;
        if ( !nIntLeading )
            nIntLeading = 1;
    }

    tools::Long nCeiling = -mnAscent;

    mnAboveUnderlineSize       = mnUnderlineSize;
    mnAboveUnderlineOffset     = nCeiling + (nIntLeading - mnUnderlineSize + 1) / 2;

    mnAboveBUnderlineSize      = mnBUnderlineSize;
    mnAboveBUnderlineOffset    = nCeiling + (nIntLeading - mnBUnderlineSize + 1) / 2;

    mnAboveDUnderlineSize      = mnDUnderlineSize;
    mnAboveDUnderlineOffset1   = nCeiling + (nIntLeading - 3*mnDUnderlineSize + 1) / 2;
    mnAboveDUnderlineOffset2   = nCeiling + (nIntLeading +   mnDUnderlineSize + 1) / 2;

    mnAboveWUnderlineSize = mnWUnderlineSize;
    mnAboveWUnderlineOffset = nCeiling + (nIntLeading + 1) / 2;
}

void FontMetricData::ImplInitFlags( const OutputDevice* pDev )
{
    const vcl::Font& rFont ( pDev->GetFont() );
    bool bCentered = true;
    if (MsLangId::isCJK(rFont.GetLanguage()))
    {
        tools::Rectangle aRect;
        pDev->GetTextBoundRect( aRect, u"\x3001"_ustr ); // Fullwidth fullstop
        const auto nH = rFont.GetFontSize().Height();
        const auto nB = aRect.Left();
        // Use 18.75% as a threshold to define a centered fullwidth fullstop.
        // In general, nB/nH < 5% for most Japanese fonts.
        bCentered = nB > (((nH >> 1)+nH)>>3);
    }
    SetFullstopCenteredFlag( bCentered );
}

bool FontMetricData::ShouldUseWinMetrics(int nAscent, int nDescent, int nTypoAscent,
                                             int nTypoDescent, int nWinAscent,
                                             int nWinDescent) const
{
    if (comphelper::IsFuzzing())
        return false;

    OUString aFontIdentifier(
        GetFamilyName() + ","
        + OUString::number(nAscent) + "," + OUString::number(nDescent) + ","
        + OUString::number(nTypoAscent) + "," + OUString::number(nTypoDescent) + ","
        + OUString::number(nWinAscent) + "," + OUString::number(nWinDescent));

    css::uno::Sequence<OUString> rWinMetricFontList(
        officecfg::Office::Common::Misc::FontsUseWinMetrics::get());
    if (comphelper::findValue(rWinMetricFontList, aFontIdentifier) != -1)
    {
        SAL_INFO("vcl.gdi.fontmetric", "Using win metrics for: " << aFontIdentifier);
        return true;
    }
    return false;
}

// These are “private” HarfBuzz metrics tags, they are supported by not exposed
// in the public header. They are safe to use, HarfBuzz just does not want to
// advertise them.
constexpr auto ASCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'a', 's', 'c'));
constexpr auto DESCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'd', 's', 'c'));
constexpr auto LINEGAP_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'l', 'g', 'p'));
constexpr auto ASCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'a', 's', 'c'));
constexpr auto DESCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'd', 's', 'c'));
constexpr auto LINEGAP_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'l', 'g', 'p'));

void FontMetricData::ImplCalcLineSpacing(LogicalFontInstance* pFontInstance)
{
    mnAscent = mnDescent = mnExtLeading = mnIntLeading = 0;
    auto* pFace = pFontInstance->GetFontFace();
    auto* pHbFont = pFontInstance->GetHbFont();

    double fScale = 0;
    pFontInstance->GetScale(nullptr, &fScale);
    double fAscent = 0, fDescent = 0, fExtLeading = 0;

    auto aFvar(pFace->GetRawFontData(HB_TAG('f', 'v', 'a', 'r')));
    if (!aFvar.empty())
    {
        // This is a variable font, trust HarfBuzz to give us the right metrics
        // and apply variations to them.
        hb_position_t nAscent, nDescent, nLineGap;
        if (hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &nAscent)
            && hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER,
                                          &nDescent)
            && hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_HORIZONTAL_LINE_GAP,
                                          &nLineGap))
        {
            fAscent = nAscent * fScale;
            fDescent = -nDescent * fScale;
            fExtLeading = nLineGap * fScale;
        }
    }
    else
    {
        // This is not a variable font, we try to choose the best metrics
        // ourselves for backward comparability:
        //
        // - hhea metrics should be used, since hhea is a mandatory font table
        //   and should always be present.
        // - But if OS/2 is present, it should be used since it is mandatory in
        //   Windows.
        //   OS/2 has Typo and Win metrics, but the later was meant to control
        //   text clipping not line spacing and can be ridiculously large.
        //   Unfortunately many Windows application incorrectly use the Win
        //   metrics (thanks to GDI’s TEXTMETRIC) and old fonts might be
        //   designed with this in mind, so OpenType introduced a flag for
        //   fonts to indicate that they really want to use Typo metrics. So
        //   for best backward compatibility:
        //   - Use Win metrics if available.
        //   - Unless USE_TYPO_METRICS flag is set, in which case use Typo
        //     metrics.

        // Try hhea table first.
        hb_position_t nAscent = 0, nDescent = 0, nLineGap = 0;
        if (hb_ot_metrics_get_position(pHbFont, ASCENT_HHEA, &nAscent)
            && hb_ot_metrics_get_position(pHbFont, DESCENT_HHEA, &nDescent)
            && hb_ot_metrics_get_position(pHbFont, LINEGAP_HHEA, &nLineGap))
        {
            // tdf#107605: Some fonts have weird values here, so check that
            // ascender is +ve and descender is -ve as they normally should.
            if (nAscent >= 0 && nDescent <= 0)
            {
                fAscent = nAscent * fScale;
                fDescent = -nDescent * fScale;
                fExtLeading = nLineGap * fScale;
            }
        }

        // But if OS/2 is present, prefer it.
        hb_position_t nTypoAscent, nTypoDescent, nTypoLineGap, nWinAscent, nWinDescent;
        if (hb_ot_metrics_get_position(pHbFont, ASCENT_OS2, &nTypoAscent)
            && hb_ot_metrics_get_position(pHbFont, DESCENT_OS2, &nTypoDescent)
            && hb_ot_metrics_get_position(pHbFont, LINEGAP_OS2, &nTypoLineGap)
            && hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_ASCENT,
                                          &nWinAscent)
            && hb_ot_metrics_get_position(pHbFont, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_DESCENT,
                                          &nWinDescent))
        {
            if ((fAscent == 0.0 && fDescent == 0.0)
                || ShouldUseWinMetrics(nAscent, nDescent, nTypoAscent, nTypoDescent, nWinAscent,
                                       nWinDescent))
            {
                fAscent = nWinAscent * fScale;
                fDescent = nWinDescent * fScale;
                fExtLeading = 0;
            }

            bool bUseTypoMetrics = false;
            {
                // TODO: Use HarfBuzz API instead of raw access
                // https://github.com/harfbuzz/harfbuzz/issues/1920
                sal_uInt16 fsSelection = 0;
                auto aOS2(pFace->GetRawFontData(HB_TAG('O', 'S', '/', '2')));
                SvMemoryStream aStream(const_cast<uint8_t*>(aOS2.data()), aOS2.size(),
                                       StreamMode::READ);
                // Font data are big endian.
                aStream.SetEndian(SvStreamEndian::BIG);
                if (aStream.Seek(vcl::OS2_fsSelection_offset) == vcl::OS2_fsSelection_offset)
                    aStream.ReadUInt16(fsSelection);
                bUseTypoMetrics = fsSelection & (1 << 7);
            }
            if (bUseTypoMetrics && nTypoAscent >= 0 && nTypoDescent <= 0)
            {
                fAscent = nTypoAscent * fScale;
                fDescent = -nTypoDescent * fScale;
                fExtLeading = nTypoLineGap * fScale;
            }
        }
    }

    mnAscent = round(fAscent);
    mnDescent = round(fDescent);
    mnExtLeading = round(fExtLeading);

    if (mnAscent || mnDescent)
        mnIntLeading = mnAscent + mnDescent - mnHeight;
}

void FontMetricData::ImplInitBaselines(LogicalFontInstance *pFontInstance)
{
    hb_font_t* pHbFont = pFontInstance->GetHbFont();
    double fScale = 0;
    pFontInstance->GetScale(nullptr, &fScale);
    hb_position_t nBaseline = 0;

    if (hb_ot_layout_get_baseline(pHbFont,
             HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                                HB_DIRECTION_INVALID,
                                HB_SCRIPT_UNKNOWN,
                                HB_TAG_NONE,
                                &nBaseline))
    {
        mnHangingBaseline = nBaseline * fScale;
    }
    else
    {
        mnHangingBaseline = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
