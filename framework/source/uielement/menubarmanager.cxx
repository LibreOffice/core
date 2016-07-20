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

#include <uielement/menubarmanager.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <framework/addonsoptions.hxx>
#include <classes/fwkresid.hxx>
#include <helper/mischelper.hxx>
#include <framework/menuextensionsupplier.hxx>
#include <classes/resource.hrc>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XStringWidth.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <uno/current_context.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/miscopt.hxx>
#include <uielement/menubarmerger.hxx>
#include <tools/urlobj.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;

const sal_uInt16 ADDONMENU_MERGE_ITEMID_START = 1500;

namespace framework
{

// special menu ids/command ids for dynamic popup menus
#define SID_SFX_START           5000
#define SID_MDIWINDOWLIST       (SID_SFX_START + 610)
#define SID_HELPMENU            (SID_SFX_START + 410)

#define aCmdHelpIndex ".uno:HelpIndex"
#define aCmdToolsMenu ".uno:ToolsMenu"
#define aCmdHelpMenu ".uno:HelpMenu"
#define aSlotHelpMenu "slot:5410"

#define aSpecialWindowMenu "window"
#define aSlotSpecialWindowMenu "slot:5610"
#define aSlotSpecialToolsMenu "slot:6677"

// special uno commands for window list
#define aSpecialWindowCommand ".uno:WindowList"

static sal_Int16 getImageTypeFromBools( bool bBig )
{
    sal_Int16 n( 0 );
    if ( bBig )
        n |= css::ui::ImageType::SIZE_LARGE;
    return n;
}

MenuBarManager::MenuBarManager(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& rFrame,
    const Reference< XURLTransformer >& _xURLTransformer,
    const Reference< XDispatchProvider >& rDispatchProvider,
    const OUString& rModuleIdentifier,
    Menu* pMenu, bool bDelete, bool bDeleteChildren, bool bHasMenuBar ):
    OWeakObject()
    , m_bDisposed( false )
    , m_bRetrieveImages( false )
    , m_bAcceleratorCfg( false )
    , m_bModuleIdentified( false )
    , m_bHasMenuBar( bHasMenuBar )
    , m_aListenerContainer( m_mutex )
    , m_xContext(rxContext)
    , m_xURLTransformer(_xURLTransformer)
    , m_sIconTheme( SvtMiscOptions().GetIconTheme() )
{
    m_aAsyncSettingsTimer.SetDebugName( "framework::MenuBarManager::Deactivate m_aAsyncSettingsTimer" );
    m_xPopupMenuControllerFactory = frame::thePopupMenuControllerFactory::get(m_xContext);
    FillMenuManager( pMenu, rFrame, rDispatchProvider, rModuleIdentifier, bDelete, bDeleteChildren );
}

MenuBarManager::MenuBarManager(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& rFrame,
    const Reference< XURLTransformer >& _xURLTransformer,
    Menu* pAddonMenu,
    bool bDelete,
    bool bDeleteChildren,
    bool popup):
    OWeakObject()
    , m_bDisposed( false )
    , m_bRetrieveImages( true )
    , m_bAcceleratorCfg( false )
    , m_bModuleIdentified( false )
    , m_bHasMenuBar( true )
    , m_aListenerContainer( m_mutex )
    , m_xContext(rxContext)
    , m_xURLTransformer(_xURLTransformer)
    , m_sIconTheme( SvtMiscOptions().GetIconTheme() )
{
    m_aAsyncSettingsTimer.SetDebugName( "framework::MenuBarManager::Deactivate m_aAsyncSettingsTimer" );
    Init(rFrame,pAddonMenu,bDelete,bDeleteChildren, popup);
}

Any SAL_CALL MenuBarManager::queryInterface( const Type & rType ) throw ( RuntimeException, std::exception )
{
    Any a = ::cppu::queryInterface(
                rType ,
                (static_cast< css::frame::XStatusListener* >(this)),
                (static_cast< css::frame::XFrameActionListener* >(this)),
                (static_cast< css::ui::XUIConfigurationListener* >(this)),
                (static_cast< XEventListener* >(static_cast<XStatusListener *>(this))),
                (static_cast< XComponent* >(this)),
                (static_cast< css::awt::XSystemDependentMenuPeer* >(this)));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL MenuBarManager::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL MenuBarManager::release() throw()
{
    OWeakObject::release();
}

Any SAL_CALL MenuBarManager::getMenuHandle( const Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    Any a;

    if ( m_pVCLMenu )
    {
        SystemMenuData aSystemMenuData;

        m_pVCLMenu->GetSystemMenuData( &aSystemMenuData );
#ifdef _WIN32
        if( SystemType == SystemDependent::SYSTEM_WIN32 )
        {
            a <<= sal_Int64(
                reinterpret_cast<sal_IntPtr>(aSystemMenuData.hMenu));
        }
#else
        (void) SystemType;
#endif
    }

    return a;
}

MenuBarManager::~MenuBarManager()
{
    // stop asynchronous settings timer
    m_xDeferedItemContainer.clear();
    m_aAsyncSettingsTimer.Stop();

    SAL_WARN_IF( OWeakObject::m_refCount != 0, "fwk.uielement", "Who wants to delete an object with refcount > 0!" );
}

void MenuBarManager::Destroy()
{
    SolarMutexGuard aGuard;

    if ( !m_bDisposed )
    {
        // stop asynchronous settings timer and
        // release defered item container reference
        m_aAsyncSettingsTimer.Stop();
        m_xDeferedItemContainer.clear();
        RemoveListener();

        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
            pItemHandler->xSubMenuManager.clear();
            pItemHandler->xPopupMenu.clear();
            delete pItemHandler;
        }
        m_aMenuItemHandlerVector.clear();

        if ( m_bDeleteMenu )
        {
            m_pVCLMenu.disposeAndClear();
        }
    }
}

// XComponent
void SAL_CALL MenuBarManager::dispose() throw( RuntimeException, std::exception )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        SolarMutexGuard g;
        Destroy();
        m_bDisposed = true;

        if ( m_xDocImageManager.is() )
        {
            try
            {
                m_xDocImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( const Exception& )
            {
            }
        }
        if ( m_xModuleImageManager.is() )
        {
            try
            {
                m_xModuleImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( const Exception& )
            {
            }
        }
        m_xDocImageManager.clear();
        m_xModuleImageManager.clear();
        Reference< XComponent > xCompGAM( m_xGlobalAcceleratorManager, UNO_QUERY );
        if ( xCompGAM.is() )
            xCompGAM->dispose();
        m_xGlobalAcceleratorManager.clear();
        m_xModuleAcceleratorManager.clear();
        m_xDocAcceleratorManager.clear();
        m_xUICommandLabels.clear();
        m_xPopupMenuControllerFactory.clear();
        m_xContext.clear();
    }
}

void SAL_CALL MenuBarManager::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL MenuBarManager::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard g;
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL MenuBarManager::elementInserted( const css::ui::ConfigurationEvent& Event )
throw (RuntimeException, std::exception)
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    sal_Int16 nImageType = sal_Int16();
    sal_Int16 nCurrentImageType = getImageTypeFromBools( false );
    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ))
        RequestImages();
}

void SAL_CALL MenuBarManager::elementRemoved( const css::ui::ConfigurationEvent& Event )
throw (RuntimeException, std::exception)
{
    elementInserted(Event);
}

void SAL_CALL MenuBarManager::elementReplaced( const css::ui::ConfigurationEvent& Event )
throw (RuntimeException, std::exception)
{
    elementInserted(Event);
}

// XFrameActionListener
void SAL_CALL MenuBarManager::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
    {
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            // Clear dispatch reference as we will requery it later o
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
        }
    }
}

// XStatusListener
void SAL_CALL MenuBarManager::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException, std::exception )
{
    OUString aFeatureURL = Event.FeatureURL.Complete;

    SolarMutexGuard aSolarGuard;
    {
        if ( m_bDisposed )
            return;

        // We have to check all menu entries as there can be identical entries in a popup menu.
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pMenuItemHandler = *p;
            if ( pMenuItemHandler->aMenuItemURL == aFeatureURL )
            {
                bool            bCheckmark( false );
                bool            bMenuItemEnabled( m_pVCLMenu->IsItemEnabled( pMenuItemHandler->nItemId ));
                bool            bEnabledItem( Event.IsEnabled );
                OUString       aItemText;
                status::Visibility  aVisibilityStatus;

                #ifdef UNIX
                //enable some slots hardly, because UNIX clipboard does not notify all changes
                // Can be removed if follow up task will be fixed directly within applications.
                // Note: PasteSpecial is handled specifically by calc
                // Calc also disables Paste under some circumstances, do not override.
                /* TODO: is this workaround even needed anymore? Was introduced
                 * in 2009 with commit 426ab2c0e8f6e3fe2b766f74f6b8da873d860260
                 * as some "metropatch" and the other places it touched seem to
                 * be gone. */
                if ( (pMenuItemHandler->aMenuItemURL == ".uno:Paste" &&
                            m_aModuleIdentifier != "com.sun.star.sheet.SpreadsheetDocument")
                        || pMenuItemHandler->aMenuItemURL == ".uno:PasteClipboard" )    // special for draw/impress
                    bEnabledItem = true;
                #endif

                // Enable/disable item
                if ( bEnabledItem != bMenuItemEnabled )
                    m_pVCLMenu->EnableItem( pMenuItemHandler->nItemId, bEnabledItem );

                if ( Event.State >>= bCheckmark )
                {
                    // Checkmark or RadioButton
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId );
                    m_pVCLMenu->CheckItem( pMenuItemHandler->nItemId, bCheckmark );

                    MenuItemBits nBits = m_pVCLMenu->GetItemBits( pMenuItemHandler->nItemId );
                    //If not already designated RadioButton set as CheckMark
                    if (!(nBits & MenuItemBits::RADIOCHECK))
                        m_pVCLMenu->SetItemBits( pMenuItemHandler->nItemId, nBits | MenuItemBits::CHECKABLE );
                }
                else if ( Event.State >>= aItemText )
                {
                    INetURLObject aURL( aFeatureURL );
                    OUString aEnumPart = aURL.GetURLPath().getToken( 1, '.' );
                    if ( !aEnumPart.isEmpty() && aURL.GetProtocol() == INetProtocol::Uno )
                    {
                        // Checkmark or RadioButton
                        m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId );
                        m_pVCLMenu->CheckItem( pMenuItemHandler->nItemId, aItemText == aEnumPart );

                        MenuItemBits nBits = m_pVCLMenu->GetItemBits( pMenuItemHandler->nItemId );
                        //If not already designated RadioButton set as CheckMark
                        if (!(nBits & MenuItemBits::RADIOCHECK))
                            m_pVCLMenu->SetItemBits( pMenuItemHandler->nItemId, nBits | MenuItemBits::CHECKABLE );
                    }
                    else
                    {
                        // Replacement for place holders
                        if ( aItemText.startsWith("($1)") )
                        {
                            OUString aTmp(FWK_RESSTR(STR_UPDATEDOC));
                            aTmp += " ";
                            aTmp += aItemText.copy( 4 );
                            aItemText = aTmp;
                        }
                        else if ( aItemText.startsWith("($2)") )
                        {
                            OUString aTmp(FWK_RESSTR(STR_CLOSEDOC_ANDRETURN));
                            aTmp += aItemText.copy( 4 );
                            aItemText = aTmp;
                        }
                        else if ( aItemText.startsWith("($3)") )
                        {
                            OUString aTmp(FWK_RESSTR(STR_SAVECOPYDOC));
                            aTmp += aItemText.copy( 4 );
                            aItemText = aTmp;
                        }

                        m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId );
                        m_pVCLMenu->SetItemText( pMenuItemHandler->nItemId, aItemText );
                    }
                }
                else if ( Event.State >>= aVisibilityStatus )
                {
                    // Visibility
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId, aVisibilityStatus.bVisible );
                }
                else
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId );
            }

            if ( Event.Requery )
            {
                // Release dispatch object - will be requeried on the next activate!
                pMenuItemHandler->xMenuItemDispatch.clear();
            }
        }
    }
}

// Helper to retrieve own structure from item ID
MenuBarManager::MenuItemHandler* MenuBarManager::GetMenuItemHandler( sal_uInt16 nItemId )
{
    SolarMutexGuard g;

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->nItemId == nItemId )
            return pItemHandler;
    }

    return nullptr;
}

// Helper to set request images flag
void MenuBarManager::RequestImages()
{

    m_bRetrieveImages = true;
    const sal_uInt32 nCount = m_aMenuItemHandlerVector.size();
    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        MenuItemHandler* pItemHandler = m_aMenuItemHandlerVector[i];
        if ( pItemHandler->xSubMenuManager.is() )
        {
            MenuBarManager* pMenuBarManager = static_cast<MenuBarManager*>(pItemHandler->xSubMenuManager.get());
            pMenuBarManager->RequestImages();
        }
    }
}

// Helper to reset objects to prepare shutdown
void MenuBarManager::RemoveListener()
{
    SolarMutexGuard g;

    // Check service manager reference. Remove listener can be called due
    // to a disposing call from the frame and therefore we already removed
    // our listeners and release the service manager reference!
    if ( m_xContext.is() )
    {
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pItemHandler = *p;
            if ( pItemHandler->xMenuItemDispatch.is() )
            {
                URL aTargetURL;
                aTargetURL.Complete = pItemHandler->aMenuItemURL;
                m_xURLTransformer->parseStrict( aTargetURL );

                pItemHandler->xMenuItemDispatch->removeStatusListener(
                    static_cast< XStatusListener* >( this ), aTargetURL );
            }

            pItemHandler->xMenuItemDispatch.clear();

            // Remove popup menu from menu structure (regardless whether an
            // xPopupMenu proxy has been set up for it; calling SetPopupMenu for
            // a non-popup child will effectively do nothing:
            m_pVCLMenu->SetPopupMenu( pItemHandler->nItemId, nullptr );

            if ( pItemHandler->xPopupMenu.is() )
            {
                Reference< css::lang::XEventListener > xEventListener( pItemHandler->xPopupMenuController, UNO_QUERY );
                if ( xEventListener.is() )
                {
                    EventObject aEventObject;
                    aEventObject.Source = static_cast<OWeakObject *>(this);
                    xEventListener->disposing( aEventObject );
                }

                // We now provide a popup menu controller to external code.
                // Therefore the life-time must be explicitly handled via
                // dispose!!
                try
                {
                    Reference< XComponent > xComponent( pItemHandler->xPopupMenuController, UNO_QUERY );
                    if ( xComponent.is() )
                        xComponent->dispose();
                }
                catch ( const RuntimeException& )
                {
                    throw;
                }
                catch ( const Exception& )
                {
                }

                // Release references to controller and popup menu
                pItemHandler->xPopupMenuController.clear();
                pItemHandler->xPopupMenu.clear();
            }

            Reference< XComponent > xComponent( pItemHandler->xSubMenuManager, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
    }

    try
    {
        if ( m_xFrame.is() )
            m_xFrame->removeFrameActionListener( Reference< XFrameActionListener >(
                                                    static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
    catch ( const Exception& )
    {
    }

    m_xFrame = nullptr;
}

void SAL_CALL MenuBarManager::disposing( const EventObject& Source ) throw ( RuntimeException, std::exception )
{
    MenuItemHandler* pMenuItemDisposing = nullptr;

    SolarMutexGuard g;

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pMenuItemHandler = *p;
        if ( pMenuItemHandler->xMenuItemDispatch.is() &&
             pMenuItemHandler->xMenuItemDispatch == Source.Source )
        {
            // disposing called from menu item dispatcher, remove listener
            pMenuItemDisposing = pMenuItemHandler;
            break;
        }
    }

    if ( pMenuItemDisposing )
    {
        // Release references to the dispatch object
        URL aTargetURL;
        aTargetURL.Complete = pMenuItemDisposing->aMenuItemURL;

        // Check reference of service manager before we use it. Reference could
        // be cleared due to RemoveListener call!
        if ( m_xContext.is() )
        {
            m_xURLTransformer->parseStrict( aTargetURL );

            pMenuItemDisposing->xMenuItemDispatch->removeStatusListener(
                static_cast< XStatusListener* >( this ), aTargetURL );
            pMenuItemDisposing->xMenuItemDispatch.clear();
            if ( pMenuItemDisposing->xPopupMenu.is() )
            {
                Reference< css::lang::XEventListener > xEventListener( pMenuItemDisposing->xPopupMenuController, UNO_QUERY );
                if ( xEventListener.is() )
                    xEventListener->disposing( Source );

                {
                    // Remove popup menu from menu structure as we release our reference to
                    // the controller.
                    m_pVCLMenu->SetPopupMenu( pMenuItemDisposing->nItemId, nullptr );
                }

                pMenuItemDisposing->xPopupMenuController.clear();
                pMenuItemDisposing->xPopupMenu.clear();
            }
        }
        return;
    }
    else if ( Source.Source == m_xFrame )
    {
        // Our frame gets disposed. We have to remove all our listeners
        RemoveListener();
    }
    else if ( Source.Source == Reference< XInterface >( m_xDocImageManager, UNO_QUERY ))
        m_xDocImageManager.clear();
    else if ( Source.Source == Reference< XInterface >( m_xModuleImageManager, UNO_QUERY ))
        m_xModuleImageManager.clear();
}

void MenuBarManager::CheckAndAddMenuExtension( Menu* pMenu )
{

    // retrieve menu extension item
    MenuExtensionItem aMenuItem( GetMenuExtension() );
    if (( !aMenuItem.aURL.isEmpty() ) &&
        ( !aMenuItem.aLabel.isEmpty() ))
    {
        // remove all old window list entries from menu
        sal_uInt16 nNewItemId( 0 );
        sal_uInt16 nInsertPos( MENU_APPEND );
        sal_uInt16 nBeforePos( MENU_APPEND );
        OUString aCommandBefore( ".uno:About" );
        for ( sal_uInt16 n = 0; n < pMenu->GetItemCount(); n++ )
        {
            sal_uInt16 nItemId = pMenu->GetItemId( n );
            nNewItemId = std::max( nItemId, nNewItemId );
            if ( pMenu->GetItemCommand( nItemId ) == aCommandBefore )
                nBeforePos = n;
        }
        ++nNewItemId;

        if ( nBeforePos != MENU_APPEND )
            nInsertPos = nBeforePos;

        pMenu->InsertItem(nNewItemId, aMenuItem.aLabel, MenuItemBits::NONE, OString(), nInsertPos);
        pMenu->SetItemCommand( nNewItemId, aMenuItem.aURL );
    }
}

static void lcl_CheckForChildren(Menu* pMenu, sal_uInt16 nItemId)
{
    if (PopupMenu* pThisPopup = pMenu->GetPopupMenu( nItemId ))
        pMenu->EnableItem( nItemId, pThisPopup->GetItemCount() != 0 );
}

// vcl handler

namespace {

class QuietInteractionContext:
    public cppu::WeakImplHelper< css::uno::XCurrentContext >
{
public:
    explicit QuietInteractionContext(
        css::uno::Reference< css::uno::XCurrentContext >
            const & context):
        context_(context) {}
    QuietInteractionContext(const QuietInteractionContext&) = delete;
    QuietInteractionContext& operator=(const QuietInteractionContext&) = delete;

private:
    virtual ~QuietInteractionContext() override {}

    virtual css::uno::Any SAL_CALL getValueByName(
        OUString const & Name)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return Name != JAVA_INTERACTION_HANDLER_NAME && context_.is()
            ? context_->getValueByName(Name)
            : css::uno::Any();
    }

    css::uno::Reference< css::uno::XCurrentContext >
        context_;
};

}

IMPL_LINK_TYPED( MenuBarManager, Activate, Menu *, pMenu, bool )
{
    if ( pMenu == m_pVCLMenu )
    {
        css::uno::ContextLayer layer(
            new QuietInteractionContext(
                css::uno::getCurrentContext()));

        // set/unset hiding disabled menu entries
        bool bDontHide           = SvtMenuOptions().IsEntryHidingEnabled();
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        bool bShowMenuImages     = rSettings.GetUseImagesInMenus();
        bool bShowShortcuts      = m_bHasMenuBar || rSettings.GetContextMenuShortcuts();
        bool bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );

        SolarMutexGuard g;

        MenuFlags nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MenuFlags::HideDisabledEntries;
        else
            nFlag |= MenuFlags::HideDisabledEntries;
        pMenu->SetMenuFlags( nFlag );

        if ( m_bActive )
            return false;

        m_bActive = true;

        OUString aMenuCommand( m_aMenuItemCommand );
        if ( m_aMenuItemCommand == aSpecialWindowMenu || m_aMenuItemCommand == aSlotSpecialWindowMenu || aMenuCommand == aSpecialWindowCommand )
            UpdateSpecialWindowMenu( pMenu, m_xContext );

        // Check if some modes have changed so we have to update our menu images
        OUString sIconTheme = SvtMiscOptions().GetIconTheme();

        if ( m_bRetrieveImages ||
             bShowMenuImages != m_bShowMenuImages ||
             sIconTheme != m_sIconTheme )
        {
            m_bShowMenuImages   = bShowMenuImages;
            m_bRetrieveImages   = false;
            m_sIconTheme     = sIconTheme;
            FillMenuImages( m_xFrame, pMenu, bShowMenuImages );
        }

        // Try to map commands to labels
        for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            sal_uInt16 nItemId = pMenu->GetItemId( nPos );
            if (( pMenu->GetItemType( nPos ) != MenuItemType::SEPARATOR ) &&
                ( pMenu->GetItemText( nItemId ).isEmpty() ))
            {
                OUString aCommand = pMenu->GetItemCommand( nItemId );
                if ( !aCommand.isEmpty() ) {
                    pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aCommand ));
                }
            }
        }

        // Try to set accelerator keys
        {
            if ( bShowShortcuts )
                RetrieveShortcuts( m_aMenuItemHandlerVector );

            std::vector< MenuItemHandler* >::iterator p;
            for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
            {
                MenuItemHandler* pMenuItemHandler = *p;

                if ( !bShowShortcuts )
                {
                    pMenu->SetAccelKey( pMenuItemHandler->nItemId, vcl::KeyCode() );
                }
                else if ( pMenuItemHandler->aMenuItemURL == aCmdHelpIndex )
                {
                    // Set key code, workaround for hard-coded shortcut F1 mapped to .uno:HelpIndex
                    // Only non-popup menu items can have a short-cut
                    vcl::KeyCode aKeyCode( KEY_F1 );
                    pMenu->SetAccelKey( pMenuItemHandler->nItemId, aKeyCode );
                }
                else if ( pMenu->GetPopupMenu( pMenuItemHandler->nItemId ) == nullptr )
                    pMenu->SetAccelKey( pMenuItemHandler->nItemId, pMenuItemHandler->aKeyCode );
            }
        }

        URL aTargetURL;

        // Use provided dispatch provider => fallback to frame as dispatch provider
        Reference< XDispatchProvider > xDispatchProvider;
        if ( m_xDispatchProvider.is() )
            xDispatchProvider = m_xDispatchProvider;
        else
            xDispatchProvider.set( m_xFrame, UNO_QUERY );

        if ( xDispatchProvider.is() )
        {
            vcl::KeyCode      aEmptyKeyCode;
            SvtCommandOptions aCmdOptions;
            std::vector< MenuItemHandler* >::iterator p;
            for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
            {
                MenuItemHandler* pMenuItemHandler = *p;
                if ( pMenuItemHandler )
                {
                    if ( !pMenuItemHandler->xMenuItemDispatch.is() &&
                         !pMenuItemHandler->xSubMenuManager.is()      )
                    {
                        // There is no dispatch mechanism for the special window list menu items,
                        // because they are handled directly through XFrame->activate!!!
                        // Don't update dispatches for special file menu items.
                        if ( !(( pMenuItemHandler->nItemId >= START_ITEMID_WINDOWLIST &&
                                 pMenuItemHandler->nItemId < END_ITEMID_WINDOWLIST )))
                        {
                            Reference< XDispatch > xMenuItemDispatch;

                            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;

                            m_xURLTransformer->parseStrict( aTargetURL );

                            if ( bHasDisabledEntries )
                            {
                                if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aTargetURL.Path ))
                                    pMenu->HideItem( pMenuItemHandler->nItemId );
                            }

                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

                            bool bPopupMenu( false );
                            if ( !pMenuItemHandler->xPopupMenuController.is() &&
                                 m_xPopupMenuControllerFactory->hasController( pMenuItemHandler->aMenuItemURL, m_aModuleIdentifier ) )
                            {
                                if( xMenuItemDispatch.is() || pMenuItemHandler->aMenuItemURL != ".uno:RecentFileList" )
                                    bPopupMenu = CreatePopupMenuController( pMenuItemHandler );
                            }
                            else if ( pMenuItemHandler->xPopupMenuController.is() )
                            {
                                // Force update of popup menu
                                pMenuItemHandler->xPopupMenuController->updatePopupMenu();
                                bPopupMenu = true;
                                if (PopupMenu*  pThisPopup = pMenu->GetPopupMenu( pMenuItemHandler->nItemId ))
                                    pMenu->EnableItem( pMenuItemHandler->nItemId, pThisPopup->GetItemCount() != 0 );
                            }
                            lcl_CheckForChildren(pMenu, pMenuItemHandler->nItemId);

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;

                                if ( !bPopupMenu )
                                {
                                    xMenuItemDispatch->addStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                                    if ( !m_bHasMenuBar )
                                        xMenuItemDispatch->removeStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                                }
                            }
                            else if ( !bPopupMenu )
                                pMenu->EnableItem( pMenuItemHandler->nItemId, false );
                        }
                    }
                    else if ( pMenuItemHandler->xPopupMenuController.is() )
                    {
                        // Force update of popup menu
                        pMenuItemHandler->xPopupMenuController->updatePopupMenu();
                        lcl_CheckForChildren(pMenu, pMenuItemHandler->nItemId);
                    }
                    else if ( pMenuItemHandler->xMenuItemDispatch.is() )
                    {
                        // We need an update to reflect the current state
                        try
                        {
                            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                            m_xURLTransformer->parseStrict( aTargetURL );

                            pMenuItemHandler->xMenuItemDispatch->addStatusListener(
                                                                    static_cast< XStatusListener* >( this ), aTargetURL );
                            pMenuItemHandler->xMenuItemDispatch->removeStatusListener(
                                                                    static_cast< XStatusListener* >( this ), aTargetURL );
                        }
                        catch ( const Exception& )
                        {
                        }
                    }
                    else if ( pMenuItemHandler->xSubMenuManager.is() )
                        lcl_CheckForChildren(pMenu, pMenuItemHandler->nItemId);
                }
            }
        }
    }

    return true;
}

IMPL_LINK_TYPED( MenuBarManager, Deactivate, Menu *, pMenu, bool )
{
    if ( pMenu == m_pVCLMenu )
    {
        m_bActive = false;
        if ( pMenu->IsMenuBar() && m_xDeferedItemContainer.is() )
        {
            // Start timer to handle settings asynchronous
            // Changing the menu inside this handler leads to
            // a crash under X!
            m_aAsyncSettingsTimer.SetTimeoutHdl(LINK(this, MenuBarManager, AsyncSettingsHdl));
            m_aAsyncSettingsTimer.SetTimeout(10);
            m_aAsyncSettingsTimer.Start();
        }
    }

    return true;
}

IMPL_LINK_NOARG_TYPED( MenuBarManager, AsyncSettingsHdl, Timer*, void)
{
    SolarMutexGuard g;
    Reference< XInterface > xSelfHold(
        static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY_THROW );

    m_aAsyncSettingsTimer.Stop();
    if ( !m_bActive && m_xDeferedItemContainer.is() )
    {
        SetItemContainer( m_xDeferedItemContainer );
        m_xDeferedItemContainer.clear();
    }
}

IMPL_LINK_TYPED( MenuBarManager, Select, Menu *, pMenu, bool )
{
    URL                     aTargetURL;
    Sequence<PropertyValue> aArgs;
    Reference< XDispatch >  xDispatch;

    {
        SolarMutexGuard g;

        sal_uInt16 nCurItemId = pMenu->GetCurItemId();
        sal_uInt16 nCurPos    = pMenu->GetItemPos( nCurItemId );
        if ( pMenu == m_pVCLMenu &&
             pMenu->GetItemType( nCurPos ) != MenuItemType::SEPARATOR )
        {
            if ( nCurItemId >= START_ITEMID_WINDOWLIST &&
                 nCurItemId <= END_ITEMID_WINDOWLIST )
            {
                // window list menu item selected

                Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );

                sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
                Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
                sal_Int32 nCount = xList->getCount();
                for ( sal_Int32 i=0; i<nCount; ++i )
                {
                    Reference< XFrame > xFrame;
                    xList->getByIndex(i) >>= xFrame;
                    if ( xFrame.is() && nTaskId == nCurItemId )
                    {
                        vcl::Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                        pWin->GrabFocus();
                        pWin->ToTop( ToTopFlags::RestoreWhenMin );
                        break;
                    }

                    nTaskId++;
                }
            }
            else
            {
                MenuItemHandler* pMenuItemHandler = GetMenuItemHandler( nCurItemId );
                if ( pMenuItemHandler && pMenuItemHandler->xMenuItemDispatch.is() )
                {
                    aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                    m_xURLTransformer->parseStrict( aTargetURL );

                    if ( m_bIsBookmarkMenu )
                    {
                        // bookmark menu item selected
                        aArgs.realloc( 1 );
                        aArgs[0].Name = "Referer";
                        aArgs[0].Value <<= OUString( "private:user" );
                    }

                    xDispatch = pMenuItemHandler->xMenuItemDispatch;
                }
            }
        }
    }

    if ( xDispatch.is() )
    {
        SolarMutexReleaser aReleaser;
        xDispatch->dispatch( aTargetURL, aArgs );
    }

    if ( !m_bHasMenuBar )
        // Standalone (non-native) popup menu doesn't fire deactivate event
        // in this case, so we have to reset the active flag here.
        m_bActive = false;

    return true;
}

bool MenuBarManager::MustBeHidden( PopupMenu* pPopupMenu, const Reference< XURLTransformer >& rTransformer )
{
    if ( pPopupMenu )
    {
        URL               aTargetURL;
        SvtCommandOptions aCmdOptions;

        sal_uInt16 nCount = pPopupMenu->GetItemCount();
        sal_uInt16 nHideCount( 0 );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            sal_uInt16 nId = pPopupMenu->GetItemId( i );
            if ( nId > 0 )
            {
                PopupMenu* pSubPopupMenu = pPopupMenu->GetPopupMenu( nId );
                if ( pSubPopupMenu )
                {
                    if ( MustBeHidden( pSubPopupMenu, rTransformer ))
                    {
                        pPopupMenu->HideItem( nId );
                        ++nHideCount;
                    }
                }
                else
                {
                    aTargetURL.Complete = pPopupMenu->GetItemCommand( nId );
                    rTransformer->parseStrict( aTargetURL );

                    if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aTargetURL.Path ))
                        ++nHideCount;
                }
            }
            else
                ++nHideCount;
        }

        return ( nCount == nHideCount );
    }

    return true;
}

OUString MenuBarManager::RetrieveLabelFromCommand(const OUString& rCmdURL)
{
    if ( !m_bHasMenuBar )
    {
        // This is a context menu, prefer "PopupLabel" over "Label".
        return vcl::CommandInfoProvider::Instance().GetPopupLabelForCommand(rCmdURL, m_xFrame);
    }
    return vcl::CommandInfoProvider::Instance().GetMenuLabelForCommand(rCmdURL, m_xFrame);
}

bool MenuBarManager::CreatePopupMenuController( MenuItemHandler* pMenuItemHandler )
{
    OUString aItemCommand( pMenuItemHandler->aMenuItemURL );

    // Try instanciate a popup menu controller. It is stored in the menu item handler.
    if ( !m_xPopupMenuControllerFactory.is() )
        return false;

    Sequence< Any > aSeq( 2 );
    PropertyValue aPropValue;

    aPropValue.Name         = "ModuleIdentifier";
    aPropValue.Value      <<= m_aModuleIdentifier;
    aSeq[0] <<= aPropValue;
    aPropValue.Name         = "Frame";
    aPropValue.Value      <<= m_xFrame;
    aSeq[1] <<= aPropValue;

    Reference< XPopupMenuController > xPopupMenuController(
                                            m_xPopupMenuControllerFactory->createInstanceWithArgumentsAndContext(
                                                aItemCommand,
                                                aSeq,
                                                m_xContext ),
                                            UNO_QUERY );

    if ( xPopupMenuController.is() )
    {
        // Provide our awt popup menu to the popup menu controller
        pMenuItemHandler->xPopupMenuController = xPopupMenuController;
        xPopupMenuController->setPopupMenu( pMenuItemHandler->xPopupMenu );
        return true;
    }

    return false;
}

void MenuBarManager::FillMenuManager( Menu* pMenu, const Reference< XFrame >& rFrame, const Reference< XDispatchProvider >& rDispatchProvider, const OUString& rModuleIdentifier, bool bDelete, bool bDeleteChildren )
{
    m_xFrame            = rFrame;
    m_bActive           = false;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_bIsBookmarkMenu   = false;
    m_xDispatchProvider = rDispatchProvider;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();
    m_bRetrieveImages   = false;

    // Add root as ui configuration listener
    RetrieveImageManagers();

    if ( pMenu->IsMenuBar() && rFrame.is() )
    {
        // First merge all addon popup menus into our structure
        sal_uInt16 nPos = 0;
        for ( nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            sal_uInt16          nItemId  = pMenu->GetItemId( nPos );
            OUString aCommand = pMenu->GetItemCommand( nItemId );
            if ( nItemId == SID_MDIWINDOWLIST || aCommand == aSpecialWindowCommand ||
                 nItemId == SID_HELPMENU || aCommand == aCmdHelpMenu )
            {
                // Retrieve addon popup menus and add them to our menu bar
                framework::AddonMenuManager::MergeAddonPopupMenus( rFrame, nPos, static_cast<MenuBar *>(pMenu), m_xContext );
                break;
            }
        }

        // Merge the Add-Ons help menu items into the Office help menu
        framework::AddonMenuManager::MergeAddonHelpMenu( rFrame, static_cast<MenuBar *>(pMenu), m_xContext );
    }

    bool    bAccessibilityEnabled( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() );
    sal_uInt16 nItemCount = pMenu->GetItemCount();
    OUString aItemCommand;
    m_aMenuItemHandlerVector.reserve(nItemCount);
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = FillItemCommand(aItemCommand,pMenu, i );

        // Command can be just an alias to another command.
        OUString aRealCommand = vcl::CommandInfoProvider::Instance().GetRealCommandForCommand( aItemCommand, m_xFrame );
        if ( !aRealCommand.isEmpty() )
            aItemCommand = aRealCommand;

        // Set module identifier when provided from outside
        if ( !rModuleIdentifier.isEmpty() )
        {
            m_aModuleIdentifier = rModuleIdentifier;
            m_bModuleIdentified = true;
        }

        if (( pMenu->IsMenuBar() || bAccessibilityEnabled ) &&
            ( pMenu->GetItemText( nItemId ).isEmpty() ))
        {
            if ( !aItemCommand.isEmpty() )
                pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aItemCommand ));
        }

        Reference< XDispatch > xDispatch;
        Reference< XStatusListener > xStatusListener;
        VclPtr<PopupMenu> pPopup = pMenu->GetPopupMenu( nItemId );
        bool bItemShowMenuImages = m_bShowMenuImages;
        // overwrite the show icons on menu option?
        if (!bItemShowMenuImages)
        {
            MenuItemBits nBits =  pMenu->GetItemBits( nItemId );
            bItemShowMenuImages = ( ( nBits & MenuItemBits::ICON ) == MenuItemBits::ICON );
        }
        if ( pPopup )
        {
            // Retrieve module identifier from Help Command entry
            OUString aModuleIdentifier( rModuleIdentifier );
            if (!pMenu->GetHelpCommand(nItemId).isEmpty())
            {
                aModuleIdentifier = pMenu->GetHelpCommand( nItemId );
                pMenu->SetHelpCommand( nItemId, "" );
            }

            if ( m_xPopupMenuControllerFactory.is() &&
                 pPopup->GetItemCount() == 0 &&
                 m_xPopupMenuControllerFactory->hasController( aItemCommand, m_aModuleIdentifier )
                  )
            {
                // Check if we have to create a popup menu for a uno based popup menu controller.
                // We have to set an empty popup menu into our menu structure so the controller also
                // works with inplace OLE. Remove old dummy popup menu!
                MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );
                VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                PopupMenu* pNewPopupMenu = static_cast<PopupMenu *>(pVCLXPopupMenu->GetMenu());
                pMenu->SetPopupMenu( nItemId, pNewPopupMenu );
                pItemHandler->xPopupMenu.set( static_cast<OWeakObject *>(pVCLXPopupMenu), UNO_QUERY );
                pItemHandler->aMenuItemURL = aItemCommand;
                m_aMenuItemHandlerVector.push_back( pItemHandler );
                pPopup.disposeAndClear();

                if ( bAccessibilityEnabled )
                {
                    if ( CreatePopupMenuController( pItemHandler ))
                        pItemHandler->xPopupMenuController->updatePopupMenu();
                }
                lcl_CheckForChildren(pMenu, nItemId);
            }
            else if ( aItemCommand.startsWith( ADDONSPOPUPMENU_URL_PREFIX_STR ) )
            {
                // A special addon popup menu, must be created with a different ctor
                MenuBarManager* pSubMenuManager = new MenuBarManager( m_xContext, m_xFrame, m_xURLTransformer,
                                                          pPopup, bDeleteChildren, bDeleteChildren, true );
                AddMenu(pSubMenuManager,aItemCommand,nItemId);
            }
            else
            {
                Reference< XDispatchProvider > xPopupMenuDispatchProvider( rDispatchProvider );

                // Retrieve possible attributes struct
                MenuAttributes* pAttributes = reinterpret_cast<MenuAttributes *>(pMenu->GetUserValue( nItemId ));
                if ( pAttributes )
                    xPopupMenuDispatchProvider = pAttributes->xDispatchProvider;

                // Check if this is the help menu. Add menu item if needed
                if ( nItemId == SID_HELPMENU || aItemCommand == aSlotHelpMenu || aItemCommand == aCmdHelpMenu )
                {
                    // Check if this is the help menu. Add menu item if needed
                    CheckAndAddMenuExtension( pPopup );
                }
                else if (( aItemCommand == aSlotSpecialToolsMenu || aItemCommand == aCmdToolsMenu ) &&
                        AddonMenuManager::HasAddonMenuElements() )
                {
                    // Create addon popup menu if there exist elements and this is the tools popup menu
                    VclPtr<AddonMenu> pSubMenu = AddonMenuManager::CreateAddonMenu(rFrame, m_xContext);
                    if ( pSubMenu && ( pSubMenu->GetItemCount() > 0 ))
                    {
                        sal_uInt16 nCount = 0;
                        if ( pPopup->GetItemType( nCount-1 ) != MenuItemType::SEPARATOR )
                            pPopup->InsertSeparator();

                        // Use resource to load popup menu title
                        OUString aAddonsStrRes(FWK_RESSTR(STR_MENU_ADDONS));
                        pPopup->InsertItem( ITEMID_ADDONLIST, aAddonsStrRes );
                        pPopup->SetPopupMenu( ITEMID_ADDONLIST, pSubMenu );

                        // Set item command for popup menu to enable it for GetImageFromURL
                        OUString aNewItemCommand = "slot:" + OUString::number( ITEMID_ADDONLIST );
                        pPopup->SetItemCommand( ITEMID_ADDONLIST, aNewItemCommand );
                    }
                    else
                        pSubMenu.disposeAndClear();
                }

                if ( nItemId == ITEMID_ADDONLIST )
                {
                    AddonMenu* pSubMenu = dynamic_cast< AddonMenu* >( pPopup.get() );
                    if ( pSubMenu )
                    {
                        MenuBarManager* pSubMenuManager = new MenuBarManager( m_xContext, m_xFrame, m_xURLTransformer,pSubMenu, true, false, false );
                        AddMenu(pSubMenuManager,aItemCommand,nItemId);
                        (pSubMenuManager->m_aMenuItemCommand).clear();

                        // Set image for the addon popup menu item
                        if ( bItemShowMenuImages && !pPopup->GetItemImage( ITEMID_ADDONLIST ))
                        {
                            Image aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand( aItemCommand, false, rFrame );
                            if ( !!aImage )
                                   pPopup->SetItemImage( ITEMID_ADDONLIST, aImage );
                        }
                    }
                }
                else
                {
                    MenuBarManager* pSubMenuMgr = new MenuBarManager( m_xContext, rFrame, m_xURLTransformer,rDispatchProvider, aModuleIdentifier, pPopup, bDeleteChildren, bDeleteChildren, m_bHasMenuBar );
                    AddMenu(pSubMenuMgr,aItemCommand,nItemId);
                }
            }
        }
        else if ( pMenu->GetItemType( i ) != MenuItemType::SEPARATOR )
        {
            if ( bItemShowMenuImages )
            {
                if ( AddonMenuManager::IsAddonMenuId( nItemId ))
                {
                    // Add-Ons uses images from different places
                    Image           aImage;
                    OUString   aImageId;

                    MenuAttributes* pMenuAttributes =
                        reinterpret_cast<MenuAttributes*>(pMenu->GetUserValue( nItemId ));

                    if ( pMenuAttributes && !pMenuAttributes->aImageId.isEmpty() )
                    {
                        // Retrieve image id from menu attributes
                        aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand( aImageId, false, m_xFrame );
                    }

                    if ( !aImage )
                    {
                        aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand( aItemCommand, false, m_xFrame );
                        if ( !aImage )
                            aImage = AddonsOptions().GetImageFromURL( aItemCommand, false );
                    }

                    if ( !!aImage )
                        pMenu->SetItemImage( nItemId, aImage );
                    else
                        m_bRetrieveImages = true;
                }
                m_bRetrieveImages = true;
            }

            MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );
            pItemHandler->aMenuItemURL = aItemCommand;

            if ( m_xPopupMenuControllerFactory.is() &&
                 m_xPopupMenuControllerFactory->hasController( aItemCommand, m_aModuleIdentifier ) )
            {
                // Check if we have to create a popup menu for a uno based popup menu controller.
                // We have to set an empty popup menu into our menu structure so the controller also
                // works with inplace OLE.
                VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                PopupMenu* pPopupMenu = static_cast<PopupMenu *>(pVCLXPopupMenu->GetMenu());
                pMenu->SetPopupMenu( pItemHandler->nItemId, pPopupMenu );
                pItemHandler->xPopupMenu.set( static_cast<OWeakObject *>(pVCLXPopupMenu), UNO_QUERY );

                if ( bAccessibilityEnabled && CreatePopupMenuController( pItemHandler ) )
                {
                    pItemHandler->xPopupMenuController->updatePopupMenu();
                }

                lcl_CheckForChildren(pMenu, pItemHandler->nItemId);
            }

            m_aMenuItemHandlerVector.push_back( pItemHandler );
        }
    }

    if ( m_bHasMenuBar && bAccessibilityEnabled )
    {
        RetrieveShortcuts( m_aMenuItemHandlerVector );
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pMenuItemHandler = *p;

            // Set key code, workaround for hard-coded shortcut F1 mapped to .uno:HelpIndex
            // Only non-popup menu items can have a short-cut
            if ( pMenuItemHandler->aMenuItemURL == aCmdHelpIndex )
            {
                vcl::KeyCode aKeyCode( KEY_F1 );
                pMenu->SetAccelKey( pMenuItemHandler->nItemId, aKeyCode );
            }
            else if ( pMenu->GetPopupMenu( pMenuItemHandler->nItemId ) == nullptr )
                pMenu->SetAccelKey( pMenuItemHandler->nItemId, pMenuItemHandler->aKeyCode );
        }
    }

    SetHdl();
}

void MenuBarManager::impl_RetrieveShortcutsFromConfiguration(
    const Reference< XAcceleratorConfiguration >& rAccelCfg,
    const Sequence< OUString >& rCommands,
    std::vector< MenuItemHandler* >& aMenuShortCuts )
{
    if ( rAccelCfg.is() )
    {
        try
        {
            css::awt::KeyEvent aKeyEvent;
            Sequence< Any > aSeqKeyCode = rAccelCfg->getPreferredKeyEventsForCommandList( rCommands );
            for ( sal_Int32 i = 0; i < aSeqKeyCode.getLength(); i++ )
            {
                if ( aSeqKeyCode[i] >>= aKeyEvent )
                    aMenuShortCuts[i]->aKeyCode = svt::AcceleratorExecute::st_AWTKey2VCLKey( aKeyEvent );
            }
        }
        catch ( const IllegalArgumentException& )
        {
        }
    }
}

void MenuBarManager::RetrieveShortcuts( std::vector< MenuItemHandler* >& aMenuShortCuts )
{
    if ( !m_bModuleIdentified )
    {
        m_bModuleIdentified = true;
        Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );

        try
        {
            m_aModuleIdentifier = xModuleManager->identify( m_xFrame );
        }
        catch( const Exception& )
        {
        }
    }

    if ( m_bModuleIdentified )
    {
        Reference< XAcceleratorConfiguration > xDocAccelCfg( m_xDocAcceleratorManager );
        Reference< XAcceleratorConfiguration > xModuleAccelCfg( m_xModuleAcceleratorManager );
        Reference< XAcceleratorConfiguration > xGlobalAccelCfg( m_xGlobalAcceleratorManager );

        if ( !m_bAcceleratorCfg )
        {
            // Retrieve references on demand
            m_bAcceleratorCfg = true;
            if ( !xDocAccelCfg.is() )
            {
                Reference< XController > xController = m_xFrame->getController();
                Reference< XModel > xModel;
                if ( xController.is() )
                {
                    xModel = xController->getModel();
                    if ( xModel.is() )
                    {
                        Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
                        if ( xSupplier.is() )
                        {
                            Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
                            if ( xDocUICfgMgr.is() )
                            {
                                xDocAccelCfg = xDocUICfgMgr->getShortCutManager();
                                m_xDocAcceleratorManager = xDocAccelCfg;
                            }
                        }
                    }
                }
            }

            if ( !xModuleAccelCfg.is() )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier =
                    theModuleUIConfigurationManagerSupplier::get( m_xContext );
                try
                {
                    Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
                    if ( xUICfgMgr.is() )
                    {
                        xModuleAccelCfg = xUICfgMgr->getShortCutManager();
                        m_xModuleAcceleratorManager = xModuleAccelCfg;
                    }
                }
                catch ( const RuntimeException& )
                {
                    throw;
                }
                catch ( const Exception& )
                {
                }
            }

            if ( !xGlobalAccelCfg.is() ) try
            {
                xGlobalAccelCfg = GlobalAcceleratorConfiguration::create( m_xContext );
                m_xGlobalAcceleratorManager = xGlobalAccelCfg;
            }
            catch ( const css::uno::DeploymentException& )
            {
                SAL_WARN("fwk.uielement", "GlobalAcceleratorConfiguration"
                        " not available. This should happen only on mobile platforms.");
            }
        }

        vcl::KeyCode aEmptyKeyCode;
        Sequence< OUString > aSeq( aMenuShortCuts.size() );
        const sal_uInt32 nCount = aMenuShortCuts.size();
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            rtl::OUString aItemURL = aMenuShortCuts[i]->aMenuItemURL;
            if( aItemURL.isEmpty() && aMenuShortCuts[i]->xSubMenuManager.is())
                aItemURL = "-"; // tdf#99527 prevent throw in case of empty commands
            aSeq[i] = aItemURL;
            aMenuShortCuts[i]->aKeyCode = aEmptyKeyCode;
        }

        if ( m_xGlobalAcceleratorManager.is() )
            impl_RetrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xModuleAcceleratorManager.is() )
            impl_RetrieveShortcutsFromConfiguration( xModuleAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xDocAcceleratorManager.is() )
            impl_RetrieveShortcutsFromConfiguration( xDocAccelCfg, aSeq, aMenuShortCuts );
    }
}

void MenuBarManager::RetrieveImageManagers()
{
    if ( !m_xDocImageManager.is() )
    {
        Reference< XController > xController = m_xFrame->getController();
        Reference< XModel > xModel;
        if ( xController.is() )
        {
            xModel = xController->getModel();
            if ( xModel.is() )
            {
                Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
                if ( xSupplier.is() )
                {
                    Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
                    m_xDocImageManager.set( xDocUICfgMgr->getImageManager(), UNO_QUERY );
                    m_xDocImageManager->addConfigurationListener(
                                            Reference< XUIConfigurationListener >(
                                                static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
            }
        }
    }

    Reference< XModuleManager2 > xModuleManager;
    if ( m_aModuleIdentifier.isEmpty() )
        xModuleManager.set( ModuleManager::create( m_xContext ) );

    try
    {
        if ( xModuleManager.is() )
            m_aModuleIdentifier = xModuleManager->identify( Reference< XInterface >( m_xFrame, UNO_QUERY ) );
    }
    catch( const Exception& )
    {
    }

    if ( !m_xModuleImageManager.is() )
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier =
            theModuleUIConfigurationManagerSupplier::get( m_xContext );
        Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
        m_xModuleImageManager.set( xUICfgMgr->getImageManager(), UNO_QUERY );
        m_xModuleImageManager->addConfigurationListener( Reference< XUIConfigurationListener >(
                                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

void MenuBarManager::FillMenuWithConfiguration(
    sal_uInt16&                         nId,
    Menu*                               pMenu,
    const OUString&              rModuleIdentifier,
    const Reference< XIndexAccess >&    rItemContainer,
    const Reference< XURLTransformer >& rTransformer )
{
    Reference< XDispatchProvider > xEmptyDispatchProvider;
    MenuBarManager::FillMenu( nId, pMenu, rModuleIdentifier, rItemContainer, xEmptyDispatchProvider );

    // Merge add-on menu entries into the menu bar
    MenuBarManager::MergeAddonMenus( pMenu,
                                     AddonsOptions().GetMergeMenuInstructions(),
                                     rModuleIdentifier );

    bool bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );
    if ( bHasDisabledEntries )
    {
        sal_uInt16 nCount = pMenu->GetItemCount();
        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            sal_uInt16 nID = pMenu->GetItemId( i );
            if ( nID > 0 )
            {
                PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nID );
                if ( pPopupMenu )
                {
                    if ( MustBeHidden( pPopupMenu, rTransformer ))
                        pMenu->HideItem( nId );
                }
            }
        }
    }
}

void MenuBarManager::FillMenu(
    sal_uInt16&                           nId,
    Menu*                                 pMenu,
    const OUString&                  rModuleIdentifier,
    const Reference< XIndexAccess >&      rItemContainer,
    const Reference< XDispatchProvider >& rDispatchProvider )
{
    // Fill menu bar with container contents
     for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >       aProp;
        OUString                   aCommandURL;
        OUString                   aLabel;
        OUString                   aHelpURL;
        OUString                   aModuleIdentifier( rModuleIdentifier );
        sal_uInt16                      nType = 0;
        Reference< XIndexAccess >       xIndexContainer;
        Reference< XDispatchProvider >  xDispatchProvider( rDispatchProvider );
        sal_Int16 nStyle = 0;
        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                bool bShow = true;
                bool bEnabled = true;

                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    OUString aPropName = aProp[i].Name;
                    if ( aPropName == "CommandURL" )
                        aProp[i].Value >>= aCommandURL;
                    else if ( aPropName == "HelpURL" )
                        aProp[i].Value >>= aHelpURL;
                    else if ( aPropName == "ItemDescriptorContainer" )
                        aProp[i].Value >>= xIndexContainer;
                    else if ( aPropName == "Label" )
                        aProp[i].Value >>= aLabel;
                    else if ( aPropName == "Type" )
                        aProp[i].Value >>= nType;
                    else if ( aPropName == "ModuleIdentifier" )
                        aProp[i].Value >>= aModuleIdentifier;
                    else if ( aPropName == "DispatchProvider" )
                        aProp[i].Value >>= xDispatchProvider;
                    else if ( aPropName == "Style" )
                        aProp[i].Value >>= nStyle;
                    else if ( aPropName == "IsVisible" )
                        aProp[i].Value >>= bShow;
                    else if ( aPropName == "Enabled" )
                        aProp[i].Value >>= bEnabled;
                }

                if ( nType == css::ui::ItemType::DEFAULT )
                {
                    pMenu->InsertItem( nId, aLabel );
                    pMenu->SetItemCommand( nId, aCommandURL );

                    if ( nStyle )
                    {
                        MenuItemBits nBits = pMenu->GetItemBits( nId );
                        if ( nStyle & css::ui::ItemStyle::ICON )
                           nBits |= MenuItemBits::ICON;
                        if ( nStyle & css::ui::ItemStyle::TEXT )
                           nBits |= MenuItemBits::TEXT;
                        if ( nStyle & css::ui::ItemStyle::RADIO_CHECK )
                           nBits |= MenuItemBits::RADIOCHECK;
                        pMenu->SetItemBits( nId, nBits );
                    }

                    if ( !bShow )
                        pMenu->HideItem( nId );

                    if ( !bEnabled)
                        pMenu->EnableItem( nId, false );

                    if ( xIndexContainer.is() )
                    {
                        VclPtr<PopupMenu> pNewPopupMenu = VclPtr<PopupMenu>::Create();
                        pMenu->SetPopupMenu( nId, pNewPopupMenu );

                        if ( xDispatchProvider.is() )
                        {
                            // Use attributes struct to transport special dispatch provider
                            sal_uIntPtr nAttributePtr = MenuAttributes::CreateAttribute(xDispatchProvider);
                            pMenu->SetUserValue(nId, nAttributePtr, MenuAttributes::ReleaseAttribute);
                        }

                        // Use help command to transport module identifier
                        if ( !aModuleIdentifier.isEmpty() )
                            pMenu->SetHelpCommand( nId, aModuleIdentifier );

                        ++nId;
                        FillMenu( nId, pNewPopupMenu, aModuleIdentifier, xIndexContainer, xDispatchProvider );
                    }
                    else
                        ++nId;
                }
                else
                {
                    pMenu->InsertSeparator();
                    ++nId;
                }
            }
        }
        catch ( const IndexOutOfBoundsException& )
        {
            break;
        }
    }
}

void MenuBarManager::MergeAddonMenus(
    Menu* pMenuBar,
    const MergeMenuInstructionContainer& aMergeInstructionContainer,
    const OUString& rModuleIdentifier )
{
    // set start value for the item ID for the new addon menu items
    sal_uInt16 nItemId = ADDONMENU_MERGE_ITEMID_START;

    const sal_uInt32 nCount = aMergeInstructionContainer.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        const MergeMenuInstruction& rMergeInstruction = aMergeInstructionContainer[i];

        if ( MenuBarMerger::IsCorrectContext( rMergeInstruction.aMergeContext, rModuleIdentifier ))
        {
            ::std::vector< OUString > aMergePath;

            // retrieve the merge path from the merge point string
            MenuBarMerger::RetrieveReferencePath( rMergeInstruction.aMergePoint, aMergePath );

            // convert the sequence/sequence property value to a more convenient vector<>
            AddonMenuContainer aMergeMenuItems;
            MenuBarMerger::GetSubMenu( rMergeInstruction.aMergeMenu, aMergeMenuItems );

            // try to find the reference point for our merge operation
            Menu* pMenu = pMenuBar;
            ReferencePathInfo aResult = MenuBarMerger::FindReferencePath( aMergePath, pMenu );

            if ( aResult.eResult == RP_OK )
            {
                // normal merge operation
                MenuBarMerger::ProcessMergeOperation( aResult.pPopupMenu,
                                                      aResult.nPos,
                                                      nItemId,
                                                      rMergeInstruction.aMergeCommand,
                                                      rMergeInstruction.aMergeCommandParameter,
                                                      rModuleIdentifier,
                                                      aMergeMenuItems );
            }
            else
            {
                // fallback
                MenuBarMerger::ProcessFallbackOperation( aResult,
                                                         nItemId,
                                                         rMergeInstruction.aMergeCommand,
                                                         rMergeInstruction.aMergeFallback,
                                                         aMergePath,
                                                         rModuleIdentifier,
                                                         aMergeMenuItems );
            }
        }
    }
}

void MenuBarManager::SetItemContainer( const Reference< XIndexAccess >& rItemContainer )
{
    SolarMutexGuard aSolarMutexGuard;

    Reference< XFrame > xFrame = m_xFrame;

    if ( !m_bModuleIdentified )
    {
        m_bModuleIdentified = true;
        Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );

        try
        {
            m_aModuleIdentifier = xModuleManager->identify( xFrame );
        }
        catch( const Exception& )
        {
        }
    }

    // Clear MenuBarManager structures
    {
        // Check active state as we cannot change our VCL menu during activation by the user
        if ( m_bActive )
        {
            m_xDeferedItemContainer = rItemContainer;
            return;
        }

        RemoveListener();
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
            pItemHandler->xSubMenuManager.clear();
            delete pItemHandler;
        }
        m_aMenuItemHandlerVector.clear();

        // Remove top-level parts
        m_pVCLMenu->Clear();

        sal_uInt16          nId = 1;

        // Fill menu bar with container contents
        FillMenuWithConfiguration( nId, m_pVCLMenu, m_aModuleIdentifier, rItemContainer, m_xURLTransformer );

        // Refill menu manager again
        Reference< XDispatchProvider > xDispatchProvider;
        FillMenuManager( m_pVCLMenu, xFrame, xDispatchProvider, m_aModuleIdentifier, false, true );

        // add itself as frame action listener
        m_xFrame->addFrameActionListener( Reference< XFrameActionListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

void MenuBarManager::GetPopupController( PopupControllerCache& rPopupController )
{

    SolarMutexGuard aSolarMutexGuard;

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->xPopupMenuController.is() )
        {
            Reference< XDispatchProvider > xDispatchProvider( pItemHandler->xPopupMenuController, UNO_QUERY );

            PopupControllerEntry aPopupControllerEntry;
            aPopupControllerEntry.m_xDispatchProvider = xDispatchProvider;

            // Just use the main part of the URL for popup menu controllers
            sal_Int32     nQueryPart( 0 );
            sal_Int32     nSchemePart( 0 );
            OUString aMainURL( "vnd.sun.star.popup:" );
            OUString aMenuURL( pItemHandler->aMenuItemURL );

            nSchemePart = aMenuURL.indexOf( ':' );
            if (( nSchemePart > 0 ) &&
                ( aMenuURL.getLength() > ( nSchemePart+1 )))
            {
                nQueryPart  = aMenuURL.indexOf( '?', nSchemePart );
                if ( nQueryPart > 0 )
                    aMainURL += aMenuURL.copy( nSchemePart, nQueryPart-nSchemePart );
                else if ( nQueryPart == -1 )
                    aMainURL += aMenuURL.copy( nSchemePart+1 );

                rPopupController.insert( PopupControllerCache::value_type(
                                           aMainURL, aPopupControllerEntry ));
            }
        }
        if ( pItemHandler->xSubMenuManager.is() )
        {
            MenuBarManager* pMenuBarManager = static_cast<MenuBarManager*>(pItemHandler->xSubMenuManager.get());
            if ( pMenuBarManager )
                pMenuBarManager->GetPopupController( rPopupController );
        }
    }
}

void MenuBarManager::AddMenu(MenuBarManager* pSubMenuManager,const OUString& _sItemCommand,sal_uInt16 _nItemId)
{
    Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
    m_xFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

    // store menu item command as we later have to know which menu is active (see Activate handler)
    pSubMenuManager->m_aMenuItemCommand = _sItemCommand;
    Reference< XDispatch > xDispatch;
    MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                _nItemId,
                                                xSubMenuManager,
                                                xDispatch );
    pMenuItemHandler->aMenuItemURL = _sItemCommand;
    m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
}

sal_uInt16 MenuBarManager::FillItemCommand(OUString& _rItemCommand, Menu* _pMenu,sal_uInt16 _nIndex) const
{
    sal_uInt16 nItemId = _pMenu->GetItemId( _nIndex );

    _rItemCommand = _pMenu->GetItemCommand( nItemId );
    if ( _rItemCommand.isEmpty() )
    {
        _rItemCommand = "slot:" + OUString::number( nItemId );
        _pMenu->SetItemCommand( nItemId, _rItemCommand );
    }
    return nItemId;
}
void MenuBarManager::Init(const Reference< XFrame >& rFrame,Menu* pAddonMenu,bool bDelete,bool bDeleteChildren,bool _bHandlePopUp)
{
    m_bActive           = false;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pAddonMenu;
    m_xFrame            = rFrame;
    m_bIsBookmarkMenu   = true;
    m_bShowMenuImages   = true;

    OUString aModuleIdentifier;
    m_xPopupMenuControllerFactory = frame::thePopupMenuControllerFactory::get(
        ::comphelper::getProcessComponentContext());

    Reference< XStatusListener > xStatusListener;
    Reference< XDispatch > xDispatch;
    sal_uInt16 nItemCount = pAddonMenu->GetItemCount();
    OUString aItemCommand;
    m_aMenuItemHandlerVector.reserve(nItemCount);
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = FillItemCommand(aItemCommand,pAddonMenu, i );

        PopupMenu* pPopupMenu = pAddonMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            Reference< XDispatchProvider > xDispatchProvider;
            MenuBarManager* pSubMenuManager = new MenuBarManager( m_xContext, rFrame, m_xURLTransformer,xDispatchProvider, aModuleIdentifier, pPopupMenu, !_bHandlePopUp && bDeleteChildren, !_bHandlePopUp && bDeleteChildren );

            Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );

            // store menu item command as we later have to know which menu is active (see Activate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        xSubMenuManager,
                                                        xDispatch );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pAddonMenu->GetItemType( i ) != MenuItemType::SEPARATOR )
            {
                MenuAttributes* pAddonAttributes = reinterpret_cast<MenuAttributes *>(pAddonMenu->GetUserValue( nItemId ));
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );

                if ( pAddonAttributes )
                {
                    // read additional attributes from attributes struct and AddonMenu implementation will delete all attributes itself!!
                    pMenuItemHandler->aTargetFrame = pAddonAttributes->aTargetFrame;
                }

                pMenuItemHandler->aMenuItemURL = aItemCommand;
                if ( _bHandlePopUp )
                {
                    // Check if we have to create a popup menu for a uno based popup menu controller.
                    // We have to set an empty popup menu into our menu structure so the controller also
                    // works with inplace OLE.
                    if ( m_xPopupMenuControllerFactory.is() &&
                         m_xPopupMenuControllerFactory->hasController( aItemCommand, m_aModuleIdentifier ) )
                    {
                        VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                        PopupMenu* pCtlPopupMenu = static_cast<PopupMenu *>(pVCLXPopupMenu->GetMenu());
                        pAddonMenu->SetPopupMenu( pMenuItemHandler->nItemId, pCtlPopupMenu );
                        pMenuItemHandler->xPopupMenu.set( static_cast<OWeakObject *>(pVCLXPopupMenu), UNO_QUERY );

                    }
                }
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }

    SetHdl();
}

void MenuBarManager::SetHdl()
{
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));

    if ( !m_xURLTransformer.is() && m_xContext.is() )
        m_xURLTransformer.set( URLTransformer::create( m_xContext) );
}

void MenuBarManager::UpdateSpecialWindowMenu( Menu* pMenu,const Reference< XComponentContext >& xContext )
{
    // update window list
    ::std::vector< OUString > aNewWindowListVector;

    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );

    sal_uInt16  nActiveItemId = 0;
    sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

    Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
    Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
    sal_Int32 nFrameCount = xList->getCount();
    aNewWindowListVector.reserve(nFrameCount);
    for (sal_Int32 i=0; i<nFrameCount; ++i )
    {
        Reference< XFrame > xFrame;
        xList->getByIndex(i) >>= xFrame;

        if (xFrame.is())
        {
            if ( xFrame == xCurrentFrame )
                nActiveItemId = nItemId;

            vcl::Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
            OUString sWindowTitle;
            if ( pWin && pWin->IsVisible() )
                sWindowTitle = pWin->GetText();

            // tdf#101658 In case the frame is embedded somewhere, LO has no control over it.
            // So we just skip it.
            if ( sWindowTitle.isEmpty() )
                continue;

            aNewWindowListVector.push_back( sWindowTitle );
            ++nItemId;
        }
    }

    {
        SolarMutexGuard g;

        int nItemCount = pMenu->GetItemCount();

        if ( nItemCount > 0 )
        {
            // remove all old window list entries from menu
            sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
            for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                pMenu->RemoveItem( n );

            if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MenuItemType::SEPARATOR )
                pMenu->RemoveItem( pMenu->GetItemCount()-1 );
        }

        if ( !aNewWindowListVector.empty() )
        {
            // append new window list entries to menu
            pMenu->InsertSeparator();
            nItemId = START_ITEMID_WINDOWLIST;
            const sal_uInt32 nCount = aNewWindowListVector.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                pMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MenuItemBits::RADIOCHECK );
                if ( nItemId == nActiveItemId )
                    pMenu->CheckItem( nItemId );
                ++nItemId;
            }
        }
    }
}

void MenuBarManager::FillMenuImages(Reference< XFrame >& _xFrame, Menu* _pMenu,bool bShowMenuImages)
{
    AddonsOptions       aAddonOptions;

    for ( sal_uInt16 nPos = 0; nPos < _pMenu->GetItemCount(); nPos++ )
    {
        sal_uInt16 nId = _pMenu->GetItemId( nPos );
        if ( _pMenu->GetItemType( nPos ) != MenuItemType::SEPARATOR )
        {
            bool bTmpShowMenuImages( bShowMenuImages );
            // overwrite the show icons on menu option?
            if (!bTmpShowMenuImages)
            {
                MenuItemBits nBits =  _pMenu->GetItemBits( nId );
                bTmpShowMenuImages = ( ( nBits & MenuItemBits::ICON ) == MenuItemBits::ICON );
            }

            if ( bTmpShowMenuImages )
            {
                bool        bImageSet = false;
                OUString aImageId;

                ::framework::MenuAttributes* pMenuAttributes =
                    reinterpret_cast< ::framework::MenuAttributes*>(_pMenu->GetUserValue( nId ));

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                if ( !aImageId.isEmpty() )
                {
                    Image aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand(aImageId, false, _xFrame );
                    if ( !!aImage )
                    {
                        bImageSet = true;
                        _pMenu->SetItemImage( nId, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    OUString aMenuItemCommand = _pMenu->GetItemCommand( nId );
                    Image aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand(aMenuItemCommand, false, _xFrame );
                    if ( !aImage )
                        aImage = aAddonOptions.GetImageFromURL( aMenuItemCommand, false );

                    _pMenu->SetItemImage( nId, aImage );
                }
            }
            else
                _pMenu->SetItemImage( nId, Image() );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
