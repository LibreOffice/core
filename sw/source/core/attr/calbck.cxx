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

static SwClientIter* pClientIters = nullptr;

TYPEINIT0( SwClient );

SwClient::SwClient( SwModify* pToRegisterIn )
    : pRegisteredIn( nullptr )
{
    if(pToRegisterIn)
        // connect to SwModify
        pToRegisterIn->Add(this);
}

void SwClient::CheckRegistration( const SfxPoolItem* pOld, const SfxPoolItem* )
{
    // this method only handles notification about dying SwModify objects
    if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
        return;

    const SwPtrMsgPoolItem* pDead = static_cast<const SwPtrMsgPoolItem*>(pOld);
    if(pDead && pDead->pObject == pRegisteredIn)
    {
        // I've got a notification from the object I know
        SwModify* pAbove = const_cast<SwModify*>(pRegisteredIn->GetRegisteredIn());
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

void SwClient::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    CheckRegistration( pOldValue, pNewValue );
}

SwClient::~SwClient()
{
    OSL_ENSURE( !pRegisteredIn || pRegisteredIn->GetDepends(), "SwModify still known, but Client already disconnected!" );
    if( pRegisteredIn && pRegisteredIn->GetDepends() )
        // still connected
        pRegisteredIn->Remove( this );
}

bool SwClient::GetInfo( SfxPoolItem& ) const
{
    return true;
}

SwModify::SwModify()
    : SwClient(nullptr), pRoot(nullptr)
{
    bModifyLocked = false;
    bLockClientList = false;
    bInDocDTOR = false;
    bInCache = false;
    bInSwFntCache = false;
}

SwModify::SwModify( SwModify* pToRegisterIn )
    : SwClient( pToRegisterIn ), pRoot( nullptr )
{
    bModifyLocked = false;
    bLockClientList = false;
    bInDocDTOR = false;
    bInCache = false;
    bInSwFntCache = false;
}

SwModify::~SwModify()
{
    OSL_ENSURE( !IsModifyLocked(), "Modify destroyed but locked." );

    if ( IsInCache() )
        SwFrm::GetCache().Delete( this );

    if ( IsInSwFntCache() )
        pSwFontCache->Delete( this );

    if( pRoot )
    {
        // there are depending objects
        if( IsInDocDTOR() )
        {
            // If the document gets destroyed anyway, just tell clients to
            // forget me so that they don't try to get removed from my list
            // later when they also get destroyed
            SwClientIter aIter( *this );
            SwClient* p = aIter.GoStart();
            while ( p )
            {
                p->pRegisteredIn = nullptr;
                p = ++aIter;
            }
        }
        else
        {
            // notify all clients that they shall remove themselves
            SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
            NotifyClients( &aDyObject, &aDyObject );

            // remove all clients that have not done themselves
            // mba: possibly a hotfix for forgotten base class calls?!
            while( pRoot )
                static_cast<SwClient*>(pRoot)->CheckRegistration( &aDyObject, &aDyObject );
        }
    }
}

void SwModify::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    NotifyClients( pOldValue, pNewValue );
}

void SwModify::NotifyClients( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                                        pNewValue ? pNewValue->Which() : 0;
        CheckCaching( nWhich );
    }

    if ( !pRoot || IsModifyLocked() )
        return;

    LockModify();

    // mba: WTF?!
    if( !pOldValue )
    {
        bLockClientList = true;
    }
    else
    {
        switch( pOldValue->Which() )
        {
        case RES_OBJECTDYING:
        case RES_REMOVE_UNO_OBJECT:
            bLockClientList = static_cast<const SwPtrMsgPoolItem*>(pOldValue)->pObject != this;
            break;

        default:
            bLockClientList = true;
        }
    }

    ModifyBroadcast( pOldValue, pNewValue );
    bLockClientList = false;
    UnlockModify();
}

bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    bool bRet = true;       // means: continue with next

    if( pRoot )
    {
        SwClientIter aIter( *(SwModify*)this );

        SwClient* pLast = aIter.GoStart();
        if( pLast )
        {
            while( ( bRet = pLast->GetInfo( rInfo ) ) &&
                   nullptr != ( pLast = ++aIter ) )
                ;
        }
    }

    return bRet;
}

void SwModify::Add( SwClient* pDepend )
{
    OSL_ENSURE( !bLockClientList, "Client inserted while in Modify" );

    if(pDepend->pRegisteredIn != this )
    {
#if OSL_DEBUG_LEVEL > 0
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            OSL_ENSURE( &pTmp->GetModify() != pRoot, "Client added to active ClientIter" );
            pTmp = pTmp->pNxtIter;
        }
#endif
        // deregister new client in case it is already registered elsewhere
        if( pDepend->pRegisteredIn != nullptr )
            pDepend->pRegisteredIn->Remove( pDepend );

        if( !pRoot )
        {
            // first client added
            pRoot = pDepend;
            pRoot->m_pLeft = nullptr;
            pRoot->m_pRight = nullptr;
        }
        else
        {
            // append client
            pDepend->m_pRight = pRoot->m_pRight;
            pRoot->m_pRight = pDepend;
            pDepend->m_pLeft = pRoot;
            if( pDepend->m_pRight )
                pDepend->m_pRight->m_pLeft = pDepend;
        }

        // connect client to me
        pDepend->pRegisteredIn = this;
    }
}

SwClient* SwModify::Remove( SwClient* pDepend )
{
    if ( bInDocDTOR )
        return nullptr;

    if( pDepend->pRegisteredIn == this )
    {
        // SwClient is my listener
        // remove it from my list
        ::sw::WriterListener* pR = pDepend->m_pRight;
        ::sw::WriterListener* pL = pDepend->m_pLeft;
        if( pRoot == pDepend )
            pRoot = pL ? pL : pR;

        if( pL )
            pL->m_pRight = pR;
        if( pR )
            pR->m_pLeft = pL;

        // update ClientIters
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            if( pTmp->pAct == pDepend || pTmp->pDelNext == pDepend )
            {
                // if object being removed is the current or next object in an
                // iterator, advance this iterator
                pTmp->pDelNext = static_cast<SwClient*>(pR);
            }
            pTmp = pTmp->pNxtIter;
        }

        pDepend->m_pLeft = nullptr;
        pDepend->m_pRight = nullptr;
    }
    else
    {
        OSL_FAIL( "SwModify::Remove(): could not find pDepend" );
    }

    // disconnect client from me
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
            // fall through
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
        case RES_FRM_SIZE:
        case RES_KEEP:
        case RES_BREAK:
            if( IsInCache() )
            {
                SwFrm::GetCache().Delete( this );
                SetInCache( false );
            }
            break;
        }
    }
}

void SwModify::CallSwClientNotify( const SfxHint& rHint ) const
{
    SwClientIter aIter(*this);
    SwClient* pClient = aIter.GoStart();
    while( pClient )
    {
        pClient->SwClientNotify( *this, rHint );
        pClient = ++aIter;
    }
}

void SwModify::ModifyBroadcast( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue, TypeId nType )
{
    SwClientIter aIter( *this );
    SwClient* pClient = aIter.First( nType );
    while( pClient )
    {
        pClient->Modify( pOldValue, pNewValue );
        pClient = aIter.Next();
    }
}

SwDepend::SwDepend( SwClient* pTellHim, SwModify* pDepend )
    : SwClient( pDepend )
{
    pToTell  = pTellHim;
}

void SwDepend::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    if( pNewValue && pNewValue->Which() == RES_OBJECTDYING )
        CheckRegistration(pOldValue,pNewValue);
    else if( pToTell )
        pToTell->ModifyNotification(pOldValue, pNewValue);
}

void SwDepend::SwClientNotify( const SwModify& rMod, const SfxHint& rHint )
{
    if ( pToTell )
        pToTell->SwClientNotifyCall( rMod, rHint );
}

bool SwDepend::GetInfo( SfxPoolItem& rInfo ) const
{
    return pToTell ? pToTell->GetInfo( rInfo ) : true;
}

SwClientIter::SwClientIter( const SwModify& rModify )
    : rRoot(rModify)
    , pNxtIter(nullptr)
    , aSrchId(nullptr)
{
    if( pClientIters )
    {
        // append to list of ClientIters
        SwClientIter* pTmp = pClientIters;
        while( pTmp->pNxtIter )
            pTmp = pTmp->pNxtIter;
        pTmp->pNxtIter = this;
    }
    else
        pClientIters = this;

    pAct = const_cast<SwClient*>(rRoot.GetDepends());
    pDelNext = pAct;
}

SwClientIter::~SwClientIter()
{
    if( pClientIters )
    {
        // reorganize list of ClientIters
        if( pClientIters == this )
            pClientIters = pNxtIter;
        else
        {
            SwClientIter* pTmp = pClientIters;
            while( pTmp->pNxtIter != this )
                if( nullptr == ( pTmp = pTmp->pNxtIter ) )
                {
                    OSL_ENSURE( this, "Lost my pointer" );
                    return ;
                }
            pTmp->pNxtIter = pNxtIter;
        }
    }
}

SwClient* SwClientIter::operator++()
{
    if( pDelNext == pAct )
        pDelNext = static_cast<SwClient*>(pDelNext->m_pRight);
    return pAct = pDelNext;
}

SwClient* SwClientIter::GoStart()
{
    pDelNext = const_cast<SwClient*>(rRoot.GetDepends());
    if(pDelNext)
        while( pDelNext->m_pLeft )
            pDelNext = static_cast<SwClient*>(pDelNext->m_pLeft);
    return pAct = pDelNext;
}

SwClient* SwClientIter::GoEnd()
{
    if(!pDelNext)
        pDelNext = const_cast<SwClient*>(rRoot.GetDepends());
    if(pDelNext)
        while( pDelNext->m_pRight )
            pDelNext = static_cast<SwClient*>(pDelNext->m_pRight);
    return pAct = pDelNext;
}

SwClient* SwClientIter::First( TypeId nType )
{
    aSrchId = nType;
    GoStart();
    if( pAct )
        do {
            if( pAct->IsA( aSrchId ) )
                break;

            if( pDelNext == pAct )
            {
                pAct = static_cast<SwClient*>(pAct->m_pRight);
                pDelNext = pAct;
            }
            else
                pAct = pDelNext;
        } while( pAct );
    return pAct;
}

SwClient* SwClientIter::Last( TypeId nType )
{
    aSrchId = nType;
    GoEnd();
    if( pAct )
        do {
            if( pAct->IsA( aSrchId ) )
                break;

            if( pDelNext == pAct )
                pAct = static_cast<SwClient*>(pAct->m_pLeft);
            else
                pAct = static_cast<SwClient*>(pDelNext->m_pLeft);
            pDelNext = pAct;
        } while( pAct );
    return pAct;
}

SwClient* SwClientIter::Next()
{
    do {
        if( pDelNext == pAct )
        {
            pAct = static_cast<SwClient*>(pAct->m_pRight);
            pDelNext = pAct;
        }
        else
            pAct = pDelNext;

        if( pAct && pAct->IsA( aSrchId ) )
            break;
    } while( pAct );
    return pAct;
}

SwClient* SwClientIter::Previous()
{
    do {
        if( pDelNext == pAct )
            pAct = static_cast<SwClient*>(pAct->m_pLeft);
        else
            pAct = static_cast<SwClient*>(pDelNext->m_pLeft);
        pDelNext = pAct;

        if( pAct && pAct->IsA( aSrchId ) )
            break;
    } while( pAct );
    return pAct;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
