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

#include "epptbase.hxx"
#include "epptdef.hxx"
#include "text.hxx"
#include <tools/color.hxx>
#include <editeng/svxenum.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;

PPTExCharSheet::PPTExCharSheet( int nInstance )
{
    sal_uInt16 nFontHeight = 24;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExCharLevel& rLev = maCharLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
                nFontHeight = 44;
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                switch ( nDepth )
                {
                    case 0 : nFontHeight = 32; break;
                    case 1 : nFontHeight = 28; break;
                    case 2 : nFontHeight = 24; break;
                    default :nFontHeight = 20; break;
                }
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nFontHeight = 12;
            break;
            case EPP_TEXTTYPE_notUsed :
            case EPP_TEXTTYPE_Other :
                nFontHeight = 24;
            break;
        }
        rLev.mnFlags = 0;
        rLev.mnFont = 0;
        rLev.mnAsianOrComplexFont = 0xffff;
        rLev.mnFontHeight = nFontHeight;
        rLev.mnFontColor = 0;
        rLev.mnEscapement = 0;
    }
}

void PPTExCharSheet::SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                                    FontCollection& rFontCollection, int nLevel )
{
    PortionObj  aPortionObj( rXPropSet, rFontCollection );

    PPTExCharLevel& rLev = maCharLevel[ nLevel ];

    if ( aPortionObj.meCharColor == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontColor = aPortionObj.mnCharColor;
    if ( aPortionObj.meCharEscapement == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnEscapement = aPortionObj.mnCharEscapement;
    if ( aPortionObj.meCharHeight == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontHeight = aPortionObj.mnCharHeight;
    if ( aPortionObj.meFontName == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFont = aPortionObj.mnFont;
    if ( aPortionObj.meAsianOrComplexFont == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAsianOrComplexFont = aPortionObj.mnAsianOrComplexFont;
    rLev.mnFlags = aPortionObj.mnCharAttr;
}

void PPTExCharSheet::Write( SvStream& rSt, sal_uInt16 nLev, bool bSimpleText,
    const css::uno::Reference< css::beans::XPropertySet > & rPagePropSet )
{
    const PPTExCharLevel& rLev = maCharLevel[ nLev ];

    sal_uInt32 nCharFlags = 0xefffff;
    if ( bSimpleText )
        nCharFlags = 0x7ffff;

    rSt.WriteUInt32( nCharFlags )
       .WriteUInt16( rLev.mnFlags )
       .WriteUInt16( rLev.mnFont );

    Color nFontColor = rLev.mnFontColor;
    if ( nFontColor == COL_AUTO )
    {
        bool bIsDark = false;
        css::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rPagePropSet, "IsBackgroundDark", true ) )
            aAny >>= bIsDark;
        nFontColor = bIsDark ? 0xffffff : 0x000000;
    }
    nFontColor.SetTransparency(0xfe);
    if ( bSimpleText )
    {
        rSt.WriteUInt16( rLev.mnFontHeight )
           .WriteUInt32( sal_uInt32(nFontColor) );
    }
    else
    {
        rSt.WriteUInt16( rLev.mnAsianOrComplexFont )
           .WriteUInt16( 0xffff )       // unknown
           .WriteUInt16( 0xffff )       // unknown
           .WriteUInt16( rLev.mnFontHeight )
           .WriteUInt32( sal_uInt32(nFontColor) )
           .WriteUInt16( rLev.mnEscapement );
    }
}

PPTExParaSheet::PPTExParaSheet( int nInstance, sal_uInt16 nDefaultTab, PPTExBulletProvider* pProv ) :
    pBuProv     ( pProv ),
    mnInstance  ( nInstance )
{
    bool bHasBullet = false;

    sal_uInt16 nUpperDist = 0;
    sal_uInt16 nBulletChar = 0x2022;
    sal_uInt16 nBulletOfs = 0;
    sal_uInt16 nTextOfs = 0;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExParaLevel& rLev = maParaLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                bHasBullet = true;
                nUpperDist = 0x14;
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nUpperDist = 0x1e;
            break;

        }
        switch ( nDepth )
        {
            case 0 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0;
                nTextOfs = bHasBullet ? 0xd8 : 0;
            }
            break;
            case 1 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x120;
                nTextOfs = 0x1d4;
            }
            break;
            case 2 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0x240;
                nTextOfs = 0x2d0;
            }
            break;
            case 3 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x360;
                nTextOfs = 0x3f0;
            }
            break;
            case 4 :
            {
                nBulletChar = 0xbb;
                nBulletOfs = 0x480;
                nTextOfs = 0x510;
            }
            break;
        }
        rLev.mbIsBullet = bHasBullet;
        rLev.mnBulletChar = nBulletChar;
        rLev.mnBulletFont = 0;
        rLev.mnBulletHeight = 100;
        rLev.mnBulletColor = 0;
        rLev.mnAdjust = 0;
        rLev.mnLineFeed = 100;
        rLev.mnLowerDist = 0;
        rLev.mnUpperDist = nUpperDist;
        rLev.mnTextOfs = nTextOfs;
        rLev.mnBulletOfs = nBulletOfs;
        rLev.mnDefaultTab = nDefaultTab;
        rLev.mnAsianSettings = 2;
        rLev.mnBiDi = 0;

        rLev.mbExtendedBulletsUsed = false;
        rLev.mnBulletId = 0xffff;
        rLev.mnBulletStart = 0;
        rLev.mnMappedNumType = 0;
        rLev.mnNumberingType = 0;
    }
}

void PPTExParaSheet::SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nLevel, const PPTExCharLevel& rCharLevel )
{
    ParagraphObj aParagraphObj( rXPropSet, pBuProv );
    aParagraphObj.CalculateGraphicBulletSize( rCharLevel.mnFontHeight );
    PPTExParaLevel& rLev = maParaLevel[ nLevel ];

    if ( aParagraphObj.meTextAdjust == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAdjust = aParagraphObj.mnTextAdjust;
    if ( aParagraphObj.meLineSpacing == css::beans::PropertyState_DIRECT_VALUE )
    {
        sal_Int16 nLineSpacing = aParagraphObj.mnLineSpacing;
        if ( nLineSpacing > 0 ) // if nLinespacing is < 0 the linespacing is an absolute spacing
        {
            bool bFixedLineSpacing = false;
            uno::Any aAny = rXPropSet->getPropertyValue("FontIndependentLineSpacing");
            if( !(aAny >>= bFixedLineSpacing) || !bFixedLineSpacing )
            {
                const FontCollectionEntry* pDesc = rFontCollection.GetById( rCharLevel.mnFont );
                if ( pDesc )
                    nLineSpacing = static_cast<sal_Int16>( static_cast<double>(nLineSpacing) * pDesc->Scaling + 0.5 );
            }
        }
        else
        {
            if ( rCharLevel.mnFontHeight > static_cast<sal_uInt16>( static_cast<double>(-nLineSpacing) * 0.001 * 72.0 / 2.54 ) ) // 1/100mm to point
            {
                const FontCollectionEntry* pDesc = rFontCollection.GetById( rCharLevel.mnFont );
                if ( pDesc )
                     nLineSpacing = static_cast<sal_Int16>( 100.0 * pDesc->Scaling + 0.5 );
                else
                    nLineSpacing = 100;
            }
            else
                nLineSpacing = static_cast<sal_Int16>( static_cast<double>(nLineSpacing) / 4.40972 );
        }
        rLev.mnLineFeed = nLineSpacing;
    }
    if ( aParagraphObj.meLineSpacingBottom == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnLowerDist = aParagraphObj.mnLineSpacingBottom;
    if ( aParagraphObj.meLineSpacingTop == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnUpperDist = aParagraphObj.mnLineSpacingTop;
    if ( aParagraphObj.meForbiddenRules == css::beans::PropertyState_DIRECT_VALUE )
    {
        rLev.mnAsianSettings &=~1;
        if ( aParagraphObj.mbForbiddenRules )
            rLev.mnAsianSettings |= 1;
    }
    if ( aParagraphObj.meParagraphPunctation == css::beans::PropertyState_DIRECT_VALUE )
    {
        rLev.mnAsianSettings &=~4;
        if ( aParagraphObj.mbParagraphPunctation )
            rLev.mnAsianSettings |= 4;
    }

    if ( aParagraphObj.meBiDi == css::beans::PropertyState_DIRECT_VALUE )
        rLev.mnBiDi = aParagraphObj.mnBiDi;

    rLev.mbIsBullet = aParagraphObj.mbIsBullet; //( ( aParagraphObj.nBulletFlags & 1 ) != 0 );

    if ( nLevel )
        return;

    if (!(aParagraphObj.bExtendedParameters &&
         aParagraphObj.meBullet == css::beans::PropertyState_DIRECT_VALUE))
        return;

    for ( sal_Int16 i = 0; i < 5; i++ )
    {
        PPTExParaLevel& rLevel = maParaLevel[ i ];
        if ( i )
            aParagraphObj.ImplGetNumberingLevel( pBuProv, i, false, false );
        rLevel.mnTextOfs = aParagraphObj.nTextOfs;
        rLevel.mnBulletOfs = static_cast<sal_uInt16>(aParagraphObj.nBulletOfs);
        rLevel.mnBulletChar = aParagraphObj.cBulletId;
        FontCollectionEntry aFontDescEntry( aParagraphObj.aFontDesc.Name, aParagraphObj.aFontDesc.Family,
                                                aParagraphObj.aFontDesc.Pitch, aParagraphObj.aFontDesc.CharSet );
        rLevel.mnBulletFont = static_cast<sal_uInt16>(rFontCollection.GetId( aFontDescEntry ));
        rLevel.mnBulletHeight = aParagraphObj.nBulletRealSize;
        rLevel.mnBulletColor = aParagraphObj.nBulletColor;

        rLevel.mbExtendedBulletsUsed = aParagraphObj.bExtendedBulletsUsed;
        rLevel.mnBulletId = aParagraphObj.nBulletId;
        rLevel.mnNumberingType = aParagraphObj.nNumberingType;
        rLevel.mnBulletStart = aParagraphObj.nStartWith;
        rLevel.mnMappedNumType = aParagraphObj.nMappedNumType;
    }
}

void PPTExParaSheet::Write( SvStream& rSt, sal_uInt16 nLev, bool bSimpleText,
    const css::uno::Reference< css::beans::XPropertySet > & rPagePropSet )
{
    const PPTExParaLevel& rLev = maParaLevel[ nLev ];

    if ( maParaLevel[ 0 ].mbExtendedBulletsUsed || maParaLevel[ 1 ].mbExtendedBulletsUsed ||
            maParaLevel[ 2 ].mbExtendedBulletsUsed || maParaLevel[ 3 ].mbExtendedBulletsUsed ||
                maParaLevel[ 4 ].mbExtendedBulletsUsed )
    {
        SvStream& rOut = pBuProv->aBuExMasterStream;
        if ( !nLev )
        {
            rOut.WriteUInt32( ( EPP_PST_ExtendedParagraphMasterAtom << 16 ) | ( mnInstance << 4 ) )
                .WriteUInt32( 5 * 16 + 2 )
                .WriteUInt16( 5 );              // depth
        }
        sal_uInt16 nBulletId = rLev.mnBulletId;
        if ( rLev.mnNumberingType != SVX_NUM_BITMAP )
            nBulletId = 0xffff;
        rOut.WriteUInt32( 0x03800000 )
            .WriteUInt16( nBulletId )
            .WriteUInt32( rLev.mnMappedNumType )
            .WriteUInt16( rLev.mnBulletStart )
            .WriteUInt32( 0 );
    }

    sal_uInt32 nParaFlags = 0x3ffdff;
    sal_uInt16 nBulletFlags = ( rLev.mbIsBullet ) ? 0xf : 0xe;

    if ( nLev )
        nParaFlags &= 0x207fff;
    if ( bSimpleText )
        nParaFlags &= 0x7fff;
    sal_uInt32 nBulletColor = rLev.mnBulletColor;
    if ( nBulletColor == sal_uInt32(COL_AUTO) )
    {
        bool bIsDark = false;
        css::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rPagePropSet, "IsBackgroundDark", true ) )
            aAny >>= bIsDark;
        nBulletColor = bIsDark ? 0xffffff : 0x000000;
    }
    nBulletColor &= 0xffffff;
    nBulletColor |= 0xfe000000;
    rSt.WriteUInt32( nParaFlags )
       .WriteUInt16( nBulletFlags )
       .WriteUInt16( rLev.mnBulletChar )
       .WriteUInt16( rLev.mnBulletFont )
       .WriteUInt16( rLev.mnBulletHeight )
       .WriteUInt32( nBulletColor )
       .WriteUInt16( rLev.mnAdjust )
       .WriteUInt16( rLev.mnLineFeed )
       .WriteUInt16( rLev.mnUpperDist )
       .WriteUInt16( rLev.mnLowerDist )
       .WriteUInt16( rLev.mnTextOfs )
       .WriteUInt16( rLev.mnBulletOfs );

    if ( bSimpleText || nLev )
    {
        if ( nParaFlags & 0x200000 )
            rSt.WriteUInt16( rLev.mnBiDi );
    }
    else
    {
        rSt.WriteUInt16( rLev.mnDefaultTab )
           .WriteUInt16( 0 )
           .WriteUInt16( 0 )
           .WriteUInt16( rLev.mnAsianSettings )
           .WriteUInt16( rLev.mnBiDi );
    }
}

PPTExStyleSheet::PPTExStyleSheet( sal_uInt16 nDefaultTab, PPTExBulletProvider* pBuProv )
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if (nInstance != EPP_TEXTTYPE_notUsed)
        {
            mpParaSheet[ nInstance ].reset(new PPTExParaSheet( nInstance, nDefaultTab, pBuProv ));
            mpCharSheet[ nInstance ].reset(new PPTExCharSheet( nInstance ));
        }
    }
}

PPTExStyleSheet::~PPTExStyleSheet()
{
}

void PPTExStyleSheet::SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nInstance, int nLevel )
{
    if ( nInstance == EPP_TEXTTYPE_notUsed )
        return;
    mpCharSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel );
    mpParaSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel, mpCharSheet[ nInstance ]->maCharLevel[ nLevel ] );
}

bool PPTExStyleSheet::IsHardAttribute( sal_uInt32 nInstance, sal_uInt32 nLevel, PPTExTextAttr eAttr, sal_uInt32 nValue )
{
    assert(nInstance < PPTEX_STYLESHEETENTRYS && nLevel < 5);

    const PPTExParaLevel& rPara = mpParaSheet[ nInstance ]->maParaLevel[ nLevel ];
    const PPTExCharLevel& rChar = mpCharSheet[ nInstance ]->maCharLevel[ nLevel ];

    sal_uInt32 nFlag = 0;

    switch ( eAttr )
    {
        case ParaAttr_BulletOn : return ( rPara.mbIsBullet ) ? nValue == 0 : nValue != 0;
        case ParaAttr_BuHardFont :
        case ParaAttr_BulletFont : return ( rPara.mnBulletFont != nValue );
        case ParaAttr_BuHardColor :
        case ParaAttr_BulletColor : return ( rPara.mnBulletColor != nValue );
        case ParaAttr_BuHardHeight :
        case ParaAttr_BulletHeight : return ( rPara.mnBulletHeight != nValue );
        case ParaAttr_BulletChar : return ( rPara.mnBulletChar != nValue );
        case ParaAttr_Adjust : return ( rPara.mnAdjust != nValue );
        case ParaAttr_LineFeed : return ( rPara.mnLineFeed != nValue );
        case ParaAttr_UpperDist : return ( rPara.mnUpperDist != nValue );
        case ParaAttr_LowerDist : return ( rPara.mnLowerDist != nValue );
        case ParaAttr_TextOfs : return ( rPara.mnTextOfs != nValue );
        case ParaAttr_BulletOfs : return ( rPara.mnBulletOfs != nValue );
        case ParaAttr_DefaultTab : return ( rPara.mnDefaultTab != nValue );
        case ParaAttr_BiDi : return ( rPara.mnBiDi != nValue );
        case CharAttr_Bold : nFlag = 1; break;
        case CharAttr_Italic : nFlag = 2; break;
        case CharAttr_Underline : nFlag = 4; break;
        case CharAttr_Shadow : nFlag = 16; break;
        case CharAttr_Strikeout : nFlag = 256; break;
        case CharAttr_Embossed : nFlag = 512; break;
        case CharAttr_Font : return ( rChar.mnFont != nValue );
        case CharAttr_AsianOrComplexFont : return ( rChar.mnAsianOrComplexFont != nValue );
        case CharAttr_Symbol : return true;
        case CharAttr_FontHeight : return ( rChar.mnFontHeight != nValue );
        case CharAttr_FontColor : return ( rChar.mnFontColor != nValue );
        case CharAttr_Escapement : return ( rChar.mnEscapement != nValue );
        default:
            break;
    }
    if ( nFlag )
    {
        if ( rChar.mnFlags & nFlag )
            return ( ( nValue & nFlag ) == 0 );
        else
            return ( ( nValue & nFlag ) != 0 );
    }
    return true;
}

// the TxCFStyleAtom stores the text properties that are used
// when creating new objects in PowerPoint.

void PPTExStyleSheet::WriteTxCFStyleAtom( SvStream& rSt )
{
    const PPTExCharLevel& rCharStyle = mpCharSheet[ EPP_TEXTTYPE_Other ]->maCharLevel[ 0 ];

    sal_uInt16 const nFlags = 0x60        // ??
                      | 0x02        // fontsize;
                      | 0x04;       // fontcolor

    sal_uInt32 nCharFlags = rCharStyle.mnFlags;
    nCharFlags &= CharAttr_Italic | CharAttr_Bold | CharAttr_Underline | CharAttr_Shadow;

    rSt.WriteUInt32( EPP_TxCFStyleAtom << 16 )  // recordheader
       .WriteUInt32( SizeOfTxCFStyleAtom() - 8 )
       .WriteUInt16( 0x80 | nCharFlags )
       .WriteUInt16( nFlags )
       .WriteUInt16( nCharFlags )
       .WriteInt32( -1 )                            // ?
       .WriteUInt16( rCharStyle.mnFontHeight )
       .WriteUInt32( sal_uInt32(rCharStyle.mnFontColor) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
