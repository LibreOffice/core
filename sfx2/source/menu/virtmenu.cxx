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


#include <sot/factory.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/imagemgr.hxx>
#include <svl/imageitm.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "virtmenu.hxx"
#include <sfx2/msgpool.hxx>
#include "statcach.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/mnuitem.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"
#include "arrdecl.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/viewsh.hxx>
#include "sfxpicklist.hxx"
#include <sfx2/sfxresid.hxx>
#include "menu.hrc"
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <framework/addonsoptions.hxx>

#include <framework/addonmenu.hxx>
#include <framework/menuconfiguration.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

class SfxMenuImageControl_Impl : public SfxControllerItem
{
    SfxVirtualMenu*     pMenu;
    long                lRotation;
    bool                bIsMirrored;

protected:
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
public:
                        SfxMenuImageControl_Impl( sal_uInt16 nSlotId, SfxBindings& rBindings, SfxVirtualMenu* pVMenu )
                            : SfxControllerItem( nSlotId, rBindings )
                            , pMenu( pVMenu )
                            , lRotation( 0 )
                            , bIsMirrored( false )
                        {}
    void                Update();
};

void SfxMenuImageControl_Impl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState /*eState*/, const SfxPoolItem* pState )
{
    const SfxImageItem* pItem = dynamic_cast<const SfxImageItem*>( pState  );
    if ( pItem )
    {
        lRotation = pItem->GetRotation();
        bIsMirrored = pItem->IsMirrored();
        Update();
    }
}

void SfxMenuImageControl_Impl::Update()
{
    SfxViewFrame* pViewFrame = GetBindings().GetDispatcher_Impl()->GetFrame();
    SfxModule* pModule = pViewFrame->GetObjectShell()->GetModule();
    SfxSlotPool* pPool = pModule->GetSlotPool();
    Menu* pSVMenu = pMenu->GetSVMenu();
    for (sal_uInt16 nPos = 0; nPos<pSVMenu->GetItemCount(); nPos++)
    {
        sal_uInt16 nslotId = pSVMenu->GetItemId( nPos );
        const SfxSlot* pSlot = pPool->GetSlot( nslotId );
        if ( pSlot && pSlot->IsMode( SfxSlotMode::IMAGEROTATION ) )
        {
            pSVMenu->SetItemImageMirrorMode( nslotId, false );
            pSVMenu->SetItemImageAngle( nslotId, lRotation );
        }

        if ( pSlot && pSlot->IsMode( SfxSlotMode::IMAGEREFLECTION ) )
            pSVMenu->SetItemImageMirrorMode( nslotId, bIsMirrored );
    }
}



static Image RetrieveAddOnImage( Reference< com::sun::star::frame::XFrame >& rFrame,
                                 const OUString& aImageId,
                                 const OUString& aURL,
                                 bool bBigImage
)
{
    Image aImage;

    if ( !aImageId.isEmpty() )
    {
        aImage = GetImage( rFrame, aImageId, bBigImage );
        if ( !!aImage )
            return aImage;
    }

    aImage = GetImage( rFrame, aURL, bBigImage );
    if ( !aImage )
        aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage );

    return aImage;
}



/*  This helper function checks whether a Slot-id in the current application
    status is visible or not. This relates to the application status to see
    if the OLE server application exist or not.
*/

bool IsItemHidden_Impl( sal_uInt16 nItemId, bool bOleServer, bool bMac )
{
    return ( bMac &&
             ( nItemId == SID_MINIMIZED ) ) ||
           (  bOleServer &&
             ( nItemId == SID_QUITAPP || nItemId == SID_SAVEDOC ||
               nItemId == SID_OPENDOC || nItemId == SID_SAVEASDOC ||
               nItemId == SID_NEWDOC ) ) ||
           ( !bOleServer &&
             ( nItemId == SID_EXITANDRETURN || nItemId == SID_UPDATEDOC ) );
}



void SfxVirtualMenu::Construct_Impl()
{
    pSVMenu->SetHighlightHdl( LINK(this, SfxVirtualMenu, Highlight) );
    pSVMenu->SetActivateHdl( LINK(this, SfxVirtualMenu, Activate) );
    pSVMenu->SetDeactivateHdl( LINK(this, SfxVirtualMenu, Deactivate) );
    pSVMenu->SetSelectHdl( LINK(this, SfxVirtualMenu, Select) );

    if ( !pResMgr && pParent )
        pResMgr = pParent->pResMgr;
}



SfxVirtualMenu::SfxVirtualMenu( sal_uInt16 nOwnId,
                SfxVirtualMenu* pOwnParent, Menu& rMenu, bool bWithHelp,
                SfxBindings &rBindings, bool bOLEServer, bool bRes, bool bIsAddonMenu ):
    pItems(0),
       pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{
    pSVMenu = &rMenu;

    bResCtor = bRes;
    bOLE = bOLEServer;
    nId = nOwnId;
    pParent = pOwnParent;
    nVisibleItems = 0;
    pAppCtrl = 0;
    pWindowMenu = NULL;
    pPickMenu = NULL;
    pAddonsMenu = NULL;
    bIsActive = false;
    bControllersUnBound = false;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = false;
}



// creates a virtual menu from a StarView MenuBar or PopupMenu

SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, bool bWithHelp,
                    SfxBindings &rBindings, bool bOLEServer, bool bRes, bool bIsAddonMenu ):
    pItems(0),
       pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{

    pSVMenu = pStarViewMenu;

    bResCtor = bRes;
    bOLE = bOLEServer;
    nId = 0;
    pParent = 0;
    pAppCtrl = 0;
    nVisibleItems = 0;
    pWindowMenu = NULL;
    pPickMenu = NULL;
    pAddonsMenu = NULL;
    bIsActive = false;
    bControllersUnBound = false;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = false;
}



/*  The destructor of the class SfxVirtualMenu releases bounded items and
    the associated StarView-PopupMenu is released from its parent.
    If it is related to the Pickmenu or the MDI-menu, it is unregistered here.
*/

SfxVirtualMenu::~SfxVirtualMenu()
{

    DELETEZ( pImageControl );
    SvtMenuOptions().RemoveListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    if ( bIsActive )
    {
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = false;
    }

    if (pItems)
    {
        delete [] pItems;
    }

    delete pAppCtrl;
    pBindings = 0;

    // All the menus, which were created by SV, will also be there deleted
    // again (i.e. created by loading them from the resource)
    // The top-level menu is never deleted by SV, since the allocation
    // in done in the SFX
    if ( !bResCtor || !pParent)
    {
        if ( pParent )
        {
            if( pParent->pSVMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND )
                pParent->pSVMenu->SetPopupMenu( nId, 0 );
            if ( pParent->pPickMenu == pSVMenu )
                pParent->pPickMenu = 0;
            if ( pParent->pWindowMenu == pSVMenu)
                pParent->pWindowMenu = 0;
            if ( pParent->pAddonsMenu == pSVMenu )
                pParent->pAddonsMenu = 0;
        }

        delete pSVMenu;
    }

    SAL_INFO("sfx", "SfxVirtualMenu " << this << " destroyed");
    DBG_ASSERT( !nLocks, "destroying active menu" );
}

// internal: creates the virtual menu from the pSVMenu

void SfxVirtualMenu::CreateFromSVMenu()
{

    // Merge Addon popup menus into the SV Menu
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
    Reference< com::sun::star::frame::XFrame > xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( pSVMenu->IsMenuBar() )
    {
        Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        sal_uInt16 nPos = pSVMenu->GetItemPos( SID_MDIWINDOWLIST );
        if ( nPos != MENU_ITEM_NOTFOUND && xFrame.is() )
        {
            // Retrieve addon popup menus and add them to our menu bar
            framework::AddonMenuManager::MergeAddonPopupMenus( xFrame, nPos, static_cast<MenuBar *>(pSVMenu), xContext );
        }

        // Merge the Add-Ons help menu items into the Office help menu
        if ( xFrame.is() )
            framework::AddonMenuManager::MergeAddonHelpMenu( xFrame, static_cast<MenuBar *>(pSVMenu), xContext );

        // Set addon menu pointer here to avoid problems. When accessibility is enabled, the whole menu
        // is created immediately!
        pAddonsMenu = pSVMenu->GetPopupMenu( SID_ADDONLIST );
    }
    else if ( pParent )
    {
        if ( pSVMenu == pParent->pAddonsMenu &&
             framework::AddonsOptions().HasAddonsMenu() &&
             !pSVMenu->GetPopupMenu( SID_ADDONS ) )
        {
            // Create menu item at the end of the tools popup menu for the addons popup menu
            InsertAddOnsMenuItem( pSVMenu );
        }
    }

    // get and store the number of items
    nCount = pSVMenu->GetItemCount();

    // Note: only this time it is guaranteed that nCount and the ItemCount
    // the SV-menus match; later on the SvMenu can have more entries
    // (Pick list!)
    if (nCount)
        pItems = new SfxMenuControl[nCount];

    // remember some values
    SfxGetpApp();
    const bool bOleServer = false;
    const bool bMac = false;
    SvtMenuOptions aOptions;
    aOptions.AddListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    // iterate through the items
    pBindings->ENTERREGISTRATIONS(); ++nLocks;
    pImageControl = new SfxMenuImageControl_Impl( SID_IMAGE_ORIENTATION, *pBindings, this );

    sal_uInt16 nSVPos = 0;
    for ( sal_uInt16 nPos=0; nPos<nCount; ++nPos, ++nSVPos )
    {
        sal_uInt16 nSlotId = pSVMenu->GetItemId(nSVPos);
        PopupMenu* pPopup = pSVMenu->GetPopupMenu(nSlotId);
        if( pPopup && nSlotId >= SID_OBJECTMENU0 && nSlotId <= SID_OBJECTMENU_LAST )
        {
            // artefact in XML menuconfig: every entry in root menu must have a popup!
            pSVMenu->SetPopupMenu( nSlotId, NULL );
            DELETEZ( pPopup );
        }

        const OUString sItemText = pSVMenu->GetItemText(nSlotId);

        if ( pPopup )
        {

            SfxMenuControl *pMnuCtrl =
                SfxMenuControl::CreateControl(nSlotId, *pPopup, *pBindings);

            if ( pMnuCtrl )
            {
                // The pop was obviously not "real" and such are never loaded
                // from the resource and need thus to be explicitly deleted.
                if ( pSVMenu->GetPopupMenu( nSlotId ) == pPopup )
                    pSVMenu->SetPopupMenu( nSlotId, NULL );
                delete pPopup;
                pPopup = 0;

                SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                rCtrlArr.push_back(pMnuCtrl);
                (pItems+nPos)->Bind( 0, nSlotId, sItemText, *pBindings);
                pMnuCtrl->Bind( this, nSlotId, sItemText, *pBindings);

                if (  Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
                {
                    OUString aSlotURL( "slot:" );
                    aSlotURL += OUString::number( nSlotId);
                    Image aImage = GetImage( xFrame, aSlotURL, false );
                    pSVMenu->SetItemImage( nSlotId, aImage );
                }
            }
            else
            {
                pMnuCtrl = pItems+nPos;

                // Normally only now in Activate-Handler
                if ( bOLE )
                {
                    pMnuCtrl->Bind( this, nSlotId,
                        *new SfxVirtualMenu(nSlotId, this, *pPopup, bHelpInitialized, *pBindings, bOLE, bResCtor),
                        sItemText, *pBindings );
                }
            }

            ++nVisibleItems;
        }
        else
        {
            switch ( pSVMenu->GetItemType(nSVPos) )
            {
                case MenuItemType::STRING:
                case MenuItemType::STRINGIMAGE:
                {
                    SfxMenuControl *pMnuCtrl=0;
                    OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    if ( !aCmd.isEmpty() && (( nSlotId < SID_SFX_START ) || ( nSlotId > SHRT_MAX )) )
                    {
                        // try to create control via command name
                        pMnuCtrl = SfxMenuControl::CreateControl( aCmd, nSlotId, *pSVMenu, sItemText, *pBindings, this );
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.push_back(pMnuCtrl);
                            (pItems+nPos)->Bind( 0, nSlotId, sItemText, *pBindings);
                        }
                    }

                    if ( !pMnuCtrl )
                    {
                        // try to create control via Id
                        pMnuCtrl = SfxMenuControl::CreateControl(nSlotId, *pSVMenu, *pBindings);
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.push_back(pMnuCtrl);
                            (pItems+nPos)->Bind( 0, nSlotId, sItemText, *pBindings);
                        }
                        else
                            // take default control
                            pMnuCtrl = (pItems+nPos);

                        pMnuCtrl->Bind( this, nSlotId, sItemText, *pBindings);
                    }

                    if ( Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
                    {
                        Image aImage;
                        if ( bIsAddonPopupMenu || framework::AddonMenuManager::IsAddonMenuId( nSlotId ))
                        {
                            OUString aImageId;

                            ::framework::MenuAttributes* pMenuAttributes =
                                reinterpret_cast< ::framework::MenuAttributes*>(pSVMenu->GetUserValue( nSlotId ));

                            if ( pMenuAttributes )
                                aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                            aImage = RetrieveAddOnImage( xFrame, aImageId, aCmd, false );
                        }
                        else
                        {
                            OUString aSlotURL( "slot:" );
                            aSlotURL += OUString::number( nSlotId);
                            aImage = GetImage( xFrame, aSlotURL, false );
                        }

                        if ( !!aImage )
                            pSVMenu->SetItemImage( nSlotId, aImage );
                    }

                    if ( !IsItemHidden_Impl(nSlotId, bOleServer, bMac) )
                        ++nVisibleItems;
                    else
                        pSVMenu->RemoveItem( nSVPos-- );
                    break;
                }

                case MenuItemType::IMAGE:
                    //! not implemented
                    break;

                case MenuItemType::SEPARATOR:
                    //! not implemented
                    break;
                default:
                    break; // DONTKNOW and STRINGIMAGE not handled.
            }
        }
    }
    pBindings->LEAVEREGISTRATIONS(); --nLocks;
}



// called on activation of the SV-Menu

IMPL_STATIC_LINK_NOARG_TYPED(
    SfxVirtualMenu, Highlight, Menu *, bool )
{

    return true;
}

IMPL_LINK_NOARG_TYPED(SfxVirtualMenu, SettingsChanged, LinkParamNone*, void)
{
    sal_uInt16 nItemCount = pSVMenu->GetItemCount();
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    bool bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( !bIsAddonPopupMenu )
    {
        for ( sal_uInt16 nSVPos=0; nSVPos<nItemCount; ++nSVPos )
        {
            sal_uInt16      nSlotId = pSVMenu->GetItemId( nSVPos );
            MenuItemType    nType   = pSVMenu->GetItemType( nSVPos );
            if ( nType == MenuItemType::STRING && bIcons )
            {
                if ( framework::AddonMenuManager::IsAddonMenuId( nSlotId ))
                {
                    // Special code for Add-On menu items. They can appear inside the help menu.
                    OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    OUString aImageId;

                    ::framework::MenuAttributes* pMenuAttributes =
                        reinterpret_cast< ::framework::MenuAttributes*>(pSVMenu->GetUserValue( nSlotId ));

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, false ));
                }
                else
                {
                    OUString aSlotURL( "slot:" );
                    aSlotURL += OUString::number( nSlotId);
                    pSVMenu->SetItemImage( nSlotId, GetImage( xFrame, aSlotURL, false ));
                }
            }
            else if( nType == MenuItemType::STRINGIMAGE && !bIcons )
            {
                pSVMenu->SetItemImage( nSlotId, Image() );
            }
        }
    }
    else
    {
        // Remove/update images from Add-Ons top-level popup menus when settings have changed
        if ( !bIcons )
            RemoveMenuImages( pSVMenu );
        else
            UpdateImages( pSVMenu );
    }

    // Special code to remove menu images from runtime popup menus when settings have changed
    if ( pParent && pSVMenu == pParent->pAddonsMenu )
    {
        if ( !bIcons )
            RemoveMenuImages( pParent->pAddonsMenu->GetPopupMenu( SID_ADDONS ));
        else
            UpdateImages( pParent->pAddonsMenu->GetPopupMenu( SID_ADDONS ));
    }

    if ( pImageControl )
        pImageControl->Update();
}



void SfxVirtualMenu::UpdateImages( Menu* pMenu )
{
    if ( !pMenu )
        return;

    framework::AddonsOptions    aAddonOptions;

    bool bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    if ( bIcons )
    {
        sal_uInt16          nItemCount          = pMenu->GetItemCount();
        Reference<com::sun::star::frame::XFrame> aXFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

        for ( sal_uInt16 nPos=0; nPos < nItemCount; ++nPos )
        {
            sal_uInt16 nSlotId = pMenu->GetItemId( nPos );
            PopupMenu* pPopup = pMenu->GetPopupMenu( nSlotId );
            if ( pMenu->GetItemType( nPos ) != MenuItemType::SEPARATOR )
            {
                OUString aImageId;

                ::framework::MenuAttributes* pMenuAttributes =
                    reinterpret_cast< ::framework::MenuAttributes*>(pMenu->GetUserValue( nSlotId ));

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                pMenu->SetItemImage( nSlotId, RetrieveAddOnImage( aXFrame, aImageId, pMenu->GetItemCommand( nSlotId ), false ));
            }

            if ( pPopup )
                UpdateImages( pPopup );
        }

        if ( pImageControl )
            pImageControl->Update();
    }
}



void SfxVirtualMenu::RemoveMenuImages( Menu* pMenu )
{
    if ( !pMenu )
        return;

    sal_uInt16 nItemCount = pMenu->GetItemCount();
    for ( sal_uInt16 nPos=0; nPos < nItemCount; ++nPos )
    {
        sal_uInt16 nSlotId = pMenu->GetItemId( nPos );
        PopupMenu* pPopup = pMenu->GetPopupMenu( nSlotId );
        if ( pMenu->GetItemType( nPos ) == MenuItemType::STRINGIMAGE )
            pMenu->SetItemImage( nSlotId, Image() );
        if ( pPopup )
            RemoveMenuImages( pPopup );
    }
}



bool SfxVirtualMenu::Bind_Impl( Menu *pMenu )
{
    // Search , as SV with 'sal_uInt16 nSID = pSVMenu->GetCurItemId();' always
    // returns 0. It is like this, since the Event-Forwarding has nothing to do
    // with the Parent-Menus CurItem.

    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
    {
        sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        // found the Sub-Menu in question?
        bool bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            // Only a binded Menu-Controller as already an Id!
            if ( !rCtrl.GetId() )
            {
                bIsAddonPopupMenu = false;
                DBG_ASSERT( !pSubMenu, "Popup already exists!");

                // Check if the popup is an Add-On popup menu
                // Either the popup menu has a special ID  or a special command URL prefix!
                OUString aCommand = pSVMenu->GetItemCommand( nSID );
                if ( ( nSID == SID_ADDONS ) ||
                     ( nSID == SID_ADDONHELP ) ||
                     ( aCommand.startsWith( ADDONSPOPUPMENU_URL_PREFIX_STR ) ) )
                    bIsAddonPopupMenu = true;

                // Create VirtualMenu for Sub-Menu
                bool bRes = bResCtor;
                pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, false, *pBindings, bOLE, bRes, bIsAddonPopupMenu );

                SAL_INFO("sfx", "New VirtualMenu " << pSubMenu << " created");

                rCtrl.Bind( this, nSID, *pSubMenu, pSVMenu->GetItemText(nSID), *pBindings );

                // Forward Activate
                pSubMenu->Bind_Impl( pMenu );
                pSubMenu->Activate( pMenu );
            }
        }

        // continue searching recursively (SV Activate only the menu itself
        // and Top-Menu)
        if ( !bFound && pSubMenu )
            bFound = pSubMenu->Bind_Impl( pMenu );

        // If found, break
        if ( bFound )
            return true;
    }

    // Not found in this submenu
    return false;
}

void SfxVirtualMenu::BindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    sal_uInt16 nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl& rCtrl = pItems[nPos];
        if ( rCtrl.IsBindable_Impl() && !rCtrl.GetPopupMenu() )
            rCtrl.ReBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for (SfxMenuCtrlArr_Impl::iterator i = rCtrlArr.begin();
            i != rCtrlArr.end(); ++i)
    {
        sal_uInt16 nSlotId = i->GetId();
        if (pSVMenu->GetItemCommand(nSlotId).isEmpty())
        {
            i->ReBind();
        }
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = false;
}

void SfxVirtualMenu::UnbindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    sal_uInt16 nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( rCtrl.IsBound() )
            rCtrl.UnBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for (SfxMenuCtrlArr_Impl::iterator i = rCtrlArr.begin();
            i != rCtrlArr.end(); ++i)
    {
        if (i->IsBound())
        {
            // UnoController is not bound!
            i->UnBind();
        }
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = true;
}



void SfxVirtualMenu::InsertAddOnsMenuItem( Menu* pMenu )
{
    // Create special popup menu that is filled with the 3rd party components popup menu items
    ::framework::MenuConfiguration aConf( ::comphelper::getProcessComponentContext() );
    Reference<com::sun::star::frame::XFrame> xFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

    PopupMenu* pAddonMenu = NULL;
    try
    {
        Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        pAddonMenu = framework::AddonMenuManager::CreateAddonMenu(xFrame, xContext);
    }
    catch ( const ::com::sun::star::lang::WrappedTargetException& )
    {
    }

    // Create menu item at the end of the tools popup menu for the addons popup menu
    if ( pAddonMenu && pAddonMenu->GetItemCount() > 0 )
    {
        sal_uInt16 nItemCount = pMenu->GetItemCount();
        OUString aAddonsTitle(SfxResId(STR_MENU_ADDONS).toString());
        if ( nItemCount > 0 && pMenu->GetItemType( nItemCount-1 ) != MenuItemType::SEPARATOR )
            pMenu->InsertSeparator();
        pMenu->InsertItem( SID_ADDONS, aAddonsTitle );
        pMenu->SetPopupMenu( SID_ADDONS, pAddonMenu );

        if ( Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
        {
               OUString aSlotURL( "slot:" );
               aSlotURL += OUString::number( SID_ADDONS);
         pMenu->SetItemImage( SID_ADDONS, GetImage( xFrame, aSlotURL, false ));
        }
    }
    else
        delete pAddonMenu;
}



// called on activation of the SV-Menu

IMPL_LINK_TYPED( SfxVirtualMenu, Activate, Menu *, pMenu, bool )
{
    SAL_INFO(
        "sfx",
        "SfxVirtualMenu " << this << " activated " << pMenu << ", own "
            << pSVMenu);

    // MI: for what was it still good for?
    // MBA: seems to be an old QAP-Hack (checked-in in rev.1.41!)

    if ( pMenu )
    {
        bool bDontHide = SvtMenuOptions().IsEntryHidingEnabled();
        MenuFlags nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MenuFlags::HideDisabledEntries;
        else
            nFlag |= MenuFlags::HideDisabledEntries;
        pMenu->SetMenuFlags( nFlag );
    }

    // Own StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // Prevent Double Activate
        if ( bIsActive )
            return true;

        // ggf. Pick-Menu erzeugen
        if ( pParent && pSVMenu == pParent->pPickMenu )
        {
            SfxPickList::Get().CreateMenuEntries( pParent->pPickMenu );
        }
        else
            pPickMenu = pSVMenu->GetPopupMenu(SID_PICKLIST);

        if ( pParent && pSVMenu == pParent->pWindowMenu )
        {
            // update window list
            ::std::vector< OUString > aNewWindowListVector;
            Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );;

            sal_uInt16  nActiveItemId = 0;
            sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

            Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
            Reference< XIndexAccess > xList ( xDesktop->getFrames(), UNO_QUERY );
            sal_Int32 nFrameCount = xList->getCount();
            for( sal_Int32 i=0; i<nFrameCount; ++i )
            {
                Reference< XFrame > xFrame;
                Any aVal = xList->getByIndex(i);
                if (!(aVal>>=xFrame) || !xFrame.is() )
                    continue;

                if ( xFrame == xCurrentFrame )
                    nActiveItemId = nItemId;

                vcl::Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWin && pWin->IsVisible() )
                {
                    aNewWindowListVector.push_back( pWin->GetText() );
                    ++nItemId;
                }
            }

            int nItemCount       = pMenu->GetItemCount();

            if ( nItemCount > 0 )
            {
                // remove all old window list entries from menu
                sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
                for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                    pMenu->RemoveItem( n );

                if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MenuItemType::SEPARATOR )
                    pMenu->RemoveItem( pMenu->GetItemCount()-1 );
            }

            if ( aNewWindowListVector.size() > 0 )
            {
                // append new window list entries to menu
                pMenu->InsertSeparator();
                nItemId = START_ITEMID_WINDOWLIST;
                for ( size_t i = 0; i < aNewWindowListVector.size(); i++ )
                {
                    pMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MenuItemBits::RADIOCHECK );
                    if ( nItemId == nActiveItemId )
                        pMenu->CheckItem( nItemId );
                    ++nItemId;
                }
            }
        }
        else
            pWindowMenu = pSVMenu->GetPopupMenu(SID_MDIWINDOWLIST);

        if ( !pParent && pSVMenu->IsMenuBar() && !pAddonsMenu )
        {
            // Store Add-Ons parents of our runtime menu items
            pAddonsMenu = pSVMenu->GetPopupMenu( SID_ADDONLIST );
        }

        // Provides consistency to the Status
        if ( bControllersUnBound )
            BindControllers();

        pBindings->GetDispatcher_Impl()->Flush();
        for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
        {
            sal_uInt16 nSlotId = (pItems+nPos)->GetId();
            if ( nSlotId && nSlotId > END_ITEMID_WINDOWLIST )
                pBindings->Update(nSlotId);
        }

        pBindings->Update( SID_IMAGE_ORIENTATION );

        // Suppress the Status updates until Deactivate
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = true;

        return true;
    }
    else
    {
        // Find the VirtualMenu for the SubMenu and if possible, bind a
        // VirtualMenu
        bool bRet = pMenu && Bind_Impl(pMenu);
#ifdef DBG_UTIL
        if ( !bRet)
            SAL_INFO( "sfx", "W1: Virtual menu could not be created!" );
#endif
        return bRet;
    }
}



IMPL_LINK_TYPED( SfxVirtualMenu, Deactivate, Menu *, pMenu, bool )
{
    SAL_INFO(
        "sfx",
        "SfxVirtualMenu " << this << " deactivated " << pMenu << ", own "
            << pSVMenu);
    if ( bIsActive && ( 0 == pMenu || pMenu == pSVMenu ) )
    {
        // All controllers can be unbinded all the way up to the Menubar,
        // when the menu is disabled (= closed)
        if ( pParent )
            UnbindControllers();
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = false;
    }
    return true;
}


// called on activation of the SV-Menu

IMPL_LINK_TYPED( SfxVirtualMenu, Select, Menu *, pMenu, bool )
{
    sal_uInt16 nSlotId = (sal_uInt16) pMenu->GetCurItemId();
    SAL_INFO(
        "sfx",
        "SfxVirtualMenu " << this << " selected " << nSlotId << " from "
            << pMenu);

    if ( nSlotId >= START_ITEMID_WINDOWLIST && nSlotId <= END_ITEMID_WINDOWLIST )
    {
        // window list menu item selected
        Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
        Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
        sal_Int32 nFrameCount = xList->getCount();
        for ( sal_Int32 i=0; i<nFrameCount; ++i )
        {
            Any aItem = xList->getByIndex(i);
            Reference< XFrame > xFrame;
            if (( aItem >>= xFrame ) && xFrame.is() && nTaskId == nSlotId )
            {
                vcl::Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                pWin->GrabFocus();
                pWin->ToTop( ToTopFlags::RestoreWhenMin );
                break;
            }

            nTaskId++;
        }

        return true;
    }
    else if ( nSlotId >= START_ITEMID_PICKLIST && nSlotId <= END_ITEMID_PICKLIST )
    {
        SfxPickList::ExecuteMenuEntry( nSlotId );
        return true;
    }

    OUString sCommand = pMenu->GetItemCommand(nSlotId);
    if (!sCommand.isEmpty())
        pBindings->ExecuteCommand_Impl(sCommand);
    else
        pBindings->Execute(nSlotId);

    return true;
}



// returns the associated StarView-menu




// set the checkmark of the specified item

void SfxVirtualMenu::CheckItem( sal_uInt16 nItemId, bool bCheck )
{
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->CheckItem( nItemId, bCheck );
}


// set the enabled-state of the specified item

void SfxVirtualMenu::EnableItem( sal_uInt16 nItemId, bool bEnable )
{
    DBG_ASSERT( pSVMenu != 0, "" );

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->EnableItem( nItemId, bEnable );
}


// set the text of the specified item

void SfxVirtualMenu::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->SetItemText( nItemId, rText );
}




void SfxVirtualMenu::SetPopupMenu( sal_uInt16 nItemId, PopupMenu *pMenu )
{

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        GetSVMenu()->SetPopupMenu( nItemId, pMenu );
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SfxVirtualMenu *pSubMenu = (pItems+n)->GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->SetPopupMenu( nItemId, pMenu );
    }
}



// Forces the initialization, which is otherwise only happens in Activate

void SfxVirtualMenu::InitPopup( sal_uInt16 nPos, bool /*bOLE*/ )
{

    sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "No popup exist here!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        // Generate VirtualMenu for Sub-Menu
        bool bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, false, *pBindings, bOLE, bRes);

        SAL_INFO("sfx", "New VirtualMenu " << pSubMenu << " created");

        rCtrl.Bind( this, nSID, *pSubMenu, pSVMenu->GetItemText(nSID), *pBindings );
    }
}

void SfxVirtualMenu::InitializeHelp()
{
    for ( sal_uInt16 nPos = 0; nPos<pSVMenu->GetItemCount(); ++nPos )
    {
        sal_uInt16 nSlotId = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( nSlotId && !rCtrl.GetId() )
        {
            InitPopup( nPos );
        }

        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->InitializeHelp();
    }

    bHelpInitialized = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
