/*************************************************************************
 *
 *  $RCSfile: virtmenu.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 09:46:19 $
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

#include <sot/factory.hxx>
#include <vcl/system.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/imageitm.hxx>

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#pragma hdrstop

#include "virtmenu.hxx"
#include "msgpool.hxx"
#include "statcach.hxx"
#include "msg.hxx"
#include "idpool.hxx"
#include "mnuitem.hxx"
#include "mnumgr.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "app.hxx"
#include "accmgr.hxx"
#include "sfxtypes.hxx"
#include "arrdecl.hxx"
#include "sfx.hrc"
#include "ipfrm.hxx"
#include "ipenv.hxx"
#include "appdata.hxx"
#include "viewsh.hxx"
#include "imgmgr.hxx"
#include "sfxpicklist.hxx"
#include "macrconf.hxx"
#include "sfxresid.hxx"
#include "menu.hrc"
#include "imagemgr.hxx"

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <framework/addonsoptions.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONMENUS_HXX_
#include <framework/addonmenu.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <framework/menuconfiguration.hxx>
#endif

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

//=========================================================================

DBG_NAME(SfxVirtualMenu);

static long nAutoDeactivateTimeout_Impl = -1;

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

void SfxMenuImageControl_Impl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
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
        USHORT nId = pSVMenu->GetItemId( nPos );
        const SfxSlot* pSlot = pPool->GetSlot( nId );
        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEROTATION ) )
        {
            pSVMenu->SetItemImageMirrorMode( nId, FALSE );
            pSVMenu->SetItemImageAngle( nId, lRotation );
        }

        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEREFLECTION ) )
            pSVMenu->SetItemImageMirrorMode( nId, bIsMirrored );
    }
}

//=========================================================================

static Image RetrieveAddOnImage( Reference< com::sun::star::frame::XFrame >& rFrame,
                                 const rtl::OUString& aImageId,
                                 const rtl::OUString& aURL,
                                 BOOL bBigImage,
                                 BOOL bHiContrast )
{
    Image aImage;

    if ( aImageId.getLength() > 0 )
    {
        aImage = GetImage( rFrame, aImageId, bBigImage, bHiContrast );
        if ( !!aImage )
            return aImage;
    }

    aImage = GetImage( rFrame, aURL, bBigImage, bHiContrast );
    if ( !aImage )
        aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage, bHiContrast );

    return aImage;
}

//=========================================================================

/*  Diese Hilfsfunktion pr"uft, ob eine Slot-Id im aktuellen Applikations-
    Status sichtbar ist oder nicht. Dabei bezieht sich der Applikations-Status
    darauf, ob die Applikation OLE-Server ist oder nicht.
*/

BOOL IsItemHidden_Impl( USHORT nItemId, int bOleServer, int bMac )
{
    return ( bMac &&
             ( nItemId == SID_MINIMIZEWINS ) ) ||
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
    pWindowMenu = NULL;
    pPickMenu = NULL;
    pAddonsMenu = NULL;
    bIsActive = FALSE;
    bControllersUnBound = FALSE;
    pSVMenu->SetHighlightHdl( LINK(this, SfxVirtualMenu, Highlight) );
    pSVMenu->SetActivateHdl( LINK(this, SfxVirtualMenu, Activate) );
    pSVMenu->SetDeactivateHdl( LINK(this, SfxVirtualMenu, Deactivate) );
    pSVMenu->SetSelectHdl( LINK(this, SfxVirtualMenu, Select) );

    // #107258# accelerator keys are needed for accessibility
    if ( bOLE )
        InvalidateKeyCodes();

    if ( !pResMgr && pParent )
        pResMgr = pParent->pResMgr;
}

//--------------------------------------------------------------------

SfxVirtualMenu::SfxVirtualMenu( USHORT nOwnId,
                SfxVirtualMenu* pOwnParent, Menu& rMenu, BOOL bWithHelp,
                SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes, BOOL bIsAddonMenu ):
    pItems(0),
    pBindings(&rBindings),
    pResMgr(0),
    pImageControl(0),
    nLocks(0), pAutoDeactivate(0), bHelpInitialized( bWithHelp ),
    bWasHighContrast( FALSE ),
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
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
}

//--------------------------------------------------------------------

// creates a virtual menu from a StarView MenuBar or PopupMenu

SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, BOOL bWithHelp,
                    SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes, BOOL bIsAddonMenu ):
    pItems(0),
    pBindings(&rBindings),
    pResMgr(0),
    pImageControl(0),
    nLocks(0), pAutoDeactivate(0),  bHelpInitialized( bWithHelp ),
    bWasHighContrast( FALSE ),
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
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
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
    SvtMenuOptions().RemoveListener( LINK( this, SfxVirtualMenu, SettingsChanged ) );

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
#ifdef MPW
        // der MPW-Compiler ruft sonst keine Dtoren!
        for ( USHORT n = 0; n < nCount; ++n )
            (pItems+n)->SfxMenuControl::~SfxMenuControl();
        delete (void*) pItems;
#else
        __DELETE(nCount) pItems;
#endif
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

BOOL SfxVirtualMenu::IsHiContrastMode() const
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Color aMenuColor = rSettings.GetMenuColor();
    if ( aMenuColor.IsDark() )
        return TRUE;
    else
        return FALSE;
}

//--------------------------------------------------------------------
// internal: creates the virtual menu from the pSVMenu

void SfxVirtualMenu::CreateFromSVMenu()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    // Merge Addon popup menus into the SV Menu
    Reference< com::sun::star::frame::XFrame > xFrame( pBindings->GetDispatcher()->GetFrame()->GetFrame()->GetFrameInterface() );

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

    // Achtung: nur zu diesem Zeitpunkt ist garantiert, da\s nCount und
    // der ItemCount des SV-Menues "ubereinstimmen; sp"ater kann das SvMenue
    // auch mehr Eintr"age haben (Pickliste!)
    if (nCount)
        pItems = new SfxMenuControl[nCount];

    // remember some values
    SfxApplication *pSfxApp = SFX_APP();
    const int bOleServer = FALSE;
    const int bMac = FALSE;
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    SfxModule* pModule = pViewFrame->GetObjectShell()->GetModule();
    SvtMenuOptions aOptions;
    aOptions.AddListener( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    // iterate through the items
    pBindings->ENTERREGISTRATIONS(); ++nLocks;
    pImageControl = new SfxMenuImageControl_Impl( SID_IMAGE_ORIENTATION, *pBindings, this );

    // Update high contrast state
    bWasHighContrast = IsHiContrastMode();

    USHORT nSVPos = 0;
    for ( USHORT nPos=0; nPos<nCount; ++nPos, ++nSVPos )
    {
        USHORT nId = pSVMenu->GetItemId(nSVPos);
        PopupMenu* pPopup = pSVMenu->GetPopupMenu(nId);
        if( pPopup && nId >= SID_OBJECTMENU0 && nId <= SID_OBJECTMENU_LAST )
        {
            // artefact in XML menuconfig: every entry in root menu must have a popup!
            pSVMenu->SetPopupMenu( nId, NULL );
            DELETEZ( pPopup );
        }

        if ( pPopup )
        {
            DBG_ASSERT( SfxMenuManager::IsPopupFunction(nId) ||
                        !SFX_SLOTPOOL().GetIdPool().IsLocked(nId),
                        "popupmenu with existing func-id" );
            SfxMenuControl *pMnuCtrl =
                SfxMenuControl::CreateControl(nId, *pPopup, *pBindings);

            if ( pMnuCtrl )
            {
                // Das Popup war offensichtlich kein "echtes"; solche werden
                // niemals aus der Resource geladen und m"ussen daher explizit
                // gel"oscht werden
                if ( pSVMenu->GetPopupMenu( nId ) == pPopup )
                    pSVMenu->SetPopupMenu( nId, NULL );
                DELETEZ( pPopup );

                SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count() );
                (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId),
                                    pSVMenu->GetHelpText(nId), *pBindings);
                pMnuCtrl->Bind( this, nId, pSVMenu->GetItemText(nId),
                                pSVMenu->GetHelpText(nId), *pBindings);

                if (  aOptions.IsMenuIconsEnabled() )
                    pSVMenu->SetItemImage( nId, pBindings->GetImageManager()->GetImage( nId, pModule, FALSE, bWasHighContrast ) );
            }
            else
            {
/*
                if ( nId >= SID_SFX_START && !SfxMenuManager::IsPopupFunction(nId) )
                {
                    // Echte Popups sollen keine SlotIds haben; leider sind
                    // da noch Altlasten mit herumzuschleppen ...
                    String aTitle = pSVMenu->GetItemText( nId );
                    pSVMenu->SetPopupMenu( nId, NULL );
                    USHORT nPos = pSVMenu->GetItemPos( nId );
                    pSVMenu->RemoveItem( nPos );
                    nId = 1;
                    while ( pSVMenu->GetItemPos(nId) != MENU_ITEM_NOTFOUND )
                        nId++;
                    pSVMenu->InsertItem( nId, aTitle, 0, nPos );
                    pSVMenu->SetPopupMenu( nId, pPopup );
                }
*/
                pSVMenu->SetHelpId( nId, 0L );
                pMnuCtrl = pItems+nPos;

                // normalerweise jetzt erst im Activate-Handler
                if ( bOLE )
                {
                    pMnuCtrl->Bind( this, nId,
                        *new SfxVirtualMenu(nId, this, *pPopup, bHelpInitialized, *pBindings, bOLE, bResCtor),
                        pSVMenu->GetItemText(nId), pSVMenu->GetHelpText(nId),
                        *pBindings );
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
                    String aCmd( pSVMenu->GetItemCommand( nId ) );
                    if ( aCmd.CompareToAscii("slot:", 5) == 0 )
                    {
                        SfxMacroConfig* pCfg = SFX_APP()->GetMacroConfig();
                        if ( pCfg->IsMacroSlot( nId ) )
                        {
                            if ( pCfg->GetMacroInfo( nId ) )
                            {
                                pCfg->RegisterSlotId( nId );
                                pSVMenu->SetItemCommand( nId, String() );
                                aCmd.Erase();
                            }
                            else
                            {
                                pSVMenu->SetItemCommand( nId, String::CreateFromAscii("macro:///macro.not.founc") );
                            }
                        }
                    }

                    if ( aCmd.Len() && (( nId < SID_SFX_START ) || ( nId > SHRT_MAX )) )
                    {
                        // try to create control via comand name
                        pMnuCtrl = SfxMenuControl::CreateControl( aCmd, nId, *pSVMenu, *pBindings, this );
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count());
                            (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId), pSVMenu->GetHelpText(nId), *pBindings);
                        }
                    }

                    if ( !pMnuCtrl )
                    {
                        // try to create control via Id
                        pMnuCtrl = SfxMenuControl::CreateControl(nId, *pSVMenu, *pBindings);
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count());
                            (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId), pSVMenu->GetHelpText(nId), *pBindings);
                        }
                        else
                            // take default control
                            pMnuCtrl = (pItems+nPos);

                        pMnuCtrl->Bind( this, nId, pSVMenu->GetItemText(nId), pSVMenu->GetHelpText(nId), *pBindings);
                    }

                    if ( aOptions.IsMenuIconsEnabled() )
                    {
                        // try to get image via URL
//                        Image aImage = SvFileInformationManager::GetImage( aCmd, FALSE, bWasHighContrast );
//                        if ( !aImage )
                            // try to get image via Id
                        Image aImage;
                        if ( bIsAddonPopupMenu || framework::AddonMenuManager::IsAddonMenuId( nId ))
                        {
                            rtl::OUString aCmd( pSVMenu->GetItemCommand( nId ) );
                            rtl::OUString aImageId;

                            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                                (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nId );

                            if ( pMenuAttributes )
                                aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                            aImage = RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE, bWasHighContrast );
                        }
                        else
                            aImage = pBindings->GetImageManager()->GetImage( nId, pModule, FALSE, bWasHighContrast );

                        if ( !!aImage )
                            pSVMenu->SetItemImage( nId, aImage );
                    }

                    if ( !IsItemHidden_Impl(nId, bOleServer, bMac) )
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
        USHORT nId = pMenu->GetCurItemId();
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();
    }

    return TRUE;
}

IMPL_LINK( SfxVirtualMenu, SettingsChanged, void*, pVoid )
{
    SvtMenuOptions aOptions;
    USHORT nCount = pSVMenu->GetItemCount();
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    SfxModule* pModule = pViewFrame->GetObjectShell()->GetModule();
    BOOL bIcons = aOptions.IsMenuIconsEnabled();
    BOOL bIsHiContrastMode = IsHiContrastMode();
    Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame()->GetFrameInterface() );

    if ( !bIsAddonPopupMenu )
    {
        for ( USHORT nSVPos=0; nSVPos<nCount; ++nSVPos )
        {
            USHORT          nId   = pSVMenu->GetItemId( nSVPos );
            MenuItemType    nType = pSVMenu->GetItemType( nSVPos );
            if ( nType == MENUITEM_STRING && bIcons )
            {
                if ( framework::AddonMenuManager::IsAddonMenuId( nId ))
                {
                    // Special code for Add-On menu items. They can appear inside the help menu.
                    rtl::OUString aCmd( pSVMenu->GetItemCommand( nId ) );
                    rtl::OUString aImageId;

                    ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                        (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nId );

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                    pSVMenu->SetItemImage( nId, RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE, bIsHiContrastMode ));
                }
                else
                    pSVMenu->SetItemImage( nId, pBindings->GetImageManager()->GetImage( nId, pModule, FALSE, bIsHiContrastMode ) );
            }
            else if( nType == MENUITEM_STRINGIMAGE && !bIcons )
            {
                pSVMenu->SetItemImage( nId, Image() );
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
    SvtMenuOptions aOptions;
    BOOL bIcons = aOptions.IsMenuIconsEnabled();

    if ( bIcons )
    {
        BOOL            bIsHiContrastMode   = IsHiContrastMode();
        USHORT          nCount              = pSVMenu->GetItemCount();
        SfxViewFrame *  pViewFrame          = pBindings->GetDispatcher()->GetFrame();
        SfxModule*      pModule             = pViewFrame->GetObjectShell()->GetModule();
        Reference<com::sun::star::frame::XFrame> xFrame( pViewFrame->GetFrame()->GetFrameInterface() );

        for ( USHORT nSVPos=0; nSVPos < nCount; ++nSVPos )
        {
            USHORT nId = pSVMenu->GetItemId( nSVPos );
            if ( pSVMenu->GetItemType( nSVPos ) == MENUITEM_STRINGIMAGE )
            {
                if ( framework::AddonMenuManager::IsAddonMenuId( nId ))
                {
                    // Special code for Add-On menu items. They can appear inside the help menu.
                    rtl::OUString aCmd( pSVMenu->GetItemCommand( nId ) );
                    rtl::OUString aImageId;

                    ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                        (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nId );

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                    pSVMenu->SetItemImage( nId, RetrieveAddOnImage( xFrame, aImageId, aCmd, FALSE, bIsHiContrastMode ));
                }
                else
                    pSVMenu->SetItemImage( nId, pBindings->GetImageManager()->GetImage( nId, pModule, FALSE, bIsHiContrastMode ) );
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

    SvtMenuOptions              aOptions;
    framework::AddonsOptions    aAddonOptions;

    BOOL bIcons = aOptions.IsMenuIconsEnabled();
    if ( bIcons )
    {
        BOOL            bIsHiContrastMode   = IsHiContrastMode();
        USHORT          nCount              = pMenu->GetItemCount();
        SfxViewFrame *  pViewFrame          = pBindings->GetDispatcher()->GetFrame();
        SfxModule*      pModule             = pViewFrame->GetObjectShell()->GetModule();
        Reference<com::sun::star::frame::XFrame> aXFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame()->GetFrameInterface() );

        for ( USHORT nPos=0; nPos < nCount; ++nPos )
        {
            USHORT nId = pMenu->GetItemId( nPos );
            PopupMenu* pPopup = pMenu->GetPopupMenu( nId );
            if ( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
            {
                sal_Bool      bImageSet = sal_False;
                rtl::OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nId );

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                pMenu->SetItemImage( nId, RetrieveAddOnImage( aXFrame, aImageId, pMenu->GetItemCommand( nId ), FALSE, bIsHiContrastMode ));
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

    USHORT  nCount = pMenu->GetItemCount();
    for ( USHORT nPos=0; nPos < nCount; ++nPos )
    {
        USHORT nId = pMenu->GetItemId( nPos );
        PopupMenu* pPopup = pMenu->GetPopupMenu( nId );
        if ( pMenu->GetItemType( nPos ) == MENUITEM_STRINGIMAGE )
            pMenu->SetItemImage( nId, Image() );
        if ( pPopup )
            RemoveMenuImages( pPopup );
    }
}

//--------------------------------------------------------------------

FASTBOOL SfxVirtualMenu::Bind_Impl( Menu *pMenu )
{
    // Selber suchen, da SV mit 'USHORT nSID = pSVMenu->GetCurItemId();' immer
    // 0 liefert. Das ist so, weil die Event-Weiterleitung lt. TH nichts mit
    // CurItem des Parent-Menus zu tun hat.
    BOOL bIsMenuBar = pSVMenu->IsMenuBar();
    sal_uInt32 nAddonsPopupPrefixLen = ADDONSPOPUPMENU_URL_PREFIX.getLength();

    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
    {
        // angesprochenes Sub-Menu gefunden?
        FASTBOOL bFound = FALSE;
        USHORT nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            // Nur ein gebundener Menu-Controller hat schon seine Id!
            if ( !rCtrl.GetId() )
            {
                BOOL bIsAddonPopupMenu = FALSE;
                DBG_ASSERT( !pSubMenu, "Popup schon vorhanden!");

                // Check if the popup is an Add-On popup menu
                // Either the popup menu has a special ID  or a special command URL prefix!
                rtl::OUString aCommand = pSVMenu->GetItemCommand( nSID );
                if ( ( nSID == SID_ADDONS ) ||
                     ( nSID == SID_ADDONHELP ) ||
                     (( (sal_uInt32)aCommand.getLength() > nAddonsPopupPrefixLen ) &&
                      ( aCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 )) )
                    bIsAddonPopupMenu = TRUE;

                // VirtualMenu f"ur Sub-Menu erzeugen
                BOOL bRes = bResCtor;
                SfxVirtualMenu *pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, FALSE, *pBindings, bOLE, bRes, bIsAddonPopupMenu );

                DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

                rCtrl.Bind( this, nSID, *pSubMenu,
                    pSVMenu->GetItemText(nSID), pSVMenu->GetHelpText(nSID),
                    *pBindings );

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
            return TRUE;
    }

    // nicht in diesem Untermenu gefunden
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
        USHORT nId = pCtrl->GetId();
        if ( !pSVMenu->GetItemCommand(nId).Len() )
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
            // UnoController sind nicht gebunden!
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
    Reference<com::sun::star::frame::XFrame> xFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame()->GetFrameInterface() );

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
        USHORT nCount = pMenu->GetItemCount();
        String aAddonsTitle( SfxResId( STR_MENU_ADDONS ));
        if ( nCount > 0 && pMenu->GetItemType( nCount-1 ) != MENUITEM_SEPARATOR )
            pMenu->InsertSeparator();
        pMenu->InsertItem( SID_ADDONS, aAddonsTitle );
        pMenu->SetPopupMenu( SID_ADDONS, pAddonMenu );

        if ( SvtMenuOptions().IsMenuIconsEnabled() )
        {
            SfxModule* pModule = pBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetModule();
            pMenu->SetItemImage( SID_ADDONS, pBindings->GetImageManager()->GetImage( SID_ADDONS, pModule, FALSE, IsHiContrastMode() ) );
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
//  if ( Application::IsInModalMode() )
//      return TRUE; // abw"urgen

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
                sal_Int32 nCount = xList->getCount();
                for( sal_Int32 i=0; i<nCount; ++i )
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

            int nRemoveItemCount = 0;
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

        InvalidateKeyCodes();
        pBindings->GetDispatcher_Impl()->Flush();
        for ( USHORT nPos = 0; nPos < nCount; ++nPos )
        {
            USHORT nId = (pItems+nPos)->GetId();
            if ( nId && nId > END_ITEMID_WINDOWLIST )
                pBindings->Update(nId);
        }

        pBindings->Update( SID_IMAGE_ORIENTATION );

        // HelpText on-demand
        if ( !bHelpInitialized )
        {
            bHelpInitialized = TRUE;
            SfxSlotPool &rSlotPool = SFX_SLOTPOOL();
            for ( USHORT nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
            {
                USHORT nId = pMenu->GetItemId(nPos);
                pMenu->SetHelpText( nId, rSlotPool.GetSlotHelpText_Impl(nId) );
            }
        }

        // bis zum Deactivate die Statusupdates unterdr"ucken
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = TRUE;

        if ( pAutoDeactivate ) // QAP-Hack
            pAutoDeactivate->Start();

        if ( IsHiContrastMode() != bWasHighContrast )
        {
            // Refresh images as our background color changed and remember it!!
            bWasHighContrast = IsHiContrastMode();
            if ( bIsAddonPopupMenu )
                UpdateImages( pSVMenu );
            else
                UpdateImages();
        }

        // erledigt
        return TRUE;
    }
    else
    {
        // VirtualMenu fuer SubMenu finden und ggf. an VirtualMenu binden
        BOOL bRet = Bind_Impl( pMenu );
#ifdef DBG_UTIL
        if ( !bRet)
            DBG_WARNING( "W1: Virtual menu konnte nicht erzeugt werden!" );
#endif
        return bRet;
    }

    return FALSE; // dead code for WTC
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
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = FALSE;
    }
    return TRUE;
}
//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Select, Menu *, pMenu )
{
    USHORT nId = (USHORT) pMenu->GetCurItemId();
    DBG_OUTF( ("SfxVirtualMenu %lx selected %u from %lx", this, nId, pMenu) );
/*
    if ( pSVMenu->GetItemCommand( nId ).Len() )
    {
        SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
        for ( USHORT nPos=0; nPos<rCtrlArr.Count(); nPos++ )
        {
            SfxMenuControl* pCtrl = rCtrlArr[nPos];
            if ( pCtrl->GetId() == nId )
            {
                SfxUnoMenuControl *pUnoCtrl = (SfxUnoMenuControl*) pCtrl;
                pUnoCtrl->Select();
                return TRUE;
            }
        }
    }
*/
    if ( nId >= START_ITEMID_WINDOWLIST && nId <= END_ITEMID_WINDOWLIST )
    {
        // window list menu item selected
        Reference< XFramesSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                        DEFINE_CONST_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
        USHORT  nWindowItemId = START_ITEMID_WINDOWLIST;

        if ( xDesktop.is() )
        {
            USHORT nTaskId = START_ITEMID_WINDOWLIST;
            Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
            sal_Int32 nCount = xList->getCount();
            for ( sal_Int32 i=0; i<nCount; ++i )
            {
                Any aItem = xList->getByIndex(i);
                Reference< XFrame > xFrame;
                if (( aItem >>= xFrame ) && xFrame.is() && nTaskId == nId )
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
    else if ( nId >= START_ITEMID_PICKLIST && nId <= END_ITEMID_PICKLIST )
    {
        SfxPickList::Get()->ExecuteMenuEntry( nId );
        return sal_True;
    }

    if ( pMenu->GetItemCommand( nId ).Len() )
        pBindings->ExecuteCommand_Impl( pMenu->GetItemCommand( nId ) );
    else
        pBindings->Execute( nId );

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

SfxVirtualMenu* SfxVirtualMenu::GetPopupMenu( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetPopupMenu();
    return 0;
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemText( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetTitle();
    return String();
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemHelpText( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetHelpText();
    return String();
}

//--------------------------------------------------------------------

void SfxVirtualMenu::InvalidateKeyCodes()
{
    DBG_ASSERT( pSVMenu, "invalidating key of incomplete menu" );

    SfxApplication* pSfxApp = SFX_APP();
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher()->GetFrame();
    SfxAcceleratorManager* pAccMgr = pViewFrame->GetViewShell()->GetAccMgr_Impl();
    SfxAcceleratorManager* pAppAccel = pSfxApp->GetAppAccel_Impl();
    if ( !pAccMgr )
        pAccMgr = pAppAccel;

    for ( USHORT nPos = 0; nPos < pSVMenu->GetItemCount(); ++nPos )
    {
        USHORT nId = pSVMenu->GetItemId(nPos);
        SfxVirtualMenu *pPopup = GetPopupMenu(nId);
//        if ( pPopup )
//            pPopup->InvalidateKeyCodes();
//        else if ( nId )
        if ( nId && !pSVMenu->GetPopupMenu( nId ) )
        {
            KeyCode aCode = pAccMgr->GetKeyCode( nId );
            if ( !aCode.GetCode() && pAccMgr != pAppAccel )
                aCode = pAppAccel->GetKeyCode( nId );
            pSVMenu->SetAccelKey( nId, aCode );
        }
    }
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

void SfxVirtualMenu::SetPopupMenu( USHORT nId, PopupMenu *pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    if (pSVMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND )
        GetSVMenu()->SetPopupMenu( nId, pMenu );
    for ( USHORT n = 0; n < nCount; ++n )
    {
        SfxVirtualMenu *pSubMenu = (pItems+n)->GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->SetPopupMenu( nId, pMenu );
    }
}

//--------------------------------------------------------------------

// Erzwingt die Initialisierung, die sonst nur im Activate kommt

void SfxVirtualMenu::InitPopup( USHORT nPos, BOOL bOLE )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "Hier gibt es kein Popup!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        // VirtualMenu f"ur Sub-Menu erzeugen
        BOOL bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, FALSE, *pBindings, bOLE, bRes);

        DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

        rCtrl.Bind( this, nSID, *pSubMenu,
            pSVMenu->GetItemText(nSID), pSVMenu->GetHelpText(nSID),
            *pBindings );
    }
}

void SfxVirtualMenu::InitializeHelp()
{
    SfxSlotPool &rSlotPool = SFX_SLOTPOOL();
    for ( USHORT nPos = 0; nPos<pSVMenu->GetItemCount(); ++nPos )
    {
        USHORT nId = pSVMenu->GetItemId(nPos);
        if ( !bHelpInitialized )
            pSVMenu->SetHelpText( nId, rSlotPool.GetSlotHelpText_Impl( nId ) );
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( nId && !rCtrl.GetId() )
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

