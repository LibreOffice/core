/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sot.hxx"

#define _SOT_OBJECT_CXX

#include <tools/debug.hxx>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <agg.hxx>

/************** class SvAggregateMemberList *****************************/
/************************************************************************/
PRV_SV_IMPL_OWNER_LIST(SvAggregateMemberList,SvAggregate);

/************** class SotObject ******************************************/
class SotObjectFactory : public SotFactory
{
public:
         TYPEINFO();
        SotObjectFactory( const SvGlobalName & rName,
                              const String & rClassName,
                              CreateInstanceType pCreateFuncP )
            : SotFactory( rName, rClassName, pCreateFuncP )
        {}
};
TYPEINIT1(SotObjectFactory,SotFactory);


SO2_IMPL_BASIC_CLASS_DLL(SotObject,SotObjectFactory,
                    SvGlobalName( 0xf44b7830, 0xf83c, 0x11d0,
                            0xaa, 0xa1, 0x0, 0xa0, 0x24, 0x9d, 0x55, 0x90 ) )
SO2_IMPL_INVARIANT(SotObject)

/*************************************************************************
|*  SotObject::TestMemberObjRef()
|*
|*  Beschreibung:
*************************************************************************/
void SotObject::TestMemberObjRef( BOOL /*bFree*/ )
{
}

/*************************************************************************
|*  SotObject::TestMemberObjRef()
|*
|*  Beschreibung:
*************************************************************************/
#ifdef TEST_INVARIANT
void SotObject::TestMemberInvariant( BOOL /*bPrint*/ )
{
}
#endif

/*************************************************************************
|*    SotObject::SotObject()
|*
|*    Beschreibung
*************************************************************************/
SotObject::SotObject()
    : pAggList    ( NULL )
    , nStrongLockCount( 0 )
    , nOwnerLockCount( 0 )
    , bOwner      ( TRUE )
    , bSVObject   ( FALSE )
    , bInClose    ( FALSE )
{
    SotFactory::IncSvObjectCount( this );
}

/*************************************************************************
|*
|*    SotObject::~SotObject()
|*
*************************************************************************/
SotObject::~SotObject()
{
    SotFactory::DecSvObjectCount( this );
}

/*************************************************************************
|*    SotObject::GetInterface()
|*
|*    Beschreibung: Um so3 zu helfen
*************************************************************************/
IUnknown * SotObject::GetInterface( const SvGlobalName & )
{
    return NULL;
}

/*************************************************************************
|*    SotObject::IsSvClass()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotObject::IsSvObject() const
{
    return Owner() || bSVObject;
}

/*************************************************************************
|*    SotObject::QueryDelete()
|*
|*    Beschreibung: Bei allen aggregierten Objekte muss der RefCount auf
|*                  0 gehen, damit das Gesammt-Objekt zerstoert wird. Das
|*                  zerst�ren von Teilen ist verboten. Da der Aggregator
|*                  (oder Cast-Verwalter) den Zaehler der aggregierten
|*                  Objekte um 1 erhoeht, muss dies bei der Berechnung
|*                  des 0-RefCounts beruecksichtigt werden.
*************************************************************************/
BOOL SotObject::ShouldDelete()
{
    if( !pAggList )
        return TRUE;

    SvAggregate & rMO = pAggList->GetObject( 0 );
    if(  rMO.bMainObj )
    {
        AddRef();
        pAggList->GetObject( 0 ).pObj->ReleaseRef();
        return FALSE;
    }

   ULONG i;
    for( i = 1; i < pAggList->Count(); i++ )
    {
        SvAggregate & rAgg = pAggList->GetObject( i );
        // Groesser 1, wegen AddRef() bei AddInterface
        if( !rAgg.bFactory && rAgg.pObj->GetRefCount() > 1 )
        {
            // den eigenen hochzaehelen
            AddRef();
            // einen Aggregierten runterzaehlen
            rAgg.pObj->ReleaseRef();
            return FALSE;
        }
    }
    AddNextRef(); // rekursion stoppen
    for( i = pAggList->Count() -1; i > 0; i-- )
    {
        // Referenzen aufloesen
        DBG_ASSERT( !pAggList->GetObject( i ).bMainObj, "main object reference is opened" );
        RemoveInterface( i );
    }
    delete pAggList;
    pAggList = NULL;
    // und zerstoeren, dies ist unabhaengig vom RefCount
    return TRUE;
}

/*************************************************************************
|*    SotObject::QueryDelete()
|*
|*    Beschreibung
*************************************************************************/
void SotObject::QueryDelete()
{
    if( ShouldDelete() )
        SvRefBase::QueryDelete();
}



/*************************************************************************
|*    SotObject::GetAggList()
|*
|*    Beschreibung
*************************************************************************/
SvAggregateMemberList & SotObject::GetAggList()
{
    if( !pAggList )
    {
        pAggList = new SvAggregateMemberList( 2, 1 );
        pAggList->Append( SvAggregate() );
    }
    return *pAggList;
}


/*************************************************************************
|*    SotObject::RemoveInterface()
|*
|*    Beschreibung
*************************************************************************/
void SotObject::RemoveInterface( ULONG nPos )
{
    SvAggregate & rAgg = pAggList->GetObject( nPos );
    if( !rAgg.bFactory )
    {
        DBG_ASSERT( rAgg.pObj->pAggList, "no aggregation list" );
        DBG_ASSERT( rAgg.pObj->pAggList->GetObject( 0 ).pObj == this,
                        "not owner of aggregated object" );
        // sich selbst als Cast-Verwalter austragen
        rAgg.pObj->pAggList->GetObject( 0 ) = SvAggregate();
        // Referenz aufloesen
        rAgg.pObj->ReleaseRef();
        // Aus der eigenen List austragen
        pAggList->Remove( nPos );
    }
}

/*************************************************************************
|*    SotObject::RemoveInterface()
|*
|*    Beschreibung
*************************************************************************/
void SotObject::RemoveInterface( SotObject * pObjP )
{
    DBG_ASSERT( pObjP, "null pointer" );
    DBG_ASSERT( pAggList, "no aggregation list" );
    ULONG i;
    for( i = 0; i < pAggList->Count(); i++ )
    {
        SvAggregate & rAgg = pAggList->GetObject( i );
        if( !rAgg.bFactory && pObjP == rAgg.pObj )
            RemoveInterface( i );
    }
    DBG_ASSERT( i < pAggList->Count(), "object not found" );
}

/*************************************************************************
|*    SotObject::AddInterface()
|*
|*    Beschreibung
*************************************************************************/
void SotObject::AddInterface( SotObject * pObjP )
{
    pObjP->AddRef(); // Objekt festhalten
    GetAggList();
    pAggList->Append( SvAggregate( pObjP, FALSE ) );

    // sich selbst als Typecast-Verwalter eintragen
    SvAggregateMemberList & rAList = pObjP->GetAggList();
    DBG_ASSERT( !rAList.GetObject( 0 ).bMainObj, "try to aggregate twice" );
    rAList[ 0 ] = SvAggregate( this, TRUE );
}

/*************************************************************************
|*    SotObject::AddInterface()
|*
|*    Beschreibung
*************************************************************************/
void SotObject::AddInterface( SotFactory * pFactP )
{
    GetAggList();
    pAggList->Append( SvAggregate( pFactP ) );
}

/*************************************************************************
|*    SotObject::CreateAggObj()
|*
|*    Beschreibung
*************************************************************************/
SotObjectRef SotObject::CreateAggObj( const SotFactory * )
{
    return SotObjectRef();
}


/*************************************************************************
|*    SotObject::DownAggCast()
|*
|*    Beschreibung
*************************************************************************/
void * SotObject::DownAggCast( const SotFactory * pFact )
{
    void * pCast = NULL;
    // geht den Pfad nur Richtung aggregierte Objekte
    if( pAggList )
    {
        for( ULONG i = 1; !pCast || i < pAggList->Count(); i++ )
        {
            SvAggregate & rAgg = pAggList->GetObject( i );
            if( rAgg.bFactory )
            {
                if( rAgg.pFact->Is( pFact ) )
                {
                    // On-Demand erzeugen, wenn Typ gebraucht
                    SotObjectRef aObj( CreateAggObj( rAgg.pFact ) );
                    rAgg.bFactory = FALSE;
                    rAgg.pObj = aObj;
                    rAgg.pObj->AddRef();

                    // sich selbst als Typecast-Verwalter eintragen
                    SvAggregateMemberList & rAList = rAgg.pObj->GetAggList();
                    DBG_ASSERT( !rAList.GetObject( 0 ).bMainObj, "try to aggregate twice" );
                    rAList[ 0 ] = SvAggregate( this, TRUE );
                }
            }
            if( !rAgg.bFactory )
            {
                // muss der (void *) auf Klasse pFact sein
                pCast = rAgg.pObj->Cast( pFact );
                if( !pCast )
                    pCast = rAgg.pObj->DownAggCast( pFact );
                if( pCast )
                    break;
            }
        }
    }
    return pCast;
}

/*************************************************************************
|*    SotObject::AggCast()
|*
|*    Beschreibung
*************************************************************************/
void * SotObject::AggCast( const SotFactory * pFact )
{
    void * pCast = NULL;
    if( pAggList )
    {
        SvAggregate & rAgg = pAggList->GetObject( 0 );
        if( rAgg.bMainObj )
            return rAgg.pObj->AggCast( pFact );
        pCast = Cast( pFact );
        if( !pCast )
            pCast = DownAggCast( pFact );
    }
    else
        pCast = Cast( pFact );
    return pCast;
}

/*************************************************************************
|*    SotObject::CastAndAddRef()
|*
|*    Beschreibung
*************************************************************************/
void * SotObject::CastAndAddRef( const SotFactory * pFact )
{
    void * pCast = Cast( pFact );
    if( pCast )
        AddRef();
    return pCast;
}

/*************************************************************************
|*    SotObject::GetMainObj()
|*
|*    Beschreibung
*************************************************************************/
SotObject * SotObject::GetMainObj() const
{
    if( pAggList )
    {
        if( pAggList->GetObject( 0 ).bMainObj )
            return pAggList->GetObject( 0 ).pObj->GetMainObj();
    }
    return (SotObject *)this;
}

//=========================================================================
USHORT SotObject::FuzzyLock
(
    BOOL bLock,         /* TRUE, lock. FALSE, unlock. */
    BOOL /*bIntern*/,   /* TRUE, es handelt sich um einen internen Lock.
                           FALSE, der Lock kam von aussen (Ole2, Ipc2) */
    BOOL bClose         /* TRUE, Close aufrufen wenn letzte Lock */
)
/*  [Beschreibung]

    Erlaubte Parameterkombinationen:
    ( TRUE,  TRUE,  *     ) ->  interner Lock.
    ( FALSE, TRUE,  TRUE  ) ->  interner Unlock mit Close,
                                 wenn LockCount() == 0
    ( TRUE,  FALSE, *     ) ->  externer Lock.
    ( FALSE, FALSE, TRUE  ) ->  externer Unlock mit Close,
                                 wenn LockCount() == 0
    ( FALSE, FALSE, FALSE ) ->  externer Unlock

    F"ur !Owner() wird der Aufruf an das externe Objekt weitergeleitet.
     F"ur diese muss das <IOleItemContainer>-Interface zur Vef"ugung stehen.
    bIntern und bClose werden dann ignoriert.
    Wenn der LockCount auf 0 wechselt, wird <SotObject::DoClose>
    gerufen, wenn kein OwnerLock besteht.

    [Anmerkung]

*/
{
    SotObjectRef xHoldAlive( this );
    USHORT nRet;
    if( bLock )
    {
        AddRef();
        nRet = ++nStrongLockCount;
    }
    else
    {
        nRet = --nStrongLockCount;
        ReleaseRef();
    }

    if( !nRet && bClose && !nOwnerLockCount )
        DoClose();
    return nRet;
}

//=========================================================================
void SotObject::OwnerLock
(
    BOOL bLock      /* TRUE, lock. FALSE, unlock. */
)
/*  [Beschreibung]

    Wenn der OwnerLock auf Null dekrementiert, dann wird die Methode
    DoClose gerufen. Dies geschieht unabh"angig vom Lock. bzw. RefCount.
    Ist der OwnerLock-Z"ahler != Null, dann wird kein DoClose durch
    <SotObject::FuzzyLock> gerufen.
*/
{
    if( bLock )
    {
        nOwnerLockCount++;
        AddRef();
    }
    else if ( nOwnerLockCount )
    {
        if( 0 == --nOwnerLockCount )
            DoClose();
        ReleaseRef();
    }
}

void SotObject::RemoveOwnerLock()
{
    if ( nOwnerLockCount )
    {
        --nOwnerLockCount;
        ReleaseRef();
    }
    else {
        OSL_FAIL("OwnerLockCount underflow!");
    }
}

//=========================================================================
BOOL SotObject::DoClose()
{
    BOOL bRet = FALSE;
    if( !bInClose )
    {
        SotObjectRef xHoldAlive( this );
        bInClose = TRUE;
        bRet = Close();
        bInClose = FALSE;
    }
    return bRet;
}

//=========================================================================
BOOL SotObject::Close()
{
    return TRUE;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
