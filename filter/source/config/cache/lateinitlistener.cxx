/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "lateinitlistener.hxx"
#include "lateinitthread.hxx"

//_______________________________________________
// includes
#include <rtl/ustring.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    14.08.2003 07:35
-----------------------------------------------*/
LateInitListener::LateInitListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : BaseLock(     )
    , m_xSMGR (xSMGR)
{
    // important to do so ...
    // Otherwhise the temp. reference to ourselves
    // will kill us at realeasing time!
    osl_incrementInterlockedCount( &m_refCount );

    m_xBroadcaster = css::uno::Reference< css::document::XEventBroadcaster >(
        m_xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster")),
        css::uno::UNO_QUERY);

    m_xBroadcaster->addEventListener(static_cast< css::document::XEventListener* >(this));

    osl_decrementInterlockedCount( &m_refCount );
}

/*-----------------------------------------------
    14.08.2003 07:25
-----------------------------------------------*/
LateInitListener::~LateInitListener()
{
}

/*-----------------------------------------------
    14.08.2003 08:45
-----------------------------------------------*/
void SAL_CALL LateInitListener::notifyEvent(const css::document::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // wait for events, which indicates finished open of the first document
    if (
        (aEvent.EventName.equalsAscii("OnNew") ) ||
        (aEvent.EventName.equalsAscii("OnLoad"))
       )
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

        LateInitThread* pThread = new LateInitThread();
        pThread->create();
    }
}

/*-----------------------------------------------
    14.08.2003 07:48
-----------------------------------------------*/
void SAL_CALL LateInitListener::disposing(const css::lang::EventObject& /* aEvent */ )
    throw(css::uno::RuntimeException)
{
    // ???
    // Normaly it should never be called. Because we cancel our listener connection
    // if we got the event about finished open of the first office document.
    // But if this method was reached, it indicates an office, which was started
    // (might as remote script container for an external API client) but not really used.

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    m_xBroadcaster.clear();
    aLock.clear();
    // <- SAFE
}

    } // namespace config
} // namespace filter
