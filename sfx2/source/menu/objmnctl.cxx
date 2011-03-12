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
#include "precompiled_sfx2.hxx"
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/VerbAttributes.hpp>

#include <vcl/menu.hxx>
#include <svl/stritem.hxx>

#include <sfx2/sfxsids.hrc>
#include "objmnctl.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SfxObjectVerbsControl, SfxStringItem);

using namespace com::sun::star;
//--------------------------------------------------------------------

/*
    Ctor; setzt Select-Handler am Menu und traegt Menu
    in seinen Parent ein.
 */

SfxObjectVerbsControl::SfxObjectVerbsControl(sal_uInt16 nSlotId, Menu &rMenu, SfxBindings &rBindings)
    : SfxMenuControl( nSlotId, rBindings )
    , pMenu(new PopupMenu)
    , rParent(rMenu)
{
    rMenu.SetPopupMenu(nSlotId, pMenu);
    pMenu->SetSelectHdl(LINK(this, SfxObjectVerbsControl, MenuSelect));
    FillMenu();
}

//--------------------------------------------------------------------

/*
    Fuellt das Menu mit den aktuellen Verben aus der ViewShell.
 */

void SfxObjectVerbsControl::FillMenu()
{
    pMenu->Clear();
    SfxViewShell *pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    if (pView)
    {
        SfxObjectShell* pDoc = pView->GetObjectShell();
        const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& aVerbs =  pView->GetVerbs();
        if ( aVerbs.getLength() )
        {
            sal_uInt16 nSlotId = SID_VERB_START;
            for (sal_uInt16 n=0; n<aVerbs.getLength(); n++)
            {
                // check for ReadOnly verbs
                if ( pDoc->IsReadOnly() && !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES) )
                    continue;

                // check for verbs that shouldn't appear in the menu
                if ( !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU) )
                    continue;

                DBG_ASSERT(nSlotId <= SID_VERB_END, "Zuviele Verben!");
                if (nSlotId > SID_VERB_END)
                    break;

                pMenu->InsertItem(nSlotId++, aVerbs[n].VerbName);
            }
        }
    }

    rParent.EnableItem( GetId(), (sal_Bool)pMenu->GetItemCount() );
}

//--------------------------------------------------------------------

/*
    Statusbenachrichtigung;
    fuellt gfs. das Menu mit den aktuellen Verben aus der ViewShell.
    der DocumentShell.
    Ist die Funktionalit"at disabled, wird der entsprechende
    Menueeintrag im Parentmenu disabled, andernfalls wird er enabled.
 */

void SfxObjectVerbsControl::StateChanged(
    sal_uInt16 /*nSID*/,
    SfxItemState eState,
    const SfxPoolItem* /*pState*/ )
{
    rParent.EnableItem(GetId(), SFX_ITEM_AVAILABLE == eState );
    if ( SFX_ITEM_AVAILABLE == eState )
        FillMenu();
}

//--------------------------------------------------------------------

/*
    Select-Handler des Menus;
    das selektierte Verb mit ausgef"uhrt,
 */

IMPL_LINK_INLINE_START( SfxObjectVerbsControl, MenuSelect, Menu *, pSelMenu )
{
    const sal_uInt16 nSlotId = pSelMenu->GetCurItemId();
    if( nSlotId )
        GetBindings().Execute(nSlotId);
    return 1;
}
IMPL_LINK_INLINE_END( SfxObjectVerbsControl, MenuSelect, Menu *, pSelMenu )

//--------------------------------------------------------------------

/*
    Dtor; gibt das Menu frei.
 */

SfxObjectVerbsControl::~SfxObjectVerbsControl()
{
    delete pMenu;
}

//--------------------------------------------------------------------

PopupMenu* SfxObjectVerbsControl::GetPopup() const
{
    return pMenu;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
