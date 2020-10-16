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

#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <sal/log.hxx>

#include <fontinstance.hxx>
#include <fontselect.hxx>
#include <impfontmetricdata.hxx>
#include <sft.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

FontMetric::FontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnBulletOffset( 0 ),
    mbFullstopCentered( false )
{}

FontMetric::FontMetric( const FontMetric& rFontMetric ) = default;

FontMetric::~FontMetric()
{
}

FontMetric& FontMetric::operator=(const FontMetric& rFontMetric) = default;

FontMetric& FontMetric::operator=(FontMetric&& rFontMetric) = default;

bool FontMetric::operator==( const FontMetric& r ) const
{
    if( Font::operator!=(r) )
        return false;
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

ImplFontMetricData::ImplFontMetricData( const FontSelectPattern& rFontSelData )
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
    SetFamilyName( GetNextFontToken( rFontSelData.GetFamilyName(), nTokenPos ) );
    SetStyleName( rFontSelData.GetStyleName() );
}

void ImplFontMetricData::ImplInitTextLineSize( const OutputDevice* pDev )
{
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
    tools::Long nStrikeoutOffset = -((mnAscent - mnIntLeading) / 3);

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

    mnBulletOffset = ( pDev->GetTextWidth( OUString( u' ' ) ) - pDev->GetTextWidth( OUString( u'\x00b7' ) ) ) >> 1 ;

}


void ImplFontMetricData::ImplInitAboveTextLineSize()
{
    tools::Long nIntLeading = mnIntLeading;
    // TODO: assess usage of nLeading below (changed in extleading CWS)
    // if no leading is available, we assume 15% of the ascent
    if ( nIntLeading <= 0 )
    {
        nIntLeading = mnAscent*15/100;
        if ( !nIntLeading )
            nIntLeading = 1;
    }

    tools::Long nLineHeight = ((nIntLeading*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;

    tools::Long nBLineHeight = ((nIntLeading*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;

    tools::Long n2LineHeight = ((nIntLeading*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;

    tools::Long nCeiling = -mnAscent;

    mnAboveUnderlineSize       = nLineHeight;
    mnAboveUnderlineOffset     = nCeiling + (nIntLeading - nLineHeight + 1) / 2;

    mnAboveBUnderlineSize      = nBLineHeight;
    mnAboveBUnderlineOffset    = nCeiling + (nIntLeading - nBLineHeight + 1) / 2;

    mnAboveDUnderlineSize      = n2LineHeight;
    mnAboveDUnderlineOffset1   = nCeiling + (nIntLeading - 3*n2LineHeight + 1) / 2;
    mnAboveDUnderlineOffset2   = nCeiling + (nIntLeading +   n2LineHeight + 1) / 2;

    tools::Long nWCalcSize = nIntLeading;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnAboveWUnderlineSize = nWCalcSize;
        else
            mnAboveWUnderlineSize = 3;
    }
    else
        mnAboveWUnderlineSize = ((nWCalcSize*50)+50) / 100;

    mnAboveWUnderlineOffset = nCeiling + (nIntLeading + 1) / 2;
}

void ImplFontMetricData::ImplInitFlags( const OutputDevice* pDev )
{
    const vcl::Font& rFont ( pDev->GetFont() );
    bool bCentered = true;
    if (MsLangId::isCJK(rFont.GetLanguage()))
    {
        tools::Rectangle aRect;
        pDev->GetTextBoundRect( aRect, u"\x3001" ); // Fullwidth fullstop
        const auto nH = rFont.GetFontSize().Height();
        const auto nB = aRect.Left();
        // Use 18.75% as a threshold to define a centered fullwidth fullstop.
        // In general, nB/nH < 5% for most Japanese fonts.
        bCentered = nB > (((nH >> 1)+nH)>>3);
    }
    SetFullstopCenteredFlag( bCentered );
}

bool ImplFontMetricData::ShouldUseWinMetrics(const vcl::TTGlobalFontInfo& rInfo)
{
    if (utl::ConfigManager::IsFuzzing())
        return false;

    OUString aFontIdentifier(
        GetFamilyName() + ","
        + OUString::number(rInfo.ascender) + "," + OUString::number(rInfo.descender) + ","
        + OUString::number(rInfo.typoAscender) + "," + OUString::number(rInfo.typoDescender) + ","
        + OUString::number(rInfo.winAscent) + "," + OUString::number(rInfo.winDescent));

    css::uno::Sequence<OUString> rWinMetricFontList(
        officecfg::Office::Common::Misc::FontsUseWinMetrics::get());
    if (comphelper::findValue(rWinMetricFontList, aFontIdentifier) != -1)
    {
        SAL_INFO("vcl.gdi.fontmetric", "Using win metrics for: " << aFontIdentifier);
        return true;
    }
    return false;
}

/*
 * Calculate line spacing:
 *
 * - hhea metrics should be used, since hhea is a mandatory font table and
 *   should always be present.
 * - But if OS/2 is present, it should be used since it is mandatory in
 *   Windows.
 *   OS/2 has Typo and Win metrics, but the later was meant to control
 *   text clipping not line spacing and can be ridiculously large.
 *   Unfortunately many Windows application incorrectly use the Win metrics
 *   (thanks to GDI’s TEXTMETRIC) and old fonts might be designed with this
 *   in mind, so OpenType introduced a flag for fonts to indicate that they
 *   really want to use Typo metrics. So for best backward compatibility:
 *   - Use Win metrics if available.
 *   - Unless USE_TYPO_METRICS flag is set, in which case use Typo metrics.
*/
void ImplFontMetricData::ImplCalcLineSpacing(LogicalFontInstance *pFontInstance)
{
    mnAscent = mnDescent = mnExtLeading = mnIntLeading = 0;

    hb_font_t* pHbFont = pFontInstance->GetHbFont();
    hb_face_t* pHbFace = hb_font_get_face(pHbFont);

    hb_blob_t* pHhea = hb_face_reference_table(pHbFace, HB_TAG('h', 'h', 'e', 'a'));
    hb_blob_t* pOS2 = hb_face_reference_table(pHbFace, HB_TAG('O', 'S', '/', '2'));

    vcl::TTGlobalFontInfo rInfo = {};
    GetTTFontMetrics(reinterpret_cast<const uint8_t*>(hb_blob_get_data(pHhea, nullptr)), hb_blob_get_length(pHhea),
                     reinterpret_cast<const uint8_t*>(hb_blob_get_data(pOS2, nullptr)), hb_blob_get_length(pOS2),
                     &rInfo);

    hb_blob_destroy(pHhea);
    hb_blob_destroy(pOS2);

    double nUPEM = hb_face_get_upem(pHbFace);
    double fScale = mnHeight / nUPEM;
    double fAscent = 0, fDescent = 0, fExtLeading = 0;

    // Try hhea table first.
    // tdf#107605: Some fonts have weird values here, so check that ascender is
    // +ve and descender is -ve as they normally should.
    if (rInfo.ascender >= 0 && rInfo.descender <= 0)
    {
        fAscent     =  rInfo.ascender  * fScale;
        fDescent    = -rInfo.descender * fScale;
        fExtLeading =  rInfo.linegap   * fScale;
    }

    // But if OS/2 is present, prefer it.
    if (rInfo.winAscent || rInfo.winDescent ||
        rInfo.typoAscender || rInfo.typoDescender)
    {
        if (ShouldUseWinMetrics(rInfo) || (fAscent == 0.0 && fDescent == 0.0))
        {
            fAscent     = rInfo.winAscent  * fScale;
            fDescent    = rInfo.winDescent * fScale;
            fExtLeading = 0;
        }

        const uint16_t kUseTypoMetricsMask = 1 << 7;
        if (rInfo.fsSelection & kUseTypoMetricsMask &&
            rInfo.typoAscender >= 0 && rInfo.typoDescender <= 0)
        {
            fAscent     =  rInfo.typoAscender  * fScale;
            fDescent    = -rInfo.typoDescender * fScale;
            fExtLeading =  rInfo.typoLineGap   * fScale;
        }
    }

    mnAscent = round(fAscent);
    mnDescent = round(fDescent);
    mnExtLeading = round(fExtLeading);

    if (mnAscent || mnDescent)
        mnIntLeading = mnAscent + mnDescent - mnHeight;

    SAL_INFO("vcl.gdi.fontmetric", GetFamilyName()
             << ": fsSelection: "   << rInfo.fsSelection
             << ", typoAscender: "  << rInfo.typoAscender
             << ", typoDescender: " << rInfo.typoDescender
             << ", typoLineGap: "   << rInfo.typoLineGap
             << ", winAscent: "     << rInfo.winAscent
             << ", winDescent: "    << rInfo.winDescent
             << ", ascender: "      << rInfo.ascender
             << ", descender: "     << rInfo.descender
             << ", linegap: "       << rInfo.linegap
             );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
