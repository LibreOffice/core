/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: popupmenucontrollerbase.cxx,v $
 * $Revision: 1.8 $
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
#include <helper/popupmenucontrollerbase.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>


//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_11                    (   PopupMenuControllerBase                                                     ,
                                            OWeakObject                                                                 ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                  ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                   ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XPopupMenuController       ),
                                            DIRECT_INTERFACE( css::lang::XInitialization                                ),
                                            DIRECT_INTERFACE( css::frame::XStatusListener                               ),
                                            DIRECT_INTERFACE( css::awt::XMenuListener                                   ),
                                            DIRECT_INTERFACE( css::frame::XDispatchProvider                             ),
                                            DIRECT_INTERFACE( css::frame::XDispatch                                     ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                     ),
                                            DERIVED_INTERFACE( css::lang::XEventListener, css::frame::XStatusListener   ),
                                            DERIVED_INTERFACE( css::lang::XEventListener, css::awt::XMenuListener       )
                                        )

DEFINE_XTYPEPROVIDER_10                 (   PopupMenuControllerBase                             ,
                                            css::lang::XTypeProvider                            ,
                                            css::lang::XServiceInfo                             ,
                                            ::com::sun::star::frame::XPopupMenuController ,
                                            css::lang::XInitialization                          ,
                                            css::frame::XStatusListener                         ,
                                            css::awt::XMenuListener                             ,
                                            css::lang::XEventListener                           ,
                                            css::frame::XDispatchProvider                       ,
                                            css::frame::XDispatch                               ,
                                            css::lang::XComponent
                                        )

PopupMenuControllerBase::PopupMenuControllerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() ),
    m_bInitialized( false ),
    m_bDisposed( false ),
    m_xServiceManager( xServiceManager ),
    m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
}

PopupMenuControllerBase::~PopupMenuControllerBase()
{
}

// protected function
void PopupMenuControllerBase::resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu = 0;
    if ( rPopupMenu.is() && rPopupMenu->getItemCount() > 0 )
    {
         pPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
         if ( pPopupMenu )
         {
            vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

            PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
            pVCLPopupMenu->Clear();
         }
    }
}

void SAL_CALL PopupMenuControllerBase::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XComponent > xThis( static_cast< ::cppu::OWeakObject* >(this), UNO_QUERY );

    // Send message to all listener and forget their references.
    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    // Reset our members and set disposed flag
    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
    m_xServiceManager.clear();
    m_bDisposed = true;
}

void SAL_CALL PopupMenuControllerBase::addEventListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( (const css::uno::Reference< css::lang::XEventListener >*)NULL ), xListener );
}

void SAL_CALL PopupMenuControllerBase::removeEventListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( (const css::uno::Reference< css::lang::XEventListener >*)NULL ), xListener );
}

// XStatusListener
void SAL_CALL PopupMenuControllerBase::statusChanged( const FeatureStateEvent& ) throw ( RuntimeException )
{
    // must be implemented by sub class
}

// XEventListener
void SAL_CALL PopupMenuControllerBase::disposing( const EventObject& ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XMenuListener
void SAL_CALL PopupMenuControllerBase::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();
}

void SAL_CALL PopupMenuControllerBase::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    if ( m_bDisposed )
        throw DisposedException();

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() && xDispatch.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs;
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL PopupMenuControllerBase::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();
}

void SAL_CALL PopupMenuControllerBase::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();
}

// XPopupMenuController
void SAL_CALL PopupMenuControllerBase::setPopupMenu( const Reference< css::awt::XPopupMenu >& PopupMenu ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
        m_xPopupMenu = PopupMenu;
}

void SAL_CALL PopupMenuControllerBase::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XDispatch > xDispatch( m_xDispatch );
    Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aCommandURL;
    xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    // Add/remove status listener to get a status update once
    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( xStatusListener, aTargetURL );
        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
    }
}

// XDispatchProvider
Reference< XDispatch > SAL_CALL
PopupMenuControllerBase::queryDispatch(
    const URL& /*aURL*/,
    const rtl::OUString& /*sTarget*/,
    sal_Int32 /*nFlags*/ )
throw( RuntimeException )
{
    // must be implemented by subclass
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();

    return Reference< XDispatch >();
}

Sequence< Reference< XDispatch > > SAL_CALL
PopupMenuControllerBase::queryDispatches(
    const Sequence< DispatchDescriptor >& lDescriptor )
throw( RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();
    aLock.unlock();

    sal_Int32                                                          nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );

    // Step over all descriptors and try to get any dispatcher for it.
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( lDescriptor[i].FeatureURL  ,
                                        lDescriptor[i].FrameName   ,
                                        lDescriptor[i].SearchFlags );
    }

    return lDispatcher;
}

// XDispatch
void SAL_CALL
PopupMenuControllerBase::dispatch(
    const URL& /*aURL*/,
    const Sequence< PropertyValue >& /*seqProperties*/ )
throw( ::com::sun::star::uno::RuntimeException )
{
    // must be implemented by subclass
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();
}

void SAL_CALL
PopupMenuControllerBase::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
throw( ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();
    aLock.unlock();

    bool bStatusUpdate( false );
    m_aListenerContainer.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XStatusListener >*)NULL ), xControl );

    aLock.lock();
    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
        bStatusUpdate = true;
    aLock.unlock();

    if ( bStatusUpdate )
    {
        // Dummy update for popup menu controllers
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.State      = Any();
        xControl->statusChanged( aEvent );
    }
}

void SAL_CALL PopupMenuControllerBase::removeStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& /*aURL*/ )
throw( ::com::sun::star::uno::RuntimeException )
{
    m_aListenerContainer.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XStatusListener >*)NULL ), xControl );
}

::rtl::OUString PopupMenuControllerBase::determineBaseURL( const ::rtl::OUString& aURL )
{
    // Just use the main part of the URL for popup menu controllers
    sal_Int32     nQueryPart( 0 );
    sal_Int32     nSchemePart( 0 );
    rtl::OUString aMainURL( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.popup:" ));

    nSchemePart = aURL.indexOf( ':' );
    if (( nSchemePart > 0 ) &&
        ( aURL.getLength() > ( nSchemePart+1 )))
    {
        nQueryPart  = aURL.indexOf( '?', nSchemePart );
        if ( nQueryPart > 0 )
            aMainURL += aURL.copy( nSchemePart, nQueryPart-nSchemePart );
        else if ( nQueryPart == -1 )
            aMainURL += aURL.copy( nSchemePart+1 );
    }

    return aMainURL;
}

// XInitialization
void SAL_CALL PopupMenuControllerBase::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_aBaseURL      = determineBaseURL( aCommandURL );
            m_bInitialized  = true;
        }
    }
}

}
