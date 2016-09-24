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

#include <FldRefTreeListBox.hxx>

#include <helpid.h>
#include <vcl/builderfactory.hxx>
#include <vcl/help.hxx>

SwFieldRefTreeListBox::SwFieldRefTreeListBox(vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
{
}

VCL_BUILDER_DECL_FACTORY(SwFieldRefTreeListBox)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SwFieldRefTreeListBox>::Create(pParent, nWinStyle);
}

void SwFieldRefTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase( true );
    if ( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            OUString sEntry( GetEntryText( pEntry ) );
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
                    QuickHelpFlags::Left|QuickHelpFlags::VCenter );
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
