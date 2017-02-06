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


#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "instancelocker.hxx"

using namespace ::com::sun::star;


// OInstanceLocker


OInstanceLocker::OInstanceLocker()
: m_pListenersContainer( nullptr )
, m_bDisposed( false )
, m_bInitialized( false )
{
}


OInstanceLocker::~OInstanceLocker()
{
    if ( !m_bDisposed )
    {
        m_refCount++; // to call dispose
        try {
            dispose();
        }
        catch ( uno::RuntimeException& )
        {}
    }

    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = nullptr;
    }
}

// XComponent

void SAL_CALL OInstanceLocker::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    if ( m_pListenersContainer )
        m_pListenersContainer->disposeAndClear( aSource );

    if ( m_xLockListener.is() )
    {
        m_xLockListener->Dispose();
        m_xLockListener.clear();
    }

    m_bDisposed = true;
}


void SAL_CALL OInstanceLocker::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::comphelper::OInterfaceContainerHelper2( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}


void SAL_CALL OInstanceLocker::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

// XInitialization

void SAL_CALL OInstanceLocker::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    uno::Reference< uno::XInterface > xInstance;
    uno::Reference< embed::XActionsApproval > xApproval;
    sal_Int32 nModes = 0;

    try
    {
        sal_Int32 nLen = aArguments.getLength();
        if ( nLen < 2 || nLen > 3 )
            throw lang::IllegalArgumentException(
                            "Wrong count of parameters!",
                            uno::Reference< uno::XInterface >(),
                            0 );

        if ( !( aArguments[0] >>= xInstance ) || !xInstance.is() )
            throw lang::IllegalArgumentException(
                    "Nonempty reference is expected as the first argument!",
                    uno::Reference< uno::XInterface >(),
                    0 );

        if (
            !( aArguments[1] >>= nModes ) ||
            (
              !( nModes & embed::Actions::PREVENT_CLOSE ) &&
              !( nModes & embed::Actions::PREVENT_TERMINATION )
            )
           )
        {
            throw lang::IllegalArgumentException(
                    "The correct modes set is expected as the second argument!",
                    uno::Reference< uno::XInterface >(),
                    0 );
        }

        if ( nLen == 3 && !( aArguments[2] >>= xApproval ) )
            throw lang::IllegalArgumentException(
                    "If the third argument is provided, it must be XActionsApproval implementation!",
                    uno::Reference< uno::XInterface >(),
                    0 );

        m_xLockListener = new OLockListener( uno::Reference< lang::XComponent > ( static_cast< lang::XComponent* >( this ) ),
                                            xInstance,
                                            nModes,
                                            xApproval );
        m_xLockListener->Init();
    }
    catch( uno::Exception& )
    {
        dispose();
        throw;
    }

    m_bInitialized = true;
}

// XServiceInfo
OUString SAL_CALL OInstanceLocker::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.embed.InstanceLocker" );
}

sal_Bool SAL_CALL OInstanceLocker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OInstanceLocker::getSupportedServiceNames()
{
    const OUString aServiceName( "com.sun.star.embed.InstanceLocker" );
    return uno::Sequence< OUString >( &aServiceName, 1 );
}

// OLockListener


OLockListener::OLockListener( const uno::WeakReference< lang::XComponent >& xWrapper,
                    const uno::Reference< uno::XInterface >& xInstance,
                    sal_Int32 nMode,
                    const uno::Reference< embed::XActionsApproval >& rApproval )
: m_xInstance( xInstance )
, m_xApproval( rApproval )
, m_xWrapper( xWrapper )
, m_bDisposed( false )
, m_bInitialized( false )
, m_nMode( nMode )
{
}


OLockListener::~OLockListener()
{
}


void OLockListener::Dispose()
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        return;

    if ( m_nMode & embed::Actions::PREVENT_CLOSE )
    {
        try
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xInstance, uno::UNO_QUERY );
            if ( xCloseBroadcaster.is() )
                xCloseBroadcaster->removeCloseListener( static_cast< util::XCloseListener* >( this ) );

            uno::Reference< util::XCloseable > xCloseable( m_xInstance, uno::UNO_QUERY );
            if ( xCloseable.is() )
                xCloseable->close( true );
        }
        catch( uno::Exception& )
        {}
    }

    if ( m_nMode & embed::Actions::PREVENT_TERMINATION )
    {
        try
        {
            uno::Reference< frame::XDesktop > xDesktop( m_xInstance, uno::UNO_QUERY_THROW );
            xDesktop->removeTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
        }
        catch( uno::Exception& )
        {}
    }

    m_xInstance.clear();
    m_bDisposed = true;
}

// XEventListener

void SAL_CALL OLockListener::disposing( const lang::EventObject& aEvent )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is disposed
    if ( aEvent.Source == m_xInstance )
    {
        // the object does not listen for anything any more
        m_nMode = 0;

        // dispose the wrapper;
        uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
        aGuard.clear();
        if ( xComponent.is() )
        {
            try { xComponent->dispose(); }
            catch( uno::Exception& ){}
        }
    }
}


// XCloseListener

void SAL_CALL OLockListener::queryClosing( const lang::EventObject& aEvent, sal_Bool )
{
    // GetsOwnership parameter is always ignored, the user of the service must close the object always
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( !m_bDisposed && aEvent.Source == m_xInstance && ( m_nMode & embed::Actions::PREVENT_CLOSE ) )
    {
        try
        {
            uno::Reference< embed::XActionsApproval > xApprove = m_xApproval;

            // unlock the mutex here
            aGuard.clear();

            if ( xApprove.is() && xApprove->approveAction( embed::Actions::PREVENT_CLOSE ) )
                throw util::CloseVetoException();
        }
        catch( util::CloseVetoException& )
        {
            // rethrow this exception
            throw;
        }
        catch( uno::Exception& )
        {
            // no action should be done
        }
    }
}


void SAL_CALL OLockListener::notifyClosing( const lang::EventObject& aEvent )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is closed, no reason to listen
    if ( aEvent.Source == m_xInstance )
    {
        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( aEvent.Source, uno::UNO_QUERY );
        if ( xCloseBroadcaster.is() )
        {
            xCloseBroadcaster->removeCloseListener( static_cast< util::XCloseListener* >( this ) );
            m_nMode &= ~embed::Actions::PREVENT_CLOSE;
            if ( !m_nMode )
            {
                // dispose the wrapper;
                uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
                aGuard.clear();
                if ( xComponent.is() )
                {
                    try { xComponent->dispose(); }
                    catch( uno::Exception& ){}
                }
            }
        }
    }
}


// XTerminateListener

void SAL_CALL OLockListener::queryTermination( const lang::EventObject& aEvent )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( aEvent.Source == m_xInstance && ( m_nMode & embed::Actions::PREVENT_TERMINATION ) )
    {
        try
        {
            uno::Reference< embed::XActionsApproval > xApprove = m_xApproval;

            // unlock the mutex here
            aGuard.clear();

            if ( xApprove.is() && xApprove->approveAction( embed::Actions::PREVENT_TERMINATION ) )
                throw frame::TerminationVetoException();
        }
        catch( frame::TerminationVetoException& )
        {
            // rethrow this exception
            throw;
        }
        catch( uno::Exception& )
        {
            // no action should be done
        }
    }
}


void SAL_CALL OLockListener::notifyTermination( const lang::EventObject& aEvent )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is terminated, no reason to listen
    if ( aEvent.Source == m_xInstance )
    {
        uno::Reference< frame::XDesktop > xDesktop( aEvent.Source, uno::UNO_QUERY );
        if ( xDesktop.is() )
        {
            try
            {
                xDesktop->removeTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
                m_nMode &= ~embed::Actions::PREVENT_TERMINATION;
                if ( !m_nMode )
                {
                    // dispose the wrapper;
                    uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
                    aGuard.clear();
                    if ( xComponent.is() )
                    {
                        try { xComponent->dispose(); }
                        catch( uno::Exception& ){}
                    }
                }
            }
            catch( uno::Exception& )
            {}
        }
    }
}


// XInitialization

void OLockListener::Init()
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( m_bDisposed || m_bInitialized )
        return;

    try
    {
        if ( m_nMode & embed::Actions::PREVENT_CLOSE )
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xInstance, uno::UNO_QUERY_THROW );
            xCloseBroadcaster->addCloseListener( static_cast< util::XCloseListener* >( this ) );
        }

        if ( m_nMode & embed::Actions::PREVENT_TERMINATION )
        {
            uno::Reference< frame::XDesktop > xDesktop( m_xInstance, uno::UNO_QUERY_THROW );
            xDesktop->addTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
        }
    }
    catch( uno::Exception& )
    {
        // dispose the wrapper;
        uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
        aGuard.clear();
        if ( xComponent.is() )
        {
            try { xComponent->dispose(); }
            catch( uno::Exception& ){}
        }

        throw;
    }

    m_bInitialized = true;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_embed_InstanceLocker(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OInstanceLocker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
