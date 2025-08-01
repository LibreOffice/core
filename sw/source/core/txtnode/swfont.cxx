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

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/outdev.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <vcl/metric.hxx>
#include <IDocumentSettingAccess.hxx>
#include <charatr.hxx>
#include <viewsh.hxx>
#include <swfont.hxx>
#include <fntcache.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <swmodule.hxx>

#ifdef DBG_UTIL
// global Variable
SvStatistics g_SvStat;
#endif

using namespace ::com::sun::star;

// set background brush, depending on character formatting
void SwFont::SetBackColor( std::optional<Color> xNewColor )
{
    mxBackColor = xNewColor;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

void SwFont::SetTopBorder( const editeng::SvxBorderLine* pTopBorder )
{
    if( pTopBorder )
        m_aTopBorder = *pTopBorder;
    else
    {
        m_aTopBorder.reset();
        m_nTopBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

void SwFont::SetBottomBorder( const editeng::SvxBorderLine* pBottomBorder )
{
    if( pBottomBorder )
        m_aBottomBorder = *pBottomBorder;
    else
    {
        m_aBottomBorder.reset();
        m_nBottomBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

void SwFont::SetRightBorder( const editeng::SvxBorderLine* pRightBorder )
{
    if( pRightBorder )
        m_aRightBorder = *pRightBorder;
    else
    {
        m_aRightBorder.reset();
        m_nRightBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

void SwFont::SetLeftBorder( const editeng::SvxBorderLine* pLeftBorder )
{
    if( pLeftBorder )
        m_aLeftBorder = *pLeftBorder;
    else
    {
        m_aLeftBorder.reset();
        m_nLeftBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

const std::optional<editeng::SvxBorderLine>&
SwFont::GetAbsTopBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const
{
    switch (GetOrientation(bVertLayout, bVertLayoutLRBT).get())
    {
        case 0 :
            return m_aTopBorder;
        case 900 :
            return m_aRightBorder;
        case 1800 :
            return m_aBottomBorder;
        case 2700 :
            return m_aLeftBorder;
        default :
            assert(false);
            return m_aTopBorder;
    }
}

const std::optional<editeng::SvxBorderLine>&
SwFont::GetAbsBottomBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const
{
    switch (GetOrientation(bVertLayout, bVertLayoutLRBT).get())
    {
        case 0 :
            return m_aBottomBorder;
        case 900 :
            return m_aLeftBorder;
        case 1800 :
            return m_aTopBorder;
        case 2700 :
            return m_aRightBorder;
        default :
            assert(false);
            return m_aBottomBorder;
    }
}

const std::optional<editeng::SvxBorderLine>&
SwFont::GetAbsLeftBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const
{
    switch (GetOrientation(bVertLayout, bVertLayoutLRBT).get())
    {
        case 0 :
            return m_aLeftBorder;
        case 900 :
            return m_aTopBorder;
        case 1800 :
            return m_aRightBorder;
        case 2700 :
            return m_aBottomBorder;
        default :
            assert(false);
            return m_aLeftBorder;
    }
}

const std::optional<editeng::SvxBorderLine>&
SwFont::GetAbsRightBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const
{
    switch (GetOrientation(bVertLayout, bVertLayoutLRBT).get())
    {
        case 0 :
            return m_aRightBorder;
        case 900 :
            return m_aBottomBorder;
        case 1800 :
            return m_aLeftBorder;
        case 2700 :
            return m_aTopBorder;
        default :
            assert(false);
            return m_aRightBorder;
    }
}

SvxShadowLocation SwFont::GetAbsShadowLocation(const bool bVertLayout,
                                               const bool bVertLayoutLRBT) const
{
    SvxShadowLocation aLocation = SvxShadowLocation::NONE;
    switch (GetOrientation(bVertLayout, bVertLayoutLRBT).get())
    {
        case 0:
            aLocation = m_aShadowLocation;
            break;

        case 900:
            switch ( m_aShadowLocation )
            {
                case SvxShadowLocation::TopLeft:
                    aLocation = SvxShadowLocation::BottomLeft;
                    break;
                case SvxShadowLocation::TopRight:
                    aLocation = SvxShadowLocation::TopLeft;
                    break;
                case SvxShadowLocation::BottomLeft:
                    aLocation = SvxShadowLocation::BottomRight;
                    break;
                case SvxShadowLocation::BottomRight:
                    aLocation = SvxShadowLocation::TopRight;
                    break;
                case SvxShadowLocation::NONE:
                case SvxShadowLocation::End:
                    aLocation = m_aShadowLocation;
                    break;
            }
            break;

        case 1800:
            switch ( m_aShadowLocation )
            {
                case SvxShadowLocation::TopLeft:
                    aLocation = SvxShadowLocation::BottomRight;
                    break;
                case SvxShadowLocation::TopRight:
                    aLocation = SvxShadowLocation::BottomLeft;
                    break;
                case SvxShadowLocation::BottomLeft:
                    aLocation = SvxShadowLocation::TopRight;
                    break;
                case SvxShadowLocation::BottomRight:
                    aLocation = SvxShadowLocation::TopLeft;
                    break;
                case SvxShadowLocation::NONE:
                case SvxShadowLocation::End:
                    aLocation = m_aShadowLocation;
                    break;
            }
            break;

        case 2700:
            switch ( m_aShadowLocation )
            {
                case SvxShadowLocation::TopLeft:
                    aLocation = SvxShadowLocation::TopRight;
                    break;
                case SvxShadowLocation::TopRight:
                    aLocation = SvxShadowLocation::BottomRight;
                    break;
                case SvxShadowLocation::BottomLeft:
                    aLocation = SvxShadowLocation::TopLeft;
                    break;
                case SvxShadowLocation::BottomRight:
                    aLocation = SvxShadowLocation::BottomLeft;
                    break;
                case SvxShadowLocation::NONE:
                case SvxShadowLocation::End:
                    aLocation = m_aShadowLocation;
                    break;
            }
            break;

        default:
            assert(false);
            break;
    }
    return aLocation;
}

sal_uInt16 SwFont::CalcShadowSpace(const SvxShadowItemSide nShadow, const bool bVertLayout,
                                   const bool bVertLayoutLRBT, const bool bSkipLeft,
                                   const bool bSkipRight) const
{
    sal_uInt16 nSpace = 0;
    const Degree10 nOrient = GetOrientation(bVertLayout, bVertLayoutLRBT);
    const SvxShadowLocation aLoc = GetAbsShadowLocation(bVertLayout, bVertLayoutLRBT);
    switch( nShadow )
    {
        case SvxShadowItemSide::TOP:
            if(( aLoc == SvxShadowLocation::TopLeft ||
               aLoc == SvxShadowLocation::TopRight ) &&
               ( nOrient == 0_deg10 || nOrient == 1800_deg10 ||
               ( nOrient == 900_deg10 && !bSkipRight ) ||
               ( nOrient == 2700_deg10 && !bSkipLeft )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SvxShadowItemSide::BOTTOM:
            if(( aLoc == SvxShadowLocation::BottomLeft ||
               aLoc == SvxShadowLocation::BottomRight ) &&
               ( nOrient == 0_deg10 || nOrient == 1800_deg10 ||
               ( nOrient == 900_deg10 && !bSkipLeft ) ||
               ( nOrient == 2700_deg10 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SvxShadowItemSide::LEFT:
            if(( aLoc == SvxShadowLocation::TopLeft ||
               aLoc == SvxShadowLocation::BottomLeft ) &&
               ( nOrient == 900_deg10 || nOrient == 2700_deg10 ||
               ( nOrient == 0_deg10 && !bSkipLeft ) ||
               ( nOrient == 1800_deg10 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

         case SvxShadowItemSide::RIGHT:
            if(( aLoc == SvxShadowLocation::TopRight ||
               aLoc == SvxShadowLocation::BottomRight ) &&
               ( nOrient == 900_deg10 || nOrient == 2700_deg10 ||
               ( nOrient == 0_deg10 && !bSkipRight ) ||
               ( nOrient == 1800_deg10 && !bSkipLeft )))
            {
                nSpace = m_nShadowWidth;
            }
            break;
        default:
            assert(false);
            break;
    }

    return nSpace;
}

SvxFontUnitMetrics SwFont::GetFontUnitMetrics() const
{
    // tdf#36709: Metrics conversion should use em and ic values from the bound fonts.
    // Unfortunately, this currently poses a problem due to font substitution: tests
    // abort when a missing font is set on a device.
    // In the interim, use height for all metrics. This is technically not correct, but
    // should be close enough for common fonts.
    return { /*em*/ static_cast<double>(GetHeight(GetActual())),
             /*ic*/ static_cast<double>(GetHeight(SwFontScript::CJK)) };
}

void SwFont::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, BAD_CAST("SwFont"));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ptr"), "%p", this);
    // do not use Color::AsRGBHexString() as that omits the transparency
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("color"), "%08" SAL_PRIxUINT32, sal_uInt32(GetColor()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("height"), BAD_CAST(OString::number(GetSize(GetActual()).Height()).getStr()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("width"), BAD_CAST(OString::number(GetSize(GetActual()).Width()).getStr()));
    {
        std::stringstream ss;
        ss << GetWeight();
        (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("weight"), BAD_CAST(ss.str().c_str()));
    }
    (void)xmlTextWriterEndElement(writer);
}

// maps directions for vertical layout
static Degree10 MapDirection(Degree10 nDir, const bool bVertFormat, const bool bVertFormatLRBT)
{
    if ( bVertFormat )
    {
        switch ( nDir.get() )
        {
        case 0 :
            if (bVertFormatLRBT)
                nDir = 900_deg10;
            else
                nDir = 2700_deg10;
            break;
        case 900 :
            nDir = 0_deg10;
            break;
        case 2700 :
            nDir = 1800_deg10;
            break;
#if OSL_DEBUG_LEVEL > 0
        default :
            OSL_FAIL( "Unsupported direction" );
            break;
#endif
        }
    }
    return nDir;
}

// maps the absolute direction set at the font to its logical counterpart
// in the rotated environment
Degree10 UnMapDirection(Degree10 nDir, const bool bVertFormat, const bool bVertFormatLRBT)
{
    if (bVertFormatLRBT)
    {
        switch (nDir.get())
        {
            case 900:
                nDir = 0_deg10;
                break;
            default:
                SAL_WARN("sw.core", "unsupported direction for VertLRBT");
                break;
        }
        return nDir;
    }

    if ( bVertFormat )
    {
        switch ( nDir.get() )
        {
        case 0 :
            nDir = 900_deg10;
            break;
        case 1800 :
            nDir = 2700_deg10;
            break;
        case 2700 :
            nDir = 0_deg10;
            break;
#if OSL_DEBUG_LEVEL > 0
        default :
            OSL_FAIL( "Unsupported direction" );
            break;
#endif
        }
    }
    return nDir;
}

Degree10 SwFont::GetOrientation(const bool bVertFormat, const bool bVertFormatLRBT) const
{
    return UnMapDirection(m_aSub[m_nActual].GetOrientation(), bVertFormat, bVertFormatLRBT);
}

void SwFont::SetVertical(Degree10 nDir, const bool bVertFormat, const bool bVertLayoutLRBT)
{
    // map direction if frame has vertical layout
    nDir = MapDirection(nDir, bVertFormat, bVertLayoutLRBT);

    if( nDir != m_aSub[SwFontScript::Latin].GetOrientation() )
    {
        m_bFontChg = true;
        bool bVertical = bVertFormat && !bVertLayoutLRBT;
        m_aSub[SwFontScript::Latin].SetVertical(nDir, bVertical);
        m_aSub[SwFontScript::CJK].SetVertical(nDir, bVertical);
        m_aSub[SwFontScript::CTL].SetVertical(nDir, bVertical);
    }
}

/*
 Escapement:
    frEsc:  Fraction, ratio of Escapements
    Esc = resulting Escapement
    A1 = original Ascent            (nOrgAscent)
    A2 = shrunk Ascent              (nEscAscent)
    Ax = resulting Ascent           (GetAscent())
    H1 = original Height            (nOrgHeight)
    H2 = shrunk Height              (nEscHeight)
    Hx = resulting Height           (GetHeight())
    Bx = resulting Baseline for Text (CalcPos())
         (Attention: Y - A1!)

    Escapement:
        Esc = H1 * frEsc;

    Superscript:
        Ax = A2 + Esc;
        Hx = H2 + Esc;
        Bx = A1 - Esc;

    Subscript:
        Ax = A1;
        Hx = A1 + Esc + (H2 - A2);
        Bx = A1 + Esc;
*/

// nEsc is the percentage
sal_uInt16 SwSubFont::CalcEscAscent( const sal_uInt16 nOldAscent ) const
{
    if( DFLT_ESC_AUTO_SUPER != GetEscapement() &&
        DFLT_ESC_AUTO_SUB != GetEscapement() )
    {
        const tools::Long nAscent = nOldAscent +
                             ( static_cast<tools::Long>(m_nOrgHeight) * GetEscapement() ) / 100;
        if ( nAscent>0 )
            return std::max<sal_uInt16>( nAscent, m_nOrgAscent );
    }
    return m_nOrgAscent;
}

void SwFont::SetDiffFnt( const SfxItemSet *pAttrSet,
                         const IDocumentSettingAccess *pIDocumentSettingAccess )
{
    mxBackColor.reset();

    if( pAttrSet )
    {

        if( const SvxFontItem* pFont = pAttrSet->GetItemIfSet( RES_CHRATR_FONT ) )
        {
            m_aSub[SwFontScript::Latin].SetFamily( pFont->GetFamily() );
            m_aSub[SwFontScript::Latin].Font::SetFamilyName( pFont->GetFamilyName() );
            m_aSub[SwFontScript::Latin].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SwFontScript::Latin].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SwFontScript::Latin].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( const SvxFontHeightItem *pHeight = pAttrSet->GetItemIfSet( RES_CHRATR_FONTSIZE ) )
        {
            m_aSub[SwFontScript::Latin].SvxFont::SetPropr( 100 );
            m_aSub[SwFontScript::Latin].m_aSize = m_aSub[SwFontScript::Latin].Font::GetFontSize();
            Size aTmpSize = m_aSub[SwFontScript::Latin].m_aSize;
            aTmpSize.setHeight( pHeight->GetHeight() );
            m_aSub[SwFontScript::Latin].SetSize( aTmpSize );
        }
        if( const SvxPostureItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_POSTURE ) )
            m_aSub[SwFontScript::Latin].Font::SetItalic( pItem->GetPosture() );
        if( const SvxWeightItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_WEIGHT ) )
            m_aSub[SwFontScript::Latin].Font::SetWeight( pItem->GetWeight() );
        if( const SvxLanguageItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_LANGUAGE ) )
            m_aSub[SwFontScript::Latin].SetLanguage( pItem->GetLanguage() );

        if( const SvxFontItem* pFont = pAttrSet->GetItemIfSet( RES_CHRATR_CJK_FONT ) )
        {
            m_aSub[SwFontScript::CJK].SetFamily( pFont->GetFamily() );
            m_aSub[SwFontScript::CJK].Font::SetFamilyName( pFont->GetFamilyName() );
            m_aSub[SwFontScript::CJK].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SwFontScript::CJK].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SwFontScript::CJK].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( const SvxFontHeightItem* pHeight = pAttrSet->GetItemIfSet( RES_CHRATR_CJK_FONTSIZE) )
        {
            m_aSub[SwFontScript::CJK].SvxFont::SetPropr( 100 );
            m_aSub[SwFontScript::CJK].m_aSize = m_aSub[SwFontScript::CJK].Font::GetFontSize();
            Size aTmpSize = m_aSub[SwFontScript::CJK].m_aSize;
            aTmpSize.setHeight( pHeight->GetHeight() );
            m_aSub[SwFontScript::CJK].SetSize( aTmpSize );
        }
        if( const SvxPostureItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CJK_POSTURE ) )
            m_aSub[SwFontScript::CJK].Font::SetItalic( pItem->GetPosture() );
        if( const SvxWeightItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CJK_WEIGHT ) )
            m_aSub[SwFontScript::CJK].Font::SetWeight( pItem->GetWeight() );
        if( const SvxLanguageItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CJK_LANGUAGE ) )
        {
            LanguageType eNewLang = pItem->GetLanguage();
            m_aSub[SwFontScript::CJK].SetLanguage( eNewLang );
            m_aSub[SwFontScript::Latin].SetCJKContextLanguage( eNewLang );
            m_aSub[SwFontScript::CJK].SetCJKContextLanguage( eNewLang );
            m_aSub[SwFontScript::CTL].SetCJKContextLanguage( eNewLang );
        }

        if( const SvxFontItem* pFont = pAttrSet->GetItemIfSet( RES_CHRATR_CTL_FONT ) )
        {
            m_aSub[SwFontScript::CTL].SetFamily( pFont->GetFamily() );
            m_aSub[SwFontScript::CTL].Font::SetFamilyName( pFont->GetFamilyName() );
            m_aSub[SwFontScript::CTL].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SwFontScript::CTL].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SwFontScript::CTL].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( const SvxFontHeightItem* pHeight = pAttrSet->GetItemIfSet( RES_CHRATR_CTL_FONTSIZE ) )
        {
            m_aSub[SwFontScript::CTL].SvxFont::SetPropr( 100 );
            m_aSub[SwFontScript::CTL].m_aSize = m_aSub[SwFontScript::CTL].Font::GetFontSize();
            Size aTmpSize = m_aSub[SwFontScript::CTL].m_aSize;
            aTmpSize.setHeight( pHeight->GetHeight() );
            m_aSub[SwFontScript::CTL].SetSize( aTmpSize );
        }
        if( const SvxPostureItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CTL_POSTURE ) )
            m_aSub[SwFontScript::CTL].Font::SetItalic(pItem->GetPosture() );
        if( const SvxWeightItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CTL_WEIGHT ) )
            m_aSub[SwFontScript::CTL].Font::SetWeight( pItem->GetWeight() );
        if( const SvxLanguageItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CTL_LANGUAGE ) )
            m_aSub[SwFontScript::CTL].SetLanguage( pItem->GetLanguage() );

        if( const SvxUnderlineItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_UNDERLINE ) )
        {
            SetUnderline( pItem->GetLineStyle() );
            SetUnderColor( pItem->GetColor() );
        }
        if( const SvxOverlineItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_OVERLINE ) )
        {
            SetOverline( pItem->GetLineStyle() );
            SetOverColor( pItem->GetColor() );
        }
        if( const SvxCrossedOutItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CROSSEDOUT ) )
            SetStrikeout( pItem->GetStrikeout() );
        if( const SvxColorItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_COLOR ) )
            SetColor( pItem->GetValue() );
        if( const SvxEmphasisMarkItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_EMPHASIS_MARK ))
            SetEmphasisMark( pItem->GetEmphasisMark() );

        SetTransparent( true );
        SetAlign( ALIGN_BASELINE );
        if( const SvxContourItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CONTOUR ) )
            SetOutline( pItem->GetValue() );
        if( const SvxShadowedItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_SHADOWED ) )
            SetShadow( pItem->GetValue() );
        if( const SvxCharReliefItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_RELIEF ) )
            SetRelief( pItem->GetValue() );
        if( const SvxShadowedItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_SHADOWED ))
            SetPropWidth( pItem->GetValue() ? 50 : 100 );
        if( const SvxAutoKernItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_AUTOKERN ) )
        {
            if( pItem->GetValue() )
            {
                SetAutoKern( ( !pIDocumentSettingAccess ||
                               !pIDocumentSettingAccess->get(DocumentSettingId::KERN_ASIAN_PUNCTUATION) ) ?
                                FontKerning::FontSpecific :
                                FontKerning::Asian );
            }
            else
                SetAutoKern( FontKerning::NONE );
        }
        if( const SvxWordLineModeItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_WORDLINEMODE ) )
            SetWordLineMode( pItem->GetValue() );

        if( const SvxEscapementItem* pEsc = pAttrSet->GetItemIfSet( RES_CHRATR_ESCAPEMENT ) )
        {
            SetEscapement( pEsc->GetEsc() );
            if( m_aSub[SwFontScript::Latin].IsEsc() )
                SetProportion( pEsc->GetProportionalHeight() );
        }
        if( const SvxCaseMapItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_CASEMAP ) )
            SetCaseMap( pItem->GetCaseMap() );
        if( const SvxKerningItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_KERNING ) )
            SetFixKerning( pItem->GetValue() );
        if( const SvxCharRotateItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_ROTATE ) )
            SetVertical( pItem->GetValue() );
        if( const SvxBrushItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_BACKGROUND ) )
            mxBackColor = pItem->GetColor();
        if( const SvxBrushItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_HIGHLIGHT ) )
            SetHighlightColor(pItem->GetColor());
        if( const SvxBoxItem* pBoxItem = pAttrSet->GetItemIfSet( RES_CHRATR_BOX ) )
        {
            SetTopBorder(pBoxItem->GetTop());
            SetBottomBorder(pBoxItem->GetBottom());
            SetRightBorder(pBoxItem->GetRight());
            SetLeftBorder(pBoxItem->GetLeft());
            SetTopBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::TOP));
            SetBottomBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::BOTTOM));
            SetRightBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::RIGHT));
            SetLeftBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::LEFT));
        }
        if( const SvxShadowItem* pShadowItem = pAttrSet->GetItemIfSet( RES_CHRATR_SHADOW ) )
        {
            SetShadowColor(pShadowItem->GetColor());
            SetShadowWidth(pShadowItem->GetWidth());
            SetShadowLocation(pShadowItem->GetLocation());
        }
        const SvxTwoLinesItem* pTwoLinesItem = pAttrSet->GetItemIfSet( RES_CHRATR_TWO_LINES );
        if( pTwoLinesItem && pTwoLinesItem->GetValue() )
            SetVertical( 0_deg10 );
    }
    else
    {
        Invalidate();
    }
    m_bPaintBlank = false;
    OSL_ENSURE( m_aSub[SwFontScript::Latin].IsTransparent(), "SwFont: Transparent revolution" );
}

SwFont::SwFont( const SwFont &rFont )
    : m_aSub(rFont.m_aSub)
{
    m_nActual = rFont.m_nActual;
    mxBackColor = rFont.mxBackColor;
    m_aHighlightColor = rFont.m_aHighlightColor;
    m_aTopBorder = rFont.m_aTopBorder;
    m_aBottomBorder = rFont.m_aBottomBorder;
    m_aRightBorder = rFont.m_aRightBorder;
    m_aLeftBorder = rFont.m_aLeftBorder;
    m_nTopBorderDist = rFont.m_nTopBorderDist;
    m_nBottomBorderDist = rFont.m_nBottomBorderDist;
    m_nRightBorderDist = rFont.m_nRightBorderDist;
    m_nLeftBorderDist = rFont.m_nLeftBorderDist;
    m_aShadowColor = rFont.m_aShadowColor;
    m_nShadowWidth = rFont.m_nShadowWidth;
    m_aShadowLocation = rFont.m_aShadowLocation;
    m_aUnderColor = rFont.GetUnderColor();
    m_aOverColor  = rFont.GetOverColor();
    m_nToxCount = 0;
    m_nRefCount = 0;
    m_nMetaCount = 0;
    m_nContentControlCount = 0;
    m_nInputFieldCount = 0;
    m_bFontChg = rFont.m_bFontChg;
    m_bOrgChg = rFont.m_bOrgChg;
    m_bPaintBlank = rFont.m_bPaintBlank;
    m_bGreyWave = rFont.m_bGreyWave;
    m_bURL = rFont.m_bURL;
}

SwFont::SwFont( const SwAttrSet* pAttrSet,
                const IDocumentSettingAccess* pIDocumentSettingAccess )
    : m_aSub()
{
    m_nActual = SwFontScript::Latin;
    m_nToxCount = 0;
    m_nRefCount = 0;
    m_nMetaCount = 0;
    m_nContentControlCount = 0;
    m_nInputFieldCount = 0;
    m_bPaintBlank = false;
    m_bGreyWave = false;
    m_bOrgChg = true;
    {
        const SvxFontItem& rFont = pAttrSet->GetFont();
        m_aSub[SwFontScript::Latin].SetFamily( rFont.GetFamily() );
        m_aSub[SwFontScript::Latin].SetFamilyName( rFont.GetFamilyName() );
        m_aSub[SwFontScript::Latin].SetStyleName( rFont.GetStyleName() );
        m_aSub[SwFontScript::Latin].SetPitch( rFont.GetPitch() );
        m_aSub[SwFontScript::Latin].SetCharSet( rFont.GetCharSet() );
        m_aSub[SwFontScript::Latin].SvxFont::SetPropr( 100 ); // 100% of FontSize
        Size aTmpSize = m_aSub[SwFontScript::Latin].m_aSize;
        aTmpSize.setHeight( pAttrSet->GetSize().GetHeight() );
        m_aSub[SwFontScript::Latin].SetSize( aTmpSize );
        m_aSub[SwFontScript::Latin].SetItalic( pAttrSet->GetPosture().GetPosture() );
        m_aSub[SwFontScript::Latin].SetWeight( pAttrSet->GetWeight().GetWeight() );
        m_aSub[SwFontScript::Latin].SetLanguage( pAttrSet->GetLanguage().GetLanguage() );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCJKFont();
        m_aSub[SwFontScript::CJK].SetFamily( rFont.GetFamily() );
        m_aSub[SwFontScript::CJK].SetFamilyName( rFont.GetFamilyName() );
        m_aSub[SwFontScript::CJK].SetStyleName( rFont.GetStyleName() );
        m_aSub[SwFontScript::CJK].SetPitch( rFont.GetPitch() );
        m_aSub[SwFontScript::CJK].SetCharSet( rFont.GetCharSet() );
        m_aSub[SwFontScript::CJK].SvxFont::SetPropr( 100 ); // 100% of FontSize
        Size aTmpSize = m_aSub[SwFontScript::CJK].m_aSize;
        aTmpSize.setHeight( pAttrSet->GetCJKSize().GetHeight() );
        m_aSub[SwFontScript::CJK].SetSize( aTmpSize );
        m_aSub[SwFontScript::CJK].SetItalic( pAttrSet->GetCJKPosture().GetPosture() );
        m_aSub[SwFontScript::CJK].SetWeight( pAttrSet->GetCJKWeight().GetWeight() );
        LanguageType eNewLang = pAttrSet->GetCJKLanguage().GetLanguage();
        m_aSub[SwFontScript::CJK].SetLanguage( eNewLang );
        m_aSub[SwFontScript::Latin].SetCJKContextLanguage( eNewLang );
        m_aSub[SwFontScript::CJK].SetCJKContextLanguage( eNewLang );
        m_aSub[SwFontScript::CTL].SetCJKContextLanguage( eNewLang );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCTLFont();
        m_aSub[SwFontScript::CTL].SetFamily( rFont.GetFamily() );
        m_aSub[SwFontScript::CTL].SetFamilyName( rFont.GetFamilyName() );
        m_aSub[SwFontScript::CTL].SetStyleName( rFont.GetStyleName() );
        m_aSub[SwFontScript::CTL].SetPitch( rFont.GetPitch() );
        m_aSub[SwFontScript::CTL].SetCharSet( rFont.GetCharSet() );
        m_aSub[SwFontScript::CTL].SvxFont::SetPropr( 100 ); // 100% of FontSize
        Size aTmpSize = m_aSub[SwFontScript::CTL].m_aSize;
        aTmpSize.setHeight( pAttrSet->GetCTLSize().GetHeight() );
        m_aSub[SwFontScript::CTL].SetSize( aTmpSize );
        m_aSub[SwFontScript::CTL].SetItalic( pAttrSet->GetCTLPosture().GetPosture() );
        m_aSub[SwFontScript::CTL].SetWeight( pAttrSet->GetCTLWeight().GetWeight() );
        m_aSub[SwFontScript::CTL].SetLanguage( pAttrSet->GetCTLLanguage().GetLanguage() );
    }
    if ( pAttrSet->GetCharHidden().GetValue() )
        SetUnderline( LINESTYLE_DOTTED );
    else
        SetUnderline( pAttrSet->GetUnderline().GetLineStyle() );
    SetUnderColor( pAttrSet->GetUnderline().GetColor() );
    SetOverline( pAttrSet->GetOverline().GetLineStyle() );
    SetOverColor( pAttrSet->GetOverline().GetColor() );
    SetEmphasisMark( pAttrSet->GetEmphasisMark().GetEmphasisMark() );
    SetStrikeout( pAttrSet->GetCrossedOut().GetStrikeout() );
    SetColor( pAttrSet->GetColor().GetValue() );
    SetTransparent( true );
    SetAlign( ALIGN_BASELINE );
    SetOutline( pAttrSet->GetContour().GetValue() );
    SetShadow( pAttrSet->GetShadowed().GetValue() );
    SetPropWidth( pAttrSet->GetCharScaleW().GetValue() );
    SetRelief( pAttrSet->GetCharRelief().GetValue() );
    if( pAttrSet->GetAutoKern().GetValue() )
    {
        SetAutoKern( ( !pIDocumentSettingAccess ||
                       !pIDocumentSettingAccess->get(DocumentSettingId::KERN_ASIAN_PUNCTUATION) ) ?
                        FontKerning::FontSpecific :
                        FontKerning::Asian );
    }
    else
        SetAutoKern( FontKerning::NONE );
    SetWordLineMode( pAttrSet->GetWordLineMode().GetValue() );
    const SvxEscapementItem &rEsc = pAttrSet->GetEscapement();
    SetEscapement( rEsc.GetEsc() );
    if( m_aSub[SwFontScript::Latin].IsEsc() )
        SetProportion( rEsc.GetProportionalHeight() );
    SetCaseMap( pAttrSet->GetCaseMap().GetCaseMap() );
    SetFixKerning( pAttrSet->GetKerning().GetValue() );
    if( const SvxBrushItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_BACKGROUND ) )
        mxBackColor = pItem->GetColor();
    if( const SvxBrushItem* pItem = pAttrSet->GetItemIfSet( RES_CHRATR_HIGHLIGHT ) )
        SetHighlightColor(pItem->GetColor());
    else
        SetHighlightColor(COL_TRANSPARENT);
    if( const SvxBoxItem* pBoxItem = pAttrSet->GetItemIfSet( RES_CHRATR_BOX ) )
    {
        SetTopBorder(pBoxItem->GetTop());
        SetBottomBorder(pBoxItem->GetBottom());
        SetRightBorder(pBoxItem->GetRight());
        SetLeftBorder(pBoxItem->GetLeft());
        SetTopBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::TOP));
        SetBottomBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::BOTTOM));
        SetRightBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::RIGHT));
        SetLeftBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::LEFT));
    }
    else
    {
        SetTopBorder(nullptr);
        SetBottomBorder(nullptr);
        SetRightBorder(nullptr);
        SetLeftBorder(nullptr);
        SetTopBorderDist(0);
        SetBottomBorderDist(0);
        SetRightBorderDist(0);
        SetLeftBorderDist(0);
    }

    if( const SvxShadowItem* pShadowItem = pAttrSet->GetItemIfSet( RES_CHRATR_SHADOW ) )
    {
        SetShadowColor(pShadowItem->GetColor());
        SetShadowWidth(pShadowItem->GetWidth());
        SetShadowLocation(pShadowItem->GetLocation());
    }
    else
    {
        SetShadowColor(COL_TRANSPARENT);
        SetShadowWidth(0);
        SetShadowLocation(SvxShadowLocation::NONE);
    }

    const SvxTwoLinesItem& rTwoLinesItem = pAttrSet->Get2Lines();
    if ( ! rTwoLinesItem.GetValue() )
        SetVertical( pAttrSet->GetCharRotate().GetValue() );
    else
        SetVertical( 0_deg10 );
    if( pIDocumentSettingAccess && pIDocumentSettingAccess->get( DocumentSettingId::SMALL_CAPS_PERCENTAGE_66 ))
    {
        m_aSub[ SwFontScript::Latin ].m_bSmallCapsPercentage66 = true;
        m_aSub[ SwFontScript::CJK ].m_bSmallCapsPercentage66 = true;
        m_aSub[ SwFontScript::CTL ].m_bSmallCapsPercentage66 = true;
    }
}

SwFont::~SwFont()
{
}

SwFont& SwFont::operator=( const SwFont &rFont )
{
    if (this != &rFont)
    {
        m_aSub[SwFontScript::Latin] = rFont.m_aSub[SwFontScript::Latin];
        m_aSub[SwFontScript::CJK] = rFont.m_aSub[SwFontScript::CJK];
        m_aSub[SwFontScript::CTL] = rFont.m_aSub[SwFontScript::CTL];
        m_nActual = rFont.m_nActual;
        mxBackColor = rFont.mxBackColor;
        m_aHighlightColor = rFont.m_aHighlightColor;
        m_aTopBorder = rFont.m_aTopBorder;
        m_aBottomBorder = rFont.m_aBottomBorder;
        m_aRightBorder = rFont.m_aRightBorder;
        m_aLeftBorder = rFont.m_aLeftBorder;
        m_nTopBorderDist = rFont.m_nTopBorderDist;
        m_nBottomBorderDist = rFont.m_nBottomBorderDist;
        m_nRightBorderDist = rFont.m_nRightBorderDist;
        m_nLeftBorderDist = rFont.m_nLeftBorderDist;
        m_aShadowColor = rFont.m_aShadowColor;
        m_nShadowWidth = rFont.m_nShadowWidth;
        m_aShadowLocation = rFont.m_aShadowLocation;
        m_aUnderColor = rFont.GetUnderColor();
        m_aOverColor  = rFont.GetOverColor();
        m_nToxCount = 0;
        m_nRefCount = 0;
        m_nMetaCount = 0;
        m_nContentControlCount = 0;
        m_nInputFieldCount = 0;
        m_bFontChg = rFont.m_bFontChg;
        m_bOrgChg = rFont.m_bOrgChg;
        m_bPaintBlank = rFont.m_bPaintBlank;
        m_bGreyWave = rFont.m_bGreyWave;
        m_bURL = rFont.m_bURL;
    }
    return *this;
}

void SwFont::AllocFontCacheId( SwViewShell const *pSh, SwFontScript nWhich )
{
    SwFntAccess aFntAccess( m_aSub[nWhich].m_nFontCacheId, m_aSub[nWhich].m_nFontIndex,
                            &m_aSub[nWhich], pSh, true );
}

bool SwSubFont::IsSymbol( SwViewShell const *pSh )
{
    SwFntAccess aFntAccess( m_nFontCacheId, m_nFontIndex, this, pSh, false );
    return aFntAccess.Get()->IsSymbol();
}

bool SwSubFont::ChgFnt( SwViewShell const *pSh, OutputDevice& rOut )
{
    if ( pLastFont )
        pLastFont->Unlock();
    SwFntAccess aFntAccess( m_nFontCacheId, m_nFontIndex, this, pSh, true );
    SV_STAT( nChangeFont );

    pLastFont = aFntAccess.Get();

    pLastFont->SetDevFont( pSh, rOut );

    pLastFont->Lock();
    return LINESTYLE_NONE != GetUnderline() ||
           LINESTYLE_NONE != GetOverline()  ||
           STRIKEOUT_NONE != GetStrikeout();
}

void SwFont::ChgPhysFnt( SwViewShell const *pSh, OutputDevice& rOut )
{
    if( m_bOrgChg && m_aSub[m_nActual].IsEsc() )
    {
        const sal_uInt8 nOldProp = m_aSub[m_nActual].GetPropr();
        SetProportion( 100 );
        ChgFnt( pSh, rOut );
        SwFntAccess aFntAccess( m_aSub[m_nActual].m_nFontCacheId, m_aSub[m_nActual].m_nFontIndex,
                                &m_aSub[m_nActual], pSh );
        m_aSub[m_nActual].m_nOrgHeight = aFntAccess.Get()->GetFontHeight( pSh, rOut );
        m_aSub[m_nActual].m_nOrgAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
        SetProportion( nOldProp );
        m_bOrgChg = false;
    }

    if( m_bFontChg )
    {
        ChgFnt( pSh, rOut );
        m_bFontChg = m_bOrgChg;
    }
    if( rOut.GetTextLineColor() != m_aUnderColor )
        rOut.SetTextLineColor( m_aUnderColor );
    if( rOut.GetOverlineColor() != m_aOverColor )
        rOut.SetOverlineColor( m_aOverColor );
}

//         Height = MaxAscent + MaxDescent
//      MaxAscent = Max (T1_ascent, T2_ascent + (Esc * T1_height) );
//     MaxDescent = Max (T1_height-T1_ascent,
//                       T2_height-T2_ascent - (Esc * T1_height)
sal_uInt16 SwSubFont::CalcEscHeight( const sal_uInt16 nOldHeight,
                              const sal_uInt16 nOldAscent  ) const
{
    if( DFLT_ESC_AUTO_SUPER != GetEscapement() &&
        DFLT_ESC_AUTO_SUB != GetEscapement() )
    {
        tools::Long nDescent = nOldHeight - nOldAscent -
                             ( static_cast<tools::Long>(m_nOrgHeight) * GetEscapement() ) / 100;
        const sal_uInt16 nDesc = nDescent>0
                ? std::max<sal_uInt16>( nDescent, m_nOrgHeight - m_nOrgAscent)
                : m_nOrgHeight - m_nOrgAscent;
        return ( nDesc + CalcEscAscent( nOldAscent ) );
    }
    return m_nOrgHeight;
}

short SwSubFont::CheckKerning_( )
{
    short nKernx = - short( Font::GetFontSize().Height() / 6 );

    if ( nKernx < GetFixKerning() )
        return GetFixKerning();
    return nKernx;
}

sal_uInt16 SwSubFont::GetAscent( SwViewShell const *pSh, const OutputDevice& rOut )
{
    SwFntAccess aFntAccess( m_nFontCacheId, m_nFontIndex, this, pSh );
    const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
    return GetEscapement() ? CalcEscAscent( nAscent ) : nAscent;
}

sal_uInt16 SwSubFont::GetHeight( SwViewShell const *pSh, const OutputDevice& rOut )
{
    SV_STAT( nGetTextSize );
    SwFntAccess aFntAccess( m_nFontCacheId, m_nFontIndex, this, pSh );
    const sal_uInt16 nHeight = aFntAccess.Get()->GetFontHeight( pSh, rOut );
    if ( GetEscapement() )
    {
        const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
        return CalcEscHeight( nHeight, nAscent ); // + nLeading;
    }
    return nHeight; // + nLeading;
}

sal_uInt16 SwSubFont::GetHangingBaseline( SwViewShell const *pSh, const OutputDevice& rOut )
{
    SwFntAccess aFntAccess( m_nFontCacheId, m_nFontIndex, this, pSh );
    return aFntAccess.Get()->GetFontHangingBaseline( pSh, rOut );
}

Size SwSubFont::GetTextSize_( SwDrawTextInfo& rInf )
{
    // Robust: the font is supposed to be set already, but better safe than
    // sorry...
    if ( !pLastFont || pLastFont->GetOwner() != m_nFontCacheId ||
         !IsSameInstance( rInf.GetpOut()->GetFont() ) )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier(rInf.GetOut(), rInf.GetFont()->GetLanguage(),
                                           SwModule::get()->GetCTLTextNumerals());

    Size aTextSize;
    TextFrameIndex const nLn = rInf.GetLen() == TextFrameIndex(COMPLETE_STRING)
            ? TextFrameIndex(rInf.GetText().getLength())
            : rInf.GetLen();
    rInf.SetLen( nLn );

    if( IsCapital() && nLn )
    {
        if (rInf.GetMeasureLen() != TextFrameIndex(COMPLETE_STRING))
        {
            rInf.SetLen(rInf.GetMeasureLen());
            rInf.SetMeasureLen(TextFrameIndex(COMPLETE_STRING));
        }
        aTextSize = GetCapitalSize( rInf );
    }
    else
    {
        SV_STAT( nGetTextSize );
        tools::Long nOldKern = rInf.GetKern();
        const OUString oldText = rInf.GetText();
        rInf.SetKern( CheckKerning() );
        if ( !IsCaseMap() )
            aTextSize = pLastFont->GetTextSize( rInf );
        else
        {
            const OUString aTmp = CalcCaseMap( rInf.GetText() );
            const OUString oldStr = rInf.GetText();
            bool bCaseMapLengthDiffers(aTmp.getLength() != oldStr.getLength());

            if(bCaseMapLengthDiffers && rInf.GetLen())
            {
                // If the length of the original string and the CaseMapped one
                // are different, it is necessary to handle the given text part as
                // a single snippet since its size may differ, too.
                TextFrameIndex const nOldIdx(rInf.GetIdx());
                TextFrameIndex const nOldLen(rInf.GetLen());
                TextFrameIndex const nOldMeasureLen(rInf.GetMeasureLen());
                const OUString aSnippet(oldStr.copy(sal_Int32(nOldIdx), sal_Int32(nOldLen)));
                const OUString aNewText(CalcCaseMap(aSnippet));

                rInf.SetText( aNewText );
                rInf.SetIdx( TextFrameIndex(0) );
                rInf.SetLen( TextFrameIndex(aNewText.getLength()) );
                if (nOldMeasureLen != TextFrameIndex(COMPLETE_STRING))
                {
                    const OUString aMeasureSnippet(oldStr.copy(sal_Int32(nOldIdx), sal_Int32(nOldMeasureLen)));
                    const OUString aNewMeasureText(CalcCaseMap(aMeasureSnippet));
                    rInf.SetMeasureLen(TextFrameIndex(aNewMeasureText.getLength()));
                }

                aTextSize = pLastFont->GetTextSize( rInf );

                rInf.SetIdx( nOldIdx );
                rInf.SetLen( nOldLen );
                rInf.SetMeasureLen(nOldMeasureLen);
            }
            else
            {
                rInf.SetText( aTmp );
                aTextSize = pLastFont->GetTextSize( rInf );
            }

            rInf.SetText(oldStr);
        }
        rInf.SetKern( nOldKern );
        rInf.SetText(oldText);
        // A word that's longer than one line, with escapement at the line
        // break, must report its effective height.
        if( GetEscapement() )
        {
            const sal_uInt16 nAscent = pLastFont->GetFontAscent( rInf.GetShell(),
                                                             rInf.GetOut() );
            aTextSize.setHeight(
                static_cast<tools::Long>(CalcEscHeight( o3tl::narrowing<sal_uInt16>(aTextSize.Height()), nAscent)) );
        }
    }

    if (TextFrameIndex(1) == rInf.GetLen()
        && CH_TXT_ATR_FIELDSTART == rInf.GetText()[sal_Int32(rInf.GetIdx())])
    {
        SAL_WARN("sw", "this is meant to be dead code");
        TextFrameIndex const nOldIdx(rInf.GetIdx());
        TextFrameIndex const nOldLen(rInf.GetLen());
        OUString aOldText(rInf.GetText());
        const OUString aNewText(CH_TXT_ATR_SUBST_FIELDSTART);
        rInf.SetTextIdxLen(aNewText, TextFrameIndex(0), TextFrameIndex(aNewText.getLength()));
        aTextSize = pLastFont->GetTextSize( rInf );
        rInf.SetTextIdxLen(aOldText, nOldIdx, nOldLen);
    }
    else if (TextFrameIndex(1) == rInf.GetLen()
            && CH_TXT_ATR_FIELDEND == rInf.GetText()[sal_Int32(rInf.GetIdx())])
    {
        SAL_WARN("sw", "this is meant to be dead code");
        TextFrameIndex const nOldIdx(rInf.GetIdx());
        TextFrameIndex const nOldLen(rInf.GetLen());
        OUString aOldText(rInf.GetText());
        const OUString aNewText(CH_TXT_ATR_SUBST_FIELDEND);
        rInf.SetTextIdxLen(aNewText, TextFrameIndex(0), TextFrameIndex(aNewText.getLength()));
        aTextSize = pLastFont->GetTextSize( rInf );
        rInf.SetTextIdxLen(aOldText, nOldIdx, nOldLen);
    }

    return aTextSize;
}

void SwSubFont::DrawText_( SwDrawTextInfo &rInf, const bool bGrey )
{
    rInf.SetGreyWave( bGrey );
    TextFrameIndex const nLn(rInf.GetText().getLength());
    if( !rInf.GetLen() || !nLn )
        return;
    if (TextFrameIndex(COMPLETE_STRING) == rInf.GetLen())
        rInf.SetLen( nLn );

    FontLineStyle nOldUnder = LINESTYLE_NONE;
    SwUnderlineFont* pUnderFnt = nullptr;

    if( rInf.GetUnderFnt() )
    {
        nOldUnder = GetUnderline();
        SetUnderline( LINESTYLE_NONE );
        pUnderFnt = rInf.GetUnderFnt();
    }

    if( !pLastFont || pLastFont->GetOwner() != m_nFontCacheId )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier(rInf.GetOut(), rInf.GetFont()->GetLanguage(),
                                           SwModule::get()->GetCTLTextNumerals());

    const Point aOldPos(rInf.GetPos());
    Point aPos( rInf.GetPos() );

    if( GetEscapement() )
        CalcEsc( rInf, aPos );

    rInf.SetPos( aPos );
    rInf.SetKern( CheckKerning() + rInf.GetCharacterSpacing() / SPACING_PRECISION_FACTOR +
                    // modify letter spacing for the actual line
                    // according to the available extra word spacing
                    // to get the desired word spacing
                    rInf.GetLetterSpacing() );

    if( IsCapital() )
        DrawCapital( rInf );
    else
    {
        SV_STAT( nDrawText );
        if ( !IsCaseMap() )
            pLastFont->DrawText( rInf );
        else
        {
            const OUString oldStr = rInf.GetText();
            const OUString aString( CalcCaseMap(oldStr) );
            bool bCaseMapLengthDiffers(aString.getLength() != oldStr.getLength());

            if(bCaseMapLengthDiffers && rInf.GetLen())
            {
                // If the length of the original string and the CaseMapped one
                // are different, it is necessary to handle the given text part as
                // a single snippet since its size may differ, too.
                TextFrameIndex const nOldIdx(rInf.GetIdx());
                TextFrameIndex const nOldLen(rInf.GetLen());
                const OUString aSnippet(oldStr.copy(sal_Int32(nOldIdx), sal_Int32(nOldLen)));
                const OUString aNewText = CalcCaseMap(aSnippet);

                rInf.SetText( aNewText );
                rInf.SetIdx( TextFrameIndex(0) );
                rInf.SetLen( TextFrameIndex(aNewText.getLength()) );

                pLastFont->DrawText( rInf );

                rInf.SetIdx( nOldIdx );
                rInf.SetLen( nOldLen );
            }
            else
            {
                rInf.SetText( aString );
                pLastFont->DrawText( rInf );
            }

            rInf.SetText(oldStr);
        }
    }

    if( pUnderFnt && nOldUnder != LINESTYLE_NONE )
    {
        Size aFontSize = GetTextSize_( rInf );
        const OUString oldStr = rInf.GetText();

        TextFrameIndex const nOldIdx = rInf.GetIdx();
        TextFrameIndex const nOldLen = rInf.GetLen();
        tools::Long nSpace = 0;
        if( rInf.GetSpace() )
        {
            TextFrameIndex nTmpEnd = nOldIdx + nOldLen;
            if (nTmpEnd > TextFrameIndex(oldStr.getLength()))
                nTmpEnd = TextFrameIndex(oldStr.getLength());

            const SwScriptInfo* pSI = rInf.GetScriptInfo();

            const bool bAsianFont =
                ( rInf.GetFont() && SwFontScript::CJK == rInf.GetFont()->GetActual() );
            for (TextFrameIndex nTmp = nOldIdx; nTmp < nTmpEnd; ++nTmp)
            {
                if (CH_BLANK == oldStr[sal_Int32(nTmp)] || bAsianFont ||
                    (nTmp + TextFrameIndex(1) < TextFrameIndex(oldStr.getLength())
                     && pSI
                     && i18n::ScriptType::ASIAN == pSI->ScriptType(nTmp + TextFrameIndex(1))))
                {
                    ++nSpace;
                }
            }

            // if next portion if a hole portion we do not consider any
            // extra space added because the last character was ASIAN
            if ( nSpace && rInf.IsSpaceStop() && bAsianFont )
                 --nSpace;

            nSpace *= rInf.GetSpace() / SPACING_PRECISION_FACTOR;
        }

        rInf.SetWidth( sal_uInt16(aFontSize.Width() + nSpace) );
        rInf.SetTextIdxLen( u"  "_ustr, TextFrameIndex(0), TextFrameIndex(2) );
        SetUnderline( nOldUnder );
        rInf.SetUnderFnt( nullptr );

        // set position for underline font
        rInf.SetPos( pUnderFnt->GetPos() );

        pUnderFnt->GetFont().DrawStretchText_( rInf );

        rInf.SetUnderFnt( pUnderFnt );
        rInf.SetTextIdxLen(oldStr, nOldIdx, nOldLen);
    }

    rInf.SetPos(aOldPos);
}

void SwSubFont::DrawStretchText_( SwDrawTextInfo &rInf )
{
    if( !rInf.GetLen() || !rInf.GetText().getLength() )
        return;

    FontLineStyle nOldUnder = LINESTYLE_NONE;
    SwUnderlineFont* pUnderFnt = nullptr;

    if( rInf.GetUnderFnt() )
    {
        nOldUnder = GetUnderline();
        SetUnderline( LINESTYLE_NONE );
        pUnderFnt = rInf.GetUnderFnt();
    }

    if ( !pLastFont || pLastFont->GetOwner() != m_nFontCacheId )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier(rInf.GetOut(), rInf.GetFont()->GetLanguage(),
                                           SwModule::get()->GetCTLTextNumerals());

    rInf.ApplyAutoColor();

    const Point aOldPos(rInf.GetPos());
    Point aPos( rInf.GetPos() );

    if( GetEscapement() )
        CalcEsc( rInf, aPos );

    rInf.SetKern( CheckKerning() + rInf.GetCharacterSpacing() / SPACING_PRECISION_FACTOR );
    rInf.SetPos( aPos );

    if( IsCapital() )
        DrawStretchCapital( rInf );
    else
    {
        SV_STAT( nDrawStretchText );

        if ( rInf.GetFrame() )
        {
            if ( rInf.GetFrame()->IsRightToLeft() )
                rInf.GetFrame()->SwitchLTRtoRTL( aPos );

            if ( rInf.GetFrame()->IsVertical() )
                rInf.GetFrame()->SwitchHorizontalToVertical( aPos );

            rInf.SetPos( aPos );
        }

        if ( !IsCaseMap() )
            rInf.GetOut().DrawStretchText( aPos, rInf.GetWidth(),
                rInf.GetText(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
        else
            rInf.GetOut().DrawStretchText( aPos, rInf.GetWidth(),
                    CalcCaseMap(rInf.GetText()),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
    }

    if( pUnderFnt && nOldUnder != LINESTYLE_NONE )
    {
        const OUString oldStr = rInf.GetText();
        TextFrameIndex const nOldIdx = rInf.GetIdx();
        TextFrameIndex const nOldLen = rInf.GetLen();
        rInf.SetTextIdxLen( u"  "_ustr, TextFrameIndex(0), TextFrameIndex(2) );
        SetUnderline( nOldUnder );
        rInf.SetUnderFnt( nullptr );

        // set position for underline font
        rInf.SetPos( pUnderFnt->GetPos() );

        pUnderFnt->GetFont().DrawStretchText_( rInf );

        rInf.SetUnderFnt( pUnderFnt );
        rInf.SetTextIdxLen(oldStr, nOldIdx, nOldLen);
    }

    rInf.SetPos(aOldPos);
}

TextFrameIndex SwSubFont::GetModelPositionForViewPoint_( SwDrawTextInfo& rInf )
{
    if ( !pLastFont || pLastFont->GetOwner() != m_nFontCacheId )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier(rInf.GetOut(), rInf.GetFont()->GetLanguage(),
                                           SwModule::get()->GetCTLTextNumerals());

    TextFrameIndex const nLn = rInf.GetLen() == TextFrameIndex(COMPLETE_STRING)
            ? TextFrameIndex(rInf.GetText().getLength())
            : rInf.GetLen();
    rInf.SetLen( nLn );
    TextFrameIndex nCursor(0);
    if( IsCapital() && nLn )
        nCursor = GetCapitalCursorOfst( rInf );
    else
    {
        const OUString oldText = rInf.GetText();
        tools::Long nOldKern = rInf.GetKern();
        rInf.SetKern( CheckKerning() );
        SV_STAT( nGetTextSize );
        if ( !IsCaseMap() )
            nCursor = pLastFont->GetModelPositionForViewPoint( rInf );
        else
        {
            rInf.SetText( CalcCaseMap( rInf.GetText() ) );
            nCursor = pLastFont->GetModelPositionForViewPoint( rInf );
        }
        rInf.SetKern( nOldKern );
        rInf.SetText(oldText);
    }
    return nCursor;
}

void SwSubFont::CalcEsc( SwDrawTextInfo const & rInf, Point& rPos )
{
    tools::Long nOfst;

    bool bVert = false;
    bool bVertLRBT = false;
    if (rInf.GetFrame())
    {
        bVert = rInf.GetFrame()->IsVertical();
        bVertLRBT = rInf.GetFrame()->IsVertLRBT();
    }
    const Degree10 nDir = UnMapDirection(GetOrientation(), bVert, bVertLRBT);

    switch ( GetEscapement() )
    {
    case DFLT_ESC_AUTO_SUB :
        nOfst = m_nOrgHeight - m_nOrgAscent -
            pLastFont->GetFontHeight( rInf.GetShell(), rInf.GetOut() ) +
            pLastFont->GetFontAscent( rInf.GetShell(), rInf.GetOut() );

        switch ( nDir.get() )
        {
        case 0 :
            rPos.AdjustY(nOfst );
            break;
        case 900 :
            rPos.AdjustX(nOfst );
            break;
        case 2700 :
            rPos.AdjustX( -nOfst );
            break;
        }

        break;
    case DFLT_ESC_AUTO_SUPER :
        nOfst = pLastFont->GetFontAscent( rInf.GetShell(), rInf.GetOut() ) -
                m_nOrgAscent;

        switch ( nDir.get() )
        {
        case 0 :
            rPos.AdjustY(nOfst );
            break;
        case 900 :
            rPos.AdjustX(nOfst );
            break;
        case 2700 :
            rPos.AdjustX( -nOfst );
            break;
        }

        break;
    default :
        nOfst = (static_cast<tools::Long>(m_nOrgHeight) * GetEscapement()) / 100;

        switch ( nDir.get() )
        {
        case 0 :
            rPos.AdjustY( -nOfst );
            break;
        case 900 :
            rPos.AdjustX( -nOfst );
            break;
        case 2700 :
            rPos.AdjustX(nOfst );
            break;
        }
    }
}

// used during painting of small capitals
void SwDrawTextInfo::Shift( Degree10 nDir )
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_bPos, "DrawTextInfo: Undefined Position" );
    OSL_ENSURE( m_bSize, "DrawTextInfo: Undefined Width" );
#endif

    const bool bBidiPor = ( GetFrame() && GetFrame()->IsRightToLeft() ) !=
                          ( vcl::text::ComplexTextLayoutFlags::Default != ( vcl::text::ComplexTextLayoutFlags::BiDiRtl & GetpOut()->GetLayoutMode() ) );

    bool bVert = false;
    bool bVertLRBT = false;
    if (GetFrame())
    {
        bVert = GetFrame()->IsVertical();
        bVertLRBT = GetFrame()->IsVertLRBT();
    }
    nDir = bBidiPor ? 1800_deg10 : UnMapDirection(nDir, bVert, bVertLRBT);

    switch ( nDir.get() )
    {
    case 0 :
        m_aPos.AdjustX(GetSize().Width() );
        break;
    case 900 :
        OSL_ENSURE( m_aPos.Y() >= GetSize().Width(), "Going underground" );
        m_aPos.AdjustY( -(GetSize().Width()) );
        break;
    case 1800 :
        m_aPos.AdjustX( -(GetSize().Width()) );
        break;
    case 2700 :
        m_aPos.AdjustY(GetSize().Width() );
        break;
    }
}

/**
 * @note Used for the "continuous underline" feature.
 **/
SwUnderlineFont::SwUnderlineFont(SwFont& rFnt, TextFrameIndex const nEnd, const Point& rPoint)
        : m_aPos( rPoint ), m_nEnd( nEnd ), m_pFont( &rFnt )
{
};

SwUnderlineFont::~SwUnderlineFont()
{
}

/// Helper for filters to find true lineheight of a font
tools::Long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const vcl::RenderContext &rOut, sal_Int16 nScript)
{
    SwFont aFont(&rSet, &rIDocumentSettingAccess);
    SwFontScript nActual;
    switch (nScript)
    {
        default:
        case i18n::ScriptType::LATIN:
            nActual = SwFontScript::Latin;
            break;
        case i18n::ScriptType::ASIAN:
            nActual = SwFontScript::CJK;
            break;
        case i18n::ScriptType::COMPLEX:
            nActual = SwFontScript::CTL;
            break;
    }
    aFont.SetActual(nActual);

    vcl::RenderContext &rMutableOut = const_cast<vcl::RenderContext &>(rOut);
    const vcl::Font aOldFont(rMutableOut.GetFont());

    rMutableOut.SetFont(aFont.GetActualFont());
    tools::Long nHeight = rMutableOut.GetTextHeight();

    rMutableOut.SetFont(aOldFont);
    return nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
