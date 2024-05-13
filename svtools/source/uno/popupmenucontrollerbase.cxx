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

#include <svtools/popupmenucontrollerbase.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <utility>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/awt/vclxmenu.hxx>

using namespace com::sun::star;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::beans;
using namespace css::util;

namespace svt
{

namespace {

struct PopupMenuControllerBaseDispatchInfo
{
    Reference< XDispatch > mxDispatch;
    const URL maURL;
    const Sequence< PropertyValue > maArgs;

    PopupMenuControllerBaseDispatchInfo( const Reference< XDispatch >& xDispatch, URL aURL, const Sequence< PropertyValue >& rArgs )
        : mxDispatch( xDispatch ), maURL(std::move( aURL )), maArgs( rArgs ) {}
};

}

PopupMenuControllerBase::PopupMenuControllerBase( const Reference< XComponentContext >& xContext ) :
    m_bInitialized( false )
{
    if ( xContext.is() )
        m_xURLTransformer.set( util::URLTransformer::create( xContext ) );
}

PopupMenuControllerBase::~PopupMenuControllerBase()
{
}

// protected function
void PopupMenuControllerBase::resetPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    if ( rPopupMenu.is() && rPopupMenu->getItemCount() > 0 )
    {
        rPopupMenu->clear();
    }
}

void  PopupMenuControllerBase::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    // Reset our members and set disposed flag
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XServiceInfo
sal_Bool SAL_CALL PopupMenuControllerBase::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// XEventListener
void SAL_CALL PopupMenuControllerBase::disposing( const EventObject& )
{
    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XMenuListener
void SAL_CALL PopupMenuControllerBase::itemHighlighted( const awt::MenuEvent& )
{
}

void SAL_CALL PopupMenuControllerBase::itemSelected( const awt::MenuEvent& rEvent )
{
    std::unique_lock aLock( m_aMutex );
    throwIfDisposed(aLock);

    if( m_xPopupMenu.is() )
    {
        Sequence<PropertyValue> aArgs;
        dispatchCommandImpl( aLock, m_xPopupMenu->getCommand( rEvent.MenuId ), aArgs, OUString() );
    }
}

void PopupMenuControllerBase::dispatchCommand( const OUString& sCommandURL,
                                               const css::uno::Sequence< css::beans::PropertyValue >& rArgs,
                                               const OUString& sTarget )
{
    std::unique_lock aLock( m_aMutex );
    throwIfDisposed(aLock);
    dispatchCommandImpl(aLock, sCommandURL, rArgs, sTarget);
}

void PopupMenuControllerBase::dispatchCommandImpl( std::unique_lock<std::mutex>& /*rGuard*/,
                                               const OUString& sCommandURL,
                                               const css::uno::Sequence< css::beans::PropertyValue >& rArgs,
                                               const OUString& sTarget )
{

    try
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY_THROW );
        URL aURL;
        aURL.Complete = sCommandURL;
        m_xURLTransformer->parseStrict( aURL );

        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch( aURL, sTarget, 0 ), UNO_SET_THROW );

        Application::PostUserEvent( LINK(nullptr, PopupMenuControllerBase, ExecuteHdl_Impl), new PopupMenuControllerBaseDispatchInfo( xDispatch, std::move(aURL), rArgs ) );

    }
    catch( Exception& )
    {
    }

}

IMPL_STATIC_LINK( PopupMenuControllerBase, ExecuteHdl_Impl, void*, p, void )
{
    PopupMenuControllerBaseDispatchInfo* pDispatchInfo = static_cast<PopupMenuControllerBaseDispatchInfo*>(p);
    pDispatchInfo->mxDispatch->dispatch( pDispatchInfo->maURL, pDispatchInfo->maArgs );
    delete pDispatchInfo;
}

void SAL_CALL PopupMenuControllerBase::itemActivated( const awt::MenuEvent& )
{
}

void SAL_CALL PopupMenuControllerBase::itemDeactivated( const awt::MenuEvent& )
{
}

void SAL_CALL PopupMenuControllerBase::updatePopupMenu()
{
    {
        std::unique_lock aLock(m_aMutex);
        throwIfDisposed(aLock);
    }

    updateCommand( m_aCommandURL );
}

void PopupMenuControllerBase::updateCommand( const OUString& rCommandURL )
{
    std::unique_lock aLock( m_aMutex );
    Reference< XStatusListener > xStatusListener(this);
    Reference< XDispatch > xDispatch( m_xDispatch );
    URL aTargetURL;
    aTargetURL.Complete = rCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
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
    const OUString& /*sTarget*/,
    sal_Int32 /*nFlags*/ )
{
    // must be implemented by subclass
    std::unique_lock aLock( m_aMutex );
    throwIfDisposed(aLock);

    return Reference< XDispatch >();
}

Sequence< Reference< XDispatch > > SAL_CALL PopupMenuControllerBase::queryDispatches( const Sequence< DispatchDescriptor >& lDescriptor )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    {
        std::unique_lock aLock(m_aMutex);
        throwIfDisposed(aLock);
    }

    sal_Int32                                                          nCount = lDescriptor.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > lDispatcher( nCount );

    // Step over all descriptors and try to get any dispatcher for it.
    std::transform(lDescriptor.begin(), lDescriptor.end(), lDispatcher.getArray(),
        [this](const DispatchDescriptor& rDesc) -> uno::Reference< frame::XDispatch > {
            return queryDispatch(rDesc.FeatureURL, rDesc.FrameName, rDesc.SearchFlags); });

    return lDispatcher;
}

// XDispatch
void SAL_CALL
PopupMenuControllerBase::dispatch(
    const URL& /*aURL*/,
    const Sequence< PropertyValue >& /*seqProperties*/ )
{
    // must be implemented by subclass
    std::unique_lock aLock( m_aMutex );
    throwIfDisposed(aLock);
}

void SAL_CALL
PopupMenuControllerBase::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
{
    std::unique_lock aLock( m_aMutex );
    throwIfDisposed(aLock);

    bool bStatusUpdate( false );
    maStatusListeners.addInterface( aLock, xControl );

    if ( aURL.Complete.startsWith( m_aBaseURL ) )
        bStatusUpdate = true;
    aLock.unlock();

    if ( bStatusUpdate )
    {
        // Dummy update for popup menu controllers
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = true;
        aEvent.Requery    = false;
        aEvent.State      = Any();
        xControl->statusChanged( aEvent );
    }
}

void SAL_CALL PopupMenuControllerBase::removeStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& /*aURL*/ )
{
    std::unique_lock aLock( m_aMutex );
    maStatusListeners.removeInterface( aLock, xControl );
}

OUString PopupMenuControllerBase::determineBaseURL( std::u16string_view aURL )
{
    // Just use the main part of the URL for popup menu controllers
    OUString aMainURL( u"vnd.sun.star.popup:"_ustr );

    size_t nSchemePart = aURL.find( ':' );
    if (( nSchemePart != std::u16string_view::npos && nSchemePart > 0 ) &&
        ( aURL.size() > ( nSchemePart+1 )))
    {
        size_t nQueryPart = aURL.find( '?', nSchemePart );
        if ( nQueryPart != std::u16string_view::npos && nQueryPart > 0 )
            aMainURL += aURL.substr( nSchemePart, nQueryPart-nSchemePart );
        else if ( nQueryPart == std::u16string_view::npos )
            aMainURL += aURL.substr( nSchemePart+1 );
    }

    return aMainURL;
}

// XInitialization
void SAL_CALL PopupMenuControllerBase::initialize( const Sequence< Any >& aArguments )
{
    std::unique_lock aLock( m_aMutex );
    initializeImpl(aLock, aArguments);
}

// XInitialization
void PopupMenuControllerBase::initializeImpl( std::unique_lock<std::mutex>& /*rGuard*/, const Sequence< Any >& aArguments )
{
    bool bInitialized( m_bInitialized );
    if ( bInitialized )
        return;

    PropertyValue       aPropValue;
    OUString       aCommandURL;
    Reference< XFrame > xFrame;

    for ( const auto& rArgument : aArguments )
    {
        if ( rArgument >>= aPropValue )
        {
            if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= xFrame;
            else if ( aPropValue.Name == "CommandURL" )
                aPropValue.Value >>= aCommandURL;
            else if ( aPropValue.Name == "ModuleIdentifier" )
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
// XPopupMenuController
void SAL_CALL PopupMenuControllerBase::setPopupMenu( const Reference< awt::XPopupMenu >& xPopupMenu )
{
    {
        std::unique_lock aLock( m_aMutex );
        throwIfDisposed(aLock);

        if ( !m_xFrame.is() || m_xPopupMenu.is() )
            return;

        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = dynamic_cast<VCLXPopupMenu*>(xPopupMenu.get());
        assert(bool(xPopupMenu) == bool(m_xPopupMenu) && "we only support VCLXPopupMenu");
        m_xPopupMenu->addMenuListener( Reference< awt::XMenuListener >(this) );

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

        impl_setPopupMenu();
    }
    updatePopupMenu();
}
void PopupMenuControllerBase::impl_setPopupMenu()
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
