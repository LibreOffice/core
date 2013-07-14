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


#include <string.h>

#include <cstdarg>

#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/nranges.hxx>
#include "whassert.hxx"

#include <tools/stream.hxx>
#include <tools/solar.h>

// STATIC DATA -----------------------------------------------------------

static const sal_uInt16 nInitCount = 10; // einzelne USHORTs => 5 Paare ohne '0'
#if OSL_DEBUG_LEVEL > 1
static sal_uLong nRangesCopyCount = 0;   // wie oft wurden Ranges kopiert
#endif

DBG_NAME(SfxItemSet)


#include "nranges.cxx"


#ifdef DBG_UTIL


const sal_Char *DbgCheckItemSet( const void* pVoid )
{
    const SfxItemSet *pSet = (const SfxItemSet*) pVoid;
    SfxWhichIter aIter( *pSet );
    sal_uInt16 nCount = 0, n = 0;
    for ( sal_uInt16 nWh = aIter.FirstWhich(); nWh; nWh = aIter.NextWhich(), ++n )
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
    SfxItemPool&    rPool,          /* der Pool, in dem die SfxPoolItems,
                                       welche in dieses SfxItemSet gelangen,
                                       aufgenommen werden sollen */
    sal_Bool        bTotalRanges    /* komplette Pool-Ranges uebernehmen,
                                       muss auf sal_True gesetzt werden */
)
/*  [Beschreibung]

    Konstruktor fuer ein SfxItemSet mit genau den Which-Bereichen, welche
    dem angegebenen <SfxItemPool> bekannt sind.


    [Anmerkung]

    F"ur Sfx-Programmierer ein derart konstruiertes SfxItemSet kann
    keinerlei Items mit Slot-Ids als Which-Werte aufnehmen!
*/

:   _pPool( &rPool ),
    _pParent( 0 ),
    _nCount( 0 )
{
    DBG_CTOR(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );
    DBG( _pChildCountCtor; *_pChildCount(this) = 0 );
//  DBG_ASSERT( bTotalRanges || abs( &bTotalRanges - this ) < 1000,
//              "please use suitable ranges" );
#if defined DBG_UTIL && defined SFX_ITEMSET_NO_DEFAULT_CTOR
    if ( !bTotalRanges )
        *(int*)0 = 0; // GPF
#else
    (void) bTotalRanges; // avoid warnings
#endif

    _pWhichRanges = (sal_uInt16*) _pPool->GetFrozenIdRanges();
    DBG_ASSERT( _pWhichRanges, "don't create ItemSets with full range before FreezeIdRanges()" );
    if ( !_pWhichRanges )
        _pPool->FillItemIdRanges_Impl( _pWhichRanges );

    const sal_uInt16 nSize = TotalCount();
    _aItems = new const SfxPoolItem* [ nSize ];
    memset( (void*) _aItems, 0, nSize * sizeof( SfxPoolItem* ) );
}

// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 ):
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

void SfxItemSet::InitRanges_Impl(sal_uInt16 nWh1, sal_uInt16 nWh2)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    _pWhichRanges = new sal_uInt16[ 3 ];
    *(_pWhichRanges+0) = nWh1;
    *(_pWhichRanges+1) = nWh2;
    *(_pWhichRanges+2) = 0;
    const sal_uInt16 nRg = nWh2 - nWh1 + 1;
    _aItems = new const SfxPoolItem* [ nRg ];
    memset( (void*) _aItems, 0, nRg * sizeof( SfxPoolItem* ) );
}

// -----------------------------------------------------------------------

void SfxItemSet::InitRanges_Impl(va_list pArgs, sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull)
{
    DBG_CHKTHIS(SfxItemSet, 0);

    sal_uInt16 nSize = InitializeRanges_Impl( _pWhichRanges, pArgs, nWh1, nWh2, nNull );
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
            sal::static_int_cast< sal_uInt16 >(nWh1),
            sal::static_int_cast< sal_uInt16 >(nWh2));
    else {
        va_list pArgs;
        va_start( pArgs, nNull );
        InitRanges_Impl(
            pArgs, sal::static_int_cast< sal_uInt16 >(nWh1),
            sal::static_int_cast< sal_uInt16 >(nWh2),
            sal::static_int_cast< sal_uInt16 >(nNull));
        va_end(pArgs);
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::InitRanges_Impl(const sal_uInt16 *pWhichPairTable)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    #if OSL_DEBUG_LEVEL > 1
    OSL_TRACE("SfxItemSet: Ranges-CopyCount==%ul", ++nRangesCopyCount);
    #endif

    sal_uInt16 nCnt = 0;
    const sal_uInt16* pPtr = pWhichPairTable;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    _aItems = new const SfxPoolItem* [ nCnt ];
    memset( (void*) _aItems, 0, sizeof( SfxPoolItem* ) * nCnt );

    std::ptrdiff_t cnt = pPtr - pWhichPairTable +1;
    _pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( _pWhichRanges, pWhichPairTable, sizeof( sal_uInt16 ) * cnt );
}


// -----------------------------------------------------------------------

SfxItemSet::SfxItemSet( SfxItemPool& rPool, const sal_uInt16* pWhichPairTable ):
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
    sal_uInt16 nCnt = 0;
    sal_uInt16* pPtr = rASet._pWhichRanges;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    _aItems = new const SfxPoolItem* [ nCnt ];

    // Attribute kopieren
    SfxItemArray ppDst = _aItems, ppSrc = rASet._aItems;
    for( sal_uInt16 n = nCnt; n; --n, ++ppDst, ++ppSrc )
        if ( 0 == *ppSrc ||                 // aktueller Default?
             IsInvalidItem(*ppSrc) ||       // Dont Care?
             IsStaticDefaultItem(*ppSrc) )  // nicht zu poolende Defaults
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
    #if OSL_DEBUG_LEVEL > 1
    OSL_TRACE("SfxItemSet: Ranges-CopyCount==%ul", ++nRangesCopyCount);
    #endif
    std::ptrdiff_t cnt = pPtr - rASet._pWhichRanges+1;
    _pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( _pWhichRanges, rASet._pWhichRanges, sizeof( sal_uInt16 ) * cnt);
}

// -----------------------------------------------------------------------

SfxItemSet::~SfxItemSet()
{
    DBG_DTOR(SfxItemSet, DbgCheckItemSet);
#ifdef DBG_UTIL
    DBG( DBG_ASSERT( 0 == *_pChildCount(this), "SfxItemSet: deleting parent-itemset" ) )
#endif

    sal_uInt16 nCount = TotalCount();
    if( Count() )
    {
        SfxItemArray ppFnd = _aItems;
        for( sal_uInt16 nCnt = nCount; nCnt; --nCnt, ++ppFnd )
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

sal_uInt16 SfxItemSet::ClearItem( sal_uInt16 nWhich )

// einzelnes Item oder alle Items (nWhich==0) l"oschen

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if( !Count() )
        return 0;

    sal_uInt16 nDel = 0;
    SfxItemArray ppFnd = _aItems;

    if( nWhich )
    {
        const sal_uInt16* pPtr = _pWhichRanges;
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
                                    ? _pParent->Get( nWhich, sal_True )
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

        sal_uInt16* pPtr = _pWhichRanges;
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
                                    ? _pParent->Get( nWhich, sal_True )
                                    : _pPool->GetDefaultItem( nWhich );

                            Changed( *pItemToClear, rNew );
                        }

                        // #i32448#
                        // Take care of disabled items, too.
                        if (!pItemToClear->m_nWhich)
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

void SfxItemSet::ClearInvalidItems( sal_Bool bHardDefault )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    sal_uInt16* pPtr = _pWhichRanges;
    SfxItemArray ppFnd = _aItems;
    if ( bHardDefault )
        while( *pPtr )
        {
            for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if ( IsInvalidItem(*ppFnd) )
                     *ppFnd = &_pPool->Put( _pPool->GetDefaultItem(nWhich) );
            pPtr += 2;
        }
    else
        while( *pPtr )
        {
            for( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( IsInvalidItem(*ppFnd) )
                {
                    *ppFnd = 0;
                    --_nCount;
                }
            pPtr += 2;
        }
}



void SfxItemSet::InvalidateAllItems()
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( !_nCount, "Es sind noch Items gesetzt" );

    memset( (void*)_aItems, -1, ( _nCount = TotalCount() ) * sizeof( SfxPoolItem*) );
}

// -----------------------------------------------------------------------

SfxItemState SfxItemSet::GetItemState( sal_uInt16 nWhich,
                                        sal_Bool bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    // suche den Bereich in dem das Which steht:
    const SfxItemSet* pAktSet = this;
    SfxItemState eRet = SFX_ITEM_UNKNOWN;
    do
    {
        SfxItemArray ppFnd = pAktSet->_aItems;
        const sal_uInt16* pPtr = pAktSet->_pWhichRanges;
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

                    if ( (*ppFnd)->Type() == TYPE(SfxVoidItem) )
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

bool SfxItemSet::HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem) const
{
    bool bRet = SFX_ITEM_SET == GetItemState(nWhich, true, ppItem);
    if (!bRet && ppItem)
        *ppItem = NULL;
    return bRet;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
            0 != &((const SfxSetItem&)rItem).GetItemSet(),
            "SetItem without ItemSet" );
    if ( !nWhich )
        return 0; //! nur wegen Outliner-Bug
    SfxItemArray ppFnd = _aItems;
    const sal_uInt16* pPtr = _pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // in diesem Bereich
            ppFnd += nWhich - *pPtr;
            if( *ppFnd )        // schon einer vorhanden
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
                            ? _pParent->Get( nWhich, sal_True )
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

int SfxItemSet::Put( const SfxItemSet& rSet, sal_Bool bInvalidAsDefault )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    sal_Bool bRet = sal_False;
    if( rSet.Count() )
    {
        SfxItemArray ppFnd = rSet._aItems;
        const sal_uInt16* pPtr = rSet._pWhichRanges;
        while ( *pPtr )
        {
            for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
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
    const SfxItemSet&   rSet,           // Quelle der zu puttenden Items
    SfxItemState        eDontCareAs,    // was mit DontCare-Items passiert
    SfxItemState        eDefaultAs      // was mit Default-Items passiert
)

/*  [Beschreibung]

    Diese Methode "ubernimmt die Items aus 'rSet' in '*this'. Die
    Which-Bereiche in '*this', die in 'rSet' nicht vorkommen bleiben unver-
    "andert. Der Which-Bereich von '*this' bleibt auch unver"andert.

    In 'rSet' gesetzte Items werden auch in '*this*' gesetzt. Default-
    (0 Pointer) und Invalid- (-1 Pointer) Items werden je nach Parameter
    ('eDontCareAs' und 'eDefaultAs' behandelt:

    SFX_ITEM_SET:       hart auf Default des Pools gesetzt
    SFX_ITEM_DEFAULT:   gel"oscht (0 Pointer)
    SFX_ITEM_DONTCARE:  invalidiert (-1 Pointer)

    Alle anderen Werte f"ur 'eDontCareAs' und 'eDefaultAs' sind ung"ultig.
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);

    // don't "optimize" with "if( rSet.Count()" because of dont-care + defaults
    SfxItemArray ppFnd = rSet._aItems;
    const sal_uInt16* pPtr = rSet._pWhichRanges;
    while ( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
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
                            OSL_FAIL( "invalid Argument for eDontCareAs" );
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
                        OSL_FAIL( "invalid Argument for eDefaultAs" );
                }
            }
        pPtr += 2;
    }
}

// -----------------------------------------------------------------------

void SfxItemSet::MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo )
/** <H3>Description</H3>

    Expands the ranges of settable items by 'nFrom' to 'nTo'. Keeps state of
    items which are new ranges too.
*/

{
    // special case: exactly one sal_uInt16 which is already included?
    if ( nFrom == nTo && SFX_ITEM_AVAILABLE <= GetItemState(nFrom, sal_False) )
        return;

    // merge new range
    SfxUShortRanges aRanges( _pWhichRanges );
    aRanges += SfxUShortRanges( nFrom, nTo );
    SetRanges( aRanges );
}

// -----------------------------------------------------------------------

void SfxItemSet::SetRanges( const sal_uInt16 *pNewRanges )

/** <H3>Description</H3>

    Modifies the ranges of settable items. Keeps state of items which
    are new ranges too.
*/

{
    // identische Ranges?
    if ( _pWhichRanges == pNewRanges )
        return;
    const sal_uInt16* pOld = _pWhichRanges;
    const sal_uInt16* pNew = pNewRanges;
    while ( *pOld == *pNew )
    {
        if ( !*pOld && !*pNew )
            return;
        ++pOld, ++pNew;
    }

    // create new item-array (by iterating through all new ranges)
    sal_uLong        nSize = Capacity_Impl(pNewRanges);
    SfxItemArray aNewItems = new const SfxPoolItem* [ nSize ];
    sal_uInt16 nNewCount = 0;
    if ( _nCount == 0 )
        memset( aNewItems, 0, nSize * sizeof( SfxPoolItem* ) );
    else
    {
        sal_uInt16 n = 0;
        for ( const sal_uInt16 *pRange = pNewRanges; *pRange; pRange += 2 )
        {
            // iterate through all ids in the range
            for ( sal_uInt16 nWID = *pRange; nWID <= pRange[1]; ++nWID, ++n )
            {
                // direct move of pointer (not via pool)
                SfxItemState eState = GetItemState( nWID, sal_False, aNewItems+n );
                if ( SFX_ITEM_SET == eState )
                {
                    // increment new item count and possibly increment ref count
                    ++nNewCount;
                    aNewItems[n]->AddRef();
                }
                else if ( SFX_ITEM_DISABLED == eState )
                {
                    // put "disabled" item
                    ++nNewCount;
                    aNewItems[n] = new SfxVoidItem(0);
                }
                else if ( SFX_ITEM_DONTCARE == eState )
                {
                    ++nNewCount;
                    aNewItems[n] = (SfxPoolItem*)-1;
                }
                else
                {
                    // default
                    aNewItems[n] = 0;
                }
            }
        }
        // free old items
        sal_uInt16 nOldTotalCount = TotalCount();
        for ( sal_uInt16 nItem = 0; nItem < nOldTotalCount; ++nItem )
        {
            const SfxPoolItem *pItem = _aItems[nItem];
            if ( pItem && !IsInvalidItem(pItem) && pItem->Which() )
                _pPool->Remove(*pItem);
        }
    }

    // replace old items-array and ranges
    delete[] _aItems;
    _aItems = aNewItems;
    _nCount = nNewCount;

    if( pNewRanges == GetPool()->GetFrozenIdRanges() )
    {
        delete[] _pWhichRanges;
        _pWhichRanges = ( sal_uInt16* ) pNewRanges;
    }
    else
    {
        sal_uInt16 nCount = Count_Impl(pNewRanges) + 1;
        if ( _pWhichRanges != _pPool->GetFrozenIdRanges() )
            delete[] _pWhichRanges;
        _pWhichRanges = new sal_uInt16[ nCount ];
        memcpy( _pWhichRanges, pNewRanges, sizeof( sal_uInt16 ) * nCount );
    }
}

// -----------------------------------------------------------------------

int SfxItemSet::Set
(
    const SfxItemSet&   rSet,   /*  das SfxItemSet, dessen SfxPoolItems
                                    "ubernommen werden sollen */

    sal_Bool                bDeep   /*  sal_True (default)
                                    auch die SfxPoolItems aus den ggf. an
                                    rSet vorhandenen Parents werden direkt
                                    in das SfxItemSet "ubernommen

                                    sal_False
                                    die SfxPoolItems aus den Parents von
                                    rSet werden nicht ber"ucksichtigt */
)

/*  [Beschreibung]

    Das SfxItemSet nimmt genau die SfxPoolItems an, die auch in
    rSet gesetzt sind und im eigenen <Which-Bereich> liegen. Alle
    anderen werden entfernt. Der SfxItemPool wird dabei beibehalten,
    so da"s die "ubernommenen SfxPoolItems dabei ggf. vom SfxItemPool
    von rSet in den SfxItemPool von *this "ubernommen werden.

    SfxPoolItems, f"ur die in rSet IsInvalidItem() == sal_True gilt,
    werden als Invalid-Item "ubernommen.


    [R"uckgabewert]

    int                             sal_True
                                    es wurden SfxPoolItems "ubernommen

                                    sal_False
                                    es wurden keine SfxPoolItems "ubernommen,
                                    da z.B. die Which-Bereiche der SfxItemSets
                                    keine Schnittmenge haben oder in der
                                    Schnittmenge keine SfxPoolItems in rSet
                                    gesetzt sind

*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    int bRet = sal_False;
    if ( _nCount )
        ClearItem();
    if ( bDeep )
    {
        SfxWhichIter aIter(*this);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_True, &pItem ) )
                bRet |= 0 != Put( *pItem, pItem->Which() );
            nWhich = aIter.NextWhich();
        }
    }
    else
        bRet = Put(rSet, sal_False);

    return bRet;
}


const SfxPoolItem* SfxItemSet::GetItem
(
    sal_uInt16              nId,            // Slot-Id oder Which-Id des Items
    sal_Bool                bSrchInParent,  // sal_True: auch in Parent-ItemSets suchen
    TypeId              aItemType       // != 0 =>  RTTI Pruefung mit Assertion
)   const

/*  [Beschreibung]

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
    sal_uInt16 nWhich = GetPool()->GetWhich(nId);

    // ist das Item gesetzt oder bei bDeep==sal_True verf"ugbar?
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
        OSL_FAIL( "invalid argument type" );
    }

    // kein Item gefunden oder falschen Typ gefunden
    return 0;
}




const SfxPoolItem& SfxItemSet::Get( sal_uInt16 nWhich, sal_Bool bSrchInParent) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    // suche den Bereich in dem das Which steht:
    const SfxItemSet* pAktSet = this;
    do
    {
        if( pAktSet->Count() )
        {
            SfxItemArray ppFnd = pAktSet->_aItems;
            const sal_uInt16* pPtr = pAktSet->_pWhichRanges;
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
                    break;          // dann beim Parent suchen
                }
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
// bis zum Ende vom Such-Bereich: was nun ? zum Parent, oder Default ??
//      if( !*pPtr )            // bis zum Ende vom Such-Bereich ?
//      break;
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

sal_uInt16 SfxItemSet::TotalCount() const
{
    DBG_CHKTHIS(SfxItemSet, 0); // wird im Ctor benutzt bevor vollst. init.
    sal_uInt16 nRet = 0;
    sal_uInt16* pPtr = _pWhichRanges;
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
    if( !Count() )       // gar keine gesetzt ?
        return;

    // loesche alle Items, die im rSet nicht mehr vorhanden sind
    if( !rSet.Count() )
    {
        ClearItem();        // alles loeschen
        return;
    }

    // teste mal, ob sich die Which-Bereiche unterscheiden.
    bool bEqual = true;
    sal_uInt16* pWh1 = _pWhichRanges;
    sal_uInt16* pWh2 = rSet._pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = false;
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bEqual = *pWh1 == *pWh2;        // auch die 0 abpruefen

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
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = _pParent
                            ? _pParent->Get( nWhich, sal_True )
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
        while( true )
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( 0 == rSet.GetItemState( nWhich, sal_False ) )
                ClearItem( nWhich );        // loeschen
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
    bool bEqual = true;
    sal_uInt16* pWh1 = _pWhichRanges;
    sal_uInt16* pWh2 = rSet._pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = false;
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bEqual = *pWh1 == *pWh2;        // auch die 0 abpruefen

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
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = _pParent
                            ? _pParent->Get( nWhich, sal_True )
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
        while( true )
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False ) )
                ClearItem( nWhich );        // loeschen
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

set         set         ==      sal_False           -           -           -
default     set         ==      sal_False           -           -           -
dontcare    set         ==      sal_False           -           -           -
unknown     set         ==      sal_False           -           -           -
set         default     ==      sal_False           -           -           -
default     default     ==      sal_False           -           -           -
dontcare    default     ==      sal_False           -           -           -
unknown     default     ==      sal_False           -           -           -
set         dontcare    ==      sal_False           1.-Item     -1          -
default     dontcare    ==      sal_False           -           -1          -
dontcare    dontcare    ==      sal_False           -           -           -
unknown     dontcare    ==      sal_False           -           -           -
set         unknown     ==      sal_False           1.-Item     -1          -
default     unknown     ==      sal_False           -           -           -
dontcare    unknown     ==      sal_False           -           -           -
unknown     unknown     ==      sal_False           -           -           -

set         set         !=      sal_False           1.-Item     -1          -
default     set         !=      sal_False           -           -1          -
dontcare    set         !=      sal_False           -           -           -
unknown     set         !=      sal_False           -           -           -
set         default     !=      sal_False           1.-Item     -1          -
default     default     !=      sal_False           -           -           -
dontcare    default     !=      sal_False           -           -           -
unknown     default     !=      sal_False           -           -           -
set         dontcare    !=      sal_False           1.-Item     -1          -
default     dontcare    !=      sal_False           -           -1          -
dontcare    dontcare    !=      sal_False           -           -           -
unknown     dontcare    !=      sal_False           -           -           -
set         unknown     !=      sal_False           1.-Item     -1          -
default     unknown     !=      sal_False           -           -           -
dontcare    unknown     !=      sal_False           -           -           -
unknown     unknown     !=      sal_False           -           -           -

set         set         ==      sal_True            -           -           -
default     set         ==      sal_True            -           2.-Item     2.-Item
dontcare    set         ==      sal_True            -           -           -
unknown     set         ==      sal_True            -           -           -
set         default     ==      sal_True            -           -           -
default     default     ==      sal_True            -           -           -
dontcare    default     ==      sal_True            -           -           -
unknown     default     ==      sal_True            -           -           -
set         dontcare    ==      sal_True            -           -           -
default     dontcare    ==      sal_True            -           -1          -
dontcare    dontcare    ==      sal_True            -           -           -
unknown     dontcare    ==      sal_True            -           -           -
set         unknown     ==      sal_True            -           -           -
default     unknown     ==      sal_True            -           -           -
dontcare    unknown     ==      sal_True            -           -           -
unknown     unknown     ==      sal_True            -           -           -

set         set         !=      sal_True            1.-Item     -1          -
default     set         !=      sal_True            -           2.-Item     2.-Item
dontcare    set         !=      sal_True            -           -           -
unknown     set         !=      sal_True            -           -           -
set         default     !=      sal_True            -           -           -
default     default     !=      sal_True            -           -           -
dontcare    default     !=      sal_True            -           -           -
unknown     default     !=      sal_True            -           -           -
set         dontcare    !=      sal_True            1.-Item     -1          -
default     dontcare    !=      sal_True            -           -1          -
dontcare    dontcare    !=      sal_True            -           -           -
unknown     dontcare    !=      sal_True            -           -           -
set         unknown     !=      sal_True            -           -           -
default     unknown     !=      sal_True            -           -           -
dontcare    unknown     !=      sal_True            -           -           -
unknown     unknown     !=      sal_True            -           -           -
*/


static void MergeItem_Impl( SfxItemPool *_pPool, sal_uInt16 &rCount,
                            const SfxPoolItem **ppFnd1, const SfxPoolItem *pFnd2,
                            sal_Bool bIgnoreDefaults )
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
            // Entscheidungstabelle: default, set, !=, sal_False
            *ppFnd1 = (SfxPoolItem*) -1;

        else if ( pFnd2 && bIgnoreDefaults )
            // Entscheidungstabelle: default, set, egal, sal_True
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
                // Entscheidungstabelle: set, default, !=, sal_False
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
                // Entscheidungstabelle: set, dontcare, egal, sal_False
                // oder:                 set, dontcare, !=, sal_True
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

void SfxItemSet::MergeValues( const SfxItemSet& rSet, sal_Bool bIgnoreDefaults )
{
    // Achtung!!! Bei Aenderungen/Bugfixes immer obenstehende Tabelle pflegen!
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( GetPool() == rSet.GetPool(), "MergeValues mit verschiedenen Pools" );

    // teste mal, ob sich die Which-Bereiche unterscheiden.
    bool bEqual = true;
    sal_uInt16* pWh1 = _pWhichRanges;
    sal_uInt16* pWh2 = rSet._pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            bEqual = false;
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
        sal_uInt16 nWhich;
        while( 0 != ( nWhich = aIter.NextWhich() ) )
        {
            const SfxPoolItem* pItem = 0;
            rSet.GetItemState( nWhich, sal_True, &pItem );
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

void SfxItemSet::MergeValue( const SfxPoolItem& rAttr, sal_Bool bIgnoreDefaults )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const sal_uInt16* pPtr = _pWhichRanges;
    const sal_uInt16 nWhich = rAttr.Which();
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

void SfxItemSet::InvalidateItem( sal_uInt16 nWhich )
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const sal_uInt16* pPtr = _pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // in diesem Bereich
            ppFnd += nWhich - *pPtr;

            if( *ppFnd )    // bei mir gesetzt
            {
                if( (SfxPoolItem*)-1 != *ppFnd )        // noch nicht dontcare !
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

sal_uInt16 SfxItemSet::GetWhichByPos( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    sal_uInt16 n = 0;
    sal_uInt16* pPtr  = _pWhichRanges;
    while( *pPtr )
    {
        n = ( *(pPtr+1) - *pPtr ) + 1;
        if( nPos < n )
            return *(pPtr)+nPos;
        nPos = nPos - n;
        pPtr += 2;
    }
    DBG_ASSERT( sal_False, "Hier sind wir falsch" );
    return 0;
}

// -----------------------------------------------------------------------

SvStream &SfxItemSet::Store
(
    SvStream&   rStream,        // Zielstream f"ur normale Items
    bool        bDirect         // TRUE: Items direkt speicher, FALSE: Surrogate
)   const

/*  [Beschreibung]

    Speichert die <SfxItemSet>-Instanz in den angegebenen Stream. Dabei
    werden die Surrorage der gesetzten <SfxPoolItem>s bzw. ('bDirect==sal_True')
    die gesetzten Items selbst wie folgt im Stream abgelegt:

            sal_uInt16              (Count) Anzahl der gesetzten Items
    Count*  _pPool->StoreItem()  siehe <SfxItemPool::StoreItem()const>


    [Querverweise]

    <SfxItemSet::Load(SvStream&,sal_Bool,const SfxItemPool*)>
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( _pPool, "Kein Pool" );
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "kein Master-Pool" );

    // Position des Counts merken, um ggf. zu korrigieren
    sal_uLong nCountPos = rStream.Tell();
    rStream << _nCount;

    // wenn nichts zu speichern ist, auch keinen ItemIter aufsetzen!
    if ( _nCount )
    {
        // mitz"ahlen wieviel Items tats"achlich gespeichert werden
        sal_uInt16 nWrittenCount = 0;  // Anzahl in 'rStream' gestreamter Items

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
            sal_uLong nPos = rStream.Tell();
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
    SvStream&           rStream,    //  Stream, aus dem geladen werden soll

    bool                bDirect,    /*  TRUE
                                        Items werden direkt aus dem Stream
                                        gelesen, nicht "uber Surrogate

                                        sal_False (default)
                                        Items werden "uber Surrogate gelesen */

    const SfxItemPool*  pRefPool    /*  Pool, der die Surrogate aufl"osen kann
                                        (z.B. zum Einf"ugen von Dokumenten) */
)

/*  [Beschreibung]

    Diese Methode l"adt ein <SfxItemSet> aus einem Stream. Falls der
    <SfxItemPool> ohne Ref-Counts geladen wurde, werden die geladenen
    Item-Referenzen in den Items hochgez"ahlt, ansonsten wird vorausgesetzt,
    da\s sie schon beim Laden des SfxItemPools ber"ucksichtigt waren.

    [Querverweise]

    <SfxItemSet::Store(Stream&,sal_Bool)const>
*/

{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_ASSERT( _pPool, "Kein Pool");
    DBG_ASSERTWARNING( _pPool == _pPool->GetMasterPool(), "Kein Master-Pool");

    // kein Ref-Pool => Surrogate mit Pool des ItemSets aufl"osen
    if ( !pRefPool )
        pRefPool = _pPool;

    // Anzahl der zu ladenden Items laden und dann ebensoviele Items
    sal_uInt16 nCount = 0;
    rStream >> nCount;
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        // Surrogat/Item laden und (Surrogat) aufl"osen lassen
        const SfxPoolItem *pItem =
                _pPool->LoadItem( rStream, bDirect, pRefPool );

        // konnte ein Item geladen oder via Surrogat aufgel"ost werden?
        if ( pItem )
        {
            // Position f"ur Item-Pointer im Set suchen
            sal_uInt16 nWhich = pItem->Which();
            SfxItemArray ppFnd = _aItems;
            const sal_uInt16* pPtr = _pWhichRanges;
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

int SfxItemSet::operator==(const SfxItemSet &rCmp) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    DBG_CHKOBJ(&rCmp, SfxItemSet, DbgCheckItemSet);

    // besonders schnell zu ermittelnde Werte muessen gleich sein
    if ( _pParent != rCmp._pParent ||
         _pPool != rCmp._pPool ||
         Count() != rCmp.Count() )
        return sal_False;

    // Ranges durchzaehlen lassen dauert laenger, muss aber auch gleich sein
    sal_uInt16 nCount1 = TotalCount();
    sal_uInt16 nCount2 = rCmp.TotalCount();
    if ( nCount1 != nCount2 )
        return sal_False;

    // sind die Ranges selbst ungleich?
    for ( sal_uInt16 nRange = 0; _pWhichRanges[nRange]; nRange += 2 )
        if ( _pWhichRanges[nRange] != rCmp._pWhichRanges[nRange] ||
             _pWhichRanges[nRange+1] != rCmp._pWhichRanges[nRange+1] )
        {
            // dann m"ussen wir die langsame Methode verwenden
            SfxWhichIter aIter( *this );
            for ( sal_uInt16 nWh = aIter.FirstWhich();
                  nWh;
                  nWh = aIter.NextWhich() )
            {
                // wenn die Pointer von poolable Items ungleich sind,
                // muessen die Items gleich sein
                const SfxPoolItem *pItem1 = 0, *pItem2 = 0;
                if ( GetItemState( nWh, sal_False, &pItem1 ) !=
                        rCmp.GetItemState( nWh, sal_False, &pItem2 ) ||
                     ( pItem1 != pItem2 &&
                        ( !pItem1 || IsInvalidItem(pItem1) ||
                          ( _pPool->IsItemFlag(*pItem1, SFX_ITEM_POOLABLE) &&
                            *pItem1 != *pItem2 ) ) ) )
                    return sal_False;
            }

            return sal_True;
        }

    // Pointer alle gleich?
    if ( 0 == memcmp( _aItems, rCmp._aItems, nCount1 * sizeof(_aItems[0]) ) )
        return sal_True;

    // dann werden wir wohl alle einzeln vergleichen muessen
    const SfxPoolItem **ppItem1 = (const SfxPoolItem**) _aItems;
    const SfxPoolItem **ppItem2 = (const SfxPoolItem**) rCmp._aItems;
    for ( sal_uInt16 nPos = 0; nPos < nCount1; ++nPos )
    {
        // wenn die Pointer von poolable Items ungleich sind,
        // muessen die Items gleich sein
        if ( *ppItem1 != *ppItem2 &&
             ( ( !*ppItem1 || !*ppItem2 ) ||
               ( IsInvalidItem(*ppItem1) || IsInvalidItem(*ppItem2) ) ||
               ( _pPool->IsItemFlag(**ppItem1, SFX_ITEM_POOLABLE) ) ||
                 **ppItem1 != **ppItem2 ) )
            return sal_False;

        ++ppItem1;
        ++ppItem2;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

SfxItemSet *SfxItemSet::Clone(sal_Bool bItems, SfxItemPool *pToPool ) const
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    if ( pToPool && pToPool != _pPool )
    {
        SfxItemSet *pNewSet = new SfxItemSet( *pToPool, _pWhichRanges );
        if ( bItems )
        {
            SfxWhichIter aIter(*pNewSet);
            sal_uInt16 nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SFX_ITEM_SET == GetItemState( nWhich, sal_False, &pItem ) )
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

int SfxItemSet::PutDirect(const SfxPoolItem &rItem)
{
    DBG_CHKTHIS(SfxItemSet, DbgCheckItemSet);
    SfxItemArray ppFnd = _aItems;
    const sal_uInt16* pPtr = _pWhichRanges;
    const sal_uInt16 nWhich = rItem.Which();
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
            if( pOld )      // schon einer vorhanden
            {
                if( rItem == **ppFnd )
                    return sal_False;       // schon vorhanden !
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

            return sal_True;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return sal_False;
}

// -----------------------------------------------------------------------

SfxAllItemSet::SfxAllItemSet( SfxItemPool &rPool )
:   SfxItemSet(rPool, (const sal_uInt16*) 0),
    aDefault(0),
    nFree(nInitCount)
{
    // initial keine Items
    _aItems = 0;

    // nInitCount Paare an USHORTs fuer Ranges allozieren
    _pWhichRanges = new sal_uInt16[ nInitCount + 1 ];
    memset( _pWhichRanges, 0, ( nInitCount + 1 ) * sizeof(sal_uInt16) );
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
/*  [Anmerkung]

    Der mu\s sein, da sonst vom Compiler einer generiert wird, er nimmt
    nicht den Ctor mit der 'const SfxItemSet&'!
*/
{
}

// -----------------------------------------------------------------------

static sal_uInt16 *AddRanges_Impl(
    sal_uInt16 *pUS, std::ptrdiff_t nOldSize, sal_uInt16 nIncr)

/*  Diese interne Funktion erzeugt ein neues Which-Range-Array, welches von
    dem 'nOldSize'-USHORTs langen 'pUS' kopiert wird und hinten an Platz
    f"ur 'nIncr' neue USHORTs hat. Das terminierende sal_uInt16 mit der '0'
    wird weder in 'nOldSize' noch in 'nIncr' mitgez"ahlt, sondern implizit
    hinzugerechnet.

    Das neue Which-Range-Array wird als Returnwert zur"uckgegeben, das alte
    'pUS' freigegeben.
*/

{
    // neues Which-Range-Array anlegen
    sal_uInt16 *pNew = new sal_uInt16[ nOldSize + nIncr + 1 ];

    // die alten Ranges "ubernehmen
    memcpy( pNew, pUS, nOldSize * sizeof(sal_uInt16) );

    // die neuen auf 0 initialisieren
    memset( pNew + nOldSize, 0, ( nIncr + 1 ) * sizeof(sal_uInt16) );

    // das alte Array freigeben
    delete[] pUS;

    return pNew;
}

// -----------------------------------------------------------------------

static SfxItemArray AddItem_Impl(SfxItemArray pItems, sal_uInt16 nOldSize, sal_uInt16 nPos)

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
            memcpy( (void*) pNew, pItems, nPos * sizeof(SfxPoolItem *) );

        // alte Items hinter nPos kopieren
        if ( nPos < nOldSize )
            memcpy( (void*) (pNew + nPos + 1), pItems + nPos,
                    (nOldSize-nPos) * sizeof(SfxPoolItem *) );
    }

    // neues Item initialisieren
    *(pNew + nPos) = 0;

    // altes ItemArray freigeben
    delete[] pItems;

    return pNew;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxAllItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )

// Putten mit automatischer Erweiterung der Whichs-Ids um die ID
// des Items.

{
    sal_uInt16 nPos = 0; // Position f"ur 'rItem' in '_aItems'
    const sal_uInt16 nItemCount = TotalCount();

    // erstmal sehen, ob es schon einen passenden Bereich gibt
    sal_uInt16 *pPtr = _pWhichRanges;
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
    bool bIncrementCount = false;
    const SfxPoolItem* pOld = *( _aItems + nPos );
    if ( reinterpret_cast< SfxPoolItem* >( -1 ) == pOld )   // state "dontcare"
        pOld = NULL;
    if ( !pOld )
    {
        bIncrementCount = true;
        pOld = _pParent ?
                &_pParent->Get( nWhich, sal_True )
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


/*  Diese Methode wird forwarded, damit sie nicht durch die anderen
    Put-Methoden dieser SubClass gehided wird.
*/

int SfxAllItemSet::Put( const SfxItemSet& rSet, sal_Bool bInvalidAsDefault )
{
    //? pruefen, ob Which-Ranges erweitert werden
    return SfxItemSet::Put( rSet, bInvalidAsDefault );
}

// -----------------------------------------------------------------------
// Item disablen, wenn durch ein VoidItem mit dem Which-Wert 0 ausgedrueckt

void SfxItemSet::DisableItem(sal_uInt16 nWhich)
{
    DBG_CHKTHIS(SfxItemSet, 0);
    Put( SfxVoidItem(0), nWhich );
}

// -----------------------------------------------------------------------

SfxItemSet *SfxAllItemSet::Clone(sal_Bool bItems, SfxItemPool *pToPool ) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
