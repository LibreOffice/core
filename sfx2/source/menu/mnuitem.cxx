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


#ifdef SOLARIS
#include <ctime>
#endif

#include <string>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <tools/urlobj.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/menuoptions.hxx>
#include <framework/menuconfiguration.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "idpool.hxx"
#include "sfxtypes.hxx"
#include "virtmenu.hxx"
#include <sfx2/mnuitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfx2/imgmgr.hxx"
#include "sfx2/imagemgr.hxx"
#include "sfx2/sfxresid.hxx"
#include "../doc/doc.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

//====================================================================

// binds the instance to the specified id and assignes the title

void SfxMenuControl::Bind(
    SfxVirtualMenu* pOwn,
    sal_uInt16 nSlotId,
    const String& rTitle,
    SfxBindings &rBindings )
{
    aTitle = rTitle;
    pOwnMenu = pOwn;
    pSubMenu = 0;
    if ( pOwn )
        SfxControllerItem::Bind(nSlotId, &rBindings);
    else
        SetId( nSlotId );

    DBG( CheckConfigure_Impl(SFX_SLOT_MENUCONFIG) );
}


//--------------------------------------------------------------------

// binds the item to the specified menu and assignes the title

void SfxMenuControl::Bind(
    SfxVirtualMenu* pOwn,
    sal_uInt16 nSlotId,
    SfxVirtualMenu& rMenu,
    const String& rTitle,
    SfxBindings &rBindings )
{
    SetId( nSlotId );
    SetBindings(rBindings);
    pOwnMenu = pOwn;
    pSubMenu = &rMenu;
    aTitle = rTitle;
}

//--------------------------------------------------------------------

// Constructor for explicit registration

SfxMenuControl::SfxMenuControl( sal_Bool bShowStrings )
:   pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(bShowStrings)
{
}

//--------------------------------------------------------------------

// Constructor for array
SfxMenuControl::SfxMenuControl():
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(sal_False)
{
}

//--------------------------------------------------------------------

SfxMenuControl::SfxMenuControl(sal_uInt16 nSlotId, SfxBindings& rBindings):
    SfxControllerItem(nSlotId, rBindings),
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(sal_False)
{
    // This constructor should make it possible already during the design
    // to fall back to the bindings, but can as always be bound later.
    // The usefulness of this is for example if a StatusForwarder should
    // be created in the constructor of a derived class.
    UnBind();
}


//--------------------------------------------------------------------

SfxMenuControl::~SfxMenuControl()
{
    delete pSubMenu;
}

//--------------------------------------------------------------------

// changes the state in the virtual menu

void SfxMenuControl::StateChanged
(
    sal_uInt16              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    (void)nSID;
    DBG_ASSERT( nSID == GetId(), "strange SID" );
    DBG_ASSERT( pOwnMenu != 0, "setting state to dangling SfxMenuControl" );

    bool bIsObjMenu =
                GetId() >= SID_OBJECTMENU0 && GetId() < SID_OBJECTMENU_LAST;

    // Fix inclusion of enabled/disabled-Flag

#ifdef UNIX
    if (nSID == SID_PASTE)
        pOwnMenu->EnableItem( GetId(), sal_True );
    else
#endif
    pOwnMenu->EnableItem( GetId(), bIsObjMenu
                ? 0 != pOwnMenu->GetSVMenu()->GetPopupMenu( GetId() )
                : eState != SFX_ITEM_DISABLED );

    if ( eState != SFX_ITEM_AVAILABLE )
    {
        // check only for non-Object Menus
        if ( !bIsObjMenu )
            pOwnMenu->CheckItem( GetId(), sal_False );

        if ( pOwnMenu->GetSVMenu()->GetItemText( GetId() ) != GetTitle() )
        {
             DBG_WARNING("Title of menu item changed - please check if this needs correction!");
        }
        return;
    }

    bool bCheck = false;
    if ( pState->ISA(SfxBoolItem) )
    {
        // BoolItem for check
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxBoolItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((const SfxBoolItem*)pState)->GetValue();
    }
    else if ( pState->ISA(SfxEnumItemInterface) &&
              ((SfxEnumItemInterface *)pState)->HasBoolValue() )
    {
        // Treat EnumItem as Bool
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxEnumItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((SfxEnumItemInterface *)pState)->GetBoolValue();
    }
    else if ( ( b_ShowStrings || bIsObjMenu ) && pState->ISA(SfxStringItem) )
    {
        // Get MenuText from SfxStringItem
        String aStr( ((const SfxStringItem*)pState)->GetValue() );
        if ( aStr.CompareToAscii("($1)",4) == COMPARE_EQUAL )
        {
            String aEntry(SfxResId(STR_UPDATEDOC).toString());
            aEntry += ' ';
            aEntry += aStr.Copy(4);
            aStr = aEntry;
        }
        else if ( aStr.CompareToAscii("($2)",4) == COMPARE_EQUAL )
        {
            String aEntry(SfxResId(STR_CLOSEDOC_ANDRETURN).toString());
            aEntry += aStr.Copy(4);
            aStr = aEntry;
        }

        pOwnMenu->SetItemText( GetId(), aStr );
    }

    pOwnMenu->CheckItem( GetId(), bCheck );
}

//--------------------------------------------------------------------

SfxMenuControl* SfxMenuControl::CreateImpl( sal_uInt16 /*nId*/, Menu& /*rMenu*/, SfxBindings& /*rBindings*/ )
{
    return new SfxMenuControl( sal_True );
}

void SfxMenuControl::RegisterControl( sal_uInt16 nSlotId, SfxModule *pMod )
{
    RegisterMenuControl( pMod, new SfxMenuCtrlFactory(
                SfxMenuControl::CreateImpl, TYPE(SfxStringItem), nSlotId ) );
}

//--------------------------------------------------------------------
void SfxMenuControl::RegisterMenuControl(SfxModule* pMod, SfxMenuCtrlFactory* pFact)
{
    SFX_APP()->RegisterMenuControl_Impl( pMod, pFact );
}

SfxMenuControl* SfxMenuControl::CreateControl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings )
{
    TypeId aSlotType = SFX_SLOTPOOL().GetSlotType(nId);
    if ( aSlotType )
    {
        SfxApplication *pApp = SFX_APP();
        SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
        SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
        if ( pMod )
        {
            SfxMenuCtrlFactArr_Impl *pFactories = pMod->GetMenuCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxMenuCtrlFactArr_Impl &rFactories = *pFactories;
                for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nId) ) )
                        return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
            }
        }

        SfxMenuCtrlFactArr_Impl &rFactories = pApp->GetMenuCtrlFactories_Impl();

        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nId) ) )
                return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
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

SfxUnoMenuControl* SfxMenuControl::CreateControl( const String& rCmd,
        sal_uInt16 nId, Menu& rMenu, const String& sItemText,
        SfxBindings& rBindings, SfxVirtualMenu* pVirt)
{
    return new SfxUnoMenuControl( rCmd, nId, rMenu, sItemText, rBindings, pVirt);
}

SfxUnoMenuControl::SfxUnoMenuControl(
    const String& rCmd, sal_uInt16 nSlotId, Menu& /*rMenu*/,
    const String& rItemText,
    SfxBindings& rBindings, SfxVirtualMenu* pVirt)
    : SfxMenuControl( nSlotId, rBindings )
{
    Bind( pVirt, nSlotId, rItemText, rBindings);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
