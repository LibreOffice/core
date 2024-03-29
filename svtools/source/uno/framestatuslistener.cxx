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

#include <framestatuslistener.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <vcl/svapp.hxx>

using namespace ::cppu;
using namespace css::awt;
using namespace css::uno;
using namespace css::util;
using namespace css::lang;
using namespace css::frame;

namespace svt
{

FrameStatusListener::FrameStatusListener(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame ) :
    OWeakObject()
    ,   m_bDisposed( false )
    ,   m_xFrame( xFrame )
    ,   m_xContext( rxContext )
{
}

FrameStatusListener::~FrameStatusListener()
{
}

// XInterface
Any SAL_CALL FrameStatusListener::queryInterface( const Type& rType )
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

void SAL_CALL FrameStatusListener::acquire() noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL FrameStatusListener::release() noexcept
{
    OWeakObject::release();
}

// XComponent
void SAL_CALL FrameStatusListener::dispose()
{
    Reference< XComponent > xThis = this;

    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;

    for (auto const& listener : m_aListenerMap)
    {
        try
        {
            Reference< XDispatch > xDispatch( listener.second );
            Reference< XURLTransformer > xURLTransformer( css::util::URLTransformer::create( m_xContext ) );
            css::util::URL aTargetURL;
            aTargetURL.Complete = listener.first;
            xURLTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() )
                xDispatch->removeStatusListener( this, aTargetURL );
        }
        catch (const Exception&)
        {
        }
    }

    m_bDisposed = true;
}

void SAL_CALL FrameStatusListener::addEventListener( const Reference< XEventListener >& )
{
    // helper class for status updates - no need to support listener
}

void SAL_CALL FrameStatusListener::removeEventListener( const Reference< XEventListener >& )
{
    // helper class for status updates - no need to support listener
}

// XEventListener
void SAL_CALL FrameStatusListener::disposing( const EventObject& Source )
{
    Reference< XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    for (auto & listener : m_aListenerMap)
    {
        // Compare references and release dispatch references if they are equal.
        Reference< XInterface > xIfac( listener.second, UNO_QUERY );
        if ( xSource == xIfac )
            listener.second.clear();
    }

    Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

void FrameStatusListener::frameAction( const FrameActionEvent& Action )
{
    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
        bindListener();
}

void FrameStatusListener::addStatusListener( const OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    css::util::URL    aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        // Already in the list of status listener. Do nothing.
        if ( pIter != m_aListenerMap.end() )
            return;

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            Reference< XURLTransformer > xURLTransformer( css::util::URLTransformer::create( m_xContext ) );
            aTargetURL.Complete = aCommandURL;
            xURLTransformer->parseStrict( aTargetURL );
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

            xStatusListener = this;
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
                m_aListenerMap.emplace( aCommandURL, xDispatch );
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
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

        // Collect all registered command URL's and store them temporary
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            xStatusListener = this;
            for (auto & listener : m_aListenerMap)
            {
                Reference< XURLTransformer > xURLTransformer( css::util::URLTransformer::create( m_xContext ) );
                css::util::URL aTargetURL;
                aTargetURL.Complete = listener.first;
                xURLTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch( listener.second );
                if ( xDispatch.is() )
                {
                    // We already have a dispatch object => we have to requery.
                    // Release old dispatch object and remove it as listener
                    try
                    {
                        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch (const Exception&)
                    {
                    }
                }

                // Query for dispatch object. Old dispatch will be released with this, too.
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
                }
                catch (const Exception&)
                {
                }
                listener.second = xDispatch;

                aDispatchVector.emplace_back(std::move(aTargetURL), xDispatch);
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    if ( !xStatusListener.is() )
        return;

    try
    {
        for (Listener & rListener : aDispatchVector)
        {
            if ( rListener.xDispatch.is() )
                rListener.xDispatch->addStatusListener( xStatusListener, rListener.aURL );
        }
    }
    catch (const Exception&)
    {
    }
}

} // svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
