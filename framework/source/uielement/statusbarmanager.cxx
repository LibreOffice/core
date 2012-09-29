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


#include <uielement/statusbarmanager.hxx>

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>
#include "services.h"
#include "general.h"
#include "properties.h"
#include <helper/mischelper.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Command.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/unohlp.hxx>
#include <svtools/statusbarcontroller.hxx>

#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace ::com::sun::star;
namespace css = ::com::sun::star;

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_OFFSET[]      = "Offset";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_WIDTH[]       = "Width";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";

namespace framework
{

static sal_uInt16 impl_convertItemStyleToItemBits( sal_Int16 nStyle )
{
    sal_uInt16 nItemBits( 0 );

    if (( nStyle & ::com::sun::star::ui::ItemStyle::ALIGN_RIGHT ) == ::com::sun::star::ui::ItemStyle::ALIGN_RIGHT )
        nItemBits |= SIB_RIGHT;
    else if ( nStyle & ::com::sun::star::ui::ItemStyle::ALIGN_LEFT )
        nItemBits |= SIB_LEFT;
    else
        nItemBits |= SIB_CENTER;

    if (( nStyle & ::com::sun::star::ui::ItemStyle::DRAW_FLAT ) == ::com::sun::star::ui::ItemStyle::DRAW_FLAT )
        nItemBits |= SIB_FLAT;
    else if ( nStyle & ::com::sun::star::ui::ItemStyle::DRAW_OUT3D )
        nItemBits |= SIB_OUT;
    else
        nItemBits |= SIB_IN;

    if (( nStyle & ::com::sun::star::ui::ItemStyle::AUTO_SIZE ) == ::com::sun::star::ui::ItemStyle::AUTO_SIZE )
        nItemBits |= SIB_AUTOSIZE;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::OWNER_DRAW )
        nItemBits |= SIB_USERDRAW;

    return nItemBits;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_5                     (   StatusBarManager                                                        ,
                                            ::cppu::OWeakObject                                                     ,
                                            DIRECT_INTERFACE( lang::XTypeProvider                                   ),
                                            DIRECT_INTERFACE( lang::XComponent                                      ),
                                            DIRECT_INTERFACE( frame::XFrameActionListener                           ),
                                            DIRECT_INTERFACE( css::ui::XUIConfigurationListener                    ),
                                            DERIVED_INTERFACE( lang::XEventListener, frame::XFrameActionListener    )
                                        )

DEFINE_XTYPEPROVIDER_5                  (   StatusBarManager                    ,
                                            lang::XTypeProvider                 ,
                                            lang::XComponent                    ,
                                            css::ui::XUIConfigurationListener  ,
                                            frame::XFrameActionListener         ,
                                            lang::XEventListener
                                        )

StatusBarManager::StatusBarManager(
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager,
    const uno::Reference< frame::XFrame >& rFrame,
    const rtl::OUString& rResourceName,
    StatusBar* pStatusBar ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    OWeakObject(),
    m_bDisposed( sal_False ),
    m_bFrameActionRegistered( sal_False ),
    m_bUpdateControllers( sal_False ),
    m_bModuleIdentified( sal_False ),
    m_pStatusBar( pStatusBar ),
    m_aResourceName( rResourceName ),
    m_xFrame( rFrame ),
    m_aListenerContainer( m_aLock.getShareableOslMutex() ),
    m_xServiceManager( rServiceManager )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::StatusBarManager" );

    if ( m_xServiceManager.is() )
        m_xStatusbarControllerRegistration = uno::Reference< css::frame::XUIControllerRegistration >(
                                                    m_xServiceManager->createInstance( SERVICENAME_STATUSBARCONTROLLERFACTORY ),
                                                    uno::UNO_QUERY );

    m_pStatusBar->SetClickHdl( LINK( this, StatusBarManager, Click ) );
    m_pStatusBar->SetDoubleClickHdl( LINK( this, StatusBarManager, DoubleClick ) );
}

StatusBarManager::~StatusBarManager()
{
}

StatusBar* StatusBarManager::GetStatusBar() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::GetStatusBar" );
    ResetableGuard aGuard( m_aLock );
    return m_pStatusBar;
}

void StatusBarManager::frameAction( const frame::FrameActionEvent& Action )
throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::frameAction" );
    ResetableGuard aGuard( m_aLock );
    if ( Action.Action == frame::FrameAction_CONTEXT_CHANGED )
        UpdateControllers();
}

void SAL_CALL StatusBarManager::disposing( const lang::EventObject& Source ) throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::disposing" );
    {
        ResetableGuard aGuard( m_aLock );
        if ( m_bDisposed )
            return;
    }

    RemoveControllers();

    {
        ResetableGuard aGuard( m_aLock );
        if ( Source.Source == uno::Reference< uno::XInterface >( m_xFrame, uno::UNO_QUERY ))
            m_xFrame.clear();

        m_xServiceManager.clear();
    }
}

// XComponent
void SAL_CALL StatusBarManager::dispose() throw( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::dispose" );
    uno::Reference< lang::XComponent > xThis(
        static_cast< OWeakObject* >(this), uno::UNO_QUERY );

    lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        if ( !m_bDisposed )
        {
            RemoveControllers();

            delete m_pStatusBar;
            m_pStatusBar = 0;

            if ( m_bFrameActionRegistered && m_xFrame.is() )
            {
                try
                {
                    m_xFrame->removeFrameActionListener( uno::Reference< frame::XFrameActionListener >(
                                                            static_cast< ::cppu::OWeakObject *>( this ),
                                                            uno::UNO_QUERY ));
                }
                catch ( const uno::Exception& )
                {
                }
            }

            m_xFrame.clear();
            m_xServiceManager.clear();

            m_bDisposed = sal_True;
        }
    }
}

void SAL_CALL StatusBarManager::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::addEventListener" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw lang::DisposedException();

    m_aListenerContainer.addInterface( ::getCppuType(
        ( const uno::Reference< lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL StatusBarManager::removeEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::removeEventListener" );
    m_aListenerContainer.removeInterface( ::getCppuType(
        ( const uno::Reference< lang::XEventListener >* ) NULL ), xListener );
}

// XUIConfigurationListener
void SAL_CALL StatusBarManager::elementInserted( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::elementInserted" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;
}

void SAL_CALL StatusBarManager::elementRemoved( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::elementRemoved" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;
}

void SAL_CALL StatusBarManager::elementReplaced( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::elementReplaced" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;
}

void StatusBarManager::UpdateControllers()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::UpdateControllers" );
    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = sal_True;
        const sal_uInt32 nCount = m_aControllerVector.size();
        for ( sal_uInt32 n = 0; n < nCount; n++ )
        {
            try
            {
                uno::Reference< util::XUpdatable > xUpdatable( m_aControllerVector[n], uno::UNO_QUERY );
                if ( xUpdatable.is() )
                    xUpdatable->update();
            }
            catch ( const uno::Exception& )
            {
            }
        }
    }
    m_bUpdateControllers = sal_False;
}

void StatusBarManager::RemoveControllers()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::RemoveControllers" );
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    const sal_uInt32 nCount = m_aControllerVector.size();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        try
        {
            uno::Reference< lang::XComponent > xComponent(
                m_aControllerVector[n], uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }

        m_aControllerVector[n].clear();
    }
}

rtl::OUString StatusBarManager::RetrieveLabelFromCommand( const rtl::OUString& aCmdURL )
{
    return framework::RetrieveLabelFromCommand(aCmdURL,m_xServiceManager,m_xUICommandLabels,m_xFrame,m_aModuleIdentifier,m_bModuleIdentified,"Name");
}

void StatusBarManager::CreateControllers()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::CreateControllers" );
    uno::Reference< lang::XMultiComponentFactory > xStatusbarControllerFactory( m_xStatusbarControllerRegistration, uno::UNO_QUERY );
    uno::Reference< uno::XComponentContext > xComponentContext(
        comphelper::getComponentContext( m_xServiceManager ) );
    uno::Reference< awt::XWindow > xStatusbarWindow = VCLUnoHelper::GetInterface( m_pStatusBar );

    for ( sal_uInt16 i = 0; i < m_pStatusBar->GetItemCount(); i++ )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( i );
        if ( nId == 0 )
            continue;

        rtl::OUString                            aCommandURL( m_pStatusBar->GetItemCommand( nId ));
        sal_Bool                                 bInit( sal_True );
        uno::Reference< frame::XStatusListener > xController;

        svt::StatusbarController* pController( 0 );

        if ( m_xStatusbarControllerRegistration.is() &&
             m_xStatusbarControllerRegistration->hasController( aCommandURL, m_aModuleIdentifier ))
        {
            if ( xStatusbarControllerFactory.is() )
            {
                uno::Sequence< uno::Any > aSeq( 5 );
                beans::PropertyValue aPropValue;

                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                aPropValue.Value    = uno::makeAny( m_aModuleIdentifier );
                aSeq[0] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value    = uno::makeAny( m_xFrame );
                aSeq[1] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value    = uno::makeAny( m_xServiceManager );
                aSeq[2] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                aPropValue.Value    = uno::makeAny( xStatusbarWindow );
                aSeq[3] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" ));
                aPropValue.Value    = uno::makeAny( nId );
                aSeq[4] = uno::makeAny( aPropValue );

                xController = uno::Reference< frame::XStatusListener >(
                                xStatusbarControllerFactory->createInstanceWithArgumentsAndContext(
                                    aCommandURL, aSeq, xComponentContext ),
                                uno::UNO_QUERY );
                bInit = sal_False; // Initialization is done through the factory service
            }
        }

        if ( !xController.is() )
        {
            pController = CreateStatusBarController( m_xFrame, m_pStatusBar, nId, aCommandURL );
            if ( !pController )
                pController = new svt::StatusbarController( m_xServiceManager, m_xFrame, aCommandURL, nId );

            if ( pController )
                xController = uno::Reference< frame::XStatusListener >(
                                static_cast< ::cppu::OWeakObject *>( pController ),
                                uno::UNO_QUERY );
        }

        m_aControllerVector.push_back( xController );
        uno::Reference< lang::XInitialization > xInit( xController, uno::UNO_QUERY );

        if ( xInit.is() )
        {
            if ( bInit )
            {
                beans::PropertyValue aPropValue;
                uno::Sequence< uno::Any > aArgs( 5 );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropValue.Value    = uno::makeAny( m_xFrame );
                aArgs[0] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
                aPropValue.Value    = uno::makeAny( aCommandURL );
                aArgs[1] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                aPropValue.Value    = uno::makeAny( m_xServiceManager );
                aArgs[2] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                aPropValue.Value    = uno::makeAny( xStatusbarWindow );
                aArgs[3] = uno::makeAny( aPropValue );
                aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" ));
                aPropValue.Value    = uno::makeAny( nId );
                aArgs[4] = uno::makeAny( aPropValue );
                xInit->initialize( aArgs );
            }
        }
    }

    AddFrameActionListener();
}

void StatusBarManager::AddFrameActionListener()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::AddFrameActionListener" );
    if ( !m_bFrameActionRegistered && m_xFrame.is() )
    {
        m_bFrameActionRegistered = sal_True;
        m_xFrame->addFrameActionListener( uno::Reference< frame::XFrameActionListener >(
            static_cast< ::cppu::OWeakObject *>( this ), uno::UNO_QUERY ));
    }
}

void StatusBarManager::FillStatusBar( const uno::Reference< container::XIndexAccess >& rItemContainer )
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::StatusBarManager::FillStatusbar" );

    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed || !m_pStatusBar )
        return;

    sal_uInt16         nId( 1 );

    RemoveControllers();

    // reset and fill command map
    m_pStatusBar->Clear();
    m_aControllerVector.clear();

    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::FillStatusBar" );
        uno::Sequence< beans::PropertyValue >   aProp;
        rtl::OUString                           aCommandURL;
        rtl::OUString                           aHelpURL;
        sal_Int16                               nOffset( 0 );
        sal_Int16                               nStyle( 0 );
        sal_Int16                               nWidth( 0 );
        sal_uInt16                              nType( ::com::sun::star::ui::ItemType::DEFAULT );

        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                    {
                        aProp[i].Value >>= aCommandURL;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_HELPURL )
                    {
                        aProp[i].Value >>= aHelpURL;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_STYLE )
                    {
                        aProp[i].Value >>= nStyle;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                    {
                        aProp[i].Value >>= nType;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_WIDTH )
                    {
                        aProp[i].Value >>= nWidth;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_OFFSET )
                    {
                        aProp[i].Value >>= nOffset;
                    }
                }

                if (( nType == ::com::sun::star::ui::ItemType::DEFAULT ) && !aCommandURL.isEmpty() )
                {
                    rtl::OUString aString( RetrieveLabelFromCommand( aCommandURL ));
                    sal_uInt16        nItemBits( impl_convertItemStyleToItemBits( nStyle ));

                    m_pStatusBar->InsertItem( nId, nWidth, nItemBits, nOffset );
                    m_pStatusBar->SetItemCommand( nId, aCommandURL );
                    m_pStatusBar->SetAccessibleName( nId, aString );
                    ++nId;
                }
            }
        }
        catch ( const ::com::sun::star::lang::IndexOutOfBoundsException& )
        {
            break;
        }
    }

    // Create controllers
    CreateControllers();

    // Notify controllers that they are now correctly initialized and can start listening
    UpdateControllers();
}

void StatusBarManager::StateChanged( StateChangedType )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::StateChanged" );
}

void StatusBarManager::DataChanged( const DataChangedEvent& rDCEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::DataChanged" );
    ResetableGuard aGuard( m_aLock );

    if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS         ) ||
         ( rDCEvt.GetType() == DATACHANGED_FONTS            ) ||
         ( rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION ) ||
         ( rDCEvt.GetType() == DATACHANGED_DISPLAY          ))  &&
         ( rDCEvt.GetFlags() & SETTINGS_STYLE               ))
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Reference< css::beans::XPropertySet > xPropSet( m_xFrame, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
            xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))) >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            aGuard.unlock();
            xLayoutManager->doLayout();
        }
    }
}

void StatusBarManager::UserDraw( const UserDrawEvent& rUDEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::UserDraw" );
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    sal_uInt16 nId( rUDEvt.GetItemId() );
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        uno::Reference< frame::XStatusbarController > xController(
            m_aControllerVector[nId-1], uno::UNO_QUERY );
        if ( xController.is() && rUDEvt.GetDevice() )
        {
            uno::Reference< awt::XGraphics > xGraphics =
                rUDEvt.GetDevice()->CreateUnoGraphics();

            awt::Rectangle aRect( rUDEvt.GetRect().Left(),
                                  rUDEvt.GetRect().Top(),
                                  rUDEvt.GetRect().GetWidth(),
                                  rUDEvt.GetRect().GetHeight() );
            aGuard.unlock();
            xController->paint( xGraphics, aRect, rUDEvt.GetItemId(), rUDEvt.GetStyle() );
        }
    }
}

void StatusBarManager::Command( const CommandEvent& rEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::Command" );
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    if ( rEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( rEvt.GetMousePosPixel() );
        if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
        {
            uno::Reference< frame::XStatusbarController > xController(
                m_aControllerVector[nId-1], uno::UNO_QUERY );
            if ( xController.is() )
            {
                awt::Point aPos;
                aPos.X = rEvt.GetMousePosPixel().X();
                aPos.Y = rEvt.GetMousePosPixel().Y();
                xController->command( aPos, awt::Command::CONTEXTMENU, sal_True, uno::Any() );
            }
        }
    }
}

void StatusBarManager::MouseMove( const MouseEvent& rMEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::MouseMove" );
    MouseButton(rMEvt,&frame::XStatusbarController::mouseMove);
}
void StatusBarManager::MouseButton( const MouseEvent& rMEvt ,sal_Bool ( SAL_CALL frame::XStatusbarController::*_pMethod )(const ::com::sun::star::awt::MouseEvent&))
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::MouseButton" );
    ResetableGuard aGuard( m_aLock );

    if ( !m_bDisposed )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( rMEvt.GetPosPixel() );
        if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
        {
            uno::Reference< frame::XStatusbarController > xController(
                m_aControllerVector[nId-1], uno::UNO_QUERY );
            if ( xController.is() )
            {
                ::com::sun::star::awt::MouseEvent aMouseEvent;
                aMouseEvent.Buttons = rMEvt.GetButtons();
                aMouseEvent.X = rMEvt.GetPosPixel().X();
                aMouseEvent.Y = rMEvt.GetPosPixel().Y();
                aMouseEvent.ClickCount = rMEvt.GetClicks();
                (xController.get()->*_pMethod)( aMouseEvent);
            }
        } // if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    }
}
void StatusBarManager::MouseButtonDown( const MouseEvent& rMEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::MouseButtonDown" );
    MouseButton(rMEvt,&frame::XStatusbarController::mouseButtonDown);
}

void StatusBarManager::MouseButtonUp( const MouseEvent& rMEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarManager::MouseButtonUp" );
    MouseButton(rMEvt,&frame::XStatusbarController::mouseButtonUp);
}

IMPL_LINK_NOARG(StatusBarManager, Click)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId = m_pStatusBar->GetCurItemId();
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        uno::Reference< frame::XStatusbarController > xController(
            m_aControllerVector[nId-1], uno::UNO_QUERY );
        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK_NOARG(StatusBarManager, DoubleClick)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId = m_pStatusBar->GetCurItemId();
    if (( nId > 0 ) && ( nId <= m_aControllerVector.size() ))
    {
        uno::Reference< frame::XStatusbarController > xController(
            m_aControllerVector[nId-1], uno::UNO_QUERY );
        if ( xController.is() )
            xController->doubleClick();
    }

    return 1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
