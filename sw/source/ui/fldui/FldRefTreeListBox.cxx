/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <FldRefTreeListBox.hxx>

#include <helpid.h>
#include <vcl/help.hxx>

SwFldRefTreeListBox::SwFldRefTreeListBox( Window* pParent,
                                          const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
{
    SetHelpId( HID_REFSELECTION_TOOLTIP );
}

SwFldRefTreeListBox::~SwFldRefTreeListBox()
{
}

void SwFldRefTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase( true );
    if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            String sEntry( GetEntryText( pEntry ) );
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if( pItem )
            {
                aPos = GetEntryPosition( pEntry );

                aPos.X() = GetTabPos( pEntry, pTab );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.Width() = GetSizePixel().Width() - aPos.X();

                aPos = OutputToScreenPixel(aPos);
                Rectangle aItemRect( aPos, aSize );
                Help::ShowQuickHelp( this, aItemRect, sEntry,
                    QUICKHELP_LEFT|QUICKHELP_VCENTER );
                bCallBase = false;
            }
        }
    }
    if ( bCallBase )
    {
        Window::RequestHelp( rHEvt );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
