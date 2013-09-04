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
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <limits.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/valueset.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;
using rtl::OUString;

namespace svx { namespace sidebar {
static const sal_Char cValue[] = "Value";

static Font& lcl_GetDefaultBulletFont()
{
    static sal_Bool bInit = 0;
    static Font aDefBulletFont( OUString( "StarSymbol" ),
                                OUString(), Size( 0, 14 ) );
    if(!bInit)
    {
        aDefBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
        aDefBulletFont.SetFamily( FAMILY_DONTKNOW );
        aDefBulletFont.SetPitch( PITCH_DONTKNOW );
        aDefBulletFont.SetWeight( WEIGHT_DONTKNOW );
        aDefBulletFont.SetTransparent( sal_True );
        bInit = sal_True;
    }
    return aDefBulletFont;
}

ValueSetWithTextControl::ValueSetWithTextControl(
    const tControlType eControlType,
    Window* pParent,
    const ResId& rResId)
    : ValueSet( pParent, rResId )
    , meControlType( eControlType )
    , maItems()
{
    SetColCount( 1 );
}


ValueSetWithTextControl::~ValueSetWithTextControl(void)
{
}


void ValueSetWithTextControl::AddItem(
    const Image& rItemImage,
    const Image* pSelectedItemImage,
    const OUString& rItemText,
    const OUString* pItemHelpText )
{
    if ( meControlType != IMAGE_TEXT )
    {
        return;
    }

    ValueSetWithTextItem aItem;
    aItem.maItemImage = rItemImage;
    aItem.maSelectedItemImage = (pSelectedItemImage != 0)
                                ? *pSelectedItemImage
                                : rItemImage;
    aItem.maItemText = rItemText;

    maItems.push_back( aItem );

    InsertItem( maItems.size() );
    SetItemText( maItems.size(),
                    (pItemHelpText != 0) ? *pItemHelpText : rItemText );
}


void ValueSetWithTextControl::AddItem(
    const OUString& rItemText,
    const OUString& rItemText2,
    const OUString* pItemHelpText )
{
    if ( meControlType != TEXT_TEXT )
    {
        return;
    }

    ValueSetWithTextItem aItem;
    aItem.maItemText = rItemText;
    aItem.maItemText2 = rItemText2;

    maItems.push_back( aItem );

    InsertItem( maItems.size() );
    SetItemText( maItems.size(),
                    (pItemHelpText != 0) ? *pItemHelpText : rItemText );
}


void ValueSetWithTextControl::ReplaceItemImages(
    const sal_uInt16 nItemId,
    const Image& rItemImage,
    const Image* pSelectedItemImage )
{
    if ( meControlType != IMAGE_TEXT )
    {
        return;
    }

    if ( nItemId == 0 ||
         nItemId > maItems.size() )
    {
        return;
    }

    maItems[nItemId-1].maItemImage = rItemImage;
    maItems[nItemId-1].maSelectedItemImage = (pSelectedItemImage != 0)
                                             ? *pSelectedItemImage
                                             : rItemImage;
}


void ValueSetWithTextControl::UserDraw( const UserDrawEvent& rUDEvt )
{
    const Rectangle aRect = rUDEvt.GetRect();
    OutputDevice* pDev = rUDEvt.GetDevice();
    pDev->Push( PUSH_ALL );
    const sal_uInt16 nItemId = rUDEvt.GetItemId();

    const long nRectHeight = aRect.GetHeight();
    const Point aBLPos = aRect.TopLeft();

    Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE));
    {
        Size aSize = aFont.GetSize();
        aSize.Height() = (nRectHeight*4)/9;
        aFont.SetSize( aSize );
    }

    {
        //draw backgroud
        if ( GetSelectItemId() == nItemId )
        {
            Rectangle aBackRect = aRect;
            aBackRect.Top() += 3;
            aBackRect.Bottom() -= 2;
            pDev->SetFillColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_Highlight ) );
            pDev->DrawRect(aBackRect);
        }
        else
        {
            pDev->SetFillColor( COL_TRANSPARENT );
            pDev->DrawRect(aRect);
        }

        //draw image + text resp. text + text
        Image* pImage = 0;
        if ( GetSelectItemId() == nItemId )
        {
            aFont.SetColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_HighlightText ) );
            pImage = &maItems[nItemId-1].maSelectedItemImage;
        }
        else
        {
            aFont.SetColor( GetSettings().GetStyleSettings().GetFieldTextColor() );
            pImage = &maItems[nItemId-1].maItemImage;
        }

        Rectangle aStrRect = aRect;
        aStrRect.Top() += nRectHeight/4;
        aStrRect.Bottom() -= nRectHeight/4;

        switch ( meControlType )
        {
        case IMAGE_TEXT:
            {
                Point aImgStart(
                    aBLPos.X() + 4,
                    aBLPos.Y() + ( ( nRectHeight - pImage->GetSizePixel().Height() ) / 2 ) );
                pDev->DrawImage( aImgStart, *pImage );

                aStrRect.Left() += pImage->GetSizePixel().Width() + 12;
                pDev->SetFont(aFont);
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText, TEXT_DRAW_ENDELLIPSIS);
            }
            break;
        case TEXT_TEXT:
            {
                const long nRectWidth = aRect.GetWidth();
                aStrRect.Left() += 8;
                aStrRect.Right() -= (nRectWidth*2)/3;
                pDev->SetFont(aFont);
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText, TEXT_DRAW_ENDELLIPSIS);
                aStrRect.Left() += nRectWidth/3;
                aStrRect.Right() += (nRectWidth*2)/3;
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText2, TEXT_DRAW_ENDELLIPSIS);
            }
            break;
        }
    }

    Invalidate( aRect );
    pDev->Pop();
}

SvxNumValueSet2::SvxNumValueSet2( Window* pParent, const ResId& rResId) :
    ValueSet( pParent, rResId ),
    aLineColor  ( COL_LIGHTGRAY ),
    pVDev       ( NULL )
{
    SetColCount( 3 );
    SetLineCount( 3 );
    SetStyle( GetStyle() | WB_ITEMBORDER );
}

 SvxNumValueSet2::~SvxNumValueSet2()
{
    delete pVDev;
}

void SvxNumValueSet2::SetNumberingSettings(
    const Sequence<Sequence<PropertyValue> >& aNum,
    Reference<XNumberingFormatter>& xFormat,
    const Locale& rLocale   )
{
    aNumSettings = aNum;
    xFormatter = xFormat;
    aLocale = rLocale;
        if(aNum.getLength() > 9)
        SetStyle( GetStyle()|WB_VSCROLL);
    InsertItem( DEFAULT_NONE, DEFAULT_NONE - 1 );
    SetItemText( DEFAULT_NONE, SVX_RESSTR( RID_SVXSTR_NUMBULLET_NONE ));

    for ( sal_Int32 i = 0; i < aNum.getLength(); i++ )
    {
        InsertItem( i + 1);
        if( i < 8 )
        {
            NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(eNBOType::NUMBERING);
            if ( pNumbering )
            {
                SetItemText( i + 1, pNumbering->GetDescription(i));
            }
        }
    }
}

void  SvxNumValueSet2::UserDraw( const UserDrawEvent& rUDEvt )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color aBackColor = rStyleSettings.GetFieldColor();
    const Color aTextColor = rStyleSettings.GetFieldTextColor();

    OutputDevice*  pDev = rUDEvt.GetDevice();
    Rectangle aRect = rUDEvt.GetRect();
    sal_uInt32  nItemId = rUDEvt.GetItemId();
    long nRectWidth = aRect.GetWidth();
    long nRectHeight = aRect.GetHeight();
    Size aRectSize(nRectWidth, aRect.GetHeight());
    Point aBLPos = aRect.TopLeft();
    Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    pDev->SetLineColor(aBackColor);
    Font aFont(OutputDevice::GetDefaultFont(
                DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE));

    Size aSize = aFont.GetSize();
    aSize.Height() = nRectHeight/5;
    aFont.SetColor(aTextColor);
    aFont.SetFillColor(aBackColor);
    aFont.SetSize( aSize );
    pDev->SetFont(aFont);
    pDev->SetLineColor(aTextColor);
    if(!pVDev)
    {
        // Die Linien werden nur einmalig in das VirtualDevice gepainted
        // nur die Gliederungspage bekommt es aktuell
        pVDev = new VirtualDevice(*pDev);
        pVDev->SetMapMode(pDev->GetMapMode());
        pVDev->EnableRTL( IsRTLEnabled() );
        pVDev->SetOutputSize( aRectSize );
        aOrgRect = aRect;

        pVDev->SetLineColor( aBackColor );
        pVDev->SetFillColor( aBackColor );
        pVDev->DrawRect(aOrgRect);

        if(aBackColor == aLineColor)
            aLineColor.Invert();
        if(GetSettings().GetStyleSettings().GetHighContrastMode())
            pVDev->SetLineColor(aTextColor);
        else
            pVDev->SetLineColor(aLineColor);
        // Linien nur einmalig Zeichnen
        Point aStart(aBLPos.X() + nRectWidth *30 / 100,0);
        Point aEnd(aBLPos.X() + nRectWidth * 9 / 10,0);
        for( sal_uInt32 i = 11; i < 100; i += 33)
        {
            aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * i / 100;
            pVDev->DrawLine(aStart, aEnd);
            aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * (i + 11) / 100;
            pVDev->DrawLine(aStart, aEnd);
        }
    }
    if ( nItemId != DEFAULT_NONE)
        pDev->DrawOutDev(   aRect.TopLeft(), aRectSize,
                            aOrgRect.TopLeft(), aRectSize,
                            *pVDev );

    Point aStart(aBLPos.X() + nRectWidth / 9,0);
    if ( nItemId == DEFAULT_NONE)
    {
        OUString sText(SVX_RESSTR( RID_SVXSTR_NUMBULLET_NONE));
        Font aLclFont = pDev->GetFont();
        Size aLclSize = aLclFont.GetSize();
        aLclSize.Height() = nRectHeight/4;
        aLclFont.SetSize( aLclSize );
        pDev->SetFont(aLclFont);
        long nTextWidth = pDev->GetTextWidth(sText);
        long nTextHeight = pDev->GetTextHeight();
        //GVT refine
        while (nTextWidth>nRectWidth && aLclSize.Height()>4) {
            aLclSize.Height() = aLclSize.Height()*0.9;
            aLclFont.SetSize( aLclSize );
            pDev->SetFont(aLclFont);
            nTextWidth = pDev->GetTextWidth(sText);
        }
        Point aSStart(aBLPos.X()+(nRectWidth-nTextWidth)/2, aBLPos.Y() +(nRectHeight-nTextHeight)/2);
        pDev->DrawText(aSStart, sText);
        pDev->SetFont(aOldFont);
    }
    else
    {
        NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(eNBOType::NUMBERING);
        if ( pNumbering && nItemId <= DEFAULT_BULLET_TYPES )
        {
            for( sal_uInt32 i = 0; i < 3; i++ )
            {
                sal_uInt32 nY = 11 + i * 33;
                aStart.Y() = aBLPos.Y() + nRectHeight  * nY / 100;
                OUString sText;
                sal_uInt16 nLvl = 0;
                SvxNumRule aTempRule( 0, 10, false );
                pNumbering->ApplyNumRule(aTempRule,nItemId -1,1<<nLvl);
                SvxNumberFormat aNumFmt(aTempRule.GetLevel(nLvl));
                sText = aNumFmt.GetPrefix() + aNumFmt.GetNumStr(i+1) + aNumFmt.GetSuffix();
                aStart.X() = aBLPos.X() + 2;
                aStart.Y() -= pDev->GetTextHeight()/2;
                pDev->DrawText(aStart, sText);
            }
        }
        pDev->SetFont(aOldFont);
        pDev->SetLineColor(aOldColor);
    }
    //End
}

//===============================================================================================

SvxNumValueSet3::SvxNumValueSet3( Window* pParent, const ResId& rResId) :
    ValueSet( pParent, rResId )
{
    SetColCount( 3 );
    SetLineCount( 4 );
    SetStyle( GetStyle() | WB_ITEMBORDER );
}

 SvxNumValueSet3::~SvxNumValueSet3()
{
}

void  SvxNumValueSet3::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    sal_uInt32  nItemId = rUDEvt.GetItemId();

    long nRectHeight = aRect.GetHeight();
    long nRectWidth = aRect.GetWidth();
    Point aBLPos = aRect.TopLeft();
    NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(eNBOType::MIXBULLETS);
    if ( pBullets )
    {
        if ( nItemId <= DEFAULT_BULLET_TYPES ) {
            sal_uInt16 nLvl = 0;
            SvxNumRule aTempRule( 0, 10, false );
            pBullets->ApplyNumRule(aTempRule,nItemId -1,1<<nLvl);
            SvxNumberFormat aFmt(aTempRule.GetLevel(nLvl));
            sal_Int16 eNumType = aFmt.GetNumberingType();
            if( eNumType == SVX_NUM_CHAR_SPECIAL)
            {
                sal_Unicode cChar = aFmt.GetBulletChar();
            //End
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                const Color aBackColor = rStyleSettings.GetFieldColor();
                const Color aTextColor = rStyleSettings.GetFieldTextColor();

                Font aOldFont = pDev->GetFont();
                Font aFont( lcl_GetDefaultBulletFont() );

                Size aSize = aFont.GetSize();
                aSize.Height() = nRectHeight*3/6;
                aFont.SetColor(aTextColor);
                aFont.SetFillColor(aBackColor);
                aFont.SetSize( aSize );
                pDev->SetFont(aFont);
                pDev->SetFillColor( aBackColor ); //wj

                OUString sText(cChar);
                Font aOldBulletFont = pDev->GetFont();
                Font aBulletFnt(aFmt.GetBulletFont() ? *aFmt.GetBulletFont() : aOldBulletFont);
                Size aBulSize = aOldBulletFont.GetSize();
                aBulletFnt.SetSize(aBulSize);
                pDev->SetFont(aBulletFnt);
                long nTextWidth = pDev->GetTextWidth(sText);
                long nTextHeight = pDev->GetTextHeight();
                Point aStart(aBLPos.X()+(nRectWidth-nTextWidth)/2, aBLPos.Y() +(nRectHeight-nTextHeight)/2);
                pDev->DrawText(aStart, sText);
                pDev->SetFont(aOldFont);
            }else if ( eNumType == SVX_NUM_BITMAP )
            {
                const SvxBrushItem* pBrushItem = aFmt.GetBrush();
                if(pBrushItem)
                {
                    const Graphic* pGrf = pBrushItem->GetGraphic();
                    if(pGrf)
                    {
                        Size aSize(nRectHeight*6/20, nRectHeight*6/20);
                        Point aStart(aBLPos.X() + nRectWidth*7/20, aBLPos.Y() + nRectHeight*7/20);

                        pGrf->Draw( pDev, aStart, aSize );
                    }
                }
            }
        }else if ( nItemId == DEFAULT_NONE)
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            const Color aBackColor = rStyleSettings.GetFieldColor();
            const Color aTextColor = rStyleSettings.GetFieldTextColor();

            Font aOldFont = pDev->GetFont();
            Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE));
            Size aSize = aFont.GetSize();
            //aSize.Height() = nRectHeight/5;
            aSize.Height() = nRectHeight/4;
            aFont.SetColor(aTextColor);
            aFont.SetFillColor(aBackColor);
            aFont.SetSize( aSize );
            pDev->SetFont(aFont);
            pDev->SetFillColor( aBackColor );

            OUString sText(SVX_RESSTR( RID_SVXSTR_NUMBULLET_NONE));

            long nTextWidth = pDev->GetTextWidth(sText);
            long nTextHeight = pDev->GetTextHeight();
            //GVT refine
            while (nTextWidth>nRectWidth && aSize.Height()>4) {
                aSize.Height() = aSize.Height()*0.9;
                aFont.SetSize( aSize );
                pDev->SetFont(aFont);
                nTextWidth = pDev->GetTextWidth(sText);
            }
            Point aStart(aBLPos.X()+(nRectWidth-nTextWidth)/2, aBLPos.Y() +(nRectHeight-nTextHeight)/2);
            pDev->DrawText(aStart, sText);

            pDev->SetFont(aOldFont);
        }
    }

}

} } // end of namespace svx::sidebar
