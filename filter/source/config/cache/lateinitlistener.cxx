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

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "lateinitlistener.hxx"
#include "lateinitthread.hxx"

#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;




LateInitListener::LateInitListener(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : BaseLock(     )
{
    // important to do so ...
    // Otherwise the temp. reference to ourselves
    // will kill us at releasing time!
    osl_atomic_increment( &m_refCount );

    m_xBroadcaster = css::uno::Reference< css::document::XEventBroadcaster >(
        css::frame::GlobalEventBroadcaster::create(rxContext),
        css::uno::UNO_QUERY_THROW);

    m_xBroadcaster->addEventListener(static_cast< css::document::XEventListener* >(this));

    osl_atomic_decrement( &m_refCount );
}



LateInitListener::~LateInitListener()
{
}



void SAL_CALL LateInitListener::notifyEvent(const css::document::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // wait for events which either
    // a) indicate completed open of the first document in which case launch thread
    // b) indicate close of application without any documents opened, in which case skip launching thread but drop references break cyclic dependencies in
    // case of e.g. cancel from open/new database wizard or impress wizard
    if ( aEvent.EventName == "OnNew" || aEvent.EventName == "OnLoad" || aEvent.EventName == "OnCloseApp" )
    {
        // this thread must be started one times only ...
        // cancel listener connection before!

        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        if ( !m_xBroadcaster.is() )
            // the beauty of multi-threading ... OnLoad can be notified synchronously or asynchronously. In particular,
            // SFX-based documents notify it synchronously, database documents do it asynchronously.
            // Now if multiple documents are opened "at the same time", it is well possible that we get two events from
            // different threads, where upon the first event, we already remove ourself from m_xBroadcaster, and start
            // the thread, nonetheless there's also a second notification "in the queue", which will arrive short
            // thereafter.
            // In such a case, simply ignore this second event.
            return;

        m_xBroadcaster->removeEventListener(static_cast< css::document::XEventListener* >(this));
        m_xBroadcaster.clear();

        aLock.clear();
        // <- SAFE

        if ( aEvent.EventName != "OnCloseApp" )
        {
            rtl::Reference< LateInitThread >(new LateInitThread())->launch();
                //TODO: a protocol is missing how to join with the launched
                // thread before exit(3), to ensure the thread is no longer
                // relying on any infrastructure while that infrastructure is
                // being shut down in atexit handlers
        }
    }
}



void SAL_CALL LateInitListener::disposing(const css::lang::EventObject& /* aEvent */ )
    throw(css::uno::RuntimeException)
{
    // ???
    // Normaly it should never be called. Because we cancel our listener connection
    // if we got the event about finished open of the first office document.
    // But if this method was reached, it indicates an office, which was started
    // (might as remote script container for an external API client) but not realy used.

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if ( !m_xBroadcaster.is() )
        return;

    m_xBroadcaster->removeEventListener(static_cast< css::document::XEventListener* >(this));
    m_xBroadcaster.clear();
    aLock.clear();
    // <- SAFE
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
