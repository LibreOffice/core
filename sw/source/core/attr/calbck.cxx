/*************************************************************************
 *
 *  $RCSfile: calbck.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"      // fuer RES_..
#include "frame.hxx"
#include "errhdl.hxx"
#include "hints.hxx"

#define ITERATION           // iterative SwModify::_Insert-Methode
#include "calbck.hxx"
#include "swcache.hxx"
#include "swfntcch.hxx"

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
    bInCache = FALSE;
    bInSwFntCache = FALSE;

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


void SwClient::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
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
BOOL SwClient::GetInfo( SfxPoolItem& ) const
{
    return TRUE;        // und weiter
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

    if( !pRoot || IsModifyLocked() )
        return;


    if ( IsInCache() || IsInSwFntCache() )
    {
        const USHORT nWhich = pOldValue ? pOldValue->Which() :
                                        pNewValue ? pNewValue->Which() : 0;
        CheckCaching( nWhich );
    }

    LockModify();

    bInModify =
#ifdef PRODUCT
                TRUE
#else
        // Modifies von RES_OBJECTDYING sollte kein ASSRT ausloesen
        !pOldValue ||
        (RES_OBJECTDYING != pOldValue->Which() &&
                        RES_REMOVE_UNO_OBJECT != pOldValue->Which()) ||
            ((SwPtrMsgPoolItem *)pOldValue)->pObject != this
#endif
        ;

    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            pLast->Modify( pOldValue, pNewValue );
            if( !pRoot )    // Baum schon Weg ??
                break;
        } while( 0 != ( pLast = aIter++ ));

    bInModify = FALSE;
    UnlockModify();
}

// erfrage vom Modify Informationen

BOOL SwModify::GetInfo( SfxPoolItem& rInfo ) const
{
    BOOL bRet = TRUE;       // bedeutet weiter zum naechsten

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
#ifndef PRODUCT
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
    ASSERT( !bInModify, "Client innerhalb des eigenen Modifies loeschen?" );

    // loesche das Object aus der Liste und setze den
    // Registrierungs-Pointer zurueck
    if( pDepend->pRegisteredIn == this )
    {
        register SwClient* pR = pDepend->pRight;
        register SwClient* pL = pDepend->pLeft;
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
            pTmp = pTmp->pNxtIter;
        }

        pDepend->pLeft = 0;
        pDepend->pRight = 0;
    }
    else
        ASSERT( FALSE, "SwModify::Remove(): pDepend nicht gefunden");
    pDepend->pRegisteredIn = 0;
    return pDepend;
}


/*************************************************************************
|*    SwModify::CheckCaching( const USHORT nWhich )
|*
|*    Ersterstellung    JP 25.06.95
|*    Letzte Aenderung  JP 25.06.95
*************************************************************************/



void SwModify::CheckCaching( const USHORT nWhich )
{
    if( RES_CHRATR_BEGIN <= nWhich && RES_CHRATR_END > nWhich )
        SetInSwFntCache( FALSE );
    else
        switch ( nWhich )
        {
        case RES_OBJECTDYING:
        case RES_FMT_CHG:
        case RES_ATTRSET_CHG:
            SetInSwFntCache( FALSE );

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
                SetInCache( FALSE );
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
BOOL SwDepend::GetInfo( SfxPoolItem& rInfo ) const
{
    return pToTell ? pToTell->GetInfo( rInfo ) : TRUE;
}

/********************************************************************/


SwClientIter::SwClientIter( SwModify& rModify )
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



