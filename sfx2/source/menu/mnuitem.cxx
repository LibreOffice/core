/*************************************************************************
 *
 *  $RCSfile: mnuitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop includes

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#pragma hdrstop

#include "appdata.hxx"
#include "sfx.hrc"
#include "msgpool.hxx"
#include "msgdescr.hxx"
#include "msg.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "idpool.hxx"
#include "sfxtypes.hxx"
#include "macrconf.hxx"
#include "virtmenu.hxx"
#include "mnuitem.hxx"
#include "tbxctrl.hxx"
#include "arrdecl.hxx"
#include "module.hxx"

//====================================================================

class SfxEnumMenu: public PopupMenu
{
    USHORT          nSlot;
    SfxEnumItem    *pItem;
    SfxBindings*    pBindings;

protected:
    virtual void    Select();

public:
                    SfxEnumMenu( USHORT nSlot, SfxBindings* pBind, const SfxEnumItem &rItem );
                    ~SfxEnumMenu();
};

//=========================================================================

SfxEnumMenu::SfxEnumMenu( USHORT nSlotId, SfxBindings* pBind, const SfxEnumItem &rItem ):
    nSlot( nSlotId ),
    pItem( (SfxEnumItem*) rItem.Clone() ),
    pBindings( pBind )
{
    for ( USHORT nVal = 0; nVal < pItem->GetValueCount(); ++nVal )
        InsertItem( nVal+1, pItem->GetValueTextByPos(nVal) );
    CheckItem( pItem->GetValue() + 1, TRUE );
}

//-------------------------------------------------------------------------

SfxEnumMenu::~SfxEnumMenu()
{
    delete pItem;
}

//-------------------------------------------------------------------------

void SfxEnumMenu::Select()
{
    pItem->SetValue( GetCurItemId()-1 );
    pBindings->GetDispatcher()->Execute( nSlot,
                SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                pItem, 0L, 0L );
}

//--------------------------------------------------------------------

void SfxMenuControl::SetOwnMenu( SfxVirtualMenu* pMenu )
{
    pOwnMenu = pMenu;
    if ( pSubMenu )
        pSubMenu->SetParentMenu( pMenu );
}


//--------------------------------------------------------------------

// binds the instance to the specified id and assignes the title

void SfxMenuControl::Bind( SfxVirtualMenu* pOwn,
                           USHORT nId, const String& rTitle,
                           const String &rHelpText,
                           SfxBindings &rBindings )
{
    DBG_MEMTEST();

    aTitle = rTitle;
    aHelpText = rHelpText;
    pOwnMenu = pOwn;
    pSubMenu = 0;
    if ( pOwn )
        SfxControllerItem::Bind(nId, &rBindings);
    else
        SetId( nId );

    DBG( CheckConfigure_Impl(SFX_SLOT_MENUCONFIG) );
}


//--------------------------------------------------------------------

// binds the item to the specified menu and assignes the title

void SfxMenuControl::Bind( SfxVirtualMenu* pOwn,
                           USHORT nId, SfxVirtualMenu& rMenu,
                           const String& rTitle, const String &rHelpText,
                           SfxBindings &rBindings )
{
    DBG_MEMTEST();
    if ( nId == SID_NONE )
        nId = SFX_SLOTPOOL().GetIdPool().Get();
    SetId( nId );
    SetBindings(rBindings);
    pOwnMenu = pOwn;
    pSubMenu = &rMenu;
    aTitle = rTitle;
    aHelpText = rHelpText;
}

//--------------------------------------------------------------------

// ctor for explicit registration

SfxMenuControl::SfxMenuControl( BOOL bShowStrings )
:   pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(bShowStrings)
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

// ctor for array

SfxMenuControl::SfxMenuControl():
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(FALSE)
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

SfxMenuControl::SfxMenuControl(USHORT nId, SfxBindings& rBindings):
    SfxControllerItem(nId, rBindings),
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(FALSE)
{
    DBG_MEMTEST();

    // Dieser Ctor soll es erm"oglichen, w"ahrend der Konstruktion schon
    // auf die Bindings zur"uckgreifen zu k"onnen, aber gebunden wird
    // wie immer erst sp"ater. Anwendung z.B. wenn im ctor der abgeleiteten
    // Klasse z.B. ein StatusForwarder erzeugt werden soll.
    UnBind();
}


//--------------------------------------------------------------------

// dtor

SfxMenuControl::~SfxMenuControl()
{
    if ( SfxMacroConfig::IsMacroSlot( GetId() ) )
        SFX_APP()->GetMacroConfig()->ReleaseSlotId(GetId());
    delete pSubMenu;
}

void SfxMenuControl::RemovePopup()
{
    DELETEZ( pSubMenu );
}

//--------------------------------------------------------------------

// changes the state in the virtual menu

void SfxMenuControl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_MEMTEST();
    DBG_ASSERT( nSID == GetId(), "strange SID" );
    DBG_ASSERT( pOwnMenu != 0, "setting state to dangling SfxMenuControl" );

    FASTBOOL bIsObjMenu =
                GetId() >= SID_OBJECTMENU0 && GetId() < SID_OBJECTMENU_LAST;

    // enabled/disabled-Flag pauschal korrigieren
    pOwnMenu->EnableItem( GetId(), bIsObjMenu
                ? 0 != pOwnMenu->GetSVMenu()->GetPopupMenu( GetId() )
                : eState != SFX_ITEM_DISABLED );

    if ( eState != SFX_ITEM_AVAILABLE )
    {
        // checken nur bei nicht-Object-Menus
        if ( !bIsObjMenu )
            pOwnMenu->CheckItem( GetId(), FALSE );

        // SetItemText flackert in MenuBar insbes. unter OS/2 (Bug #20658)
        if ( // !bIsObjMenu && nicht wegen "Format/Datenbank"
             pOwnMenu->GetSVMenu()->GetItemText( GetId() ) != GetTitle() )
            pOwnMenu->SetItemText( GetId(), GetTitle() );

        return;
    }

    // ggf. das alte Enum-Menu entfernen/loeschen
    //! delete pOwnMenu->GetMenu().ChangePopupMenu( GetId(), 0 );

    FASTBOOL bCheck = FALSE;
    if ( pState->ISA(SfxBoolItem) )
    {
        // BoolItem fuer checken
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxBoolItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((const SfxBoolItem*)pState)->GetValue();
    }
    else if ( pState->ISA(SfxEnumItemInterface) &&
              ((SfxEnumItemInterface *)pState)->HasBoolValue() )
    {
        // EnumItem wie Bool behandeln
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxEnumItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((SfxEnumItemInterface *)pState)->GetBoolValue();
    }
    else if ( ( b_ShowStrings || bIsObjMenu ) && pState->ISA(SfxStringItem) )
        // MenuText aus SfxStringItem holen
        pOwnMenu->SetItemText( GetId(), ((const SfxStringItem*)pState)->GetValue());

#ifdef enum_item_menu_ok
    else if ( aType == TYPE(SfxEnumItem) )
    {
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxEnumItem not allowed for SID_OBJECTMENUx" );
        pOwnMenu->GetMenu().ChangePopupMenu( GetId(), &GetBindings(),
            new SfxEnumMenu( GetId(), *(const SfxEnumItem*)pState ) );
    }
#endif

    pOwnMenu->CheckItem( GetId(), bCheck );
}

//--------------------------------------------------------------------

SfxMenuControl* SfxMenuControl::CreateImpl( USHORT nId, Menu &rMenu, SfxBindings &rBindings )
{
    return new SfxMenuControl( TRUE );
}

//--------------------------------------------------------------------

void SfxMenuControl::RegisterControl( USHORT nSlotId, SfxModule *pMod )
{
    SFX_APP()->RegisterMenuControl( pMod, new SfxMenuCtrlFactory(
                SfxMenuControl::CreateImpl, TYPE(SfxStringItem), nSlotId ) );
}

//--------------------------------------------------------------------

SfxMenuControl* SfxMenuControl::CreateControl( USHORT nId, Menu &rMenu, SfxBindings &rBindings )
{
    TypeId aSlotType = SFX_SLOTPOOL().GetSlotType(nId);
    if ( aSlotType )
    {
        SfxApplication *pApp = SFX_APP();
        SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
        SfxModule *pMod = pDisp ? pApp->GetActiveModule( pDisp->GetFrame() ) :0;
        if ( pMod )
        {
            SfxMenuCtrlFactArr_Impl *pFactories = pMod->GetMenuCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxMenuCtrlFactArr_Impl &rFactories = *pFactories;
                for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nId) ) )
                        return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
            }
        }

        SfxMenuCtrlFactArr_Impl &rFactories = pApp->GetMenuCtrlFactories_Impl();

        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nId) ) )
                return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
    }
    return 0;
}



BOOL SfxMenuControl::IsSpecialControl( USHORT nId, SfxBindings& rBindings  )
{
    TypeId aSlotType = SFX_SLOTPOOL().GetSlotType( nId );
    if ( aSlotType )
    {
        SfxApplication *pApp = SFX_APP();
        SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
        SfxModule *pMod = pDisp ? pApp->GetActiveModule( pDisp->GetFrame() ) :0;
        if ( pMod )
        {
            SfxMenuCtrlFactArr_Impl *pFactories = pMod->GetMenuCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxMenuCtrlFactArr_Impl &rFactories = *pFactories;
                for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nId) ) )
                        return TRUE;
            }
        }

        SfxMenuCtrlFactArr_Impl &rFactories = pApp->GetMenuCtrlFactories_Impl();

        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nId) ) )
                return TRUE;
    }
    return 0;
}

//--------------------------------------------------------------------

PopupMenu* SfxMenuControl::GetPopup () const
{
    if (GetPopupMenu())
        return (PopupMenu*)GetPopupMenu()->GetSVMenu();
    else
        return 0;
}

SFX_IMPL_MENU_CONTROL( SfxAppMenuControl_Impl, SfxStringItem );

SfxAppMenuControl_Impl::SfxAppMenuControl_Impl(
    USHORT nPos, Menu& rMenu, SfxBindings& rBindings )
    : SfxMenuControl( nPos, rBindings ), pMenu(0)
{
    String aText = rMenu.GetItemText( nPos );
    SfxApplication* pApp = SFX_APP();
    SfxAppData_Impl* pImpl = pApp->Get_Impl();
    PopupMenu* pView =  pImpl->GetPopupMenu( nPos );
    if ( pView )
        rMenu.SetPopupMenu( nPos, pView );
}

SfxAppMenuControl_Impl::~SfxAppMenuControl_Impl()
{
}

SfxUnoMenuControl* SfxMenuControl::CreateControl( const String& rCmd,
        USHORT nId, Menu& rMenu, SfxBindings &rBindings, SfxVirtualMenu* pVirt )
{
    return new SfxUnoMenuControl( rCmd, nId, rMenu, rBindings, pVirt );
}

SfxUnoMenuControl::SfxUnoMenuControl( const String& rCmd, USHORT nId,
    Menu& rMenu, SfxBindings& rBindings, SfxVirtualMenu* pVirt )
    : SfxMenuControl( nId, rBindings )
{
    Bind( pVirt, nId, rMenu.GetItemText(nId),
                        rMenu.GetHelpText(nId), rBindings);
    UnBind();
    pUnoCtrl = new SfxUnoControllerItem( this, rBindings, rCmd );
    pUnoCtrl->acquire();
    pUnoCtrl->GetNewDispatch();
}

SfxUnoMenuControl::~SfxUnoMenuControl()
{
    pUnoCtrl->UnBind();
    pUnoCtrl->release();
}

void SfxUnoMenuControl::Select()
{
    pUnoCtrl->Execute();
}


