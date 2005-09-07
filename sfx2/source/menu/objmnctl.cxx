/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objmnctl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:19:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_EMBED_VERBDESCRIPTOR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_VERBATTRIBUTES_HPP_
#include <com/sun/star/embed/VerbAttributes.hpp>
#endif

#include <tools/list.hxx>
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef GCC
#pragma hdrstop
#endif

#include "sfxsids.hrc"
#include "objmnctl.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SfxObjectVerbsControl, SfxStringItem);

using namespace com::sun::star;
//--------------------------------------------------------------------

/*
    Ctor; setzt Select-Handler am Menu und traegt Menu
    in seinen Parent ein.
 */

SfxObjectVerbsControl::SfxObjectVerbsControl(USHORT nId, Menu &rMenu, SfxBindings &rBindings)
    : SfxMenuControl( nId, rBindings )
    , pMenu(new PopupMenu)
    , rParent(rMenu)
{
    rMenu.SetPopupMenu(nId, pMenu);
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
            USHORT nId = SID_VERB_START;
            for (USHORT n=0; n<aVerbs.getLength(); n++)
            {
                // check for ReadOnly verbs
                if ( pDoc->IsReadOnly() && !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES) )
                    continue;

                // check for verbs that shouldn't appear in the menu
                if ( !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU) )
                    continue;

                DBG_ASSERT(nId <= SID_VERB_END, "Zuviele Verben!");
                if (nId > SID_VERB_END)
                    break;

                pMenu->InsertItem(nId++, aVerbs[n].VerbName);
            }
        }
    }

    rParent.EnableItem( GetId(), (BOOL)pMenu->GetItemCount() );
}

//--------------------------------------------------------------------

/*
    Statusbenachrichtigung;
    fuellt gfs. das Menu mit den aktuellen Verben aus der ViewShell.
    der DocumentShell.
    Ist die Funktionalit"at disabled, wird der entsprechende
    Menueeintrag im Parentmenu disabled, andernfalls wird er enabled.
 */

void SfxObjectVerbsControl::StateChanged( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState )
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

IMPL_LINK_INLINE_START( SfxObjectVerbsControl, MenuSelect, Menu *, pMenu )
{
    const USHORT nId = pMenu->GetCurItemId();
    if( nId )
        GetBindings().Execute(nId);
    return 1;
}
IMPL_LINK_INLINE_END( SfxObjectVerbsControl, MenuSelect, Menu *, pMenu )

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


