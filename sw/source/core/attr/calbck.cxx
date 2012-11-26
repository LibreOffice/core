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
#include "precompiled_sw.hxx"

#include <hintids.hxx>          // contains RES_.. IDs
#include <frame.hxx>
#include <hints.hxx>
#include <swcache.hxx>          // mba: get rid of that dependency
#include <swfntcch.hxx>         // mba: get rid of that dependency

static SwClientIter* pClientIters = 0;

/*************************************************************************/
SwClient::SwClient(SwModify *pToRegisterIn)
    : pLeft( 0 ), pRight( 0 ), pRegisteredIn( 0 ), mbIsAllowedToBeRemovedInModifyCall(false)
{
    if(pToRegisterIn)
        // connect to SwModify
        pToRegisterIn->Add(this);
}

/*************************************************************************/
void SwClient::CheckRegistration( const SfxPoolItem* pOld, const SfxPoolItem * )
{
    // this method only handles notification about dying SwModify objects
    if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
        return;

    const SwPtrMsgPoolItem *pDead = static_cast<const SwPtrMsgPoolItem*>(pOld);
    if(pDead && pDead->pObject == pRegisteredIn)
    {
        // I've got a notification from the object I know
        SwModify *pAbove = const_cast<SwModify*>(pRegisteredIn->GetRegisteredIn());
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

void SwClient::Modify( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue )
{
    CheckRegistration( pOldValue, pNewValue );
}

void SwClient::SwClientNotify( const SwModify&, const SfxHint& )
{

}

//*************************************************************************
SwClient::~SwClient()
{
    DBG_ASSERT( !pRegisteredIn || pRegisteredIn->GetDepends(),"SwModify still known, but Client already disconnected!" );
    if( pRegisteredIn && pRegisteredIn->GetDepends() )
        // still connected
        pRegisteredIn->Remove( this );
}


sal_Bool SwClient::GetInfo( SfxPoolItem& ) const
{
    return sal_True;        // und weiter
}


/*************************************************************************/
SwModify::SwModify()
    : SwClient(0), pRoot(0)
{
    bModifyLocked = sal_False;
    bLockClientList = sal_False;
    bInDocDTOR = sal_False;
    bInCache = sal_False;
    bInSwFntCache = sal_False;
}

SwModify::SwModify( SwModify *pToRegisterIn )
    : SwClient(pToRegisterIn), pRoot( 0 )
{
    bModifyLocked = sal_False;
    bLockClientList = sal_False;
    bInDocDTOR = sal_False;
    bInCache = sal_False;
    bInSwFntCache = sal_False;
}

/*************************************************************************/
SwModify::~SwModify()
{
    ASSERT( !IsModifyLocked(), "Modify destroyed but locked." );

    if ( IsInCache() )
        SwFrm::GetCache().Delete( this );

    if ( IsInSwFntCache() )
        pSwFontCache->Delete( this );

    if( pRoot )
    {
        // there are depending objects
        if( IsInDocDTOR() )
        {
            // if document gets destroyed anyway, just tell clients to forget me
            // so that they don't try to get removed from my list later when they also get destroyed
            SwClientIter aIter( *this );
            SwClient* p = aIter.SwClientIter_First();
            while ( p )
            {
                p->pRegisteredIn = 0;
                p = aIter.SwClientIter_Next();
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
                pRoot->CheckRegistration(&aDyObject, &aDyObject);
        }
    }
}

/*************************************************************************/
void SwModify::Modify( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue )
{
    NotifyClients( pOldValue, pNewValue );
}

void SwModify::NotifyClients( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue )
{
    if (IsInCache() || IsInSwFntCache())
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                                        pNewValue ? pNewValue->Which() : 0;
        CheckCaching( nWhich );
    }

      if (!pRoot || IsModifyLocked())
        return;

    LockModify();

    // mba: WTF?!
    if( !pOldValue )
        bLockClientList = sal_True;
    else
    {
        // following Modifies shouldn't call an ASSERT
        switch( pOldValue->Which() )
        {
        case RES_OBJECTDYING:
         case RES_REMOVE_UNO_OBJECT:
            bLockClientList = ((SwPtrMsgPoolItem *)pOldValue)->pObject != this;
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

sal_Bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    sal_Bool bRet = sal_True;       // bedeutet weiter zum naechsten

    if( pRoot )
    {
        SwClientIter aIter( *(SwModify*)this );
        SwClient* pLast = aIter.SwClientIter_First();

        if( pLast )
            while( 0 != ( bRet = pLast->GetInfo( rInfo )) &&
                    0 != ( pLast = aIter.SwClientIter_Next() ) )
                ;
    }

    return bRet;
}

/*************************************************************************/
void SwModify::Add(SwClient *pDepend)
{
    ASSERT( !bLockClientList, "Client inserted while in Modify" );

    if(pDepend->pRegisteredIn != this )
    {
#ifdef DBG_UTIL
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            ASSERT( &pTmp->rRoot != pRoot, "Client added to active ClientIter" );
            pTmp = pTmp->pNxtIter;
        }
#endif
        // deregister new client in case it is already registered elsewhere
        if( pDepend->pRegisteredIn != 0 )
            pDepend->pRegisteredIn->Remove( pDepend );

        if( !pRoot )
        {
            // first client added
            pRoot = pDepend;
            pRoot->pLeft = 0;
            pRoot->pRight = 0;
        }
        else
        {
            // append client
            pDepend->pRight = pRoot->pRight;
            pRoot->pRight = pDepend;
            pDepend->pLeft = pRoot;
            if( pDepend->pRight )
                pDepend->pRight->pLeft = pDepend;
        }

        // connect client to me
        pDepend->pRegisteredIn = this;
    }
}

/*************************************************************************/

SwClient* SwModify::Remove(SwClient * pDepend)
{
    if ( bInDocDTOR )
        return 0;

    ASSERT( !bLockClientList || pDepend->mbIsAllowedToBeRemovedInModifyCall, "SwClient shall be removed in Modify call!" );

    if( pDepend->pRegisteredIn == this )
    {
        // SwClient is my listener
        // remove it from my list
        SwClient* pR = pDepend->pRight;
        SwClient* pL = pDepend->pLeft;
        if( pRoot == pDepend )
            pRoot = pL ? pL : pR;

        if( pL )
            pL->pRight = pR;
        if( pR )
            pR->pLeft = pL;

        // update ClientIters
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            if( pTmp->pAct == pDepend || pTmp->pDelNext == pDepend )
                // if object being removed is the current or next object in an iterator, advance this iterator
                pTmp->pDelNext = pR;
            pTmp = pTmp->pNxtIter;
        }

        pDepend->pLeft = 0;
        pDepend->pRight = 0;
    }
    else
    {
        ASSERT( false, "SwModify::Remove(): pDepend nicht gefunden" );
    }

    // disconnect client from me
    pDepend->pRegisteredIn = 0;
    return pDepend;
}

int SwModify::GetClientCount() const
{
    int nRet=0;
    SwClientIter aIter( *this );
    SwClient *pLast = aIter.SwClientIter_First();
    if( pLast )
        do
        {
            ++nRet;
        } while( 0 != ( pLast = aIter.SwClientIter_Next() ));
    return nRet;
}

void SwModify::CheckCaching( const sal_uInt16 nWhich )
{
    if (isCHRATR(nWhich))
    {
        SetInSwFntCache( sal_False );
    }
    else
        switch ( nWhich )
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
            if ( IsInCache() )
            {
                SwFrm::GetCache().Delete( this );
                SetInCache( sal_False );
            }
            break;
        }
}

void SwModify::CallSwClientNotify( const SfxHint& rHint ) const
{
    SwClientIter aIter(*this);
    SwClient * pClient = aIter.SwClientIter_First();
    while (pClient)
    {
        pClient->SwClientNotify( *this, rHint );
        pClient = aIter.SwClientIter_Next();
    }
}

void SwModify::ModifyBroadcast(
    const SfxPoolItem *pOldValue,
    const SfxPoolItem *pNewValue,
    bool (*pIsValidSwClient)(const SwClient&))
{
    SwClientIter aIter(*this);
    SwClient* pClient = aIter.SwClientIter_First();

    while (pClient)
    {
        if(!pIsValidSwClient || pIsValidSwClient(*pClient))
        {
            pClient->Modify( pOldValue, pNewValue );
        }

        pClient = aIter.SwClientIter_Next();
    }
}

// ----------
// SwDepend
// ----------

/*************************************************************************/

SwDepend::SwDepend(SwClient *pTellHim, SwModify *pDepend)
    : SwClient(pDepend)
{
    pToTell  = pTellHim;
}

/*************************************************************************/

void SwDepend::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem *pNewValue )
{
    if(pNewValue && pNewValue->Which() == RES_OBJECTDYING)
        CheckRegistration(pOldValue,pNewValue);
    else if(pToTell)
        pToTell->ModifyNotification(pOldValue, pNewValue);
}

void SwDepend::SwClientNotify( const SwModify& rMod, const SfxHint& rHint )
{
    if ( pToTell )
        pToTell->SwClientNotifyCall( rMod, rHint );
}

sal_Bool SwDepend::GetInfo( SfxPoolItem& rInfo ) const
{
    return pToTell ? pToTell->GetInfo( rInfo ) : sal_True;
}

/********************************************************************/

SwClientIter::SwClientIter( const SwModify& rModify )
    : rRoot( rModify )
{
    pNxtIter = 0;
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
                if( 0 == ( pTmp = pTmp->pNxtIter ) )
                {
                    ASSERT( this, "wo ist mein Pointer" );
                    return ;
                }
            pTmp->pNxtIter = pNxtIter;
        }
    }
}

SwClient* SwClientIter::SwClientIter_First()
{
    pAct = const_cast<SwClient*>(rRoot.GetDepends());
    if( pAct )
        while( pAct->pLeft )
            pAct = pAct->pLeft;
    pDelNext = pAct;
    return pAct;
}

SwClient* SwClientIter::SwClientIter_Next()
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

// eof
