/*************************************************************************
 *
 *  $RCSfile: objmnctl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 21:00:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


