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


#include <hintids.hxx>      // fuer RES_..
#include <frame.hxx>
#include <hints.hxx>
#include <swcache.hxx>
#include <swfntcch.hxx>

#ifdef DBG_UTIL
#include <unotextmarkup.hxx>
#endif

static SwClientIter* pClientIters = 0;

TYPEINIT0(SwClient);    //rtti

/*************************************************************************
|*    SwClient::SwClient(SwModify *)
|*
|*    Beschreibung      callback.doc V1.14
|*    Ersterstellung    VB 20.03.91
|*    Letzte Aenderung  MA 20. Mar. 95
*************************************************************************/


SwClient::SwClient(SwModify *pToRegisterIn)
    : pLeft( 0 ), pRight( 0 ), pRegisteredIn( 0 )
{
    bModifyLocked =
    bInModify =
    bInDocDTOR =
    bInCache = sal_False;
    bInSwFntCache = sal_False;

    if(pToRegisterIn)
        pToRegisterIn->Add(this);
}

/*************************************************************************
|*    SwClient::Modify()
|*
|*    Beschreibung      callback.doc V1.14
|*    Ersterstellung    VB 20.03.91
|*    Letzte Aenderung  VB 20.03.91
*************************************************************************/


void SwClient::Modify( SfxPoolItem *pOld, SfxPoolItem * )
{
    if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
        return;

    SwPtrMsgPoolItem *pDead = (SwPtrMsgPoolItem *)pOld;
    if(pDead->pObject == pRegisteredIn)
    {
        SwModify *pAbove = (SwModify*)pRegisteredIn->GetRegisteredIn();
        if(pAbove)
        {
            pAbove->Add(this);
            return;
        }
        pRegisteredIn->Remove(this);
    }
}

/*************************************************************************
|*    SwClient::~SwClient()
|*
|*    Beschreibung      callback.doc V1.14
|*    Ersterstellung    VB 20.03.91
|*    Letzte Aenderung  MA 25. Jan. 94
*************************************************************************/


SwClient::~SwClient()
{
    if( pRegisteredIn && pRegisteredIn->GetDepends() )
        pRegisteredIn->Remove( this );

    ASSERT( !IsModifyLocked(), "Modify destroyed but locked." );
}


    // erfrage vom Client Informationen
sal_Bool SwClient::GetInfo( SfxPoolItem& ) const
{
    return sal_True;        // und weiter
}

/*************************************************************************
|*    SwModify::SwModify( SwModify * )
|*
|*    Beschreibung      Dokument 1.7
|*    Ersterstellung    JP 20.11.90
|*    Letzte Aenderung  VB 20.03.91
*************************************************************************/


SwModify::SwModify( SwModify *pToRegisterIn )
    : SwClient(pToRegisterIn), pRoot( 0 )
{
}

// @@@ forbidden and not implemented, but needs to be accessible (protected).
SwModify::SwModify( const SwModify & )
    : SwClient( 0 )
{
    OSL_PRECOND(0, "SwModify(const SwModify&): not implemented.");
}

/*************************************************************************
|*    SwModify::~SwModify()
|*
|*    Beschreibung      Dokument 1.7
|*    Ersterstellung    JP 20.11.90
|*    Letzte Aenderung  JP 15.04.94
*************************************************************************/



SwModify::~SwModify()
{
    if ( IsInCache() )
        SwFrm::GetCache().Delete( this );

    if ( IsInSwFntCache() )
        pSwFontCache->Delete( this );

    if( pRoot )
    {
        if( IsInDocDTOR() )
        {
            // alle Clients "logisch" austragen
            SwClientIter aIter( *this );
            SwClient* p;
            while( 0 != ( p = aIter++ ) )
                p->pRegisteredIn = 0;

            p = aIter.GoRoot();         // wieder ab Root (==Start) anfangen
            do {
                p->pRegisteredIn = 0;
            } while( 0 != ( p = aIter-- ) );
        }
        else
        {
            // verschicke die Nachricht an die abgeleiteten Objekte.
            SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
            Modify( &aDyObject, &aDyObject );

            // Zwangsummeldung aller derjenigen, die sich nicht ausgetragen
            // haben, durch Aufruf von SwClient::Modify()
            while( pRoot )
                pRoot->SwClient::Modify(&aDyObject, &aDyObject);
        }
    }
}

/*************************************************************************
|*    SwModify::Modify( SwHint * pOldValue, SwHint * pNewValue )
|*
|*    Beschreibung      Dokument 1.7
|*    Ersterstellung    JP 20.11.90
|*    Letzte Aenderung  MA 20. Mar. 95
*************************************************************************/



void SwModify::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
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

#ifndef DBG_UTIL
    bInModify = sal_True;
#else
    if( !pOldValue )
        bInModify = sal_True;
    else
        // following Modifies don't calls an ASSRT
        switch( pOldValue->Which() )
        {
        case RES_OBJECTDYING:
         case RES_REMOVE_UNO_OBJECT:
            bInModify = ((SwPtrMsgPoolItem *)pOldValue)->pObject != this;
            break;

        case RES_FOOTNOTE_DELETED:
        case RES_REFMARK_DELETED:
        case RES_TOXMARK_DELETED:
        case RES_FIELD_DELETED:
            bInModify = sal_False;
            break;
        default:
            bInModify = sal_True;
        }
#endif

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
    UnlockModify();
}

// erfrage vom Modify Informationen

sal_Bool SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    sal_Bool bRet = sal_True;       // bedeutet weiter zum naechsten

    if( pRoot )
    {
        SwClientIter aIter( *(SwModify*)this );

        SwClient* pLast = aIter.GoStart();
        if( pLast )
            while( 0 != ( bRet = pLast->GetInfo( rInfo )) &&
                    0 != ( pLast = aIter++ ) )
                ;
    }

    return bRet;        // und weiter
}

/*************************************************************************
|*    SwModify::Add( SwClient *pDepend )
|*
|*    Beschreibung      Dokument 1.7
|*    Ersterstellung    JP 20.11.90
|*    Letzte Aenderung  JP 14.09.94
*************************************************************************/



void SwModify::Add(SwClient *pDepend)
{
    ASSERT( !bInModify, "Client innerhalb des eigenen Modifies einfuegen?" );

    // nur wenn das hier noch nicht eingetragen ist einfuegen
    if(pDepend->pRegisteredIn != this )
    {
#ifdef DBG_UTIL
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            ASSERT( &pTmp->rRoot != pRoot,
                "Client beim angemeldeten ClientIter einfuegen?" );
            pTmp = pTmp->pNxtIter;
        }
#endif
        // wenn schon wanders angemeldet, dann dort abmelden
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
            // immer hinter die Root haengen
            pDepend->pRight = pRoot->pRight;
            pRoot->pRight = pDepend;
            pDepend->pLeft = pRoot;
            if( pDepend->pRight )
                pDepend->pRight->pLeft = pDepend;
        }

        pDepend->pRegisteredIn = this;
    }
}

/*************************************************************************
|*    SwModify::_Remove( SwClient *pDepend )
|*
|*    Beschreibung      Dokument 1.7
|*    Ersterstellung    JP 20.11.90
|*    Letzte Aenderung  JP 14.09.94
*************************************************************************/



SwClient *SwModify::_Remove(SwClient * pDepend)
{
    // FME 2007-07-16 #i79641# SwXTextMarkup is allowed to be removed ...
    ASSERT( !bInModify || 0 != dynamic_cast<SwXTextMarkup*>(pDepend), "Client innerhalb des eigenen Modifies loeschen?" );

    // loesche das Object aus der Liste und setze den
    // Registrierungs-Pointer zurueck
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

        // alle Client-Iters updaten
        SwClientIter* pTmp = pClientIters;
        while( pTmp )
        {
            if( pTmp->pAkt == pDepend || pTmp->pDelNext == pDepend )
                pTmp->pDelNext = pR;

            // --> FME 2006-02-03 #127369# Notify SwClientIter if mpWatchClient is removed
            if ( pTmp->mpWatchClient == pDepend )
                pTmp->mpWatchClient = 0;
            // <--

            pTmp = pTmp->pNxtIter;
        }

        pDepend->pLeft = 0;
        pDepend->pRight = 0;
    }
    else {
        ASSERT( sal_False, "SwModify::Remove(): pDepend nicht gefunden");
    }
    pDepend->pRegisteredIn = 0;
    return pDepend;
}


/*************************************************************************
|*    SwModify::CheckCaching( const sal_uInt16 nWhich )
|*
|*    Ersterstellung    JP 25.06.95
|*    Letzte Aenderung  JP 25.06.95
*************************************************************************/



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


// ----------
// SwDepend
// ----------

/*************************************************************************
|*    SwDepend::SwDepend(SwClient *pTellHim,SwModify *pDepend)
|*
|*    Beschreibung      callback.doc V1.14
|*    Ersterstellung    VB 20.03.91
|*    Letzte Aenderung  VB 20.03.91
*************************************************************************/


SwDepend::SwDepend(SwClient *pTellHim, SwModify *pDepend)
    : SwClient(pDepend)
{
    pToTell  = pTellHim;
}

/*************************************************************************
|*
|*    SwDepend::Modify(SwHint *, SwHint *)
|*
|*    Beschreibung      callback.doc V1.14
|*    Ersterstellung    VB 20.03.91
|*    Letzte Aenderung  VB 20.03.91
|*
*************************************************************************/


void SwDepend::Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue )
{
    if(pNewValue && pNewValue->Which() == RES_OBJECTDYING)
        SwClient::Modify(pOldValue,pNewValue);
    else if(pToTell)
        pToTell->Modify(pOldValue, pNewValue);
}


    // erfrage vom Modify Informationen
sal_Bool SwDepend::GetInfo( SfxPoolItem& rInfo ) const
{
    return pToTell ? pToTell->GetInfo( rInfo ) : sal_True;
}

/********************************************************************/


SwClientIter::SwClientIter( SwModify const& rModify )
    : rRoot( rModify )
{
    // hinten einketten!
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

    pAkt = rRoot.pRoot;
    pDelNext = pAkt;
    mpWatchClient = 0;
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
                    ASSERT( this, "wo ist mein Pointer" );
                    return ;
                }
            pTmp->pNxtIter = pNxtIter;
        }
    }
}


#ifndef CFRONT
    // Postfix Operator
SwClient* SwClientIter::operator++(int)
{
// solange der CFRONT Prefix und PostFix nicht unterscheiden kann, immer
// als Prefix-Operator arbeiten. Wenn der CFRONT es kann, muss auch der
// Code entsprechen umgestellt werden !!!
    if( pDelNext == pAkt )
    {
        pAkt = pAkt->pRight;
        pDelNext = pAkt;
    }
    else
        pAkt = pDelNext;
    return pAkt;
}
#endif


    // Prefix Operator
SwClient* SwClientIter::operator++()
{
    if( pDelNext == pAkt )
    {
        pAkt = pAkt->pRight;
        pDelNext = pAkt;
    }
    else
        pAkt = pDelNext;
    return pAkt;
}


#ifndef CFRONT
    // Postfix Operator
SwClient* SwClientIter::operator--(int)
{
// solange der CFRONT Prefix und PostFix nicht unterscheiden kann, immer
// als Prefix-Operator arbeiten. Wenn der CFRONT es kann, muss auch der
// Code entsprechen umgestellt werden !!!
    if( pDelNext == pAkt )
        pAkt = pAkt->pLeft;
    else
        pAkt = pDelNext->pLeft;
    pDelNext = pAkt;
    return pAkt;
}
#endif


    // Prefix Operator
SwClient* SwClientIter::operator--()
{
    if( pDelNext == pAkt )
        pAkt = pAkt->pLeft;
    else
        pAkt = pDelNext->pLeft;
    pDelNext = pAkt;
    return pAkt;
}


SwClient* SwClientIter::GoStart()       // zum Anfang des Baums
{
    pAkt = rRoot.pRoot;
    if( pAkt )
        while( pAkt->pLeft )
            pAkt = pAkt->pLeft;
    pDelNext = pAkt;
    return pAkt;
}


SwClient* SwClientIter::GoEnd()         // zum End des Baums
{
    pAkt = pDelNext;
    if( !pAkt )
        pAkt = rRoot.pRoot;
    if( pAkt )
        while( pAkt->pRight )
            pAkt = pAkt->pRight;
    pDelNext = pAkt;
    return pAkt;
}



SwClient* SwClientIter::First( TypeId nType )
{
    aSrchId = nType;
    GoStart();
    if( pAkt )
        do {
            if( pAkt->IsA( aSrchId ) )
                break;

            if( pDelNext == pAkt )
            {
                pAkt = pAkt->pRight;
                pDelNext = pAkt;
            }
            else
                pAkt = pDelNext;

        } while( pAkt );
    return pAkt;
}


SwClient* SwClientIter::Next()
{
    do {
        // erstmal zum naechsten
        if( pDelNext == pAkt )
        {
            pAkt = pAkt->pRight;
            pDelNext = pAkt;
        }
        else
            pAkt = pDelNext;

        if( pAkt && pAkt->IsA( aSrchId ) )
            break;
    } while( pAkt );
    return pAkt;
}



