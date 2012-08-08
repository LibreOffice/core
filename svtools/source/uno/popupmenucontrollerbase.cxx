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

#include "svtools/popupmenucontrollerbase.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XMenuExtended.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>
#include <osl/mutex.hxx>
#include <comphelper/componentcontext.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using ::rtl::OUString;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace svt
{

struct PopupMenuControllerBaseDispatchInfo
{
    Reference< XDispatch > mxDispatch;
    const URL maURL;
    const Sequence< PropertyValue > maArgs;

    PopupMenuControllerBaseDispatchInfo( const Reference< XDispatch >& xDispatch, const URL& rURL, const Sequence< PropertyValue >& rArgs )
        : mxDispatch( xDispatch ), maURL( rURL ), maArgs( rArgs ) {}
};

PopupMenuControllerBase::PopupMenuControllerBase( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ::comphelper::OBaseMutex(),
    PopupMenuControllerBaseType(m_aMutex),
    m_bInitialized( false ),
    m_xServiceManager( xServiceManager )
{
    if ( m_xServiceManager.is() )
        m_xURLTransformer.set( util::URLTransformer::create( ::comphelper::ComponentContext(m_xServiceManager).getUNOContext() ) );
}

PopupMenuControllerBase::~PopupMenuControllerBase()
{
}

// protected function
void PopupMenuControllerBase::throwIfDisposed() throw ( RuntimeException )
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw com::sun::star::lang::DisposedException();
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
            SolarMutexGuard aSolarMutexGuard;

            PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
            pVCLPopupMenu->Clear();
         }
    }
}

void SAL_CALL PopupMenuControllerBase::disposing()
{
    // Reset our members and set disposed flag
    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
    m_xServiceManager.clear();
}

// XServiceInfo

sal_Bool SAL_CALL PopupMenuControllerBase::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
{
    const Sequence< rtl::OUString > aSNL( getSupportedServiceNames() );
    const rtl::OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// XEventListener
void SAL_CALL PopupMenuControllerBase::disposing( const EventObject& ) throw ( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XMenuListener
void SAL_CALL PopupMenuControllerBase::highlight( const awt::MenuEvent& ) throw (RuntimeException)
{
}

void PopupMenuControllerBase::impl_select(const Reference< XDispatch >& _xDispatch,const URL& aURL)
{
    Sequence<PropertyValue>      aArgs;
    OSL_ENSURE(_xDispatch.is(),"PopupMenuControllerBase::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aURL, aArgs );
}

void SAL_CALL PopupMenuControllerBase::select( const awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    throwIfDisposed();

    osl::MutexGuard aLock( m_aMutex );

    Reference< awt::XMenuExtended > xExtMenu( m_xPopupMenu, UNO_QUERY );
    if( xExtMenu.is() )
    {
        Sequence<PropertyValue> aArgs;
        dispatchCommand( xExtMenu->getCommand( rEvent.MenuId ), aArgs );
    }
}

void PopupMenuControllerBase::dispatchCommand( const ::rtl::OUString& sCommandURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    try
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY_THROW );
        URL aURL;
        aURL.Complete = sCommandURL;
        m_xURLTransformer->parseStrict( aURL );

        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch( aURL, OUString(), 0 ), UNO_QUERY_THROW );

        Application::PostUserEvent( STATIC_LINK(0, PopupMenuControllerBase, ExecuteHdl_Impl), new PopupMenuControllerBaseDispatchInfo( xDispatch, aURL, rArgs ) );

    }
    catch( Exception& )
    {
    }

}

IMPL_STATIC_LINK_NOINSTANCE( PopupMenuControllerBase, ExecuteHdl_Impl, PopupMenuControllerBaseDispatchInfo*, pDispatchInfo )
{
    pDispatchInfo->mxDispatch->dispatch( pDispatchInfo->maURL, pDispatchInfo->maArgs );
    delete pDispatchInfo;
    return 0;
}

void SAL_CALL PopupMenuControllerBase::activate( const awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL PopupMenuControllerBase::deactivate( const awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL PopupMenuControllerBase::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    osl::ClearableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    updateCommand( m_aCommandURL );
}

void SAL_CALL PopupMenuControllerBase::updateCommand( const rtl::OUString& rCommandURL )
{
    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XDispatch > xDispatch( m_xDispatch );
    URL aTargetURL;
    aTargetURL.Complete = rCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

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
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();

    return Reference< XDispatch >();
}

Sequence< Reference< XDispatch > > SAL_CALL PopupMenuControllerBase::queryDispatches( const Sequence< DispatchDescriptor >& lDescriptor ) throw( RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    osl::ClearableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    sal_Int32                                                          nCount = lDescriptor.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > lDispatcher( nCount );

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
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();
}

void SAL_CALL
PopupMenuControllerBase::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
throw( ::com::sun::star::uno::RuntimeException )
{
    osl::ResettableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    bool bStatusUpdate( false );
    rBHelper.addListener( ::getCppuType( &xControl ), xControl );

    aLock.reset();
    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
        bStatusUpdate = true;
    aLock.clear();

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
    rBHelper.removeListener( ::getCppuType( &xControl ), xControl );
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
    osl::MutexGuard aLock( m_aMutex );

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
                if ( aPropValue.Name == "Frame" )
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name == "CommandURL" )
                    aPropValue.Value >>= aCommandURL;
                else if ( aPropValue.Name == "ModuleName" )
                    aPropValue.Value >>= m_aModuleName;
            }
        }

        if ( xFrame.is() && !aCommandURL.isEmpty() )
        {
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_aBaseURL      = determineBaseURL( aCommandURL );
            m_bInitialized  = true;
        }
    }
}
// XPopupMenuController
void SAL_CALL PopupMenuControllerBase::setPopupMenu( const Reference< awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        impl_setPopupMenu();

        updatePopupMenu();
    }
}
void PopupMenuControllerBase::impl_setPopupMenu()
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
