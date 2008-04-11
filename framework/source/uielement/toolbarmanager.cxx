/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbarmanager.cxx,v $
 * $Revision: 1.39 $
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
#include "precompiled_framework.hxx"
#include <uielement/toolbarmanager.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________


#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX
#include <uielement/toolbar.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX
#include <uielement/generictoolbarcontroller.hxx>
#endif
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include "general.h"
#include "properties.h"
#include <helper/imageproducer.hxx>
#include <classes/sfxhelperfunctions.hxx>
#include <classes/fwkresid.hxx>
#ifndef __FRAMEWORK_CLASES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <classes/addonsoptions.hxx>
#include <uielement/toolbarmerger.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#ifndef _COM_SUN_STAR_UI_XDOCKINGAREA_HPP_
#include <com/sun/star/ui/DockingArea.hpp>
#endif
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/frame/status/Visibility.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <svtools/imgdef.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <svtools/cmdoptions.hxx>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <comphelper/mediadescriptor.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imageitm.hxx>
#include <svtools/framestatuslistener.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <rtl/logfile.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/cmdoptions.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

namespace framework
{

static const char   ITEM_DESCRIPTOR_COMMANDURL[]    = "CommandURL";
static const char   ITEM_DESCRIPTOR_HELPURL[]       = "HelpURL";
static const char   ITEM_DESCRIPTOR_CONTAINER[]     = "ItemDescriptorContainer";
static const char   ITEM_DESCRIPTOR_LABEL[]         = "Label";
static const char   ITEM_DESCRIPTOR_TYPE[]          = "Type";
static const char   ITEM_DESCRIPTOR_VISIBLE[]       = "IsVisible";
static const char   ITEM_DESCRIPTOR_WIDTH[]         = "Width";
static const char   ITEM_DESCRIPTOR_STYLE[]         = "Style";

static const sal_Int32 ITEM_DESCRIPTOR_COMMANDURL_LEN  = 10;
static const sal_Int32 ITEM_DESCRIPTOR_HELPURL_LEN     = 7;
static const sal_Int32 ITEM_DESCRIPTOR_CONTAINER_LEN   = 23;
static const sal_Int32 ITEM_DESCRIPTOR_LABEL_LEN       = 5;
static const sal_Int32 ITEM_DESCRIPTOR_TYPE_LEN        = 4;
static const sal_Int32 ITEM_DESCRIPTOR_VISIBLE_LEN     = 9;
static const sal_Int32 ITEM_DESCRIPTOR_WIDTH_LEN       = 5;
static const sal_Int32 ITEM_DESCRIPTOR_STYLE_LEN       = 5;

static const char   HELPID_PREFIX[]                 = "helpid:";
static const char   HELPID_PREFIX_TESTTOOL[]        = ".HelpId:";
//static sal_Int32    HELPID_PREFIX_LENGTH            = 7;
static const USHORT STARTID_CUSTOMIZE_POPUPMENU     = 1000;


class ImageOrientationListener : public svt::FrameStatusListener
{
    public:
        ImageOrientationListener( const Reference< XStatusListener > rReceiver,
                                  const Reference< XMultiServiceFactory > rServiceManager,
                                  const Reference< XFrame > rFrame );
        virtual ~ImageOrientationListener();

        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

    private:
        Reference< XStatusListener > m_xReceiver;
};

ImageOrientationListener::ImageOrientationListener(
    const Reference< XStatusListener > rReceiver,
    const Reference< XMultiServiceFactory > rServiceManager,
    const Reference< XFrame > rFrame ) :
    FrameStatusListener( rServiceManager, rFrame ),
    m_xReceiver( rReceiver )
{
}

ImageOrientationListener::~ImageOrientationListener()
{
}

void SAL_CALL ImageOrientationListener::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    if ( m_xReceiver.is() )
        m_xReceiver->statusChanged( Event );
}

//*****************************************************************************************************************

static sal_Int16 getImageTypeFromBools( sal_Bool bBig, sal_Bool bHighContrast )
{
    sal_Int16 n( 0 );
    if ( bBig )
        n |= ::com::sun::star::ui::ImageType::SIZE_LARGE;
    if ( bHighContrast )
        n |= ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;
    return n;
}

static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > getLayoutManagerFromFrame(
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

    Reference< XPropertySet > xPropSet( rFrame, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            Any a( xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))) );
            a >>= xLayoutManager;
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
        }
    }

    return xLayoutManager;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_6                     (   ToolBarManager                                                                                              ,
                                            OWeakObject                                                                                                 ,
                                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider                                                     ),
                                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent                                                        ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XFrameActionListener                                             ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationListener                                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XStatusListener                                                  ),
                                            DERIVED_INTERFACE( ::com::sun::star::lang::XEventListener, ::com::sun::star::frame::XFrameActionListener    )
                                        )

DEFINE_XTYPEPROVIDER_6                  (   ToolBarManager                                          ,
                                            ::com::sun::star::lang::XTypeProvider                   ,
                                            ::com::sun::star::lang::XComponent                      ,
                                            ::com::sun::star::ui::XUIConfigurationListener  ,
                                            ::com::sun::star::frame::XFrameActionListener           ,
                                            ::com::sun::star::frame::XStatusListener                ,
                                            ::com::sun::star::lang::XEventListener
                                        )

ToolBarManager::ToolBarManager( const Reference< XMultiServiceFactory >& rServiceManager,
                                const Reference< XFrame >& rFrame,
                                const rtl::OUString& rResourceName,
                                ToolBar* pToolBar ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    OWeakObject(),
    m_bDisposed( sal_False ),
    m_bIsHiContrast( pToolBar->GetSettings().GetStyleSettings().GetFaceColor().IsDark() ),
    m_bSmallSymbols( !SvtMiscOptions().AreCurrentSymbolsLarge() ),
    m_bModuleIdentified( sal_False ),
    m_bAddedToTaskPaneList( sal_True ),
    m_bFrameActionRegistered( sal_False ),
    m_bUpdateControllers( sal_False ),
    m_bImageOrientationRegistered( sal_False ),
    m_bImageMirrored( sal_False ),
    m_bCanBeCustomized( sal_True ),
    m_lImageRotation( 0 ),
    m_pToolBar( pToolBar ),
    m_aResourceName( rResourceName ),
    m_xFrame( rFrame ),
    m_aListenerContainer( m_aLock.getShareableOslMutex() ),
    m_xServiceManager( rServiceManager ),
    m_nSymbolsStyle( SvtMiscOptions().GetCurrentSymbolsStyle() )
{
    Window* pWindow = m_pToolBar;
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( m_pToolBar );

    if ( m_xServiceManager.is() )
        m_xToolbarControllerRegistration = Reference< XUIControllerRegistration >(
                                                    m_xServiceManager->createInstance( SERVICENAME_TOOLBARCONTROLLERFACTORY ),
                                                UNO_QUERY );

    m_pToolBar->SetSelectHdl( LINK( this, ToolBarManager, Select) );
    m_pToolBar->SetActivateHdl( LINK( this, ToolBarManager, Activate) );
    m_pToolBar->SetDeactivateHdl( LINK( this, ToolBarManager, Deactivate) );
    m_pToolBar->SetClickHdl( LINK( this, ToolBarManager, Click ) );
    m_pToolBar->SetDropdownClickHdl( LINK( this, ToolBarManager, DropdownClick ) );
    m_pToolBar->SetDoubleClickHdl( LINK( this, ToolBarManager, DoubleClick ) );
    m_pToolBar->SetStateChangedHdl( LINK( this, ToolBarManager, StateChanged ) );
    m_pToolBar->SetDataChangedHdl( LINK( this, ToolBarManager, DataChanged ) );
    m_pToolBar->SetToolboxButtonSize( m_bSmallSymbols ? TOOLBOX_BUTTONSIZE_SMALL : TOOLBOX_BUTTONSIZE_LARGE );

    // enables a menu for clipped items and customization
    SvtCommandOptions aCmdOptions;
    USHORT nMenuType = TOOLBOX_MENUTYPE_CLIPPEDITEMS;
    if ( !aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, OUString::createFromAscii( "CreateDialog" )))
         nMenuType |= TOOLBOX_MENUTYPE_CUSTOMIZE;
    m_pToolBar->SetMenuType( nMenuType );
    m_pToolBar->SetMenuButtonHdl( LINK( this, ToolBarManager, MenuButton ) );
    m_pToolBar->GetMenu()->SetSelectHdl( LINK( this, ToolBarManager, MenuSelect ) );
    m_pToolBar->GetMenu()->SetDeactivateHdl( LINK( this, ToolBarManager, MenuDeactivate ) );

    // set name for testtool, the useful part is after the last '/'
    sal_Int32 idx = rResourceName.lastIndexOf('/');
    idx++; // will become 0 if '/' not found: use full string
    OUString  aHelpIdAsString( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX_TESTTOOL ));
    OUString  aToolbarName = rResourceName.copy( idx );
    aHelpIdAsString += aToolbarName;
    m_pToolBar->SetSmartHelpId( SmartId( aHelpIdAsString ) );

    m_aAsyncUpdateControllersTimer.SetTimeout( 50 );
    m_aAsyncUpdateControllersTimer.SetTimeoutHdl( LINK( this, ToolBarManager, AsyncUpdateControllersHdl ) );
}

ToolBarManager::~ToolBarManager()
{
    OSL_ASSERT( m_pToolBar == 0 );
    OSL_ASSERT( !m_bAddedToTaskPaneList );
}

void ToolBarManager::Destroy()
{
    ResetableGuard aGuard( m_aLock );
    if ( m_bAddedToTaskPaneList )
    {
        Window* pWindow = m_pToolBar;
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
            ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( m_pToolBar );
        m_bAddedToTaskPaneList = sal_False;
    }

    // Delete the additional add-ons data
    for ( sal_uInt16 i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        sal_uInt16 nItemId = m_pToolBar->GetItemId( i );
        if ( nItemId > 0 )
            delete static_cast< AddonsParams* >( m_pToolBar->GetItemData( nItemId ));
    }

    delete m_pToolBar;
    m_pToolBar = 0;
}

ToolBox* ToolBarManager::GetToolBar() const
{
    ResetableGuard aGuard( m_aLock );
    return m_pToolBar;
}

void ToolBarManager::CheckAndUpdateImages()
{
    ResetableGuard aGuard( m_aLock );
    sal_Bool bRefreshImages = sal_False;

    // Check if high contrast/normal mode have changed
    if ( m_pToolBar->GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
    {
        if ( !m_bIsHiContrast )
        {
            bRefreshImages = TRUE;
            m_bIsHiContrast = sal_True;
        }
    }
    else if ( m_bIsHiContrast )
    {
        bRefreshImages = sal_True;
        m_bIsHiContrast = sal_False;
    }

    SvtMiscOptions aMiscOptions;
    bool bCurrentSymbolsSmall = !aMiscOptions.AreCurrentSymbolsLarge();
    if ( m_bSmallSymbols != bCurrentSymbolsSmall )
    {
        bRefreshImages = sal_True;
        m_bSmallSymbols = bCurrentSymbolsSmall;
    }

    sal_Int16 nCurrentSymbolsStyle = aMiscOptions.GetCurrentSymbolsStyle();
    if ( m_nSymbolsStyle != nCurrentSymbolsStyle )
    {
        bRefreshImages = sal_True;
        m_nSymbolsStyle = nCurrentSymbolsStyle;
    }

    // Refresh images if requested
    if ( bRefreshImages )
        RefreshImages();
}

void ToolBarManager::RefreshImages()
{
    ResetableGuard aGuard( m_aLock );

    sal_Bool  bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        USHORT nId( m_pToolBar->GetItemId( nPos ) );

        if ( nId > 0 )
        {
            OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
            Image aImage = GetImageFromURL( m_xFrame, aCommandURL, bBigImages, m_bIsHiContrast );
            // Try also to query for add-on images before giving up and use an
            // empty image.
            if ( !aImage )
                aImage = QueryAddonsImage( aCommandURL, bBigImages, m_bIsHiContrast );
            m_pToolBar->SetItemImage( nId, aImage );
        }
    }

    m_pToolBar->SetToolboxButtonSize( bBigImages ? TOOLBOX_BUTTONSIZE_LARGE : TOOLBOX_BUTTONSIZE_SMALL );
    ::Size aSize = m_pToolBar->CalcWindowSizePixel();
    m_pToolBar->SetOutputSizePixel( aSize );
}

void ToolBarManager::UpdateImageOrientation()
{
    ResetableGuard aGuard( m_aLock );

    if ( m_xUICommandLabels.is() )
    {
        sal_Int32 i;
        Sequence< rtl::OUString > aSeqMirrorCmd;
        Sequence< rtl::OUString > aSeqRotateCmd;
        m_xUICommandLabels->getByName(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ))) >>= aSeqMirrorCmd;
        m_xUICommandLabels->getByName(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ))) >>= aSeqRotateCmd;

        CommandToInfoMap::iterator pIter;
        for ( i = 0; i < aSeqMirrorCmd.getLength(); i++ )
        {
            rtl::OUString aMirrorCmd = aSeqMirrorCmd[i];
            pIter = m_aCommandMap.find( aMirrorCmd );
            if ( pIter != m_aCommandMap.end() )
                pIter->second.bMirrored = sal_True;
        }
        for ( i = 0; i < aSeqRotateCmd.getLength(); i++ )
        {
            rtl::OUString aRotateCmd = aSeqRotateCmd[i];
            pIter = m_aCommandMap.find( aRotateCmd );
            if ( pIter != m_aCommandMap.end() )
                pIter->second.bRotated = sal_True;
        }
    }

    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        USHORT nId = m_pToolBar->GetItemId( nPos );
        if ( nId > 0 )
        {
            rtl::OUString aCmd = m_pToolBar->GetItemCommand( nId );

            CommandToInfoMap::const_iterator pIter = m_aCommandMap.find( aCmd );
            if ( pIter != m_aCommandMap.end() )
            {
                if ( pIter->second.bRotated )
                {
                    m_pToolBar->SetItemImageMirrorMode( nId, FALSE );
                    m_pToolBar->SetItemImageAngle( nId, m_lImageRotation );
                }
                if ( pIter->second.bMirrored )
                    m_pToolBar->SetItemImageMirrorMode( nId, m_bImageMirrored );
            }
        }
    }
}

void ToolBarManager::UpdateControllers()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ToolBarManager::UpdateControllers" );

    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = sal_True;
        ToolBarControllerMap::iterator pIter = m_aControllerMap.begin();

        while ( pIter != m_aControllerMap.end() )
        {
            try
            {
                Reference< XUpdatable > xUpdatable( pIter->second, UNO_QUERY );
                if ( xUpdatable.is() )
                    xUpdatable->update();
            }
            catch ( Exception& )
            {
            }
            ++pIter;
        }
    }
    m_bUpdateControllers = sal_False;
}

void ToolBarManager::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );
    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
        m_aAsyncUpdateControllersTimer.Start();
}

void SAL_CALL ToolBarManager::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
throw ( ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );
    if ( m_bDisposed )
        return;

    if ( Event.FeatureURL.Complete.equalsAscii( ".uno:ImageOrientation" ))
    {
        SfxImageItem aItem( 1, 0 );
        aItem.PutValue( Event.State );

        m_lImageRotation = aItem.GetRotation();
        m_bImageMirrored = aItem.IsMirrored();
        UpdateImageOrientation();
    }
}

void SAL_CALL ToolBarManager::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    {
        ResetableGuard aGuard( m_aLock );
        if ( m_bDisposed )
            return;
    }

    RemoveControllers();

    {
        ResetableGuard aGuard( m_aLock );
        if ( m_xDocImageManager.is() )
        {
            try
            {
                m_xDocImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
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
            catch ( Exception& )
            {
            }
        }

        if ( m_xImageOrientationListener.is() )
        {
            ImageOrientationListener* pImageOrientation =
                (ImageOrientationListener*)m_xImageOrientationListener.get();
            pImageOrientation->unbindListener();
            m_xImageOrientationListener.clear();
        }

        m_xDocImageManager.clear();
        m_xModuleImageManager.clear();

        if ( Source.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
            m_xFrame.clear();

        m_xServiceManager.clear();
    }
}

// XComponent
void SAL_CALL ToolBarManager::dispose() throw( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );

        // stop timer to prevent timer events after dispose
        m_aAsyncUpdateControllersTimer.Stop();

        RemoveControllers();

        if ( m_xDocImageManager.is() )
        {
            try
            {
                m_xDocImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
            {
            }
        }
        m_xDocImageManager.clear();
        if ( m_xModuleImageManager.is() )
        {
            try
            {
                m_xModuleImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
            {
            }
        }
        m_xModuleImageManager.clear();

        ImplClearPopupMenu( m_pToolBar );

        // We have to destroy our toolbar instance now.
        Destroy();

        if ( m_bFrameActionRegistered && m_xFrame.is() )
        {
            try
            {
                m_xFrame->removeFrameActionListener( Reference< XFrameActionListener >(
                                                        static_cast< ::cppu::OWeakObject *>( this ), UNO_QUERY ));
            }
            catch ( Exception& )
            {
            }
        }

        if ( m_xImageOrientationListener.is() )
        {
            ImageOrientationListener* pImageOrientation =
                (ImageOrientationListener*)m_xImageOrientationListener.get();
            pImageOrientation->unbindListener();
            m_xImageOrientationListener.clear();
        }

        m_xFrame.clear();
        m_xServiceManager.clear();

        m_bDisposed = sal_True;
    }
}

void SAL_CALL ToolBarManager::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ToolBarManager::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XUIConfigurationListener
void SAL_CALL ToolBarManager::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType = sal_Int16();
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    SvtMiscOptions().AreCurrentSymbolsLarge(),
                                                    m_bIsHiContrast );

    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ) &&
        ( Event.Element >>= xNameAccess ))
    {
        sal_Int16 nImageInfo( 1 );
        Reference< XInterface > xIfacDocImgMgr( m_xDocImageManager, UNO_QUERY );
        if ( xIfacDocImgMgr == Event.Source )
            nImageInfo = 0;

        Sequence< rtl::OUString > aSeq = xNameAccess->getElementNames();
        for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        {
            // Check if we have commands which have an image. We stored for every command
            // from which image manager it got its image. Use only images from this
            // notification if stored nImageInfo >= current nImageInfo!
            rtl::OUString aCommandURL = aSeq[i];
            CommandToInfoMap::iterator pIter = m_aCommandMap.find( aCommandURL );
            if ( pIter != m_aCommandMap.end() && ( pIter->second.nImageInfo >= nImageInfo ))
            {
                Reference< XGraphic > xGraphic;
                if ( xNameAccess->getByName( aSeq[i] ) >>= xGraphic )
                {
                    Image aImage( xGraphic );
                    m_pToolBar->SetItemImage( pIter->second.nId, aImage );
                    if ( pIter->second.aIds.size() > 0 )
                    {
                        for ( sal_uInt32 j=0; j < pIter->second.aIds.size(); j++ )
                            m_pToolBar->SetItemImage( pIter->second.aIds[j], aImage );
                    }
                }
                pIter->second.nImageInfo = nImageInfo;
            }
        }
    }
}

void SAL_CALL ToolBarManager::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType = sal_Int16();
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    SvtMiscOptions().AreCurrentSymbolsLarge(),
                                                    m_bIsHiContrast );

    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ) &&
        ( Event.Element >>= xNameAccess ))
    {
        sal_Int16 nImageInfo( 1 );
        Reference< XInterface > xIfacDocImgMgr( m_xDocImageManager, UNO_QUERY );
        if ( xIfacDocImgMgr == Event.Source )
            nImageInfo = 0;

        Sequence< rtl::OUString > aSeq = xNameAccess->getElementNames();
        for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        {
            CommandToInfoMap::const_iterator pIter = m_aCommandMap.find( aSeq[i] );
            if ( pIter != m_aCommandMap.end() && ( pIter->second.nImageInfo >= nImageInfo ))
            {
                Image aImage;
                if (( pIter->second.nImageInfo == 0 ) && ( pIter->second.nImageInfo == nImageInfo ))
                {
                    // Special case: An image from the document image manager has been removed.
                    // It is possible that we have a image at our module image manager. Before
                    // we can remove our image we have to ask our module image manager.
                    Sequence< rtl::OUString > aCmdURLSeq( 1 );
                    Sequence< Reference< XGraphic > > aGraphicSeq;
                    aCmdURLSeq[0] = pIter->first;
                    aGraphicSeq = m_xModuleImageManager->getImages( nImageType, aCmdURLSeq );
                    aImage = Image( aGraphicSeq[0] );
                }

                m_pToolBar->SetItemImage( pIter->second.nId, aImage );
                if ( pIter->second.aIds.size() > 0 )
                {
                    for ( sal_uInt32 j=0; j < pIter->second.aIds.size(); j++ )
                        m_pToolBar->SetItemImage( pIter->second.aIds[j], aImage );
                }
            }
        }
    }
}

void SAL_CALL ToolBarManager::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType = sal_Int16();
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    SvtMiscOptions().AreCurrentSymbolsLarge(),
                                                    m_bIsHiContrast );

    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ) &&
        ( Event.Element >>= xNameAccess ))
    {
        sal_Int16 nImageInfo( 1 );
        Reference< XInterface > xIfacDocImgMgr( m_xDocImageManager, UNO_QUERY );
        if ( xIfacDocImgMgr == Event.Source )
            nImageInfo = 0;

        Sequence< rtl::OUString > aSeq = xNameAccess->getElementNames();
        for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        {
            // Check if we have commands which have an image. We stored for every command
            // from which image manager it got its image. Use only images from this
            // notification if stored nImageInfo >= current nImageInfo!
            CommandToInfoMap::iterator pIter = m_aCommandMap.find( aSeq[i] );
            if ( pIter != m_aCommandMap.end() && ( pIter->second.nImageInfo >= nImageInfo ))
            {
                Reference< XGraphic > xGraphic;
                if ( xNameAccess->getByName( aSeq[i] ) >>= xGraphic )
                {
                    Image aImage( xGraphic );
                    m_pToolBar->SetItemImage( pIter->second.nId, aImage );
                    if ( pIter->second.aIds.size() > 0 )
                    {
                        for ( sal_uInt32 j=0; j < pIter->second.aIds.size(); j++ )
                            m_pToolBar->SetItemImage( pIter->second.aIds[j], aImage );
                    }
                }
                pIter->second.nImageInfo = nImageInfo;
            }
        }
    }
}

void ToolBarManager::RemoveControllers()
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    m_aSubToolBarControllerMap.clear();

    ToolBarControllerMap::iterator pIter = m_aControllerMap.begin();
    while ( pIter != m_aControllerMap.end() )
    {
        try
        {
            Reference< XComponent > xComponent( pIter->second, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( Exception& )
        {
        }
        ++pIter;
    }
    m_aControllerMap.clear();
}

OUString ToolBarManager::RetrieveLabelFromCommand( const OUString& aCmdURL )
{
    OUString aLabel;

    // Retrieve popup menu labels
    if ( !m_bModuleIdentified )
    {
        Reference< XModuleManager > xModuleManager( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );
        Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
        try
        {
            m_bModuleIdentified = sal_True;
            m_aModuleIdentifier = xModuleManager->identify( xIfac );

            if ( m_aModuleIdentifier.getLength() > 0 )
            {
                Reference< XNameAccess > xNameAccess( m_xServiceManager->createInstance( SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
                if ( xNameAccess.is() )
                {
                    Any a = xNameAccess->getByName( m_aModuleIdentifier );
                    Reference< XNameAccess > xUICommands;
                    a >>= m_xUICommandLabels;
                }
            }
        }
        catch ( Exception& )
        {
        }
    }

    if ( m_xUICommandLabels.is() )
    {
        try
        {
            if ( aCmdURL.getLength() > 0 )
            {
                rtl::OUString aStr;
                Sequence< PropertyValue > aPropSeq;
                Any a( m_xUICommandLabels->getByName( aCmdURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( "Name" ))
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}

void ToolBarManager::CreateControllers()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ToolBarManager::CreateControllers" );

    Reference< XMultiComponentFactory > xToolbarControllerFactory( m_xToolbarControllerRegistration, UNO_QUERY );
    Reference< XComponentContext > xComponentContext;
    Reference< XPropertySet > xProps( m_xServiceManager, UNO_QUERY );
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );
    Reference< css::util::XURLTransformer > xTrans( m_xServiceManager->createInstance(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), css::uno::UNO_QUERY );

    css::util::URL      aURL;
    sal_Bool            bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );
    SvtCommandOptions   aCmdOptions;

    if ( xProps.is() )
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComponentContext;

    for ( USHORT i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        USHORT nId = m_pToolBar->GetItemId( i );
        if ( nId == 0 )
            continue;

        sal_Int16                    nWidth( sal_Int16( m_pToolBar->GetHelpId( nId )));
        rtl::OUString                aLoadURL( RTL_CONSTASCII_USTRINGPARAM( ".uno:OpenUrl" ));
        rtl::OUString                aCommandURL( m_pToolBar->GetItemCommand( nId ));
        sal_Bool                     bInit( sal_True );
        sal_Bool                     bCreate( sal_True );
        Reference< XStatusListener > xController;

        svt::ToolboxController* pController( 0 );

        m_pToolBar->SetHelpId( nId, 0 ); // reset value again
        if ( bHasDisabledEntries )
        {
            aURL.Complete = aCommandURL;
            xTrans->parseStrict( aURL );
            if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aURL.Path ))
            {
                m_aControllerMap[ nId ] = xController;
                m_pToolBar->HideItem( nId );
                continue;
            }
        }

        if ( m_xToolbarControllerRegistration.is() &&
             m_xToolbarControllerRegistration->hasController( aCommandURL, m_aModuleIdentifier ))
        {
            if ( xToolbarControllerFactory.is() )
            {
                PropertyValue aPropValue;
                std::vector< Any > aPropertyVector;

                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                aPropValue.Value    = makeAny( m_aModuleIdentifier );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value    = makeAny( m_xFrame );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value    = makeAny( m_xServiceManager );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                aPropValue.Value    = makeAny( xToolbarWindow );
                aPropertyVector.push_back( makeAny( aPropValue ));

                if ( nWidth > 0 )
                {
                    aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ));
                    aPropValue.Value    = makeAny( nWidth );
                    aPropertyVector.push_back( makeAny( aPropValue ));
                }

                Sequence< Any > aArgs( comphelper::containerToSequence( aPropertyVector ));
                xController = Reference< XStatusListener >( xToolbarControllerFactory->createInstanceWithArgumentsAndContext(
                                                                aCommandURL, aArgs, xComponentContext ),
                                                            UNO_QUERY );
                bInit = sal_False; // Initialization is done through the factory service
            }
        }

        if (( aCommandURL == aLoadURL ) && ( !m_pToolBar->IsItemVisible(nId)))
            bCreate = sal_False;

        if ( !xController.is() && m_pToolBar && bCreate )
        {
            pController = CreateToolBoxController( m_xFrame, m_pToolBar, nId, aCommandURL );
            if ( !pController )
            {
                if ( m_pToolBar->GetItemData( nId ) != 0 )
                {
                    // retrieve additional parameters
                    ::rtl::OUString aControlType = static_cast< AddonsParams* >( m_pToolBar->GetItemData( nId ))->aControlType;

                    Reference< XStatusListener > xStatusListener(
                        ToolBarMerger::CreateController( m_xServiceManager,
                                                         m_xFrame,
                                                         m_pToolBar,
                                                         aCommandURL,
                                                         nId,
                                                         nWidth,
                                                         aControlType ), UNO_QUERY );

                    xController = xStatusListener;
                }
                else
                {
                    xController = Reference< XStatusListener >(
                        new GenericToolbarController( m_xServiceManager, m_xFrame, m_pToolBar, nId, aCommandURL ));
                }
            }
            else if ( pController )
            {
                xController = Reference< XStatusListener >( static_cast< ::cppu::OWeakObject *>( pController ), UNO_QUERY );
            }
        }

        // Associate ID and controller to be able to retrieve
        // the controller from the ID later.
        m_aControllerMap[ nId ] = xController;

        // Fill sub-toolbars into our hash-map
        Reference< XSubToolbarController > xSubToolBar( xController, UNO_QUERY );
        if ( xSubToolBar.is() && xSubToolBar->opensSubToolbar() )
        {
            rtl::OUString aSubToolBarName = xSubToolBar->getSubToolbarName();
            if ( aSubToolBarName.getLength() != 0 )
            {
                SubToolBarToSubToolBarControllerMap::iterator pIter =
                    m_aSubToolBarControllerMap.find( aSubToolBarName );
                if ( pIter == m_aSubToolBarControllerMap.end() )
                {
                    SubToolBarControllerVector aSubToolBarVector;
                    aSubToolBarVector.push_back( xSubToolBar );
                    m_aSubToolBarControllerMap.insert(
                        SubToolBarToSubToolBarControllerMap::value_type(
                            aSubToolBarName, aSubToolBarVector ));
                }
                else
                    pIter->second.push_back( xSubToolBar );
            }
        }

        Reference< XInitialization > xInit( xController, UNO_QUERY );
        if ( xInit.is() )
        {
            if ( bInit )
            {
                PropertyValue aPropValue;
                std::vector< Any > aPropertyVector;

                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value = makeAny( m_xFrame );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
                aPropValue.Value = makeAny( aCommandURL );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value = makeAny( m_xServiceManager );
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                aPropValue.Value = makeAny( xToolbarWindow );
                aPropertyVector.push_back( makeAny( aPropValue ));
                if ( nWidth > 0 )
                {
                    aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ));
                    aPropValue.Value    = makeAny( nWidth );
                    aPropertyVector.push_back( makeAny( aPropValue ));
                }

                Sequence< Any > aArgs( comphelper::containerToSequence( aPropertyVector ));
                xInit->initialize( aArgs );
            }

            // Request a item window from the toolbar controller and set it at the VCL toolbar
            Reference< XToolbarController > xTbxController( xController, UNO_QUERY );
            if ( xTbxController.is() && xToolbarWindow.is() )
            {
                Reference< XWindow > xWindow = xTbxController->createItemWindow( xToolbarWindow );
                if ( xWindow.is() )
                {
                    Window* pItemWin = VCLUnoHelper::GetWindow( xWindow );
                    if ( pItemWin )
                    {
                        WindowType nType = pItemWin->GetType();
                        if ( nType == WINDOW_LISTBOX || nType == WINDOW_MULTILISTBOX || nType == WINDOW_COMBOBOX )
                            pItemWin->SetAccessibleName( m_pToolBar->GetItemText( nId ) );
                        m_pToolBar->SetItemWindow( nId, pItemWin );
                    }
                }
            }
        }
    }

    AddFrameActionListener();
    AddImageOrientationListener();
}

void ToolBarManager::AddFrameActionListener()
{
    if ( !m_bFrameActionRegistered && m_xFrame.is() )
    {
        m_bFrameActionRegistered = sal_True;
        m_xFrame->addFrameActionListener( Reference< XFrameActionListener >(
                                            static_cast< ::cppu::OWeakObject *>( this ), UNO_QUERY ));
    }
}

void ToolBarManager::AddImageOrientationListener()
{
    if ( !m_bImageOrientationRegistered && m_xFrame.is() )
    {
        m_bImageOrientationRegistered = sal_True;
        ImageOrientationListener* pImageOrientation = new ImageOrientationListener(
            Reference< XStatusListener >( static_cast< ::cppu::OWeakObject *>( this ), UNO_QUERY ),
            m_xServiceManager,
            m_xFrame );
        m_xImageOrientationListener = Reference< XComponent >( static_cast< ::cppu::OWeakObject *>(
                                        pImageOrientation ), UNO_QUERY );
        pImageOrientation->addStatusListener(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ImageOrientation" )));
        pImageOrientation->bindListener();
    }
}

sal_uInt16 ToolBarManager::ConvertStyleToToolboxItemBits( sal_Int32 nStyle )
{
    sal_uInt16 nItemBits( 0 );
    if ( nStyle & ::com::sun::star::ui::ItemStyle::RADIO_CHECK )
        nItemBits |= TIB_RADIOCHECK;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::ALIGN_LEFT )
        nItemBits |= TIB_LEFT;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::AUTO_SIZE )
        nItemBits |= TIB_AUTOSIZE;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::DROP_DOWN )
        nItemBits |= TIB_DROPDOWN;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::REPEAT )
        nItemBits |= TIB_REPEAT;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::DROPDOWN_ONLY )
        nItemBits |= TIB_DROPDOWNONLY;

    return nItemBits;
}

void ToolBarManager::FillToolbar( const Reference< XIndexAccess >& rItemContainer )
{
    OString aTbxName = rtl::OUStringToOString( m_aResourceName, RTL_TEXTENCODING_ASCII_US );

    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ToolBarManager::FillToolbar" );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "framework (cd100003) ::ToolBarManager::FillToolbar %s", aTbxName.getStr() );

    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    USHORT    nId( 1 );
    OUString  aHelpIdPrefix( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX ));

    Reference< XModuleManager > xModuleManager( Reference< XModuleManager >(
                                                    m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ));
    if ( !m_xDocImageManager.is() )
    {
        Reference< XModel > xModel( GetModelFromFrame() );
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

    try
    {
        if ( xModuleManager.is() )
            m_aModuleIdentifier = xModuleManager->identify( Reference< XInterface >( m_xFrame, UNO_QUERY ) );
    }
    catch( Exception& )
    {
    }

    if ( !m_xModuleImageManager.is() )
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier( m_xServiceManager->createInstance(
                                                                                    SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ),
                                                                                  UNO_QUERY );
        Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
        m_xModuleImageManager = Reference< XImageManager >( xUICfgMgr->getImageManager(), UNO_QUERY );
        m_xModuleImageManager->addConfigurationListener( Reference< XUIConfigurationListener >(
                                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }

    RemoveControllers();

    // reset and fill command map
    m_pToolBar->Clear();
    m_aControllerMap.clear();
    m_aCommandMap.clear();

    CommandInfo aCmdInfo;
    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >   aProp;
        rtl::OUString               aCommandURL;
        rtl::OUString               aLabel;
        rtl::OUString               aHelpURL;
        sal_uInt16                  nType( ::com::sun::star::ui::ItemType::DEFAULT );
        sal_uInt16                  nWidth( 0 );
        sal_Bool                    bIsVisible( sal_True );
        sal_uInt32                  nStyle( 0 );

        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_COMMANDURL, ITEM_DESCRIPTOR_COMMANDURL_LEN ))
                        aProp[i].Value >>= aCommandURL;
                    else if (  aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_HELPURL, ITEM_DESCRIPTOR_HELPURL_LEN ))
                        aProp[i].Value >>= aHelpURL;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_LABEL, ITEM_DESCRIPTOR_LABEL_LEN ))
                        aProp[i].Value >>= aLabel;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_TYPE, ITEM_DESCRIPTOR_TYPE_LEN ))
                        aProp[i].Value >>= nType;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_VISIBLE, ITEM_DESCRIPTOR_VISIBLE_LEN ))
                        aProp[i].Value >>= bIsVisible;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_WIDTH, ITEM_DESCRIPTOR_WIDTH_LEN ))
                        aProp[i].Value >>= nWidth;
                    else if ( aProp[i].Name.equalsAsciiL( ITEM_DESCRIPTOR_STYLE, ITEM_DESCRIPTOR_STYLE_LEN ))
                        aProp[i].Value >>= nStyle;
                }

                if (( nType == ::com::sun::star::ui::ItemType::DEFAULT ) && ( aCommandURL.getLength() > 0 ))
                {
                    OUString aString( RetrieveLabelFromCommand( aCommandURL ));

                    sal_uInt16 nItemBits = ConvertStyleToToolboxItemBits( nStyle );
                    m_pToolBar->InsertItem( nId, aString, nItemBits );
                    m_pToolBar->SetItemCommand( nId, aCommandURL );
                    m_pToolBar->SetQuickHelpText( nId, aString );
                    if ( aLabel.getLength() > 0 )
                        m_pToolBar->SetItemText( nId, aLabel );
                    else
                        m_pToolBar->SetItemText( nId, aString );
                    m_pToolBar->EnableItem( nId, sal_True );
                    m_pToolBar->SetItemState( nId, STATE_NOCHECK );

                    // Fill command map. It stores all our commands and from what
                    // image manager we got our image. So we can decide if we have to use an
                    // image from a notification message.
                    CommandToInfoMap::iterator pIter = m_aCommandMap.find( aCommandURL );
                    if ( pIter == m_aCommandMap.end())
                    {
                        aCmdInfo.nId = nId;
                        m_aCommandMap.insert( CommandToInfoMap::value_type( aCommandURL, aCmdInfo ));
                    }
                    else
                    {
                        pIter->second.aIds.push_back( nId );
                    }

                    // Add additional information for the controller to the obsolete help id
                    m_pToolBar->SetHelpId( ULONG( nWidth ));

                    if ( !bIsVisible )
                        m_pToolBar->HideItem( nId );

                    ++nId;
                }
                else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_LINE )
                {
                    m_pToolBar->InsertSeparator();
                }
                else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_SPACE )
                {
                    m_pToolBar->InsertSpace();
                }
                else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_LINEBREAK )
                {
                    m_pToolBar->InsertBreak();
                }
            }
        }
        catch ( ::com::sun::star::lang::IndexOutOfBoundsException& )
        {
            break;
        }
    }

    // Support add-on toolbar merging here. Working directly on the toolbar object is much
    // simpler and faster.
    const sal_uInt16 TOOLBAR_ITEM_STARTID = 1000;

    MergeToolbarInstructionContainer aMergeInstructionContainer;

    // Retrieve the toolbar name from the resource name
    ::rtl::OUString aToolbarName( m_aResourceName );
    sal_Int32 nIndex = aToolbarName.lastIndexOf( '/' );
    if (( nIndex > 0 ) && ( nIndex < aToolbarName.getLength() ))
        aToolbarName = aToolbarName.copy( nIndex+1 );

    AddonsOptions().GetMergeToolbarInstructions( aToolbarName, aMergeInstructionContainer );

    if ( aMergeInstructionContainer.size() > 0 )
    {
        sal_uInt16 nItemId( TOOLBAR_ITEM_STARTID );
        for ( sal_uInt32 i = 0; i < aMergeInstructionContainer.size(); i++ )
        {
            MergeToolbarInstruction& rInstruction = aMergeInstructionContainer[i];
            if ( ToolBarMerger::IsCorrectContext( rInstruction.aMergeContext, m_aModuleIdentifier ))
            {
                ReferenceToolbarPathInfo aRefPoint = ToolBarMerger::FindReferencePoint( m_pToolBar, rInstruction.aMergePoint );

                // convert the sequence< sequence< propertyvalue > > structure to
                // something we can better handle. A vector with item data
                AddonToolbarItemContainer aItems;
                ToolBarMerger::ConvertSeqSeqToVector( rInstruction.aMergeToolbarItems, aItems );

                if ( aRefPoint.bResult )
                {
                    ToolBarMerger::ProcessMergeOperation( m_xFrame,
                                                          m_pToolBar,
                                                          aRefPoint.nPos,
                                                          nItemId,
                                                          m_aCommandMap,
                                                          m_aModuleIdentifier,
                                                          rInstruction.aMergeCommand,
                                                          rInstruction.aMergeCommandParameter,
                                                          aItems );
                }
                else
                {
                    ToolBarMerger::ProcessMergeFallback( m_xFrame,
                                                         m_pToolBar,
                                                         aRefPoint.nPos,
                                                         nItemId,
                                                         m_aCommandMap,
                                                         m_aModuleIdentifier,
                                                         rInstruction.aMergeCommand,
                                                         rInstruction.aMergeFallback,
                                                         aItems );
                }
            }
        }
    }

    // Request images for all toolbar items. Must be done before CreateControllers as
    // some controllers need access to the image.
    RequestImages();

    // Create controllers after we set the images. There are controllers which needs
    // an image at the toolbar at creation time!
    CreateControllers();

    // Notify controllers that they are now correctly initialized and can start listening
    // toolbars that will open in popup mode will be updated immediately to avoid flickering
    if( m_pToolBar->WillUsePopupMode() )
        UpdateControllers();
    else if ( m_pToolBar->IsReallyVisible() )
        m_aAsyncUpdateControllersTimer.Start();

    // Try to retrieve UIName from the container property set and set it as the title
    // if it is not empty.
    Reference< XPropertySet > xPropSet( rItemContainer, UNO_QUERY );
    if ( xPropSet.is() )
    {
        Any a;
        try
        {
            rtl::OUString aUIName;
            xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= aUIName;
            if ( aUIName.getLength() > 0 )
                m_pToolBar->SetText( aUIName );
        }
        catch ( Exception& )
        {
        }
    }
}

void ToolBarManager::RequestImages()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ToolBarManager::RequestImages" );

    // Request images from image manager
    Sequence< rtl::OUString > aCmdURLSeq( m_aCommandMap.size() );
    Sequence< Reference< XGraphic > > aDocGraphicSeq;
    Sequence< Reference< XGraphic > > aModGraphicSeq;

    sal_uInt32 i = 0;
    CommandToInfoMap::iterator pIter = m_aCommandMap.begin();
    while ( pIter != m_aCommandMap.end() )
    {
        aCmdURLSeq[i++] = pIter->first;
        ++pIter;
    }

    sal_Bool  bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    m_bIsHiContrast = m_pToolBar->GetSettings().GetStyleSettings().GetFaceColor().IsDark();
    sal_Int16 p = getImageTypeFromBools( SvtMiscOptions().AreCurrentSymbolsLarge(), m_bIsHiContrast );

    if ( m_xDocImageManager.is() )
        aDocGraphicSeq = m_xDocImageManager->getImages( p, aCmdURLSeq );
    aModGraphicSeq = m_xModuleImageManager->getImages( p, aCmdURLSeq );

    i = 0;
    pIter = m_aCommandMap.begin();
    while ( pIter != m_aCommandMap.end() )
    {
        rtl::OUString aCommandURL = aCmdURLSeq[i];

        Image aImage;
        if ( aDocGraphicSeq.getLength() > 0 )
            aImage = Image( aDocGraphicSeq[i] );
        if ( !aImage )
        {
            aImage = Image( aModGraphicSeq[i] );
            // Try also to query for add-on images before giving up and use an
            // empty image.
            if ( !aImage )
                aImage = QueryAddonsImage( aCmdURLSeq[i], bBigImages, m_bIsHiContrast );
            m_pToolBar->SetItemImage( pIter->second.nId, aImage );
            if ( pIter->second.aIds.size() > 0 )
            {
                for ( sal_uInt32 j=0; j < pIter->second.aIds.size(); j++ )
                    m_pToolBar->SetItemImage( pIter->second.aIds[j], aImage );
            }
            pIter->second.nImageInfo = 1; // mark image as module based
        }
        else
        {
            m_pToolBar->SetItemImage( pIter->second.nId, aImage );
            if ( pIter->second.aIds.size() > 0 )
            {
                for ( sal_uInt32 j=0; j < pIter->second.aIds.size(); j++ )
                    m_pToolBar->SetItemImage( pIter->second.aIds[j], aImage );
            }
            pIter->second.nImageInfo = 0; // mark image as document based
        }
        ++pIter;
        ++i;
    }
}

void ToolBarManager::notifyRegisteredControllers( const rtl::OUString& aUIElementName, const rtl::OUString& aCommand )
{
    ResetableGuard aGuard( m_aLock );
    if ( m_aSubToolBarControllerMap.size() > 0 )
    {
        SubToolBarToSubToolBarControllerMap::const_iterator pIter =
            m_aSubToolBarControllerMap.find( aUIElementName );

        if ( pIter != m_aSubToolBarControllerMap.end() )
        {
            const SubToolBarControllerVector& rSubToolBarVector = pIter->second;
            if ( rSubToolBarVector.size() > 0 )
            {
                SubToolBarControllerVector aNotifyVector = rSubToolBarVector;
                aGuard.unlock();

                for ( sal_uInt32 i = 0; i < aNotifyVector.size(); i++ )
                {
                    try
                    {
                        Reference< XSubToolbarController > xController = aNotifyVector[i];
                        if ( xController.is() )
                            xController->functionSelected( aCommand );
                    }
                    catch ( RuntimeException& e )
                    {
                        throw e;
                    }
                    catch ( Exception& )
                    {
                    }
                }
            }
        }
    }
}

IMPL_LINK( ToolBarManager, Click, ToolBox*, EMPTYARG )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK( ToolBarManager, DropdownClick, ToolBox*, EMPTYARG )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->createPopupWindow();
    }

    return 1;
}

IMPL_LINK( ToolBarManager, DoubleClick, ToolBox*, EMPTYARG )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->doubleClick();
    }

    return 1;
}

void ToolBarManager::ImplClearPopupMenu( ToolBox *pToolBar )
{
    if ( m_bDisposed )
        return;

    PopupMenu *pMenu = pToolBar->GetMenu();

    // remove config entries from menu, so we have a clean menu to start with
    // remove submenu first
    PopupMenu*  pItemMenu = pMenu->GetPopupMenu( 1 );
    if( pItemMenu )
    {
        pItemMenu->Clear();
        delete pItemMenu;
        pItemMenu = NULL;
        pMenu->SetPopupMenu( 1, pItemMenu );
    }

    // remove all items that were not added by the toolbar itself
    USHORT i;
    for( i=0; i<pMenu->GetItemCount(); )
    {
        if( pMenu->GetItemId( i ) < TOOLBOX_MENUITEM_START )
            pMenu->RemoveItem( i );
        else
            i++;
    }
}

IMPL_LINK( ToolBarManager, MenuDeactivate, Menu*, pMenu )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    if( pMenu != m_pToolBar->GetMenu() )
        return 1;

    ImplClearPopupMenu( m_pToolBar );

    return 0;
}

Reference< XModel > ToolBarManager::GetModelFromFrame() const
{
    Reference< XController > xController = m_xFrame->getController();
    Reference< XModel > xModel;
    if ( xController.is() )
        xModel = xController->getModel();

    return xModel;
}

sal_Bool ToolBarManager::IsPluginMode() const
{
    sal_Bool bPluginMode( sal_False );

    if ( m_xFrame.is() )
    {
        Reference< XModel > xModel = GetModelFromFrame();
        if ( xModel.is() )
        {
            Sequence< PropertyValue > aSeq = xModel->getArgs();
            comphelper::MediaDescriptor aMediaDescriptor( aSeq );
            bPluginMode = aMediaDescriptor.getUnpackedValueOrDefault< sal_Bool >(
                            comphelper::MediaDescriptor::PROP_VIEWONLY(), sal_False );
        }
    }

    return bPluginMode;
}

bool ToolBarManager::MenuItemAllowed( sal_uInt16 ) const
{
    return true;
}

IMPL_LINK( ToolBarManager, MenuButton, ToolBox*, pToolBar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    PopupMenu *pMenu = pToolBar->GetMenu();

    // remove all entries before inserting new ones
    ImplClearPopupMenu( pToolBar );

    // No config menu entries if command ".uno:ConfigureDialog" is not enabled
    Reference< XDispatch > xDisp;
    com::sun::star::util::URL aURL;
    if ( m_xFrame.is() )
    {
        Reference< XDispatchProvider > xProv( m_xFrame, UNO_QUERY );
        Reference< XURLTransformer > xTrans( m_xServiceManager->createInstance(
                                                OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        aURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ));
        xTrans->parseStrict( aURL );
        if ( xProv.is() )
            xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );

        if ( !xDisp.is() || IsPluginMode() )
            return 1;
    }

    // popup menu for quick customization
    sal_Bool bHideDisabledEntries = !SvtMenuOptions().IsEntryHidingEnabled();
    PopupMenu aPopupMenu( FwkResId( POPUPMENU_TOOLBAR_QUICKCUSTOMIZATION ));

    if ( m_pToolBar->IsCustomize() )
    {
        USHORT      nPos( 0 );
        PopupMenu*  pItemMenu( aPopupMenu.GetPopupMenu( 1 ));

        sal_Bool    bIsFloating( sal_False );

        DockingManager* pDockMgr = Window::GetDockingManager();
        if ( pDockMgr )
            bIsFloating = pDockMgr->IsFloating( m_pToolBar );

        if ( !bIsFloating )
        {
            aPopupMenu.EnableItem( MENUITEM_TOOLBAR_DOCKTOOLBAR, sal_False );
            aPopupMenu.EnableItem( MENUITEM_TOOLBAR_DOCKALLTOOLBAR, sal_False );
            Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );
            if( xDockable.is() )
                aPopupMenu.CheckItem( MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, xDockable->isLocked() );
        }
        else
            aPopupMenu.EnableItem( MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, sal_False );

        if ( !m_bCanBeCustomized )
        {
            // Non-configurable toolbars should disable configuration menu items
            aPopupMenu.EnableItem( MENUITEM_TOOLBAR_VISIBLEBUTTON, sal_False );
            aPopupMenu.EnableItem( MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR, sal_False );
        }

        pItemMenu->SetMenuFlags (pItemMenu->GetMenuFlags () |
                                 MENU_FLAG_SHOWCHECKIMAGES);

        for ( nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
        {
            if ( m_pToolBar->GetItemType(nPos) == TOOLBOXITEM_BUTTON )
            {
                USHORT nId = m_pToolBar->GetItemId(nPos);
                OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
                pItemMenu->InsertItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->GetItemText( nId ), MIB_CHECKABLE );
                pItemMenu->CheckItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->IsItemVisible( nId ) );
                pItemMenu->SetItemCommand( STARTID_CUSTOMIZE_POPUPMENU+nPos, aCommandURL );
                pItemMenu->SetItemImage( STARTID_CUSTOMIZE_POPUPMENU+nPos,
                                         GetImageFromURL( m_xFrame,
                                                          aCommandURL,
                                                          sal_False,
                                                          m_bIsHiContrast ));
            }
            else
            {
                pItemMenu->InsertSeparator();
            }
        }
    }
    else
    {
        USHORT nPos = aPopupMenu.GetItemPos( MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR );
        if ( nPos != MENU_ITEM_NOTFOUND )
            aPopupMenu.RemoveItem( nPos );
    }

    // copy all menu items to the toolbar menu
    if( pMenu->GetItemCount() )
        pMenu->InsertSeparator();

    USHORT i;
    for( i=0; i< aPopupMenu.GetItemCount(); i++)
    {
        sal_uInt16 nId = aPopupMenu.GetItemId( i );
        if ( MenuItemAllowed( nId ))
            pMenu->CopyItem( aPopupMenu, i, MENU_APPEND );
    }

    // set submenu to toolbar menu
    if( aPopupMenu.GetPopupMenu( 1 ) )
    {
        // create an own submenu to avoid auto-delete when resource menu is deleted
        PopupMenu *pItemMenu = new PopupMenu();

        pItemMenu->SetMenuFlags (pItemMenu->GetMenuFlags () |
                                 MENU_FLAG_SHOWCHECKIMAGES);

        for( i=0; i< aPopupMenu.GetPopupMenu( 1 )->GetItemCount(); i++)
            pItemMenu->CopyItem( *aPopupMenu.GetPopupMenu( 1 ), i, MENU_APPEND );

        pMenu->SetPopupMenu( 1, pItemMenu );
    }

    if ( bHideDisabledEntries )
        pMenu->RemoveDisabledEntries();

    return 0;
}

IMPL_LINK( ToolBarManager, MenuSelect, Menu*, pMenu )
{
    // We have to hold a reference to ourself as it is possible that we will be disposed and
    // our refcount could be zero (destruction) otherwise.
    Reference< XInterface > xInterface( static_cast< OWeakObject* >( this ), UNO_QUERY );

    {
        // The guard must be in its own context as the we can get destroyed when our
        // own xInterface reference get destroyed!
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            return 1;

        switch ( pMenu->GetCurItemId() )
        {
            case MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR:
            {
                Reference< XDispatch > xDisp;
                com::sun::star::util::URL aURL;
                if ( m_xFrame.is() )
                {
                    Reference< XDispatchProvider > xProv( m_xFrame, UNO_QUERY );
                    Reference< XURLTransformer > xTrans( m_xServiceManager->createInstance(
                                                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                            "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                    aURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ));
                    xTrans->parseStrict( aURL );
                    if ( xProv.is() )
                        xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
                }

                if ( xDisp.is() )
                {
                    Sequence< PropertyValue > aPropSeq( 1 );

                    aPropSeq[ 0 ].Name =
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ResourceURL"));
                    aPropSeq[ 0 ].Value <<= m_aResourceName;

                    xDisp->dispatch( aURL, aPropSeq );
                }
                break;
            }

            case MENUITEM_TOOLBAR_DOCKTOOLBAR:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_DOCKTOOLBAR;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );

                Application::PostUserEvent( STATIC_LINK(0, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
                break;
            }

            case MENUITEM_TOOLBAR_DOCKALLTOOLBAR:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_DOCKALLTOOLBARS;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );

                Application::PostUserEvent( STATIC_LINK(0, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
                break;
            }

            case MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION:
            {
                Reference< XLayoutManager > xLayoutManager = getLayoutManagerFromFrame( m_xFrame );
                if ( xLayoutManager.is() )
                {
                    Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );

                    if( xDockable->isLocked() )
                        xLayoutManager->unlockWindow( m_aResourceName );
                    else
                        xLayoutManager->lockWindow( m_aResourceName );
                }
                break;
            }

            case MENUITEM_TOOLBAR_CLOSE:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_CLOSETOOLBAR;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );
                pExecuteInfo->xWindow         = VCLUnoHelper::GetInterface( m_pToolBar );

                Application::PostUserEvent( STATIC_LINK(0, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
            }

            default:
            {
                USHORT nId = pMenu->GetCurItemId();
                if(( nId > 0 ) && ( nId < TOOLBOX_MENUITEM_START ))
                {
                    // toggle toolbar button visibility
                    rtl::OUString aCommand = pMenu->GetItemCommand( nId );

                    Reference< XLayoutManager > xLayoutManager = getLayoutManagerFromFrame( m_xFrame );
                    if ( xLayoutManager.is() )
                    {
                        Reference< XUIElementSettings > xUIElementSettings( xLayoutManager->getElement( m_aResourceName ), UNO_QUERY );
                        if ( xUIElementSettings.is() )
                        {
                            Reference< XIndexContainer > xItemContainer( xUIElementSettings->getSettings( sal_True ), UNO_QUERY );
                            sal_Int32 nCount = xItemContainer->getCount();
                            for ( sal_Int32 i = 0; i < nCount; i++ )
                            {
                                Sequence< PropertyValue > aProp;
                                sal_Int32                 nVisibleIndex( -1 );
                                rtl::OUString             aCommandURL;
                                sal_Bool                  bVisible( sal_False );

                                if ( xItemContainer->getByIndex( i ) >>= aProp )
                                {
                                    for ( sal_Int32 j = 0; j < aProp.getLength(); j++ )
                                    {
                                        if ( aProp[j].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
                                        {
                                            aProp[j].Value >>= aCommandURL;
                                        }
                                        else if ( aProp[j].Name.equalsAscii( ITEM_DESCRIPTOR_VISIBLE ))
                                        {
                                            aProp[j].Value >>= bVisible;
                                            nVisibleIndex = j;
                                        }
                                    }

                                    if (( aCommandURL == aCommand ) && ( nVisibleIndex >= 0 ))
                                    {
                                        // We have found the requested item, toggle the visible flag
                                        // and write back the configuration settings to the toolbar
                                        aProp[nVisibleIndex].Value = makeAny( !bVisible );
                                        try
                                        {
                                            xItemContainer->replaceByIndex( i, makeAny( aProp ));
                                            xUIElementSettings->setSettings( Reference< XIndexAccess >( xItemContainer, UNO_QUERY ));
                                            Reference< XPropertySet > xPropSet( xUIElementSettings, UNO_QUERY );
                                            if ( xPropSet.is() )
                                            {
                                                Reference< XUIConfigurationPersistence > xUICfgMgr;
                                                Any a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                        "ConfigurationSource" )));
                                                if (( a >>= xUICfgMgr ) && ( xUICfgMgr.is() ))
                                                    xUICfgMgr->store();
                                            }
                                        }
                                        catch ( Exception& )
                                        {
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        }

        // remove all entries - deactivate is not reliable
        // The method checks if we are already disposed and in that case does nothing!
        ImplClearPopupMenu( m_pToolBar );
    }

    return 1;
}

IMPL_LINK( ToolBarManager, Select, ToolBox*, EMPTYARG )
{
    if ( m_bDisposed )
        return 1;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    USHORT      nId( m_pToolBar->GetCurItemId() );

    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->execute( nKeyModifier );
    }

    return 1;
}

IMPL_LINK( ToolBarManager, Highlight, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( ToolBarManager, Activate, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( ToolBarManager, Deactivate, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( ToolBarManager, StateChanged, StateChangedType*, pStateChangedType )
{
    if ( m_bDisposed )
        return 1;

    if ( *pStateChangedType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        // Check if we need to get new images for normal/high contrast mode
        CheckAndUpdateImages();
    }
    else if ( *pStateChangedType == STATE_CHANGE_VISIBLE )
    {
        if ( m_pToolBar->IsReallyVisible() )
            m_aAsyncUpdateControllersTimer.Start();
    }
    else if ( *pStateChangedType == STATE_CHANGE_INITSHOW )
    {
        m_aAsyncUpdateControllersTimer.Start();
    }
    return 1;
}

IMPL_LINK( ToolBarManager, DataChanged, DataChangedEvent*, pDataChangedEvent  )
{
    if ((( pDataChangedEvent->GetType() == DATACHANGED_SETTINGS )   ||
        (  pDataChangedEvent->GetType() == DATACHANGED_DISPLAY  ))  &&
        ( pDataChangedEvent->GetFlags() & SETTINGS_STYLE        ))
    {
        // Check if we need to get new images for normal/high contrast mode
        CheckAndUpdateImages();
    }

    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
    {
        const USHORT nId = m_pToolBar->GetItemId(nPos);
        Window* pWindow = m_pToolBar->GetItemWindow( nId );
        if ( pWindow )
        {
            const DataChangedEvent& rDCEvt( *pDataChangedEvent );
            pWindow->DataChanged( rDCEvt );
        }
    }

    if ( !m_pToolBar->IsFloatingMode() &&
         m_pToolBar->IsVisible() )
    {
        // Resize toolbar, layout manager is resize listener and will calc
        // the layout automatically.
        ::Size aSize( m_pToolBar->CalcWindowSizePixel() );
        m_pToolBar->SetOutputSizePixel( aSize );
    }

    return 1;
}

IMPL_LINK( ToolBarManager, AsyncUpdateControllersHdl, Timer *, EMPTYARG )
{
    // The guard must be in its own context as the we can get destroyed when our
    // own xInterface reference get destroyed!
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    // Request to update our controllers
    m_aAsyncUpdateControllersTimer.Stop();
    UpdateControllers();

    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( ToolBarManager, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        if (( pExecuteInfo->nCmd == EXEC_CMD_CLOSETOOLBAR ) &&
            ( pExecuteInfo->xLayoutManager.is() ) &&
            ( pExecuteInfo->xWindow.is() ))
        {
            // Use docking window close to close the toolbar. The layout manager is
            // listener and will react correctly according to the context sensitive
            // flag of our toolbar.
            Window* pWin = VCLUnoHelper::GetWindow( pExecuteInfo->xWindow );
            DockingWindow* pDockWin = dynamic_cast< DockingWindow* >( pWin );
            if ( pDockWin )
                pDockWin->Close();
        }
        else if (( pExecuteInfo->nCmd == EXEC_CMD_DOCKTOOLBAR ) &&
                 ( pExecuteInfo->xLayoutManager.is() ))
        {
            ::com::sun::star::awt::Point aPoint;
            aPoint.X = aPoint.Y = SAL_MAX_INT32;
            pExecuteInfo->xLayoutManager->dockWindow( pExecuteInfo->aToolbarResName,
                                                      DockingArea_DOCKINGAREA_DEFAULT,
                                                      aPoint );
        }
        else if (( pExecuteInfo->nCmd == EXEC_CMD_DOCKALLTOOLBARS ) &&
                 ( pExecuteInfo->xLayoutManager.is() ))
        {
            pExecuteInfo->xLayoutManager->dockAllWindows( UIElementType::TOOLBAR );
        }
    }
    catch ( Exception& )
    {
    }

    delete pExecuteInfo;
    return 0;
}

Image ToolBarManager::QueryAddonsImage( const ::rtl::OUString& aCommandURL, bool bBigImages, bool bHiContrast )
{
    Image aImage = framework::AddonsOptions().GetImageFromURL( aCommandURL, bBigImages, bHiContrast );
    return aImage;
}

}
