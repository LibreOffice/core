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


#include <string.h>
#include <stdio.h>

#include <sal/log.hxx>
#include <tools/solar.h>
#include <svl/itempool.hxx>
#include "whassert.hxx"
#include <svl/brdcst.hxx>
#include <svl/filerec.hxx>
#include "poolio.hxx"



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

    
    SfxItemPool *pStoreMaster = pImp->mpMaster != this ? pImp->mpMaster : 0;
    while ( pStoreMaster && !pStoreMaster->pImp->bStreaming )
        pStoreMaster = pStoreMaster->pImp->mpSecondary;

    
    pImp->bStreaming = true;
    if ( !pStoreMaster )
    {
        rStream.WriteUInt16(  rStream.GetVersion() >= SOFFICE_FILEFORMAT_50
                ? SFX_ITEMPOOL_TAG_STARTPOOL_5
                : SFX_ITEMPOOL_TAG_STARTPOOL_4  );
        rStream.WriteUInt8( SFX_ITEMPOOL_VER_MAJOR ).WriteUInt8( SFX_ITEMPOOL_VER_MINOR );
        rStream.WriteUInt16( SFX_ITEMPOOL_TAG_TRICK4OLD );

        
        rStream.WriteUInt16( sal_uInt16(0) ); 
        rStream.WriteUInt16( sal_uInt16(0) ); 
    }

    
    SfxMiniRecordWriter aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    pStoringPool_ = this;

    
    {
        SfxMiniRecordWriter aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER);
        rStream.WriteUInt16( pImp->nVersion );
        SfxPoolItem::writeByteString(rStream, pImp->aName);
    }

    
    {
        SfxMultiVarRecordWriter aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP, 0 );
        for ( size_t nVerNo = 0; nVerNo < pImp->aVersions.size(); ++nVerNo )
        {
            aVerRec.NewContent();
            SfxPoolVersion_ImplPtr pVer = pImp->aVersions[nVerNo];
            rStream.WriteUInt16( pVer->_nVer ).WriteUInt16( pVer->_nStart ).WriteUInt16( pVer->_nEnd );
            sal_uInt16 nCount = pVer->_nEnd - pVer->_nStart + 1;
            sal_uInt16 nNewWhich = 0;
            for ( sal_uInt16 n = 0; n < nCount; ++n )
            {
                nNewWhich = pVer->_pMap[n];
                rStream.WriteUInt16( nNewWhich );
            }

            
            if ( SOFFICE_FILEFORMAT_31 == pImp->mnFileFormatVersion )
                rStream.WriteUInt16( sal_uInt16(nNewWhich+1) );
        }
    }

    
    {
        SfxMultiMixRecordWriter aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS, 0 );

        
        for (int ft = 0 ; ft < 2 && !rStream.GetError(); ft++)
        {
            pImp->bInSetItem = ft != 0;

            std::vector<SfxPoolItemArray_Impl*>::iterator itrArr = pImp->maPoolItems.begin();
            SfxPoolItem **ppDefItem = pImp->ppStaticDefaults;
            const sal_uInt16 nSize = GetSize_Impl();
            for ( size_t i = 0; i < nSize && !rStream.GetError(); ++i, ++itrArr, ++ppDefItem )
            {
                
                sal_uInt16 nItemVersion = (*ppDefItem)->GetVersion( pImp->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    
                    continue;

                
                
                if ( *itrArr && IsItemFlag(**ppDefItem, SFX_ITEM_POOLABLE) &&
                     pImp->bInSetItem == (bool) (*ppDefItem)->ISA(SfxSetItem) )
                {
                    
                    sal_uInt16 nSlotId = GetSlotId( (*ppDefItem)->Which(), false );
                    aWhichIdsRec.NewContent(nSlotId, 0);
                    rStream.WriteUInt16( (*ppDefItem)->Which() );
                    rStream.WriteUInt16( nItemVersion );
                    const sal_uInt32 nCount = ::std::min<size_t>( (*itrArr)->size(), SAL_MAX_UINT32 );
                    DBG_ASSERT(nCount, "ItemArr is empty");
                    rStream.WriteUInt32( nCount );

                    
                    SfxMultiMixRecordWriter aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS, 0 );
                    for ( size_t j = 0; j < nCount; ++j )
                    {
                        
                        const SfxPoolItem *pItem = (*itrArr)->operator[](j);
                        if ( pItem && pItem->GetRefCount() ) 
                        {
                            aItemsRec.NewContent((sal_uInt16)j, 'X' );

                            if ( pItem->GetRefCount() == SFX_ITEMS_SPECIAL )
                                rStream.WriteUInt16( (sal_uInt16) pItem->GetKind() );
                            else
                            {
                                rStream.WriteUInt16( (sal_uInt16) pItem->GetRefCount() );
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

    
    if ( !rStream.GetError() )
    {
        SfxMultiMixRecordWriter aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS, 0 );
        sal_uInt16 nCount = GetSize_Impl();
        for ( sal_uInt16 n = 0; n < nCount; ++n )
        {
            const SfxPoolItem* pDefaultItem = pImp->ppPoolDefaults[n];
            if ( pDefaultItem )
            {
                
                sal_uInt16 nItemVersion = pDefaultItem->GetVersion( pImp->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    
                    continue;

                
                sal_uInt16 nSlotId = GetSlotId( pDefaultItem->Which(), false );
                aDefsRec.NewContent( nSlotId, 0 );
                rStream.WriteUInt16( pDefaultItem->Which() );
                rStream.WriteUInt16( nItemVersion );

                
                pDefaultItem->Store( rStream, nItemVersion );
            }
        }
    }

    
    pStoringPool_ = 0;
    aPoolRec.Close();
    if ( !rStream.GetError() && pImp->mpSecondary )
        pImp->mpSecondary->Store( rStream );

    pImp->bStreaming = false;
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
    
    if ( pImp->nInitRefCount > 1 )
    {

        
        std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
        for( sal_uInt16 nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++itrItemArr )
        {
            
            if ( *itrItemArr )
            {
                
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

        
        pImp->nInitRefCount = 1;
    }

    
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
        
        sal_uInt16 nSurrogate = aItemsRec.GetContentTag();
        DBG_ASSERT( aItemsRec.GetContentVersion() == 'X',
                    "not an item content" );

        
        for ( pItem = 0, n = nLastSurrogate+1; n < nSurrogate; ++n )
            pNewArr->push_back( (SfxPoolItem*) pItem );
        nLastSurrogate = nSurrogate;

        
        sal_uInt16 nRef(0);
        rStream.ReadUInt16( nRef );

        pItem = pDefItem->Create(rStream, nVer);
        pNewArr->push_back( (SfxPoolItem*) pItem );

        if ( !mbPersistentRefCounts )
            
            SfxItemPool::AddRef(*pItem, 1);
        else
        {
            if ( nRef > SFX_ITEMS_OLD_MAXREF )
                SfxItemPool::SetKind(*pItem, nRef);
            else
                SfxItemPool::AddRef(*pItem, nRef);
        }
    }

    
    for ( pItem = 0, n = nLastSurrogate+1; n < nItemCount; ++n )
        pNewArr->push_back( (SfxPoolItem*) pItem );

    SfxPoolItemArray_Impl *pOldArr = *ppArr;
    *ppArr = pNewArr;

    
    bool bEmpty = true;
    if ( 0 != pOldArr )
        for ( n = 0; bEmpty && n < pOldArr->size(); ++n )
            bEmpty = pOldArr->operator[](n) == 0;
    DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
    if ( !bEmpty )
    {
        
        for ( size_t nOld = 0; nOld < pOldArr->size(); ++nOld )
        {
            SfxPoolItem *pOldItem = (*pOldArr)[nOld];
            if ( pOldItem )
            {
                sal_uInt32 nFree = SAL_MAX_UINT32;
                bool bFound = false;
                for ( size_t nNew = (*ppArr)->size(); nNew--; )
                {
                    
                    SfxPoolItem *&rpNewItem =
                        (SfxPoolItem*&)(*ppArr)->operator[](nNew);

                    
                    if ( !rpNewItem )
                        nFree = nNew;

                    
                    else if ( *rpNewItem == *pOldItem )
                    {
                        
                        SfxItemPool::AddRef( *pOldItem, rpNewItem->GetRefCount() );
                        SfxItemPool::SetRefCount( *rpNewItem, 0 );
                        delete rpNewItem;
                        rpNewItem = pOldItem;
                        bFound = true;
                        break;
                    }
                }

                
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



SvStream &SfxItemPool::Load(SvStream &rStream)
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(pImp->ppStaticDefaults, "kein DefaultArray");

    
    if ( !pImp->mbPersistentRefCounts )
    {

        
        std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
        for( size_t nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++itrItemArr )
        {
            
            if ( *itrItemArr )
            {
                
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

        
        pImp->nInitRefCount = 2;
    }

    
    SfxItemPool *pLoadMaster = pImp->mpMaster != this ? pImp->mpMaster : 0;
    while ( pLoadMaster && !pLoadMaster->pImp->bStreaming )
        pLoadMaster = pLoadMaster->pImp->mpSecondary;

    
    pImp->bStreaming = true;
    if ( !pLoadMaster )
    {
        
        CHECK_FILEFORMAT2( rStream,
                SFX_ITEMPOOL_TAG_STARTPOOL_5, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream.ReadUChar( pImp->nMajorVer ).ReadUChar( pImp->nMinorVer );

        
        pImp->mpMaster->pImp->nMajorVer = pImp->nMajorVer;
        pImp->mpMaster->pImp->nMinorVer = pImp->nMinorVer;

        
        if ( pImp->nMajorVer < 2 )
            
            return Load1_Impl( rStream );

        
        if ( pImp->nMajorVer > SFX_ITEMPOOL_VER_MAJOR )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImp->bStreaming = false;
            return rStream;
        }

        
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_TRICK4OLD );
        rStream.SeekRel( 4 ); 
    }

    
    SfxMiniRecordReader aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    if ( rStream.GetError() )
    {
        pImp->bStreaming = false;
        return rStream;
    }

    
    OUString aExternName;
    {
        
        SfxMiniRecordReader aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = false;
            return rStream;
        }

        
        rStream.ReadUInt16( pImp->nLoadingVersion );
        aExternName = SfxPoolItem::readByteString(rStream);
        bool bOwnPool = aExternName == pImp->aName;

        
        if ( !bOwnPool )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            aPoolRec.Skip();
            pImp->bStreaming = false;
            return rStream;
        }
    }

    
    {
        SfxMultiRecordReader aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = false;
            return rStream;
        }

        
        sal_uInt16 nOwnVersion = pImp->nVersion;
        for ( sal_uInt16 nVerNo = 0; aVerRec.GetContent(); ++nVerNo )
        {
            
            sal_uInt16 nVersion(0), nHStart(0), nHEnd(0);
            rStream.ReadUInt16( nVersion ).ReadUInt16( nHStart ).ReadUInt16( nHEnd );
            sal_uInt16 nCount = nHEnd - nHStart + 1;

            
            if ( nVerNo >= pImp->aVersions.size() )
            {
                
                sal_uInt16 *pMap = new sal_uInt16[nCount];
                memset(pMap, 0, nCount * sizeof(sal_uInt16));
                for ( sal_uInt16 n = 0; n < nCount; ++n )
                    rStream.ReadUInt16( pMap[n] );
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
        }
        pImp->nVersion = nOwnVersion;
    }

    
    bool bSecondaryLoaded = false;
    long nSecondaryEnd = 0;
    {
        SfxMultiRecordReader aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS);
        while ( aWhichIdsRec.GetContent() )
        {
            
            sal_uInt32 nCount(0);
            sal_uInt16 nVersion(0), nWhich(0);
            
            rStream.ReadUInt16( nWhich );
            if ( pImp->nLoadingVersion != pImp->nVersion )
                
                nWhich = GetNewWhich( nWhich );

            
            if ( !IsInRange(nWhich) )
                continue;

            rStream.ReadUInt16( nVersion );
            rStream.ReadUInt32( nCount );
            
            
            
            

            sal_uInt16 nIndex = GetIndex_Impl(nWhich);
            SfxPoolItemArray_Impl **ppArr = &pImp->maPoolItems[0] + nIndex;

            
            SfxPoolItem *pDefItem = *(pImp->ppStaticDefaults + nIndex);
            pImp->bInSetItem = pDefItem->ISA(SfxSetItem);
            if ( !bSecondaryLoaded && pImp->mpSecondary && pImp->bInSetItem )
            {
                
                sal_uLong nLastPos = rStream.Tell();
                aPoolRec.Skip();

                
                pImp->mpSecondary->Load( rStream );
                bSecondaryLoaded = true;
                nSecondaryEnd = rStream.Tell();

                
                rStream.Seek(nLastPos);
            }

            
            pImp->readTheItems(rStream, nCount, nVersion, pDefItem, ppArr);

            pImp->bInSetItem = false;
        }
    }

    
    {
        SfxMultiRecordReader aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS );

        while ( aDefsRec.GetContent() )
        {
            
            sal_uInt16 nVersion(0), nWhich(0);
            
            rStream.ReadUInt16( nWhich );
            if ( pImp->nLoadingVersion != pImp->nVersion )
                
                nWhich = GetNewWhich( nWhich );

            
            if ( !IsInRange(nWhich) )
                continue;

            rStream.ReadUInt16( nVersion );
            
            

            
            SfxPoolItem *pItem =
                    ( *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
                    ->Create( rStream, nVersion );
            pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
            *( pImp->ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;
        }
    }

    
    aPoolRec.Skip();
    if ( pImp->mpSecondary )
    {
        if ( !bSecondaryLoaded )
            pImp->mpSecondary->Load( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    
    if ( aExternName != pImp->aName )
        pImp->aName = OUString();

    pImp->bStreaming = false;
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
    
    if ( !pImp->bStreaming )
    {
        
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream.ReadUChar( pImp->nMajorVer ).ReadUChar( pImp->nMinorVer );
    }
    sal_uInt32 nAttribSize(0);
    OUString aExternName;
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 2 )
        rStream.ReadUInt16( pImp->nLoadingVersion );
    aExternName = SfxPoolItem::readByteString(rStream);
    bool bOwnPool = aExternName == pImp->aName;
    pImp->bStreaming = true;

    
    if ( !bOwnPool )
    {
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
        pImp->bStreaming = false;
        return rStream;
    }

    
    if ( pImp->nMajorVer == 1 && pImp->nMinorVer <= 2 &&
         pImp->nVersion < pImp->nLoadingVersion )
    {
        rStream.SetError(ERRCODE_IO_WRONGVERSION);
        pImp->bStreaming = false;
        return rStream;
    }

    
    rStream.ReadUInt32( nAttribSize );

    
    sal_uLong nStartPos = rStream.Tell();
    rStream.SeekRel( nAttribSize );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_SIZES );
    sal_uInt32 nSizeTableLen(0);
    rStream.ReadUInt32( nSizeTableLen );
    sal_Char *pBuf = new sal_Char[nSizeTableLen];
    rStream.Read( pBuf, nSizeTableLen );
    sal_uLong nEndOfSizes = rStream.Tell();
    SvMemoryStream aSizeTable( pBuf, nSizeTableLen, STREAM_READ );

    
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 3 )
    {
        
        rStream.Seek( nEndOfSizes - sizeof(sal_uInt32) );
        sal_uInt32 nVersionMapPos(0);
        rStream.ReadUInt32( nVersionMapPos );
        rStream.Seek( nVersionMapPos );

        
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_VERSIONMAP );
        sal_uInt16 nVerCount(0);
        rStream.ReadUInt16( nVerCount );
        for ( sal_uInt16 nVerNo = 0; nVerNo < nVerCount; ++nVerNo )
        {
            
            sal_uInt16 nVersion(0), nHStart(0), nHEnd(0);
            rStream.ReadUInt16( nVersion ).ReadUInt16( nHStart ).ReadUInt16( nHEnd );
            sal_uInt16 nCount = nHEnd - nHStart + 1;
            sal_uInt16 nBytes = (nCount)*sizeof(sal_uInt16);

            
            if ( nVerNo >= pImp->aVersions.size() )
            {
                
                sal_uInt16 *pMap = new sal_uInt16[nCount];
                memset(pMap, 0, nCount * sizeof(sal_uInt16));
                for ( sal_uInt16 n = 0; n < nCount; ++n )
                    rStream.ReadUInt16( pMap[n] );
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
            else
                
                rStream.SeekRel( nBytes );
        }
    }

    
    rStream.Seek( nStartPos );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ITEMS );
    bool bSecondaryLoaded = false;
    long nSecondaryEnd = 0;
    sal_uInt16 nWhich(0), nSlot(0);
    while ( rStream.ReadUInt16( nWhich ), nWhich )
    {
        
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream.ReadUInt16( nSlot );

        sal_uInt16 nRef(0), nCount(0), nVersion(0);
        sal_uInt32 nAttrSize(0);
        rStream.ReadUInt16( nVersion ).ReadUInt16( nCount );

        
        
        
        

        sal_uInt16 nIndex = GetIndex_Impl(nWhich);
        std::vector<SfxPoolItemArray_Impl*>::iterator ppArr = pImp->maPoolItems.begin();
        std::advance(ppArr, nIndex);
        SfxPoolItemArray_Impl *pNewArr = new SfxPoolItemArray_Impl();
        SfxPoolItem *pDefItem = *(pImp->ppStaticDefaults + nIndex);

        
        sal_uLong nLastPos = rStream.Tell();

        
        if ( !bSecondaryLoaded && pImp->mpSecondary && pDefItem->ISA(SfxSetItem) )
        {
            
            rStream.Seek(nEndOfSizes);
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );

            
            pImp->mpSecondary->Load1_Impl( rStream );
            bSecondaryLoaded = true;
            nSecondaryEnd = rStream.Tell();

            
            rStream.Seek(nLastPos);
        }

        
        for ( sal_uInt16 j = 0; j < nCount; ++j )
        {
            sal_uLong nPos = nLastPos;
            rStream.ReadUInt16( nRef );

            SfxPoolItem *pItem = 0;
            if ( nRef )
            {
                pItem = pDefItem->Create(rStream, nVersion);

                if ( !pImp->mbPersistentRefCounts )
                    
                    AddRef(*pItem, 1);
                else
                {
                    if ( nRef > SFX_ITEMS_OLD_MAXREF )
                        pItem->SetKind( nRef );
                    else
                        AddRef(*pItem, nRef);
                }
            }
            
            pNewArr->push_back( (SfxPoolItem*) pItem );

            
            nLastPos = rStream.Tell();

            aSizeTable.ReadUInt32( nAttrSize );
            SFX_ASSERT( ( nPos + nAttrSize) >= nLastPos,
                        nPos,
                        "too many bytes read - version mismatch?" );

            if (nLastPos < (nPos + nAttrSize))
            {
                nLastPos = nPos + nAttrSize;
                rStream.Seek( nLastPos );
            }
        }

        SfxPoolItemArray_Impl *pOldArr = *ppArr;
        *ppArr = pNewArr;

        
        bool bEmpty = true;
        if ( 0 != pOldArr )
            for ( size_t n = 0; bEmpty && n < pOldArr->size(); ++n )
                bEmpty = pOldArr->operator[](n) == 0;
        DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
        if ( !bEmpty )
        {
            
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

    
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer > 0 )
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_DEFAULTS );

    sal_uLong nLastPos = rStream.Tell();
    while ( rStream.ReadUInt16( nWhich ), nWhich )
    {
        
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream.ReadUInt16( nSlot );

        sal_uLong nPos = nLastPos;
        sal_uInt32 nSize(0);
        sal_uInt16 nVersion(0);
        rStream.ReadUInt16( nVersion );

        SfxPoolItem *pItem =
            ( *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
            ->Create( rStream, nVersion );
        pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
        *( pImp->ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;

        nLastPos = rStream.Tell();
        aSizeTable.ReadUInt32( nSize );
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

    pImp->bStreaming = false;
    return rStream;
}



const SfxPoolItem* SfxItemPool::LoadSurrogate
(
    SvStream&           rStream,    
    sal_uInt16&             rWhich,     
    sal_uInt16              nSlotId,    
    const SfxItemPool*  pRefPool    
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
    
    sal_uInt32 nSurrogat(0);
    rStream.ReadUInt32( nSurrogat );

    
    if ( SFX_ITEMS_DIRECT == nSurrogat )
        return 0;

    
    if ( SFX_ITEMS_NULL == nSurrogat )
    {
        rWhich = 0;
        return 0;
    }

    
    
    if ( !pRefPool )
        pRefPool = this;
    bool bResolvable = !pRefPool->GetName().isEmpty();
    if ( !bResolvable )
    {
        
        
        sal_uInt16 nMappedWhich = nSlotId ? GetWhich(nSlotId, true) : 0;
        if ( IsWhich(nMappedWhich) )
        {
            
            rWhich = nMappedWhich;
            bResolvable = true;
        }
    }

    
    if ( bResolvable )
    {
        const SfxPoolItem *pItem = 0;
        for ( SfxItemPool *pTarget = this; pTarget; pTarget = pTarget->pImp->mpSecondary )
        {
            
            if ( pTarget->IsInRange(rWhich) )
            {
                
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
                    rWhich = 0; 
                    return 0;
                }

                
                if ( pRefPool != pImp->mpMaster )
                    return &pTarget->Put( *pItem );

                
                if ( !pTarget->HasPersistentRefCounts() )
                    AddRef( *pItem, 1 );
                else
                    return pItem;

                return pItem;
            }
        }

        SFX_ASSERT( false, rWhich, "can't resolve Which-Id in LoadSurrogate" );
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
        rStream.WriteUInt32(  bRealSurrogate
                        ? GetSurrogate( pItem )
                        : SFX_ITEMS_DIRECT  );
        return bRealSurrogate;
    }

    rStream.WriteUInt32( SFX_ITEMS_NULL );
    return true;
}



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
        SFX_ASSERT( false, pItem->Which(), "unknown Which-Id - dont ask me for surrogates" );
    }

    
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
    SFX_ASSERT( false, pItem->Which(), "Item not in the pool");
    return SFX_ITEMS_NULL;
}



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
    
    const SfxPoolVersion_ImplPtr pVerMap = SfxPoolVersion_ImplPtr( new SfxPoolVersion_Impl(
                nVer, nOldStart, nOldEnd, pOldWhichIdTab ) );
    pImp->aVersions.push_back( pVerMap );

    DBG_ASSERT( nVer > pImp->nVersion, "Versions not sorted" );
    pImp->nVersion = nVer;

    
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



sal_uInt16 SfxItemPool::GetNewWhich
(
    sal_uInt16  nFileWhich      
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
    
    if ( !IsInVersionsRange(nFileWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetNewWhich( nFileWhich );
        SFX_ASSERT( false, nFileWhich, "unknown which in GetNewWhich()" );
    }

    
    short nDiff = (short)pImp->nLoadingVersion - (short)pImp->nVersion;

    
    if ( nDiff > 0 )
    {
        
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

    
    else if ( nDiff < 0 )
    {
        
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

    
    return nFileWhich;
}




bool SfxItemPool::IsInVersionsRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImp->nVerStart && nWhich <= pImp->nVerEnd;
}



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
        return false;
    const SfxItemPool *pPool = this;
    while ( !pPool->IsInStoringRange(rItem.Which()) )
        if ( 0 == ( pPool = pPool->pImp->mpSecondary ) )
            return false;

    DBG_ASSERT( !pImp->bInSetItem || !rItem.ISA(SfxSetItem),
                "SetItem contains ItemSet with SetItem" );

    sal_uInt16 nSlotId = pPool->GetSlotId( rItem.Which(), true );
    sal_uInt16 nItemVersion = rItem.GetVersion(pImp->mnFileFormatVersion);
    if ( USHRT_MAX == nItemVersion )
        return false;

    rStream.WriteUInt16( rItem.Which() ).WriteUInt16( nSlotId );
    if ( bDirect || !pPool->StoreSurrogate( rStream, &rItem ) )
    {
        rStream.WriteUInt16( nItemVersion );
        rStream.WriteUInt32( (sal_uInt32) 0L );           
        sal_uLong nIStart = rStream.Tell();
        rItem.Store(rStream, nItemVersion);
        sal_uLong nIEnd = rStream.Tell();
        rStream.Seek( nIStart-4 );
        rStream.WriteInt32( (sal_Int32) ( nIEnd-nIStart ) );
        rStream.Seek( nIEnd );
    }

    return true;
}



const SfxPoolItem* SfxItemPool::LoadItem( SvStream &rStream, bool bDirect,
                                          const SfxItemPool *pRefPool )



{
    sal_uInt16 nWhich(0), nSlot(0); 
    rStream.ReadUInt16( nWhich ).ReadUInt16( nSlot );

    bool bDontPut = (SfxItemPool*)-1 == pRefPool;
    if ( bDontPut || !pRefPool )
        pRefPool = this;

    
    while ( !pRefPool->IsInVersionsRange(nWhich) )
    {
        if ( pRefPool->pImp->mpSecondary )
            pRefPool = pRefPool->pImp->mpSecondary;
        else
        {
            
            sal_uInt32 nSurro(0);
            sal_uInt16 nVersion(0), nLen(0);
            rStream.ReadUInt32( nSurro );
            if ( SFX_ITEMS_DIRECT == nSurro )
            {
                rStream.ReadUInt16( nVersion ).ReadUInt16( nLen );
                rStream.SeekRel( nLen );
            }
            return 0;
        }
    }

    
    bool bCurVersion = pRefPool->IsCurrentVersionLoading();
    if ( !bCurVersion )
        
        nWhich = pRefPool->GetNewWhich( nWhich );

    DBG_ASSERT( !nWhich || !pImp->bInSetItem ||
                !pRefPool->pImp->ppStaticDefaults[pRefPool->GetIndex_Impl(nWhich)]->ISA(SfxSetItem),
                "loading SetItem in ItemSet of SetItem" );

    
    const SfxPoolItem *pItem = 0;
    if ( !bDirect )
    {
        
        if ( nWhich )
            
            pItem = LoadSurrogate( rStream, nWhich, nSlot, pRefPool );
        else
            
            rStream.SeekRel( sizeof(sal_uInt16) );
    }

    
    if ( bDirect || ( nWhich && !pItem ) )
    {
        
        sal_uInt16 nVersion(0);
        sal_uInt32 nLen(0);
        rStream.ReadUInt16( nVersion ).ReadUInt32( nLen );
        sal_uLong nIStart = rStream.Tell();

        
        if ( nWhich )
        {
            
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
            
            rStream.Seek( nIStart+nLen );
    }

    return pItem;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
