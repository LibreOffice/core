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

#include <frame.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <swcache.hxx>
#include <swfntcch.hxx>
#include <tools/debug.hxx>

sw::LegacyModifyHint::~LegacyModifyHint() {}


SwClient::~SwClient()
{
    if(GetRegisteredIn())
        DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !pRegisteredIn || pRegisteredIn->HasWriterListeners(), "SwModify still known, but Client already disconnected!" );
    if( pRegisteredIn && pRegisteredIn->HasWriterListeners() )
        pRegisteredIn->Remove( this );
}

void SwClient::CheckRegistration( const SfxPoolItem* pOld, const SfxPoolItem* )
{
    DBG_TESTSOLARMUTEX();
    // this method only handles notification about dying SwModify objects
    if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
        return;

    const SwPtrMsgPoolItem* pDead = static_cast<const SwPtrMsgPoolItem*>(pOld);
    if(pDead && pDead->pObject == pRegisteredIn)
    {
        // I've got a notification from the object I know
        SwModify* pAbove = pRegisteredIn->GetRegisteredIn();
        if(pAbove)
        {
            // if the dying object itself was listening at an SwModify, I take over
            // adding myself to pAbove will automatically remove me from my current pRegisteredIn
            pAbove->Add(this);
            return;
        }
        // destroy connection
        pRegisteredIn->Remove(this);
    }
}

void SwClient::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (typeid(rHint) == typeid(sw::LegacyModifyHint))
    {
        auto pLegacyHint(static_cast<const sw::LegacyModifyHint*>(&rHint));
        Modify(pLegacyHint->m_pOld, pLegacyHint->m_pNew);
    }
};

void SwClient::Modify(SfxPoolItem const*const pOldValue, SfxPoolItem const*const pNewValue)
{
    CheckRegistration( pOldValue, pNewValue );
}

void SwModify::SetInDocDTOR()
{
    // If the document gets destroyed anyway, just tell clients to
    // forget me so that they don't try to get removed from my list
    // later when they also get destroyed
    SwIterator<SwClient,SwModify> aIter(*this);
    for(SwClient* pClient = aIter.First(); pClient; pClient = aIter.Next())
        pClient->pRegisteredIn = nullptr;
    m_pWriterListeners = nullptr;
}

SwModify::~SwModify()
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !IsModifyLocked(), "Modify destroyed but locked." );

    if ( IsInCache() )
        SwFrame::GetCache().Delete( this );

    if ( IsInSwFntCache() )
        pSwFontCache->Delete( this );

    // notify all clients that they shall remove themselves
    SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
    NotifyClients( &aDyObject, &aDyObject );

    // remove all clients that have not done themselves
    // mba: possibly a hotfix for forgotten base class calls?!
    while( m_pWriterListeners )
        static_cast<SwClient*>(m_pWriterListeners)->CheckRegistration( &aDyObject, &aDyObject );
}

void SwModify::NotifyClients( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    DBG_TESTSOLARMUTEX();
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                                        pNewValue ? pNewValue->Which() : 0;
        CheckCaching( nWhich );
    }

    if ( !m_pWriterListeners || IsModifyLocked() )
        return;

    LockModify();

    // mba: WTF?!
    if( !pOldValue )
    {
        m_bLockClientList = true;
    }
    else
    {
        switch( pOldValue->Which() )
        {
        case RES_OBJECTDYING:
        case RES_REMOVE_UNO_OBJECT:
            m_bLockClientList = static_cast<const SwPtrMsgPoolItem*>(pOldValue)->pObject != this;
            break;

        default:
            m_bLockClientList = true;
        }
    }

    ModifyBroadcast( pOldValue, pNewValue );
    m_bLockClientList = false;
    UnlockModify();
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

void SwModify::Add( SwClient* pDepend )
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !m_bLockClientList, "Client inserted while in Modify" );

    if(pDepend->pRegisteredIn != this )
    {
#if OSL_DEBUG_LEVEL > 0
        if(sw::ClientIteratorBase::our_pClientIters)
        {
            for(auto& rIter : sw::ClientIteratorBase::our_pClientIters->GetRingContainer())
            {
                SAL_WARN_IF(&rIter.m_rRoot == m_pWriterListeners, "sw.core", "a " << typeid(*pDepend).name() << " client added as listener to a " << typeid(*this).name() << " during client iteration.");
            }
        }
#endif
        // deregister new client in case it is already registered elsewhere
        if( pDepend->pRegisteredIn != nullptr )
            pDepend->pRegisteredIn->Remove( pDepend );

        if( !m_pWriterListeners )
        {
            // first client added
            m_pWriterListeners = pDepend;
            m_pWriterListeners->m_pLeft = nullptr;
            m_pWriterListeners->m_pRight = nullptr;
        }
        else
        {
            // append client
            pDepend->m_pRight = m_pWriterListeners->m_pRight;
            m_pWriterListeners->m_pRight = pDepend;
            pDepend->m_pLeft = m_pWriterListeners;
            if( pDepend->m_pRight )
                pDepend->m_pRight->m_pLeft = pDepend;
        }

        // connect client to me
        pDepend->pRegisteredIn = this;
    }
}

SwClient* SwModify::Remove( SwClient* pDepend )
{
    DBG_TESTSOLARMUTEX();
    assert(pDepend->pRegisteredIn == this);

    // SwClient is my listener
    // remove it from my list
    ::sw::WriterListener* pR = pDepend->m_pRight;
    ::sw::WriterListener* pL = pDepend->m_pLeft;
    if( m_pWriterListeners == pDepend )
        m_pWriterListeners = pL ? pL : pR;

    if( pL )
        pL->m_pRight = pR;
    if( pR )
        pR->m_pLeft = pL;

    // update ClientIterators
    if(sw::ClientIteratorBase::our_pClientIters)
    {
        for(auto& rIter : sw::ClientIteratorBase::our_pClientIters->GetRingContainer())
        {
            if( rIter.m_pCurrent == pDepend || rIter.m_pPosition == pDepend )
            {
                // if object being removed is the current or next object in an
                // iterator, advance this iterator
                rIter.m_pPosition = static_cast<SwClient*>(pR);
            }
        }
    }
    pDepend->m_pLeft = nullptr;
    pDepend->m_pRight = nullptr;
    pDepend->pRegisteredIn = nullptr;
    return pDepend;
}

void SwModify::CheckCaching( const sal_uInt16 nWhich )
{
    if( isCHRATR( nWhich ) )
    {
        SetInSwFntCache( false );
    }
    else
    {
        switch( nWhich )
        {
        case RES_OBJECTDYING:
        case RES_FMT_CHG:
        case RES_ATTRSET_CHG:
            SetInSwFntCache( false );
            SAL_FALLTHROUGH;
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
        case RES_FRM_SIZE:
        case RES_KEEP:
        case RES_BREAK:
            if( IsInCache() )
            {
                SwFrame::GetCache().Delete( this );
                SetInCache( false );
            }
            break;
        }
    }
}

sw::ClientIteratorBase* sw::ClientIteratorBase::our_pClientIters = nullptr;
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
