/*************************************************************************
 *
 *  $RCSfile: toolbarmanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 17:22:50 $
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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#include <uielement/toolbarmanager.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________


#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX
#include <uielement/toolbar.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX
#include <uielement/generictoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif
#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif
#ifndef __FRAMEWORK_GENERAL_H_
#include "general.h"
#endif
#ifndef __FRAMEWORK_PROPERTIES_H_
#include "properties.h"
#endif
#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
#include <classes/sfxhelperfunctions.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif
#ifndef __FRAMEWORK_CLASES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _DRAFTS_COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <drafts/com/sun/star/ui/ItemType.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTOOLBARCONTROLLER_HPP_
#include <com/sun/star/frame/XToolbarController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XLAYOUTMANAGER_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOW_HPP_
#include <com/sun/star/awt/XDockableWindow.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XDOCKINGAREA_HPP_
#include <drafts/com/sun/star/ui/DockingArea.hpp>
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENTSETTINGS_HPP_
#include <drafts/com/sun/star/ui/XUIElementSettings.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONPERSISTENCE_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationPersistence.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <drafts/com/sun/star/ui/ImageType.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_UIELEMENTTYPE_HPP_
#include <drafts/com/sun/star/ui/UIElementType.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif

#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <svtools/miscopt.hxx>
#include <svtools/imageitm.hxx>
#include <svtools/framestatuslistener.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

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
using namespace ::drafts::com::sun::star::frame;
using namespace ::drafts::com::sun::star::ui;

namespace framework
{

static const char   ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char   ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char   ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char   ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char   ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char   ITEM_DESCRIPTOR_VISIBLE[]     = "IsVisible";
static const char   HELPID_PREFIX[]               = "helpid:";
static const char   HELPID_PREFIX_TESTTOOL[]      = ".HelpId:";
static sal_Int32    HELPID_PREFIX_LENGTH          = 7;
static const USHORT STARTID_CUSTOMIZE_POPUPMENU   = 1000;


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
        n |= ::drafts::com::sun::star::ui::ImageType::SIZE_LARGE;
    if ( bHighContrast )
        n |= ::drafts::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;
    return n;
}

static void ImplClearPopupMenu( ToolBox *pToolBar );

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_6                     (   ToolBarManager                                                                                              ,
                                            OWeakObject                                                                                                 ,
                                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider                                                     ),
                                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent                                                        ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XFrameActionListener                                             ),
                                            DIRECT_INTERFACE( ::drafts::com::sun::star::ui::XUIConfigurationListener                                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XStatusListener                                                  ),
                                            DERIVED_INTERFACE( ::com::sun::star::lang::XEventListener, ::com::sun::star::frame::XFrameActionListener    )
                                        )

DEFINE_XTYPEPROVIDER_6                  (   ToolBarManager                                          ,
                                            ::com::sun::star::lang::XTypeProvider                   ,
                                            ::com::sun::star::lang::XComponent                      ,
                                            ::drafts::com::sun::star::ui::XUIConfigurationListener  ,
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
    m_aListenerContainer( m_aLock.getShareableOslMutex() ),
    m_xServiceManager( rServiceManager ),
    m_xFrame( rFrame ),
    m_pToolBar( pToolBar ),
    m_bDisposed( sal_False ),
    m_bIsHiContrast( pToolBar->GetBackground().GetColor().IsDark() ),
    m_bSmallSymbols( GetCurrentSymbolSize() == SFX_SYMBOLS_SMALL ),
    m_bModuleIdentified( sal_False ),
    m_aResourceName( rResourceName ),
    m_bAddedToTaskPaneList( sal_True ),
    m_bFrameActionRegistered( sal_False ),
    m_bUpdateControllers( sal_False ),
    m_bImageOrientationRegistered( sal_False ),
    m_bImageMirrored( sal_False ),
    m_lImageRotation( 0 )
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
    m_pToolBar->SetMenuType( TOOLBOX_MENUTYPE_CLIPPEDITEMS | TOOLBOX_MENUTYPE_CUSTOMIZE );
    m_pToolBar->SetMenuButtonHdl( LINK( this, ToolBarManager, MenuButton ) );
    m_pToolBar->GetMenu()->SetSelectHdl( LINK( this, ToolBarManager, MenuSelect ) );
    m_pToolBar->GetMenu()->SetDeactivateHdl( LINK( this, ToolBarManager, MenuDeactivate ) );

    // set name for testtool, the useful part is after the last '/'
    sal_Int32 idx = rResourceName.lastIndexOf('/');
    idx++; // will become 0 if '/' not found: use full string
    OUString  aHelpIdAsString( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX_TESTTOOL ));
    aHelpIdAsString += rResourceName.copy( idx );
    m_pToolBar->SetHelpIdAsString( aHelpIdAsString );
}

ToolBarManager::~ToolBarManager()
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
    delete m_pToolBar;
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
    if ( m_pToolBar->GetBackground().GetColor().IsDark() )
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
    else if ( m_bSmallSymbols != ( GetCurrentSymbolSize() == SFX_SYMBOLS_SMALL ))
    {
        bRefreshImages = sal_True;
        m_bSmallSymbols = ( GetCurrentSymbolSize() == SFX_SYMBOLS_SMALL );
    }

    // Refresh images if requested
    if ( bRefreshImages )
        RefreshImages();
}

void ToolBarManager::RefreshImages()
{
    ResetableGuard aGuard( m_aLock );

    sal_Int16 nSymbolSet = GetCurrentSymbolSize();
    sal_Bool  bBigImages( nSymbolSet == SFX_SYMBOLS_LARGE );
    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        USHORT nId( m_pToolBar->GetItemId( nPos ) );

        if ( nId > 0 )
        {
            OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
            m_pToolBar->SetItemImage( nId, GetImageFromURL( m_xFrame,
                                                            aCommandURL,
                                                            bBigImages,
                                                            m_bIsHiContrast ));
        }
    }

    m_pToolBar->SetToolboxButtonSize( bBigImages ? TOOLBOX_BUTTONSIZE_LARGE : TOOLBOX_BUTTONSIZE_SMALL );
    ::Size aSize = m_pToolBar->CalcWindowSizePixel();
    m_pToolBar->SetSizePixel( aSize );
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
    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = sal_True;
        for ( sal_uInt32 n = 0; n < m_aControllerVector.size(); n++ )
        {
            try
            {
                Reference< XUpdatable > xUpdatable( m_aControllerVector[n], UNO_QUERY );
                if ( xUpdatable.is() )
                    xUpdatable->update();
            }
            catch ( Exception& )
            {
            }
        }
    }
    m_bUpdateControllers = sal_False;
}

void ToolBarManager::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );
    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
        UpdateControllers();
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
        try
        {
            m_xDocImageManager->removeConfigurationListener(
                Reference< XUIConfigurationListener >(
                    static_cast< OWeakObject* >( this ), UNO_QUERY ));
        }
        catch ( Exception& )
        {
        }
        try
        {
            m_xModuleImageManager->removeConfigurationListener(
                Reference< XUIConfigurationListener >(
                    static_cast< OWeakObject* >( this ), UNO_QUERY ));
        }
        catch ( Exception& )
        {
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

        if ( m_bAddedToTaskPaneList )
        {
            Window* pWindow = m_pToolBar;
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();

            if ( pWindow )
                ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( m_pToolBar );
            m_bAddedToTaskPaneList = sal_False;
        }

        ImplClearPopupMenu( m_pToolBar );
        delete m_pToolBar;
        m_pToolBar = 0;

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
void SAL_CALL ToolBarManager::elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType;
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    ( GetCurrentSymbolSize() == SFX_SYMBOLS_LARGE ),
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
                }
                pIter->second.nImageInfo = nImageInfo;
            }
        }
    }
}

void SAL_CALL ToolBarManager::elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType;
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    ( GetCurrentSymbolSize() == SFX_SYMBOLS_LARGE ),
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
            }
        }
    }
}

void SAL_CALL ToolBarManager::elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType;
    sal_Int16                nCurrentImageType = getImageTypeFromBools(
                                                    ( GetCurrentSymbolSize() == SFX_SYMBOLS_LARGE ),
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
                }
                pIter->second.nImageInfo = nImageInfo;
            }
        }
    }
}

sal_Int16 ToolBarManager::GetCurrentSymbolSize()
{
    sal_Int16 eOptSymbolSet = SvtMiscOptions().GetSymbolSet();

    if ( eOptSymbolSet == SFX_SYMBOLS_AUTO )
    {
        // Use system settings, we have to retrieve the toolbar icon size from the
        // Application class
        ULONG nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if ( nStyleIconSize == STYLE_TOOLBAR_ICONSIZE_LARGE )
            eOptSymbolSet = SFX_SYMBOLS_LARGE;
        else
            eOptSymbolSet = SFX_SYMBOLS_SMALL;
    }

    return eOptSymbolSet;
}

void ToolBarManager::RemoveControllers()
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    for ( sal_uInt32 n = 0; n < m_aControllerVector.size(); n++ )
    {
        try
        {
            Reference< XComponent > xComponent( m_aControllerVector[n], UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( Exception& )
        {
        }

        m_aControllerVector[n].clear();
    }
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
    Reference< XMultiComponentFactory > xToolbarControllerFactory( m_xToolbarControllerRegistration, UNO_QUERY );
    Reference< XComponentContext > xComponentContext;
    Reference< XPropertySet > xProps( m_xServiceManager, UNO_QUERY );
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );

    if ( xProps.is() )
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComponentContext;

    for ( USHORT i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        USHORT nId = m_pToolBar->GetItemId( i );
        if ( nId == 0 )
            continue;

        rtl::OUString                aCommandURL( m_pToolBar->GetItemCommand( nId ));
        sal_Bool                     bInit( sal_True );
        Reference< XStatusListener > xController;

        svt::ToolboxController* pController( 0 );

        if ( m_xToolbarControllerRegistration.is() &&
             m_xToolbarControllerRegistration->hasController( aCommandURL, m_aModuleIdentifier ))
        {
            if ( xToolbarControllerFactory.is() )
            {
                Sequence< Any > aSeq( 3 );
                PropertyValue aPropValue;

                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                aPropValue.Value    = makeAny( m_aModuleIdentifier );
                aSeq[0] = makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value    = makeAny( m_xFrame );
                aSeq[1] = makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value    = makeAny( m_xServiceManager );
                aSeq[2] = makeAny( aPropValue );

                xController = Reference< XStatusListener >( xToolbarControllerFactory->createInstanceWithArgumentsAndContext(
                                                                aCommandURL, aSeq, xComponentContext ),
                                                            UNO_QUERY );
                bInit = sal_False; // Initialization is done through the factory service
            }
        }

        if ( !xController.is() )
            pController = CreateToolBoxController( m_xFrame, m_pToolBar, nId, aCommandURL );
        if ( !pController )
            pController = new GenericToolbarController( m_xServiceManager, m_xFrame, m_pToolBar, nId, aCommandURL );

        if ( pController )
            xController = Reference< XStatusListener >( static_cast< ::cppu::OWeakObject *>( pController ), UNO_QUERY );

        m_aControllerVector.push_back( xController );
        Reference< XInitialization > xInit( xController, UNO_QUERY );

        if ( xInit.is() )
        {
            if ( bInit )
            {
                PropertyValue aPropValue;
                Sequence< Any > aArgs( 3 );
                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value = makeAny( m_xFrame );
                aArgs[0] = makeAny( aPropValue );
                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
                aPropValue.Value = makeAny( aCommandURL );
                aArgs[1] = makeAny( aPropValue );
                aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value = makeAny( m_xServiceManager );
                aArgs[2] = makeAny( aPropValue );
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

void ToolBarManager::FillToolbar( const Reference< XIndexAccess >& rItemContainer )
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ToolBarManager::FillToolbar" );

    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    USHORT    nId( 1 );
    sal_Int16 nSymbolSet( ToolBarManager::GetCurrentSymbolSize() );
    OUString  aHelpIdPrefix( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX ));

    Reference< XModuleManager > xModuleManager( Reference< XModuleManager >(
                                                    m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ));
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
    m_aControllerVector.clear();
    m_aCommandMap.clear();

    CommandInfo           aCmdInfo;
    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >   aProp;
        rtl::OUString               aCommandURL;
        rtl::OUString               aLabel;
        rtl::OUString               aHelpURL;
        sal_uInt16                  nType( drafts::com::sun::star::ui::ItemType::DEFAULT );
        sal_Bool                    bIsVisible( sal_True );

        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
                    {
                        aProp[i].Value >>= aCommandURL;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
                    {
                        aProp[i].Value >>= aHelpURL;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
                    {
                        aProp[i].Value >>= aLabel;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
                    {
                        aProp[i].Value >>= nType;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_VISIBLE ))
                    {
                        aProp[i].Value >>= bIsVisible;
                    }
                }

                if (( nType == drafts::com::sun::star::ui::ItemType::DEFAULT ) && ( aCommandURL.getLength() > 0 ))
                {
                    OUString aString( RetrieveLabelFromCommand( aCommandURL ));

                    m_pToolBar->InsertItem( nId, aString );
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
                    aCmdInfo.nId = nId;
                    m_aCommandMap.insert( CommandToInfoMap::value_type(
                        rtl::OUString( m_pToolBar->GetItemCommand( nId )), aCmdInfo ));

                    if ( aHelpURL.indexOf( aHelpIdPrefix ) == 0 )
                    {
                        OUString aId( aHelpURL.copy( HELPID_PREFIX_LENGTH ));
                        sal_uInt16 nHelpId = (sal_uInt16)(aId.toInt32());
                        if ( nHelpId > 0 )
                            m_pToolBar->SetHelpId( nId, nHelpId );
                    }

                    if ( !bIsVisible )
                        m_pToolBar->HideItem( nId );

                    ++nId;
                }
                else if ( nType == drafts::com::sun::star::ui::ItemType::SEPARATOR_LINE )
                {
                    m_pToolBar->InsertSeparator();
                }
                else if ( nType == drafts::com::sun::star::ui::ItemType::SEPARATOR_SPACE )
                {
                    m_pToolBar->InsertSpace();
                }
                else if ( nType == drafts::com::sun::star::ui::ItemType::SEPARATOR_LINEBREAK )
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

    m_bIsHiContrast = m_pToolBar->GetBackground().GetColor().IsDark();
    sal_Int16 j = getImageTypeFromBools( ( nSymbolSet == SFX_SYMBOLS_LARGE ), m_bIsHiContrast );

    if ( m_xDocImageManager.is() )
        aDocGraphicSeq = m_xDocImageManager->getImages( j, aCmdURLSeq );
    aModGraphicSeq = m_xModuleImageManager->getImages( j, aCmdURLSeq );

    i = 0;
    pIter = m_aCommandMap.begin();
    while ( pIter != m_aCommandMap.end() )
    {
        Image aImage;
        if ( aDocGraphicSeq.getLength() > 0 )
            aImage = Image( aDocGraphicSeq[i] );
        if ( !aImage )
        {
            m_pToolBar->SetItemImage( pIter->second.nId, Image( aModGraphicSeq[i] ));
            pIter->second.nImageInfo = 0; // mark image as document based
        }
        else
        {
            m_pToolBar->SetItemImage( pIter->second.nId, Image( aDocGraphicSeq[i] ));
            pIter->second.nImageInfo = 1; // mark image as module based
        }
        ++pIter;
        ++i;
    }

    // Create controllers after we set the images. There are controllers which needs
    // an image at the toolbar at creation time!
    CreateControllers();

    // Notify controllers that they are now correctly initialized and can start listening
    UpdateControllers();

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

IMPL_LINK( ToolBarManager, Click, ToolBox*, pToolBar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK( ToolBarManager, DropdownClick, ToolBox*, pToolBar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

        if ( xController.is() )
            xController->createPopupWindow();
    }

    return 1;
}

IMPL_LINK( ToolBarManager, DoubleClick, ToolBox*, pToolBar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

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

    if( pMenu != m_pToolBar->GetMenu() )
        return 1;

    if ( m_bDisposed )
        return 1;

    ImplClearPopupMenu( m_pToolBar );

    return 0;
}

IMPL_LINK( ToolBarManager, MenuButton, ToolBox*, pToolBar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    PopupMenu *pMenu = pToolBar->GetMenu();

    // No config menu entries if command ".uno:ConfigureDialog" is not enabled
    Reference< XDispatch > xDisp;
    com::sun::star::util::URL aURL;
    if ( m_xFrame.is() )
    {
        Reference< XDispatchProvider > xProv( m_xFrame->getController(), UNO_QUERY );
        Reference< XURLTransformer > xTrans( m_xServiceManager->createInstance(
                                                OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        aURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ));
        xTrans->parseStrict( aURL );
        if ( xProv.is() )
            xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );

        if ( !xDisp.is() )
            return 1;
    }

    // popup menu for quick customization
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
        pMenu->CopyItem( aPopupMenu, i, MENU_APPEND );

    // set submenu to toolbar menu
    if( aPopupMenu.GetPopupMenu( 1 ) )
    {
        // create an own submenu to avoid auto-delete when resource menu is deleted
        PopupMenu *pItemMenu = new PopupMenu();
        for( i=0; i< aPopupMenu.GetPopupMenu( 1 )->GetItemCount(); i++)
            pItemMenu->CopyItem( *aPopupMenu.GetPopupMenu( 1 ), i, MENU_APPEND );

        pMenu->SetPopupMenu( 1, pItemMenu );
    }

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
                    Reference< XDispatchProvider > xProv( m_xFrame->getController(), UNO_QUERY );
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
                Any a;
                Reference< XLayoutManager > xLayoutManager;
                Reference< XPropertySet > xPropSet( m_xFrame, UNO_QUERY );
                if ( xPropSet.is() )
                    a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                a >>= xLayoutManager;
                if ( xLayoutManager.is() )
                {
                    ::com::sun::star::awt::Point aPoint;
                    aPoint.X = aPoint.Y = LONG_MAX;
                    xLayoutManager->dockWindow( m_aResourceName,
                                                DockingArea_DOCKINGAREA_DEFAULT,
                                                aPoint );
                }
                break;
            }

            case MENUITEM_TOOLBAR_DOCKALLTOOLBAR:
            {
                Any a;
                Reference< XLayoutManager > xLayoutManager;
                Reference< XPropertySet > xPropSet( m_xFrame, UNO_QUERY );
                if ( xPropSet.is() )
                    a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                a >>= xLayoutManager;
                if ( xLayoutManager.is() )
                {
                    ::com::sun::star::awt::Point aPoint;
                    aPoint.X = aPoint.Y = LONG_MAX;
                    xLayoutManager->dockAllWindows( UIElementType::TOOLBAR );
                }
                break;
            }

            case MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION:
            {
                Any a;
                Reference< XLayoutManager > xLayoutManager;
                Reference< XPropertySet > xPropSet( m_xFrame, UNO_QUERY );
                if ( xPropSet.is() )
                    a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                a >>= xLayoutManager;
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

            default:
            {
                USHORT nId = pMenu->GetCurItemId();
                if(( nId > 0 ) && ( nId < TOOLBOX_MENUITEM_START ))
                {
                    // toggle toolbar button visibility
                    rtl::OUString aCommand = pMenu->GetItemCommand( nId );

                    Any a;
                    Reference< XLayoutManager > xLayoutManager;
                    Reference< XPropertySet > xPropSet( m_xFrame, UNO_QUERY );
                    if ( xPropSet.is() )
                        a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                    a >>= xLayoutManager;
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

IMPL_LINK( ToolBarManager, Select, ToolBox*, pToolBar )
{
    if ( m_bDisposed )
        return 1;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    USHORT      nId( m_pToolBar->GetCurItemId() );
    Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

    if ( xController.is() )
        xController->execute( nKeyModifier );

    return 1;
}

IMPL_LINK( ToolBarManager, Highlight, ToolBox*, pToolBar )
{
    return 1;
}

IMPL_LINK( ToolBarManager, Activate, ToolBox*, pToolBar )
{
    return 1;
}

IMPL_LINK( ToolBarManager, Deactivate, ToolBox*, pToolBar )
{
    return 1;
}

IMPL_LINK( ToolBarManager, StateChanged, StateChangedType*, pStateChangedType )
{
    if ( *pStateChangedType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        // Check if we need to get new images for normal/high contrast mode
        CheckAndUpdateImages();
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
        m_pToolBar->SetSizePixel( aSize );
    }

    return 1;
}

}
