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

#include "CategoryListBox.hxx"
#include <vcl/builderfactory.hxx>

#include <vcl/event.hxx>

namespace sd {

CategoryListBox::CategoryListBox( vcl::Window* pParent )
: ListBox( pParent, WB_TABSTOP | WB_BORDER )
{
    EnableUserDraw( true );
    SetDoubleClickHdl( LINK( this, CategoryListBox, implDoubleClickHdl ) );
}

VCL_BUILDER_FACTORY(CategoryListBox)

CategoryListBox::~CategoryListBox()
{
}

void  CategoryListBox::InsertCategory( const OUString& rStr )
{
    sal_Int32  n = ListBox::InsertEntry( rStr );
    if( n != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SetEntryFlags( n, ListBox::GetEntryFlags(n) | ListBoxEntryFlags::DisableSelection );
}

void CategoryListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    const sal_uInt16 nItem = rUDEvt.GetItemId();

    if( ListBox::GetEntryFlags(nItem) & ListBoxEntryFlags::DisableSelection )
    {
        ::tools::Rectangle aOutRect( rUDEvt.GetRect() );
        vcl::RenderContext* pDev = rUDEvt.GetRenderContext();

        // fill the background
        Color aColor (GetSettings().GetStyleSettings().GetDialogColor());

        pDev->SetFillColor (aColor);
        pDev->SetLineColor ();
        pDev->DrawRect(aOutRect);

        // Erase the four corner pixels to make the rectangle appear rounded.
        pDev->SetLineColor( GetSettings().GetStyleSettings().GetWindowColor());
        pDev->DrawPixel( aOutRect.TopLeft());
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Top()));
        pDev->DrawPixel( Point(aOutRect.Left(), aOutRect.Bottom()));
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Bottom()));

        // draw the category title
        pDev->DrawText (aOutRect, GetEntry(nItem), DrawTextFlags::Center );
    }
    else
    {
        DrawEntry( rUDEvt );
    }
}

IMPL_LINK_NOARG(CategoryListBox, implDoubleClickHdl, ListBox&, void)
{
    CaptureMouse();
}

void CategoryListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();
    if (!( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) ))
    {
        ListBox::MouseButtonUp( rMEvt );
    }
}

}
