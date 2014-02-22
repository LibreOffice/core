/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svtools/framestatuslistener.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>

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
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame ) :
    OWeakObject()
    ,   m_bInitialized( sal_True )
    ,   m_bDisposed( sal_False )
    ,   m_xFrame( xFrame )
    ,   m_xContext( rxContext )
{
}

FrameStatusListener::~FrameStatusListener()
{
}


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


void SAL_CALL FrameStatusListener::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        throw DisposedException();

    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        try
        {
            Reference< XDispatch > xDispatch( pIter->second );
            Reference< XURLTransformer > xURLTransformer( com::sun::star::util::URLTransformer::create( m_xContext ) );
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            xURLTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch (const Exception&)
        {
        }

        ++pIter;
    }

    m_bDisposed = sal_True;
}

void SAL_CALL FrameStatusListener::addEventListener( const Reference< XEventListener >& )
throw ( RuntimeException )
{
    
}

void SAL_CALL FrameStatusListener::removeEventListener( const Reference< XEventListener >& )
throw ( RuntimeException )
{
    
}


void SAL_CALL FrameStatusListener::disposing( const EventObject& Source )
throw ( RuntimeException )
{
    Reference< XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        
        Reference< XInterface > xIfac( pIter->second, UNO_QUERY );
        if ( xSource == xIfac )
            pIter->second.clear();
    }

    Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

void FrameStatusListener::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
        bindListener();
}

void FrameStatusListener::addStatusListener( const OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    com::sun::star::util::URL    aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        
        if ( pIter != m_aListenerMap.end() )
            return;

        
        
        if ( !m_bInitialized )
        {
            
            m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, Reference< XDispatch >() ));
            return;
        }
        else
        {
            
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( m_xContext.is() && xDispatchProvider.is() )
            {
                Reference< XURLTransformer > xURLTransformer( com::sun::star::util::URLTransformer::create( m_xContext ) );
                aTargetURL.Complete = aCommandURL;
                xURLTransformer->parseStrict( aTargetURL );
                xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

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
                    catch (const Exception&)
                    {
                    }
                }
                else
                    m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, xDispatch ));
            }
        }
    }

    
    try
    {
        if ( xDispatch.is() )
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
    }
    catch (const Exception&)
    {
    }
}


void FrameStatusListener::bindListener()
{
    std::vector< Listener > aDispatchVector;
    Reference< XStatusListener > xStatusListener;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !m_bInitialized )
            return;

        
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
            URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
            while ( pIter != m_aListenerMap.end() )
            {
                Reference< XURLTransformer > xURLTransformer( com::sun::star::util::URLTransformer::create( m_xContext ) );
                com::sun::star::util::URL aTargetURL;
                aTargetURL.Complete = pIter->first;
                xURLTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch( pIter->second );
                if ( xDispatch.is() )
                {
                    
                    
                    try
                    {
                        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch (const Exception&)
                    {
                    }
                }

                
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
                }
                catch (const Exception&)
                {
                }
                pIter->second = xDispatch;

                Listener aListener( aTargetURL, xDispatch );
                aDispatchVector.push_back( aListener );
                ++pIter;
            }
        }
    }

    
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
        catch (const Exception&)
        {
        }
    }
}

void FrameStatusListener::unbindListener()
{
    SolarMutexGuard aSolarMutexGuard;

    if ( !m_bInitialized )
        return;

    
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( m_xContext.is() && xDispatchProvider.is() )
    {
        Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
        while ( pIter != m_aListenerMap.end() )
        {
            Reference< XURLTransformer > xURLTransformer( com::sun::star::util::URLTransformer::create( m_xContext ) );
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch( pIter->second );
            if ( xDispatch.is() )
            {
                
                
                try
                {
                    xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                }
                catch (const Exception&)
                {
                }
            }
            pIter->second.clear();
            ++pIter;
        }
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
