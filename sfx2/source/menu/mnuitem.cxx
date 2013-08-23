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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>
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
    const OUString& rTitle,
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
    const OUString& rTitle,
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

long Select_Impl( void* pHdl, void* pVoid );

SFX_IMPL_MENU_CONTROL( SfxAppMenuControl_Impl, SfxStringItem );

SfxAppMenuControl_Impl::SfxAppMenuControl_Impl(
    sal_uInt16 nPos, Menu& rMenu, SfxBindings& rBindings )
    : SfxMenuControl( nPos, rBindings ), pMenu(0)
{
    // Determine the current background color setting for menus
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_nSymbolsStyle         = rSettings.GetSymbolsStyle();
    m_bShowMenuImages       = rSettings.GetUseImagesInMenus();

    ::framework::MenuConfiguration aConf( ::comphelper::getProcessComponentContext() );
    Reference<com::sun::star::frame::XFrame> aXFrame( GetBindings().GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );
    pMenu = aConf.CreateBookmarkMenu( aXFrame, GetId() == SID_NEWDOCDIRECT ? BOOKMARK_NEWMENU : BOOKMARK_WIZARDMENU );
    if( pMenu )
    {
        pMenu->SetSelectHdl( Link( &(this->GetBindings()), Select_Impl ) );
        pMenu->SetActivateHdl( LINK(this, SfxAppMenuControl_Impl, Activate) );
        rMenu.SetPopupMenu( nPos, pMenu );
    }
}

SfxAppMenuControl_Impl::~SfxAppMenuControl_Impl()
{
    delete pMenu;
}

IMPL_LINK( SfxAppMenuControl_Impl, Activate, Menu *, pActMenu )
{
    if ( pActMenu )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_uIntPtr nSymbolsStyle = rSettings.GetSymbolsStyle();
        sal_Bool bShowMenuImages = rSettings.GetUseImagesInMenus();

        if (( nSymbolsStyle != m_nSymbolsStyle ) ||
            ( bShowMenuImages != m_bShowMenuImages ))
        {
            m_nSymbolsStyle         = nSymbolsStyle;
            m_bShowMenuImages       = bShowMenuImages;

            sal_uInt16 nCount = pActMenu->GetItemCount();
            for ( sal_uInt16 nSVPos = 0; nSVPos < nCount; nSVPos++ )
            {
                sal_uInt16 nItemId = pActMenu->GetItemId( nSVPos );
                if ( pActMenu->GetItemType( nSVPos ) != MENUITEM_SEPARATOR )
                {
                    if ( bShowMenuImages )
                    {
                        sal_Bool        bImageSet = sal_False;
                        ::rtl::OUString aImageId;
                        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nItemId );

                        if ( pMenuAttributes )
                            aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                        if ( aImageId.getLength() > 0 )
                        {
                            Reference< ::com::sun::star::frame::XFrame > xFrame;
                            Image aImage = GetImage( xFrame, aImageId, sal_False );
                            if ( !!aImage )
                            {
                                bImageSet = sal_True;
                                pActMenu->SetItemImage( nItemId, aImage );
                            }
                        }

                        String aCmd( pActMenu->GetItemCommand( nItemId ) );
                        if ( !bImageSet && aCmd.Len() )
                        {
                            Image aImage = SvFileInformationManager::GetImage(
                                INetURLObject(aCmd), sal_False );
                            if ( !!aImage )
                                pActMenu->SetItemImage( nItemId, aImage );
                        }
                    }
                    else
                        pActMenu->SetItemImage( nItemId, Image() );
                }
            }
        }

        return sal_True;
    }

    return sal_False;
}

SfxUnoMenuControl* SfxMenuControl::CreateControl( const OUString& rCmd,
        sal_uInt16 nId, Menu& rMenu, const OUString& sItemText,
        SfxBindings& rBindings, SfxVirtualMenu* pVirt)
{
    return new SfxUnoMenuControl( rCmd, nId, rMenu, sItemText, rBindings, pVirt);
}

SfxUnoMenuControl::SfxUnoMenuControl(
    const OUString& rCmd, sal_uInt16 nSlotId, Menu& /*rMenu*/,
    const OUString& rItemText,
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

long Select_Impl( void* /*pHdl*/, void* pVoid )
{
    Menu* pMenu = (Menu*)pVoid;
    String aURL( pMenu->GetItemCommand( pMenu->GetCurItemId() ) );

    if( !aURL.Len() )
        return 0;

    Reference < ::com::sun::star::frame::XDesktop2 > xDesktop =
            ::com::sun::star::frame::Desktop::create( ::comphelper::getProcessComponentContext() );

    URL aTargetURL;
    aTargetURL.Complete = aURL;
    Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatch > xDisp;
    if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
        xDisp = xDesktop->queryDispatch( aTargetURL, OUString(), 0 );
    else
    {
        OUString aTargetFrame( OUString("_blank") );
        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

        if ( pMenuAttributes )
            aTargetFrame = pMenuAttributes->aTargetFrame;

        xDisp = xDesktop->queryDispatch( aTargetURL, aTargetFrame , 0 );
    }

    if ( xDisp.is() )
    {
        SfxAppMenuControl_Impl::ExecuteInfo* pExecuteInfo = new SfxAppMenuControl_Impl::ExecuteInfo;
        pExecuteInfo->xDispatch     = xDisp;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = Sequence< PropertyValue >();
        Application::PostUserEvent( STATIC_LINK( 0, SfxAppMenuControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
    }

    return sal_True;
}

IMPL_STATIC_LINK_NOINSTANCE( SfxAppMenuControl_Impl, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    delete pExecuteInfo;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
