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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/framestatuslistener.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

namespace svt
{

FrameStatusListener::FrameStatusListener(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >& xFrame ) :
    OWeakObject()
    ,   m_bInitialized( sal_True )
    ,   m_bDisposed( sal_False )
    ,   m_xFrame( xFrame )
    ,   m_xServiceManager( rServiceManager )
{
}

FrameStatusListener::~FrameStatusListener()
{
}

Reference< XFrame > FrameStatusListener::getFrameInterface() const
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    return m_xFrame;
}

Reference< XMultiServiceFactory > FrameStatusListener::getServiceManager() const
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    return m_xServiceManager;
}

// XInterface
Any SAL_CALL FrameStatusListener::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                static_cast< XComponent* >( this ),
                static_cast< XFrameActionListener* >( this ),
                static_cast< XStatusListener* >( this ),
                static_cast< XEventListener* >( static_cast< XStatusListener* >( this )),
                static_cast< XEventListener* >( static_cast< XFrameActionListener* >( this )));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL FrameStatusListener::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL FrameStatusListener::release() throw ()
{
    OWeakObject::release();
}

// XComponent
void SAL_CALL FrameStatusListener::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    if ( m_bDisposed )
        throw DisposedException();

    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        try
        {
            Reference< XDispatch > xDispatch( pIter->second );
            Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                "com.sun.star.util.URLTransformer" ))),
                                                          UNO_QUERY );
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            xURLTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }

        ++pIter;
    }

    m_bDisposed = sal_True;
}

void SAL_CALL FrameStatusListener::addEventListener( const Reference< XEventListener >& )
throw ( RuntimeException )
{
    // helper class for status updates - no need to support listener
}

void SAL_CALL FrameStatusListener::removeEventListener( const Reference< XEventListener >& )
throw ( RuntimeException )
{
    // helper class for status updates - no need to support listener
}

// XEventListener
void SAL_CALL FrameStatusListener::disposing( const EventObject& Source )
throw ( RuntimeException )
{
    Reference< XInterface > xSource( Source.Source );

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        // Compare references and release dispatch references if they are equal.
        Reference< XInterface > xIfac( pIter->second, UNO_QUERY );
        if ( xSource == xIfac )
            pIter->second.clear();
    }

    Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

// XStatusListener
void SAL_CALL FrameStatusListener::statusChanged( const FeatureStateEvent& )
throw ( RuntimeException )
{
    // must be implemented by sub class
}

void FrameStatusListener::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
        bindListener();
}

void FrameStatusListener::addStatusListener( const rtl::OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    com::sun::star::util::URL    aTargetURL;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        // Already in the list of status listener. Do nothing.
        if ( pIter != m_aListenerMap.end() )
            return;

        // Check if we are already initialized. Implementation starts adding itself as status listener when
        // intialize is called.
        if ( !m_bInitialized )
        {
            // Put into the hash_map of status listener. Will be activated when initialized is called
            m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, Reference< XDispatch >() ));
            return;
        }
        else
        {
            // Add status listener directly as intialize has already been called.
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( m_xServiceManager.is() && xDispatchProvider.is() )
            {
                Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                            UNO_QUERY );
                aTargetURL.Complete = aCommandURL;
                xURLTransformer->parseStrict( aTargetURL );
                xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
                URLToDispatchMap::iterator aIter = m_aListenerMap.find( aCommandURL );
                if ( aIter != m_aListenerMap.end() )
                {
                    Reference< XDispatch > xOldDispatch( aIter->second );
                    aIter->second = xDispatch;

                    try
                    {
                        if ( xOldDispatch.is() )
                            xOldDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }
                else
                    m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, xDispatch ));
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    try
    {
        if ( xDispatch.is() )
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
    }
    catch ( Exception& )
    {
    }
}

void FrameStatusListener::removeStatusListener( const rtl::OUString& aCommandURL )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );
    if ( pIter != m_aListenerMap.end() )
    {
        Reference< XDispatch > xDispatch( pIter->second );
        Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        m_aListenerMap.erase( pIter );

        try
        {
            Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = aCommandURL;
            xURLTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }
    }
}

void FrameStatusListener::bindListener()
{
    std::vector< Listener > aDispatchVector;
    Reference< XStatusListener > xStatusListener;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( !m_bInitialized )
            return;

        // Collect all registered command URL's and store them temporary
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xServiceManager.is() && xDispatchProvider.is() )
        {
            xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
            URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
            while ( pIter != m_aListenerMap.end() )
            {
                Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                            UNO_QUERY );
                com::sun::star::util::URL aTargetURL;
                aTargetURL.Complete = pIter->first;
                xURLTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch( pIter->second );
                if ( xDispatch.is() )
                {
                    // We already have a dispatch object => we have to requery.
                    // Release old dispatch object and remove it as listener
                    try
                    {
                        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }

                // Query for dispatch object. Old dispatch will be released with this, too.
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
                }
                catch ( Exception& )
                {
                }
                pIter->second = xDispatch;

                Listener aListener( aTargetURL, xDispatch );
                aDispatchVector.push_back( aListener );
                ++pIter;
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    if ( xStatusListener.is() )
    {
        try
        {
            for ( sal_uInt32 i = 0; i < aDispatchVector.size(); i++ )
            {
                Listener& rListener = aDispatchVector[i];
                if ( rListener.xDispatch.is() )
                    rListener.xDispatch->addStatusListener( xStatusListener, rListener.aURL );
            }
        }
        catch ( Exception& )
        {
        }
    }
}

void FrameStatusListener::unbindListener()
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( !m_bInitialized )
        return;

    // Collect all registered command URL's and store them temporary
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( m_xServiceManager.is() && xDispatchProvider.is() )
    {
        Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
        while ( pIter != m_aListenerMap.end() )
        {
            Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch( pIter->second );
            if ( xDispatch.is() )
            {
                // We already have a dispatch object => we have to requery.
                // Release old dispatch object and remove it as listener
                try
                {
                    xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                }
                catch ( Exception& )
                {
                }
            }
            pIter->second.clear();
            ++pIter;
        }
    }
}

void FrameStatusListener::updateStatus( const rtl::OUString aCommandURL )
{
    Reference< XDispatch > xDispatch;
    Reference< XStatusListener > xStatusListener;
    com::sun::star::util::URL aTargetURL;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( !m_bInitialized )
            return;

        // Try to find a dispatch object for the requested command URL
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
        if ( m_xServiceManager.is() && xDispatchProvider.is() )
        {
            Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );
            aTargetURL.Complete = aCommandURL;
            xURLTransformer->parseStrict( aTargetURL );
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, rtl::OUString(), 0 );
        }
    }

    if ( xDispatch.is() && xStatusListener.is() )
    {
        // Catch exception as we release our mutex, it is possible that someone else
        // has already disposed this instance!
        // Add/remove status listener to get a update status information from the
        // requested command.
        try
        {
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
            xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }
    }
}

} // svt
