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
#include <stdio.h>

#include <sal/log.hxx>
#include <tools/solar.h>
#include <svl/itempool.hxx>
#include "whassert.hxx"
#include <svl/brdcst.hxx>
#include <svl/filerec.hxx>
#include "poolio.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPool);


const SfxItemPool* SfxItemPool::GetStoringPool()

/*  [Beschreibung]

    Diese Methode liefert den <SfxItemPool>, der gerade gespeichert wird.
    Sie sollte nur in Notf"allen verwendet werden, um z.B. File-Format-
    Kompatibilit"at zu gew"ahrleisten o."o. - z.B. in der "uberladung eines
    <SfxPoolItem::Store()> zus"atzliche Daten aus dem dazuge"horigen
    Pool zu besorgen.
*/

{
    return pStoringPool_;
}


SvStream &SfxItemPool::Store(SvStream &rStream) const

/*  [Beschreibung]

    Der SfxItemPool wird inklusive aller seiner Sekund"arpools mit
    Pool-Defaults und gepoolten Items in dem angegebenen Stream gespeichert.
    Die statischen Defaults werden nicht gespeichert.


    [Fileformat]

    ;zun"achst ein Kompatiblit"ats-Header-Block
    Start:      0x1111  SFX_ITEMPOOL_TAG_STARTPOOLS(_4/_5)
                sal_uInt8   MAJOR_VER                   ;SfxItemPool-Version
                sal_uInt8   MINOR_VER                   ;"
                0xFFFF  SFX_ITEMPOOL_TAG_TRICK4OLD  ;ex. GetVersion()
                sal_uInt16  0x0000                      ;Pseudo-StyleSheetPool
                sal_uInt16  0x0000                      ;Pseudo-StyleSheetPool

    ;den ganzen Pool in einen Record
                record  SfxMiniRecod(SFX_ITEMPOOL_REC)

    ;je ein Header vorweg
    Header:     record      SfxMiniRecord(SFX_ITEMPOOL_REC_HEADER)
                sal_uInt16          GetVersion()            ;Which-Ranges etc.
                String          GetName()               ;Pool-Name

    ;die Versions-Map, um WhichIds neuer File-Versionen mappen zu k"onnen
    Versions:   record      SfxMultiRecord(SFX_ITEMPOOL_REC_VERSIONS, 0)
                sal_uInt16          OldVersion
                sal_uInt16          OldStartWhich
                sal_uInt16          OldEndWhich
                sal_uInt16[]        NewWhich (OldEndWhich-OldStartWhich+1)

    ;jetzt die gepoolten Items (zuerst nicht-SfxSetItems)
    Items:      record      SfxMultiRecord(SFX_ITEMPOOL_REC_WHICHIDS, 0)
                content         SlotId, 0
                sal_uInt16          WhichId
                sal_uInt16          pItem->GetVersion()
                sal_uInt16          Array-Size
                record          SfxMultiRecord(SFX_, 0)
                content             Surrogate
                sal_uInt16              RefCount
                unknown             pItem->Store()

    ;jetzt die gesetzten Pool-Defaults
    Defaults:   record      SfxMultiRecord(SFX_ITEMPOOL_REC_DEFAULTS, 0)
                content         SlotId, 0
                sal_uInt16          WhichId
                sal_uInt16          pPoolDef->GetVersion()
                unknown         pPoolDef->Store();

    ;dahinter folgt ggf. der Secondary ohne Kompatiblit"ats-Header-Block
*/

{
    DBG_CHKTHIS(SfxItemPool, 0);

    // Store-Master finden
    SfxItemPool *pStoreMaster = pImp->mpMaster != this ? pImp->mpMaster : 0;
    while ( pStoreMaster && !pStoreMaster->pImp->bStreaming )
        pStoreMaster = pStoreMaster->pImp->mpSecondary;

    // Alter-Header (Version des Pools an sich und Inhalts-Version 0xffff)
    pImp->bStreaming = sal_True;
    if ( !pStoreMaster )
    {
        rStream << ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_50
                ? SFX_ITEMPOOL_TAG_STARTPOOL_5
                : SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream << SFX_ITEMPOOL_VER_MAJOR << SFX_ITEMPOOL_VER_MINOR;
        rStream << SFX_ITEMPOOL_TAG_TRICK4OLD;

        // SfxStyleSheet-Bug umgehen
        rStream << sal_uInt16(0); // Version
        rStream << sal_uInt16(0); // Count (2. Schleife f"allt sonst auf die Fresse)
    }

    // jeder Pool ist als ganzes ein Record
    SfxMiniRecordWriter aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    pStoringPool_ = this;

    // Einzel-Header (Version des Inhalts und Name)
    {
        SfxMiniRecordWriter aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER);
        rStream << pImp->nVersion;
        SfxPoolItem::writeByteString(rStream, pImp->aName);
    }

    // Version-Maps
    {
        SfxMultiVarRecordWriter aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP, 0 );
        for ( size_t nVerNo = 0; nVerNo < pImp->aVersions.size(); ++nVerNo )
        {
            aVerRec.NewContent();
            SfxPoolVersion_ImplPtr pVer = pImp->aVersions[nVerNo];
            rStream << pVer->_nVer << pVer->_nStart << pVer->_nEnd;
            sal_uInt16 nCount = pVer->_nEnd - pVer->_nStart + 1;
            sal_uInt16 nNewWhich = 0;
            for ( sal_uInt16 n = 0; n < nCount; ++n )
            {
                nNewWhich = pVer->_pMap[n];
                rStream << nNewWhich;
            }

            // Workaround gegen Bug in SetVersionMap der 312
            if ( SOFFICE_FILEFORMAT_31 == pImp->mnFileFormatVersion )
                rStream << sal_uInt16(nNewWhich+1);
        }
    }

    // gepoolte Items
    {
        SfxMultiMixRecordWriter aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS, 0 );

        // erst Atomaren-Items und dann die Sets schreiben (wichtig beim Laden)
        for (int ft = 0 ; ft < 2 && !rStream.GetError(); ft++)
        {
            pImp->bInSetItem = ft != 0;

            std::vector<SfxPoolItemArray_Impl*>::iterator itrArr = pImp->maPoolItems.begin();
            SfxPoolItem **ppDefItem = pImp->ppStaticDefaults;
            const sal_uInt16 nSize = GetSize_Impl();
            for ( size_t i = 0; i < nSize && !rStream.GetError(); ++i, ++itrArr, ++ppDefItem )
            {
                // Version des Items feststellen
                sal_uInt16 nItemVersion = (*ppDefItem)->GetVersion( pImp->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    // => kam in zu exportierender Version gar nicht vor
                    continue;

                // !poolable wird gar nicht im Pool gespeichert
                // und itemsets/plain-items je nach Runde
                if ( *itrArr && IsItemFlag(**ppDefItem, SFX_ITEM_POOLABLE) &&
                     pImp->bInSetItem == (bool) (*ppDefItem)->ISA(SfxSetItem) )
                {
                    // eigene Kennung, globale Which-Id und Item-Version
                    sal_uInt16 nSlotId = GetSlotId( (*ppDefItem)->Which(), sal_False );
                    aWhichIdsRec.NewContent(nSlotId, 0);
                    rStream << (*ppDefItem)->Which();
                    rStream << nItemVersion;
                    const sal_uInt32 nCount = ::std::min<size_t>( (*itrArr)->size(), SAL_MAX_UINT32 );
                    DBG_ASSERT(nCount, "ItemArr is empty");
                    rStream << nCount;

                    // Items an sich schreiben
                    SfxMultiMixRecordWriter aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS, 0 );
                    for ( size_t j = 0; j < nCount; ++j )
                    {
                        // Item selbst besorgen
                        const SfxPoolItem *pItem = (*itrArr)->operator[](j);
                        if ( pItem && pItem->GetRefCount() ) //! siehe anderes MI-REF
                        {
                            aItemsRec.NewContent((sal_uInt16)j, 'X' );

                            if ( pItem->GetRefCount() == SFX_ITEMS_SPECIAL )
                                rStream << (sal_uInt16) pItem->GetKind();
                            else
                            {
                                rStream << (sal_uInt16) pItem->GetRefCount();
                                if( pItem->GetRefCount() > SFX_ITEMS_OLD_MAXREF )
                                    rStream.SetError( ERRCODE_IO_NOTSTORABLEINBINARYFORMAT );
                            }

                            if ( !rStream.GetError() )
                                pItem->Store(rStream, nItemVersion);
                            else
                                break;
#ifdef DBG_UTIL_MI
                            if ( !pItem->ISA(SfxSetItem) )
                            {
                                sal_uLong nMark = rStream.Tell();
                                rStream.Seek( nItemStartPos + sizeof(sal_uInt16) );
                                SfxPoolItem *pClone = pItem->Create(rStream, nItemVersion );
                                sal_uInt16 nWh = pItem->Which();
                                SFX_ASSERT( rStream.Tell() == nMark, nWh,"asymmetric store/create" );
                                SFX_ASSERT( *pClone == *pItem, nWh, "unequal after store/create" );
                                delete pClone;
                            }
#endif
                        }
                    }
                }
            }
        }

        pImp->bInSetItem = false;
    }

    // die gesetzten Defaults speichern (Pool-Defaults)
    if ( !rStream.GetError() )
    {
        SfxMultiMixRecordWriter aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS, 0 );
        sal_uInt16 nCount = GetSize_Impl();
        for ( sal_uInt16 n = 0; n < nCount; ++n )
        {
            const SfxPoolItem* pDefaultItem = pImp->ppPoolDefaults[n];
            if ( pDefaultItem )
            {
                // Version ermitteln
                sal_uInt16 nItemVersion = pDefaultItem->GetVersion( pImp->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    // => gab es in der Version noch nicht
                    continue;

                // eigene Kennung, globale Kennung, Version
                sal_uInt16 nSlotId = GetSlotId( pDefaultItem->Which(), sal_False );
                aDefsRec.NewContent( nSlotId, 0 );
                rStream << pDefaultItem->Which();
                rStream << nItemVersion;

                // Item an sich
                pDefaultItem->Store( rStream, nItemVersion );
            }
        }
    }

    // weitere Pools rausschreiben
    pStoringPool_ = 0;
    aPoolRec.Close();
    if ( !rStream.GetError() && pImp->mpSecondary )
        pImp->mpSecondary->Store( rStream );

    pImp->bStreaming = sal_False;
    return rStream;
}

bool SfxItemPool::HasPersistentRefCounts() const
{
    return pImp->mbPersistentRefCounts;
}

void SfxItemPool::LoadCompleted()

/*  [Beschreibung]

    Wurde der SfxItemPool mit 'bRefCounts' == sal_False geladen, mu\s das
    Laden der Dokumentinhalte mit einem Aufruf dieser Methode beendet
    werden. Ansonsten hat der Aufruf dieser Methode keine Funktion.


    [Anmerkung]

    Beim Laden ohne Ref-Counts werden diese tats"achlich auf 1 gesetzt,
    damit nicht w"ahrend des Ladevorgangs SfxPoolItems gel"oscht werden,
    die danach, aber auch noch beim Ladevorgang, ben"otigt werden. Diese
    Methode setzt den Ref-Count wieder zur"uck und entfernt dabei
    gleichzeitig alle nicht mehr ben"otigten Items.


    [Querverweise]

    <SfxItemPool::Load()>
*/

{
    // wurden keine Ref-Counts mitgeladen?
    if ( pImp->nInitRefCount > 1 )
    {

        // "uber alle Which-Werte iterieren
        std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
        for( sal_uInt16 nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++itrItemArr )
        {
            // ist "uberhaupt ein Item mit dem Which-Wert da?
            if ( *itrItemArr )
            {
                // "uber alle Items mit dieser Which-Id iterieren
                SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*itrItemArr)->begin();
                for( size_t n = (*itrItemArr)->size(); n; --n, ++ppHtArr )
                    if (*ppHtArr)
                    {
                        #ifdef DBG_UTIL
                        const SfxPoolItem &rItem = **ppHtArr;
                        DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                                    0 != &((const SfxSetItem&)rItem).GetItemSet(),
                                    "SetItem without ItemSet" );
                        #endif

                        if ( !ReleaseRef( **ppHtArr, 1 ) )
                            DELETEZ( *ppHtArr );
                    }
            }
        }

        // from now on normal initial ref count
        pImp->nInitRefCount = 1;
    }

    // notify secondary pool
    if ( pImp->mpSecondary )
        pImp->mpSecondary->LoadCompleted();
}

sal_uInt16 SfxItemPool::GetFirstWhich() const
{
    return pImp->mnStart;
}

sal_uInt16 SfxItemPool::GetLastWhich() const
{
    return pImp->mnEnd;
}

bool SfxItemPool::IsInRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImp->mnStart && nWhich <= pImp->mnEnd;
}

// This had to be moved to a method of its own to keep Solaris GCC happy:
void SfxItemPool_Impl::readTheItems (
    SvStream & rStream, sal_uInt32 nItemCount, sal_uInt16 nVer,
    SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** ppArr)
{
    SfxMultiRecordReader aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS );

    SfxPoolItemArray_Impl *pNewArr = new SfxPoolItemArray_Impl();
    SfxPoolItem *pItem = 0;

    sal_uLong n, nLastSurrogate = sal_uLong(-1);
    while (aItemsRec.GetContent())
    {
        // n"achstes Surrogat holen
        sal_uInt16 nSurrogate = aItemsRec.GetContentTag();
        DBG_ASSERT( aItemsRec.GetContentVersion() == 'X',
                    "not an item content" );

        // fehlende auff"ullen
        for ( pItem = 0, n = nLastSurrogate+1; n < nSurrogate; ++n )
            pNewArr->push_back( (SfxPoolItem*) pItem );
        nLastSurrogate = nSurrogate;

        // Ref-Count und Item laden
        sal_uInt16 nRef(0);
        rStream >> nRef;

        pItem = pDefItem->Create(rStream, nVer);
        pNewArr->push_back( (SfxPoolItem*) pItem );

        if ( !mbPersistentRefCounts )
            // bis <SfxItemPool::LoadCompleted()> festhalten
            SfxItemPool::AddRef(*pItem, 1);
        else
        {
            if ( nRef > SFX_ITEMS_OLD_MAXREF )
                SfxItemPool::SetKind(*pItem, nRef);
            else
                SfxItemPool::AddRef(*pItem, nRef);
        }
    }

    // fehlende auff"ullen
    for ( pItem = 0, n = nLastSurrogate+1; n < nItemCount; ++n )
        pNewArr->push_back( (SfxPoolItem*) pItem );

    SfxPoolItemArray_Impl *pOldArr = *ppArr;
    *ppArr = pNewArr;

    // die Items merken, die schon im Pool sind
    bool bEmpty = true;
    if ( 0 != pOldArr )
        for ( n = 0; bEmpty && n < pOldArr->size(); ++n )
            bEmpty = pOldArr->operator[](n) == 0;
    DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
    if ( !bEmpty )
    {
        // f"ur alle alten suchen, ob ein gleiches neues existiert
        for ( size_t nOld = 0; nOld < pOldArr->size(); ++nOld )
        {
            SfxPoolItem *pOldItem = (*pOldArr)[nOld];
            if ( pOldItem )
            {
                sal_uInt32 nFree = SAL_MAX_UINT32;
                bool bFound = false;
                for ( size_t nNew = (*ppArr)->size(); nNew--; )
                {
                    // geladenes Item
                    SfxPoolItem *&rpNewItem =
                        (SfxPoolItem*&)(*ppArr)->operator[](nNew);

                    // surrogat unbenutzt?
                    if ( !rpNewItem )
                        nFree = nNew;

                    // gefunden?
                    else if ( *rpNewItem == *pOldItem )
                    {
                        // wiederverwenden
                        SfxItemPool::AddRef( *pOldItem, rpNewItem->GetRefCount() );
                        SfxItemPool::SetRefCount( *rpNewItem, 0 );
                        delete rpNewItem;
                        rpNewItem = pOldItem;
                        bFound = true;
                        break;
                    }
                }

                // vorhervorhandene, nicht geladene uebernehmen
                if ( !bFound )
                {
                    if ( nFree != SAL_MAX_UINT32 )
                        (SfxPoolItem*&)(*ppArr)->operator[](nFree) = pOldItem;
                    else
                        (*ppArr)->push_back( (SfxPoolItem*) pOldItem );
                }
            }
        }
    }
    delete pOldArr;
}

// -----------------------------------------------------------------------

SvStream &SfxItemPool::Load(SvStream &rStream)
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(pImp->ppStaticDefaults, "kein DefaultArray");

    // protect items by increasing ref count
    if ( !pImp->mbPersistentRefCounts )
    {

        // "uber alle Which-Werte iterieren
        std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
        for( size_t nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++itrItemArr )
        {
            // ist "uberhaupt ein Item mit dem Which-Wert da?
            if ( *itrItemArr )
            {
                // "uber alle Items mit dieser Which-Id iterieren
                SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*itrItemArr)->begin();
                for( size_t n = (*itrItemArr)->size(); n; --n, ++ppHtArr )
                    if (*ppHtArr)
                    {
                        #ifdef DBG_UTIL
                        const SfxPoolItem &rItem = **ppHtArr;
                        DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                                    0 != &((const SfxSetItem&)rItem).GetItemSet(),
                                    "SetItem without ItemSet" );
                        DBG_WARNING( "loading non-empty ItemPool" );
                        #endif

                        AddRef( **ppHtArr, 1 );
                    }
            }
        }

        // during loading (until LoadCompleted()) protect all items
        pImp->nInitRefCount = 2;
    }

    // Load-Master finden
    SfxItemPool *pLoadMaster = pImp->mpMaster != this ? pImp->mpMaster : 0;
    while ( pLoadMaster && !pLoadMaster->pImp->bStreaming )
        pLoadMaster = pLoadMaster->pImp->mpSecondary;

    // Gesamt Header einlesen
    pImp->bStreaming = sal_True;
    if ( !pLoadMaster )
    {
        // Format-Version laden
        CHECK_FILEFORMAT2( rStream,
                SFX_ITEMPOOL_TAG_STARTPOOL_5, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream >> pImp->nMajorVer >> pImp->nMinorVer;

        // Format-Version in Master-Pool "ubertragen
        pImp->mpMaster->pImp->nMajorVer = pImp->nMajorVer;
        pImp->mpMaster->pImp->nMinorVer = pImp->nMinorVer;

        // altes Format?
        if ( pImp->nMajorVer < 2 )
            // pImp->bStreaming wird von Load1_Impl() zur"uckgesetzt
            return Load1_Impl( rStream );

        // zu neues Format?
        if ( pImp->nMajorVer > SFX_ITEMPOOL_VER_MAJOR )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImp->bStreaming = sal_False;
            return rStream;
        }

        // Version 1.2-Trick-Daten "uberspringen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_TRICK4OLD );
        rStream.SeekRel( 4 ); // Hack-Daten wegen SfxStyleSheetPool-Bug  skippen
    }

    // neues Record-orientiertes Format
    SfxMiniRecordReader aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    if ( rStream.GetError() )
    {
        pImp->bStreaming = sal_False;
        return rStream;
    }

    // Einzel-Header
    int bOwnPool = sal_True;
    OUString aExternName;
    {
        // Header-Record suchen
        SfxMiniRecordReader aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = sal_False;
            return rStream;
        }

        // Header-lesen
        rStream >> pImp->nLoadingVersion;
        aExternName = SfxPoolItem::readByteString(rStream);
        bOwnPool = aExternName == pImp->aName;

        //! solange wir keine fremden Pools laden k"onnen
        if ( !bOwnPool )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            aPoolRec.Skip();
            pImp->bStreaming = sal_False;
            return rStream;
        }
    }

    // Version-Maps
    {
        SfxMultiRecordReader aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = sal_False;
            return rStream;
        }

        // Versions-Maps einlesen
        sal_uInt16 nOwnVersion = pImp->nVersion;
        for ( sal_uInt16 nVerNo = 0; aVerRec.GetContent(); ++nVerNo )
        {
            // Header f"ur einzelne Version einlesen
            sal_uInt16 nVersion(0), nHStart(0), nHEnd(0);
            rStream >> nVersion >> nHStart >> nHEnd;
            sal_uInt16 nCount = nHEnd - nHStart + 1;

            // Is new version is known?
            if ( nVerNo >= pImp->aVersions.size() )
            {
                // Add new Version
                sal_uInt16 *pMap = new sal_uInt16[nCount];
                memset(pMap, 0, nCount * sizeof(sal_uInt16));
                for ( sal_uInt16 n = 0; n < nCount; ++n )
                    rStream >> pMap[n];
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
        }
        pImp->nVersion = nOwnVersion;
    }

    // Items laden
    bool bSecondaryLoaded = false;
    long nSecondaryEnd = 0;
    {
        SfxMultiRecordReader aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS);
        while ( aWhichIdsRec.GetContent() )
        {
            // SlotId, Which-Id und Item-Version besorgen
            sal_uInt32 nCount(0);
            sal_uInt16 nVersion(0), nWhich(0);
            //!sal_uInt16 nSlotId = aWhichIdsRec.GetContentTag();
            rStream >> nWhich;
            if ( pImp->nLoadingVersion != pImp->nVersion )
                // Which-Id aus File-Version in Pool-Version verschieben
                nWhich = GetNewWhich( nWhich );

            // unbekanntes Item aus neuerer Version
            if ( !IsInRange(nWhich) )
                continue;

            rStream >> nVersion;
            rStream >> nCount;
            //!SFX_ASSERTWARNING( !nSlotId || !HasMap() ||
            //!         ( nSlotId == GetSlotId( nWhich, sal_False ) ) ||
            //!         !GetSlotId( nWhich, sal_False ),
            //!         nWhich, "Slot/Which mismatch" );

            sal_uInt16 nIndex = GetIndex_Impl(nWhich);
            SfxPoolItemArray_Impl **ppArr = &pImp->maPoolItems[0] + nIndex;

            // SfxSetItems k"onnten Items aus Sekund"arpools beinhalten
            SfxPoolItem *pDefItem = *(pImp->ppStaticDefaults + nIndex);
            pImp->bInSetItem = pDefItem->ISA(SfxSetItem);
            if ( !bSecondaryLoaded && pImp->mpSecondary && pImp->bInSetItem )
            {
                // an das Ende des eigenen Pools seeken
                sal_uLong nLastPos = rStream.Tell();
                aPoolRec.Skip();

                // Sekund"arpool einlesen
                pImp->mpSecondary->Load( rStream );
                bSecondaryLoaded = true;
                nSecondaryEnd = rStream.Tell();

                // zur"uck zu unseren eigenen Items
                rStream.Seek(nLastPos);
            }

            // Items an sich lesen
            pImp->readTheItems(rStream, nCount, nVersion, pDefItem, ppArr);

            pImp->bInSetItem = false;
        }
    }

    // Pool-Defaults lesen
    {
        SfxMultiRecordReader aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS );

        while ( aDefsRec.GetContent() )
        {
            // SlotId, Which-Id und Item-Version besorgen
            sal_uInt16 nVersion(0), nWhich(0);
            //!sal_uInt16 nSlotId = aDefsRec.GetContentTag();
            rStream >> nWhich;
            if ( pImp->nLoadingVersion != pImp->nVersion )
                // Which-Id aus File-Version in Pool-Version verschieben
                nWhich = GetNewWhich( nWhich );

            // unbekanntes Item aus neuerer Version
            if ( !IsInRange(nWhich) )
                continue;

            rStream >> nVersion;
            //!SFX_ASSERTWARNING( !HasMap() || ( nSlotId == GetSlotId( nWhich, sal_False ) ),
            //!         nWhich, "Slot/Which mismatch" );

            // Pool-Default-Item selbst laden
            SfxPoolItem *pItem =
                    ( *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
                    ->Create( rStream, nVersion );
            pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
            *( pImp->ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;
        }
    }

    // ggf. Secondary-Pool laden
    aPoolRec.Skip();
    if ( pImp->mpSecondary )
    {
        if ( !bSecondaryLoaded )
            pImp->mpSecondary->Load( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    // wenn nicht own-Pool, dann kein Name
    if ( aExternName != pImp->aName )
        pImp->aName = OUString();

    pImp->bStreaming = sal_False;
    return rStream;
};

sal_uInt16 SfxItemPool::GetIndex_Impl(sal_uInt16 nWhich) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(nWhich >= pImp->mnStart && nWhich <= pImp->mnEnd, "Which-Id nicht im Pool-Bereich");
    return nWhich - pImp->mnStart;
}

sal_uInt16 SfxItemPool::GetSize_Impl() const
{
    return pImp->mnEnd - pImp->mnStart + 1;
}

SvStream &SfxItemPool::Load1_Impl(SvStream &rStream)
{
    // beim Master ist der Header schon von <Load()> geladen worden
    if ( !pImp->bStreaming )
    {
        // Header des Secondary lesen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream >> pImp->nMajorVer >> pImp->nMinorVer;
    }
    sal_uInt32 nAttribSize(0);
    int bOwnPool = sal_True;
    OUString aExternName;
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 2 )
        rStream >> pImp->nLoadingVersion;
    aExternName = SfxPoolItem::readByteString(rStream);
    bOwnPool = aExternName == pImp->aName;
    pImp->bStreaming = sal_True;

    //! solange wir keine fremden laden k"onnen
    if ( !bOwnPool )
    {
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
        pImp->bStreaming = sal_False;
        return rStream;
    }

    // Versionen bis 1.3 k"onnen noch keine Which-Verschiebungen lesen
    if ( pImp->nMajorVer == 1 && pImp->nMinorVer <= 2 &&
         pImp->nVersion < pImp->nLoadingVersion )
    {
        rStream.SetError(ERRCODE_IO_WRONGVERSION);
        pImp->bStreaming = sal_False;
        return rStream;
    }

    // Size-Table liegt hinter den eigentlichen Attributen
    rStream >> nAttribSize;

    // Size-Table einlesen
    sal_uLong nStartPos = rStream.Tell();
    rStream.SeekRel( nAttribSize );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_SIZES );
    sal_uInt32 nSizeTableLen(0);
    rStream >> nSizeTableLen;
    sal_Char *pBuf = new sal_Char[nSizeTableLen];
    rStream.Read( pBuf, nSizeTableLen );
    sal_uLong nEndOfSizes = rStream.Tell();
    SvMemoryStream aSizeTable( pBuf, nSizeTableLen, STREAM_READ );

    // ab Version 1.3 steht in der Size-Table eine Versions-Map
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 3 )
    {
        // Version-Map finden (letztes sal_uLong der Size-Table gibt Pos an)
        rStream.Seek( nEndOfSizes - sizeof(sal_uInt32) );
        sal_uInt32 nVersionMapPos(0);
        rStream >> nVersionMapPos;
        rStream.Seek( nVersionMapPos );

        // Versions-Maps einlesen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_VERSIONMAP );
        sal_uInt16 nVerCount(0);
        rStream >> nVerCount;
        for ( sal_uInt16 nVerNo = 0; nVerNo < nVerCount; ++nVerNo )
        {
            // Header f"ur einzelne Version einlesen
            sal_uInt16 nVersion(0), nHStart(0), nHEnd(0);
            rStream >> nVersion >> nHStart >> nHEnd;
            sal_uInt16 nCount = nHEnd - nHStart + 1;
            sal_uInt16 nBytes = (nCount)*sizeof(sal_uInt16);

            // Is new version is known?
            if ( nVerNo >= pImp->aVersions.size() )
            {
                // Add new Version
                sal_uInt16 *pMap = new sal_uInt16[nCount];
                memset(pMap, 0, nCount * sizeof(sal_uInt16));
                for ( sal_uInt16 n = 0; n < nCount; ++n )
                    rStream >> pMap[n];
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
            else
                // Version schon bekannt => "uberspringen
                rStream.SeekRel( nBytes );
        }
    }

    // Items laden
    rStream.Seek( nStartPos );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ITEMS );
    bool bSecondaryLoaded = false;
    long nSecondaryEnd = 0;
    sal_uInt16 nWhich(0), nSlot(0);
    while ( rStream >> nWhich, nWhich )
    {
        // ggf. Which-Id aus alter Version verschieben?
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream >> nSlot;
        sal_uInt16 nMappedWhich = GetWhich(nSlot, sal_False);
        int bKnownItem = bOwnPool || IsWhich(nMappedWhich);

        sal_uInt16 nRef(0), nCount(0), nVersion(0);
        sal_uInt32 nAttrSize(0);
        rStream >> nVersion >> nCount;

        std::vector<SfxPoolItemArray_Impl*>::iterator ppArr;
        SfxPoolItemArray_Impl *pNewArr = 0;
        SfxPoolItem *pDefItem = 0;
        if ( bKnownItem )
        {
            if ( !bOwnPool )
                nWhich = nMappedWhich;

            //!SFX_ASSERTWARNING( !nSlot || !HasMap() ||
            //!         ( nSlot == GetSlotId( nWhich, sal_False ) ) ||
            //!         !GetSlotId( nWhich, sal_False ),
            //!         nWhich, "Slot/Which mismatch" );

            sal_uInt16 nIndex = GetIndex_Impl(nWhich);
            ppArr = pImp->maPoolItems.begin();
            std::advance(ppArr, nIndex);
            pNewArr = new SfxPoolItemArray_Impl();
            pDefItem = *(pImp->ppStaticDefaults + nIndex);
        }

        // Position vor ersten Item merken
        sal_uLong nLastPos = rStream.Tell();

        // SfxSetItems k"onnten Items aus Sekund"arpools beinhalten
        if ( !bSecondaryLoaded && pImp->mpSecondary && pDefItem->ISA(SfxSetItem) )
        {
            // an das Ende des eigenen Pools seeken
            rStream.Seek(nEndOfSizes);
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );

            // Sekund"arpool einlesen
            pImp->mpSecondary->Load1_Impl( rStream );
            bSecondaryLoaded = true;
            nSecondaryEnd = rStream.Tell();

            // zur"uck zu unseren eigenen Items
            rStream.Seek(nLastPos);
        }

        // Items an sich lesen
        for ( sal_uInt16 j = 0; j < nCount; ++j )
        {
            sal_uLong nPos = nLastPos;
            rStream >> nRef;

            if ( bKnownItem )
            {
                SfxPoolItem *pItem = 0;
                if ( nRef )
                {
                    pItem = pDefItem->Create(rStream, nVersion);

                    if ( !pImp->mbPersistentRefCounts )
                        // bis <SfxItemPool::LoadCompleted()> festhalten
                        AddRef(*pItem, 1);
                    else
                    {
                        if ( nRef > SFX_ITEMS_OLD_MAXREF )
                            pItem->SetKind( nRef );
                        else
                            AddRef(*pItem, nRef);
                    }
                }
                //pNewArr->insert( pItem, j );
                pNewArr->push_back( (SfxPoolItem*) pItem );

                // restliche gespeicherte Laenge skippen (neueres Format)
                nLastPos = rStream.Tell();
            }

            aSizeTable >> nAttrSize;
            SFX_ASSERT( !bKnownItem || ( nPos + nAttrSize) >= nLastPos,
                        nPos,
                        "too many bytes read - version mismatch?" );

            if ( !bKnownItem || ( nLastPos < (nPos + nAttrSize) ) )
            {
                nLastPos = nPos + nAttrSize;
                rStream.Seek( nLastPos );
            }
        }

        if ( bKnownItem )
        {
            SfxPoolItemArray_Impl *pOldArr = *ppArr;
            *ppArr = pNewArr;

            // die Items merken, die schon im Pool sind
            int bEmpty = sal_True;
            if ( 0 != pOldArr )
                for ( size_t n = 0; bEmpty && n < pOldArr->size(); ++n )
                    bEmpty = pOldArr->operator[](n) == 0;
            DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
            if ( !bEmpty )
            {
                // f"ur alle alten suchen, ob ein gleiches neues existiert
                for ( size_t nOld = 0; nOld < pOldArr->size(); ++nOld )
                {
                    SfxPoolItem *pOldItem = (*pOldArr)[nOld];
                    if ( pOldItem )
                    {
                        bool bFound = false;
                        for ( size_t nNew = 0;
                              nNew < (*ppArr)->size();  ++nNew )
                        {
                            SfxPoolItem *&rpNewItem =
                                (SfxPoolItem*&)(*ppArr)->operator[](nNew);

                            if ( rpNewItem && *rpNewItem == *pOldItem )
                            {
                                AddRef( *pOldItem, rpNewItem->GetRefCount() );
                                SetRefCount( *rpNewItem, 0 );
                                delete rpNewItem;
                                rpNewItem = pOldItem;
                                bFound = true;
                                SAL_INFO("svl", "reusing item" << pOldItem);
                                break;
                            }
                        }
                        SAL_INFO_IF(
                            !bFound, "svl", "item not found: " << pOldItem);
                    }
                }
            }
            delete pOldArr; /* @@@ */
        }
    }

    // Pool-Defaults lesen
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer > 0 )
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_DEFAULTS );

    sal_uLong nLastPos = rStream.Tell();
    while ( rStream >> nWhich, nWhich )
    {
        // ggf. Which-Id aus alter Version verschieben?
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream >> nSlot;
        sal_uInt16 nMappedWhich = GetWhich(nSlot, sal_False);
        int bKnownItem = bOwnPool || IsWhich(nMappedWhich);

        sal_uLong nPos = nLastPos;
        sal_uInt32 nSize(0);
        sal_uInt16 nVersion(0);
        rStream >> nVersion;

        if ( bKnownItem )
        {
            if ( !bOwnPool )
                nWhich = nMappedWhich;
            SfxPoolItem *pItem =
                ( *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
                ->Create( rStream, nVersion );
            pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
            *( pImp->ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;
        }

        nLastPos = rStream.Tell();
        aSizeTable >> nSize;
        SFX_ASSERT( ( nPos + nSize) >= nLastPos, nPos,
                    "too many bytes read - version mismatch?" );
        if ( nLastPos < (nPos + nSize) )
            rStream.Seek( nPos + nSize );
    }

    delete[] pBuf;
    rStream.Seek(nEndOfSizes);
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ENDPOOL );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ENDPOOL );

    if ( pImp->mpSecondary )
    {
        if ( !bSecondaryLoaded )
            pImp->mpSecondary->Load1_Impl( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    if ( aExternName != pImp->aName )
        pImp->aName = OUString();

    pImp->bStreaming = sal_False;
    return rStream;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxItemPool::LoadSurrogate
(
    SvStream&           rStream,    // vor einem Surrogat positionierter Stream
    sal_uInt16&             rWhich,     // Which-Id des zu ladenden <SfxPoolItem>s
    sal_uInt16              nSlotId,    // Slot-Id des zu ladenden <SfxPoolItem>s
    const SfxItemPool*  pRefPool    // <SfxItemPool> in dem das Surrogat gilt
)

/*  [Beschreibung]

    L"adt Surrogat aus 'rStream' und liefert das dadurch in 'rRefPool'
    repr"asentierte SfxPoolItem zu"ruck. Ist das im Stream befindliche
    Surrogat == SFX_ITEMS_DIRECT (!SFX_ITEM_POOLABLE) wird 0 zur"uckgegeben,
    das Item ist direkt aus dem Stream zu laden. Bei 0xfffffff0 (SFX_ITEMS_NULL)
    wird auch 0 zurueckgegeben und rWhich auf 0 gesetzt, das Item ist nicht
    verfuegbar.

    Ansonsten wird ber"ucksichtigt, ob der betroffene Pool ohne Ref-Counts
    geladen wird, ob aus einem neuen Pool nachgeladen wird (&rRefPool != this)
    oder ob aus einem g"anzlich anders aufgebauten Pool geladen wird.

    Wird aus einem anders aufgebauten Pool geladen und die 'nSlotId' kann
    nicht in eine Which-Id dieses Pools gemappt werden, wird ebenfalls 0
    zur"uckgeliefert.

    Preconditions:  - Pool mu\s geladen sein
                    - LoadCompleted darf noch nicht gerufen worden sein
                    - 'rStream' steht genau an der Position, an der ein
                      Surrogat f"ur ein Item mit der SlotId 'nSlotId' und
                      der WhichId 'rWhichId' mit StoreSurrogate gepeichert
                      wurde

    Postconditions: - 'rStream' ist so positioniert, wie auch StoreSurrogate
                      sein speichern beendet hatte
                    - konnte ein Item geladen werden, befindet es sich
                      in diesem SfxItemPool
                    - 'rWhichId' enth"alt die ggf. gemappte Which-Id
    Laufzeit:       Tiefe des Ziel Sekund"arpools * 10 + 10

    [Querverweise]

    <SfxItemPool::StoreSurrogate(SvStream&,const SfxPoolItem &)const>
*/

{
    // Read the first surrogate
    sal_uInt32 nSurrogat(0);
    rStream >> nSurrogat;

    // Is item stored directly?
    if ( SFX_ITEMS_DIRECT == nSurrogat )
        return 0;

    // Item does not exist?
    if ( SFX_ITEMS_NULL == nSurrogat )
    {
        rWhich = 0;
        return 0;
    }

    // Bei einem identisch aufgebauten Pool (im Stream) kann das Surrogat
    // auf jeden Fall aufgel"ost werden.
    if ( !pRefPool )
        pRefPool = this;
    bool bResolvable = !pRefPool->GetName().isEmpty();
    if ( !bResolvable )
    {
        // Bei einem anders aufgebauten Pool im Stream, mu\s die SlotId
        // aus dem Stream in eine Which-Id gemappt werden k"onnen.
        sal_uInt16 nMappedWhich = nSlotId ? GetWhich(nSlotId, sal_True) : 0;
        if ( IsWhich(nMappedWhich) )
        {
            // gemappte SlotId kann "ubernommen werden
            rWhich = nMappedWhich;
            bResolvable = sal_True;
        }
    }

    // kann Surrogat aufgel"ost werden?
    if ( bResolvable )
    {
        const SfxPoolItem *pItem = 0;
        for ( SfxItemPool *pTarget = this; pTarget; pTarget = pTarget->pImp->mpSecondary )
        {
            // richtigen (Folge-) Pool gefunden?
            if ( pTarget->IsInRange(rWhich) )
            {
                // dflt-Attribut?
                if ( SFX_ITEMS_DEFAULT == nSurrogat )
                    return *(pTarget->pImp->ppStaticDefaults +
                            pTarget->GetIndex_Impl(rWhich));

                SfxPoolItemArray_Impl* pItemArr =
                    pTarget->pImp->maPoolItems[pTarget->GetIndex_Impl(rWhich)];
                pItem = pItemArr && nSurrogat < pItemArr->size()
                            ? (*pItemArr)[nSurrogat]
                            : 0;
                if ( !pItem )
                {
                    OSL_FAIL( "can't resolve surrogate" );
                    rWhich = 0; // nur zur Sicherheit fuer richtige Stream-Pos
                    return 0;
                }

                // Nachladen aus Ref-Pool?
                if ( pRefPool != pImp->mpMaster )
                    return &pTarget->Put( *pItem );

                // Referenzen sind NICHT schon mit Pool geladen worden?
                if ( !pTarget->HasPersistentRefCounts() )
                    AddRef( *pItem, 1 );
                else
                    return pItem;

                return pItem;
            }
        }

        SFX_ASSERT( sal_False, rWhich, "can't resolve Which-Id in LoadSurrogate" );
    }

    return 0;
}



bool SfxItemPool::StoreSurrogate
(
    SvStream&           rStream,
    const SfxPoolItem*  pItem
)   const

/*  [Beschreibung]

    Speichert ein Surrogat f"ur '*pItem' in 'rStream'.


    [R"uckgabewert]

    bool                    TRUE
                            es wurde ein echtes Surrogat gespeichert, auch
                            SFX_ITEMS_NULL bei 'pItem==0',
                            SFX_ITEMS_STATICDEFAULT und SFX_ITEMS_POOLDEFAULT
                            gelten als 'echte' Surrogate

                            sal_False
                            es wurde ein Dummy-Surrogat (SFX_ITEMS_DIRECT)
                            gespeichert, das eigentliche Item mu\s direkt
                            hinterher selbst gespeichert werden
*/

{
    if ( pItem )
    {
        bool bRealSurrogate = IsItemFlag(*pItem, SFX_ITEM_POOLABLE);
        rStream << ( bRealSurrogate
                        ? GetSurrogate( pItem )
                        : SFX_ITEMS_DIRECT );
        return bRealSurrogate;
    }

    rStream << SFX_ITEMS_NULL;
    return sal_True;
}

// -----------------------------------------------------------------------

sal_uInt32 SfxItemPool::GetSurrogate(const SfxPoolItem *pItem) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT( pItem, "no 0-Pointer Surrogate" );
    DBG_ASSERT( !IsInvalidItem(pItem), "no Invalid-Item Surrogate" );
    DBG_ASSERT( !IsPoolDefaultItem(pItem), "no Pool-Default-Item Surrogate" );

    if ( !IsInRange(pItem->Which()) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetSurrogate( pItem );
        SFX_ASSERT( 0, pItem->Which(), "unknown Which-Id - dont ask me for surrogates" );
    }

    // Pointer auf static- oder pool-dflt-Attribut?
    if( IsStaticDefaultItem(pItem) || IsPoolDefaultItem(pItem) )
        return SFX_ITEMS_DEFAULT;

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[GetIndex_Impl(pItem->Which())];
    DBG_ASSERT(pItemArr, "ItemArr is not available");

    for ( size_t i = 0; i < pItemArr->size(); ++i )
    {
        const SfxPoolItem *p = (*pItemArr)[i];
        if ( p == pItem )
            return i;
    }
    SFX_ASSERT( 0, pItem->Which(), "Item not in the pool");
    return SFX_ITEMS_NULL;
}

// -----------------------------------------------------------------------

bool SfxItemPool::IsInStoringRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImp->nStoringStart &&
           nWhich <= pImp->nStoringEnd;
}


void SfxItemPool::SetStoringRange( sal_uInt16 nFrom, sal_uInt16 nTo )

/*  [Beschreibung]

    Mit dieser Methode kann der Which-Bereich eingeengt werden, der
    von ItemSets dieses Pool (und dem Pool selbst) gespeichert wird.
    Die Methode muss dazu vor <SfxItemPool::Store()> gerufen werden
    und die Werte muessen auch noch gesetzt sein, wenn das eigentliche
    Dokument (also die ItemSets gespeicher werden).

    Ein Zuruecksetzen ist dann nicht noetig, wenn dieser Range vor
    JEDEM Speichern richtig gesetzt wird, da er nur beim Speichern
    beruecksichtigt wird.

    Dieses muss fuer das 3.1-Format gemacht werden, da dort eine
    Bug in der Pool-Lade-Methode vorliegt.
*/

{
    pImp->nStoringStart = nFrom;
    pImp->nStoringEnd = nTo;
}

// -----------------------------------------------------------------------

void SfxItemPool::SetVersionMap
(
    sal_uInt16  nVer,               /*  neue Versionsnummer */
    sal_uInt16  nOldStart,          /*  alte erste Which-Id */
    sal_uInt16  nOldEnd,            /*  alte letzte Which-Id */
    const sal_uInt16* pOldWhichIdTab /*  Array mit genau dem Aufbau der Which-Ids
                                    der vorhergehenden Version, in denen
                                    die jeweils neue Which-Id steht. */
)

/*  [Beschreibung]

    Mit dieser Methode k"onnen neue, inkompatible Which-Id-Folgen oder
    Verteilungen realisiert werden. Pools, die noch mit alten Versionen
    gespeichert wurden, werden dann "uber die angegebene Tabelle solange
    gemappt, bis die aktuelle Version erreicht ist. Neuere Pools k"onnen
    unter Verlust neuer Attribute geladen werden, da die Map mit dem Pool
    gespeichert wird.

    Precondition:   Pool darf noch nicht geladen sein
    Postcondition:  Which-Ids aus fr"uheren Versionen k"onnen bei Laden auf
                    Version 'nVer' gemappt werden
    Laufzeit:       1.5 * new + 10

    [Anmerkung]

    F"ur neue Which-Ranges (nStart,nEnd) m"ssen im Vergleich zur Vorg"anger-
    Version (nOldStart,nOldEnd) immer gelten, da\s (nOldStart,nOldEnd)
    vollst"andig in (nStart,nEnd) enthalten ist. Es ist also zul"assig, den
    Which-Range in beide Richtungen zu erweitern, auch durch Einf"ugung
    von Which-Ids, nicht aber ihn zu beschneiden.

    Diese Methode sollte nur im oder direkt nach Aufruf des Konstruktors
    gerufen werden.

    Das Array mu\s statisch sein, da es nicht kopiert wird und au\serdem
    im Copy-Ctor des SfxItemPool wiederverwendet wird.


    [Beispiel]

    Urspr"unglich (Version 0) hatte der Pool folgende Which-Ids:

        1:A, 2:B, 3:C, 4:D

    Nun soll eine neue Version (Version 1) zwei zus"atzliche Ids X und Y
    zwischen B und C erhalten, also wie folgt aussehen:

        1:A, 2:B, 3:X, 4:Y, 5:C, 6:D

    Dabei haben sich also die Ids 3 und 4 ge"andert. F"ur die neue Version
    m"u\ste am Pool folgendes gesetzt werden:

        static sal_uInt16 nVersion1Map = { 1, 2, 5, 6 };
        pPool->SetVersionMap( 1, 1, 4, &nVersion1Map );


    [Querverweise]

    <SfxItemPool::IsLoadingVersionCurrent()const>
    <SfxItemPool::GetNewWhich(sal_uInt16)>
    <SfxItemPool::GetVersion()const>
*/

{
    // create new map entry to insert
    const SfxPoolVersion_ImplPtr pVerMap = SfxPoolVersion_ImplPtr( new SfxPoolVersion_Impl(
                nVer, nOldStart, nOldEnd, pOldWhichIdTab ) );
    pImp->aVersions.push_back( pVerMap );

    DBG_ASSERT( nVer > pImp->nVersion, "Versions not sorted" );
    pImp->nVersion = nVer;

    // Versions-Range anpassen
    for ( sal_uInt16 n = 0; n < nOldEnd-nOldStart+1; ++n )
    {
        sal_uInt16 nWhich = pOldWhichIdTab[n];
        if ( nWhich < pImp->nVerStart )
        {
            if ( !nWhich )
                nWhich = 0;
            pImp->nVerStart = nWhich;
        }
        else if ( nWhich > pImp->nVerEnd )
            pImp->nVerEnd = nWhich;
    }
}

// -----------------------------------------------------------------------

sal_uInt16 SfxItemPool::GetNewWhich
(
    sal_uInt16  nFileWhich      // die aus dem Stream geladene Which-Id
)   const

/*  [Beschreibung]

    Diese Methoden rechnet Which-Ids aus einem File-Format in die der
    aktuellen Pool-Version um. Ist das File-Format "alter, werden die vom
    Pool-Entwickler mit SetVersion() gesetzten Tabellen verwendet,
    ist das File-Format neuer, dann die aus dem File geladenen Tabellen.
    Im letzteren Fall kann ggf. nicht jede Which-Id gemappt werden,
    so da\s 0 zur"uckgeliefert wird.

    Die Berechnung ist nur f"ur Which-Ids definiert, die in der betreffenden
    File-Version unterst"utzt wurden. Dies ist per Assertion abgesichert.

    Precondition:   Pool mu\s geladen sein
    Postcondition:  unver"andert
    Laufzeit:       linear(Anzahl der Sekund"arpools) +
                    linear(Differenz zwischen alter und neuer Version)


    [Querverweise]

    <SfxItemPool::IsLoadingVersionCurrent()const>
    <SfxItemPool::SetVersionMap(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16*)>
    <SfxItemPool::GetVersion()const>
*/

{
    // (Sekund"ar-) Pool bestimmen
    if ( !IsInVersionsRange(nFileWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetNewWhich( nFileWhich );
        SFX_ASSERT( 0, nFileWhich, "unknown which in GetNewWhich()" );
    }

    // Version neuer/gleich/"alter?
    short nDiff = (short)pImp->nLoadingVersion - (short)pImp->nVersion;

    // Which-Id einer neueren Version?
    if ( nDiff > 0 )
    {
        // von der Top-Version bis runter zur File-Version stufenweise mappen
        for ( size_t nMap = pImp->aVersions.size(); nMap > 0; --nMap )
        {
            SfxPoolVersion_ImplPtr pVerInfo = pImp->aVersions[nMap-1];
            if ( pVerInfo->_nVer > pImp->nVersion )
            {   sal_uInt16 nOfs;
                sal_uInt16 nCount = pVerInfo->_nEnd - pVerInfo->_nStart + 1;
                for ( nOfs = 0;
                      nOfs <= nCount &&
                        pVerInfo->_pMap[nOfs] != nFileWhich;
                      ++nOfs )
                    continue;

                if ( pVerInfo->_pMap[nOfs] == nFileWhich )
                    nFileWhich = pVerInfo->_nStart + nOfs;
                else
                    return 0;
            }
            else
                break;
        }
    }

    // Which-Id einer neueren Version?
    else if ( nDiff < 0 )
    {
        // von der File-Version bis zur aktuellen Version stufenweise mappen
        for ( size_t nMap = 0; nMap < pImp->aVersions.size(); ++nMap )
        {
            SfxPoolVersion_ImplPtr pVerInfo = pImp->aVersions[nMap];
            if ( pVerInfo->_nVer > pImp->nLoadingVersion )
            {
                DBG_ASSERT( nFileWhich >= pVerInfo->_nStart &&
                            nFileWhich <= pVerInfo->_nEnd,
                            "which-id unknown in version" );
                nFileWhich = pVerInfo->_pMap[nFileWhich - pVerInfo->_nStart];
            }
        }
    }

    // originale (nDiff==0) bzw. gemappte (nDiff!=0) Id zur"uckliefern
    return nFileWhich;
}

// -----------------------------------------------------------------------


bool SfxItemPool::IsInVersionsRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImp->nVerStart && nWhich <= pImp->nVerEnd;
}

// -----------------------------------------------------------------------

bool SfxItemPool::IsCurrentVersionLoading() const

/*  [Beschreibung]

    Mit dieser Methode kann festgestellt werden, ob die geladene Pool-Version
    dem aktuellen Pool-Aufbau entspricht.

    Precondition:   Pool mu\s geladen sein
    Postcondition:  unver"andert
    Laufzeit:       linear(Anzahl der Sekund"arpools)


    [Querverweise]

    <SfxItemPool::SetVersionMap(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16*)>
    <SfxItemPool::GetNewWhich(sal_uInt16)const>
    <SfxItemPool::GetVersion()const>
*/

{
    return ( pImp->nVersion == pImp->nLoadingVersion ) &&
           ( !pImp->mpSecondary || pImp->mpSecondary->IsCurrentVersionLoading() );
}



bool SfxItemPool::StoreItem( SvStream &rStream, const SfxPoolItem &rItem,
                                 bool bDirect ) const

/*  [Beschreibung]

    Speichert das <SfxPoolItem> 'rItem' in den <SvStream> 'rStream'
    entweder als Surrogat ('bDirect == sal_False') oder direkt mit 'rItem.Store()'.
    Nicht poolable Items werden immer direkt gespeichert. Items ohne Which-Id,
    also SID-Items, werden nicht gespeichert, ebenso wenn Items, die in der
    File-Format-Version noch nicht vorhanden waren (return sal_False).

    Das Item wird im Stream wie folgt abgelegt:

    sal_uInt16  rItem.Which()
    sal_uInt16  GetSlotId( rItem.Which() ) bzw. 0 falls nicht verf"urbar
    sal_uInt16  GetSurrogate( &rItem ) bzw. SFX_ITEM_DIRECT bei '!SFX_ITEM_POOLBLE'

    optional (falls 'bDirect == sal_True' oder '!rItem.IsPoolable()':

    sal_uInt16  rItem.GetVersion()
    sal_uLong   Size
    Size    rItem.Store()


    [Querverweise]

    <SfxItemPool::LoadItem(SvStream&,bool)const>
*/

{
    DBG_ASSERT( !IsInvalidItem(&rItem), "cannot store invalid items" );

    if ( IsSlot( rItem.Which() ) )
        return sal_False;
    const SfxItemPool *pPool = this;
    while ( !pPool->IsInStoringRange(rItem.Which()) )
        if ( 0 == ( pPool = pPool->pImp->mpSecondary ) )
            return sal_False;

    DBG_ASSERT( !pImp->bInSetItem || !rItem.ISA(SfxSetItem),
                "SetItem contains ItemSet with SetItem" );

    sal_uInt16 nSlotId = pPool->GetSlotId( rItem.Which(), sal_True );
    sal_uInt16 nItemVersion = rItem.GetVersion(pImp->mnFileFormatVersion);
    if ( USHRT_MAX == nItemVersion )
        return sal_False;

    rStream << rItem.Which() << nSlotId;
    if ( bDirect || !pPool->StoreSurrogate( rStream, &rItem ) )
    {
        rStream << nItemVersion;
        rStream << (sal_uInt32) 0L;           // Platz fuer Laenge in Bytes
        sal_uLong nIStart = rStream.Tell();
        rItem.Store(rStream, nItemVersion);
        sal_uLong nIEnd = rStream.Tell();
        rStream.Seek( nIStart-4 );
        rStream << (sal_Int32) ( nIEnd-nIStart );
        rStream.Seek( nIEnd );
    }

    return sal_True;
}



const SfxPoolItem* SfxItemPool::LoadItem( SvStream &rStream, bool bDirect,
                                          const SfxItemPool *pRefPool )

// pRefPool==-1 => nicht putten!

{
    sal_uInt16 nWhich(0), nSlot(0); // nSurrogate;
    rStream >> nWhich >> nSlot;

    bool bDontPut = (SfxItemPool*)-1 == pRefPool;
    if ( bDontPut || !pRefPool )
        pRefPool = this;

    // richtigen Sekund"ar-Pool finden
    while ( !pRefPool->IsInVersionsRange(nWhich) )
    {
        if ( pRefPool->pImp->mpSecondary )
            pRefPool = pRefPool->pImp->mpSecondary;
        else
        {
            // WID in der Version nicht vorhanden => ueberspringen
            sal_uInt32 nSurro(0);
            sal_uInt16 nVersion(0), nLen(0);
            rStream >> nSurro;
            if ( SFX_ITEMS_DIRECT == nSurro )
            {
                rStream >> nVersion >> nLen;
                rStream.SeekRel( nLen );
            }
            return 0;
        }
    }

    // wird eine andere Version geladen?
    bool bCurVersion = pRefPool->IsCurrentVersionLoading();
    if ( !bCurVersion )
        // Which-Id auf neue Version mappen
        nWhich = pRefPool->GetNewWhich( nWhich );

    DBG_ASSERT( !nWhich || !pImp->bInSetItem ||
                !pRefPool->pImp->ppStaticDefaults[pRefPool->GetIndex_Impl(nWhich)]->ISA(SfxSetItem),
                "loading SetItem in ItemSet of SetItem" );

    // soll "uber Surrogat geladen werden?
    const SfxPoolItem *pItem = 0;
    if ( !bDirect )
    {
        // Which-Id in dieser Version bekannt?
        if ( nWhich )
            // Surrogat laden, reagieren falls keins vorhanden
            pItem = LoadSurrogate( rStream, nWhich, nSlot, pRefPool );
        else
            // sonst "uberspringen
            rStream.SeekRel( sizeof(sal_uInt16) );
    }

    // wird direkt, also nicht "uber Surrogat geladen?
    if ( bDirect || ( nWhich && !pItem ) )
    {
        // bDirekt bzw. nicht IsPoolable() => Item direkt laden
        sal_uInt16 nVersion(0);
        sal_uInt32 nLen(0);
        rStream >> nVersion >> nLen;
        sal_uLong nIStart = rStream.Tell();

        // Which-Id in dieser Version bekannt?
        if ( nWhich )
        {
            // Item direkt laden
            SfxPoolItem *pNewItem =
                    pRefPool->GetDefaultItem(nWhich).Create(rStream, nVersion);
            if ( bDontPut )
                pItem = pNewItem;
            else
                if ( pNewItem )
                {
                    pItem = &Put(*pNewItem);
                    delete pNewItem;
                }
                else
                    pItem = 0;
            sal_uLong nIEnd = rStream.Tell();
            DBG_ASSERT( nIEnd <= (nIStart+nLen), "read past end of item" );
            if ( (nIStart+nLen) != nIEnd )
                rStream.Seek( nIStart+nLen );
        }
        else
            // Item "uberspringen
            rStream.Seek( nIStart+nLen );
    }

    return pItem;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
