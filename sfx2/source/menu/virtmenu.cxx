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
#include "sfx2/sfxresid.hxx"
#include "menu.hrc"
#include "sfx2/imagemgr.hxx"
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
    sal_Bool                bIsMirrored;

protected:
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
public:
                        SfxMenuImageControl_Impl( sal_uInt16 nSlotId, SfxBindings& rBindings, SfxVirtualMenu* pVMenu )
                            : SfxControllerItem( nSlotId, rBindings )
                            , pMenu( pVMenu )
                            , lRotation( 0 )
                            , bIsMirrored( sal_False )
                        {}
    void                Update();
};

void SfxMenuImageControl_Impl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState /*eState*/, const SfxPoolItem* pState )
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
    for (sal_uInt16 nPos = 0; nPos<pSVMenu->GetItemCount(); nPos++)
    {
        sal_uInt16 nslotId = pSVMenu->GetItemId( nPos );
        const SfxSlot* pSlot = pPool->GetSlot( nslotId );
        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEROTATION ) )
        {
            pSVMenu->SetItemImageMirrorMode( nslotId, sal_False );
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
                                 bool bBigImage
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

/*  Diese Hilfsfunktion pr"uft, ob eine Slot-Id im aktuellen Applikations-
    Status sichtbar ist oder nicht. Dabei bezieht sich der Applikations-Status
    darauf, ob die Applikation OLE-Server ist oder nicht.
*/

sal_Bool IsItemHidden_Impl( sal_uInt16 nItemId, int bOleServer, int bMac )
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

SfxVirtualMenu::SfxVirtualMenu( sal_uInt16 nOwnId,
                SfxVirtualMenu* pOwnParent, Menu& rMenu, sal_Bool bWithHelp,
                SfxBindings &rBindings, sal_Bool bOLEServer, sal_Bool bRes, sal_Bool bIsAddonMenu ):
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
    bIsActive = sal_False;
    bControllersUnBound = sal_False;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = sal_False;
}

//--------------------------------------------------------------------

// creates a virtual menu from a StarView MenuBar or PopupMenu

SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, sal_Bool bWithHelp,
                    SfxBindings &rBindings, sal_Bool bOLEServer, sal_Bool bRes, sal_Bool bIsAddonMenu ):
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
    bIsActive = sal_False;
    bControllersUnBound = sal_False;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = sal_False;
}

//--------------------------------------------------------------------

/*  Der Destruktor der Klasse SfxVirtualMenu gib die gebundenen Items frei
    und klinkt das zugeh"orige StarView-PopupMenu aus seinem Parent aus.
    Falls es sich um das Pickmenu oder das MDI-Menu handelt, wird es
    dort abgemeldet.
*/

SfxVirtualMenu::~SfxVirtualMenu()
{
    DBG_MEMTEST();
    DBG_DTOR(SfxVirtualMenu, 0);

    DELETEZ( pImageControl );
    SvtMenuOptions().RemoveListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    if ( bIsActive )
    {
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = sal_False;
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

    // Alle Menues, die von SV erzeugt wurden, werden auch dort wieder
    // gel"oscht (also die beim Laden aus der Resource erzeugten).
    // Das Top-Level-Menu wird nie von SV gel"oscht, da die Allocierung
    // im SFX erfolgt
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
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
    SfxSlotPool* pSlotPool = pViewFrame->GetObjectShell()->GetModule()->GetSlotPool();
    Reference< com::sun::star::frame::XFrame > xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( pSVMenu->IsMenuBar() )
    {
        sal_uInt16 nPos = pSVMenu->GetItemPos( SID_MDIWINDOWLIST );
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

    // Achtung: nur zu diesem Zeitpunkt ist garantiert, da\s nCount und
    // der ItemCount des SV-Menues "ubereinstimmen; sp"ater kann das SvMenue
    // auch mehr Eintr"age haben (Pickliste!)
    if (nCount)
        pItems = new SfxMenuControl[nCount];

    // remember some values
    SFX_APP();
    const int bOleServer = sal_False;
    const int bMac = sal_False;
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

        const String sItemText = pSVMenu->GetItemText(nSlotId);

        if ( pPopup )
        {

            SfxMenuControl *pMnuCtrl =
                SfxMenuControl::CreateControl(nSlotId, *pPopup, *pBindings);

            if ( pMnuCtrl )
            {
                // Das Popup war offensichtlich kein "echtes"; solche werden
                // niemals aus der Resource geladen und m"ussen daher explizit
                // gel"oscht werden
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
                    Image aImage = GetImage( xFrame, aSlotURL, false );
                    pSVMenu->SetItemImage( nSlotId, aImage );
                }
            }
            else
            {
                pMnuCtrl = pItems+nPos;

                // normalerweise jetzt erst im Activate-Handler
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

                            aImage = RetrieveAddOnImage( xFrame, aImageId, aCmd, false );
                        }
                        else
                        {
                            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                            aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
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

    // eigenes StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // AutoDeactivate ist jetzt nicht mehr n"otig
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();
    }

    return sal_True;
}

IMPL_LINK( SfxVirtualMenu, SettingsChanged, void*, EMPTYARG )
{
    sal_uInt16 nItemCount = pSVMenu->GetItemCount();
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    sal_Bool bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( !bIsAddonPopupMenu )
    {
        for ( sal_uInt16 nSVPos=0; nSVPos<nItemCount; ++nSVPos )
        {
            sal_uInt16          nSlotId = pSVMenu->GetItemId( nSVPos );
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

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, false ));
                }
                else
                {
                    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                    pSVMenu->SetItemImage( nSlotId, GetImage( xFrame, aSlotURL, false ));
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
    sal_Bool bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    if ( bIcons )
    {
        sal_uInt16          nItemCount          = pSVMenu->GetItemCount();
        SfxViewFrame *  pViewFrame          = pBindings->GetDispatcher()->GetFrame();
        Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame().GetFrameInterface() );

        for ( sal_uInt16 nSVPos=0; nSVPos < nItemCount; ++nSVPos )
        {
            sal_uInt16 nSlotId = pSVMenu->GetItemId( nSVPos );
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

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, false ));
                }
                else
                {
                    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlotId ));
                    pSVMenu->SetItemImage( nSlotId, GetImage( xFrame, aSlotURL, false ));
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

    sal_Bool bIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    if ( bIcons )
    {
        sal_uInt16          nItemCount          = pMenu->GetItemCount();
        Reference<com::sun::star::frame::XFrame> aXFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

        for ( sal_uInt16 nPos=0; nPos < nItemCount; ++nPos )
        {
            sal_uInt16 nSlotId = pMenu->GetItemId( nPos );
            PopupMenu* pPopup = pMenu->GetPopupMenu( nSlotId );
            if ( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
            {
                rtl::OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nSlotId );

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

//--------------------------------------------------------------------

void SfxVirtualMenu::RemoveMenuImages( Menu* pMenu )
{
    if ( !pMenu )
        return;

    sal_uInt16 nItemCount = pMenu->GetItemCount();
    for ( sal_uInt16 nPos=0; nPos < nItemCount; ++nPos )
    {
        sal_uInt16 nSlotId = pMenu->GetItemId( nPos );
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
    // Selber suchen, da SV mit 'sal_uInt16 nSID = pSVMenu->GetCurItemId();' immer
    // 0 liefert. Das ist so, weil die Event-Weiterleitung lt. TH nichts mit
    // CurItem des Parent-Menus zu tun hat.
    sal_uInt32 nAddonsPopupPrefixLen = ADDONSPOPUPMENU_URL_PREFIX.getLength();

    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
    {
        // angesprochenes Sub-Menu gefunden?
        bool bFound = false;
        sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            // Nur ein gebundener Menu-Controller hat schon seine Id!
            if ( !rCtrl.GetId() )
            {
                bIsAddonPopupMenu = sal_False;
                DBG_ASSERT( !pSubMenu, "Popup schon vorhanden!");

                // Check if the popup is an Add-On popup menu
                // Either the popup menu has a special ID  or a special command URL prefix!
                rtl::OUString aCommand = pSVMenu->GetItemCommand( nSID );
                if ( ( nSID == SID_ADDONS ) ||
                     ( nSID == SID_ADDONHELP ) ||
                     (( (sal_uInt32)aCommand.getLength() > nAddonsPopupPrefixLen ) &&
                      ( aCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 )) )
                    bIsAddonPopupMenu = sal_True;

                // VirtualMenu f"ur Sub-Menu erzeugen
                sal_Bool bRes = bResCtor;
                pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, sal_False, *pBindings, bOLE, bRes, bIsAddonPopupMenu );

                DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

                rCtrl.Bind( this, nSID, *pSubMenu, pSVMenu->GetItemText(nSID), *pBindings );

                // Activate weiterleiten
                pSubMenu->Bind_Impl( pMenu );
                pSubMenu->Activate( pMenu );
            }
        }

        // rekursiv weitersuchen (SV Activate nur am Menu selbst und Top-Menu)
        if ( !bFound && pSubMenu )
            bFound = pSubMenu->Bind_Impl( pMenu );

        // gefunden, dann abbrechen
        if ( bFound )
            return true;
    }

    // nicht in diesem Untermenu gefunden
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
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        sal_uInt16 nSlotId = pCtrl->GetId();
        if ( !pSVMenu->GetItemCommand(nSlotId).Len() )
            pCtrl->ReBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = sal_False;
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
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        if ( pCtrl->IsBound() )
            // UnoController sind nicht gebunden!
            pCtrl->UnBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = sal_True;
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
        sal_uInt16 nItemCount = pMenu->GetItemCount();
        String aAddonsTitle( SfxResId( STR_MENU_ADDONS ));
        if ( nItemCount > 0 && pMenu->GetItemType( nItemCount-1 ) != MENUITEM_SEPARATOR )
            pMenu->InsertSeparator();
        pMenu->InsertItem( SID_ADDONS, aAddonsTitle );
        pMenu->SetPopupMenu( SID_ADDONS, pAddonMenu );

        if ( Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
        {
               rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
               aSlotURL += rtl::OUString::valueOf( sal_Int32( SID_ADDONS ));
         pMenu->SetItemImage( SID_ADDONS, GetImage( xFrame, aSlotURL, false ));
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

    // MI: wozu war der noch gut?
    // MBA: scheint ein alter QAP-Hack gewesen zu sein ( in rev.1.41 eingecheckt ! )

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

    // eigenes StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // doppelt-Activate verhindern
        if ( bIsActive )
            return sal_True;

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

            sal_uInt16  nActiveItemId = 0;
            sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

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

        // f"ur konstistenten Status sorgen
        if ( bControllersUnBound )
            BindControllers();

        //InvalidateKeyCodes();
        pBindings->GetDispatcher_Impl()->Flush();
        for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
        {
            sal_uInt16 nSlotId = (pItems+nPos)->GetId();
            if ( nSlotId && nSlotId > END_ITEMID_WINDOWLIST )
                pBindings->Update(nSlotId);
        }

        pBindings->Update( SID_IMAGE_ORIENTATION );

        // bis zum Deactivate die Statusupdates unterdr"ucken
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = sal_True;

        if ( pAutoDeactivate ) // QAP-Hack
            pAutoDeactivate->Start();

        // erledigt
        return sal_True;
    }
    else
    {
        // VirtualMenu fuer SubMenu finden und ggf. an VirtualMenu binden
        bool bRet = Bind_Impl( pMenu );
#ifdef DBG_UTIL
        if ( !bRet)
            DBG_WARNING( "W1: Virtual menu konnte nicht erzeugt werden!" );
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

        // Bis auf die Menubar k"onnen alle Controller unbinded werden, wenn
        // das Menue deaktiviert ( = zugeklappt ) wird
        if ( pParent )
            UnbindControllers();
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = sal_False;
    }
    return sal_True;
}
//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Select, Menu *, pMenu )
{
    sal_uInt16 nSlotId = (sal_uInt16) pMenu->GetCurItemId();
    DBG_OUTF( ("SfxVirtualMenu %lx selected %u from %lx", this, nSlotId, pMenu) );

    if ( nSlotId >= START_ITEMID_WINDOWLIST && nSlotId <= END_ITEMID_WINDOWLIST )
    {
        // window list menu item selected
        Reference< XFramesSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                        DEFINE_CONST_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
        if ( xDesktop.is() )
        {
            sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
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

        return sal_True;
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

    return sal_True;
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

sal_uInt16 SfxVirtualMenu::GetItemPos( sal_uInt16 nItemId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
        if ( (pItems+nPos)->GetId() == nItemId )
            return nPos;
    return MENU_ITEM_NOTFOUND;
}

//--------------------------------------------------------------------

// returns the popup-menu assigned to the item or 0 if none

SfxVirtualMenu* SfxVirtualMenu::GetPopupMenu( sal_uInt16 nItemId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    sal_uInt16 nPos = GetItemPos(nItemId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetPopupMenu();
    return 0;
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemText( sal_uInt16 nSlotId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    sal_uInt16 nPos = GetItemPos(nSlotId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetTitle();
    return String();
}
//--------------------------------------------------------------------

// set the checkmark of the specified item

void SfxVirtualMenu::CheckItem( sal_uInt16 nItemId, sal_Bool bCheck )
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

void SfxVirtualMenu::EnableItem( sal_uInt16 nItemId, sal_Bool bEnable )
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

void SfxVirtualMenu::SetItemText( sal_uInt16 nItemId, const String& rText )
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

void SfxVirtualMenu::SetPopupMenu( sal_uInt16 nItemId, PopupMenu *pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        GetSVMenu()->SetPopupMenu( nItemId, pMenu );
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SfxVirtualMenu *pSubMenu = (pItems+n)->GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->SetPopupMenu( nItemId, pMenu );
    }
}

//--------------------------------------------------------------------

// Erzwingt die Initialisierung, die sonst nur im Activate kommt

void SfxVirtualMenu::InitPopup( sal_uInt16 nPos, sal_Bool /*bOLE*/ )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "Hier gibt es kein Popup!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        // VirtualMenu f"ur Sub-Menu erzeugen
        sal_Bool bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, sal_False, *pBindings, bOLE, bRes);

        DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

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
            InitPopup( nPos, sal_True );
        }

        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->InitializeHelp();
    }

    bHelpInitialized = sal_True;
}

typedef sal_uIntPtr (__LOADONCALLAPI *HelpIdFunc) ( const String& );

void SfxVirtualMenu::SetHelpIds( ResMgr *pRes )
{
    pResMgr = pRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
