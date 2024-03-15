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

#include "CommandDispatch.hxx"
#include <com/sun/star/util/URLTransformer.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

CommandDispatch::CommandDispatch(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext )
{
}

CommandDispatch::~CommandDispatch()
{}

void CommandDispatch::initialize()
{}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void CommandDispatch::disposing(std::unique_lock<std::mutex>& rGuard)
{
    Reference< uno::XInterface > xEventSource(static_cast< cppu::OWeakObject* >( this ));
    for( auto& rElement : m_aListeners )
        rElement.second.disposeAndClear( rGuard, xEventSource );
    m_aListeners.clear();
}

// ____ XDispatch ____
void SAL_CALL CommandDispatch::dispatch( const util::URL& /* URL */, const Sequence< beans::PropertyValue >& /* Arguments */ )
{}

void SAL_CALL CommandDispatch::addStatusListener( const Reference< frame::XStatusListener >& Control, const util::URL& URL )
{
    {
        std::unique_lock g(m_aMutex);
        tListenerMap::iterator aIt( m_aListeners.find( URL.Complete ));
        if( aIt == m_aListeners.end())
        {
            aIt = m_aListeners.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(URL.Complete),
                        std::forward_as_tuple()).first;
        }
        assert( aIt != m_aListeners.end());

        aIt->second.addInterface( g, Control );
    }
    fireStatusEvent( URL.Complete, Control );
}

void SAL_CALL CommandDispatch::removeStatusListener( const Reference< frame::XStatusListener >& Control, const util::URL& URL )
{
    std::unique_lock g(m_aMutex);
    tListenerMap::iterator aIt( m_aListeners.find( URL.Complete ));
    if( aIt != m_aListeners.end())
        (*aIt).second.removeInterface( g, Control );
}

// ____ XModifyListener ____
void SAL_CALL CommandDispatch::modified( const lang::EventObject& /* aEvent */ )
{
    fireAllStatusEvents( nullptr );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL CommandDispatch::disposing( const lang::EventObject& /* Source */ )
{}

void CommandDispatch::fireAllStatusEvents(
    const css::uno::Reference< css::frame::XStatusListener > & xSingleListener )
{
    fireStatusEvent( OUString(), xSingleListener );
}

void CommandDispatch::fireStatusEventForURL(
    const OUString & rURL,
    const uno::Any & rState,
    bool bEnabled,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */)
{
    // prepare event to send
    util::URL aURL;
    aURL.Complete = rURL;
    if( !m_xURLTransformer.is())
    {
        m_xURLTransformer.set( util::URLTransformer::create(m_xContext) );
    }
    m_xURLTransformer->parseStrict( aURL );

    frame::FeatureStateEvent aEventToSend(
        static_cast< cppu::OWeakObject* >( this ), // Source
        aURL,                                      // FeatureURL
        OUString(),                                // FeatureDescriptor
        bEnabled,                                  // IsEnabled
        false,                                     // Requery
        rState                                     // State
        );

    // send event either to single listener or all registered ones
    if( xSingleListener.is())
        xSingleListener->statusChanged( aEventToSend );
    else
    {
        tListenerMap::iterator aIt( m_aListeners.find( aURL.Complete ));
        if( aIt != m_aListeners.end())
        {
            std::unique_lock g(m_aMutex);
            aIt->second.notifyEach(g, &css::frame::XStatusListener::statusChanged, aEventToSend);
        }
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
