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


#include <string.h>

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#ifndef GCC
#endif

#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS

#include <bf_svtools/svstdarr.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemiter.hxx>
#include "whiter.hxx"
#include <bf_svtools/nranges.hxx>
#include "whassert.hxx"

#include <tools/stream.hxx>
#include <tools/solar.h>

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

static const USHORT nInitCount = 10; // einzelne USHORTs => 5 Paare ohne '0'
#ifdef DBG_UTIL
static ULONG nRangesCopyCount = 0;	 // wie oft wurden Ranges kopiert
#endif

DBG_NAME(SfxItemSet)

//========================================================================

#define NUMTYPE 		USHORT
#define SvNums			SvUShorts
#define SfxNumRanges    SfxUShortRanges
#include "svt_nranges.cxx"
#undef NUMTYPE
#undef SvNums
#undef SfxNumRanges

#define NUMTYPE 		ULONG
#define SvNums			SvULongs
#define SfxNumRanges    SfxULongRanges
#include "svt_nranges.cxx"
#undef NUMTYPE
#undef SvNums
#undef SfxNumRanges

//========================================================================

#ifdef DBG_UTIL


const sal_Char *DbgCheckItemSet( const void* pVoid )
{
    const SfxItemSet *pSet = (const SfxItemSet*) pVoid;
    SfxWhichIter aIter( *pSet );
    USHORT nCount = 0, n = 0;
    for ( USHORT nWh = aIter.FirstWhich(); nWh; nWh = aIter.NextWhich(), ++n )
    {
        const SfxPoolItem *pItem = pSet->_aItems[n];
        if ( pItem )
        {
            ++nCount;
            DBG_ASSERT( IsInvalidItem(pItem) ||
                        pItem->Which() == 0 || pItem->Which() == nWh,
                        "SfxItemSet: invalid which-id" );
            DBG_ASSERT( IsInvalidItem(pItem) || !pItem->Which() ||
                    !SfxItemPool::IsWhich(pItem->Which()) ||
                    pSet->GetPool()->IsItemFlag(nWh, SFX_ITEM_NOT_POOLABLE) ||
                    SFX_ITEMS_NULL != pSet->GetPool()->GetSurrogate(pItem),
                    "SfxItemSet: item in set which is not in pool" );
        }

    }
    DBG_ASSERT( pSet->_nCount == nCount, "wrong SfxItemSet::nCount detected" );

    return 0;
}

#endif
// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet
(
    SfxItemPool&	rPool,  		/* der Pool, in dem die SfxPoolItems,
                                       welche in dieses SfxItemSet gelangen,
                                       aufgenommen werden sollen */
    BOOL
#ifdef DBG_UTIL
#ifdef SFX_ITEMSET_NO_DEFAULT_CTOR

                    bTotalRanges	/* komplette Pool-Ranges uebernehmen,
                                       muss auf TRUE gesetzt werden */
#endif
#endif
)
/*	[Beschreibung]

    Konstruktor fuer ein SfxItemSet mit genau den Which-Bereichen, welche
    dem angegebenen <SfxItemPool> bekannt sind.


    [Anmerkung]

    F"ur Sfx-Programmierer ein derart konstruiertes SfxItemSet kann
    keinerlei Items mit Slot-Ids als Which-Werte aufnehmen!
*/

:	_pPool( &rPool ),
    _pParent( 0 ),
    _nCount( 0 )
{
    DBG_CTOR(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );
//	DBG_ASSERT( bTotalRanges || abs( &bTotalRanges - this ) < 1000,
//				"please use suitable ranges" );
#ifdef DBG_UTIL
#ifdef SFX_ITEMSET_NO_DEFAULT_CTOR
    if ( !bTotalRanges )
        *(int*)0 = 0; // GPF
#endif
#endif

    _pWhichRanges = (USHORT*) _pPool->GetFrozenIdRanges();
    DBG_ASSERT( _pWhichRanges, "don't create ItemSets with full range before FreezeIdRanges()" );
    if ( !_pWhichRanges )
        _pPool->FillItemIdRanges_Impl( _pWhichRanges );

    const USHORT nSize = TotalCount();
    _aItems = new const SfxPoolItem* [ nSize ];
    memset( (void*) _aItems, 0, nSize * sizeof( SfxPoolItem* ) );
}

// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 ):
    _pPool( &rPool ),
    _pParent( 0 ),
    _nCount( 0 )
{
    DBG_CTOR(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( nWhich1 <= nWhich2, "Ungueltiger Bereich" );
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );

    InitRanges_Impl(nWhich1, nWhich2);
}

// -----------------------------------------------------------------------

void SfxItemSet::InitRanges_Impl(USHORT nWh1, USHORT nWh2)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    _pWhichRanges = new USHORT[ 3 ];
    *(_pWhichRanges+0) = nWh1;
    *(_pWhichRanges+1) = nWh2;
    *(_pWhichRanges+2) = 0;
    const USHORT nRg = nWh2 - nWh1 + 1;
    _aItems = new const SfxPoolItem* [ nRg ];
    memset( (void*) _aItems, 0, nRg * sizeof( SfxPoolItem* ) );
}

//------------------------------------------------------------------------

USHORT InitializeRanges_Impl( USHORT *&rpRanges, va_list pArgs,
                               USHORT nWh1, USHORT nWh2, USHORT nNull )

/**	<H3>Description</H3>

    Creates an USHORT-ranges-array in 'rpRanges' using 'nWh1' and 'nWh2' as
    first range, 'nNull' as terminator or start of 2nd range and 'pArgs' as
    remaider.

    It returns the number of USHORTs which are contained in the described
    set of USHORTs.
*/

{
    USHORT nSize = 0, nIns = 0;
    USHORT nCnt = 0;
    SvUShorts aNumArr( 11, 8 );
    aNumArr.Insert( nWh1, nCnt++ );
    aNumArr.Insert( nWh2, nCnt++ );
    DBG_ASSERT( nWh1 <= nWh2, "Ungueltiger Bereich" );
    nSize += nWh2 - nWh1 + 1;
    aNumArr.Insert( nNull, nCnt++ );
    while ( 0 !=
            ( nIns =
              sal::static_int_cast< USHORT >(
                  va_arg( pArgs, USHORT_ARG ) ) ) )
    {
        aNumArr.Insert( nIns, nCnt++ );
        if ( 0 == (nCnt & 1) )		 // 4,6,8, usw.
        {
            DBG_ASSERT( aNumArr[ nCnt-2 ] <= nIns, "Ungueltiger Bereich" );
            nSize += nIns - aNumArr[ nCnt-2 ] + 1;
        }
    }
    va_end( pArgs );

    DBG_ASSERT( 0 == (nCnt & 1), "ungerade Anzahl von Which-Paaren!" );

    // so, jetzt sind alle Bereiche vorhanden und
    rpRanges = new USHORT[ nCnt+1 ];
    memcpy( rpRanges, aNumArr.GetData(), sizeof(USHORT) * nCnt );
    *(rpRanges+nCnt) = 0;

    return nSize;
}

// -----------------------------------------------------------------------

void SfxItemSet::InitRanges_Impl(va_list pArgs, USHORT nWh1, USHORT nWh2, USHORT nNull)
{
    DBG_CHKTHIS(SfxItemSet, 0);

    USHORT nSize = InitializeRanges_Impl( _pWhichRanges, pArgs, nWh1, nWh2, nNull );
    _aItems = new const SfxPoolItem* [ nSize ];
    memset( (void*) _aItems, 0, sizeof( SfxPoolItem* ) * nSize );
}

// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( SfxItemPool& rPool,
                        USHORT_ARG nWh1, USHORT_ARG nWh2, USHORT_ARG nNull, ... ):
    _pPool( &rPool ),
    _pParent( 0 ),
    _pWhichRanges( 0 ),
    _nCount( 0 )
{
    DBG_CTOR(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( nWh1 <= nWh2, "Ungueltiger Bereich" );
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );

    if(!nNull)
        InitRanges_Impl(
            sal::static_int_cast< USHORT >(nWh1),
            sal::static_int_cast< USHORT >(nWh2));
    else {
        va_list pArgs;
        va_start( pArgs, nNull );
        InitRanges_Impl(
            pArgs, sal::static_int_cast< USHORT >(nWh1),
            sal::static_int_cast< USHORT >(nWh2),
            sal::static_int_cast< USHORT >(nNull));
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::InitRanges_Impl(const USHORT *pWhichPairTable)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    DBG_TRACE1("SfxItemSet: Ranges-CopyCount==%ul", ++nRangesCopyCount);

    USHORT nCnt = 0;
    const USHORT* pPtr = pWhichPairTable;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    _aItems = new const SfxPoolItem* [ nCnt ];
    memset( (void*) _aItems, 0, sizeof( SfxPoolItem* ) * nCnt );

    std::ptrdiff_t cnt = pPtr - pWhichPairTable +1;
    _pWhichRanges = new USHORT[ cnt ];
    memcpy( _pWhichRanges, pWhichPairTable, sizeof( USHORT ) * cnt );
}


// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( SfxItemPool& rPool, const USHORT* pWhichPairTable ):
    _pPool( &rPool ),
    _pParent( 0 ),
    _pWhichRanges(0),
    _nCount( 0 )
{
    DBG_CTOR(SfxItemSet, 0);
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );

    // pWhichPairTable == 0 ist f"ur das SfxAllEnumItemSet
    if ( pWhichPairTable )
        InitRanges_Impl(pWhichPairTable);
}

// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( const SfxItemSet& rASet ):
    _pPool( rASet._pPool ),
    _pParent( rASet._pParent ),
    _nCount( rASet._nCount )
{
    DBG_CTOR(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );
    DBG( ++*_pChildCount(_pParent) );

    // errechne die Anzahl von Attributen
    USHORT nCnt = 0;
    USHORT* pPtr = rASet._pWhichRanges;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    _aItems = new const SfxPoolItem* [ nCnt ];

    // Attribute kopieren
    SfxItemArray ppDst = _aItems, ppSrc = rASet._aItems;
    for( USHORT n = nCnt; n; --n, ++ppDst, ++ppSrc )
        if ( 0 == *ppSrc || 				// aktueller Default?
             IsInvalidItem(*ppSrc) ||		// Dont Care?
             IsStaticDefaultItem(*ppSrc) )	// nicht zu poolende Defaults
            // einfach Pointer kopieren
            *ppDst = *ppSrc;
        else if ( _pPool->IsItemFlag( **ppSrc, SFX_ITEM_POOLABLE ) )
        {
            // einfach Pointer kopieren und Ref-Count erh"ohen
            *ppDst = *ppSrc;
            ( (SfxPoolItem*) (*ppDst) )->AddRef();
        }
        else if ( !(*ppSrc)->Which() )
            *ppDst = (*ppSrc)->Clone();
        else
            // !IsPoolable() => via Pool zuweisen
            *ppDst = &_pPool->Put( **ppSrc );

    // dann noch die Which Ranges kopieren
    DBG_TRACE1("SfxItemSet: Ranges-CopyCount==%ul", ++nRangesCopyCount);
    std::ptrdiff_t cnt = pPtr - rASet._pWhichRanges+1;
    _pWhichRanges = new USHORT[ cnt ];
    memcpy( _pWhichRanges, rASet._pWhichRanges, sizeof( USHORT ) * cnt);
}

// -----------------------------------------------------------------------

SfxItemSet::~SfxItemSet()
{
    DBG_DTOR(SfxItemSet, DbgCheckItemSet);
#ifdef DBG_UTIL
    DBG( DBG_ASSERT( 0 == *_pChildCount(this), "SfxItemSet: deleting parent-itemset" ) )
#endif

    USHORT nCount = TotalCount();
    if( Count() )
    {
        SfxItemArray ppFnd = _aItems;
        for( USHORT nCnt = nCount; nCnt; --nCnt, ++ppFnd )
            if( *ppFnd && !IsInvalidItem(*ppFnd) )
            {
                if( !(*ppFnd)->Which() )
                    delete (SfxPoolItem*) *ppFnd;
                else {
                    // noch mehrer Referenzen vorhanden, also nur den
                    // ReferenzCounter manipulieren
                    if ( 1 < (*ppFnd)->GetRefCount() && !IsDefaultItem(*ppFnd) )
                        (*ppFnd)->ReleaseRef();
                    else
                        if ( !IsDefaultItem(*ppFnd) )
                            // aus dem Pool loeschen
                            _pPool->Remove( **ppFnd );
                }
            }
    }

    // FIXME: could be delete[] (SfxPoolItem **)_aItems;
    delete[] _aItems;
    if ( _pWhichRanges != _pPool->GetFrozenIdRanges() )
        delete[] _pWhichRanges;
    _pWhichRanges = 0; // for invariant-testing

    DBG( --*_pChildCount(_pParent) );
    DBG( delete _pChildCount(this); _pChildCountDtor );
}

// -----------------------------------------------------------------------

USHORT SfxItemSet::ClearItem( USHORT nWhich )

// einzelnes Item oder alle Items (nWhich==0) l"oschen

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if( !Count() )
        return 0;

    USHORT nDel = 0;
    SfxItemArray ppFnd = _aItems;

    if( nWhich )
    {
        const USHORT* pPtr = _pWhichRanges;
        while( *pPtr )
        {
            // in diesem Bereich?
            if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
            {
                // "uberhaupt gesetzt?
                ppFnd += nWhich - *pPtr;
                if( *ppFnd )
                {
                    // wegen der Assertions ins Sub-Calls mu\s das hier sein
                    --_nCount;
                    const SfxPoolItem *pItemToClear = *ppFnd;
                    *ppFnd = 0;

                    if ( !IsInvalidItem(pItemToClear) )
                    {
                        if ( nWhich <= SFX_WHICH_MAX )
                        {
                            const SfxPoolItem& rNew = _pParent
                                    ? _pParent->Get( nWhich, TRUE )
                                    : _pPool->GetDefaultItem( nWhich );

                            Changed( *pItemToClear, rNew );
                        }
                        if ( pItemToClear->Which() )
                            _pPool->Remove( *pItemToClear );
                    }
                    ++nDel;
                }

                // gefunden => raus
                break;
            }
            ppFnd += *(pPtr+1) - *pPtr + 1;
            pPtr += 2;
        }
    }
    else
    {
        nDel = _nCount;

        USHORT* pPtr = _pWhichRanges;
        while( *pPtr )
        {
            for( nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( *ppFnd )
                {
                    // wegen der Assertions ins Sub-Calls mu\s das hier sein
                    --_nCount;
                    const SfxPoolItem *pItemToClear = *ppFnd;
                    *ppFnd = 0;

                    if ( !IsInvalidItem(pItemToClear) )
                    {
                        if ( nWhich <= SFX_WHICH_MAX )
                        {
                            const SfxPoolItem& rNew = _pParent
                                    ? _pParent->Get( nWhich, TRUE )
                                    : _pPool->GetDefaultItem( nWhich );

                            Changed( *pItemToClear, rNew );
                        }

                        // #i32448#
                        // Take care of disabled items, too.
                        if(!pItemToClear->nWhich)
                        {
                            // item is disabled, delete it
                            delete pItemToClear;
                        }
                        else
                        {
                            // remove item from pool
                            _pPool->Remove( *pItemToClear );
                        }
                    }
                }
            pPtr += 2;
        }
    }
    return nDel;
}

// -----------------------------------------------------------------------

void SfxItemSet::ClearInvalidItems( BOOL bHardDefault )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    USHORT* pPtr = _pWhichRanges;
    SfxItemArray ppFnd = _aItems;
    if ( bHardDefault )
        while( *pPtr )
        {
            for ( USHORT nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if ( IsInvalidItem(*ppFnd) )
                     *ppFnd = &_pPool->Put( _pPool->GetDefaultItem(nWhich) );
            pPtr += 2;
        }
    else
        while( *pPtr )
        {
            for( USHORT nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( IsInvalidItem(*ppFnd) )
                {
                    *ppFnd = 0;
                    --_nCount;
                }
            pPtr += 2;
        }
}

//------------------------------------------------------------------------


void SfxItemSet::InvalidateAllItems()
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( !_nCount, "Es sind noch Items gesetzt" );

    memset( (void*)_aItems, -1, ( _nCount = TotalCount() ) * sizeof( SfxPoolItem*) );
}

// -----------------------------------------------------------------------

SfxItemState SfxItemSet::GetItemState( USHORT nWhich,
                                        BOOL bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    // suche den Bereich in dem das Which steht:
    const SfxItemSet* pAktSet = this;
    SfxItemState eRet = SFX_ITEM_UNKNOWN;
    do
    {
        SfxItemArray ppFnd = pAktSet->_aItems;
        const USHORT* pPtr = pAktSet->_pWhichRanges;
        if (pPtr)
        {
            while ( *pPtr )
            {
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // in diesem Bereich
                    ppFnd += nWhich - *pPtr;
                    if ( !*ppFnd )
                    {
                        eRet = SFX_ITEM_DEFAULT;
                        if( !bSrchInParent )
                            return eRet;  // nicht vorhanden
                        break; // JP: in den Parents weitersuchen !!!
                    }

                    if ( (SfxPoolItem*) -1 == *ppFnd )
                        // Unterschiedlich vorhanden
                        return SFX_ITEM_DONTCARE;

                    if ( (*ppFnd)->IsA(TYPE(SfxVoidItem)) )
                        return SFX_ITEM_DISABLED;

                    if (ppItem)
                    {
                        #ifdef DBG_UTIL
                        const SfxPoolItem *pItem = *ppFnd;
                        DBG_ASSERT( !pItem->ISA(SfxSetItem) ||
                                0 != &((const SfxSetItem*)pItem)->GetItemSet(),
                                "SetItem without ItemSet" );
                        #endif
                        *ppItem = *ppFnd;
                    }
                    return SFX_ITEM_SET;
                }
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
    } while( bSrchInParent && 0 != ( pAktSet = pAktSet->_pParent ));
    return eRet;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxItemSet::Put( const SfxPoolItem& rItem, USHORT nWhich )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
            0 != &((const SfxSetItem&)rItem).GetItemSet(),
            "SetItem without ItemSet" );
    if ( !nWhich )
        return 0; //! nur wegen Outliner-Bug
    SfxItemArray ppFnd = _aItems;
    const USHORT* pPtr = _pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // in diesem Bereich
            ppFnd += nWhich - *pPtr;
            if( *ppFnd )		// schon einer vorhanden
            {
                // selbes Item bereits vorhanden?
                if ( *ppFnd == &rItem )
                    return 0;

                // wird dontcare oder disabled mit was echtem ueberschrieben?
                if ( rItem.Which() && ( IsInvalidItem(*ppFnd) || !(*ppFnd)->Which() ) )
                {
                    *ppFnd = &_pPool->Put( rItem, nWhich );
                    return *ppFnd;
                }

                // wird disabled?
                if( !rItem.Which() )
                {
                    *ppFnd = rItem.Clone(_pPool);
                    return 0;
                }
                else
                {
                    // selber Wert bereits vorhanden?
                    if ( rItem == **ppFnd )
                        return 0;

                    // den neuen eintragen, den alten austragen
                    const SfxPoolItem& rNew = _pPool->Put( rItem, nWhich );
                    const SfxPoolItem* pOld = *ppFnd;
                    *ppFnd = &rNew;
                    if(nWhich <= SFX_WHICH_MAX)
                        Changed( *pOld, rNew );
                    _pPool->Remove( *pOld );
                }
            }
            else
            {
                ++_nCount;
                if( !rItem.Which() )
                    *ppFnd = rItem.Clone(_pPool);
                else {
                    const SfxPoolItem& rNew = _pPool->Put( rItem, nWhich );
                    *ppFnd = &rNew;
                    if (nWhich <= SFX_WHICH_MAX )
                    {
                        const SfxPoolItem& rOld = _pParent
                            ? _pParent->Get( nWhich, TRUE )
                            : _pPool->GetDefaultItem( nWhich );
                        Changed( rOld, rNew );
                    }
                }
            }
            SFX_ASSERT( !_pPool->IsItemFlag(nWhich, SFX_ITEM_POOLABLE) ||
                        rItem.ISA(SfxSetItem) || **ppFnd == rItem,
                        nWhich, "putted Item unequal" );
            return *ppFnd;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return 0;
}

// -----------------------------------------------------------------------

int SfxItemSet::Put( const SfxItemSet& rSet, BOOL bInvalidAsDefault )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    BOOL bRet = FALSE;
    if( rSet.Count() )
    {
        SfxItemArray ppFnd = rSet._aItems;
        const USHORT* pPtr = rSet._pWhichRanges;
        while ( *pPtr )
        {
            for ( USHORT nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( *ppFnd )
                {
                    if ( IsInvalidItem( *ppFnd ) )
                    {
                        if ( bInvalidAsDefault )
                            bRet |= 0 != ClearItem( nWhich );
                            // gab GPF bei non.WIDs:
                            // bRet |= 0 != Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                        else
                            InvalidateItem( nWhich );
                    }
                    else
                        bRet |= 0 != Put( **ppFnd, nWhich );
                }
            pPtr += 2;
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------

void SfxItemSet::PutExtended
(
    const SfxItemSet&	rSet,			// Quelle der zu puttenden Items
    SfxItemState		eDontCareAs,	// was mit DontCare-Items passiert
    SfxItemState		eDefaultAs		// was mit Default-Items passiert
)

/*	[Beschreibung]

    Diese Methode "ubernimmt die Items aus 'rSet' in '*this'. Die
    Which-Bereiche in '*this', die in 'rSet' nicht vorkommen bleiben unver-
    "andert. Der Which-Bereich von '*this' bleibt auch unver"andert.

    In 'rSet' gesetzte Items werden auch in '*this*' gesetzt. Default-
    (0 Pointer) und Invalid- (-1 Pointer) Items werden je nach Parameter
    ('eDontCareAs' und 'eDefaultAs' behandelt:

    SFX_ITEM_SET:		hart auf Default des Pools gesetzt
    SFX_ITEM_DEFAULT:	gel"oscht (0 Pointer)
    SFX_ITEM_DONTCARE:	invalidiert (-1 Pointer)

    Alle anderen Werte f"ur 'eDontCareAs' und 'eDefaultAs' sind ung"ultig.
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);

    // don't "optimize" with "if( rSet.Count()" because of dont-care + defaults
    SfxItemArray ppFnd = rSet._aItems;
    const USHORT* pPtr = rSet._pWhichRanges;
    while ( *pPtr )
    {
        for ( USHORT nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
            if( *ppFnd )
            {
                if ( IsInvalidItem( *ppFnd ) )
                {
                    // Item ist DontCare:
                    switch ( eDontCareAs )
                    {
                        case SFX_ITEM_SET:
                            Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                            break;

                        case SFX_ITEM_DEFAULT:
                            ClearItem( nWhich );
                            break;

                        case SFX_ITEM_DONTCARE:
                            InvalidateItem( nWhich );
                            break;

                        default:
                            DBG_ERROR( "invalid Argument for eDontCareAs" );
                    }
                }
                else
                    // Item ist gesetzt:
                    Put( **ppFnd, nWhich );
            }
            else
            {
                // Item ist Default:
                switch ( eDefaultAs )
                {
                    case SFX_ITEM_SET:
                        Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                        break;

                    case SFX_ITEM_DEFAULT:
                        ClearItem( nWhich );
                        break;

                    case SFX_ITEM_DONTCARE:
                        InvalidateItem( nWhich );
                        break;

                    default:
                        DBG_ERROR( "invalid Argument for eDefaultAs" );
                }
            }
        pPtr += 2;
    }
}

// -----------------------------------------------------------------------

int SfxItemSet::Set
(
    const SfxItemSet&	rSet,	/*	das SfxItemSet, dessen SfxPoolItems
                                    "ubernommen werden sollen */

    BOOL				bDeep	/*	TRUE (default)
                                    auch die SfxPoolItems aus den ggf. an
                                    rSet vorhandenen Parents werden direkt
                                    in das SfxItemSet "ubernommen

                                    FALSE
                                    die SfxPoolItems aus den Parents von
                                    rSet werden nicht ber"ucksichtigt */
)

/*	[Beschreibung]

    Das SfxItemSet nimmt genau die SfxPoolItems an, die auch in
    rSet gesetzt sind und im eigenen <Which-Bereich> liegen. Alle
    anderen werden entfernt. Der SfxItemPool wird dabei beibehalten,
    so da"s die "ubernommenen SfxPoolItems dabei ggf. vom SfxItemPool
    von rSet in den SfxItemPool von *this "ubernommen werden.

    SfxPoolItems, f"ur die in rSet IsInvalidItem() == TRUE gilt,
    werden als Invalid-Item "ubernommen.


    [R"uckgabewert]

    int 							TRUE
                                    es wurden SfxPoolItems "ubernommen

                                    FALSE
                                    es wurden keine SfxPoolItems "ubernommen,
                                    da z.B. die Which-Bereiche der SfxItemSets
                                    keine Schnittmenge haben oder in der
                                    Schnittmenge keine SfxPoolItems in rSet
                                    gesetzt sind

*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    int bRet = FALSE;
    if ( _nCount )
        ClearItem();
    if ( bDeep )
    {
        SfxWhichIter aIter(*this);
        USHORT nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rSet.GetItemState( nWhich, TRUE, &pItem ) )
                bRet |= 0 != Put( *pItem, pItem->Which() );
            nWhich = aIter.NextWhich();
        }
    }
    else
        bRet = Put(rSet, FALSE);

    return bRet;
}

//------------------------------------------------------------------------

const SfxPoolItem* SfxItemSet::GetItem
(
    USHORT 				nId,   			// Slot-Id oder Which-Id des Items
    BOOL 				bSrchInParent,  // TRUE: auch in Parent-ItemSets suchen
    TypeId 				aItemType       // != 0 =>  RTTI Pruefung mit Assertion
)	const

/*	[Beschreibung]

    Mit dieser Methode wird der Zugriff auf einzelne Items im
    SfxItemSet wesentlich vereinfacht. Insbesondere wird die Typpr"ufung
    (per Assertion) durchgef"uhrt, wodurch die Applikations-Sourcen
    wesentlich "ubersichtlicher werden. In der PRODUCT-Version wird
    eine 0 zur"uckgegeben, wenn das gefundene Item nicht von der
    angegebenen Klasse ist. Ist kein Item mit der Id 'nWhich' in dem ItemSet,
    so wird 0 zurueckgegeben.
*/

{
    // ggf. in Which-Id umrechnen
    USHORT nWhich = GetPool()->GetWhich(nId);

    // ist das Item gesetzt oder bei bDeep==TRUE verf"ugbar?
    const SfxPoolItem *pItem = 0;
    SfxItemState eState = GetItemState( nWhich, bSrchInParent, &pItem );
    if ( bSrchInParent && SFX_ITEM_AVAILABLE == eState &&
         nWhich <= SFX_WHICH_MAX )
        pItem = &_pPool->GetDefaultItem(nWhich);
    if ( pItem )
    {
        // stimmt der Typ "uberein?
        if ( !aItemType || pItem->IsA(aItemType) )
            return pItem;

        // sonst Fehler melden
        DBG_ERROR( "invalid argument type" );
    }

    // kein Item gefunden oder falschen Typ gefunden
    return 0;
}


//------------------------------------------------------------------------


const SfxPoolItem& SfxItemSet::Get( USHORT nWhich, BOOL bSrchInParent) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    // suche den Bereich in dem das Which steht:
    const SfxItemSet* pAktSet = this;
    do
    {
        if( pAktSet->Count() )
        {
            SfxItemArray ppFnd = pAktSet->_aItems;
            const USHORT* pPtr = pAktSet->_pWhichRanges;
            while( *pPtr )
            {
                if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // in diesem Bereich
                    ppFnd += nWhich - *pPtr;
                    if( *ppFnd )
                    {
                        if( (SfxPoolItem*)-1 == *ppFnd ) {
                            //?MI: folgender code ist Doppelt (unten)
                            SFX_ASSERT(_pPool, nWhich, "kein Pool, aber Status uneindeutig");
                            //!((SfxAllItemSet *)this)->aDefault.SetWhich(nWhich);
                            //!return aDefault;
                            return _pPool->GetDefaultItem( nWhich );
                        }
#ifdef DBG_UTIL
                        const SfxPoolItem *pItem = *ppFnd;
                        DBG_ASSERT( !pItem->ISA(SfxSetItem) ||
                                0 != &((const SfxSetItem*)pItem)->GetItemSet(),
                                "SetItem without ItemSet" );
                        if ( pItem->ISA(SfxVoidItem) || !pItem->Which() )
                            DBG_WARNING( "SFX_WARNING: Getting disabled Item" );
#endif
                        return **ppFnd;
                    }
                    break; 			// dann beim Parent suchen
                }
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
// bis zum Ende vom Such-Bereich: was nun ? zum Parent, oder Default ??
//		if( !*pPtr )			// bis zum Ende vom Such-Bereich ?
//		break;
    } while( bSrchInParent && 0 != ( pAktSet = pAktSet->_pParent ));

    // dann das Default vom Pool holen und returnen
    SFX_ASSERT(_pPool, nWhich, "kein Pool, aber Status uneindeutig");
    const SfxPoolItem *pItem = &_pPool->GetDefaultItem( nWhich );
    DBG_ASSERT( !pItem->ISA(SfxSetItem) ||
            0 != &((const SfxSetItem*)pItem)->GetItemSet(),
            "SetItem without ItemSet" );
    return *pItem;
}

    // Notification-Callback
// -----------------------------------------------------------------------

void SfxItemSet::Changed( const SfxPoolItem&, const SfxPoolItem& )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
}

// -----------------------------------------------------------------------

USHORT SfxItemSet::TotalCount() const
{
    DBG_CHKTHIS(SfxItemSet, 0); // wird im Ctor benutzt bevor vollst. init.
    USHORT nRet = 0;
    USHORT* pPtr = _pWhichRanges;
    while( *pPtr )
    {
        nRet += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }
    return nRet;
}
// -----------------------------------------------------------------------

// behalte nur die Items, die auch in rSet enthalten sein (Wert egal)

void SfxItemSet::Intersect( const SfxItemSet& rSet )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT(_pPool, "nicht implementiert ohne Pool");
    if( !Count() )		 // gar keine gesetzt ?
        return;

    // loesche alle Items, die im rSet nicht mehr vorhanden sind
    if( !rSet.Count() )
    {
        ClearItem();		// alles loeschen
        return;
    }

    // teste mal, ob sich die Which-Bereiche unterscheiden.
    BOOL bEqual = TRUE;
    USHORT* pWh1 = _pWhichRanges;
    USHORT* pWh2 = rSet._pWhichRanges;
    USHORT nSize = 0;

    for( USHORT n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = FALSE;
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bEqual = *pWh1 == *pWh2;		// auch die 0 abpruefen

    // sind die Bereiche identisch, ist es einfacher zu handhaben !
    if( bEqual )
    {
        SfxItemArray ppFnd1 = _aItems;
        SfxItemArray ppFnd2 = rSet._aItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && !*ppFnd2 )
            {
                // aus dem Pool loeschen
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    USHORT nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = _pParent
                            ? _pParent->Get( nWhich, TRUE )
                            : _pPool->GetDefaultItem( nWhich );

                        Changed( **ppFnd1, rNew );
                    }
                    _pPool->Remove( **ppFnd1 );
                }
                *ppFnd1 = 0;
                --_nCount;
            }
    }
    else
    {
        SfxItemIter aIter( *this );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( TRUE )
        {
            USHORT nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( 0 == rSet.GetItemState( nWhich, FALSE ) )
                ClearItem( nWhich );		// loeschen
            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::Differentiate( const SfxItemSet& rSet )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if( !Count() || !rSet.Count() )  // gar keine gesetzt ?
        return;

    // teste mal, ob sich die Which-Bereiche unterscheiden.
    BOOL bEqual = TRUE;
    USHORT* pWh1 = _pWhichRanges;
    USHORT* pWh2 = rSet._pWhichRanges;
    USHORT nSize = 0;

    for( USHORT n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = FALSE;
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bEqual = *pWh1 == *pWh2;		// auch die 0 abpruefen

    // sind die Bereiche identisch, ist es einfacher zu handhaben !
    if( bEqual )
    {
        SfxItemArray ppFnd1 = _aItems;
        SfxItemArray ppFnd2 = rSet._aItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && *ppFnd2 )
            {
                // aus dem Pool loeschen
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    USHORT nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = _pParent
                            ? _pParent->Get( nWhich, TRUE )
                            : _pPool->GetDefaultItem( nWhich );

                        Changed( **ppFnd1, rNew );
                    }
                    _pPool->Remove( **ppFnd1 );
                }
                *ppFnd1 = 0;
                --_nCount;
            }
    }
    else
    {
        SfxItemIter aIter( *this );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( TRUE )
        {
            USHORT nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE ) )
                ClearItem( nWhich );		// loeschen
            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }

    }
}

// -----------------------------------------------------------------------
/* Entscheidungstabelle fuer MergeValue[s]

Grundsaetze:
    1. Ist der Which-Wert im 1.Set "unknown", dann folgt niemals eine Aktion.
    2. Ist der Which-Wert im 2.Set "unknown", dann gilt er als "default".
    3. Es gelten fuer Vergleiche die Werte der "default"-Items.

1.-Item     2.-Item     Values  bIgnoreDefs     Remove      Assign      Add

set         set         ==      FALSE           -           -           -
default     set         ==      FALSE           -           -           -
dontcare    set         ==      FALSE           -           -           -
unknown     set         ==      FALSE           -           -           -
set         default     ==      FALSE           -           -           -
default     default     ==      FALSE           -           -           -
dontcare    default     ==      FALSE           -           -           -
unknown     default     ==      FALSE           -           -           -
set         dontcare    ==      FALSE           1.-Item     -1          -
default 	dontcare	==		FALSE			-			-1			-
dontcare    dontcare    ==      FALSE           -           -           -
unknown     dontcare    ==      FALSE           -           -           -
set         unknown     ==      FALSE           1.-Item     -1          -
default     unknown     ==      FALSE           -           -           -
dontcare    unknown     ==      FALSE           -           -           -
unknown     unknown     ==      FALSE           -           -           -

set         set         !=      FALSE           1.-Item     -1          -
default     set         !=      FALSE           -           -1          -
dontcare    set         !=      FALSE           -           -           -
unknown     set         !=      FALSE           -           -           -
set         default     !=      FALSE           1.-Item     -1          -
default     default     !=      FALSE           -           -           -
dontcare    default     !=      FALSE           -           -           -
unknown     default     !=      FALSE           -           -           -
set         dontcare    !=      FALSE           1.-Item     -1          -
default     dontcare    !=      FALSE           -           -1          -
dontcare    dontcare    !=      FALSE           -           -           -
unknown     dontcare    !=      FALSE           -           -           -
set         unknown     !=      FALSE           1.-Item     -1          -
default     unknown     !=      FALSE           -           -           -
dontcare    unknown     !=      FALSE           -           -           -
unknown     unknown     !=      FALSE           -           -           -

set         set         ==      TRUE            -           -           -
default 	set 		==		TRUE			-			2.-Item 	2.-Item
dontcare    set         ==      TRUE            -           -           -
unknown     set         ==      TRUE            -           -           -
set         default     ==      TRUE            -           -           -
default     default     ==      TRUE            -           -           -
dontcare    default     ==      TRUE            -           -           -
unknown     default     ==      TRUE            -           -           -
set         dontcare    ==      TRUE            -           -           -
default 	dontcare	==		TRUE			-			-1			-
dontcare    dontcare    ==      TRUE            -           -           -
unknown     dontcare    ==      TRUE            -           -           -
set         unknown     ==      TRUE            -           -           -
default     unknown     ==      TRUE            -           -           -
dontcare    unknown     ==      TRUE            -           -           -
unknown     unknown     ==      TRUE            -           -           -

set         set         !=      TRUE            1.-Item     -1          -
default     set         !=      TRUE            -           2.-Item     2.-Item
dontcare	set 		!=		TRUE			-			-			-
unknown     set         !=      TRUE            -           -           -
set         default     !=      TRUE            -           -           -
default     default     !=      TRUE            -           -           -
dontcare    default     !=      TRUE            -           -           -
unknown     default     !=      TRUE            -           -           -
set         dontcare    !=      TRUE            1.-Item     -1          -
default     dontcare    !=      TRUE            -           -1          -
dontcare    dontcare    !=      TRUE            -           -           -
unknown     dontcare    !=      TRUE            -           -           -
set         unknown     !=      TRUE            -           -           -
default     unknown     !=      TRUE            -           -           -
dontcare    unknown     !=      TRUE            -           -           -
unknown     unknown     !=      TRUE            -           -           -
*/


static void MergeItem_Impl( SfxItemPool *_pPool, USHORT &rCount,
                            const SfxPoolItem **ppFnd1, const SfxPoolItem *pFnd2,
                            BOOL bIgnoreDefaults )
{
    DBG_ASSERT( ppFnd1 != 0, "Merging to 0-Item" );

    // 1. Item ist default?
    if ( !*ppFnd1 )
    {
        if ( IsInvalidItem(pFnd2) )
            // Entscheidungstabelle: default, dontcare, egal, egal
            *ppFnd1 = (SfxPoolItem*) -1;

        else if ( pFnd2 && !bIgnoreDefaults &&
                  _pPool->GetDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Entscheidungstabelle: default, set, !=, FALSE
            *ppFnd1 = (SfxPoolItem*) -1;

        else if ( pFnd2 && bIgnoreDefaults )
            // Entscheidungstabelle: default, set, egal, TRUE
            *ppFnd1 = &_pPool->Put( *pFnd2 );

        if ( *ppFnd1 )
            ++rCount;
    }

    // 1. Item ist gesetzt?
    else if ( !IsInvalidItem(*ppFnd1) )
    {
        if ( !pFnd2 )
        {
            // 2. Item ist default
            if ( !bIgnoreDefaults &&
                 **ppFnd1 != _pPool->GetDefaultItem((*ppFnd1)->Which()) )
            {
                // Entscheidungstabelle: set, default, !=, FALSE
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = (SfxPoolItem*) -1;
            }
        }
        else if ( IsInvalidItem(pFnd2) )
        {
            // 2. Item ist dontcare
            if ( !bIgnoreDefaults ||
                 **ppFnd1 != _pPool->GetDefaultItem( (*ppFnd1)->Which()) )
            {
                // Entscheidungstabelle: set, dontcare, egal, FALSE
                // oder:				 set, dontcare, !=, TRUE
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = (SfxPoolItem*) -1;
            }
        }
        else
        {
            // 2. Item ist gesetzt
            if ( **ppFnd1 != *pFnd2 )
            {
                // Entscheidungstabelle: set, set, !=, egal
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = (SfxPoolItem*) -1;
            }
        }
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::MergeValues( const SfxItemSet& rSet, BOOL bIgnoreDefaults )
{
    // Achtung!!! Bei Aenderungen/Bugfixes immer obenstehende Tabelle pflegen!
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( GetPool() == rSet.GetPool(), "MergeValues mit verschiedenen Pools" );

    // teste mal, ob sich die Which-Bereiche unterscheiden.
    BOOL bEqual = TRUE;
    USHORT* pWh1 = _pWhichRanges;
    USHORT* pWh2 = rSet._pWhichRanges;
    USHORT nSize = 0;

    for( USHORT n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = FALSE;
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bEqual = *pWh1 == *pWh2; // auch die 0 abpruefen

    // sind die Bereiche identisch, ist es effizieter zu handhaben !
    if( bEqual )
    {
        SfxItemArray ppFnd1 = _aItems;
        SfxItemArray ppFnd2 = rSet._aItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            MergeItem_Impl( _pPool, _nCount, ppFnd1, *ppFnd2, bIgnoreDefaults );
    }
    else
    {
        SfxWhichIter aIter( rSet );
        register USHORT nWhich;
        while( 0 != ( nWhich = aIter.NextWhich() ) )
        {
            const SfxPoolItem* pItem = 0;
            rSet.GetItemState( nWhich, TRUE, &pItem );
            if( !pItem )
            {
                // nicht gesetzt, also default
                if ( !bIgnoreDefaults )
                    MergeValue( rSet.GetPool()->GetDefaultItem( nWhich ), bIgnoreDefaults );
            }
            else if( IsInvalidItem( pItem ) )
                // dont care
                InvalidateItem( nWhich );
            else
                MergeValue( *pItem, bIgnoreDefaults );
        }
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::MergeValue( const SfxPoolItem& rAttr, BOOL bIgnoreDefaults )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const USHORT* pPtr = _pWhichRanges;
    const USHORT nWhich = rAttr.Which();
    while( *pPtr )
    {
        // in diesem Bereich?
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            ppFnd += nWhich - *pPtr;
            MergeItem_Impl( _pPool, _nCount, ppFnd, &rAttr, bIgnoreDefaults );
            break;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::InvalidateItem( USHORT nWhich )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const USHORT* pPtr = _pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // in diesem Bereich
            ppFnd += nWhich - *pPtr;

            if( *ppFnd )   	// bei mir gesetzt
            {
                if( (SfxPoolItem*)-1 != *ppFnd )		// noch nicht dontcare !
                {
                    _pPool->Remove( **ppFnd );
                    *ppFnd = (SfxPoolItem*)-1;
                }
            }
            else
            {
                *ppFnd = (SfxPoolItem*)-1;
                ++_nCount;
            }
            break;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
}

// -----------------------------------------------------------------------

USHORT SfxItemSet::GetWhichByPos( USHORT nPos ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    USHORT n = 0;
    USHORT* pPtr  = _pWhichRanges;
    while( *pPtr )
    {
        n = ( *(pPtr+1) - *pPtr ) + 1;
        if( nPos < n )
            return *(pPtr)+nPos;
        nPos = nPos - n;
        pPtr += 2;
    }
    DBG_ASSERT( FALSE, "Hier sind wir falsch" );
    return 0;
}

// -----------------------------------------------------------------------

SvStream &SfxItemSet::Store
(
    SvStream&	rStream,		// Zielstream f"ur normale Items
    FASTBOOL	bDirect 		// TRUE: Items direkt speicher, FALSE: Surrogate
)	const

/*	[Beschreibung]

    Speichert die <SfxItemSet>-Instanz in den angegebenen Stream. Dabei
    werden die Surrorage der gesetzten <SfxPoolItem>s bzw. ('bDirect==TRUE')
    die gesetzten Items selbst wie folgt im Stream abgelegt:

            USHORT				(Count) Anzahl der gesetzten Items
    Count*	_pPool->StoreItem()  siehe <SfxItemPool::StoreItem()const>


    [Querverweise]

    <SfxItemSet::Load(SvStream&,BOOL,const SfxItemPool*)>
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( _pPool, "Kein Pool" );
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );

    // Position des Counts merken, um ggf. zu korrigieren
    ULONG nCountPos = rStream.Tell();
    rStream << _nCount;

    // wenn nichts zu speichern ist, auch keinen ItemIter aufsetzen!
    if ( _nCount )
    {
        // mitz"ahlen wieviel Items tats"achlich gespeichert werden
        USHORT nWrittenCount = 0;  // Anzahl in 'rStream' gestreamter Items

        // "uber alle gesetzten Items iterieren
        SfxItemIter aIter(*this);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            // Item (ggf. als Surrogat) via Pool speichern lassen
            DBG_ASSERT( !IsInvalidItem(pItem), "can't store invalid items" );
            if ( !IsInvalidItem(pItem) &&
                 _pPool->StoreItem( rStream, *pItem, bDirect ) )
                // Item wurde in 'rStream' gestreamt
                ++nWrittenCount;
        };

        // weniger geschrieben als enthalten (z.B. altes Format)
        if ( nWrittenCount != _nCount )
        {
            // tats"achlichen Count im Stream ablegen
            ULONG nPos = rStream.Tell();
            rStream.Seek( nCountPos );
            rStream << nWrittenCount;
            rStream.Seek( nPos );
        }
    }

    return rStream;
}

// -----------------------------------------------------------------------

SvStream &SfxItemSet::Load
(
    SvStream&			rStream,	//	Stream, aus dem geladen werden soll

    FASTBOOL			bDirect,	/*	TRUE
                                        Items werden direkt aus dem Stream
                                        gelesen, nicht "uber Surrogate

                                        FALSE (default)
                                        Items werden "uber Surrogate gelesen */

    const SfxItemPool*	pRefPool 	/*	Pool, der die Surrogate aufl"osen kann
                                        (z.B. zum Einf"ugen von Dokumenten) */
)

/*	[Beschreibung]

    Diese Methode l"adt ein <SfxItemSet> aus einem Stream. Falls der
    <SfxItemPool> ohne Ref-Counts geladen wurde, werden die geladenen
    Item-Referenzen in den Items hochgez"ahlt, ansonsten wird vorausgesetzt,
    da\s sie schon beim Laden des SfxItemPools ber"ucksichtigt waren.

    [Querverweise]

    <SfxItemSet::Store(Stream&,BOOL)const>
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( _pPool, "Kein Pool");
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "Kein Master-Pool");

    // kein Ref-Pool => Surrogate mit Pool des ItemSets aufl"osen
    if ( !pRefPool )
        pRefPool = _pPool;

    // Anzahl der zu ladenden Items laden und dann ebensoviele Items
    USHORT nCount = 0;
    rStream >> nCount;
    for ( USHORT i = 0; i < nCount; ++i )
    {
        // Surrogat/Item laden und (Surrogat) aufl"osen lassen
        const SfxPoolItem *pItem =
                _pPool->LoadItem( rStream, bDirect, pRefPool );

        // konnte ein Item geladen oder via Surrogat aufgel"ost werden?
        if ( pItem )
        {
            // Position f"ur Item-Pointer im Set suchen
            USHORT nWhich = pItem->Which();
            SfxItemArray ppFnd = _aItems;
            const USHORT* pPtr = _pWhichRanges;
            while ( *pPtr )
            {
                // in diesem Bereich?
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // Item-Pointer im Set merken
                    ppFnd += nWhich - *pPtr;
                    SFX_ASSERT( !*ppFnd, nWhich, "Item doppelt eingetragen");
                    *ppFnd = pItem;
                    ++_nCount;
                    break;
                }

                // im Range-Array und Item-Array zum n"achsten Which-Range
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
    }

    return rStream;
}

// -----------------------------------------------------------------------

int	SfxItemSet::operator==(const SfxItemSet &rCmp) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_CHKOBJ(&rCmp, SfxItemSet, DbgCheckItemSet);

    // besonders schnell zu ermittelnde Werte muessen gleich sein
    if ( _pParent != rCmp._pParent ||
         _pPool != rCmp._pPool ||
         Count() != rCmp.Count() )
        return FALSE;

    // Ranges durchzaehlen lassen dauert laenger, muss aber auch gleich sein
    USHORT nCount1 = TotalCount();
    USHORT nCount2 = rCmp.TotalCount();
    if ( nCount1 != nCount2 )
        return FALSE;

    // sind die Ranges selbst ungleich?
    for ( USHORT nRange = 0; _pWhichRanges[nRange]; nRange += 2 )
        if ( _pWhichRanges[nRange] != rCmp._pWhichRanges[nRange] ||
             _pWhichRanges[nRange+1] != rCmp._pWhichRanges[nRange+1] )
        {
            // dann m"ussen wir die langsame Methode verwenden
            SfxWhichIter aIter( *this );
            for ( USHORT nWh = aIter.FirstWhich();
                  nWh;
                  nWh = aIter.NextWhich() )
            {
                // wenn die Pointer von poolable Items ungleich sind,
                // muessen die Items gleich sein
                const SfxPoolItem *pItem1 = 0, *pItem2 = 0;
                if ( GetItemState( nWh, FALSE, &pItem1 ) !=
                        rCmp.GetItemState( nWh, FALSE, &pItem2 ) ||
                     ( pItem1 != pItem2 &&
                        ( !pItem1 || IsInvalidItem(pItem1) ||
                          ( _pPool->IsItemFlag(*pItem1, SFX_ITEM_POOLABLE) &&
                            *pItem1 != *pItem2 ) ) ) )
                    return FALSE;
            }

            return TRUE;
        }

    // Pointer alle gleich?
    if ( 0 == memcmp( _aItems, rCmp._aItems, nCount1 * sizeof(_aItems[0]) ) )
        return TRUE;

    // dann werden wir wohl alle einzeln vergleichen muessen
    const SfxPoolItem **ppItem1 = (const SfxPoolItem**) _aItems;
    const SfxPoolItem **ppItem2 = (const SfxPoolItem**) rCmp._aItems;
    for ( USHORT nPos = 0; nPos < nCount1; ++nPos )
    {
        // wenn die Pointer von poolable Items ungleich sind,
        // muessen die Items gleich sein
        if ( *ppItem1 != *ppItem2 &&
             ( ( !*ppItem1 || !*ppItem2 ) ||
               ( IsInvalidItem(*ppItem1) || IsInvalidItem(*ppItem2) ) ||
               ( _pPool->IsItemFlag(**ppItem1, SFX_ITEM_POOLABLE) ) ||
                 **ppItem1 != **ppItem2 ) )
            return FALSE;

        ++ppItem1;
        ++ppItem2;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

SfxItemSet *SfxItemSet::Clone(BOOL bItems, SfxItemPool *pToPool ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if ( pToPool && pToPool != _pPool )
    {
        SfxItemSet *pNewSet = new SfxItemSet( *pToPool, _pWhichRanges );
        if ( bItems )
        {
            SfxWhichIter aIter(*pNewSet);
            USHORT nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SFX_ITEM_SET == GetItemState( nWhich, FALSE, &pItem ) )
                    pNewSet->Put( *pItem, pItem->Which() );
                nWhich = aIter.NextWhich();
            }
        }
        return pNewSet;
    }
    else
        return bItems
                ? new SfxItemSet(*this)
                : new SfxItemSet(*_pPool, _pWhichRanges);
}

// -----------------------------------------------------------------------

int	SfxItemSet::PutDirect(const SfxPoolItem &rItem)
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const USHORT* pPtr = _pWhichRanges;
    const USHORT nWhich = rItem.Which();
#ifdef DBG_UTIL
    IsPoolDefaultItem(&rItem) || _pPool->GetSurrogate(&rItem);
        // nur Assertion in den callees provozieren
#endif
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // in diesem Bereich
            ppFnd += nWhich - *pPtr;
            const SfxPoolItem* pOld = *ppFnd;
            if( pOld )		// schon einer vorhanden
            {
                if( rItem == **ppFnd )
                    return FALSE;		// schon vorhanden !
                _pPool->Remove( *pOld );
            }
            else
                ++_nCount;

            // den neuen eintragen
            if( IsPoolDefaultItem(&rItem) )
                *ppFnd = &_pPool->Put( rItem );
            else
            {
                *ppFnd = &rItem;
                if( !IsStaticDefaultItem( &rItem ) )
                    rItem.AddRef();
            }

            return TRUE;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return FALSE;
}

// -----------------------------------------------------------------------

SfxAllItemSet::SfxAllItemSet( SfxItemPool &rPool )
:	SfxItemSet(rPool, (const USHORT*) 0),
    aDefault(0),
    nFree(nInitCount)
{
    // initial keine Items
    _aItems = 0;

    // nInitCount Paare an USHORTs fuer Ranges allozieren
    _pWhichRanges = new USHORT[ nInitCount + 1 ];
    memset( _pWhichRanges, 0, ( nInitCount + 1 ) * sizeof(USHORT) );
}


// -----------------------------------------------------------------------


SfxAllItemSet::SfxAllItemSet(const SfxItemSet &rCopy)
:   SfxItemSet(rCopy),
    aDefault(0),
    nFree(0)
{
}

// -----------------------------------------------------------------------



SfxAllItemSet::SfxAllItemSet(const SfxAllItemSet &rCopy)
:   SfxItemSet(rCopy),
    aDefault(0),
    nFree(0)
/*	[Anmerkung]

    Der mu\s sein, da sonst vom Compiler einer generiert wird, er nimmt
    nicht den Ctor mit der 'const SfxItemSet&'!
*/
{
}

// -----------------------------------------------------------------------

static USHORT *AddRanges_Impl(
    USHORT *pUS, std::ptrdiff_t nOldSize, USHORT nIncr)

/* 	Diese interne Funktion erzeugt ein neues Which-Range-Array, welches von
    dem 'nOldSize'-USHORTs langen 'pUS' kopiert wird und hinten an Platz
    f"ur 'nIncr' neue USHORTs hat. Das terminierende USHORT mit der '0'
    wird weder in 'nOldSize' noch in 'nIncr' mitgez"ahlt, sondern implizit
    hinzugerechnet.

    Das neue Which-Range-Array wird als Returnwert zur"uckgegeben, das alte
    'pUS' freigegeben.
*/

{
    // neues Which-Range-Array anlegen
    USHORT *pNew = new USHORT[ nOldSize + nIncr + 1 ];

    // die alten Ranges "ubernehmen
    memcpy( pNew, pUS, nOldSize * sizeof(USHORT) );

    // die neuen auf 0 initialisieren
    memset( pNew + nOldSize, 0, ( nIncr + 1 ) * sizeof(USHORT) );

    // das alte Array freigeben
    delete[] pUS;

    return pNew;
}

// -----------------------------------------------------------------------

static SfxItemArray AddItem_Impl(SfxItemArray pItems, USHORT nOldSize, USHORT nPos)

/*  Diese interne Funktion erzeugt ein neues ItemArray, welches von 'pItems'
    kopiert wird, an der Position 'nPos' jedoch Platz f"ur einen neuen
    ItemPointer hat.

    Das neue ItemArray wird als Returnwert zur"uckgegeben, das alte 'pItems'
    wird freigegeben.
*/

{
    // neues ItemArray anlegen
    SfxItemArray pNew = new const SfxPoolItem*[nOldSize+1];

    // war schon vorher eins da?
    if ( pItems )
    {
        // alte Items vor nPos kopieren
        if ( nPos )
            memcpy( (void*) pNew, pItems, nPos * sizeof(SfxPoolItem **) );

        // alte Items hinter nPos kopieren
        if ( nPos < nOldSize )
            memcpy( (void*) (pNew + nPos + 1), pItems + nPos,
                    (nOldSize-nPos) * sizeof(SfxPoolItem **) );
    }

    // neues Item initialisieren
    *(pNew + nPos) = 0;

    // altes ItemArray freigeben
    delete[] pItems;

    return pNew;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxAllItemSet::Put( const SfxPoolItem& rItem, USHORT nWhich )

// Putten mit automatischer Erweiterung der Whichs-Ids um die ID
// des Items.

{
    USHORT nPos = 0; // Position f"ur 'rItem' in '_aItems'
    const USHORT nItemCount = TotalCount();

    // erstmal sehen, ob es schon einen passenden Bereich gibt
    USHORT *pPtr = _pWhichRanges;
    while ( *pPtr )
    {
        // Which-Id liegt in diesem Bereich?
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // Einfuegen
            nPos += nWhich - *pPtr;
            break;
        }

        // Position des Items in _aItems mitf"uhren
        nPos += *(pPtr+1) - *pPtr + 1;

        // zum n"achsten Bereich
        pPtr += 2;
    }

    // Which-Id noch nicht vorhanden?
    if ( !*pPtr )
    {
        // suchen, ob man sie irgendwo dranpacken kann
        pPtr = _pWhichRanges;
        nPos = 0;
        while ( *pPtr )
        {
            // Which-Id liegt exakt vor diesem Bereich?
            if ( (nWhich+1) == *pPtr )
            {
                // Bereich waechst nach unten
                (*pPtr)--;

                // vor erstem Item dieses Bereichs Platz schaffen
                _aItems = AddItem_Impl(_aItems, nItemCount, nPos);
                break;
            }

            // Which-Id liegt exakt hinter diesem Bereich?
            else if ( (nWhich-1) == *(pPtr+1) )
            {
                // Bereich waechst nach oben
                (*(pPtr+1))++;

                // hinter letztem Item dieses Bereichs Platz schaffen
                nPos += nWhich - *pPtr;
                _aItems = AddItem_Impl(_aItems, nItemCount, nPos);
                break;
            }

            // Position des Items in _aItems mitf"uhren
            nPos += *(pPtr+1) - *pPtr + 1;

            // zum n"achsten Bereich
            pPtr += 2;
        }
    }

    // keinen erweiterbaren Bereich gefunden?
    if ( !*pPtr )
    {
        // kein Platz mehr in _pWhichRanges => erweitern
        std::ptrdiff_t nSize = pPtr - _pWhichRanges;
        if( !nFree )
        {
            _pWhichRanges = AddRanges_Impl(_pWhichRanges, nSize, nInitCount);
            nFree += nInitCount;
        }

        // neuen Which-Range anh"angen
        pPtr = _pWhichRanges + nSize;
        *pPtr++ = nWhich;
        *pPtr = nWhich;
        nFree -= 2;

        // Itemarray vergroessern
        nPos = nItemCount;
        _aItems = AddItem_Impl(_aItems, nItemCount, nPos);
    }

    // neues Item in Pool aufnehmen
    const SfxPoolItem& rNew = _pPool->Put( rItem, nWhich );

    // altes Item merken
    BOOL bIncrementCount = FALSE;
    const SfxPoolItem* pOld = *( _aItems + nPos );
    if ( reinterpret_cast< SfxPoolItem* >( -1 ) == pOld )   // state "dontcare"
        pOld = NULL;
    if ( !pOld )
    {
        bIncrementCount = TRUE;
        pOld = _pParent ?
                &_pParent->Get( nWhich, TRUE )
                : nWhich <= SFX_WHICH_MAX ? &_pPool->GetDefaultItem( nWhich ) : 0;
    }

    // neue Item in ItemSet aufnehmen
    *(_aItems + nPos) = &rNew;

    // Changed Notification versenden
    if ( pOld )
    {
        Changed( *pOld, rNew );
        if ( !IsDefaultItem(pOld) )
            _pPool->Remove( *pOld );
    }

    if ( bIncrementCount )
        ++_nCount;

    return &rNew;
}

// -----------------------------------------------------------------------


/*	Diese Methode wird forwarded, damit sie nicht durch die anderen
    Put-Methoden dieser SubClass gehided wird.
*/

int SfxAllItemSet::Put( const SfxItemSet& rSet, BOOL bInvalidAsDefault )
{
    //? pruefen, ob Which-Ranges erweitert werden
    return SfxItemSet::Put( rSet, bInvalidAsDefault );
}

// -----------------------------------------------------------------------
// Item disablen, wenn durch ein VoidItem mit dem Which-Wert 0 ausgedrueckt

void SfxItemSet::DisableItem(USHORT nWhich)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    Put( SfxVoidItem(0), nWhich );
}

// -----------------------------------------------------------------------

#if 0
BOOL SfxAllItemSet::Remove(USHORT nWhich)
{
    DBG_CHKTHIS(SfxAllItemSet, 0);
    USHORT *pPtr = _pWhichRanges;
    USHORT nPos = 0;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            USHORT *pTmp = pPtr;
            USHORT nLeft = 0;
            USHORT nRest = 0;
            while(*++pTmp){
                if( nLeft & 1 )
                    nRest = *pTmp - *(pTmp-1) + 1;
                ++nLeft;
            }

            // in diesem Bereich
            nPos += nWhich - *pPtr;
            nRest -= nWhich - *pPtr;
            // 3,3
            if(*pPtr == nWhich && *(pPtr+1) == nWhich) {
                memmove(pPtr, pPtr + 2, nLeft * sizeof(USHORT));
                nFree += 2;
            }
                // Anfang
            else if(*pPtr == nWhich)
                (*pPtr)++;
                // Ende
            else if(*(pPtr+1) == nWhich)
                (*(pPtr+1))--;
            else {
                if(nPos + nRest + 2 > nFree) {
                    USHORT nOf = pPtr - _pWhichRanges;
                    _pWhichRanges = IncrSize(_pWhichRanges, nPos + nRest, nInitCount);
                    nFree += nInitCount;
                    pPtr = _pWhichRanges + nOf;
                }
                memmove(pPtr +2, pPtr, (nLeft+2) * sizeof(USHORT));
                *++pPtr  = nWhich-1;
                *++pPtr = nWhich+1;
                nFree -= 2;
            }
            SfxPoolItem* pItem = *( _aItems + nPos );
            if( pItem )
            {
                if(_pPool)
                    _pPool->Remove(*pItem );
                else
                    delete pItem;
                --_nCount;
            }
            memmove(_aItems + nPos +1, _aItems + nPos,
                    sizeof(SfxPoolItem *) * (nRest - 1));
            break; 			// dann beim Parent suchen
        }
        nPos += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return *pPtr? TRUE: FALSE;
}
#endif

// -----------------------------------------------------------------------

SfxItemSet *SfxAllItemSet::Clone(BOOL bItems, SfxItemPool *pToPool ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if ( pToPool && pToPool != _pPool )
    {
        SfxAllItemSet *pNewSet = new SfxAllItemSet( *pToPool );
        if ( bItems )
            pNewSet->Set( *this );
        return pNewSet;
    }
    else
        return bItems ? new SfxAllItemSet(*this) : new SfxAllItemSet(*_pPool);
}

}
