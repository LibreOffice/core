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
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <limits.h>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/valueset.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>

namespace svx { namespace sidebar {

ValueSetWithTextControl::ValueSetWithTextControl(
    const tControlType eControlType,
    vcl::Window* pParent,
    const ResId& rResId)
    : ValueSet( pParent, rResId )
    , meControlType( eControlType )
    , maItems()
{
    SetColCount();
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
    aItem.maSelectedItemImage = (pSelectedItemImage != nullptr)
                                ? *pSelectedItemImage
                                : rItemImage;

    if ( GetDPIScaleFactor() > 1 )
    {
        BitmapEx b = aItem.maItemImage.GetBitmapEx();
        b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());
        aItem.maItemImage = Image(b);

        if ( pSelectedItemImage != nullptr )
        {
            b = aItem.maSelectedItemImage.GetBitmapEx();
            b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());
            aItem.maSelectedItemImage = Image(b);
        }
    }

    aItem.maItemText = rItemText;

    maItems.push_back( aItem );

    InsertItem( maItems.size() );
    SetItemText( maItems.size(),
                    (pItemHelpText != nullptr) ? *pItemHelpText : rItemText );
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
                    (pItemHelpText != nullptr) ? *pItemHelpText : rItemText );
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
    maItems[nItemId-1].maSelectedItemImage = (pSelectedItemImage != nullptr)
                                             ? *pSelectedItemImage
                                             : rItemImage;

//#ifndef MACOSX
    if ( GetDPIScaleFactor() > 1 )
    {
        BitmapEx b = maItems[nItemId-1].maItemImage.GetBitmapEx();
        b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());
        maItems[nItemId-1].maItemImage = Image(b);

        if ( pSelectedItemImage != nullptr )
        {
            b = maItems[nItemId-1].maSelectedItemImage.GetBitmapEx();
            b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());
            maItems[nItemId-1].maSelectedItemImage = Image(b);
        }
    }
//#endif
}


void ValueSetWithTextControl::UserDraw( const UserDrawEvent& rUDEvt )
{
    const Rectangle aRect = rUDEvt.GetRect();
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    pDev->Push();
    const sal_uInt16 nItemId = rUDEvt.GetItemId();

    const long nRectHeight = aRect.GetHeight();
    const Point aBLPos = aRect.TopLeft();

    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, MsLangId::getSystemLanguage(), GetDefaultFontFlags::OnlyOne));
    {
        Size aSize = aFont.GetSize();
        aSize.Height() = (nRectHeight*4)/9;
        aFont.SetSize( aSize );
    }

    {
        //draw background
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
        Image* pImage = nullptr;
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
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText, DrawTextFlags::EndEllipsis);
            }
            break;
        case TEXT_TEXT:
            {
                const long nRectWidth = aRect.GetWidth();
                aStrRect.Left() += 8;
                aStrRect.Right() -= (nRectWidth*2)/3;
                pDev->SetFont(aFont);
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText, DrawTextFlags::EndEllipsis);
                aStrRect.Left() += nRectWidth/3;
                aStrRect.Right() += (nRectWidth*2)/3;
                pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText2, DrawTextFlags::EndEllipsis);
            }
            break;
        }
    }

    Invalidate( aRect );
    pDev->Pop();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
