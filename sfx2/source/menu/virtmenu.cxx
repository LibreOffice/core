/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

DBG_NAME(SfxVirtualMenu)

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
            pSVMenu->SetItemImageMirrorMode( nslotId, false );
            pSVMenu->SetItemImageAngle( nslotId, lRotation );
        }

        if ( pSlot && pSlot->IsMode( SFX_SLOT_IMAGEREFLECTION ) )
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
                SfxVirtualMenu* pOwnParent, Menu& rMenu, sal_Bool bWithHelp,
                SfxBindings &rBindings, sal_Bool bOLEServer, sal_Bool bRes, sal_Bool bIsAddonMenu ):
    pItems(0),
       pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{
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





SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, sal_Bool bWithHelp,
                    SfxBindings &rBindings, sal_Bool bOLEServer, sal_Bool bRes, sal_Bool bIsAddonMenu ):
    pItems(0),
       pImageControl(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0),
    bHelpInitialized( bWithHelp ),
    bIsAddonPopupMenu( bIsAddonMenu )
{
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



/*  The destructor of the class SfxVirtualMenu releases bounded items and
    the associated StarView-PopupMenu is released from its parent.
    If it is related to the Pickmenu or the MDI-menu, it is unregistered here.
*/

SfxVirtualMenu::~SfxVirtualMenu()
{
    DBG_DTOR(SfxVirtualMenu, 0);

    DELETEZ( pImageControl );
    SvtMenuOptions().RemoveListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    if ( bIsActive )
    {
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = sal_False;
    }

    if (pItems)
    {
        delete [] pItems;
    }

    delete pAppCtrl;
    pBindings = 0;

    
    
    
    
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



void SfxVirtualMenu::CreateFromSVMenu()
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
    Reference< com::sun::star::frame::XFrame > xFrame( pViewFrame->GetFrame().GetFrameInterface() );

    if ( pSVMenu->IsMenuBar() )
    {
        sal_uInt16 nPos = pSVMenu->GetItemPos( SID_MDIWINDOWLIST );
        if ( nPos != MENU_ITEM_NOTFOUND && xFrame.is() )
        {
            
            Reference< com::sun::star::frame::XModel >      xModel;
            Reference< com::sun::star::frame::XController > xController( xFrame->getController(), UNO_QUERY );
            if ( xController.is() )
                xModel = Reference< com::sun::star::frame::XModel >( xController->getModel(), UNO_QUERY );
            framework::AddonMenuManager::MergeAddonPopupMenus( xFrame, xModel, nPos, (MenuBar *)pSVMenu );
        }

        
        if ( xFrame.is() )
            framework::AddonMenuManager::MergeAddonHelpMenu( xFrame, (MenuBar *)pSVMenu );

        
        
        pAddonsMenu = pSVMenu->GetPopupMenu( SID_ADDONLIST );
    }
    else if ( pParent )
    {
        if ( pSVMenu == pParent->pAddonsMenu &&
             framework::AddonsOptions().HasAddonsMenu() &&
             !pSVMenu->GetPopupMenu( SID_ADDONS ) )
        {
            
            InsertAddOnsMenuItem( pSVMenu );
        }
    }

    
    nCount = pSVMenu->GetItemCount();

    
    
    
    if (nCount)
        pItems = new SfxMenuControl[nCount];

    
    SFX_APP();
    const int bOleServer = sal_False;
    const int bMac = sal_False;
    SvtMenuOptions aOptions;
    aOptions.AddListenerLink( LINK( this, SfxVirtualMenu, SettingsChanged ) );

    
    pBindings->ENTERREGISTRATIONS(); ++nLocks;
    pImageControl = new SfxMenuImageControl_Impl( SID_IMAGE_ORIENTATION, *pBindings, this );

    sal_uInt16 nSVPos = 0;
    for ( sal_uInt16 nPos=0; nPos<nCount; ++nPos, ++nSVPos )
    {
        sal_uInt16 nSlotId = pSVMenu->GetItemId(nSVPos);
        PopupMenu* pPopup = pSVMenu->GetPopupMenu(nSlotId);
        if( pPopup && nSlotId >= SID_OBJECTMENU0 && nSlotId <= SID_OBJECTMENU_LAST )
        {
            
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
                    OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    if ( !aCmd.isEmpty() && (( nSlotId < SID_SFX_START ) || ( nSlotId > SHRT_MAX )) )
                    {
                        
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
                        
                        pMnuCtrl = SfxMenuControl::CreateControl(nSlotId, *pSVMenu, *pBindings);
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.push_back(pMnuCtrl);
                            (pItems+nPos)->Bind( 0, nSlotId, sItemText, *pBindings);
                        }
                        else
                            
                            pMnuCtrl = (pItems+nPos);

                        pMnuCtrl->Bind( this, nSlotId, sItemText, *pBindings);
                    }

                    if ( Application::GetSettings().GetStyleSettings().GetUseImagesInMenus() )
                    {
                        Image aImage;
                        if ( bIsAddonPopupMenu || framework::AddonMenuManager::IsAddonMenuId( nSlotId ))
                        {
                            OUString aImageId;

                            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                                (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nSlotId );

                            if ( pMenuAttributes )
                                aImageId = pMenuAttributes->aImageId; 

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

                case MENUITEM_IMAGE:
                    
                    break;

                case MENUITEM_SEPARATOR:
                    
                    break;
                default:
                    break; 
            }
        }
    }
    pBindings->LEAVEREGISTRATIONS(); --nLocks;
}





IMPL_LINK_NOARG( SfxVirtualMenu, Highlight )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    return sal_True;
}

IMPL_LINK_NOARG(SfxVirtualMenu, SettingsChanged)
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
                    
                    OUString aCmd( pSVMenu->GetItemCommand( nSlotId ) );
                    OUString aImageId;

                    ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                        (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nSlotId );

                    if ( pMenuAttributes )
                        aImageId = pMenuAttributes->aImageId; 

                    pSVMenu->SetItemImage( nSlotId, RetrieveAddOnImage( xFrame, aImageId, aCmd, false ));
                }
                else
                {
                    OUString aSlotURL( "slot:" );
                    aSlotURL += OUString::number( nSlotId);
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
        
        if ( !bIcons )
            RemoveMenuImages( pSVMenu );
        else
            UpdateImages( pSVMenu );
    }

    
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
                OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nSlotId );

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; 

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
        if ( pMenu->GetItemType( nPos ) == MENUITEM_STRINGIMAGE )
            pMenu->SetItemImage( nSlotId, Image() );
        if ( pPopup )
            RemoveMenuImages( pPopup );
    }
}



bool SfxVirtualMenu::Bind_Impl( Menu *pMenu )
{
    
    
    
    sal_uInt32 nAddonsPopupPrefixLen = ADDONSPOPUPMENU_URL_PREFIX.getLength();

    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
    {
        sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        
        bool bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            
            if ( !rCtrl.GetId() )
            {
                bIsAddonPopupMenu = sal_False;
                DBG_ASSERT( !pSubMenu, "Popup already exists!");

                
                
                OUString aCommand = pSVMenu->GetItemCommand( nSID );
                if ( ( nSID == SID_ADDONS ) ||
                     ( nSID == SID_ADDONHELP ) ||
                     (( (sal_uInt32)aCommand.getLength() > nAddonsPopupPrefixLen ) &&
                      ( aCommand.startsWith( ADDONSPOPUPMENU_URL_PREFIX ) ) ) )
                    bIsAddonPopupMenu = sal_True;

                
                sal_Bool bRes = bResCtor;
                pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, sal_False, *pBindings, bOLE, bRes, bIsAddonPopupMenu );

                DBG_OUTF( ("New VirtualMenu %lx created", pSubMenu) );

                rCtrl.Bind( this, nSID, *pSubMenu, pSVMenu->GetItemText(nSID), *pBindings );

                
                pSubMenu->Bind_Impl( pMenu );
                pSubMenu->Activate( pMenu );
            }
        }

        
        
        if ( !bFound && pSubMenu )
            bFound = pSubMenu->Bind_Impl( pMenu );

        
        if ( bFound )
            return true;
    }

    
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
    for (SfxMenuCtrlArr_Impl::iterator i = rCtrlArr.begin();
            i != rCtrlArr.end(); ++i)
    {
        if (i->IsBound())
        {
            
            i->UnBind();
        }
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = sal_True;
}



void SfxVirtualMenu::InsertAddOnsMenuItem( Menu* pMenu )
{
    
    ::framework::MenuConfiguration aConf( ::comphelper::getProcessComponentContext() );
    Reference<com::sun::star::frame::XFrame> xFrame( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );

    PopupMenu* pAddonMenu = NULL;
    try
    {
        pAddonMenu = framework::AddonMenuManager::CreateAddonMenu( xFrame );
    }
    catch ( const ::com::sun::star::lang::WrappedTargetException& )
    {
    }

    
    if ( pAddonMenu && pAddonMenu->GetItemCount() > 0 )
    {
        sal_uInt16 nItemCount = pMenu->GetItemCount();
        OUString aAddonsTitle(SfxResId(STR_MENU_ADDONS).toString());
        if ( nItemCount > 0 && pMenu->GetItemType( nItemCount-1 ) != MENUITEM_SEPARATOR )
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





IMPL_LINK( SfxVirtualMenu, Activate, Menu *, pMenu )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_OUTF( ("SfxVirtualMenu %lx activated %lx, own %lx", this, pMenu, pSVMenu));

    
    

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

    
    if ( pMenu == pSVMenu )
    {
        
        if ( bIsActive )
            return sal_True;

        
        if ( pParent && pSVMenu == pParent->pPickMenu )
        {
            SfxPickList::Get().CreateMenuEntries( pParent->pPickMenu );
        }
        else
            pPickMenu = pSVMenu->GetPopupMenu(SID_PICKLIST);

        if ( pParent && pSVMenu == pParent->pWindowMenu )
        {
            
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

                Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWin && pWin->IsVisible() )
                {
                    aNewWindowListVector.push_back( pWin->GetText() );
                    ++nItemId;
                }
            }

            int nItemCount       = pMenu->GetItemCount();

            if ( nItemCount > 0 )
            {
                
                sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
                for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                    pMenu->RemoveItem( n );

                if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                    pMenu->RemoveItem( pMenu->GetItemCount()-1 );
            }

            if ( aNewWindowListVector.size() > 0 )
            {
                
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
            
            pAddonsMenu = pSVMenu->GetPopupMenu( SID_ADDONLIST );
        }

        
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

        
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = sal_True;

        return sal_True;
    }
    else
    {
        
        
        bool bRet = Bind_Impl( pMenu );
#ifdef DBG_UTIL
        if ( !bRet)
            DBG_WARNING( "W1: Virtual menu could not be created!" );
#endif
        return long(bRet);
    }
}



IMPL_LINK( SfxVirtualMenu, Deactivate, Menu *, pMenu )
{
    DBG_OUTF( ("SfxVirtualMenu %lx deactivated %lx, own %lx", this, pMenu, pSVMenu) );
    if ( bIsActive && ( 0 == pMenu || pMenu == pSVMenu ) )
    {
        
        
        if ( pParent )
            UnbindControllers();
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = sal_False;
    }
    return sal_True;
}




IMPL_LINK( SfxVirtualMenu, Select, Menu *, pMenu )
{
    sal_uInt16 nSlotId = (sal_uInt16) pMenu->GetCurItemId();
    DBG_OUTF( ("SfxVirtualMenu %lx selected %u from %lx", this, nSlotId, pMenu) );

    if ( nSlotId >= START_ITEMID_WINDOWLIST && nSlotId <= END_ITEMID_WINDOWLIST )
    {
        
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
                Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                pWin->GrabFocus();
                pWin->ToTop( TOTOP_RESTOREWHENMIN );
                break;
            }

            nTaskId++;
        }

        return sal_True;
    }
    else if ( nSlotId >= START_ITEMID_PICKLIST && nSlotId <= END_ITEMID_PICKLIST )
    {
        SfxPickList::Get().ExecuteMenuEntry( nSlotId );
        return sal_True;
    }

    OUString sCommand = pMenu->GetItemCommand(nSlotId);
    if (!sCommand.isEmpty())
        pBindings->ExecuteCommand_Impl(sCommand);
    else
        pBindings->Execute(nSlotId);

    return sal_True;
}





Menu* SfxVirtualMenu::GetSVMenu() const
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    return pSVMenu;
}





void SfxVirtualMenu::CheckItem( sal_uInt16 nItemId, sal_Bool bCheck )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->CheckItem( nItemId, bCheck );
}




void SfxVirtualMenu::EnableItem( sal_uInt16 nItemId, sal_Bool bEnable )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->EnableItem( nItemId, bEnable );
}




void SfxVirtualMenu::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->SetItemText( nItemId, rText );
}




void SfxVirtualMenu::SetPopupMenu( sal_uInt16 nItemId, PopupMenu *pMenu )
{
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





void SfxVirtualMenu::InitPopup( sal_uInt16 nPos, sal_Bool /*bOLE*/ )
{
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    sal_uInt16 nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "No popup exist here!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        
        sal_Bool bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, sal_False, *pBindings, bOLE, bRes);

        DBG_OUTF( ("New VirtualMenu %lx created", pSubMenu) );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
