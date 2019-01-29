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
#include <svx/sidebar/ValueSetWithTextControl.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <limits.h>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/valueset.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>

namespace svx { namespace sidebar {

ValueSetWithTextControl::ValueSetWithTextControl(Window* pParent, WinBits nBits)
    : ValueSet( pParent, nBits )
{
    SetColCount();
}


void ValueSetWithTextControl::AddItem(
    const OUString& rItemText,
    const OUString& rItemText2 )
{
    ValueSetWithTextItem aItem;
    aItem.maItemText = rItemText;
    aItem.maItemText2 = rItemText2;

    maItems.push_back( aItem );

    InsertItem( maItems.size() );
    SetItemText( maItems.size(), rItemText );
}

void ValueSetWithTextControl::UserDraw( const UserDrawEvent& rUDEvt )
{
    const tools::Rectangle aRect = rUDEvt.GetRect();
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    pDev->Push();
    const sal_uInt16 nItemId = rUDEvt.GetItemId();

    const long nRectHeight = aRect.GetHeight();

    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, MsLangId::getSystemLanguage(), GetDefaultFontFlags::OnlyOne));
    {
        Size aSize = aFont.GetFontSize();
        aSize.setHeight( (nRectHeight*4)/9 );
        aFont.SetFontSize( aSize );
    }

    {
        //draw background
        if ( GetSelectedItemId() == nItemId )
        {
            tools::Rectangle aBackRect = aRect;
            aBackRect.AdjustTop(3 );
            aBackRect.AdjustBottom( -2 );
            pDev->SetFillColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_Highlight ) );
            pDev->DrawRect(aBackRect);
        }
        else
        {
            pDev->SetFillColor( COL_TRANSPARENT );
            pDev->DrawRect(aRect);
        }

        if ( GetSelectedItemId() == nItemId )
        {
            aFont.SetColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_HighlightText ) );
        }
        else
        {
            aFont.SetColor( GetSettings().GetStyleSettings().GetFieldTextColor() );
        }

        tools::Rectangle aStrRect = aRect;
        aStrRect.AdjustTop(nRectHeight/4 );
        aStrRect.AdjustBottom( -(nRectHeight/4) );

        const long nRectWidth = aRect.GetWidth();
        aStrRect.AdjustLeft(8 );
        aStrRect.AdjustRight( -((nRectWidth*2)/3) );
        pDev->SetFont(aFont);
        pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText, DrawTextFlags::EndEllipsis);
        aStrRect.AdjustLeft(nRectWidth/3 );
        aStrRect.AdjustRight((nRectWidth*2)/3 );
        pDev->DrawText(aStrRect, maItems[nItemId-1].maItemText2, DrawTextFlags::EndEllipsis);
    }

    Invalidate( aRect );
    pDev->Pop();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
