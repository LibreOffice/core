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

#include <frame.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <swcache.hxx>
#include <swfntcch.hxx>

static SwClientIter* pClientIters = 0;

TYPEINIT0( SwClient );





SwClient::SwClient( SwModify* pToRegisterIn )
    : pLeft( 0 ), pRight( 0 ), pRegisteredIn( 0 ), mbIsAllowedToBeRemovedInModifyCall( false )
{
    if(pToRegisterIn)
        
        pToRegisterIn->Add(this);
}

void SwClient::CheckRegistration( const SfxPoolItem* pOld, const SfxPoolItem* )
{
    
    if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
        return;

    const SwPtrMsgPoolItem* pDead = static_cast<const SwPtrMsgPoolItem*>(pOld);
    if(pDead && pDead->pObject == pRegisteredIn)
    {
        
        SwModify* pAbove = const_cast<SwModify*>(pRegisteredIn->GetRegisteredIn());
        if(pAbove)
        {
            
            
            pAbove->Add(this);
            return;
        }
        
        pRegisteredIn->Remove(this);
    }
}

void SwClient::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    CheckRegistration( pOldValue, pNewValue );
}

void SwClient::SwClientNotify( const SwModify&, const SfxHint& )
{
}

SwClient::~SwClient()
{
    OSL_ENSURE( !pRegisteredIn || pRegisteredIn->GetDepends(), "SwModify still known, but Client already disconnected!" );
    if( pRegisteredIn && pRegisteredIn->GetDepends() )
        
        pRegisteredIn->Remove( this );
}

bool SwClient::GetInfo( SfxPoolItem& ) const
{
    return true;
}





SwModify::SwModify()
    : SwClient(0), pRoot(0)
{
    bModifyLocked = false;
    bLockClientList = sal_False;
    bInDocDTOR = sal_False;
    bInCache = sal_False;
    bInSwFntCache = sal_False;
}

SwModify::SwModify( SwModify* pToRegisterIn )
    : SwClient( pToRegisterIn ), pRoot( 0 )
{
    bModifyLocked = false;
    bLockClientList = sal_False;
    bInDocDTOR = sal_False;
    bInCache = sal_False;
    bInSwFntCache = sal_False;
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
        
        if( IsInDocDTOR() )
        {
            
            
            
            SwClientIter aIter( *this );
            SwClient* p = aIter.GoStart();
            while ( p )
            {
                p->pRegisteredIn = 0;
                p = ++aIter;
            }
        }
        else
        {
            
            SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
            NotifyClients( &aDyObject, &aDyObject );

            
            
            while( pRoot )
                pRoot->CheckRegistration( &aDyObject, &aDyObject );
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

    
    if( !pOldValue )
    {
        bLockClientList = sal_True;
    }
    else
    {
        switch( pOldValue->Which() )
        {
        case RES_OBJECTDYING:
        case RES_REMOVE_UNO_OBJECT:
            bLockClientList = ((SwPtrMsgPoolItem*)pOldValue)->pObject != this;
            break;

        case RES_FOOTNOTE_DELETED:
        case RES_REFMARK_DELETED:
        case RES_TOXMARK_DELETED:
        case RES_FIELD_DELETED:
            bLockClientList = sal_False;
            break;

        default:
            bLockClientList = sal_True;
        }
    }

    ModifyBroadcast( pOldValue, pNewValue );
    bLockClientList = sal_False;
    UnlockModify();
}

bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    bool bRet = true;       

    if( pRoot )
    {
        SwClientIter aIter( *(SwModify*)this );

        SwClient* pLast = aIter.GoStart();
        if( pLast )
        {
            while( ( bRet = pLast->GetInfo( rInfo ) ) &&
                   0 != ( pLast = ++aIter ) )
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
        
        if( pDepend->pRegisteredIn != 0 )
            pDepend->pRegisteredIn->Remove( pDepend );

        if( !pRoot )
        {
            
            pRoot = pDepend;
            pRoot->pLeft = 0;
            pRoot->pRight = 0;
        }
        else
        {
            
            pDepend->pRight = pRoot->pRight;
            pRoot->pRight = pDepend;
            pDepend->pLeft = pRoot;
            if( pDepend->pRight )
                pDepend->pRight->pLeft = pDepend;
        }

        
        pDepend->pRegisteredIn = this;
    }
}

SwClient* SwModify::Remove( SwClient* pDepend )
{
    if ( bInDocDTOR )
        return 0;

    OSL_ENSURE( !bLockClientList || pDepend->mbIsAllowedToBeRemovedInModifyCall, "SwClient shall be removed in Modify call!" );

    if( pDepend->pRegisteredIn == this )
    {
        
        
        SwClient* pR = pDepend->pRight;
        SwClient* pL = pDepend->pLeft;
        if( pRoot == pDepend )
            pRoot = pL ? pL : pR;

        if( pL )
            pL->pRight = pR;
        if( pR )
            pR->pLeft = pL;

        
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            if( pTmp->pAct == pDepend || pTmp->pDelNext == pDepend )
            {
                
                
                pTmp->pDelNext = pR;
            }
            pTmp = pTmp->pNxtIter;
        }

        pDepend->pLeft = 0;
        pDepend->pRight = 0;
    }
    else
    {
        OSL_FAIL( "SwModify::Remove(): could not find pDepend" );
    }

    
    pDepend->pRegisteredIn = 0;
    return pDepend;
}

void SwModify::CheckCaching( const sal_uInt16 nWhich )
{
    if( isCHRATR( nWhich ) )
    {
        SetInSwFntCache( sal_False );
    }
    else
    {
        switch( nWhich )
        {
        case RES_OBJECTDYING:
        case RES_FMT_CHG:
        case RES_ATTRSET_CHG:
            SetInSwFntCache( sal_False );

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
                SetInCache( sal_False );
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
    : rRoot( rModify )
{
    pNxtIter = 0;
    if( pClientIters )
    {
        
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
        
        if( pClientIters == this )
            pClientIters = pNxtIter;
        else
        {
            SwClientIter* pTmp = pClientIters;
            while( pTmp->pNxtIter != this )
                if( 0 == ( pTmp = pTmp->pNxtIter ) )
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
    {
        pAct = pAct->pRight;
        pDelNext = pAct;
    }
    else
        pAct = pDelNext;
    return pAct;
}

SwClient* SwClientIter::GoStart()
{
    pAct = const_cast<SwClient*>(rRoot.GetDepends());
    if( pAct )
    {
        while( pAct->pLeft )
            pAct = pAct->pLeft;
    }
    pDelNext = pAct;
    return pAct;
}

SwClient* SwClientIter::GoEnd()
{
    pAct = pDelNext;
    if( !pAct )
        pAct = const_cast<SwClient*>(rRoot.GetDepends());
    if( pAct )
    {
        while( pAct->pRight )
            pAct = pAct->pRight;
    }
    pDelNext = pAct;
    return pAct;
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
                pAct = pAct->pRight;
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
                pAct = pAct->pLeft;
            else
                pAct = pDelNext->pLeft;
            pDelNext = pAct;
        } while( pAct );
    return pAct;
}

SwClient* SwClientIter::Next()
{
    do {
        if( pDelNext == pAct )
        {
            pAct = pAct->pRight;
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
            pAct = pAct->pLeft;
        else
            pAct = pDelNext->pLeft;
        pDelNext = pAct;

        if( pAct && pAct->IsA( aSrchId ) )
            break;
    } while( pAct );
    return pAct;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
