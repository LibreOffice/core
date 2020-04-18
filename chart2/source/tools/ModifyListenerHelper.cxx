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

#include <ModifyListenerHelper.hxx>
#include <WeakListenerAdapter.hxx>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace
{

void lcl_fireModifyEvent(
    ::cppu::OBroadcastHelper const & rBroadcastHelper,
    const Reference< uno::XWeak > & xEventSource,
    const lang::EventObject * pEvent )
{
    ::cppu::OInterfaceContainerHelper * pCntHlp = rBroadcastHelper.getContainer(
        cppu::UnoType<util::XModifyListener>::get());
    if( !pCntHlp )
        return;

    lang::EventObject aEventToSend;
    if( pEvent )
        aEventToSend = *pEvent;
    else
        aEventToSend.Source.set( xEventSource );
    OSL_ENSURE( aEventToSend.Source.is(), "Sending event without source" );

    ::cppu::OInterfaceIteratorHelper aIt( *pCntHlp );

    while( aIt.hasMoreElements())
    {
        Reference< util::XModifyListener > xModListener( aIt.next(), uno::UNO_QUERY );
        if( xModListener.is())
            xModListener->modified( aEventToSend );
    }
}

struct lcl_weakReferenceToSame
{
    explicit lcl_weakReferenceToSame( const Reference< util::XModifyListener > & xModListener ) :
            m_xHardRef( xModListener )
    {}

    // argument type is same as tListenerMap::value_type&
    bool operator() ( const std::pair<css::uno::WeakReference< css::util::XModifyListener>,
                                      css::uno::Reference< css::util::XModifyListener> > & xElem )
    {
        Reference< util::XModifyListener > xWeakAsHard( xElem.first );
        if( xWeakAsHard.is())
            return (xWeakAsHard == m_xHardRef);
        return false;
    }

private:
    Reference< util::XModifyListener > m_xHardRef;
};

} //  anonymous namespace

namespace chart::ModifyListenerHelper
{

uno::Reference< util::XModifyListener > createModifyEventForwarder()
{
    return new ModifyEventForwarder();
}

ModifyEventForwarder::ModifyEventForwarder() :
        ::cppu::WeakComponentImplHelper<
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >( m_aMutex ),
        m_aModifyListeners( m_aMutex )
{
}

void ModifyEventForwarder::AddListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyListener > xListenerToAdd( aListener );

        Reference< uno::XWeak > xWeak( aListener, uno::UNO_QUERY );
        if( xWeak.is())
        {
            // remember the helper class for later remove
            uno::WeakReference< util::XModifyListener > xWeakRef( aListener );
            xListenerToAdd.set( new WeakModifyListenerAdapter( xWeakRef ));
            m_aListenerMap.emplace_back( xWeakRef, xListenerToAdd );
        }

        m_aModifyListeners.addListener( cppu::UnoType<decltype(xListenerToAdd)>::get(), xListenerToAdd );
    }
    catch( const uno::Exception &  )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ModifyEventForwarder::RemoveListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        // look up fitting helper class that has been added
        Reference< util::XModifyListener > xListenerToRemove( aListener );
        tListenerMap::iterator aIt(
            std::find_if( m_aListenerMap.begin(), m_aListenerMap.end(), lcl_weakReferenceToSame( aListener )));
        if( aIt != m_aListenerMap.end())
        {
            xListenerToRemove.set( (*aIt).second );
            // map entry is no longer needed
            m_aListenerMap.erase( aIt );
        }

        m_aModifyListeners.removeListener( cppu::UnoType<decltype(aListener)>::get(), xListenerToRemove );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ModifyEventForwarder::DisposeAndClear( const Reference< uno::XWeak > & xSource )
{
    ::cppu::OInterfaceContainerHelper * pCntHlp = m_aModifyListeners.getContainer(
        cppu::UnoType<util::XModifyListener>::get());
    if( pCntHlp )
        pCntHlp->disposeAndClear( lang::EventObject( xSource ) );
}

// ____ XModifyBroadcaster ____
void SAL_CALL ModifyEventForwarder::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    AddListener( aListener );
}

void SAL_CALL ModifyEventForwarder::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    RemoveListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL ModifyEventForwarder::modified( const lang::EventObject& aEvent )
{
   lcl_fireModifyEvent( m_aModifyListeners, Reference< uno::XWeak >(), &aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ModifyEventForwarder::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ WeakComponentImplHelperBase ____
void SAL_CALL ModifyEventForwarder::disposing()
{
    // dispose was called at this
    DisposeAndClear( this );
}

} //  namespace chart::ModifyListenerHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
