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
#include "precompiled_svl.hxx"

#include <string.h>
#include <stdio.h>
#ifndef GCC
#endif

#include <svl/itempool.hxx>
#include "whassert.hxx"
#include <svl/brdcst.hxx>
#include <svl/smplhint.hxx>
#include "poolio.hxx"

//========================================================================


void SfxItemPool::AddSfxItemPoolUser(SfxItemPoolUser& rNewUser)
{
    maSfxItemPoolUsers.push_back(&rNewUser);
}

void SfxItemPool::RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser)
{
    const SfxItemPoolUserVector::iterator aFindResult = ::std::find(maSfxItemPoolUsers.begin(), maSfxItemPoolUsers.end(), &rOldUser);
    if(aFindResult != maSfxItemPoolUsers.end())
    {
        maSfxItemPoolUsers.erase(aFindResult);
    }
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem( sal_uInt16 nWhich ) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    const SfxPoolItem* pRet;
    if( IsInRange( nWhich ) )
        pRet = *(ppPoolDefaults + GetIndex_Impl( nWhich ));
    else if( pSecondary )
        pRet = pSecondary->GetPoolDefaultItem( nWhich );
    else
    {
        SFX_ASSERT( 0, nWhich, "unknown Which-Id - cannot get pool default" );
        pRet = 0;
    }
    return pRet;
}

// -----------------------------------------------------------------------

inline FASTBOOL SfxItemPool::IsItemFlag_Impl( sal_uInt16 nPos, sal_uInt16 nFlag ) const
{
    sal_uInt16 nItemFlag = pItemInfos[nPos]._nFlags;
    return nFlag == (nItemFlag & nFlag);
}

// -----------------------------------------------------------------------

FASTBOOL SfxItemPool::IsItemFlag( sal_uInt16 nWhich, sal_uInt16 nFlag ) const
{
    for ( const SfxItemPool *pPool = this; pPool; pPool = pPool->pSecondary )
    {
        if ( pPool->IsInRange(nWhich) )
            return pPool->IsItemFlag_Impl( pPool->GetIndex_Impl(nWhich), nFlag);
    }
    DBG_ASSERT( !IsWhich(nWhich), "unknown which-id" );
    return sal_False;
}

// -----------------------------------------------------------------------

SfxBroadcaster& SfxItemPool::BC()
{
    return pImp->aBC;
}

// -----------------------------------------------------------------------


SfxItemPool::SfxItemPool
(
    UniString const &   rName,          /* Name des Pools zur Idetifikation
                                           im File-Format */
    sal_uInt16              nStartWhich,    /* erste Which-Id des Pools */
    sal_uInt16              nEndWhich,      /* letzte Which-Id des Pools */
#ifdef TF_POOLABLE
    const SfxItemInfo*  pInfos,         /* SID-Map und Item-Flags */
#endif
    SfxPoolItem**       pDefaults,      /* Pointer auf statische Defaults,
                                           wird direkt vom Pool referenziert,
                                           jedoch kein Eigent"umer"ubergang */
#ifndef TF_POOLABLE
    sal_uInt16*             pSlotIdArray,   /* Zuordnung von Slot-Ids zu Which-Ids */
#endif
    FASTBOOL            bLoadRefCounts  /* Ref-Counts mitladen oder auf 1 setzen */
)

/*  [Beschreibung]

    Der im Normalfall verwendete Konstruktor der Klasse SfxItemPool. Es
    wird eine SfxItemPool-Instanz initialisiert, die Items im b"undigen
    Which-Bereich von 'nStartWhich' bis 'nEndWhich' verwalten kann.

    F"ur jede dieser Which-Ids mu\s ein statischer Default im Array 'pDefaults'
    vorhanden sein, die dort beginnend mit einem <SfxPoolItem> mit der
    Which-Id 'nStartWhich' nach Which-Ids sortiert aufeinanderfolgend
    eingetragen sein m"ussen.

    'pItemInfos' ist ein identisch angeordnetes Array von USHORTs, die
    Slot-Ids darstellen und Flags. Die Slot-Ids k"onnen 0 sein, wenn die
    betreffenden Items ausschlie\slich in der Core verwendet werden.
    "Uber die Flags kann z.B. bestimmt werden, ob Value-Sharing
    (SFX_ITEM_POOLABLE) stattfinden soll.

    [Anmerkung]

    Wenn der Pool <SfxSetItem>s enthalten soll, k"onnen im Konstruktor noch
    keine static-Defaults angegeben werden. Dies mu\s dann nachtr"aglich
    mit <SfxItemPool::SetDefaults(SfxItemPool**)> geschehen.


    [Querverweise]

    <SfxItemPool::SetDefaults(SfxItemPool**)>
    <SfxItemPool::ReleaseDefaults(SfxPoolItem**,sal_uInt16,sal_Bool)>
    <SfxItemPool::ReldaseDefaults(sal_Bool)>
*/

:   aName(rName),
    nStart(nStartWhich),
    nEnd(nEndWhich),
#ifdef TF_POOLABLE
    pItemInfos(pInfos),
#else
    pSlotIds(pSlotIdArray),
#endif
    pImp( new SfxItemPool_Impl( nStart, nEnd ) ),
    ppStaticDefaults(0),
    ppPoolDefaults(new SfxPoolItem* [ nEndWhich - nStartWhich + 1]),
    pSecondary(0),
    pMaster(this),
    _pPoolRanges( 0 ),
    bPersistentRefCounts(bLoadRefCounts),
    maSfxItemPoolUsers()
{
    DBG_CTOR(SfxItemPool, 0);
    DBG_ASSERT(nStart, "Start-Which-Id must be greater 0" );

    pImp->eDefMetric = SFX_MAPUNIT_TWIP;
    pImp->nVersion = 0;
    pImp->bStreaming = sal_False;
    pImp->nLoadingVersion = 0;
    pImp->nInitRefCount = 1;
    pImp->nVerStart = nStart;
    pImp->nVerEnd = nEnd;
    pImp->bInSetItem = sal_False;
    pImp->nStoringStart = nStartWhich;
    pImp->nStoringEnd = nEndWhich;

    memset( ppPoolDefaults, 0, sizeof( SfxPoolItem* ) * (nEnd - nStart + 1));

    if ( pDefaults )
        SetDefaults(pDefaults);
}

// -----------------------------------------------------------------------


SfxItemPool::SfxItemPool
(
    const SfxItemPool&  rPool,                  //  von dieser Instanz kopieren
    sal_Bool                bCloneStaticDefaults    /*  sal_True
                                                    statische Defaults kopieren

                                                    sal_False
                                                    statische Defaults
                                                    "ubernehehmen */
)

/*  [Beschreibung]

    Copy-Konstruktor der Klasse SfxItemPool.


    [Querverweise]

    <SfxItemPool::Clone()const>
*/

:   aName(rPool.aName),
    nStart(rPool.nStart),
    nEnd(rPool.nEnd),
#ifdef TF_POOLABLE
    pItemInfos(rPool.pItemInfos),
#else
    pSlotIds(rPool.pSlotIds),
#endif
    pImp( new SfxItemPool_Impl( nStart, nEnd ) ),
    ppStaticDefaults(0),
    ppPoolDefaults(new SfxPoolItem* [ nEnd - nStart + 1]),
    pSecondary(0),
    pMaster(this),
    _pPoolRanges( 0 ),
    bPersistentRefCounts(rPool.bPersistentRefCounts ),
    maSfxItemPoolUsers()
{
    DBG_CTOR(SfxItemPool, 0);
    pImp->eDefMetric = rPool.pImp->eDefMetric;
    pImp->nVersion = rPool.pImp->nVersion;
    pImp->bStreaming = sal_False;
    pImp->nLoadingVersion = 0;
    pImp->nInitRefCount = 1;
    pImp->nVerStart = rPool.pImp->nVerStart;
    pImp->nVerEnd = rPool.pImp->nVerEnd;
    pImp->bInSetItem = sal_False;
    pImp->nStoringStart = nStart;
    pImp->nStoringEnd = nEnd;

    memset( ppPoolDefaults, 0, sizeof( SfxPoolItem* ) * (nEnd - nStart + 1));

    // Static Defaults "ubernehmen
    if ( bCloneStaticDefaults )
    {
        SfxPoolItem **ppDefaults = new SfxPoolItem*[nEnd-nStart+1];
        for ( sal_uInt16 n = 0; n <= nEnd - nStart; ++n )
        {
            (*( ppDefaults + n )) = (*( rPool.ppStaticDefaults + n ))->Clone(this);
            (*( ppDefaults + n ))->SetKind( SFX_ITEMS_STATICDEFAULT );
        }

        SetDefaults( ppDefaults );
    }
    else
        SetDefaults( rPool.ppStaticDefaults );

    // Pool Defaults kopieren
    for ( sal_uInt16 n = 0; n <= nEnd - nStart; ++n )
        if ( (*( rPool.ppPoolDefaults + n )) )
        {
            (*( ppPoolDefaults + n )) = (*( rPool.ppPoolDefaults + n ))->Clone(this);
            (*( ppPoolDefaults + n ))->SetKind( SFX_ITEMS_POOLDEFAULT );
        }

    // Copy Version-Map
    for ( size_t nVer = 0; nVer < rPool.pImp->aVersions.size(); ++nVer )
    {
        const SfxPoolVersion_ImplPtr pOld = rPool.pImp->aVersions[nVer];
        SfxPoolVersion_ImplPtr pNew = SfxPoolVersion_ImplPtr( new SfxPoolVersion_Impl( *pOld ) );
        pImp->aVersions.push_back( pNew );
    }

    // Verkettung wiederherstellen
    if ( rPool.pSecondary )
        SetSecondaryPool( rPool.pSecondary->Clone() );
}

// -----------------------------------------------------------------------

void SfxItemPool::SetDefaults( SfxPoolItem **pDefaults )
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT( pDefaults, "erst wollen, dann nichts geben..." );
    DBG_ASSERT( !ppStaticDefaults, "habe schon defaults" );

    ppStaticDefaults = pDefaults;
    //! if ( (*ppStaticDefaults)->GetKind() != SFX_ITEMS_STATICDEFAULT )
    //! geht wohl nicht im Zshg mit SetItems, die hinten stehen
    {
        DBG_ASSERT( (*ppStaticDefaults)->GetRefCount() == 0 ||
                    IsDefaultItem( (*ppStaticDefaults) ),
                    "das sind keine statics" );
        for ( sal_uInt16 n = 0; n <= nEnd - nStart; ++n )
        {
            SFX_ASSERT( (*( ppStaticDefaults + n ))->Which() == n + nStart,
                        n + nStart, "static defaults not sorted" );
            (*( ppStaticDefaults + n ))->SetKind( SFX_ITEMS_STATICDEFAULT );
            DBG_ASSERT( !(pImp->ppPoolItems[n]), "defaults with setitems with items?!" );
        }
    }
}

// -----------------------------------------------------------------------

void SfxItemPool::ReleaseDefaults
(
    sal_Bool    bDelete     /*  sal_True
                            l"oscht sowohl das Array als auch die einzelnen
                            statischen Defaults

                            sal_False
                            l"oscht weder das Array noch die einzelnen
                            statischen Defaults */
)

/*  [Beschreibung]

    Gibt die statischen Defaults der betreffenden SfxItemPool-Instanz frei
    und l"oscht ggf. die statischen Defaults.

    Nach Aufruf dieser Methode darf die SfxItemPool-Instanz nicht mehr
    verwendet werden, einzig ist der Aufruf des Destruktors zu"lassig.
*/

{
    DBG_ASSERT( ppStaticDefaults, "keine Arme keine Kekse" );
    ReleaseDefaults( ppStaticDefaults, nEnd - nStart + 1, bDelete );

    // KSO (22.10.98): ppStaticDefaults zeigt auf geloeschten Speicher,
    // wenn bDelete == sal_True.
    if ( bDelete )
        ppStaticDefaults = 0;
}

// -----------------------------------------------------------------------

void SfxItemPool::ReleaseDefaults
(
    SfxPoolItem**   pDefaults,  /*  freizugebende statische Defaults */

    sal_uInt16          nCount,     /*  Anzahl der statischen Defaults */

    sal_Bool            bDelete     /*  sal_True
                                    l"oscht sowohl das Array als auch die
                                    einzelnen statischen Defaults

                                    sal_False
                                    l"oscht weder das Array noch die
                                    einzelnen statischen Defaults */
)

/*  [Beschreibung]

    Gibt die angegebenen statischen Defaults frei und l"oscht ggf.
    die statischen Defaults.

    Diese Methode darf erst nach Zerst"orung aller SfxItemPool-Instanzen,
    welche die angegebenen statischen Defaults 'pDefault' verwenden,
    aufgerufen werden.
*/

{
    DBG_ASSERT( pDefaults, "erst wollen, dann nichts geben..." );

    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SFX_ASSERT( IsStaticDefaultItem( *(pDefaults+n) ),
                    n, "das ist kein static-default" );
        (*( pDefaults + n ))->SetRefCount( 0 );
        if ( bDelete )
            { delete *( pDefaults + n ); *(pDefaults + n) = 0; }
    }

    if ( bDelete )
        { delete[] pDefaults; pDefaults = 0; }
}

// -----------------------------------------------------------------------

SfxItemPool::~SfxItemPool()
{
    DBG_DTOR(SfxItemPool, 0);
    DBG_ASSERT( pMaster == this, "destroying active Secondary-Pool" );

    if ( pImp->ppPoolItems && ppPoolDefaults )
        Delete();
    delete[] _pPoolRanges;
    delete pImp;
}

void SfxItemPool::Free(SfxItemPool* pPool)
{
    if(pPool)
    {
        // tell all the registered SfxItemPoolUsers that the pool is in destruction
        SfxItemPoolUserVector aListCopy(pPool->maSfxItemPoolUsers.begin(), pPool->maSfxItemPoolUsers.end());
        for(SfxItemPoolUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); aIterator++)
        {
            SfxItemPoolUser* pSfxItemPoolUser = *aIterator;
            DBG_ASSERT(pSfxItemPoolUser, "corrupt SfxItemPoolUser list (!)");
            pSfxItemPoolUser->ObjectInDestruction(*pPool);
        }

        // Clear the vector. This means that user do not need to call RemoveSfxItemPoolUser()
        // when they get called from ObjectInDestruction().
        pPool->maSfxItemPoolUsers.clear();

        // delete pool
        delete pPool;
    }
}

// -----------------------------------------------------------------------


void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // ggf. an abgeh"angten Pools den Master zur"ucksetzen
    if ( pSecondary )
    {
#ifdef DBG_UTIL
        HACK( "fuer Image, dort gibt es derzeit keine Statics - Bug" )
        if ( ppStaticDefaults )
        {
            // Delete() ist noch nicht gelaufen?
            if ( pImp->ppPoolItems && pSecondary->pImp->ppPoolItems )
            {
                // hat der master SetItems?
                sal_Bool bHasSetItems = sal_False;
                for ( sal_uInt16 i = 0; !bHasSetItems && i < nEnd-nStart; ++i )
                    bHasSetItems = ppStaticDefaults[i]->ISA(SfxSetItem);

                // abgehaengte Pools muessen leer sein
                sal_Bool bOK = bHasSetItems;
                for ( sal_uInt16 n = 0;
                      bOK && n <= pSecondary->nEnd - pSecondary->nStart;
                      ++n )
                {
                    SfxPoolItemArray_Impl** ppItemArr =
                                                pSecondary->pImp->ppPoolItems + n;
                    if ( *ppItemArr )
                    {
                        SfxPoolItemArrayBase_Impl::iterator ppHtArr =   (*ppItemArr)->begin();
                        for( size_t i = (*ppItemArr)->size(); i; ++ppHtArr, --i )
                            if ( !(*ppHtArr) )
                            {
                                DBG_ERROR( "old secondary pool must be empty" );
                                bOK = sal_False;
                                break;
                            }
                    }
                }
            }
        }
#endif

        pSecondary->pMaster = pSecondary;
        for ( SfxItemPool *p = pSecondary->pSecondary; p; p = p->pSecondary )
            p->pMaster = pSecondary;
    }

    // ggf. den Master der neuen Secondary-Pools setzen
    DBG_ASSERT( !pPool || pPool->pMaster == pPool, "Secondary tanzt auf zwei Hochzeiten " );
    SfxItemPool *pNewMaster = pMaster ? pMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->pSecondary )
        p->pMaster = pNewMaster;

    // neuen Secondary-Pool merken
    pSecondary = pPool;
}

// -----------------------------------------------------------------------

SfxMapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxItemPool, 0);

    return pImp->eDefMetric;
}

// -----------------------------------------------------------------------

void SfxItemPool::SetDefaultMetric( SfxMapUnit eNewMetric )
{
    DBG_CHKTHIS(SfxItemPool, 0);

    pImp->eDefMetric = eNewMetric;
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxItemPool::GetPresentation
(
    const SfxPoolItem&  rItem,      /*  IN: <SfxPoolItem>, dessen textuelle
                                            Wert-Darstellung geliefert werden
                                            soll */
    SfxItemPresentation ePresent,   /*  IN: gew"unschte Art der Darstellung;
                                            siehe <SfxItemPresentation> */
    SfxMapUnit          eMetric,    /*  IN: gew"unschte Ma\seinheit der Darstellung */
    XubString&           rText,      /*  OUT: textuelle Darstellung von 'rItem' */
    const IntlWrapper * pIntlWrapper
)   const

/*  [Beschreibung]

    "Uber diese virtuelle Methode k"onnen textuelle Darstellungen der
    von der jeweilige SfxItemPool-Subklasse verwalteten SfxPoolItems
    angefordert werden.

    In Ableitungen sollte diese Methode "uberladen werden und auf
    SfxPoolItems reagiert werden, die bei <SfxPoolItem::GetPresentation()const>
    keine vollst"andige Information liefern k"onnen.

    Die Basisklasse liefert die unver"anderte Presentation von 'rItem'.
*/

{
    DBG_CHKTHIS(SfxItemPool, 0);
    return rItem.GetPresentation(
        ePresent, GetMetric(rItem.Which()), eMetric, rText, pIntlWrapper );
}


// -----------------------------------------------------------------------

SfxItemPool* SfxItemPool::Clone() const
{
    DBG_CHKTHIS(SfxItemPool, 0);

    SfxItemPool *pPool = new SfxItemPool( *this );
    return pPool;
}

// ----------------------------------------------------------------------

void SfxItemPool::Delete()
{
    DBG_CHKTHIS(SfxItemPool, 0);

    // schon deleted?
    if ( !pImp->ppPoolItems || !ppPoolDefaults )
        return;

    // z.B. laufenden Requests bescheidsagen
    pImp->aBC.Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );

    //MA 16. Apr. 97: Zweimal durchlaufen, in der ersten Runde fuer die SetItems.
    //Der Klarheit halber wird das jetzt in zwei besser lesbare Schleifen aufgeteilt.

    SfxPoolItemArray_Impl** ppItemArr = pImp->ppPoolItems;
    SfxPoolItem** ppDefaultItem = ppPoolDefaults;
    SfxPoolItem** ppStaticDefaultItem = ppStaticDefaults;
    sal_uInt16 nArrCnt;

    //Erst die SetItems abraeumen
    HACK( "fuer Image, dort gibt es derzeit keine Statics - Bug" )
    if ( ppStaticDefaults )
    {
        for ( nArrCnt = GetSize_Impl();
                nArrCnt;
                --nArrCnt, ++ppItemArr, ++ppDefaultItem, ++ppStaticDefaultItem )
        {
            // KSO (22.10.98): *ppStaticDefaultItem kann im dtor einer
            // von SfxItemPool abgeleiteten Klasse bereits geloescht worden
            // sein! -> CHAOS Itempool
            if ( *ppStaticDefaultItem && (*ppStaticDefaultItem)->ISA(SfxSetItem) )
            {
                if ( *ppItemArr )
                {
                    SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*ppItemArr)->begin();
                    for ( size_t n = (*ppItemArr)->size(); n; --n, ++ppHtArr )
                        if (*ppHtArr)
                        {
#ifdef DBG_UTIL
                            ReleaseRef( **ppHtArr, (*ppHtArr)->GetRefCount() );
#endif
                            delete *ppHtArr;
                        }
                    DELETEZ( *ppItemArr );
                }
                if ( *ppDefaultItem )
                {
#ifdef DBG_UTIL
                    SetRefCount( **ppDefaultItem, 0 );
#endif
                    DELETEZ( *ppDefaultItem );
                }
            }
        }
    }

    ppItemArr = pImp->ppPoolItems;
    ppDefaultItem = ppPoolDefaults;

    //Jetzt die 'einfachen' Items
    for ( nArrCnt = GetSize_Impl();
            nArrCnt;
            --nArrCnt, ++ppItemArr, ++ppDefaultItem )
    {
        if ( *ppItemArr )
        {
            SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*ppItemArr)->begin();
            for ( size_t n = (*ppItemArr)->size(); n; --n, ++ppHtArr )
                if (*ppHtArr)
                {
#ifdef DBG_UTIL
                    ReleaseRef( **ppHtArr, (*ppHtArr)->GetRefCount() );
#endif
                    delete *ppHtArr;
                }
            delete *ppItemArr;
        }
        if ( *ppDefaultItem )
        {
#ifdef DBG_UTIL
            SetRefCount( **ppDefaultItem, 0 );
#endif
            delete *ppDefaultItem;
        }
    }

    pImp->DeleteItems();
    delete[] ppPoolDefaults; ppPoolDefaults = 0;
}

// ----------------------------------------------------------------------

void SfxItemPool::Cleanup()
{
    DBG_CHKTHIS(SfxItemPool, 0);

    //MA 16. Apr. 97: siehe ::Delete()

    SfxPoolItemArray_Impl** ppItemArr = pImp->ppPoolItems;
    SfxPoolItem** ppDefaultItem = ppPoolDefaults;
    SfxPoolItem** ppStaticDefaultItem = ppStaticDefaults;
    sal_uInt16 nArrCnt;

    HACK( "fuer Image, dort gibt es derzeit keine Statics - Bug" )
    if ( ppStaticDefaults ) //HACK fuer Image, dort gibt es keine Statics!!
    {
        for ( nArrCnt = GetSize_Impl();
                nArrCnt;
                --nArrCnt, ++ppItemArr, ++ppDefaultItem, ++ppStaticDefaultItem )
        {
            //Fuer jedes Item gibt es entweder ein Default oder ein static Default!
            if ( *ppItemArr &&
                 ((*ppDefaultItem && (*ppDefaultItem)->ISA(SfxSetItem)) ||
                  (*ppStaticDefaultItem)->ISA(SfxSetItem)) )
            {
                SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*ppItemArr)->begin();
                for ( size_t n = (*ppItemArr)->size(); n; --n, ++ppHtArr )
                    if ( *ppHtArr && !(*ppHtArr)->GetRefCount() )
                    {
                         DELETEZ(*ppHtArr);
                    }
            }
        }
    }

    ppItemArr = pImp->ppPoolItems;

    for ( nArrCnt = GetSize_Impl();
          nArrCnt;
          --nArrCnt, ++ppItemArr )
    {
        if ( *ppItemArr )
        {
            SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*ppItemArr)->begin();
            for ( size_t n = (*ppItemArr)->size(); n; --n, ++ppHtArr )
                if ( *ppHtArr && !(*ppHtArr)->GetRefCount() )
                    DELETEZ( *ppHtArr );
        }
    }
}

// ----------------------------------------------------------------------

void SfxItemPool::SetPoolDefaultItem(const SfxPoolItem &rItem)
{
    DBG_CHKTHIS(SfxItemPool, 0);
    if ( IsInRange(rItem.Which()) )
    {
        SfxPoolItem **ppOldDefault =
            ppPoolDefaults + GetIndex_Impl(rItem.Which());
        SfxPoolItem *pNewDefault = rItem.Clone(this);
        pNewDefault->SetKind(SFX_ITEMS_POOLDEFAULT);
        if ( *ppOldDefault )
        {
            (*ppOldDefault)->SetRefCount(0);
            DELETEZ( *ppOldDefault );
        }
        *ppOldDefault = pNewDefault;
    }
    else if ( pSecondary )
        pSecondary->SetPoolDefaultItem(rItem);
    else
    {
        SFX_ASSERT( 0, rItem.Which(), "unknown Which-Id - cannot set pool default" );
    }
}

/*
 * Resets the default of the given <Which-Id> back to the static default.
 * If a pool default exists it is removed.
 */
void SfxItemPool::ResetPoolDefaultItem( sal_uInt16 nWhichId )
{
    DBG_CHKTHIS(SfxItemPool, 0);
    if ( IsInRange(nWhichId) )
    {
        SfxPoolItem **ppOldDefault =
            ppPoolDefaults + GetIndex_Impl( nWhichId );
        if ( *ppOldDefault )
        {
            (*ppOldDefault)->SetRefCount(0);
            DELETEZ( *ppOldDefault );
        }
    }
    else if ( pSecondary )
        pSecondary->ResetPoolDefaultItem(nWhichId);
    else
    {
        SFX_ASSERT( 0, nWhichId, "unknown Which-Id - cannot set pool default" );
    }
}

// -----------------------------------------------------------------------

const SfxPoolItem& SfxItemPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                0 != &((const SfxSetItem&)rItem).GetItemSet(),
                "SetItem without ItemSet" );

    DBG_CHKTHIS(SfxItemPool, 0);
    if ( 0 == nWhich )
        nWhich = rItem.Which();

    // richtigen Secondary-Pool finden
    sal_Bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pSecondary )
            return pSecondary->Put( rItem, nWhich );
        DBG_ERROR( "unknown Which-Id - cannot put item" );
    }

    // SID oder nicht poolable (neue Definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( USHRT_MAX == nIndex ||
         IsItemFlag_Impl( nIndex, SFX_ITEM_NOT_POOLABLE ) )
    {
        SFX_ASSERT( USHRT_MAX != nIndex || rItem.Which() != nWhich ||
                    !IsDefaultItem(&rItem) || rItem.GetKind() == SFX_ITEMS_DELETEONIDLE,
                    nWhich, "ein nicht Pool-Item ist Default?!" );
        SfxPoolItem *pPoolItem = rItem.Clone(pMaster);
        pPoolItem->SetWhich(nWhich);
        AddRef( *pPoolItem );
        return *pPoolItem;
    }

    SFX_ASSERT( rItem.IsA(GetDefaultItem(nWhich).Type()), nWhich,
                "SFxItemPool: wrong item type in Put" );

    SfxPoolItemArray_Impl** ppItemArr = pImp->ppPoolItems + nIndex;
    if( !*ppItemArr )
        *ppItemArr = new SfxPoolItemArray_Impl;

    SfxPoolItemArrayBase_Impl::iterator ppFree;
    sal_Bool ppFreeIsSet = sal_False;
    SfxPoolItemArrayBase_Impl::iterator ppHtArray = (*ppItemArr)->begin();
    if ( IsItemFlag_Impl( nIndex, SFX_ITEM_POOLABLE ) )
    {
        // wenn es ueberhaupt gepoolt ist, koennte es schon drin sein
        if ( IsPooledItem(&rItem) )
        {
            // 1. Schleife: teste ob der Pointer vorhanden ist.
            for( size_t n = (*ppItemArr)->size(); n; ++ppHtArray, --n )
                if( &rItem == (*ppHtArray) )
                {
                    AddRef( **ppHtArray );
                    return **ppHtArray;
                }
        }

        // 2. Schleife: dann muessen eben die Attribute verglichen werden
        size_t n;
        for ( n = (*ppItemArr)->size(), ppHtArray = (*ppItemArr)->begin();
              n; ++ppHtArray, --n )
        {
            if ( *ppHtArray )
            {
                if( **ppHtArray == rItem )
                {
                    AddRef( **ppHtArray );
                    return **ppHtArray;
                }
            }
            else
                if ( ppFreeIsSet == sal_False )
                {
                    ppFree = ppHtArray;
                    ppFreeIsSet = sal_True;
                }
        }
    }
    else
    {
        // freien Platz suchen
        SfxPoolItemArrayBase_Impl::iterator ppHtArr;
        size_t n, nCount = (*ppItemArr)->size();
        for ( n = (*ppItemArr)->nFirstFree,
                  ppHtArr = (*ppItemArr)->begin() + n;
              n < nCount;
              ++ppHtArr, ++n )
            if ( !*ppHtArr )
            {
                ppFree = ppHtArr;
                ppFreeIsSet = sal_True;
                break;
            }

        // naechstmoeglichen freien Platz merken
        (*ppItemArr)->nFirstFree = n;
    }

    // nicht vorhanden, also im PtrArray eintragen
    SfxPoolItem* pNewItem = rItem.Clone(pMaster);
    pNewItem->SetWhich(nWhich);
#ifdef DBG_UTIL
    SFX_ASSERT( rItem.Type() == pNewItem->Type(), nWhich, "unequal types in Put(): no Clone()?" )
#ifdef TF_POOLABLE
    if ( !rItem.ISA(SfxSetItem) )
    {
        SFX_ASSERT( !IsItemFlag(nWhich, SFX_ITEM_POOLABLE) ||
                    rItem == *pNewItem,
                    nWhich, "unequal items in Put(): no operator==?" );
        SFX_ASSERT( !IsItemFlag(*pNewItem, SFX_ITEM_POOLABLE) ||
                    *pNewItem == rItem,
                    nWhich, "unequal items in Put(): no operator==?" );
    }
#endif
#endif
    AddRef( *pNewItem, pImp->nInitRefCount );
    SfxPoolItem* pTemp = pNewItem;
    if ( ppFreeIsSet == sal_False )
        (*ppItemArr)->push_back( pTemp );
    else
    {
        DBG_ASSERT( *ppFree == 0, "using surrogate in use" );
        *ppFree = pNewItem;
    }
    return *pNewItem;
}

// -----------------------------------------------------------------------

void SfxItemPool::Remove( const SfxPoolItem& rItem )
{
    DBG_CHKTHIS(SfxItemPool, 0);

    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                0 != &((const SfxSetItem&)rItem).GetItemSet(),
                "SetItem without ItemSet" );

    SFX_ASSERT( !IsPoolDefaultItem(&rItem), rItem.Which(),
                "wo kommt denn hier ein Pool-Default her" );

    // richtigen Secondary-Pool finden
    const sal_uInt16 nWhich = rItem.Which();
    sal_Bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pSecondary )
        {
            pSecondary->Remove( rItem );
            return;
        }
        DBG_ERROR( "unknown Which-Id - cannot remove item" );
    }

    // SID oder nicht poolable (neue Definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( bSID || IsItemFlag_Impl( nIndex, SFX_ITEM_NOT_POOLABLE ) )
    {
        SFX_ASSERT( USHRT_MAX != nIndex ||
                    !IsDefaultItem(&rItem), rItem.Which(),
                    "ein nicht Pool-Item ist Default?!" );
        if ( 0 == ReleaseRef(rItem) )
        {
            SfxPoolItem *pItem = &(SfxPoolItem &)rItem;
            delete pItem;
        }
        return;
    }

    SFX_ASSERT( rItem.GetRefCount(), rItem.Which(), "RefCount == 0, Remove unmoeglich" );

    // statische Defaults sind eben einfach da
    if ( rItem.GetKind() == SFX_ITEMS_STATICDEFAULT &&
         &rItem == *( ppStaticDefaults + GetIndex_Impl(nWhich) ) )
        return;

    // Item im eigenen Pool suchen
    SfxPoolItemArray_Impl** ppItemArr = (pImp->ppPoolItems + nIndex);
    SFX_ASSERT( *ppItemArr, rItem.Which(), "removing Item not in Pool" );
    SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*ppItemArr)->begin();
    for( size_t n = (*ppItemArr)->size(); n; ++ppHtArr, --n )
        if( *ppHtArr == &rItem )
        {
            if ( (*ppHtArr)->GetRefCount() ) //!
                ReleaseRef( **ppHtArr );
            else
            {
                SFX_ASSERT( 0, rItem.Which(), "removing Item without ref" );
                SFX_TRACE( "to be removed, but not no refs: ", *ppHtArr );
            }

            // ggf. kleinstmoegliche freie Position merken
            size_t nPos = (*ppItemArr)->size() - n;
            if ( (*ppItemArr)->nFirstFree > nPos )
                (*ppItemArr)->nFirstFree = nPos;

            //! MI: Hack, solange wir das Problem mit dem Outliner haben
            //! siehe anderes MI-REF
            if ( 0 == (*ppHtArr)->GetRefCount() && nWhich < 4000 )
                DELETEZ(*ppHtArr);
            return;
        }

    // nicht vorhanden
    SFX_ASSERT( 0, rItem.Which(), "removing Item not in Pool" );
    SFX_TRACE( "to be removed, but not in pool: ", &rItem );
}

// -----------------------------------------------------------------------

const SfxPoolItem& SfxItemPool::GetDefaultItem( sal_uInt16 nWhich ) const
{
    DBG_CHKTHIS(SfxItemPool, 0);

    if ( !IsInRange(nWhich) )
    {
        if ( pSecondary )
            return pSecondary->GetDefaultItem( nWhich );
        SFX_ASSERT( 0, nWhich, "unknown which - dont ask me for defaults" );
    }

    DBG_ASSERT( ppStaticDefaults, "no defaults known - dont ask me for defaults" );
    sal_uInt16 nPos = GetIndex_Impl(nWhich);
    SfxPoolItem *pDefault = *(ppPoolDefaults + nPos);
    if ( pDefault )
        return *pDefault;
    return **(ppStaticDefaults + nPos);
}

// -----------------------------------------------------------------------


void SfxItemPool::FreezeIdRanges()

/*  [Beschreibung]

    This method should be called at the master pool, when all secondary
    pools are appended to it.

    It calculates the ranges of 'which-ids' for fast construction of
    item-sets, which contains all 'which-ids'.
*/

{
    FillItemIdRanges_Impl( _pPoolRanges );
}


// -----------------------------------------------------------------------

void SfxItemPool::FillItemIdRanges_Impl( sal_uInt16*& pWhichRanges ) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT( !_pPoolRanges, "GetFrozenRanges() would be faster!" );

    const SfxItemPool *pPool;
    sal_uInt16 nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pSecondary )
        ++nLevel;

    pWhichRanges = new sal_uInt16[ 2*nLevel + 1 ];

    nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pSecondary )
    {
        *(pWhichRanges+(nLevel++)) = pPool->nStart;
        *(pWhichRanges+(nLevel++)) = pPool->nEnd;
        *(pWhichRanges+nLevel) = 0;
    }
}

// -----------------------------------------------------------------------

const SfxPoolItem *SfxItemPool::GetItem2(sal_uInt16 nWhich, sal_uInt32 nOfst) const
{
    DBG_CHKTHIS(SfxItemPool, 0);

    if ( !IsInRange(nWhich) )
    {
        if ( pSecondary )
            return pSecondary->GetItem2( nWhich, nOfst );
        SFX_ASSERT( 0, nWhich, "unknown Which-Id - cannot resolve surrogate" );
        return 0;
    }

    // dflt-Attribut?
    if ( nOfst == SFX_ITEMS_DEFAULT )
        return *(ppStaticDefaults + GetIndex_Impl(nWhich));

    SfxPoolItemArray_Impl* pItemArr = *(pImp->ppPoolItems + GetIndex_Impl(nWhich));
    if( pItemArr && nOfst < pItemArr->size() )
        return (*pItemArr)[nOfst];

    return 0;
}

// -----------------------------------------------------------------------

sal_uInt32 SfxItemPool::GetItemCount2(sal_uInt16 nWhich) const
{
    DBG_CHKTHIS(SfxItemPool, 0);

    if ( !IsInRange(nWhich) )
    {
        if ( pSecondary )
            return pSecondary->GetItemCount2( nWhich );
        SFX_ASSERT( 0, nWhich, "unknown Which-Id - cannot resolve surrogate" );
        return 0;
    }

    SfxPoolItemArray_Impl* pItemArr = *(pImp->ppPoolItems + GetIndex_Impl(nWhich));
    if  ( pItemArr )
        return pItemArr->size();
    return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxItemPool::GetWhich( sal_uInt16 nSlotId, sal_Bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return nSlotId;

#ifdef TF_POOLABLE
    sal_uInt16 nCount = nEnd - nStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + nStart;
#else
    if ( pSlotIds )
    {
        sal_uInt16 nCount = nEnd - nStart + 1;
        for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
            if ( pSlotIds[nOfs] == nSlotId )
                return nOfs + nStart;
    }
#endif
    if ( pSecondary && bDeep )
        return pSecondary->GetWhich(nSlotId);
    return nSlotId;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxItemPool::GetSlotId( sal_uInt16 nWhich, sal_Bool bDeep ) const
{
    if ( !IsWhich(nWhich) )
        return nWhich;

    if ( !IsInRange( nWhich ) )
    {
        if ( pSecondary && bDeep )
            return pSecondary->GetSlotId(nWhich);
        SFX_ASSERT( 0, nWhich, "unknown Which-Id - cannot get slot-id" );
        return 0;
    }
#ifdef TF_POOLABLE

    sal_uInt16 nSID = pItemInfos[nWhich - nStart]._nSID;
    return nSID ? nSID : nWhich;
#else
    else if ( pSlotIds )
        return pSlotIds[nWhich - nStart];
    return nWhich;
#endif
}

// -----------------------------------------------------------------------

sal_uInt16 SfxItemPool::GetTrueWhich( sal_uInt16 nSlotId, sal_Bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return 0;

#ifdef TF_POOLABLE
    sal_uInt16 nCount = nEnd - nStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + nStart;
#else
    if ( pSlotIds )
    {
        sal_uInt16 nCount = nEnd - nStart + 1;
        for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
            if ( pSlotIds[nOfs] == nSlotId )
                return nOfs + nStart;
    }
#endif
    if ( pSecondary && bDeep )
        return pSecondary->GetTrueWhich(nSlotId);
    return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich, sal_Bool bDeep ) const
{
    if ( !IsWhich(nWhich) )
        return 0;

    if ( !IsInRange( nWhich ) )
    {
        if ( pSecondary && bDeep )
            return pSecondary->GetTrueSlotId(nWhich);
        SFX_ASSERT( 0, nWhich, "unknown Which-Id - cannot get slot-id" );
        return 0;
    }
#ifdef TF_POOLABLE
    return pItemInfos[nWhich - nStart]._nSID;
#else
    else if ( pSlotIds )
        return pSlotIds[nWhich - nStart];
    else
        return 0;
#endif
}
// -----------------------------------------------------------------------
void SfxItemPool::SetFileFormatVersion( sal_uInt16 nFileFormatVersion )

/*  [Description]

    You must call this function to set the file format version after
    concatenating your secondary-pools but before you store any
    pool, itemset or item. Only set the version at the master pool,
    never at any secondary pool.
*/

{
    DBG_ASSERT( this == pMaster,
                "SfxItemPool::SetFileFormatVersion() but not a master pool" );
    for ( SfxItemPool *pPool = this; pPool; pPool = pPool->pSecondary )
        pPool->_nFileFormatVersion = nFileFormatVersion;
}


