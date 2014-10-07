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
    delete pBackColor;
    pBackColor = pNewColor;
    bFntChg = true;
    aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
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
    bFntChg = true;
    aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
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
    bFntChg = true;
    aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
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
    bFntChg = true;
    aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
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
    bFntChg = true;
    aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
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
        const sal_uInt16 nShadow, const bool bVertLayout,
        const bool bSkipLeft, const bool bSkipRight ) const
{
    sal_uInt16 nSpace = 0;
    const sal_uInt16 nOrient = GetOrientation( bVertLayout );
    const SvxShadowLocation aLoc = GetAbsShadowLocation( bVertLayout );
    switch( nShadow )
    {
        case SHADOW_TOP:
            if(( aLoc == SVX_SHADOW_TOPLEFT ||
               aLoc == SVX_SHADOW_TOPRIGHT ) &&
               ( nOrient == 0 || nOrient == 1800 ||
               ( nOrient == 900 && !bSkipRight ) ||
               ( nOrient == 2700 && !bSkipLeft )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SHADOW_BOTTOM:
            if(( aLoc == SVX_SHADOW_BOTTOMLEFT ||
               aLoc == SVX_SHADOW_BOTTOMRIGHT ) &&
               ( nOrient == 0 || nOrient == 1800 ||
               ( nOrient == 900 && !bSkipLeft ) ||
               ( nOrient == 2700 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

        case SHADOW_LEFT:
            if(( aLoc == SVX_SHADOW_TOPLEFT ||
               aLoc == SVX_SHADOW_BOTTOMLEFT ) &&
               ( nOrient == 900 || nOrient == 2700 ||
               ( nOrient == 0 && !bSkipLeft ) ||
               ( nOrient == 1800 && !bSkipRight )))
            {
                nSpace = m_nShadowWidth;
            }
            break;

         case SHADOW_RIGHT:
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
    return UnMapDirection( aSub[nActual].GetOrientation(), bVertFormat );
}

void SwFont::SetVertical( sal_uInt16 nDir, const bool bVertFormat )
{
    // map direction if frame has vertical layout
    nDir = MapDirection( nDir, bVertFormat );

    if( nDir != aSub[0].GetOrientation() )
    {
        bFntChg = true;
        aSub[0].SetVertical( nDir, bVertFormat );
        aSub[1].SetVertical( nDir, bVertFormat || nDir > 1000 );
        aSub[2].SetVertical( nDir, bVertFormat );
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
                             ( (long) nOrgHeight * GetEscapement() ) / 100L;
        if ( nAscent>0 )
            return std::max<sal_uInt16>( nAscent, nOrgAscent );
    }
    return nOrgAscent;
}

void SwFont::SetDiffFnt( const SfxItemSet *pAttrSet,
                         const IDocumentSettingAccess *pIDocumentSettingAccess )
{
    delete pBackColor;
    pBackColor = NULL;

    if( pAttrSet )
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = (const SvxFontItem *)pItem;
            aSub[SW_LATIN].SetFamily( pFont->GetFamily() );
            aSub[SW_LATIN].Font::SetName( pFont->GetFamilyName() );
            aSub[SW_LATIN].Font::SetStyleName( pFont->GetStyleName() );
            aSub[SW_LATIN].Font::SetPitch( pFont->GetPitch() );
            aSub[SW_LATIN].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
            aSub[SW_LATIN].SvxFont::SetPropr( 100 );
            aSub[SW_LATIN].aSize = aSub[SW_LATIN].Font::GetSize();
            Size aTmpSize = aSub[SW_LATIN].aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            aSub[SW_LATIN].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_POSTURE,
            true, &pItem ))
            aSub[SW_LATIN].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_WEIGHT,
            true, &pItem ))
            aSub[SW_LATIN].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_LANGUAGE,
            true, &pItem ))
            aSub[SW_LATIN].SetLanguage( ((SvxLanguageItem*)pItem)->GetLanguage() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = (const SvxFontItem *)pItem;
            aSub[SW_CJK].SetFamily( pFont->GetFamily() );
            aSub[SW_CJK].Font::SetName( pFont->GetFamilyName() );
            aSub[SW_CJK].Font::SetStyleName( pFont->GetStyleName() );
            aSub[SW_CJK].Font::SetPitch( pFont->GetPitch() );
            aSub[SW_CJK].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
            aSub[SW_CJK].SvxFont::SetPropr( 100 );
            aSub[SW_CJK].aSize = aSub[SW_CJK].Font::GetSize();
            Size aTmpSize = aSub[SW_CJK].aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            aSub[SW_CJK].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_POSTURE,
            true, &pItem ))
            aSub[SW_CJK].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_WEIGHT,
            true, &pItem ))
            aSub[SW_CJK].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CJK_LANGUAGE,
            true, &pItem ))
        {
            LanguageType eNewLang = ((SvxLanguageItem*)pItem)->GetLanguage();
            aSub[SW_CJK].SetLanguage( eNewLang );
            aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
            aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
            aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
        }

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONT,
            true, &pItem ))
        {
            const SvxFontItem *pFont = (const SvxFontItem *)pItem;
            aSub[SW_CTL].SetFamily( pFont->GetFamily() );
            aSub[SW_CTL].Font::SetName( pFont->GetFamilyName() );
            aSub[SW_CTL].Font::SetStyleName( pFont->GetStyleName() );
            aSub[SW_CTL].Font::SetPitch( pFont->GetPitch() );
            aSub[SW_CTL].Font::SetCharSet( pFont->GetCharSet() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONTSIZE,
            true, &pItem ))
        {
            const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
            aSub[SW_CTL].SvxFont::SetPropr( 100 );
            aSub[SW_CTL].aSize = aSub[SW_CTL].Font::GetSize();
            Size aTmpSize = aSub[SW_CTL].aSize;
            aTmpSize.Height() = pHeight->GetHeight();
            aSub[SW_CTL].SetSize( aTmpSize );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_POSTURE,
            true, &pItem ))
            aSub[SW_CTL].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_WEIGHT,
            true, &pItem ))
            aSub[SW_CTL].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CTL_LANGUAGE,
            true, &pItem ))
            aSub[SW_CTL].SetLanguage( ((SvxLanguageItem*)pItem)->GetLanguage() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_UNDERLINE,
            true, &pItem ))
        {
            SetUnderline( ((SvxUnderlineItem*)pItem)->GetLineStyle() );
            SetUnderColor( ((SvxUnderlineItem*)pItem)->GetColor() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_OVERLINE,
            true, &pItem ))
        {
            SetOverline( ((SvxOverlineItem*)pItem)->GetLineStyle() );
            SetOverColor( ((SvxOverlineItem*)pItem)->GetColor() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CROSSEDOUT,
            true, &pItem ))
            SetStrikeout( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_COLOR,
            true, &pItem ))
            SetColor( ((SvxColorItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_EMPHASIS_MARK,
            true, &pItem ))
            SetEmphasisMark( ((SvxEmphasisMarkItem*)pItem)->GetEmphasisMark() );

        SetTransparent( true );
        SetAlign( ALIGN_BASELINE );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CONTOUR,
            true, &pItem ))
            SetOutline( ((SvxContourItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
            true, &pItem ))
            SetShadow( ((SvxShadowedItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_RELIEF,
            true, &pItem ))
            SetRelief( (FontRelief)((SvxCharReliefItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
            true, &pItem ))
            SetPropWidth(((SvxShadowedItem*)pItem)->GetValue() ? 50 : 100 );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_AUTOKERN,
            true, &pItem ))
        {
            if( ((SvxAutoKernItem*)pItem)->GetValue() )
            {
                SetAutoKern( ( !pIDocumentSettingAccess ||
                               !pIDocumentSettingAccess->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION) ) ?
                                KERNING_FONTSPECIFIC :
                                KERNING_ASIAN );
            }
            else
                SetAutoKern( 0 );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_WORDLINEMODE,
            true, &pItem ))
            SetWordLineMode( ((SvxWordLineModeItem*)pItem)->GetValue() );

        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_ESCAPEMENT,
            true, &pItem ))
        {
            const SvxEscapementItem *pEsc = (const SvxEscapementItem *)pItem;
            SetEscapement( pEsc->GetEsc() );
            if( aSub[SW_LATIN].IsEsc() )
                SetProportion( pEsc->GetProp() );
        }
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_CASEMAP,
            true, &pItem ))
            SetCaseMap( ((SvxCaseMapItem*)pItem)->GetCaseMap() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_KERNING,
            true, &pItem ))
            SetFixKerning( ((SvxKerningItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_NOHYPHEN,
            true, &pItem ))
            SetNoHyph( ((SvxNoHyphenItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BLINK,
            true, &pItem ))
            SetBlink( ((SvxBlinkItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_ROTATE,
            true, &pItem ))
            SetVertical( ((SvxCharRotateItem*)pItem)->GetValue() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
            true, &pItem ))
            pBackColor = new Color( ((SvxBrushItem*)pItem)->GetColor() );
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_HIGHLIGHT,
            true, &pItem ))
            SetHighlightColor(((SvxBrushItem*)pItem)->GetColor());
        if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BOX,
            true, &pItem ))
        {
            const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
            SetTopBorder(pBoxItem->GetTop());
            SetBottomBorder(pBoxItem->GetBottom());
            SetRightBorder(pBoxItem->GetRight());
            SetLeftBorder(pBoxItem->GetLeft());
            SetTopBorderDist(pBoxItem->GetDistance(BOX_LINE_TOP));
            SetBottomBorderDist(pBoxItem->GetDistance(BOX_LINE_BOTTOM));
            SetRightBorderDist(pBoxItem->GetDistance(BOX_LINE_RIGHT));
            SetLeftBorderDist(pBoxItem->GetDistance(BOX_LINE_LEFT));
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
            if ( ((SvxTwoLinesItem*)pTwoLinesItem)->GetValue() )
                SetVertical( 0 );
    }
    else
    {
        Invalidate();
        bNoHyph = false;
        bBlink = false;
    }
    bPaintBlank = false;
    bPaintWrong = false;
    OSL_ENSURE( aSub[SW_LATIN].IsTransparent(), "SwFont: Transparent revolution" );
}

SwFont::SwFont( const SwFont &rFont )
{
    aSub[SW_LATIN] = rFont.aSub[SW_LATIN];
    aSub[SW_CJK] = rFont.aSub[SW_CJK];
    aSub[SW_CTL] = rFont.aSub[SW_CTL];
    nActual = rFont.nActual;
    pBackColor = rFont.pBackColor ? new Color( *rFont.pBackColor ) : NULL;
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
    aUnderColor = rFont.GetUnderColor();
    aOverColor  = rFont.GetOverColor();
    nToxCnt = 0;
    nRefCnt = 0;
    m_nMetaCount = 0;
    m_nInputFieldCount = 0;
    bFntChg = rFont.bFntChg;
    bOrgChg = rFont.bOrgChg;
    bPaintBlank = rFont.bPaintBlank;
    bPaintWrong = false;
    bURL = rFont.bURL;
    bGreyWave = rFont.bGreyWave;
    bNoColReplace = rFont.bNoColReplace;
    bNoHyph = rFont.bNoHyph;
    bBlink = rFont.bBlink;
}

SwFont::SwFont( const SwAttrSet* pAttrSet,
                const IDocumentSettingAccess* pIDocumentSettingAccess )
{
    nActual = SW_LATIN;
    nToxCnt = 0;
    nRefCnt = 0;
    m_nMetaCount = 0;
    m_nInputFieldCount = 0;
    bPaintBlank = false;
    bPaintWrong = false;
    bURL = false;
    bGreyWave = false;
    bNoColReplace = false;
    bNoHyph = pAttrSet->GetNoHyphenHere().GetValue();
    bBlink = pAttrSet->GetBlink().GetValue();
    bOrgChg = true;
    {
        const SvxFontItem& rFont = pAttrSet->GetFont();
        aSub[SW_LATIN].SetFamily( rFont.GetFamily() );
        aSub[SW_LATIN].SetName( rFont.GetFamilyName() );
        aSub[SW_LATIN].SetStyleName( rFont.GetStyleName() );
        aSub[SW_LATIN].SetPitch( rFont.GetPitch() );
        aSub[SW_LATIN].SetCharSet( rFont.GetCharSet() );
        aSub[SW_LATIN].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = aSub[SW_LATIN].aSize;
        aTmpSize.Height() = pAttrSet->GetSize().GetHeight();
        aSub[SW_LATIN].SetSize( aTmpSize );
        aSub[SW_LATIN].SetItalic( pAttrSet->GetPosture().GetPosture() );
        aSub[SW_LATIN].SetWeight( pAttrSet->GetWeight().GetWeight() );
        aSub[SW_LATIN].SetLanguage( pAttrSet->GetLanguage().GetLanguage() );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCJKFont();
        aSub[SW_CJK].SetFamily( rFont.GetFamily() );
        aSub[SW_CJK].SetName( rFont.GetFamilyName() );
        aSub[SW_CJK].SetStyleName( rFont.GetStyleName() );
        aSub[SW_CJK].SetPitch( rFont.GetPitch() );
        aSub[SW_CJK].SetCharSet( rFont.GetCharSet() );
        aSub[SW_CJK].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = aSub[SW_CJK].aSize;
        aTmpSize.Height() = pAttrSet->GetCJKSize().GetHeight();
        aSub[SW_CJK].SetSize( aTmpSize );
        aSub[SW_CJK].SetItalic( pAttrSet->GetCJKPosture().GetPosture() );
        aSub[SW_CJK].SetWeight( pAttrSet->GetCJKWeight().GetWeight() );
        LanguageType eNewLang = pAttrSet->GetCJKLanguage().GetLanguage();
        aSub[SW_CJK].SetLanguage( eNewLang );
        aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
        aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
        aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
    }

    {
        const SvxFontItem& rFont = pAttrSet->GetCTLFont();
        aSub[SW_CTL].SetFamily( rFont.GetFamily() );
        aSub[SW_CTL].SetName( rFont.GetFamilyName() );
        aSub[SW_CTL].SetStyleName( rFont.GetStyleName() );
        aSub[SW_CTL].SetPitch( rFont.GetPitch() );
        aSub[SW_CTL].SetCharSet( rFont.GetCharSet() );
        aSub[SW_CTL].SvxFont::SetPropr( 100 );   // 100% der FontSize
        Size aTmpSize = aSub[SW_CTL].aSize;
        aTmpSize.Height() = pAttrSet->GetCTLSize().GetHeight();
        aSub[SW_CTL].SetSize( aTmpSize );
        aSub[SW_CTL].SetItalic( pAttrSet->GetCTLPosture().GetPosture() );
        aSub[SW_CTL].SetWeight( pAttrSet->GetCTLWeight().GetWeight() );
        aSub[SW_CTL].SetLanguage( pAttrSet->GetCTLLanguage().GetLanguage() );
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
                       !pIDocumentSettingAccess->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION) ) ?
                        KERNING_FONTSPECIFIC :
                        KERNING_ASIAN );
    }
    else
        SetAutoKern( 0 );
    SetWordLineMode( pAttrSet->GetWordLineMode().GetValue() );
    const SvxEscapementItem &rEsc = pAttrSet->GetEscapement();
    SetEscapement( rEsc.GetEsc() );
    if( aSub[SW_LATIN].IsEsc() )
        SetProportion( rEsc.GetProp() );
    SetCaseMap( pAttrSet->GetCaseMap().GetCaseMap() );
    SetFixKerning( pAttrSet->GetKerning().GetValue() );
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
        true, &pItem ))
        pBackColor = new Color( ((SvxBrushItem*)pItem)->GetColor() );
    else
        pBackColor = NULL;
    if( SfxItemState::SET == pAttrSet->GetItemState( RES_CHRATR_HIGHLIGHT,
        true, &pItem ))
        SetHighlightColor(((SvxBrushItem*)pItem)->GetColor());
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
        SetTopBorderDist(pBoxItem->GetDistance(BOX_LINE_TOP));
        SetBottomBorderDist(pBoxItem->GetDistance(BOX_LINE_BOTTOM));
        SetRightBorderDist(pBoxItem->GetDistance(BOX_LINE_RIGHT));
        SetLeftBorderDist(pBoxItem->GetDistance(BOX_LINE_LEFT));
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
    if( pIDocumentSettingAccess && pIDocumentSettingAccess->get( IDocumentSettingAccess::SMALL_CAPS_PERCENTAGE_66 ))
    {
        aSub[ SW_LATIN ].smallCapsPercentage66 = true;
        aSub[ SW_CJK ].smallCapsPercentage66 = true;
        aSub[ SW_CTL ].smallCapsPercentage66 = true;
    }
}

SwFont::~SwFont()
{
    delete pBackColor;
}

SwSubFont& SwSubFont::operator=( const SwSubFont &rFont )
{
    SvxFont::operator=( rFont );
    pMagic = rFont.pMagic;
    nFntIndex = rFont.nFntIndex;
    nOrgHeight = rFont.nOrgHeight;
    nOrgAscent = rFont.nOrgAscent;
    nPropWidth = rFont.nPropWidth;
    aSize = rFont.aSize;
    smallCapsPercentage66 = rFont.smallCapsPercentage66;
    return *this;
}

SwFont& SwFont::operator=( const SwFont &rFont )
{
    aSub[SW_LATIN] = rFont.aSub[SW_LATIN];
    aSub[SW_CJK] = rFont.aSub[SW_CJK];
    aSub[SW_CTL] = rFont.aSub[SW_CTL];
    nActual = rFont.nActual;
    delete pBackColor;
    pBackColor = rFont.pBackColor ? new Color( *rFont.pBackColor ) : NULL;
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
    aUnderColor = rFont.GetUnderColor();
    aOverColor  = rFont.GetOverColor();
    nToxCnt = 0;
    nRefCnt = 0;
    m_nMetaCount = 0;
    m_nInputFieldCount = 0;
    bFntChg = rFont.bFntChg;
    bOrgChg = rFont.bOrgChg;
    bPaintBlank = rFont.bPaintBlank;
    bPaintWrong = false;
    bURL = rFont.bURL;
    bGreyWave = rFont.bGreyWave;
    bNoColReplace = rFont.bNoColReplace;
    bNoHyph = rFont.bNoHyph;
    bBlink = rFont.bBlink;
    return *this;
}

void SwFont::GoMagic( SwViewShell *pSh, sal_uInt8 nWhich )
{
    SwFntAccess aFntAccess( aSub[nWhich].pMagic, aSub[nWhich].nFntIndex,
                            &aSub[nWhich], pSh, true );
}

bool SwSubFont::IsSymbol( SwViewShell *pSh )
{
    SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh, false );
    return aFntAccess.Get()->IsSymbol();
}

bool SwSubFont::ChgFnt( SwViewShell const *pSh, OutputDevice& rOut )
{
    if ( pLastFont )
        pLastFont->Unlock();
    SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh, true );
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
    if( bOrgChg && aSub[nActual].IsEsc() )
    {
        const sal_uInt8 nOldProp = aSub[nActual].GetPropr();
        SetProportion( 100 );
        ChgFnt( pSh, rOut );
        SwFntAccess aFntAccess( aSub[nActual].pMagic, aSub[nActual].nFntIndex,
                                &aSub[nActual], pSh );
        aSub[nActual].nOrgHeight = aFntAccess.Get()->GetFontHeight( pSh, rOut );
        aSub[nActual].nOrgAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
        SetProportion( nOldProp );
        bOrgChg = false;
    }

    if( bFntChg )
    {
        ChgFnt( pSh, rOut );
        bFntChg = bOrgChg;
    }
    if( rOut.GetTextLineColor() != aUnderColor )
        rOut.SetTextLineColor( aUnderColor );
    if( rOut.GetOverlineColor() != aOverColor )
        rOut.SetOverlineColor( aOverColor );
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
                             ( (long) nOrgHeight * GetEscapement() ) / 100L;
        const sal_uInt16 nDesc = nDescent>0
                ? std::max<sal_uInt16>( nDescent, nOrgHeight - nOrgAscent)
                : nOrgHeight - nOrgAscent;
        return ( nDesc + CalcEscAscent( nOldAscent ) );
    }
    return nOrgHeight;
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
    SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh );
    const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
    return GetEscapement() ? CalcEscAscent( nAscent ) : nAscent;
}

sal_uInt16 SwSubFont::GetHeight( SwViewShell *pSh, const OutputDevice& rOut )
{
    SV_STAT( nGetTextSize );
    SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh );
    const sal_uInt16 nHeight = aFntAccess.Get()->GetFontHeight( pSh, rOut );
    if ( GetEscapement() )
    {
        const sal_uInt16 nAscent = aFntAccess.Get()->GetFontAscent( pSh, rOut );
        return CalcEscHeight( nHeight, nAscent ); // + nLeading;
    }
    return nHeight; // + nLeading;
}

Size SwSubFont::_GetTxtSize( SwDrawTextInfo& rInf )
{
    // Robust: Eigentlich sollte der Font bereits eingestellt sein, aber
    // sicher ist sicher ...
    if ( !pLastFont || pLastFont->GetOwner()!=pMagic ||
         !IsSameInstance( rInf.GetpOut()->GetFont() ) )
        ChgFnt( rInf.GetShell(), rInf.GetOut() );

    SwDigitModeModifier aDigitModeModifier( rInf.GetOut(), rInf.GetFont()->GetLanguage() );

    Size aTxtSize;
    sal_Int32 nLn = ( rInf.GetLen() == COMPLETE_STRING ? rInf.GetText().getLength()
                                                   : rInf.GetLen() );
    rInf.SetLen( nLn );
    if( IsCapital() && nLn )
        aTxtSize = GetCapitalSize( rInf );
    else
    {
        SV_STAT( nGetTextSize );
        long nOldKern = rInf.GetKern();
        const OUString oldTxt = rInf.GetText();
        rInf.SetKern( CheckKerning() );
        if ( !IsCaseMap() )
            aTxtSize = pLastFont->GetTextSize( rInf );
        else
        {
            OUString aTmp = CalcCaseMap( rInf.GetText() );
            const OUString oldStr = rInf.GetText();
            bool bCaseMapLengthDiffers(aTmp.getLength() != oldStr.getLength());

            if(bCaseMapLengthDiffers && rInf.GetLen())
            {
                // #108203#
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

                aTxtSize = pLastFont->GetTextSize( rInf );

                rInf.SetIdx( nOldIdx );
                rInf.SetLen( nOldLen );
            }
            else
            {
                rInf.SetText( aTmp );
                aTxtSize = pLastFont->GetTextSize( rInf );
            }

            rInf.SetText(oldStr);
        }
        rInf.SetKern( nOldKern );
        rInf.SetText(oldTxt);
        // 15142: Ein Wort laenger als eine Zeile, beim Zeilenumbruch
        //        hochgestellt, muss seine effektive Hoehe melden.
        if( GetEscapement() )
        {
            const sal_uInt16 nAscent = pLastFont->GetFontAscent( rInf.GetShell(),
                                                             rInf.GetOut() );
            aTxtSize.Height() =
                (long)CalcEscHeight( (sal_uInt16)aTxtSize.Height(), nAscent);
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
        aTxtSize = pLastFont->GetTextSize( rInf );
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
        aTxtSize = pLastFont->GetTextSize( rInf );
        rInf.SetIdx( nOldIdx );
        rInf.SetLen( nOldLen );
    }

    return aTxtSize;
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

    if( !pLastFont || pLastFont->GetOwner()!=pMagic )
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
                // #108203#
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
        Size aFontSize = _GetTxtSize( rInf );
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

    if ( !pLastFont || pLastFont->GetOwner() != pMagic )
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
    if ( !pLastFont || pLastFont->GetOwner()!=pMagic )
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
        const OUString oldTxt = rInf.GetText();
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
        rInf.SetText(oldTxt);
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
        nOfst = nOrgHeight - nOrgAscent -
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
                nOrgAscent;

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
        nOfst = ((long)nOrgHeight * GetEscapement()) / 100L;

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
        : aPos( rPoint ), pFnt( &rFnt )
{
};

SwUnderlineFont::~SwUnderlineFont()
{
     delete pFnt;
}

/// Helper for filters to find true lineheight of a font
long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const OutputDevice &rOut, sal_Int16 nScript)
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

    OutputDevice &rMutableOut = const_cast<OutputDevice &>(rOut);
    const vcl::Font aOldFont(rMutableOut.GetFont());

    rMutableOut.SetFont(aFont.GetActualFont());
    long nHeight = rMutableOut.GetTextHeight();

    rMutableOut.SetFont(aOldFont);
    return nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
