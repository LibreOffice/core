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

#include <uielement/menubarmanager.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/bmkmenu.hxx>
#include <framework/addonmenu.hxx>
#include <framework/imageproducer.hxx>
#include <threadhelp/resetableguard.hxx>
#include "framework/addonsoptions.hxx"
#include <classes/fwkresid.hxx>
#include <classes/menumanager.hxx>
#include <helper/mischelper.hxx>
#include <framework/menuextensionsupplier.hxx>
#include <classes/resource.hrc>
#include <services.h>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XStringWidth.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
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
#include <toolkit/unohlp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase1.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <rtl/logfile.hxx>
#include "svtools/miscopt.hxx"
#include <framework/addonmenu.hxx>
#include <uielement/menubarmerger.hxx>

// Be careful removing this "bad" construct. There are serious problems
// with #define STRICT and including windows.h. Changing this needs some
// redesign on other projects, too. Especially sal/main.h which defines
// HINSTANCE depending on STRCIT!!!!!!!!!!!!!!!
struct SystemMenuData
{
    unsigned long nSize;
    long          hMenu;
};

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

static const char ITEM_DESCRIPTOR_COMMANDURL[]        = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]           = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]         = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]             = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]              = "Type";
static const char ITEM_DESCRIPTOR_MODULEIDENTIFIER[]  = "ModuleIdentifier";
static const char ITEM_DESCRIPTOR_DISPATCHPROVIDER[]  = "DispatchProvider";
static const char ITEM_DESCRIPTOR_STYLE[]             = "Style";
static const char ITEM_DESCRIPTOR_ISVISIBLE[]         = "IsVisible";
static const char ITEM_DESCRIPTOR_ENABLED[]           = "Enabled";

static const sal_Int32 LEN_DESCRIPTOR_COMMANDURL       = 10;
static const sal_Int32 LEN_DESCRIPTOR_HELPURL          = 7;
static const sal_Int32 LEN_DESCRIPTOR_CONTAINER        = 23;
static const sal_Int32 LEN_DESCRIPTOR_LABEL            = 5;
static const sal_Int32 LEN_DESCRIPTOR_TYPE             = 4;
static const sal_Int32 LEN_DESCRIPTOR_MODULEIDENTIFIER = 16;
static const sal_Int32 LEN_DESCRIPTOR_DISPATCHPROVIDER = 16;
static const sal_Int32 LEN_DESCRIPTOR_STYLE            = 5;
static const sal_Int32 LEN_DESCRIPTOR_ISVISIBLE        = 9;
static const sal_Int32 LEN_DESCRIPTOR_ENABLED          = 7;

const sal_uInt16 ADDONMENU_MERGE_ITEMID_START = 1500;

class StringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw (RuntimeException)
        {
            return aString.getLength();
        }
};

namespace framework
{

// special menu ids/command ids for dynamic popup menus
#define SID_SFX_START           5000
#define SID_NEWDOCDIRECT        (SID_SFX_START + 537)
#define SID_AUTOPILOTMENU       (SID_SFX_START + 1381)
#define SID_PICKLIST            (SID_SFX_START + 510)
#define SID_MDIWINDOWLIST       (SID_SFX_START + 610)
#define SID_ADDONLIST           (SID_SFX_START + 1677)
#define SID_HELPMENU            (SID_SFX_START + 410)

#define SFX_REFERER_USER        "private:user"

#define aCmdHelpIndex ".uno:HelpIndex"
#define aCmdToolsMenu ".uno:ToolsMenu"
#define aCmdHelpMenu ".uno:HelpMenu"
#define aSlotHelpMenu "slot:5410"

#define aSpecialWindowMenu "window"
#define aSlotSpecialWindowMenu "slot:5610"
#define aSlotSpecialToolsMenu "slot:6677"

// special uno commands for window list
#define aSpecialWindowCommand ".uno:WindowList"

static sal_Int16 getImageTypeFromBools( sal_Bool bBig )
{
    sal_Int16 n( 0 );
    if ( bBig )
        n |= ::com::sun::star::ui::ImageType::SIZE_LARGE;
    return n;
}

MenuBarManager::MenuBarManager(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XFrame >& rFrame,
    const Reference< XURLTransformer >& _xURLTransformer,
    const Reference< XDispatchProvider >& rDispatchProvider,
    const rtl::OUString& rModuleIdentifier,
    Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
: ThreadHelpBase( &Application::GetSolarMutex() ), OWeakObject()
    , m_bDisposed( sal_False )
    , m_bRetrieveImages( sal_False )
    , m_bAcceleratorCfg( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , mxServiceFactory(xServiceFactory)
    , m_xURLTransformer(_xURLTransformer)
    , m_nSymbolsStyle( SvtMiscOptions().GetCurrentSymbolsStyle() )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::MenuBarManager" );
    m_xPopupMenuControllerRegistration = Reference< ::com::sun::star::frame::XUIControllerRegistration >(
        getServiceFactory()->createInstance( rtl::OUString( "com.sun.star.frame.PopupMenuControllerFactory" )),
        UNO_QUERY );
    FillMenuManager( pMenu, rFrame, rDispatchProvider, rModuleIdentifier, bDelete, bDeleteChildren );
}

MenuBarManager::MenuBarManager(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XFrame >& rFrame,
    const Reference< XURLTransformer >& _xURLTransformer,
    AddonMenu* pAddonMenu,
    sal_Bool bDelete,
    sal_Bool bDeleteChildren )
:   ThreadHelpBase( &Application::GetSolarMutex() )
    , OWeakObject()
    , m_bDisposed( sal_False )
    , m_bRetrieveImages( sal_True )
    , m_bAcceleratorCfg( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , mxServiceFactory(xServiceFactory)
    , m_xURLTransformer(_xURLTransformer)
    , m_nSymbolsStyle( SvtMiscOptions().GetCurrentSymbolsStyle() )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::MenuBarManager" );
    Init(rFrame,pAddonMenu,bDelete,bDeleteChildren);
}

MenuBarManager::MenuBarManager(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XFrame >& rFrame,
    const Reference< XURLTransformer >& _xURLTransformer,
    AddonPopupMenu* pAddonPopupMenu,
    sal_Bool bDelete,
    sal_Bool bDeleteChildren )
:     ThreadHelpBase( &Application::GetSolarMutex() )
    , OWeakObject()
    , m_bDisposed( sal_False )
    , m_bRetrieveImages( sal_True )
    , m_bAcceleratorCfg( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , mxServiceFactory(xServiceFactory)
    , m_xURLTransformer(_xURLTransformer)
    , m_nSymbolsStyle( SvtMiscOptions().GetCurrentSymbolsStyle() )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::MenuBarManager" );
    Init(rFrame,pAddonPopupMenu,bDelete,bDeleteChildren,true);
}

Any SAL_CALL MenuBarManager::queryInterface( const Type & rType ) throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                (static_cast< ::com::sun::star::frame::XStatusListener* >(this)),
                (static_cast< ::com::sun::star::frame::XFrameActionListener* >(this)),
                (static_cast< ::com::sun::star::ui::XUIConfigurationListener* >(this)),
                (static_cast< XEventListener* >((XStatusListener *)this)),
                (static_cast< XComponent* >(this)),
                (static_cast< ::com::sun::star::awt::XSystemDependentMenuPeer* >(this)));

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


Any SAL_CALL MenuBarManager::getMenuHandle( const Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::getMenuHandle" );
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw com::sun::star::lang::DisposedException();

    Any a;

    if ( m_pVCLMenu )
    {
        SolarMutexGuard aSolarGuard;

        SystemMenuData aSystemMenuData;
        aSystemMenuData.nSize = sizeof( SystemMenuData );

        m_pVCLMenu->GetSystemMenuData( &aSystemMenuData );
#ifdef QUARTZ
        if( SystemType == SystemDependent::SYSTEM_MAC )
        {
        }
#elif (defined WNT)
        if( SystemType == SystemDependent::SYSTEM_WIN32 )
        {
            a <<= (long) aSystemMenuData.hMenu;
        }
#elif (defined UNX)
        if( SystemType == SystemDependent::SYSTEM_XWINDOW )
        {
        }
#endif
    }

    return a;
}

MenuBarManager::~MenuBarManager()
{
    // stop asynchronous settings timer
    m_xDeferedItemContainer.clear();
    m_aAsyncSettingsTimer.Stop();

    DBG_ASSERT( OWeakObject::m_refCount == 0, "Who wants to delete an object with refcount > 0!" );
}

void MenuBarManager::Destroy()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::Destroy" );
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
            delete m_pVCLMenu;
            m_pVCLMenu = 0;
        }
    }
}

// XComponent
void SAL_CALL MenuBarManager::dispose() throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::dispose" );
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        Destroy();
        m_bDisposed = sal_True;

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
        m_xGlobalAcceleratorManager.clear();
        m_xModuleAcceleratorManager.clear();
        m_xDocAcceleratorManager.clear();
        m_xUICommandLabels.clear();
        m_xPopupMenuControllerRegistration.clear();
        mxServiceFactory.clear();
    }
}

void SAL_CALL MenuBarManager::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::addEventListener" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL MenuBarManager::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::removeEventListener" );
    ResetableGuard aGuard( m_aLock );
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL MenuBarManager::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::elementInserted" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    sal_Int16 nImageType = sal_Int16();
    sal_Int16 nCurrentImageType = getImageTypeFromBools( sal_False );
    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ))
        RequestImages();
}

void SAL_CALL MenuBarManager::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::elementRemoved" );
    elementInserted(Event);
}

void SAL_CALL MenuBarManager::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::elementReplaced" );
    elementInserted(Event);
}

// XFrameActionListener
void SAL_CALL MenuBarManager::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::frameAction" );
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw com::sun::star::lang::DisposedException();

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
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::statusChanged" );
    ::rtl::OUString aFeatureURL = Event.FeatureURL.Complete;

    SolarMutexGuard aSolarGuard;
    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            return;

        // We have to check all menu entries as there can be identical entries in a popup menu.
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pMenuItemHandler = *p;
            if ( pMenuItemHandler->aMenuItemURL == aFeatureURL )
            {
                sal_Bool            bCheckmark( sal_False );
                sal_Bool            bMenuItemEnabled( m_pVCLMenu->IsItemEnabled( pMenuItemHandler->nItemId ));
                sal_Bool            bEnabledItem( Event.IsEnabled );
                rtl::OUString       aItemText;
                status::Visibility  aVisibilityStatus;

                #ifdef UNIX
                //enable some slots hardly, because UNIX clipboard does not notify all changes
                // Can be removed if follow up task will be fixed directly within applications.
                // Note: PasteSpecial is handled specifically by calc
                if ( pMenuItemHandler->aMenuItemURL == ".uno:Paste"
                    || pMenuItemHandler->aMenuItemURL == ".uno:PasteClipboard" )      // special for draw/impress
                    bEnabledItem = sal_True;
                #endif

                // Enable/disable item
                if ( bEnabledItem != bMenuItemEnabled )
                    m_pVCLMenu->EnableItem( pMenuItemHandler->nItemId, bEnabledItem );

                if ( Event.State >>= bCheckmark )
                {
                    // Checkmark or RadioButton
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId, sal_True );
                    m_pVCLMenu->CheckItem( pMenuItemHandler->nItemId, bCheckmark );

                    MenuItemBits nBits = m_pVCLMenu->GetItemBits( pMenuItemHandler->nItemId );
                    //If not already designated RadioButton set as CheckMark
                    if (!(nBits & MIB_RADIOCHECK))
                        m_pVCLMenu->SetItemBits( pMenuItemHandler->nItemId, nBits | MIB_CHECKABLE );
                }
                else if ( Event.State >>= aItemText )
                {
                    // Replacement for place holders
                    if ( aItemText.matchAsciiL( "($1)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_UPDATEDOC ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += rtl::OUString( " " );
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }
                    else if ( aItemText.matchAsciiL( "($2)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_CLOSEDOC_ANDRETURN ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }
                    else if ( aItemText.matchAsciiL( "($3)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_SAVECOPYDOC ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }

                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId, sal_True );
                    m_pVCLMenu->SetItemText( pMenuItemHandler->nItemId, aItemText );
                }
                else if ( Event.State >>= aVisibilityStatus )
                {
                    // Visibility
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId, aVisibilityStatus.bVisible );
                }
                else
                    m_pVCLMenu->ShowItem( pMenuItemHandler->nItemId, sal_True );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::GetMenuItemHandler" );
    ResetableGuard aGuard( m_aLock );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->nItemId == nItemId )
            return pItemHandler;
    }

    return 0;
}

// Helper to set request images flag
void MenuBarManager::RequestImages()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::RequestImages" );

    m_bRetrieveImages = sal_True;
    const sal_uInt32 nCount = m_aMenuItemHandlerVector.size();
    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        MenuItemHandler* pItemHandler = m_aMenuItemHandlerVector[i];
        if ( pItemHandler->xSubMenuManager.is() )
        {
            MenuBarManager* pMenuBarManager = (MenuBarManager*)(pItemHandler->xSubMenuManager.get());
            pMenuBarManager->RequestImages();
        }
    }
}

// Helper to reset objects to prepare shutdown
void MenuBarManager::RemoveListener()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::RemoveListener" );
    ResetableGuard aGuard( m_aLock );

    // Check service manager reference. Remove listener can be called due
    // to a disposing call from the frame and therefore we already removed
    // our listeners and release the service manager reference!
    Reference< XMultiServiceFactory > xServiceManager = getServiceFactory();
    if ( xServiceManager.is() )
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
            if ( pItemHandler->xPopupMenu.is() )
            {
                {
                    // Remove popup menu from menu structure
                    SolarMutexGuard aGuard2;
                    m_pVCLMenu->SetPopupMenu( pItemHandler->nItemId, 0 );
                }

                Reference< com::sun::star::lang::XEventListener > xEventListener( pItemHandler->xPopupMenuController, UNO_QUERY );
                if ( xEventListener.is() )
                {
                    EventObject aEventObject;
                    aEventObject.Source = (OWeakObject *)this;
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

    m_xFrame = 0;
}

void SAL_CALL MenuBarManager::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::disposing(evt)" );
    MenuItemHandler* pMenuItemDisposing = NULL;

    ResetableGuard aGuard( m_aLock );

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
        Reference< XMultiServiceFactory > xServiceManager( getServiceFactory() );
        if ( xServiceManager.is() )
        {
            m_xURLTransformer->parseStrict( aTargetURL );

            pMenuItemDisposing->xMenuItemDispatch->removeStatusListener(
                static_cast< XStatusListener* >( this ), aTargetURL );
            pMenuItemDisposing->xMenuItemDispatch = Reference< XDispatch >();
            if ( pMenuItemDisposing->xPopupMenu.is() )
            {
                Reference< com::sun::star::lang::XEventListener > xEventListener( pMenuItemDisposing->xPopupMenuController, UNO_QUERY );
                if ( xEventListener.is() )
                    xEventListener->disposing( Source );

                {
                    // Remove popup menu from menu structure as we release our reference to
                    // the controller.
                    SolarMutexGuard aGuard2;
                    m_pVCLMenu->SetPopupMenu( pMenuItemDisposing->nItemId, 0 );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::CheckAndAddMenuExtension" );

    // retrieve menu extension item
    MenuExtensionItem aMenuItem( GetMenuExtension() );
    if (( !aMenuItem.aURL.isEmpty() ) &&
        ( !aMenuItem.aLabel.isEmpty() ))
    {
        // remove all old window list entries from menu
        sal_uInt16 nNewItemId( 0 );
        sal_uInt16 nInsertPos( MENU_APPEND );
        sal_uInt16 nBeforePos( MENU_APPEND );
        String aCommandBefore( rtl::OUString(".uno:About"));
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

        pMenu->InsertItem( nNewItemId, aMenuItem.aLabel, 0, nInsertPos );
        pMenu->SetItemCommand( nNewItemId, aMenuItem.aURL );
    }
}

static void lcl_CheckForChildren(Menu* pMenu, sal_uInt16 nItemId)
{
    if (PopupMenu* pThisPopup = pMenu->GetPopupMenu( nItemId ))
        pMenu->EnableItem( nItemId, pThisPopup->GetItemCount() ? true : false );
}

//_________________________________________________________________________________________________________________
// vcl handler
//_________________________________________________________________________________________________________________

namespace {

class QuietInteractionContext:
    public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >,
    private boost::noncopyable
{
public:
    QuietInteractionContext(
        com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext >
            const & context):
        context_(context) {}

private:
    virtual ~QuietInteractionContext() {}

    virtual com::sun::star::uno::Any SAL_CALL getValueByName(
        rtl::OUString const & Name)
        throw (com::sun::star::uno::RuntimeException)
    {
        return Name != JAVA_INTERACTION_HANDLER_NAME && context_.is()
            ? context_->getValueByName(Name)
            : com::sun::star::uno::Any();
    }

    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext >
        context_;
};

}

IMPL_LINK( MenuBarManager, Activate, Menu *, pMenu )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::Activate" );
    if ( pMenu == m_pVCLMenu )
    {
        com::sun::star::uno::ContextLayer layer(
            new QuietInteractionContext(
                com::sun::star::uno::getCurrentContext()));

        // set/unset hiding disabled menu entries
        sal_Bool bDontHide           = SvtMenuOptions().IsEntryHidingEnabled();
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bShowMenuImages     = rSettings.GetUseImagesInMenus();
        sal_Bool bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );

        ResetableGuard aGuard( m_aLock );

        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );

        if ( m_bActive )
            return 0;

        m_bActive = sal_True;

        ::rtl::OUString aMenuCommand( m_aMenuItemCommand );
        if ( m_aMenuItemCommand == aSpecialWindowMenu || m_aMenuItemCommand == aSlotSpecialWindowMenu || aMenuCommand == aSpecialWindowCommand )
             MenuManager::UpdateSpecialWindowMenu( pMenu,getServiceFactory(),m_aLock );

        // Check if some modes have changed so we have to update our menu images
        sal_Int16 nSymbolsStyle = SvtMiscOptions().GetCurrentSymbolsStyle();

        if ( m_bRetrieveImages ||
             bShowMenuImages != m_bShowMenuImages ||
             nSymbolsStyle != m_nSymbolsStyle )
        {
            m_bShowMenuImages   = bShowMenuImages;
            m_bRetrieveImages   = sal_False;
            m_nSymbolsStyle     = nSymbolsStyle;
            MenuManager::FillMenuImages( m_xFrame, pMenu, bShowMenuImages );
        }

        // Try to map commands to labels
        for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            sal_uInt16 nItemId = pMenu->GetItemId( nPos );
            if (( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR ) &&
                ( pMenu->GetItemText( nItemId ).Len() == 0 ))
            {
                String aCommand = pMenu->GetItemCommand( nItemId );
                if ( aCommand.Len() > 0 )
                    pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aCommand ));
            }
        }

        // Try to set accelerator keys
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
                    KeyCode aKeyCode( KEY_F1 );
                    pMenu->SetAccelKey( pMenuItemHandler->nItemId, aKeyCode );
                }
                else if ( pMenu->GetPopupMenu( pMenuItemHandler->nItemId ) == 0 )
                    pMenu->SetAccelKey( pMenuItemHandler->nItemId, pMenuItemHandler->aKeyCode );
            }
        }

        URL aTargetURL;

        // Use provided dispatch provider => fallback to frame as dispatch provider
        Reference< XDispatchProvider > xDispatchProvider;
        if ( m_xDispatchProvider.is() )
            xDispatchProvider = m_xDispatchProvider;
        else
            xDispatchProvider = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );

        if ( xDispatchProvider.is() )
        {
            KeyCode             aEmptyKeyCode;
            SvtCommandOptions   aCmdOptions;
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

                            ::rtl::OUString aItemCommand = pMenu->GetItemCommand( pMenuItemHandler->nItemId );
                            if ( aItemCommand.isEmpty() )
                            {
                                aItemCommand = ::rtl::OUString( "slot:" );
                                aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                                pMenu->SetItemCommand( pMenuItemHandler->nItemId, aItemCommand );
                            }

                            aTargetURL.Complete = aItemCommand;

                            m_xURLTransformer->parseStrict( aTargetURL );

                            if ( bHasDisabledEntries )
                            {
                                if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aTargetURL.Path ))
                                    pMenu->HideItem( pMenuItemHandler->nItemId );
                            }

                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                            sal_Bool bPopupMenu( sal_False );
                            if ( !pMenuItemHandler->xPopupMenuController.is() &&
                                 m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ))
                            {
                                bPopupMenu = CreatePopupMenuController( pMenuItemHandler );
                            }
                            else if ( pMenuItemHandler->xPopupMenuController.is() )
                            {
                                // Force update of popup menu
                                pMenuItemHandler->xPopupMenuController->updatePopupMenu();
                                bPopupMenu = sal_True;
                                if (PopupMenu*  pThisPopup = pMenu->GetPopupMenu( pMenuItemHandler->nItemId ))
                                    pMenu->EnableItem( pMenuItemHandler->nItemId, pThisPopup->GetItemCount() ? true : false );
                            }

                            lcl_CheckForChildren(pMenu, pMenuItemHandler->nItemId);

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;

                                if ( !bPopupMenu )
                                {
                                    // We need only an update to reflect the current state
                                    xMenuItemDispatch->addStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                                    xMenuItemDispatch->removeStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                                }
                            }
                            else if ( !bPopupMenu )
                                pMenu->EnableItem( pMenuItemHandler->nItemId, sal_False );
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

    return 1;
}


IMPL_LINK( MenuBarManager, Deactivate, Menu *, pMenu )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::Deactivate" );
    if ( pMenu == m_pVCLMenu )
    {
        m_bActive = sal_False;
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

    return 1;
}

IMPL_LINK( MenuBarManager, AsyncSettingsHdl, Timer*,)
{
    SolarMutexGuard aGuard;
    Reference< XInterface > xSelfHold(
        static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY_THROW );

    m_aAsyncSettingsTimer.Stop();
    if ( !m_bActive && m_xDeferedItemContainer.is() )
    {
        SetItemContainer( m_xDeferedItemContainer );
        m_xDeferedItemContainer.clear();
    }

    return 0;
}

IMPL_LINK( MenuBarManager, Select, Menu *, pMenu )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::Select" );
    URL                     aTargetURL;
    Sequence<PropertyValue> aArgs;
    Reference< XDispatch >  xDispatch;

    {
        ResetableGuard aGuard( m_aLock );

        sal_uInt16 nCurItemId = pMenu->GetCurItemId();
        sal_uInt16 nCurPos    = pMenu->GetItemPos( nCurItemId );
        if ( pMenu == m_pVCLMenu &&
             pMenu->GetItemType( nCurPos ) != MENUITEM_SEPARATOR )
        {
            if ( nCurItemId >= START_ITEMID_WINDOWLIST &&
                 nCurItemId <= END_ITEMID_WINDOWLIST )
            {
                // window list menu item selected

                Reference< XFramesSupplier > xDesktop( getServiceFactory()->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );

                if ( xDesktop.is() )
                {
                    sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
                    Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
                    sal_Int32 nCount = xList->getCount();
                    for ( sal_Int32 i=0; i<nCount; ++i )
                    {
                        Reference< XFrame > xFrame;
                        xList->getByIndex(i) >>= xFrame;
                        if ( xFrame.is() && nTaskId == nCurItemId )
                        {
                            Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                            pWin->GrabFocus();
                            pWin->ToTop( TOTOP_RESTOREWHENMIN );
                            break;
                        }

                        nTaskId++;
                    }
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
                        aArgs[0].Name = ::rtl::OUString( "Referer" );
                        aArgs[0].Value <<= ::rtl::OUString( SFX_REFERER_USER );
                    }

                    xDispatch = pMenuItemHandler->xMenuItemDispatch;
                }
            }
        }
    }

    if ( xDispatch.is() )
    {
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        xDispatch->dispatch( aTargetURL, aArgs );
        Application::AcquireSolarMutex( nRef );
    }

    return 1;
}


IMPL_LINK_NOARG(MenuBarManager, Highlight)
{
    return 0;
}

sal_Bool MenuBarManager::MustBeHidden( PopupMenu* pPopupMenu, const Reference< XURLTransformer >& rTransformer )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::MustBeHidden" );
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

    return sal_True;
}
String MenuBarManager::RetrieveLabelFromCommand( const String& aCmdURL )
{
    return framework::RetrieveLabelFromCommand(aCmdURL,mxServiceFactory,m_xUICommandLabels,m_xFrame,m_aModuleIdentifier,m_bModuleIdentified,"Label");
}



sal_Bool MenuBarManager::CreatePopupMenuController( MenuItemHandler* pMenuItemHandler )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::CreatePopupMenuController" );
    rtl::OUString aItemCommand( pMenuItemHandler->aMenuItemURL );

    // Try instanciate a popup menu controller. It is stored in the menu item handler.
    Reference< XMultiComponentFactory > xPopupMenuControllerFactory( m_xPopupMenuControllerRegistration, UNO_QUERY );
    if ( xPopupMenuControllerFactory.is() )
    {
        Sequence< Any > aSeq( 2 );
        PropertyValue aPropValue;

        aPropValue.Name         = rtl::OUString( "ModuleName" );
        aPropValue.Value      <<= m_aModuleIdentifier;
        aSeq[0] <<= aPropValue;
        aPropValue.Name         = rtl::OUString( "Frame" );
        aPropValue.Value      <<= m_xFrame;
        aSeq[1] <<= aPropValue;

        Reference< XComponentContext > xComponentContext(
            comphelper::getComponentContext( getServiceFactory() ) );

        Reference< XPopupMenuController > xPopupMenuController(
                                                xPopupMenuControllerFactory->createInstanceWithArgumentsAndContext(
                                                    aItemCommand,
                                                    aSeq,
                                                    xComponentContext ),
                                                UNO_QUERY );

        if ( xPopupMenuController.is() )
        {
            // Provide our awt popup menu to the popup menu controller
            pMenuItemHandler->xPopupMenuController = xPopupMenuController;
            xPopupMenuController->setPopupMenu( pMenuItemHandler->xPopupMenu );
            return sal_True;
        }
    }

    return sal_False;
}

void MenuBarManager::FillMenuManager( Menu* pMenu, const Reference< XFrame >& rFrame, const Reference< XDispatchProvider >& rDispatchProvider, const rtl::OUString& rModuleIdentifier, sal_Bool bDelete, sal_Bool bDeleteChildren )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::FillMenuManager" );
    m_xFrame            = rFrame;
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_False;
    m_xDispatchProvider = rDispatchProvider;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();
    m_bRetrieveImages   = sal_False;

    sal_Int32 nAddonsURLPrefixLength = ADDONSPOPUPMENU_URL_PREFIX.getLength();

    // Add root as ui configuration listener
    RetrieveImageManagers();

    if ( pMenu->IsMenuBar() && rFrame.is() )
    {
        // First merge all addon popup menus into our structure
        sal_uInt16 nPos = 0;
        for ( nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            sal_uInt16          nItemId  = pMenu->GetItemId( nPos );
            ::rtl::OUString aCommand = pMenu->GetItemCommand( nItemId );
            if ( nItemId == SID_MDIWINDOWLIST || aCommand == aSpecialWindowCommand)
            {
                // Retrieve addon popup menus and add them to our menu bar
                Reference< com::sun::star::frame::XModel >      xModel;
                Reference< com::sun::star::frame::XController > xController( rFrame->getController(), UNO_QUERY );
                if ( xController.is() )
                    xModel = Reference< com::sun::star::frame::XModel >( xController->getModel(), UNO_QUERY );
                framework::AddonMenuManager::MergeAddonPopupMenus( rFrame, xModel, nPos, (MenuBar *)pMenu );
                break;
            }
        }

        // Merge the Add-Ons help menu items into the Office help menu
        framework::AddonMenuManager::MergeAddonHelpMenu( rFrame, (MenuBar *)pMenu );
    }

    String      aEmpty;
    sal_Bool    bAccessibilityEnabled( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() );
    sal_uInt16 nItemCount = pMenu->GetItemCount();
    ::rtl::OUString aItemCommand;
    m_aMenuItemHandlerVector.reserve(nItemCount);
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = FillItemCommand(aItemCommand,pMenu, i );

        // Set module identifier when provided from outside
        if ( !rModuleIdentifier.isEmpty() )
        {
            m_aModuleIdentifier = rModuleIdentifier;
            m_bModuleIdentified = sal_True;
        }

        if (( pMenu->IsMenuBar() || bAccessibilityEnabled ) &&
            ( pMenu->GetItemText( nItemId ).Len() == 0 ))
        {
            if ( !aItemCommand.isEmpty() )
                pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aItemCommand ));
        }

        Reference< XDispatch > xDispatch;
        Reference< XStatusListener > xStatusListener;
        PopupMenu* pPopup = pMenu->GetPopupMenu( nItemId );
        bool bItemShowMenuImages = m_bShowMenuImages;
        // overwrite the show icons on menu option?
        if (!bItemShowMenuImages)
        {
            MenuItemBits nBits =  pMenu->GetItemBits( nItemId );
            bItemShowMenuImages = ( ( nBits & MIB_ICON ) == MIB_ICON );
        }
        if ( pPopup )
        {
            // Retrieve module identifier from Help Command entry
            rtl::OUString aModuleIdentifier( rModuleIdentifier );
            if ( pMenu->GetHelpCommand( nItemId ).Len() > 0 )
            {
                aModuleIdentifier = pMenu->GetHelpCommand( nItemId );
                pMenu->SetHelpCommand( nItemId, aEmpty );
            }

            if ( m_xPopupMenuControllerRegistration.is() &&
                 pPopup->GetItemCount() == 0 &&
                 m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() )
                  )
            {
                // Check if we have to create a popup menu for a uno based popup menu controller.
                // We have to set an empty popup menu into our menu structure so the controller also
                // works with inplace OLE. Remove old dummy popup menu!
                MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );
                VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                PopupMenu* pNewPopupMenu = (PopupMenu *)pVCLXPopupMenu->GetMenu();
                pMenu->SetPopupMenu( nItemId, pNewPopupMenu );
                pItemHandler->xPopupMenu = Reference< com::sun::star::awt::XPopupMenu >( (OWeakObject *)pVCLXPopupMenu, UNO_QUERY );
                pItemHandler->aMenuItemURL = aItemCommand;
                m_aMenuItemHandlerVector.push_back( pItemHandler );
                delete pPopup;

                if ( bAccessibilityEnabled )
                {
                    if ( CreatePopupMenuController( pItemHandler ))
                        pItemHandler->xPopupMenuController->updatePopupMenu();
                }
                lcl_CheckForChildren(pMenu, nItemId);
            }
            else if (( aItemCommand.getLength() > nAddonsURLPrefixLength ) &&
                     ( aItemCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 ))
            {
                // A special addon popup menu, must be created with a different ctor
                MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), m_xFrame, m_xURLTransformer,(AddonPopupMenu *)pPopup, bDeleteChildren, bDeleteChildren );
                AddMenu(pSubMenuManager,aItemCommand,nItemId);
            }
            else
            {
                Reference< XDispatchProvider > xPopupMenuDispatchProvider( rDispatchProvider );

                // Retrieve possible attributes struct
                MenuConfiguration::Attributes* pAttributes = (MenuConfiguration::Attributes *)(pMenu->GetUserValue( nItemId ));
                if ( pAttributes )
                    xPopupMenuDispatchProvider = pAttributes->xDispatchProvider;

                // Check if this is the help menu. Add menu item if needed
                if ( nItemId == SID_HELPMENU || aItemCommand == aSlotHelpMenu || aItemCommand == aCmdHelpMenu )
                {
                    // Check if this is the help menu. Add menu item if needed
                    CheckAndAddMenuExtension( pPopup );
                }
                else if (( nItemId == SID_ADDONLIST || aItemCommand == aSlotSpecialToolsMenu || aItemCommand == aCmdToolsMenu ) &&
                        AddonMenuManager::HasAddonMenuElements() )
                {
                    // Create addon popup menu if there exist elements and this is the tools popup menu
                    AddonMenu*  pSubMenu = AddonMenuManager::CreateAddonMenu( rFrame );
                    if ( pSubMenu && ( pSubMenu->GetItemCount() > 0 ))
                    {
                        sal_uInt16 nCount = 0;
                        if ( pPopup->GetItemType( nCount-1 ) != MENUITEM_SEPARATOR )
                            pPopup->InsertSeparator();

                        // Use resource to load popup menu title
                        String aAddonsStrRes = String( FwkResId( STR_MENU_ADDONS ));
                        pPopup->InsertItem( ITEMID_ADDONLIST, aAddonsStrRes );
                        pPopup->SetPopupMenu( ITEMID_ADDONLIST, pSubMenu );

                        // Set item command for popup menu to enable it for GetImageFromURL
                        const ::rtl::OUString aSlotString( "slot:" );
                        ::rtl::OUString aNewItemCommand( aSlotString );
                        aNewItemCommand += ::rtl::OUString::valueOf( (sal_Int32)ITEMID_ADDONLIST );
                        pPopup->SetItemCommand( ITEMID_ADDONLIST, aNewItemCommand );
                    }
                    else
                        delete pSubMenu;
                }

                if ( nItemId == ITEMID_ADDONLIST )
                {
                    AddonMenu* pSubMenu = dynamic_cast< AddonMenu* >( pPopup );
                    if ( pSubMenu )
                    {
                        MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), m_xFrame, m_xURLTransformer,pSubMenu, sal_True, sal_False );
                        AddMenu(pSubMenuManager,aItemCommand,nItemId);
                        pSubMenuManager->m_aMenuItemCommand = ::rtl::OUString();

                        // Set image for the addon popup menu item
                        if ( bItemShowMenuImages && !pPopup->GetItemImage( ITEMID_ADDONLIST ))
                        {
                            Reference< XFrame > xTemp( rFrame );
                            Image aImage = GetImageFromURL( xTemp, aItemCommand, false );
                            if ( !!aImage )
                                   pPopup->SetItemImage( ITEMID_ADDONLIST, aImage );
                        }
                    }
                }
                else
                {
                    MenuBarManager* pSubMenuMgr = new MenuBarManager( getServiceFactory(), rFrame, m_xURLTransformer,rDispatchProvider, aModuleIdentifier, pPopup, bDeleteChildren, bDeleteChildren );
                    AddMenu(pSubMenuMgr,aItemCommand,nItemId);
                }
            }
        }
        else if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            if ( bItemShowMenuImages )
            {
                if ( AddonMenuManager::IsAddonMenuId( nItemId ))
                {
                    // Add-Ons uses images from different places
                    Image           aImage;
                    rtl::OUString   aImageId;

                    MenuConfiguration::Attributes* pMenuAttributes =
                        (MenuConfiguration::Attributes*)pMenu->GetUserValue( nItemId );

                    if ( pMenuAttributes && !pMenuAttributes->aImageId.isEmpty() )
                    {
                        // Retrieve image id from menu attributes
                        aImage = GetImageFromURL( m_xFrame, aImageId, false );
                    }

                    if ( !aImage )
                    {
                        aImage = GetImageFromURL( m_xFrame, aItemCommand, false );
                        if ( !aImage )
                            aImage = AddonsOptions().GetImageFromURL( aItemCommand, false );
                    }

                    if ( !!aImage )
                        pMenu->SetItemImage( nItemId, aImage );
                    else
                        m_bRetrieveImages = sal_True;
                }
                m_bRetrieveImages = sal_True;
            }

            MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );
            pItemHandler->aMenuItemURL = aItemCommand;

            if ( m_xPopupMenuControllerRegistration.is() &&
                 m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ))
            {
                // Check if we have to create a popup menu for a uno based popup menu controller.
                // We have to set an empty popup menu into our menu structure so the controller also
                // works with inplace OLE.
                VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                PopupMenu* pPopupMenu = (PopupMenu *)pVCLXPopupMenu->GetMenu();
                pMenu->SetPopupMenu( pItemHandler->nItemId, pPopupMenu );
                pItemHandler->xPopupMenu = Reference< com::sun::star::awt::XPopupMenu >( (OWeakObject *)pVCLXPopupMenu, UNO_QUERY );

                if ( bAccessibilityEnabled && CreatePopupMenuController( pItemHandler ) )
                {
                    pItemHandler->xPopupMenuController->updatePopupMenu();
                }

                lcl_CheckForChildren(pMenu, pItemHandler->nItemId);
            }

            m_aMenuItemHandlerVector.push_back( pItemHandler );
        }
    }

    if ( bAccessibilityEnabled )
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
                KeyCode aKeyCode( KEY_F1 );
                pMenu->SetAccelKey( pMenuItemHandler->nItemId, aKeyCode );
            }
            else if ( pMenu->GetPopupMenu( pMenuItemHandler->nItemId ) == 0 )
                pMenu->SetAccelKey( pMenuItemHandler->nItemId, pMenuItemHandler->aKeyCode );
        }
    }

    SetHdl();
}

void MenuBarManager::impl_RetrieveShortcutsFromConfiguration(
    const Reference< XAcceleratorConfiguration >& rAccelCfg,
    const Sequence< rtl::OUString >& rCommands,
    std::vector< MenuItemHandler* >& aMenuShortCuts )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::impl_RetrieveShortcutsFromConfiguration" );
    if ( rAccelCfg.is() )
    {
        try
        {
            com::sun::star::awt::KeyEvent aKeyEvent;
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::RetrieveShortcuts" );
    if ( !m_bModuleIdentified )
    {
        m_bModuleIdentified = sal_True;
        Reference< XModuleManager > xModuleManager;
        xModuleManager = Reference< XModuleManager >( getServiceFactory()->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

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
            m_bAcceleratorCfg = sal_True;
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
                                xDocAccelCfg = Reference< XAcceleratorConfiguration >( xDocUICfgMgr->getShortCutManager(), UNO_QUERY );
                                m_xDocAcceleratorManager = xDocAccelCfg;
                            }
                        }
                    }
                }
            }

            if ( !xModuleAccelCfg.is() )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier( getServiceFactory()->createInstance(
                                                                                            SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ),
                                                                                        UNO_QUERY );
                try
                {
                    Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
                    if ( xUICfgMgr.is() )
                    {
                        xModuleAccelCfg = Reference< XAcceleratorConfiguration >( xUICfgMgr->getShortCutManager(), UNO_QUERY );
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

            if ( !xGlobalAccelCfg.is() )
            {
                xGlobalAccelCfg = Reference< XAcceleratorConfiguration >( getServiceFactory()->createInstance(
                                                                            SERVICENAME_GLOBALACCELERATORCONFIGURATION ),
                                                                          UNO_QUERY );
                m_xGlobalAcceleratorManager = xGlobalAccelCfg;
            }
        }

        KeyCode aEmptyKeyCode;
        Sequence< rtl::OUString > aSeq( aMenuShortCuts.size() );
        const sal_uInt32 nCount = aMenuShortCuts.size();
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            aSeq[i] = aMenuShortCuts[i]->aMenuItemURL;
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::RetrieveImageManagers" );
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
                    m_xDocImageManager = Reference< XImageManager >( xDocUICfgMgr->getImageManager(), UNO_QUERY );
                    m_xDocImageManager->addConfigurationListener(
                                            Reference< XUIConfigurationListener >(
                                                static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
            }
        }
    }

    Reference< XModuleManager > xModuleManager;
    if ( m_aModuleIdentifier.isEmpty() )
        xModuleManager.set( getServiceFactory()->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

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
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier( getServiceFactory()->createInstance(
                                                                                    SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ),
                                                                                  UNO_QUERY );
        Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
        m_xModuleImageManager.set( xUICfgMgr->getImageManager(), UNO_QUERY );
        m_xModuleImageManager->addConfigurationListener( Reference< XUIConfigurationListener >(
                                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

void MenuBarManager::FillMenuWithConfiguration(
    sal_uInt16&                             nId,
    Menu*                               pMenu,
    const ::rtl::OUString&              rModuleIdentifier,
    const Reference< XIndexAccess >&    rItemContainer,
    const Reference< XURLTransformer >& rTransformer )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::FillMenuWithConfiguration" );
    Reference< XDispatchProvider > xEmptyDispatchProvider;
    MenuBarManager::FillMenu( nId, pMenu, rModuleIdentifier, rItemContainer, xEmptyDispatchProvider );

    // Merge add-on menu entries into the menu bar
    MenuBarManager::MergeAddonMenus( static_cast< Menu* >( pMenu ),
                                     AddonsOptions().GetMergeMenuInstructions(),
                                     rModuleIdentifier );

    sal_Bool bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );
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
    sal_uInt16&                               nId,
    Menu*                                 pMenu,
    const rtl::OUString&                  rModuleIdentifier,
    const Reference< XIndexAccess >&      rItemContainer,
    const Reference< XDispatchProvider >& rDispatchProvider )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::FillMenu" );
    // Fill menu bar with container contents
    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >       aProp;
        rtl::OUString                   aCommandURL;
        rtl::OUString                   aLabel;
        rtl::OUString                   aHelpURL;
        rtl::OUString                   aModuleIdentifier( rModuleIdentifier );
        sal_Bool                        bShow(sal_True);
        sal_Bool                        bEnabled(sal_True);
        sal_uInt16                      nType = 0;
        Reference< XIndexAccess >       xIndexContainer;
        Reference< XDispatchProvider >  xDispatchProvider( rDispatchProvider );
        sal_Int16 nStyle = 0;
        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    rtl::OUString aPropName = aProp[i].Name;
                    if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_COMMANDURL, LEN_DESCRIPTOR_COMMANDURL ))
                        aProp[i].Value >>= aCommandURL;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_HELPURL, LEN_DESCRIPTOR_HELPURL ))
                        aProp[i].Value >>= aHelpURL;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_CONTAINER, LEN_DESCRIPTOR_CONTAINER ))
                        aProp[i].Value >>= xIndexContainer;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_LABEL, LEN_DESCRIPTOR_LABEL ))
                        aProp[i].Value >>= aLabel;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_TYPE, LEN_DESCRIPTOR_TYPE ))
                        aProp[i].Value >>= nType;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_MODULEIDENTIFIER, LEN_DESCRIPTOR_MODULEIDENTIFIER ))
                        aProp[i].Value >>= aModuleIdentifier;
                    else if ( aPropName.equalsAsciiL( ITEM_DESCRIPTOR_DISPATCHPROVIDER, LEN_DESCRIPTOR_DISPATCHPROVIDER ))
                        aProp[i].Value >>= xDispatchProvider;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_STYLE, LEN_DESCRIPTOR_STYLE ))
                        aProp[i].Value >>= nStyle;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_ISVISIBLE, LEN_DESCRIPTOR_ISVISIBLE ))
                        aProp[i].Value >>= bShow;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_ENABLED, LEN_DESCRIPTOR_ENABLED ))
                        aProp[i].Value >>= bEnabled;
                }

                if ( nType == ::com::sun::star::ui::ItemType::DEFAULT )
                {
                    pMenu->InsertItem( nId, aLabel );
                    pMenu->SetItemCommand( nId, aCommandURL );

                    if ( nStyle )
                    {
                        MenuItemBits nBits = pMenu->GetItemBits( nId );
                        if ( nStyle & ::com::sun::star::ui::ItemStyle::ICON )
                           nBits |= MIB_ICON;
                        if ( nStyle & ::com::sun::star::ui::ItemStyle::TEXT )
                           nBits |= MIB_TEXT;
                        if ( nStyle & ::com::sun::star::ui::ItemStyle::RADIO_CHECK )
                           nBits |= MIB_RADIOCHECK;
                        pMenu->SetItemBits( nId, nBits );
                    }

                    if ( !bShow )
                        pMenu->HideItem( nId );

                    if ( !bEnabled)
                        pMenu->EnableItem( nId, sal_False );

                    if ( xIndexContainer.is() )
                    {
                        PopupMenu* pNewPopupMenu = new PopupMenu;
                        pMenu->SetPopupMenu( nId, pNewPopupMenu );

                        if ( xDispatchProvider.is() )
                        {
                            // Use attributes struct to transport special dispatch provider
                            MenuConfiguration::Attributes* pAttributes = new MenuConfiguration::Attributes;
                            pAttributes->xDispatchProvider = xDispatchProvider;
                            pMenu->SetUserValue( nId, (sal_uIntPtr)( pAttributes ));
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
    const ::rtl::OUString& rModuleIdentifier )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::MergeAddonMenus" );
    // set start value for the item ID for the new addon menu items
    sal_uInt16 nItemId = ADDONMENU_MERGE_ITEMID_START;

    const sal_uInt32 nCount = aMergeInstructionContainer.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        const MergeMenuInstruction& rMergeInstruction = aMergeInstructionContainer[i];

        if ( MenuBarMerger::IsCorrectContext( rMergeInstruction.aMergeContext, rModuleIdentifier ))
        {
            ::std::vector< ::rtl::OUString > aMergePath;

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
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::MenuBarManager::SetItemContainer" );

    ResetableGuard aGuard( m_aLock );

    Reference< XFrame > xFrame = m_xFrame;

    if ( !m_bModuleIdentified )
    {
        m_bModuleIdentified = sal_True;
        Reference< XModuleManager > xModuleManager;
        xModuleManager = Reference< XModuleManager >( getServiceFactory()->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

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
        SolarMutexGuard aSolarMutexGuard;

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
        FillMenuWithConfiguration( nId, (Menu *)m_pVCLMenu, m_aModuleIdentifier, rItemContainer, m_xURLTransformer );

        // Refill menu manager again
        Reference< XDispatchProvider > xDispatchProvider;
        FillMenuManager( m_pVCLMenu, xFrame, xDispatchProvider, m_aModuleIdentifier, sal_False, sal_True );

        // add itself as frame action listener
        m_xFrame->addFrameActionListener( Reference< XFrameActionListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

void MenuBarManager::GetPopupController( PopupControllerCache& rPopupController )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::GetPopupController" );

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
            rtl::OUString aMainURL( "vnd.sun.star.popup:" );
            rtl::OUString aMenuURL( pItemHandler->aMenuItemURL );

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
            MenuBarManager* pMenuBarManager = (MenuBarManager*)(pItemHandler->xSubMenuManager.get());
            if ( pMenuBarManager )
                pMenuBarManager->GetPopupController( rPopupController );
        }
    }
}

const Reference< XMultiServiceFactory >& MenuBarManager::getServiceFactory()
{
    return mxServiceFactory;
}

void MenuBarManager::AddMenu(MenuBarManager* pSubMenuManager,const ::rtl::OUString& _sItemCommand,sal_uInt16 _nItemId)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::AddMenu" );
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

sal_uInt16 MenuBarManager::FillItemCommand(::rtl::OUString& _rItemCommand,Menu* _pMenu,sal_uInt16 _nIndex) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::FillItemCommand" );
    sal_uInt16 nItemId = _pMenu->GetItemId( _nIndex );

    _rItemCommand = _pMenu->GetItemCommand( nItemId );
    if ( _rItemCommand.isEmpty() )
    {
        const static ::rtl::OUString aSlotString( "slot:" );
        _rItemCommand = aSlotString;
        _rItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
        _pMenu->SetItemCommand( nItemId, _rItemCommand );
    }
    return nItemId;
}
void MenuBarManager::Init(const Reference< XFrame >& rFrame,AddonMenu* pAddonMenu,sal_Bool bDelete,sal_Bool bDeleteChildren,bool _bHandlePopUp)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "MenuBarManager::Init" );
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pAddonMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_True;

    rtl::OUString aModuleIdentifier;
    m_xPopupMenuControllerRegistration = Reference< ::com::sun::star::frame::XUIControllerRegistration >(
        getServiceFactory()->createInstance( rtl::OUString( "com.sun.star.frame.PopupMenuControllerFactory" )),
        UNO_QUERY );

    Reference< XStatusListener > xStatusListener;
    Reference< XDispatch > xDispatch;
    sal_uInt16 nItemCount = pAddonMenu->GetItemCount();
    ::rtl::OUString aItemCommand;
    m_aMenuItemHandlerVector.reserve(nItemCount);
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = FillItemCommand(aItemCommand,pAddonMenu, i );

        PopupMenu* pPopupMenu = pAddonMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            Reference< XDispatchProvider > xDispatchProvider;
            MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, m_xURLTransformer,xDispatchProvider, aModuleIdentifier, pPopupMenu, _bHandlePopUp ? sal_False : bDeleteChildren, _bHandlePopUp ? sal_False : bDeleteChildren );

            Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );

            // store menu item command as we later have to know which menu is active (see Acivate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        xSubMenuManager,
                                                        xDispatch );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pAddonMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                MenuConfiguration::Attributes* pAddonAttributes = (MenuConfiguration::Attributes *)(pAddonMenu->GetUserValue( nItemId ));
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
                    if ( m_xPopupMenuControllerRegistration.is() &&
                        m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ))
                    {
                        VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                        PopupMenu* pCtlPopupMenu = (PopupMenu *)pVCLXPopupMenu->GetMenu();
                        pAddonMenu->SetPopupMenu( pMenuItemHandler->nItemId, pCtlPopupMenu );
                        pMenuItemHandler->xPopupMenu = Reference< com::sun::star::awt::XPopupMenu >( (OWeakObject *)pVCLXPopupMenu, UNO_QUERY );

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
    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuBarManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));

    if ( !m_xURLTransformer.is() && mxServiceFactory.is() )
        m_xURLTransformer.set(
             URLTransformer::create(
                 ::comphelper::getComponentContext(mxServiceFactory)) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
