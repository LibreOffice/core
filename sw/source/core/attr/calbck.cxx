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
#include <frmfmt.hxx>
#include <frame.hxx>
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
    bool ListenerEntry::GetInfo(SwFindNearestNode& rInfo) const
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

template<typename T>
sw::ClientBase<T>::ClientBase(sw::ClientBase<T>&& o) noexcept
    : m_pRegisteredIn(nullptr)
{
    if(o.m_pRegisteredIn)
    {
        o.m_pRegisteredIn->Add(*this);
        o.EndListeningAll();
    }
}

template<typename T>
sw::ClientBase<T>::~ClientBase()
{
    if(GetRegisteredIn())
        DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !m_pRegisteredIn || m_pRegisteredIn->HasWriterListeners(), "SwModify still known, but Client already disconnected!" );
    if( m_pRegisteredIn && m_pRegisteredIn->HasWriterListeners() )
        m_pRegisteredIn->Remove(*this);
}

template<typename T>
std::optional<sw::ModifyChangedHint> sw::ClientBase<T>::CheckRegistration( const SfxPoolItem* pOld )
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

template<typename T>
void sw::ClientBase<T>::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CheckRegistration(pLegacyHint->m_pOld);
};

template<typename T>
void sw::ClientBase<T>::StartListeningToSameModifyAs(const sw::ClientBase<T>& other)
{
    if(other.m_pRegisteredIn)
        other.m_pRegisteredIn->Add(*this);
    else
        EndListeningAll();
}

template<typename T>
void sw::ClientBase<T>::EndListeningAll()
{
    if(m_pRegisteredIn)
        m_pRegisteredIn->Remove(*this);
}

template<typename T>
void sw::ClientBase<T>::RegisterIn(SwModify* pModify)
{
    if(pModify)
        pModify->Add(*this);
    else if(m_pRegisteredIn)
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

namespace {
    class WriterListenerIterator final : private sw::ClientIteratorBase
    {
    public:
        WriterListenerIterator(const SwModify& rSrc) : sw::ClientIteratorBase(rSrc) {}
        sw::WriterListener* First()
            { return GoStart(); }
        sw::WriterListener* Next()
        {
            if(!IsChanged())
                m_pPosition = GetRightOfPos();
            return Sync();
        }
        using sw::ClientIteratorBase::IsChanged;
    };
}

void SwModify::PrepareFormatDeath(const SwFormatChangeHint& rHint)
{
    assert(rHint.m_pOldFormat == this);
    assert(rHint.m_pNewFormat);
    auto aIter = WriterListenerIterator(*this);
    for(auto pListener = aIter.First(); pListener; pListener = aIter.Next())
    {
        SAL_INFO("sw.core", "reparenting " << typeid(*pListener).name() << " at " << pListener << " from " << typeid(this).name() << " at " << this << " to "  << typeid(rHint.m_pNewFormat).name() << " at " << rHint.m_pNewFormat);
        pListener->RegisterIn(rHint.m_pNewFormat);
        pListener->SwClientNotify(*this, rHint);
    }
}

template<typename T>
bool SwModify::HasOnlySpecificWriterListeners() const {
    auto aIter = WriterListenerIterator(*this);
    for(auto pListener = aIter.First(); pListener; pListener = aIter.Next())
        if(!dynamic_cast<T*>(pListener))
            return false;
    return true;
}

void SwModify::RemoveAllWriterListeners()
{
    while(m_pWriterListeners)
        m_pWriterListeners->RegisterIn(nullptr);
}

bool SwModify::GetInfo( SwFindNearestNode& rInfo ) const
{
    if(!m_pWriterListeners)
        return true;
    auto aIter = WriterListenerIterator(*this);
    for(auto pListener = aIter.First(); pListener; pListener = aIter.Next())
        if(!pListener->GetInfo( rInfo ))
            return false;
    return true;
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
    if (rHint.GetId() != SfxHintId::SwLegacyModify && rHint.GetId() != SfxHintId::SwRemoveUnoObject && rHint.GetId() != SfxHintId::SwRemoveUnoObject)
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
    auto aIter = WriterListenerIterator(*this);
    for(auto pListener = aIter.First(); pListener; pListener = aIter.Next())
        pListener->SwClientNotify(*this, rHint);
}

void SwModify::EnsureBroadcasting()
{
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

template class sw::ClientBase<SwModify>;
template class sw::ClientBase<SwFrameFormat>;
template bool SwModify::HasOnlySpecificWriterListeners<SwFrame>() const;
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
