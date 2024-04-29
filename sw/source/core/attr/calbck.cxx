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


#include <algorithm>
#include <format.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>

#ifdef DBG_UTIL
#include <sal/backtrace.hxx>
#endif

namespace sw
{
    bool ListenerEntry::GetInfo(SfxPoolItem& rInfo) const
        { return m_pToTell == nullptr || m_pToTell->GetInfo( rInfo ); }
    void ListenerEntry::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
    {
        if (rHint.GetId() == SfxHintId::SwLegacyModify)
        {
            auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
            if (pLegacyHint->m_pNew && pLegacyHint->m_pNew->Which() == RES_OBJECTDYING)
            {
                auto pModifyChanged = CheckRegistration(pLegacyHint->m_pOld);
                if (pModifyChanged)
                    m_pToTell->SwClientNotify(rModify, *pModifyChanged);
            }
            else if (m_pToTell)
                m_pToTell->SwClientNotify(rModify, rHint);
        }
        else if (m_pToTell)
            m_pToTell->SwClientNotify(rModify, rHint);
    }
}

sw::LegacyModifyHint::~LegacyModifyHint() {}

SwClient::SwClient(SwClient&& o) noexcept
    : m_pRegisteredIn(nullptr)
{
    if(o.m_pRegisteredIn)
    {
        o.m_pRegisteredIn->Add(*this);
        o.EndListeningAll();
    }
}

SwClient::~SwClient()
{
    if(GetRegisteredIn())
        DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !m_pRegisteredIn || m_pRegisteredIn->HasWriterListeners(), "SwModify still known, but Client already disconnected!" );
    if( m_pRegisteredIn && m_pRegisteredIn->HasWriterListeners() )
        m_pRegisteredIn->Remove(*this);
}

std::optional<sw::ModifyChangedHint> SwClient::CheckRegistration( const SfxPoolItem* pOld )
{
    DBG_TESTSOLARMUTEX();
    // this method only handles notification about dying SwModify objects
    if( !pOld || pOld->Which() != RES_OBJECTDYING )
        return {};

    assert(dynamic_cast<const SwPtrMsgPoolItem*>(pOld));
    const SwPtrMsgPoolItem* pDead = static_cast<const SwPtrMsgPoolItem*>(pOld);
    if(pDead->pObject != m_pRegisteredIn)
    {
        // we should only care received death notes from objects we are following
        return {};
    }
    // I've got a notification from the object I know
    SwModify* pAbove = m_pRegisteredIn->GetRegisteredIn();
    if(pAbove)
    {
        // if the dying object itself was listening at an SwModify, I take over
        // adding myself to pAbove will automatically remove me from my current pRegisteredIn
        pAbove->Add(*this);
    }
    else
    {
        // destroy connection
        EndListeningAll();
    }
    return sw::ModifyChangedHint(pAbove);
}

void SwClient::CheckRegistrationFormat(SwFormat& rOld)
{
    assert(GetRegisteredIn() == &rOld);
    auto pNew = rOld.DerivedFrom();
    SAL_INFO("sw.core", "reparenting " << typeid(*this).name() << " at " << this << " from " << typeid(rOld).name() << " at " << &rOld << " to "  << typeid(*pNew).name() << " at " << pNew);
    assert(pNew);
    pNew->Add(*this);
    const SwFormatChg aOldFormat(&rOld);
    const SwFormatChg aNewFormat(pNew);
    const sw::LegacyModifyHint aHint(&aOldFormat, &aNewFormat);
    SwClientNotify(rOld, aHint);
}

void SwClient::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CheckRegistration(pLegacyHint->m_pOld);
};

void SwClient::StartListeningToSameModifyAs(const SwClient& other)
{
    if(other.m_pRegisteredIn)
        other.m_pRegisteredIn->Add(*this);
    else
        EndListeningAll();
}

void SwClient::EndListeningAll()
{
    if(m_pRegisteredIn)
        m_pRegisteredIn->Remove(*this);
}

SwModify::~SwModify()
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !IsModifyLocked(), "Modify destroyed but locked." );

    // notify all clients that they shall remove themselves
    SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
    SwModify::SwClientNotify(*this, sw::LegacyModifyHint(&aDyObject, &aDyObject));

    const bool hasListenersOnDeath = m_pWriterListeners;
    (void)hasListenersOnDeath;
    while(m_pWriterListeners)
    {
        SAL_WARN("sw.core", "lost a client of type: " << typeid(*m_pWriterListeners).name() << " at " << m_pWriterListeners << " still registered on type: " << typeid(*this).name() << " at " << this << ".");
        static_cast<SwClient*>(m_pWriterListeners)->CheckRegistration(&aDyObject);
    }
    assert(!hasListenersOnDeath);
}

bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    if(!m_pWriterListeners)
        return true;
    SwIterator<SwClient,SwModify> aIter(*this);
    for(SwClient* pClient = aIter.First(); pClient; pClient = aIter.Next())
        if(!pClient->GetInfo( rInfo ))
            return false;
    return true;
}

void SwModify::Add(SwClient& rDepend)
{
    DBG_TESTSOLARMUTEX();
#ifdef DBG_UTIL
    // You should not EVER use SwModify directly in new code:
    // - Preexisting SwModifys should only ever be used via sw::BroadcastingModify.
    //   This includes sw::BroadcastMixin, which is the long-term target (without
    //   SwModify).
    // - New classes should use sw::BroadcastMixin alone.
    if(!dynamic_cast<sw::BroadcastingModify*>(this))
    {
        auto pBT = sal::backtrace_get(20);
        SAL_WARN("sw.core", "Modify that is not broadcasting used!\n" << sal::backtrace_to_string(pBT.get()));
    }
#endif

    if (rDepend.m_pRegisteredIn == this)
        return;

    // deregister new client in case it is already registered elsewhere
    if( rDepend.m_pRegisteredIn != nullptr )
        rDepend.m_pRegisteredIn->Remove(rDepend);

    if( !m_pWriterListeners )
    {
        // first client added
        m_pWriterListeners = &rDepend;
        m_pWriterListeners->m_pLeft = nullptr;
        m_pWriterListeners->m_pRight = nullptr;
    }
    else
    {
        // append client
        rDepend.m_pRight = m_pWriterListeners->m_pRight;
        m_pWriterListeners->m_pRight = &rDepend;
        rDepend.m_pLeft = m_pWriterListeners;
        if( rDepend.m_pRight )
            rDepend.m_pRight->m_pLeft = &rDepend;
    }

    // connect client to me
    rDepend.m_pRegisteredIn = this;
}

void SwModify::Remove(SwClient& rDepend)
{
    DBG_TESTSOLARMUTEX();
    assert(rDepend.m_pRegisteredIn == this);

    // SwClient is my listener
    // remove it from my list
    ::sw::WriterListener* pR = rDepend.m_pRight;
    ::sw::WriterListener* pL = rDepend.m_pLeft;
    if( m_pWriterListeners == &rDepend )
        m_pWriterListeners = pL ? pL : pR;

    if( pL )
        pL->m_pRight = pR;
    if( pR )
        pR->m_pLeft = pL;

    // update ClientIterators
    if(sw::ClientIteratorBase::s_pClientIters)
    {
        for(auto& rIter : sw::ClientIteratorBase::s_pClientIters->GetRingContainer())
        {
            if (&rIter.m_rRoot == this &&
                (rIter.m_pCurrent == &rDepend || rIter.m_pPosition == &rDepend))
            {
                // if object being removed is the current or next object in an
                // iterator, advance this iterator
                rIter.m_pPosition = pR;
            }
        }
    }
    rDepend.m_pLeft = nullptr;
    rDepend.m_pRight = nullptr;
    rDepend.m_pRegisteredIn = nullptr;
}

sw::WriterMultiListener::WriterMultiListener(SwClient& rToTell)
    : m_rToTell(rToTell)
{}

sw::WriterMultiListener::~WriterMultiListener()
{}

void sw::WriterMultiListener::StartListening(SwModify* pDepend)
{
    EndListening(nullptr);
    m_vDepends.emplace_back(&m_rToTell, pDepend);
}


bool sw::WriterMultiListener::IsListeningTo(const SwModify* const pBroadcaster) const
{
    return std::any_of(m_vDepends.begin(), m_vDepends.end(),
        [&pBroadcaster](const ListenerEntry& aListener)
        {
            return aListener.GetRegisteredIn() == pBroadcaster;
        });
}

void sw::WriterMultiListener::EndListening(SwModify* pBroadcaster)
{
    std::erase_if(
            m_vDepends,
            [&pBroadcaster](const ListenerEntry& aListener)
            {
                return aListener.GetRegisteredIn() == nullptr || aListener.GetRegisteredIn() == pBroadcaster;
            });
}

void sw::WriterMultiListener::EndListeningAll()
{
    m_vDepends.clear();
}

sw::ClientIteratorBase* sw::ClientIteratorBase::s_pClientIters = nullptr;

void SwModify::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;

    DBG_TESTSOLARMUTEX();
    if(IsModifyLocked())
        return;

    LockModify();
    CallSwClientNotify(rHint);
    UnlockModify();
}

void SwModify::CallSwClientNotify( const SfxHint& rHint ) const
{
    DBG_TESTSOLARMUTEX();
    SwIterator<SwClient,SwModify> aIter(*this);
    for(SwClient* pClient = aIter.First(); pClient; pClient = aIter.Next())
        pClient->SwClientNotify( *this, rHint );
}

void sw::BroadcastingModify::CallSwClientNotify(const SfxHint& rHint) const
{
    SwModify::CallSwClientNotify(rHint);
    const_cast<BroadcastingModify*>(this)->GetNotifier().Broadcast(rHint);
}

void sw::ClientNotifyAttrChg(SwModify& rModify, const SwAttrSet& aSet, SwAttrSet& aOld, SwAttrSet& aNew)
{
    const SwAttrSetChg aChgOld(aSet, aOld);
    const SwAttrSetChg aChgNew(aSet, aNew);
    const sw::LegacyModifyHint aHint(&aChgOld, &aChgNew);
    rModify.SwClientNotify(rModify, aHint);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
