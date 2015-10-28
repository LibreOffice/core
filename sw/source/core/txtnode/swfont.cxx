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
#include <unotools/localedatawrapper.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/charsetcoloritem.hxx>
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
#include <IDocumentSettingAccess.hxx>
#include <vcl/window.hxx>
#include <charatr.hxx>
#include <viewsh.hxx>
#include <swfont.hxx>
#include <fntcache.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>

#ifdef DBG_UTIL
// global Variable
SvStatistics g_SvStat;
#endif

using namespace ::com::sun::star;

// Hintergrundbrush setzen, z.B. bei Zeichenvorlagen
void SwFont::SetBackColor( Color* pNewColor )
{
    delete m_pBackColor;
    m_pBackColor = pNewColor;
    m_bFontChg = true;
    m_aSub[SW_LATIN].m_pMagic = m_aSub[SW_CJK].m_pMagic = m_aSub[SW_CTL].m_pMagic = 0;
}

void SwFont::SetTopBorder( const editeng::SvxBorderLine* pTopBorder )
{
    if( pTopBorder )
        m_aTopBorder = *pTopBorder;
    else
    {
        m_aTopBorder = boost::none;
        m_nTopBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SW_LATIN].m_pMagic = m_aSub[SW_CJK].m_pMagic = m_aSub[SW_CTL].m_pMagic = 0;
}

void SwFont::SetBottomBorder( const editeng::SvxBorderLine* pBottomBorder )
{
    if( pBottomBorder )
        m_aBottomBorder = *pBottomBorder;
    else
    {
        m_aBottomBorder = boost::none;
        m_nBottomBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SW_LATIN].m_pMagic = m_aSub[SW_CJK].m_pMagic = m_aSub[SW_CTL].m_pMagic = 0;
}

void SwFont::SetRightBorder( const editeng::SvxBorderLine* pRightBorder )
{
    if( pRightBorder )
        m_aRightBorder = *pRightBorder;
    else
    {
        m_aRightBorder = boost::none;
        m_nRightBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SW_LATIN].m_pMagic = m_aSub[SW_CJK].m_pMagic = m_aSub[SW_CTL].m_pMagic = 0;
}

void SwFont::SetLeftBorder( const editeng::SvxBorderLine* pLeftBorder )
{
    if( pLeftBorder )
        m_aLeftBorder = *pLeftBorder;
    else
    {
        m_aLeftBorder = boost::none;
        m_nLeftBorderDist = 0;
    }
    m_bFontChg = true;
    m_aSub[SW_LATIN].m_pMagic = m_aSub[SW_CJK].m_pMagic = m_aSub[SW_CTL].m_pMagic = 0;
}

const boost::optional<editeng::SvxBorderLine>&
SwFont::GetAbsTopBorder( const bool bVertLayout ) const
{
    switch( GetOrientation( bVertLayout ) )
    {
        case 0 :
            return m_aTopBorder;
            break;
        case 900 :
            return m_aRightBorder;
            break;
        case 1800 :
            return m_aBottomBorder;
            break;
        case 2700 :
            return m_aLeftBorder;
            break;
        default :
            assert(false);
            return m_aTopBorder;
            break;
    }
}

const boost::optional<editeng::SvxBorderLine>&
SwFont::GetAbsBottomBorder( const bool bVertLayout ) const
{
    switch( GetOrientation( bVertLayout ) )
    {
        case 0 :
            return m_aBottomBorder;
            break;
        case 900 :
            return m_aLeftBorder;
            break;
        case 1800 :
            return m_aTopBorder;
            break;
        case 2700 :
            return m_aRightBorder;
            break;
        default :
            assert(false);
            return m_aBottomBorder;
            break;
    }
}

const boost::optional<editeng::SvxBorderLine>&
SwFont::GetAbsLeftBorder( const bool bVertLayout ) const
{
    switch( GetOrientation( bVertLayout ) )
    {
        case 0 :
            return m_aLeftBorder;
            break;
        case 900 :
            return m_aTopBorder;
            break;
        case 1800 :
            return m_aRightBorder;
            break;
        case 2700 :
            return m_aBottomBorder;
            break;
        default :
            assert(false);
            return m_aLeftBorder;
            break;
    }
}

const boost::optional<editeng::SvxBorderLine>&
SwFont::GetAbsRightBorder( const bool bVertLayout ) const
{
    switch( GetOrientation( bVertLayout ) )
    {
        case 0 :
            return m_aRightBorder;
            break;
        case 900 :
            return m_aBottomBorder;
            break;
        case 1800 :
            return m_aLeftBorder;
            break;
        case 2700 :
            return m_aTopBorder;
            break;
        default :
            assert(false);
            return m_aRightBorder;
            break;
    }
}

SvxShadowLocation SwFont::GetAbsShadowLocation( const bool bVertLayout ) const
{
    SvxShadowLocation aLocation = SVX_SHADOW_NONE;
    switch( GetOrientation( bVertLayout ) )
    {
        case 0:
            aLocation = m_aShadowLocation;
            break;

        case 900:
            switch ( m_aShadowLocation )
            {
                case SVX_SHADOW_TOPLEFT:
                    aLocation = SVX_SHADOW_BOTTOMLEFT;
                    break;
                case SVX_SHADOW_TOPRIGHT:
                    aLocation = SVX_SHADOW_TOPLEFT;
                    break;
                case SVX_SHADOW_BOTTOMLEFT:
                    aLocation = SVX_SHADOW_BOTTOMRIGHT;
                    break;
                case SVX_SHADOW_BOTTOMRIGHT:
                    aLocation = SVX_SHADOW_TOPRIGHT;
                    break;
                case SVX_SHADOW_NONE:
                case SVX_SHADOW_END:
                    aLocation = m_aShadowLocation;
                    break;
            }
            break;

        case 1800:
            switch ( m_aShadowLocation )
            {
                case SVX_SHADOW_TOPLEFT:
                    aLocation = SVX_SHADOW_BOTTOMRIGHT;
                    break;
                case SVX_SHADOW_TOPRIGHT:
                    aLocation = SVX_SHADOW_BOTTOMLEFT;
                    break;
                case SVX_SHADOW_BOTTOMLEFT:
                    aLocation = SVX_SHADOW_TOPRIGHT;
                    break;
                case SVX_SHADOW_BOTTOMRIGHT:
                    aLocation = SVX_SHADOW_TOPLEFT;
                    break;
                case SVX_SHADOW_NONE:
                case SVX_SHADOW_END:
                    aLocation = m_aShadowLocation;
                    break;
            }
            break;

        case 2700:
            switch ( m_aShadowLocation )
            {
                case SVX_SHADOW_TOPLEFT:
                    aLocation = SVX_SHADOW_TOPRIGHT;
                    break;
                case SVX_SHADOW_TOPRIGHT:
                    aLocation = SVX_SHADOW_BOTTOMRIGHT;
                    break;
                case SVX_SHADOW_BOTTOMLEFT:
                    aLocation = SVX_SHADOW_TOPLEFT;
                    break;
                case SVX_SHADOW_BOTTOMRIGHT:
                    aLocation = SVX_SHADOW_BOTTOMLEFT;
                    break;
                case SVX_SHADOW_NONE:
                case SVX_SHADOW_END:
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

sal_uInt16 SwFont::CalcShadowSpace(
        const SvxShadowItemSide nShadow, const bool bVertLayout,
        const bool bSkipLeft, const bool bSkipRight ) const
{
    sal_uInt16 nSpace = 0;
    const sal_uInt16 nOrient = GetOrientation( bVertLayout );
    const SvxShadowLocation aLoc = GetAbsShadowLocation( bVertLayout );
    switch( nShadow )
    {
        case SvxShadowItemSide::TOP:
            if(( aLoc == SVX_SHADOW_TOPLEFT ||
               aLoc == SVX_SHADOW_TOPRIGHT ) &&
               ( nOrient == 0 || nOrient == 1800 ||
               ( nOrient == 900 && !bSkipRight ) ||
               ( nOrient == 2700 && !bSkipLeft )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SvxShadowItemSide::BOTTOM:
            if(( aLoc == SVX_SHADOW_BOTTOMLEFT ||
               aLoc == SVX_SHADOW_BOTTOMRIGHT ) &&
               ( nOrient == 0 || nOrient == 1800 ||
               ( nOrient == 900 && !bSkipLeft ) ||
               ( nOrient == 2700 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SvxShadowItemSide::LEFT:
            if(( aLoc == SVX_SHADOW_TOPLEFT ||
               aLoc == SVX_SHADOW_BOTTOMLEFT ) &&
               ( nOrient == 900 || nOrient == 2700 ||
               ( nOrient == 0 && !bSkipLeft ) ||
               ( nOrient == 1800 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

         case SvxShadowItemSide::RIGHT:
            if(( aLoc == SVX_SHADOW_TOPRIGHT ||
               aLoc == SVX_SHADOW_BOTTOMRIGHT ) &&
               ( nOrient == 900 || nOrient == 2700 ||
               ( nOrient == 0 && !bSkipRight ) ||
               ( nOrient == 1800 && !bSkipLeft )))
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

// maps directions for vertical layout
sal_uInt16 MapDirection( sal_uInt16 nDir, const bool bVertFormat )
{
    if ( bVertFormat )
    {
        switch ( nDir )
        {
        case 0 :
            nDir = 2700;
            break;
        case 900 :
            nDir = 0;
            break;
        case 2700 :
            nDir = 1800;
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

// maps the absolute direction set at the font to its logical conterpart
// in the rotated environment
sal_uInt16 UnMapDirection( sal_uInt16 nDir, const bool bVertFormat )
{
    if ( bVertFormat )
    {
        switch ( nDir )
        {
        case 0 :
            nDir = 900;
            break;
        case 1800 :
            nDir = 2700;
            break;
        case 2700 :
            nDir = 0;
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

sal_uInt16 SwFont::GetOrientation( const bool bVertFormat ) const
{
    return UnMapDirection( m_aSub[m_nActual].GetOrientation(), bVertFormat );
}

void SwFont::SetVertical( sal_uInt16 nDir, const bool bVertFormat )
{
    // map direction if frame has vertical layout
    nDir = MapDirection( nDir, bVertFormat );

    if( nDir != m_aSub[0].GetOrientation() )
    {
        m_bFontChg = true;
        m_aSub[0].SetVertical( nDir, bVertFormat );
        m_aSub[1].SetVertical( nDir, bVertFormat );
        m_aSub[2].SetVertical( nDir, bVertFormat );
    }
}

/*
 Escapement:
    frEsc:  Fraction, Grad des Escapements
    Esc = resultierendes Escapement
    A1 = Original-Ascent            (nOrgAscent)
    A2 = verkleinerter Ascent       (nEscAscent)
    Ax = resultierender Ascent      (GetAscent())
    H1 = Original-Hoehe             (nOrgHeight)
    H2 = verkleinerter Hoehe        (nEscHeight)
    Hx = resultierender Hoehe       (GetHeight())
    Bx = resultierende Baseline fuer die Textausgabe (CalcPos())
         (Vorsicht: Y - A1!)

    Escapement:
        Esc = H1 * frEsc;

    Hochstellung:
        Ax = A2 + Esc;
        Hx = H2 + Esc;
        Bx = A1 - Esc;

    Tiefstellung:
        Ax = A1;
        Hx = A1 + Esc + (H2 - A2);
        Bx = A1 + Esc;
*/

// nEsc ist der Prozentwert
sal_uInt16 SwSubFont::CalcEscAscent( const sal_uInt16 nOldAscent ) const
{
    if( DFLT_ESC_AUTO_SUPER != GetEscapement() &&
        DFLT_ESC_AUTO_SUB != GetEscapement() )
    {
        const long nAscent = nOldAscent +
                             ( (long) m_nOrgHeight * GetEscapement() ) / 100L;
        if ( nAscent>0 )
            return std::max<sal_uInt16>( nAscent, m_nOrgAscent );
    }
    return m_nOrgAscent;
}

void SwFont::SetDiffFnt( const SfxItemSet *pAttrSet,
                         const IDocumentSettingAccess *pIDocumentSettingAccess )
{
    delete m_pBackColor;
    m_pBackColor = NULL;

    if( pAttrSet )
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = static_cast<const SvxFontItem *>(pItem);
            m_aSub[SW_LATIN].SetFamily( pFont->GetFamily() );
            m_aSub[SW_LATIN].Font::SetName( pFont->GetFamilyName() );
            m_aSub[SW_LATIN].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SW_LATIN].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SW_LATIN].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = static_cast<const SvxFontHeightItem *>(pItem);
            m_aSub[SW_LATIN].SvxFont::SetPropr( 100 );
            m_aSub[SW_LATIN].m_aSize = m_aSub[SW_LATIN].Font::GetSize();
            Size aTmpSize = m_aSub[SW_LATIN].m_aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            m_aSub[SW_LATIN].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_POSTURE,
            true, &pItem ))
            m_aSub[SW_LATIN].Font::SetItalic( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_WEIGHT,
            true, &pItem ))
            m_aSub[SW_LATIN].Font::SetWeight( static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_LANGUAGE,
            true, &pItem ))
            m_aSub[SW_LATIN].SetLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = static_cast<const SvxFontItem *>(pItem);
            m_aSub[SW_CJK].SetFamily( pFont->GetFamily() );
            m_aSub[SW_CJK].Font::SetName( pFont->GetFamilyName() );
            m_aSub[SW_CJK].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SW_CJK].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SW_CJK].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = static_cast<const SvxFontHeightItem *>(pItem);
            m_aSub[SW_CJK].SvxFont::SetPropr( 100 );
            m_aSub[SW_CJK].m_aSize = m_aSub[SW_CJK].Font::GetSize();
            Size aTmpSize = m_aSub[SW_CJK].m_aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            m_aSub[SW_CJK].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_POSTURE,
            true, &pItem ))
            m_aSub[SW_CJK].Font::SetItalic( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_WEIGHT,
            true, &pItem ))
            m_aSub[SW_CJK].Font::SetWeight( static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_LANGUAGE,
            true, &pItem ))
        {
            LanguageType eNewLang = static_cast<const SvxLanguageItem*>(pItem)->GetLanguage();
            m_aSub[SW_CJK].SetLanguage( eNewLang );
            m_aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
            m_aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
            m_aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
        }

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = static_cast<const SvxFontItem *>(pItem);
            m_aSub[SW_CTL].SetFamily( pFont->GetFamily() );
            m_aSub[SW_CTL].Font::SetName( pFont->GetFamilyName() );
            m_aSub[SW_CTL].Font::SetStyleName( pFont->GetStyleName() );
            m_aSub[SW_CTL].Font::SetPitch( pFont->GetPitch() );
            m_aSub[SW_CTL].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = static_cast<const SvxFontHeightItem *>(pItem);
            m_aSub[SW_CTL].SvxFont::SetPropr( 100 );
            m_aSub[SW_CTL].m_aSize = m_aSub[SW_CTL].Font::GetSize();
            Size aTmpSize = m_aSub[SW_CTL].m_aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            m_aSub[SW_CTL].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_POSTURE,
            true, &pItem ))
            m_aSub[SW_CTL].Font::SetItalic( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_WEIGHT,
            true, &pItem ))
            m_aSub[SW_CTL].Font::SetWeight( static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_LANGUAGE,
            true, &pItem ))
            m_aSub[SW_CTL].SetLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_UNDERLINE,
            true, &pItem ))
        {
            SetUnderline( static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle() );
            SetUnderColor( static_cast<const SvxUnderlineItem*>(pItem)->GetColor() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_OVERLINE,
            true, &pItem ))
        {
            SetOverline( static_cast<const SvxOverlineItem*>(pItem)->GetLineStyle() );
            SetOverColor( static_cast<const SvxOverlineItem*>(pItem)->GetColor() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CROSSEDOUT,
            true, &pItem ))
            SetStrikeout( static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_COLOR,
            true, &pItem ))
            SetColor( static_cast<const SvxColorItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_EMPHASIS_MARK,
            true, &pItem ))
            SetEmphasisMark( static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark() );

        SetTransparent( true );
        SetAlign( ALIGN_BASELINE );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CONTOUR,
            true, &pItem ))
            SetOutline( static_cast<const SvxContourItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
            true, &pItem ))
            SetShadow( static_cast<const SvxShadowedItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_RELIEF,
            true, &pItem ))
            SetRelief( (FontRelief)static_cast<const SvxCharReliefItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
            true, &pItem ))
            SetPropWidth(static_cast<const SvxShadowedItem*>(pItem)->GetValue() ? 50 : 100 );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_AUTOKERN,
            true, &pItem ))
        {
            if( static_cast<const SvxAutoKernItem*>(pItem)->GetValue() )
            {
                SetAutoKern( ( !pIDocumentSettingAccess ||
                               !pIDocumentSettingAccess->get(DocumentSettingId::KERN_ASIAN_PUNCTUATION) ) ?
                                FontKerning::FontSpecific :
                                FontKerning::Asian );
            }
            else
                SetAutoKern( FontKerning::NONE );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_WORDLINEMODE,
            true, &pItem ))
            SetWordLineMode( static_cast<const SvxWordLineModeItem*>(pItem)->GetValue() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_ESCAPEMENT,
            true, &pItem ))
        {
            const SvxEscapementItem *pEsc = static_cast<const SvxEscapementItem *>(pItem);
            SetEscapement( pEsc->GetEsc() );
            if( m_aSub[SW_LATIN].IsEsc() )
                SetProportion( pEsc->GetProp() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CASEMAP,
            true, &pItem ))
            SetCaseMap( static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_KERNING,
            true, &pItem ))
            SetFixKerning( static_cast<const SvxKerningItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_NOHYPHEN,
            true, &pItem ))
            SetNoHyph( static_cast<const SvxNoHyphenItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BLINK,
            true, &pItem ))
            SetBlink( static_cast<const SvxBlinkItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_ROTATE,
            true, &pItem ))
            SetVertical( static_cast<const SvxCharRotateItem*>(pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
            true, &pItem ))
            m_pBackColor = new Color( static_cast<const SvxBrushItem*>(pItem)->GetColor() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_HIGHLIGHT,
            true, &pItem ))
            SetHighlightColor(static_cast<const SvxBrushItem*>(pItem)->GetColor());
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BOX,
            true, &pItem ))
        {
            const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
            SetTopBorder(pBoxItem->GetTop());
            SetBottomBorder(pBoxItem->GetBottom());
            SetRightBorder(pBoxItem->GetRight());
            SetLeftBorder(pBoxItem->GetLeft());
            SetTopBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::TOP));
            SetBottomBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::BOTTOM));
            SetRightBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::RIGHT));
            SetLeftBorderDist(pBoxItem->GetDistance(SvxBoxItemLine::LEFT));
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOW,
            true, &pItem ))
        {
            const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
            SetShadowColor(pShadowItem->GetColor());
            SetShadowWidth(pShadowItem->GetWidth());
            SetShadowLocation(pShadowItem->GetLocation());
        }
        const SfxPoolItem* pTwoLinesItem = 0;
        if( SfxItemState::SET ==
                pAttrSet->GetItemState( RES_CHRATR_TWO_LINES, true, &pTwoLinesItem ))
            if ( static_cast<const SvxTwoLinesItem*>(pTwoLinesItem)->GetValue() )
                SetVertical( 0 );
    }
    else
    {
        Invalidate();
        m_bNoHyph = false;
        m_bBlink = false;
    }
    m_bPaintBlank = false;
    m_bPaintWrong = false;
    OSL_ENSURE( m_aSub[SW_LATIN].IsTransparent(), "SwFont: Transparent revolution" );
}

SwFont::SwFont( const SwFont &rFont )
{
    m_aSub[SW_LATIN] = rFont.m_aSub[SW_LATIN];
    m_aSub[SW_CJK] = rFont.m_aSub[SW_CJK];
    m_aSub[SW_CTL] = rFont.m_aSub[SW_CTL];
    m_nActual = rFont.m_nActual;
    m_pBackColor = rFont.m_pBackColor ? new Color( *rFont.m_pBackColor ) : NULL;
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
    m_nInputFieldCount = 0;
    m_bFontChg = rFont.m_bFontChg;
    m_bOrgChg = rFont.m_bOrgChg;
    m_bPaintBlank = rFont.m_bPaintBlank;
    m_bPaintWrong = false;
    m_bURL = rFont.m_bURL;
    m_bGreyWave = rFont.m_bGreyWave;
    m_bNoColorReplace = rFont.m_bNoColorReplace;
    m_bNoHyph = rFont.m_bNoHyph;
    m_bBlink = rFont.m_bBlink;
}

SwFont::SwFont( const SwAttrSet* pAttrSet,
                const IDocumentSettingAccess* pIDocumentSettingAccess )
{
    m_nActual = SW_LATIN;
    m_nToxCount = 0;
    m_nRefCount = 0;
    m_nMetaCount = 0;
    m_nInputFieldCount = 0;
    m_bPaintBlank = false;
    m_bPaintWrong = false;
    m_bURL = false;
    m_bGreyWave = false;
    m_bNoColorReplace = false;
    m_bNoHyph = pAttrSet->GetNoHyphenHere().GetValue();
    m_bBlink = pAttrSet->GetBlink().GetValue();
    m_bOrgChg = true;
    {
        const SvxFontItem& rFont = pAttrSet->GetFont();
        m_aSub[SW_LATIN].SetFamily( rFont.GetFamily() );
        m_aSub[SW_LATIN].SetName( rFont.GetFamilyName() );
        m_aSub[SW_LATIN].SetStyleName( rFont.GetStyleName() );
        m_aSub[SW_LATIN].SetPitch( rFont.GetPitch() );
        m_aSub[SW_LATIN].SetCharSet( rFont.GetCharSet() );
        m_aSub[SW_LATIN].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = m_aSub[SW_LATIN].m_aSize;
        aTmpSize.Height() = pAttrSet->GetSize().GetHeight();
        m_aSub[SW_LATIN].SetSize( aTmpSize );
        m_aSub[SW_LATIN].SetItalic( pAttrSet->GetPosture().GetPosture() );
        m_aSub[SW_LATIN].SetWeight( pAttrSet->GetWeight().GetWeight() );
        m_aSub[SW_LATIN].SetLanguage( pAttrSet->GetLanguage().GetLanguage() );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCJKFont();
        m_aSub[SW_CJK].SetFamily( rFont.GetFamily() );
        m_aSub[SW_CJK].SetName( rFont.GetFamilyName() );
        m_aSub[SW_CJK].SetStyleName( rFont.GetStyleName() );
        m_aSub[SW_CJK].SetPitch( rFont.GetPitch() );
        m_aSub[SW_CJK].SetCharSet( rFont.GetCharSet() );
        m_aSub[SW_CJK].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = m_aSub[SW_CJK].m_aSize;
        aTmpSize.Height() = pAttrSet->GetCJKSize().GetHeight();
        m_aSub[SW_CJK].SetSize( aTmpSize );
        m_aSub[SW_CJK].SetItalic( pAttrSet->GetCJKPosture().GetPosture() );
        m_aSub[SW_CJK].SetWeight( pAttrSet->GetCJKWeight().GetWeight() );
        LanguageType eNewLang = pAttrSet->GetCJKLanguage().GetLanguage();
        m_aSub[SW_CJK].SetLanguage( eNewLang );
        m_aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
        m_aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
        m_aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCTLFont();
        m_aSub[SW_CTL].SetFamily( rFont.GetFamily() );
        m_aSub[SW_CTL].SetName( rFont.GetFamilyName() );
        m_aSub[SW_CTL].SetStyleName( rFont.GetStyleName() );
        m_aSub[SW_CTL].SetPitch( rFont.GetPitch() );
        m_aSub[SW_CTL].SetCharSet( rFont.GetCharSet() );
        m_aSub[SW_CTL].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = m_aSub[SW_CTL].m_aSize;
        aTmpSize.Height() = pAttrSet->GetCTLSize().GetHeight();
        m_aSub[SW_CTL].SetSize( aTmpSize );
        m_aSub[SW_CTL].SetItalic( pAttrSet->GetCTLPosture().GetPosture() );
        m_aSub[SW_CTL].SetWeight( pAttrSet->GetCTLWeight().GetWeight() );
        m_aSub[SW_CTL].SetLanguage( pAttrSet->GetCTLLanguage().GetLanguage() );
    }
    if ( pAttrSet->GetCharHidden().GetValue() )
        SetUnderline( UNDERLINE_DOTTED );
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
    SetRelief( (FontRelief)pAttrSet->GetCharRelief().GetValue() );
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
    if( m_aSub[SW_LATIN].IsEsc() )
        SetProportion( rEsc.GetProp() );
    SetCaseMap( pAttrSet->GetCaseMap().GetCaseMap() );
    SetFixKerning( pAttrSet->GetKerning().GetValue() );
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
        true, &pItem ))
        m_pBackColor = new Color( static_cast<const SvxBrushItem*>(pItem)->GetColor() );
    else
        m_pBackColor = NULL;
    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_HIGHLIGHT,
        true, &pItem ))
        SetHighlightColor(static_cast<const SvxBrushItem*>(pItem)->GetColor());
    else
        SetHighlightColor(COL_TRANSPARENT);
    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BOX,
        true, &pItem ))
    {
        const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
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
        SetTopBorder(0);
        SetBottomBorder(0);
        SetRightBorder(0);
        SetLeftBorder(0);
        SetTopBorderDist(0);
        SetBottomBorderDist(0);
        SetRightBorderDist(0);
        SetLeftBorderDist(0);
    }

    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOW,
        true, &pItem ))
    {
        const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
        SetShadowColor(pShadowItem->GetColor());
        SetShadowWidth(pShadowItem->GetWidth());
        SetShadowLocation(pShadowItem->GetLocation());
    }
    else
    {
        SetShadowColor(COL_TRANSPARENT);
        SetShadowWidth(0);
        SetShadowLocation(SVX_SHADOW_NONE);
    }

    const SvxTwoLinesItem& rTwoLinesItem = pAttrSet->Get2Lines();
    if ( ! rTwoLinesItem.GetValue() )
        SetVertical( pAttrSet->GetCharRotate().GetValue() );
    else
        SetVertical( 0 );
    if( pIDocumentSettingAccess && pIDocumentSettingAccess->get( DocumentSettingId::SMALL_CAPS_PERCENTAGE_66 ))
    {
        m_aSub[ SW_LATIN ].m_bSmallCapsPercentage66 = true;
        m_aSub[ SW_CJK ].m_bSmallCapsPercentage66 = true;
        m_aSub[ SW_CTL ].m_bSmallCapsPercentage66 = true;
    }
}

SwFont::~SwFont()
{
    delete m_pBackColor;
}

SwSubFont& SwSubFont::operator=( const SwSubFont &rFont )
{
    SvxFont::operator=( rFont );
    m_pMagic = rFont.m_pMagic;
    m_nFontIndex = rFont.m_nFontIndex;
    m_nOrgHeight = rFont.m_nOrgHeight;
    m_nOrgAscent = rFont.m_nOrgAscent;
    m_nProportionalWidth = rFont.m_nProportionalWidth;
    m_aSize = rFont.m_aSize;
    m_bSmallCapsPercentage66 = rFont.m_bSmallCapsPercentage66;
    return *this;
}

SwFont& SwFont::operator=( const SwFont &rFont )
{
    m_aSub[SW_LATIN] = rFont.m_aSub[SW_LATIN];
    m_aSub[SW_CJK] = rFont.m_aSub[SW_CJK];
    m_aSub[SW_CTL] = rFont.m_aSub[SW_CTL];
    m_nActual = rFont.m_nActual;
    delete m_pBackColor;
    m_pBackColor = rFont.m_pBackColor ? new Color( *rFont.m_pBackColor ) : NULL;
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
    m_nInputFieldCount = 0;
    m_bFontChg = rFont.m_bFontChg;
    m_bOrgChg = rFont.m_bOrgChg;
    m_bPaintBlank = rFont.m_bPaintBlank;
    m_bPaintWrong = false;
    m_bURL = rFont.m_bURL;
    m_bGreyWave = rFont.m_bGreyWave;
    m_bNoColorReplace = rFont.m_bNoColorReplace;
    m_bNoHyph = rFont.m_bNoHyph;
    m_bBlink = rFont.m_bBlink;
    return *this;
}

void SwFont::GoMagic( SwViewShell *pSh, sal_uInt8 nWhich )
{
    SwFntAccess aFntAccess( m_aSub[nWhich].m_pMagic, m_aSub[nWhich].m_nFontIndex,
                            &m_aSub[nWhich], pSh, true );
}

bool SwSubFont::IsSymbol( SwViewShell *pSh )
{
    SwFntAccess aFntAccess( m_pMagic, m_nFontIndex, this, pSh, false );
    return aFntAccess.Get()->IsSymbol();
}

bool SwSubFont::ChgFnt( SwViewShell const *pSh, OutputDevice& rOut )
{
    if ( pLastFont )
        pLastFont->Unlock();
    SwFntAccess aFntAccess( m_pMagic, m_nFontIndex, this, pSh, true );
    SV_STAT( nChangeFont );

    pLastFont = aFntAccess.Get();

    pLastFont->SetDevFont( pSh, rOut );

    pLastFont->Lock();
    return UNDERLINE_NONE != GetUnderline() ||
           UNDERLINE_NONE != GetOverline()  ||
           STRIKEOUT_NONE != GetStrikeout();
}

void SwFont::ChgPhysFnt( SwViewShell *pSh, OutputDevice& rOut )
{
    if( m_bOrgChg && m_aSub[m_nActual].IsEsc() )
    {
        const sal_uInt8 nOldProp = m_aSub[m_nActual].GetPropr();
        SetProportion( 100 );
        ChgFnt( pSh, rOut );
        SwFntAccess aFntAccess( m_aSub[m_nActual].m_pMagic, m_aSub[m_nActual].m_nFontIndex,
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
        long nDescent = nOldHeight - nOldAscent -
                             ( (long) m_nOrgHeight * GetEscapement() ) / 100L;
        const sal_uInt16 nDesc = nDescent>0
                ? std::max<sal_uInt16>( nDescent, m_nOrgHeight - m_nOrgAscent)
                : m_nOrgHeight - m_nOrgAscent;
        return ( nDesc + CalcEscAscent( nOldAscent ) );
    }
    return m_nOrgHeight;
}

short SwSubFont::_CheckKerning( )
{
    short nKernx = - short( Font::GetSize().Height() / 6 );

    if ( nKernx < GetFixKerning() )
        return GetFixKerning();
    return nKernx;
}

sal_uInt16 SwSubFont::GetAscent( SwViewShell *pSh, const OutputDevice& rOut )
{
    SwFntAccess aFntAccess( m_pMagic, m_nFontIndex, this, pSh );
    const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
    return GetEscapement() ? CalcEscAscent( nAscent ) : nAscent;
}

sal_uInt16 SwSubFont::GetHeight( SwViewShell *pSh, const OutputDevice& rOut )
{
    SV_STAT( nGetTextSize );
    SwFntAccess aFntAccess( m_pMagic, m_nFontIndex, this, pSh );
    const sal_uInt16 nHeight = aFntAccess.Get()->GetFontHeight( pSh, rOut );
    if ( GetEscapement() )
    {
        const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
        return CalcEscHeight( nHeight, nAscent ); // + nLeading;
    }
    return nHeight; // + nLeading;
}

Size SwSubFont::_GetTextSize( SwDrawTextInfo& rInf )
{
    // Robust: Eigentlich sollte der Font bereits eingestellt sein, aber
    // sicher ist sicher ...
    if ( !pLastFont || pLastFont->GetOwner()!=m_pMagic ||
         !IsSameInstance( rInf.GetpOut()->GetFont() ) )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier( rInf.GetOut(), rInf.GetFont()->GetLanguage() );

    Size aTextSize;
    sal_Int32 nLn = ( rInf.GetLen() == COMPLETE_STRING ? rInf.GetText().getLength()
                                                   : rInf.GetLen() );
    rInf.SetLen( nLn );
    if( IsCapital() && nLn )
        aTextSize = GetCapitalSize( rInf );
    else
    {
        SV_STAT( nGetTextSize );
        long nOldKern = rInf.GetKern();
        const OUString oldText = rInf.GetText();
        rInf.SetKern( CheckKerning() );
        if ( !IsCaseMap() )
            aTextSize = pLastFont->GetTextSize( rInf );
        else
        {
            OUString aTmp = CalcCaseMap( rInf.GetText() );
            const OUString oldStr = rInf.GetText();
            bool bCaseMapLengthDiffers(aTmp.getLength() != oldStr.getLength());

            if(bCaseMapLengthDiffers && rInf.GetLen())
            {
                // If the length of the original string and the CaseMapped one
                // are different, it is necessary to handle the given text part as
                // a single snippet since its size may differ, too.
                sal_Int32 nOldIdx(rInf.GetIdx());
                sal_Int32 nOldLen(rInf.GetLen());
                const OUString aSnippet(oldStr.copy(nOldIdx, nOldLen));
                OUString aNewText(CalcCaseMap(aSnippet));

                rInf.SetText( aNewText );
                rInf.SetIdx( 0 );
                rInf.SetLen( aNewText.getLength() );

                aTextSize = pLastFont->GetTextSize( rInf );

                rInf.SetIdx( nOldIdx );
                rInf.SetLen( nOldLen );
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
        // 15142: Ein Wort laenger als eine Zeile, beim Zeilenumbruch
        //        hochgestellt, muss seine effektive Hoehe melden.
        if( GetEscapement() )
        {
            const sal_uInt16 nAscent = pLastFont->GetFontAscent( rInf.GetShell(),
                                                             rInf.GetOut() );
            aTextSize.Height() =
                (long)CalcEscHeight( (sal_uInt16)aTextSize.Height(), nAscent);
        }
    }

    if (1==rInf.GetLen() && CH_TXT_ATR_FIELDSTART==rInf.GetText()[rInf.GetIdx()])
    {
        sal_Int32 nOldIdx(rInf.GetIdx());
        sal_Int32 nOldLen(rInf.GetLen());
        OUString aNewText(CH_TXT_ATR_SUBST_FIELDSTART);
        rInf.SetText( aNewText );
        rInf.SetIdx( 0 );
        rInf.SetLen( aNewText.getLength() );
        aTextSize = pLastFont->GetTextSize( rInf );
        rInf.SetIdx( nOldIdx );
        rInf.SetLen( nOldLen );
    }
    else if (1==rInf.GetLen() && CH_TXT_ATR_FIELDEND==rInf.GetText()[ rInf.GetIdx() ])
    {
        sal_Int32 nOldIdx(rInf.GetIdx());
        sal_Int32 nOldLen(rInf.GetLen());
        OUString aNewText(CH_TXT_ATR_SUBST_FIELDEND);
        rInf.SetText( aNewText );
        rInf.SetIdx( 0 );
        rInf.SetLen( aNewText.getLength() );
        aTextSize = pLastFont->GetTextSize( rInf );
        rInf.SetIdx( nOldIdx );
        rInf.SetLen( nOldLen );
    }

    return aTextSize;
}

void SwSubFont::_DrawText( SwDrawTextInfo &rInf, const bool bGrey )
{
    rInf.SetGreyWave( bGrey );
    sal_Int32 nLn = rInf.GetText().getLength();
    if( !rInf.GetLen() || !nLn )
        return;
    if( COMPLETE_STRING == rInf.GetLen() )
        rInf.SetLen( nLn );

    FontUnderline nOldUnder = UNDERLINE_NONE;
    SwUnderlineFont* pUnderFnt = 0;

    if( rInf.GetUnderFnt() )
    {
        nOldUnder = GetUnderline();
        SetUnderline( UNDERLINE_NONE );
        pUnderFnt = rInf.GetUnderFnt();
    }

    if( !pLastFont || pLastFont->GetOwner()!=m_pMagic )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier( rInf.GetOut(), rInf.GetFont()->GetLanguage() );

    const Point aOldPos(rInf.GetPos());
    Point aPos( rInf.GetPos() );

    if( GetEscapement() )
        CalcEsc( rInf, aPos );

    rInf.SetPos( aPos );
    rInf.SetKern( CheckKerning() + rInf.GetSperren() / SPACING_PRECISION_FACTOR );

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
            OUString aString( CalcCaseMap(oldStr) );
            bool bCaseMapLengthDiffers(aString.getLength() != oldStr.getLength());

            if(bCaseMapLengthDiffers && rInf.GetLen())
            {
                // If the length of the original string and the CaseMapped one
                // are different, it is necessary to handle the given text part as
                // a single snippet since its size may differ, too.
                sal_Int32 nOldIdx(rInf.GetIdx());
                sal_Int32 nOldLen(rInf.GetLen());
                const OUString aSnippet(oldStr.copy(nOldIdx, nOldLen));
                OUString aNewText = CalcCaseMap(aSnippet);

                rInf.SetText( aNewText );
                rInf.SetIdx( 0 );
                rInf.SetLen( aNewText.getLength() );

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

    if( pUnderFnt && nOldUnder != UNDERLINE_NONE )
    {
        Size aFontSize = _GetTextSize( rInf );
        const OUString oldStr = rInf.GetText();
        OUString aStr("  ");

        sal_Int32 nOldIdx = rInf.GetIdx();
        sal_Int32 nOldLen = rInf.GetLen();
        long nSpace = 0;
        if( rInf.GetSpace() )
        {
            sal_Int32 nTmpEnd = nOldIdx + nOldLen;
            if (nTmpEnd > oldStr.getLength())
                nTmpEnd = oldStr.getLength();

            const SwScriptInfo* pSI = rInf.GetScriptInfo();

            const bool bAsianFont =
                ( rInf.GetFont() && SW_CJK == rInf.GetFont()->GetActual() );
            for( sal_Int32 nTmp = nOldIdx; nTmp < nTmpEnd; ++nTmp )
            {
                if (CH_BLANK == oldStr[nTmp] || bAsianFont ||
                    ( nTmp + 1 < oldStr.getLength() && pSI &&
                      i18n::ScriptType::ASIAN == pSI->ScriptType( nTmp + 1 ) ) )
                    ++nSpace;
            }

            // if next portion if a hole portion we do not consider any
            // extra space added because the last character was ASIAN
            if ( nSpace && rInf.IsSpaceStop() && bAsianFont )
                 --nSpace;

            nSpace *= rInf.GetSpace() / SPACING_PRECISION_FACTOR;
        }

        rInf.SetWidth( sal_uInt16(aFontSize.Width() + nSpace) );
        rInf.SetText( aStr );
        rInf.SetIdx( 0 );
        rInf.SetLen( 2 );
        SetUnderline( nOldUnder );
        rInf.SetUnderFnt( 0 );

        // set position for underline font
        rInf.SetPos( pUnderFnt->GetPos() );

        pUnderFnt->GetFont()._DrawStretchText( rInf );

        rInf.SetUnderFnt( pUnderFnt );
        rInf.SetText(oldStr);
        rInf.SetIdx( nOldIdx );
        rInf.SetLen( nOldLen );
    }

    rInf.SetPos(aOldPos);
}

void SwSubFont::_DrawStretchText( SwDrawTextInfo &rInf )
{
    if( !rInf.GetLen() || !rInf.GetText().getLength() )
        return;

    FontUnderline nOldUnder = UNDERLINE_NONE;
    SwUnderlineFont* pUnderFnt = 0;

    if( rInf.GetUnderFnt() )
    {
        nOldUnder = GetUnderline();
        SetUnderline( UNDERLINE_NONE );
        pUnderFnt = rInf.GetUnderFnt();
    }

    if ( !pLastFont || pLastFont->GetOwner() != m_pMagic )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier( rInf.GetOut(), rInf.GetFont()->GetLanguage() );

    rInf.ApplyAutoColor();

    const Point aOldPos(rInf.GetPos());
    Point aPos( rInf.GetPos() );

    if( GetEscapement() )
        CalcEsc( rInf, aPos );

    rInf.SetKern( CheckKerning() + rInf.GetSperren() / SPACING_PRECISION_FACTOR );
    rInf.SetPos( aPos );

    if( IsCapital() )
        DrawStretchCapital( rInf );
    else
    {
        SV_STAT( nDrawStretchText );

        if ( rInf.GetFrm() )
        {
            if ( rInf.GetFrm()->IsRightToLeft() )
                rInf.GetFrm()->SwitchLTRtoRTL( aPos );

            if ( rInf.GetFrm()->IsVertical() )
                rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

            rInf.SetPos( aPos );
        }

        if ( !IsCaseMap() )
            rInf.GetOut().DrawStretchText( aPos, rInf.GetWidth(),
                            rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
        else
            rInf.GetOut().DrawStretchText( aPos, rInf.GetWidth(), CalcCaseMap(
                            rInf.GetText() ), rInf.GetIdx(), rInf.GetLen() );
    }

    if( pUnderFnt && nOldUnder != UNDERLINE_NONE )
    {
        const OUString oldStr = rInf.GetText();
        OUString aStr("  ");
        sal_Int32 nOldIdx = rInf.GetIdx();
        sal_Int32 nOldLen = rInf.GetLen();
        rInf.SetText( aStr );
        rInf.SetIdx( 0 );
        rInf.SetLen( 2 );
        SetUnderline( nOldUnder );
        rInf.SetUnderFnt( 0 );

        // set position for underline font
        rInf.SetPos( pUnderFnt->GetPos() );

        pUnderFnt->GetFont()._DrawStretchText( rInf );

        rInf.SetUnderFnt( pUnderFnt );
        rInf.SetText(oldStr);
        rInf.SetIdx( nOldIdx );
        rInf.SetLen( nOldLen );
    }

    rInf.SetPos(aOldPos);
}

sal_Int32 SwSubFont::_GetCrsrOfst( SwDrawTextInfo& rInf )
{
    if ( !pLastFont || pLastFont->GetOwner()!=m_pMagic )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier( rInf.GetOut(), rInf.GetFont()->GetLanguage() );

    sal_Int32 nLn = rInf.GetLen() == COMPLETE_STRING ? rInf.GetText().getLength()
                                                 : rInf.GetLen();
    rInf.SetLen( nLn );
    sal_Int32 nCrsr = 0;
    if( IsCapital() && nLn )
        nCrsr = GetCapitalCrsrOfst( rInf );
    else
    {
        const OUString oldText = rInf.GetText();
        long nOldKern = rInf.GetKern();
        rInf.SetKern( CheckKerning() );
        SV_STAT( nGetTextSize );
        if ( !IsCaseMap() )
            nCrsr = pLastFont->GetCrsrOfst( rInf );
        else
        {
            OUString aTmp = CalcCaseMap( rInf.GetText() );
            rInf.SetText( aTmp );
            nCrsr = pLastFont->GetCrsrOfst( rInf );
        }
        rInf.SetKern( nOldKern );
        rInf.SetText(oldText);
    }
    return nCrsr;
}

void SwSubFont::CalcEsc( SwDrawTextInfo& rInf, Point& rPos )
{
    long nOfst;

    const sal_uInt16 nDir = UnMapDirection(
                GetOrientation(), rInf.GetFrm() && rInf.GetFrm()->IsVertical() );

    switch ( GetEscapement() )
    {
    case DFLT_ESC_AUTO_SUB :
        nOfst = m_nOrgHeight - m_nOrgAscent -
            pLastFont->GetFontHeight( rInf.GetShell(), rInf.GetOut() ) +
            pLastFont->GetFontAscent( rInf.GetShell(), rInf.GetOut() );

        switch ( nDir )
        {
        case 0 :
            rPos.Y() += nOfst;
            break;
        case 900 :
            rPos.X() += nOfst;
            break;
        case 2700 :
            rPos.X() -= nOfst;
            break;
        }

        break;
    case DFLT_ESC_AUTO_SUPER :
        nOfst = pLastFont->GetFontAscent( rInf.GetShell(), rInf.GetOut() ) -
                m_nOrgAscent;

        switch ( nDir )
        {
        case 0 :
            rPos.Y() += nOfst;
            break;
        case 900 :
            rPos.X() += nOfst;
            break;
        case 2700 :
            rPos.X() -= nOfst;
            break;
        }

        break;
    default :
        nOfst = ((long)m_nOrgHeight * GetEscapement()) / 100L;

        switch ( nDir )
        {
        case 0 :
            rPos.Y() -= nOfst;
            break;
        case 900 :
            rPos.X() -= nOfst;
            break;
        case 2700 :
            rPos.X() += nOfst;
            break;
        }
    }
}

// used during painting of small capitals
void SwDrawTextInfo::Shift( sal_uInt16 nDir )
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_bPos, "DrawTextInfo: Undefined Position" );
    OSL_ENSURE( m_bSize, "DrawTextInfo: Undefined Width" );
#endif

    const bool bBidiPor = ( GetFrm() && GetFrm()->IsRightToLeft() ) !=
                          ( TEXT_LAYOUT_DEFAULT != ( TEXT_LAYOUT_BIDI_RTL & GetpOut()->GetLayoutMode() ) );

    nDir = bBidiPor ?
            1800 :
            UnMapDirection( nDir, GetFrm() && GetFrm()->IsVertical() );

    switch ( nDir )
    {
    case 0 :
        m_aPos.X() += GetSize().Width();
        break;
    case 900 :
        OSL_ENSURE( m_aPos.Y() >= GetSize().Width(), "Going underground" );
        m_aPos.Y() -= GetSize().Width();
        break;
    case 1800 :
        m_aPos.X() -= GetSize().Width();
        break;
    case 2700 :
        m_aPos.Y() += GetSize().Width();
        break;
    }
}

/**
 * @note Used for the "continuous underline" feature.
 **/
SwUnderlineFont::SwUnderlineFont( SwFont& rFnt, const Point& rPoint )
        : m_aPos( rPoint ), m_pFont( &rFnt )
{
};

SwUnderlineFont::~SwUnderlineFont()
{
     delete m_pFont;
}

/// Helper for filters to find true lineheight of a font
long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const vcl::RenderContext &rOut, sal_Int16 nScript)
{
    SwFont aFont(&rSet, &rIDocumentSettingAccess);
    sal_uInt8 nActual;
    switch (nScript)
    {
        default:
        case i18n::ScriptType::LATIN:
            nActual = SW_LATIN;
            break;
        case i18n::ScriptType::ASIAN:
            nActual = SW_CJK;
            break;
        case i18n::ScriptType::COMPLEX:
            nActual = SW_CTL;
            break;
    }
    aFont.SetActual(nActual);

    vcl::RenderContext &rMutableOut = const_cast<vcl::RenderContext &>(rOut);
    const vcl::Font aOldFont(rMutableOut.GetFont());

    rMutableOut.SetFont(aFont.GetActualFont());
    long nHeight = rMutableOut.GetTextHeight();

    rMutableOut.SetFont(aOldFont);
    return nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
