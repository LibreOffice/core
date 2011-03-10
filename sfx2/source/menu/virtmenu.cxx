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

#include <sot/factory.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/imagemgr.hxx>
#include <svl/imageitm.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/unohlp.hxx>
#include <tools/urlobj.hxx>

#include "virtmenu.hxx"
#include <sfx2/msgpool.hxx>
#include "statcach.hxx"
#include <sfx2/msg.hxx>
#include "idpool.hxx"
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
#include <sfx2/macrconf.hxx>
#include "sfxresid.hxx"
#include "menu.hrc"
#include "imagemgr.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <framework/addonsoptions.hxx>

#include <framework/addonmenu.hxx>
#include <framework/menuconfiguration.hxx>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

//=========================================================================

DBG_NAME(SfxVirtualMenu)

//=========================================================================

typedef SfxMenuControl* SfxMenuControlPtr;
SV_IMPL_PTRARR(SfxMenuCtrlArr_Impl, SfxMenuControlPtr);

class SfxMenuImageControl_Impl : public SfxControllerItem
{
    SfxVirtualMenu*     pMenu;
    long                lRotation;
    BOOL                bIsMirrored;

protected:
    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
public:
                        SfxMenuImageControl_Impl( USHORT nSlotId, SfxBindings& rBindings, SfxVirtualMenu* pVMenu )
                            : SfxControllerItem( nSlotId, rBindings )
                            , pMenu( pVMenu )
                            , lRotation( 0 )
                            , bIsMirrored( FALSE )
                        {}
    void                Update();
};

void SfxMenuImageControl_Impl::StateChanged( USHORT /*nSID*/, SfxItemState /*eState*/, const SfxPoolItem* pState )
{
    const SfxImageItem* pItem = PTR_CAST( SfxImageItem, pState );
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
    for (USHORT nPos = 0; nPos<pSVMenu->GetItemCount(); nPos++)
    {
        USHORT nslotId = pSVMenu->GetItemId( nPos );
        const SfxSlot* pSlot = pPool->GetSlot( nslotId );
        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEROTATION ) )
        {
            pSVMenu->SetItemImageMirrorMode( nslotId, FALSE );
            pSVMenu->SetItemImageAngle( nslotId, lRotation );
        }

        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEREFLECTION ) )
            pSVMenu->SetItemImageMirrorMode( nslotId, bIsMirrored );
    }
}

//=========================================================================

static Image RetrieveAddOnImage( Reference< com::sun::star::frame::XFrame >& rFrame,
                                 const rtl::OUString& aImageId,
                                 const rtl::OUString& aURL,
                                 BOOL bBigImage
)
{
    Image aImage;

    if ( aImageId.getLength() > 0 )
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

//=========================================================================

/*  This helper function checks whether a Slot-id in the current application
    status is visible or not. This relates to the application status to see
    if the OLE server application exist or not.
*/

BOOL IsItemHidden_Impl( USHORT nItemId, int bOleServer, int bMac )
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

//====================================================================

void SfxVirtualMenu::Construct_Impl()
{
    pSVMenu->SetHighlightHdl( LINK(this, SfxVirtualMenu, Highlight) );
    pSVMenu->SetActivateHdl( LINK(this, SfxVirtualMenu, Activate) );
    pSVMenu->SetDeactivateHdl( LINK(this, SfxVirtualMenu, Deactivate) );
    pSVMenu->SetSelectHdl( LINK(this, SfxVirtualMenu, Select) );

    if ( !pResMgr && pParent )
        pResMgr = pParent->pResMgr;
}

//--------------------------------------------------------------------

SfxVirtualMenu::SfxVirtualMenu( USHORT nOwnId,
                SfxVirtualMenu* pOwnParent, Menu& rMenu, BOOL bWithHelp,
                SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes, BOOL bIsAddonMenu ):
    pItems(0),
    pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    pAutoDeactivate(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{
    DBG_MEMTEST();
    DBG_CTOR(SfxVirtualMenu, 0);
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
    bIsActive = FALSE;
    bControllersUnBound = FALSE;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
}

//--------------------------------------------------------------------

// creates a virtual menu from a StarView MenuBar or PopupMenu

SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, BOOL bWithHelp,
                    SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes, BOOL bIsAddonMenu ):
    pItems(0),
       pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    pAutoDeactivate(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{
    DBG_MEMTEST();
    DBG_CTOR(SfxVirtualMenu, 0);

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
    bIsActive = FALSE;
    bControllersUnBound = FALSE;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
}

//--------------------------------------------------------------------

/*  The destructor of the class SfxVirtualMenu releases bounded items and
    the associated StarView-PopupMenu is released from its parent.
    If it is related to the Pickmenu or the MDI-menu, it is unregistered here.
*/

SfxVirtualMenu::~SfxVirtualMenu()
{
    DBG_MEMTEST();
    DBG_DTOR(SfxVirtualMenu, 0);

    DELETEZ( pImageControl );
    SvtMenuOptions().RemoveListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    if ( bIsActive )
    {
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = FALSE;
    }

    // QAP-Hack
    if ( pAutoDeactivate )
    {
        if ( pAutoDeactivate->IsActive() )
            Deactivate(0);
        DELETEX(pAutoDeactivate);
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

    DBG_OUTF( ("SfxVirtualMenu %lx destroyed", this) );
    DBG_ASSERT( !nLocks, "destroying active menu" );
}
//--------------------------------------------------------------------
// internal: creates the virtual menu from the pSVMenu

void SfxVirtualMenu::CreateFromSVMenu()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    // Merge Addon popup menus into the SV Menu
    Reference< com::sun::star::frame::XFrame > xFrame( pBindings->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface() );

    if ( pSVMenu->IsMenuBar() )
    {
        USHORT nPos = pSVMenu->GetItemPos( SID_MDIWINDOWLIST );
        if ( nPos != MENU_ITEM_NOTFOUND && xFrame.is() )
        {
            // Retrieve addon popup menus and add them to our menu bar
            Reference< com::sun::star::frame::XModel >      xModel;
            Reference< com::sun::star::frame::XController > xController( xFrame->getController(), UNO_QUERY );
            if ( xController.is() )
                xModel = Reference< com::sun::star::frame::XModel >( xController->getModel(), UNO_QUERY );
            framework::AddonMenuManager::MergeAddonPopupMenus( xFrame, xModel, nPos, (MenuBar *)pSVMenu );
        }

        // Merge the Add-Ons help menu items into the Office help menu
        if ( xFrame.is() )
            framework::AddonMenuManager::MergeAddonHelpMenu( xFrame, (MenuBar *)pSVMenu );

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
    SFX_APP();
    const int bOleServer = FALSE;
    const int bMac = FALSE;
    SvtMenuOptions aOptions;
    aOptions.AddListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    // iterate through the items
    pBindings->ENTERREGISTRATIONS(); ++nLocks;
    pImageControl = new SfxMenuImageControl_Impl( SID_IMAGE_ORIENTATION, *pBindings, this );

    USHORT nSVPos = 0;
    for ( USHORT nPos=0; nPos<nCount; ++nPos, ++nSVPos )
    {
        USHORT nSlotId = pSVMenu->GetItemId(nSVPos);
        PopupMenu* pPopup = pSVMenu->GetPopupMenu(nSlotId);
        if( pPopup && nSlotId >= SID_OBJECTMENU0 && nSlotId <= SID_OBJECTMENU_LAST )
        {
            // artefact in XML menuconfig: every entry in root menu must have a popup!
            pSVMenu->SetPopupMenu( nSlotId, NULL );
            DELETEZ( pPopup );
        }

        const String sItemText = pSVMenu->GetItemText(nSlotId);

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
                rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count() );
                (pItems+nPos)->Bind( 0, nSlotId, sItemText, *pBindings);
                pMnuCtrl->Bind( this, nSlotId, sItemText, *pBindings);

                if (  Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
                {
                    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                    Image aImage = GetImage( xFrame, aSlotURL, FALSE );
                    pSVMenu->SetItemImage( nSlotId, aImage );
                }
            }
            else
            {
                pSVMenu->SetHelpId( nSlotId, 0L );
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
                case MENUITEM_STRING:
                case MENUITEM_STRINGIMAGE:
                {
                    SfxMenuControl *pMnuCtrl=0;
                    String aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    if ( aCmd.CompareToAscii("slot:", 5) == 0 )
                    {
                        SfxMacroConfig* pCfg = SFX_APP()->GetMacroConfig();
                        if ( pCfg->IsMacroSlot( nSlotId ) )
                        {
                            if ( pCfg->GetMacroInfo( nSlotId ) )
                            {
                                pCfg->RegisterSlotId( nSlotId );
                                pSVMenu->SetItemCommand( nSlotId, String() );
                                aCmd.Erase();
                            }
                            else
                            {
                                pSVMenu->SetItemCommand( nSlotId, String::CreateFromAscii("macro:///macro.not.founc") );
                            }
                        }
                    }

                    if ( aCmd.Len() && (( nSlotId < SID_SFX_START ) || ( nSlotId > SHRT_MAX )) )
                    {
                        // try to create control via comand name
                        pMnuCtrl = SfxMenuControl::CreateControl( aCmd, nSlotId, *pSVMenu, sItemText, *pBindings, this );
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count());
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
                            rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count());
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
                            rtl::OUString aImageId;

                            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                                (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nSlotId );

                            if ( pMenuAttributes )
                                aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                            aImage = RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE );
                        }
                        else
                        {
                            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                            aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                            aImage = GetImage( xFrame, aSlotURL, FALSE );
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

                case MENUITEM_IMAGE:
                    //! not implemented
                    break;

                case MENUITEM_SEPARATOR:
                    //! not implemented
                    break;
                default:
                    break; // DONTKNOW and STRINGIMAGE not handled.
            }
        }
    }
    pBindings->LEAVEREGISTRATIONS(); --nLocks;
}

//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Highlight, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    // own StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // AutoDeactivate is not necessary anymore
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();
    }

    return TRUE;
}

IMPL_LINK( SfxVirtualMenu, SettingsChanged, void*, EMPTYARG )
{
    USHORT nItemCount = pSVMenu->GetItemCount();
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    BOOL bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( !bIsAddonPopupMenu )
    {
        for ( USHORT nSVPos=0; nSVPos<nItemCount; ++nSVPos )
        {
            USHORT          nSlotId = pSVMenu->GetItemId( nSVPos );
            MenuItemType    nType   = pSVMenu->GetItemType( nSVPos );
            if ( nType == MENUITEM_STRING && bIcons )
            {
                if ( framework::AddonMenuManager::IsAddonMenuId( nSlotId ))
                {
                    // Special code for Add-On menu items. They can appear inside the help menu.
                    rtl::OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    rtl::OUString aImageId;

                    ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                        (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nSlotId );

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE ));
                }
                else
                {
                    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                    pSVMenu->SetItemImage( nSlotId, GetImage( xFrame, aSlotURL, FALSE ));
                }
            }
            else if( nType == MENUITEM_STRINGIMAGE && !bIcons )
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

    return 0;
}

//--------------------------------------------------------------------

void SfxVirtualMenu::UpdateImages()
{
    BOOL bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    if ( bIcons )
    {
        USHORT          nItemCount          = pSVMenu->GetItemCount();
        SfxViewFrame *  pViewFrame          = pBindings->GetDispatcher()->GetFrame();
        Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame().GetFrameInterface() );

        for ( USHORT nSVPos=0; nSVPos < nItemCount; ++nSVPos )
        {
            USHORT nSlotId = pSVMenu->GetItemId( nSVPos );
            if ( pSVMenu->GetItemType( nSVPos ) == MENUITEM_STRINGIMAGE )
            {
                if ( framework::AddonMenuManager::IsAddonMenuId( nSlotId ))
                {
                    // Special code for Add-On menu items. They can appear inside the help menu.
                    rtl::OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    rtl::OUString aImageId;

                    ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                        (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nSlotId );

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE ));
                }
                else
                {
                    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                    pSVMenu->SetItemImage( nSlotId, GetImage( xFrame, aSlotURL, FALSE ));
                }
            }
        }

        if ( pImageControl )
            pImageControl->Update();
    }
}

//--------------------------------------------------------------------

void SfxVirtualMenu::UpdateImages( Menu* pMenu )
{
    if ( !pMenu )
        return;

    framework::AddonsOptions    aAddonOptions;

    BOOL bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    if ( bIcons )
    {
        USHORT          nItemCount          = pMenu->GetItemCount();
        Reference<com::sun::star::frame::XFrame> aXFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

        for ( USHORT nPos=0; nPos < nItemCount; ++nPos )
        {
            USHORT nSlotId = pMenu->GetItemId( nPos );
            PopupMenu* pPopup = pMenu->GetPopupMenu( nSlotId );
            if ( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
            {
                rtl::OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nSlotId );

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                pMenu->SetItemImage( nSlotId, RetrieveAddOnImage( aXFrame, aImageId, pMenu->GetItemCommand( nSlotId ), FALSE ));
            }

            if ( pPopup )
                UpdateImages( pPopup );
        }

        if ( pImageControl )
            pImageControl->Update();
    }
}

//--------------------------------------------------------------------

void SfxVirtualMenu::RemoveMenuImages( Menu* pMenu )
{
    if ( !pMenu )
        return;

    USHORT nItemCount = pMenu->GetItemCount();
    for ( USHORT nPos=0; nPos < nItemCount; ++nPos )
    {
        USHORT nSlotId = pMenu->GetItemId( nPos );
        PopupMenu* pPopup = pMenu->GetPopupMenu( nSlotId );
        if ( pMenu->GetItemType( nPos ) == MENUITEM_STRINGIMAGE )
            pMenu->SetItemImage( nSlotId, Image() );
        if ( pPopup )
            RemoveMenuImages( pPopup );
    }
}

//--------------------------------------------------------------------

bool SfxVirtualMenu::Bind_Impl( Menu *pMenu )
{
    // Search , as SV with 'USHORT nSID = pSVMenu->GetCurItemId();' always
    // returns 0. It is like this, since the Event-Forwarding has nothing to do
    // with the Parent-Menus CurItem.
    sal_uInt32 nAddonsPopupPrefixLen = ADDONSPOPUPMENU_URL_PREFIX.getLength();

    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
    {
        // found the Sub-Menu in question?
        bool bFound = false;
        USHORT nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            // Only a binded Menu-Controller as already an Id!
            if ( !rCtrl.GetId() )
            {
                bIsAddonPopupMenu = FALSE;
                DBG_ASSERT( !pSubMenu, "Popup already exists!");

                // Check if the popup is an Add-On popup menu
                // Either the popup menu has a special ID  or a special command URL prefix!
                rtl::OUString aCommand = pSVMenu->GetItemCommand( nSID );
                if ( ( nSID == SID_ADDONS ) ||
                     ( nSID == SID_ADDONHELP ) ||
                     (( (sal_uInt32)aCommand.getLength() > nAddonsPopupPrefixLen ) &&
                      ( aCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 )) )
                    bIsAddonPopupMenu = TRUE;

                // Create VirtualMenu for Sub-Menu
                BOOL bRes = bResCtor;
                pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, FALSE, *pBindings, bOLE, bRes, bIsAddonPopupMenu );

                DBG_OUTF( ("New VirtualMenu %lx created", pSubMenu) );

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
            return TRUE;
    }

    // Not found in this submenu
    return FALSE;
}

void SfxVirtualMenu::BindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    USHORT nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl& rCtrl = pItems[nPos];
        if ( rCtrl.IsBindable_Impl() && !rCtrl.GetPopupMenu() )
            rCtrl.ReBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        USHORT nSlotId = pCtrl->GetId();
        if ( !pSVMenu->GetItemCommand(nSlotId).Len() )
            pCtrl->ReBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = FALSE;
}

void SfxVirtualMenu::UnbindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    USHORT nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( rCtrl.IsBound() )
            rCtrl.UnBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        if ( pCtrl->IsBound() )
            // UnoController is not binded!
            pCtrl->UnBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = TRUE;
}


//--------------------------------------------------------------------
void SfxVirtualMenu::InsertAddOnsMenuItem( Menu* pMenu )
{
    // Create special popup menu that is filled with the 3rd party components popup menu items
    Reference<com::sun::star::lang::XMultiServiceFactory> aXMultiServiceFactory(::comphelper::getProcessServiceFactory());
    ::framework::MenuConfiguration aConf( aXMultiServiceFactory );
    Reference<com::sun::star::frame::XFrame> xFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

    PopupMenu* pAddonMenu = NULL;
    try
    {
        pAddonMenu = framework::AddonMenuManager::CreateAddonMenu( xFrame );
    }
    catch ( ::com::sun::star::lang::WrappedTargetException )
    {
    }

    // Create menu item at the end of the tools popup menu for the addons popup menu
    if ( pAddonMenu && pAddonMenu->GetItemCount() > 0 )
    {
        USHORT nItemCount = pMenu->GetItemCount();
        String aAddonsTitle( SfxResId( STR_MENU_ADDONS ));
        if ( nItemCount > 0 && pMenu->GetItemType( nItemCount-1 ) != MENUITEM_SEPARATOR )
            pMenu->InsertSeparator();
        pMenu->InsertItem( SID_ADDONS, aAddonsTitle );
        pMenu->SetPopupMenu( SID_ADDONS, pAddonMenu );

        if ( Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
        {
               rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
               aSlotURL += rtl::OUString::valueOf( sal_Int32( SID_ADDONS ));
         pMenu->SetItemImage( SID_ADDONS, GetImage( xFrame, aSlotURL, FALSE ));
        }
    }
    else
        delete pAddonMenu;
}

//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Activate, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_OUTF( ("SfxVirtualMenu %lx activated %lx, own %lx", this, pMenu, pSVMenu));

    // MI: for what was it still good for?
    // MBA: seemes to be an old QAP-Hack( checked-in in rev.1.41  ! )

    if ( pMenu )
    {
        sal_Bool bDontHide = SvtMenuOptions().IsEntryHidingEnabled();
        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );
    }

    // Own StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // Prevent Double Activate
        if ( bIsActive )
            return TRUE;

        // ggf. Pick-Menu erzeugen
        if ( pParent && pSVMenu == pParent->pPickMenu )
        {
            SfxPickList::Get()->CreateMenuEntries( pParent->pPickMenu );
        }
        else
            pPickMenu = pSVMenu->GetPopupMenu(SID_PICKLIST);

        if ( pParent && pSVMenu == pParent->pWindowMenu )
        {
            // update window list
            ::std::vector< ::rtl::OUString > aNewWindowListVector;
            Reference< XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                            DEFINE_CONST_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

            USHORT  nActiveItemId = 0;
            USHORT  nItemId = START_ITEMID_WINDOWLIST;

            if ( xDesktop.is() )
            {
                Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
                Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
                Reference< XIndexAccess > xList ( xTasksSupplier->getFrames(), UNO_QUERY );
                sal_Int32 nFrameCount = xList->getCount();
                for( sal_Int32 i=0; i<nFrameCount; ++i )
                {
                    Reference< XFrame > xFrame;
                    Any aVal = xList->getByIndex(i);
                    if (!(aVal>>=xFrame) || !xFrame.is() )
                        continue;

                    if ( xFrame == xCurrentFrame )
                        nActiveItemId = nItemId;

                    Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                    if ( pWin && pWin->IsVisible() )
                    {
                        aNewWindowListVector.push_back( pWin->GetText() );
                        ++nItemId;
                    }
                }
            }

            int nItemCount       = pMenu->GetItemCount();

            if ( nItemCount > 0 )
            {
                // remove all old window list entries from menu
                sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
                for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                    pMenu->RemoveItem( n );

                if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                    pMenu->RemoveItem( pMenu->GetItemCount()-1 );
            }

            if ( aNewWindowListVector.size() > 0 )
            {
                // append new window list entries to menu
                pMenu->InsertSeparator();
                nItemId = START_ITEMID_WINDOWLIST;
                for ( sal_uInt32 i = 0; i < aNewWindowListVector.size(); i++ )
                {
                    pMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MIB_RADIOCHECK );
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

        //InvalidateKeyCodes();
        pBindings->GetDispatcher_Impl()->Flush();
        for ( USHORT nPos = 0; nPos < nCount; ++nPos )
        {
            USHORT nSlotId = (pItems+nPos)->GetId();
            if ( nSlotId && nSlotId > END_ITEMID_WINDOWLIST )
                pBindings->Update(nSlotId);
        }

        pBindings->Update( SID_IMAGE_ORIENTATION );

        // Supress the Status updates until Deactivate
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = TRUE;

        if ( pAutoDeactivate ) // QAP-Hack
            pAutoDeactivate->Start();

        return TRUE;
    }
    else
    {
        // Find the VirtualMenu for the SubMenu and if possible, bind a
        // VirtualMenu
        bool bRet = Bind_Impl( pMenu );
#ifdef DBG_UTIL
        if ( !bRet)
            DBG_WARNING( "W1: Virtual menu could not be created!" );
#endif
        return bRet;
    }
}

//--------------------------------------------------------------------

IMPL_LINK( SfxVirtualMenu, Deactivate, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_OUTF( ("SfxVirtualMenu %lx deactivated %lx, own %lx", this, pMenu, pSVMenu) );
    if ( bIsActive && ( 0 == pMenu || pMenu == pSVMenu ) )
    {
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();

        // All controllers can be unbinded all the way up to the Menubar,
        // when the menu is disabled (= closed)
        if ( pParent )
            UnbindControllers();
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = FALSE;
    }
    return TRUE;
}
//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Select, Menu *, pMenu )
{
    USHORT nSlotId = (USHORT) pMenu->GetCurItemId();
    DBG_OUTF( ("SfxVirtualMenu %lx selected %u from %lx", this, nSlotId, pMenu) );

    if ( nSlotId >= START_ITEMID_WINDOWLIST && nSlotId <= END_ITEMID_WINDOWLIST )
    {
        // window list menu item selected
        Reference< XFramesSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                        DEFINE_CONST_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
        if ( xDesktop.is() )
        {
            USHORT nTaskId = START_ITEMID_WINDOWLIST;
            Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
            sal_Int32 nFrameCount = xList->getCount();
            for ( sal_Int32 i=0; i<nFrameCount; ++i )
            {
                Any aItem = xList->getByIndex(i);
                Reference< XFrame > xFrame;
                if (( aItem >>= xFrame ) && xFrame.is() && nTaskId == nSlotId )
                {
                    Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                    pWin->GrabFocus();
                    pWin->ToTop( TOTOP_RESTOREWHENMIN );
                    break;
                }

                nTaskId++;
            }
        }

        return TRUE;
    }
    else if ( nSlotId >= START_ITEMID_PICKLIST && nSlotId <= END_ITEMID_PICKLIST )
    {
        SfxPickList::Get()->ExecuteMenuEntry( nSlotId );
        return sal_True;
    }

    if ( pMenu->GetItemCommand( nSlotId ).Len() )
        pBindings->ExecuteCommand_Impl( pMenu->GetItemCommand( nSlotId ) );
    else
        pBindings->Execute( nSlotId );

    return TRUE;
}

//--------------------------------------------------------------------

// returns the associated StarView-menu

Menu* SfxVirtualMenu::GetSVMenu() const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    return pSVMenu;
}

//--------------------------------------------------------------------

// return the position of the specified item

USHORT SfxVirtualMenu::GetItemPos( USHORT nItemId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
        if ( (pItems+nPos)->GetId() == nItemId )
            return nPos;
    return MENU_ITEM_NOTFOUND;
}

//--------------------------------------------------------------------

// returns the popup-menu assigned to the item or 0 if none

SfxVirtualMenu* SfxVirtualMenu::GetPopupMenu( USHORT nItemId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nItemId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetPopupMenu();
    return 0;
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemText( USHORT nSlotId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nSlotId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetTitle();
    return String();
}
//--------------------------------------------------------------------

// set the checkmark of the specified item

void SfxVirtualMenu::CheckItem( USHORT nItemId, BOOL bCheck )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->CheckItem( nItemId, bCheck );
}
//--------------------------------------------------------------------

// set the enabled-state of the specified item

void SfxVirtualMenu::EnableItem( USHORT nItemId, BOOL bEnable )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->EnableItem( nItemId, bEnable );
}
//--------------------------------------------------------------------

// set the text of the specified item

void SfxVirtualMenu::SetItemText( USHORT nItemId, const String& rText )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->SetItemText( nItemId, rText );
}

//--------------------------------------------------------------------

//

void SfxVirtualMenu::SetPopupMenu( USHORT nItemId, PopupMenu *pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        GetSVMenu()->SetPopupMenu( nItemId, pMenu );
    for ( USHORT n = 0; n < nCount; ++n )
    {
        SfxVirtualMenu *pSubMenu = (pItems+n)->GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->SetPopupMenu( nItemId, pMenu );
    }
}

//--------------------------------------------------------------------

// Forces the initialization, which is otherwise only happens in Activate

void SfxVirtualMenu::InitPopup( USHORT nPos, BOOL /*bOLE*/ )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "No popup exist here!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        // Generate VirtualMenu for Sub-Menu
        BOOL bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, FALSE, *pBindings, bOLE, bRes);

        DBG_OUTF( ("New VirtualMenu %lx created", pSubMenu) );

        rCtrl.Bind( this, nSID, *pSubMenu, pSVMenu->GetItemText(nSID), *pBindings );
    }
}

void SfxVirtualMenu::InitializeHelp()
{
    for ( USHORT nPos = 0; nPos<pSVMenu->GetItemCount(); ++nPos )
    {
        USHORT nSlotId = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( nSlotId && !rCtrl.GetId() )
        {
            InitPopup( nPos, TRUE );
        }

        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->InitializeHelp();
    }

    bHelpInitialized = TRUE;
}

typedef ULONG (__LOADONCALLAPI *HelpIdFunc) ( const String& );

void SfxVirtualMenu::SetHelpIds( ResMgr *pRes )
{
    pResMgr = pRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
