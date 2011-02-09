/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>          // contains RES_.. IDs
#include <frame.hxx>
#include <hints.hxx>
#include <swcache.hxx>          // mba: get rid of that dependency
#include <swfntcch.hxx>         // mba: get rid of that dependency

static SwClientIter* pClientIters = 0;

TYPEINIT0(SwClient);

/*************************************************************************/
SwClient::SwClient(SwModify *pToRegisterIn)
    : pLeft( 0 ), pRight( 0 ), pRegisteredIn( 0 ), mbIsAllowedToBeRemovedInModifyCall(false)
{
<<<<<<< local
=======
    bModifyLocked =
    bInModify =
    bInDocDTOR =
    bInCache = sal_False;
    bInSwFntCache = sal_False;

>>>>>>> other
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


<<<<<<< local
/*************************************************************************/
BOOL SwClient::GetInfo( SfxPoolItem& ) const
=======
    // erfrage vom Client Informationen
sal_Bool SwClient::GetInfo( SfxPoolItem& ) const
>>>>>>> other
{
<<<<<<< local
    return TRUE;        // no information here, return TRUE to enable continuation
=======
    return sal_True;        // und weiter
>>>>>>> other
}


/*************************************************************************/
SwModify::SwModify()
    : SwClient(0), pRoot(0)
{
    bModifyLocked = FALSE;
    bLockClientList = FALSE;
    bInDocDTOR = FALSE;
    bInCache = FALSE;
    bInSwFntCache = FALSE;
}

SwModify::SwModify( SwModify *pToRegisterIn )
    : SwClient(pToRegisterIn), pRoot( 0 )
{
    bModifyLocked = FALSE;
    bLockClientList = FALSE;
    bInDocDTOR = FALSE;
    bInCache = FALSE;
    bInSwFntCache = FALSE;
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
            SwClient* p = aIter.GoStart();
            while ( p )
            {
                p->pRegisteredIn = 0;
                p = aIter++;
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

<<<<<<< local
    // mba: WTF?!
=======
#ifndef DBG_UTIL
    bInModify = sal_True;
#else
>>>>>>> other
    if( !pOldValue )
<<<<<<< local
        bLockClientList = TRUE;
=======
        bInModify = sal_True;
>>>>>>> other
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
<<<<<<< local
            bLockClientList = FALSE;
=======
            bInModify = sal_False;
>>>>>>> other
            break;
        default:
<<<<<<< local
            bLockClientList = TRUE;
=======
            bInModify = sal_True;
>>>>>>> other
        }
    }

<<<<<<< local
    ModifyBroadcast( pOldValue, pNewValue );
    bLockClientList = FALSE;
=======
    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do
        {
            pLast->Modify( pOldValue, pNewValue );
            if( !pRoot )    // Baum schon Weg ??
                break;
        } while( 0 != ( pLast = aIter++ ));

    bInModify = sal_False;
>>>>>>> other
    UnlockModify();
}

<<<<<<< local
BOOL SwModify::GetInfo( SfxPoolItem& rInfo ) const
=======
// erfrage vom Modify Informationen

sal_Bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
>>>>>>> other
{
<<<<<<< local
    BOOL bRet = TRUE;       // means: continue with next one
=======
    sal_Bool bRet = sal_True;       // bedeutet weiter zum naechsten
>>>>>>> other

    if( pRoot )
    {
        SwClientIter aIter( *(SwModify*)this );

        SwClient* pLast = aIter.GoStart();
        if( pLast )
            while( 0 != ( bRet = pLast->GetInfo( rInfo )) &&
                    0 != ( pLast = aIter++ ) )
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
            ASSERT( &pTmp->GetModify() != pRoot, "Client added to active ClientIter" );
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
<<<<<<< local
    else
    {
        ASSERT( false, "SwModify::Remove(): pDepend nicht gefunden" );
=======
    else {
        ASSERT( sal_False, "SwModify::Remove(): pDepend nicht gefunden");
>>>>>>> other
    }

    // disconnect client from me
    pDepend->pRegisteredIn = 0;
    return pDepend;
}

int SwModify::GetClientCount() const
{
    int nRet=0;
    SwClientIter aIter( *this );
    SwClient *pLast = aIter.GoStart();
    if( pLast )
        do
        {
            ++nRet;
        } while( 0 != ( pLast = aIter++ ));
    return nRet;
}

<<<<<<< local
/*************************************************************************/
=======
/*************************************************************************
|*    SwModify::CheckCaching( const sal_uInt16 nWhich )
|*
|*    Ersterstellung    JP 25.06.95
|*    Letzte Aenderung  JP 25.06.95
*************************************************************************/


>>>>>>> other

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
    SwClient * pClient = aIter.GoStart();
    while (pClient)
    {
        pClient->SwClientNotify( *this, rHint );
        pClient = aIter++;
    }
}

void SwModify::ModifyBroadcast( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue, TypeId nType )
{
    SwClientIter aIter(*this);
    SwClient * pClient = aIter.First( nType );
    while (pClient)
    {
        pClient->Modify( pOldValue, pNewValue );
        pClient = aIter.Next();
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

<<<<<<< local
BOOL SwDepend::GetInfo( SfxPoolItem& rInfo ) const
=======
    // erfrage vom Modify Informationen
sal_Bool SwDepend::GetInfo( SfxPoolItem& rInfo ) const
>>>>>>> other
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


SwClient* SwClientIter::operator++(int)
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
        while( pAct->pLeft )
            pAct = pAct->pLeft;
    pDelNext = pAct;
    return pAct;
}

SwClient* SwClientIter::GoEnd()
{
    pAct = pDelNext;
    if( !pAct )
        pAct = const_cast<SwClient*>(rRoot.GetDepends());
    if( pAct )
        while( pAct->pRight )
            pAct = pAct->pRight;
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

