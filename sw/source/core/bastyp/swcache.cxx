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
#include "precompiled_sw.hxx"

#include <tools/debug.hxx>
#include <errhdl.hxx>
#include <swcache.hxx>

SV_IMPL_PTRARR(SwCacheObjArr,SwCacheObj*);

#if OSL_DEBUG_LEVEL > 1
#define INCREMENT( nVar )   ++nVar
#else
#define INCREMENT( nVar )
#endif

/*************************************************************************
|*  SwCache::Check()
|*************************************************************************/

#if OSL_DEBUG_LEVEL > 1

void SwCache::Check()
{
    if ( !pRealFirst )
        return;

    //Konsistenspruefung.
    OSL_ENSURE( !pLast->GetNext(), "Last but not last." );
    OSL_ENSURE( !pRealFirst->GetPrev(), "First but not first." );
    USHORT nCnt = 0;
    BOOL bFirstFound = FALSE;
    SwCacheObj *pObj = pRealFirst;
    SwCacheObj *pRekursive = pObj;
    while ( pObj )
    {
        //Das Objekt muss auch auf dem Rueckwaertsweg gefunden werden.
        SwCacheObj *pTmp = pLast;
        while ( pTmp && pTmp != pObj )
            pTmp = pTmp->GetPrev();
        OSL_ENSURE( pTmp, "Objekt not found." );

        ++nCnt;
        if ( pObj == pFirst )
            bFirstFound = TRUE;
        if ( !pObj->GetNext() )
            OSL_ENSURE( pObj == pLast, "Last not Found." );
        pObj = pObj->GetNext();
        OSL_ENSURE( pObj != pRekursive, "Recursion in SwCache." );
    }
    OSL_ENSURE( bFirstFound, "First not Found." );
    OSL_ENSURE( (nCnt + aFreePositions.Count()) == Count(), "Lost Chain." );
    if ( Count() == nCurMax )
        OSL_ENSURE( (nCurMax - nCnt) == aFreePositions.Count(), "Lost FreePositions." );
}
#endif

#if OSL_DEBUG_LEVEL > 1
#define CHECK Check();
#else
#define CHECK
#endif

/*************************************************************************
|*  SwCache::SwCache(), ~SwCache()
|*************************************************************************/


SwCache::SwCache( const USHORT nInitSize, const USHORT nGrowSize
#if OSL_DEBUG_LEVEL > 1
    , const ByteString &rNm
#endif
    ) :
    SwCacheObjArr( (BYTE)nInitSize, (BYTE)nGrowSize ),
    aFreePositions( 5, 5 ),
    pRealFirst( 0 ),
    pFirst( 0 ),
    pLast( 0 ),
    nMax( nInitSize ),
    nCurMax( nInitSize )
#if OSL_DEBUG_LEVEL > 1
    , aName( rNm ),
    nAppend( 0 ),
    nInsertFree( 0 ),
    nReplace( 0 ),
    nGetSuccess( 0 ),
    nGetFail( 0 ),
    nToTop( 0 ),
    nDelete( 0 ),
    nGetSeek( 0 ),
    nAverageSeekCnt( 0 ),
    nFlushCnt( 0 ),
    nFlushedObjects( 0 ),
    nIncreaseMax( 0 ),
    nDecreaseMax( 0 )
#endif
{
}

#if OSL_DEBUG_LEVEL > 1
SwCache::~SwCache()
{
    {
        ByteString sOut( aName ); sOut += '\n';
        (( sOut += "Anzahl neuer Eintraege:             " )
                    += ByteString::CreateFromInt32( nAppend ))+= '\n';
        (( sOut += "Anzahl Insert auf freie Plaetze:    " )
                    += ByteString::CreateFromInt32( nInsertFree ))+= '\n';
        (( sOut += "Anzahl Ersetzungen:                 " )
                    += ByteString::CreateFromInt32( nReplace ))+= '\n';
        (( sOut += "Anzahl Erfolgreicher Get's:         " )
                    += ByteString::CreateFromInt32( nGetSuccess ))+= '\n';
        (( sOut += "Anzahl Fehlgeschlagener Get's:      " )
                    += ByteString::CreateFromInt32( nGetFail ))+= '\n';
        (( sOut += "Anzahl Umsortierungen (LRU):        " )
                    += ByteString::CreateFromInt32( nToTop ))+= '\n';
        (( sOut += "Anzahl Loeschungen:                 " )
                    += ByteString::CreateFromInt32( nDelete ))+= '\n';
        (( sOut += "Anzahl Get's ohne Index:            " )
                    += ByteString::CreateFromInt32( nGetSeek ))+= '\n';
        (( sOut += "Anzahl Seek fuer Get ohne Index:    " )
                    += ByteString::CreateFromInt32( nAverageSeekCnt ))+= '\n';
        (( sOut += "Anzahl Flush-Aufrufe:               " )
                    += ByteString::CreateFromInt32( nFlushCnt ))+= '\n';
        (( sOut += "Anzahl geflush'ter Objekte:         " )
                    += ByteString::CreateFromInt32( nFlushedObjects ))+= '\n';
        (( sOut += "Anzahl Cache-Erweiterungen:         " )
                    += ByteString::CreateFromInt32( nIncreaseMax ))+= '\n';
        (( sOut += "Anzahl Cache-Verkleinerungen:       " )
                    += ByteString::CreateFromInt32( nDecreaseMax ))+= '\n';

        DBG_ERROR( sOut.GetBuffer() );
    }
    Check();
}
#endif

/*************************************************************************
|*  SwCache::Flush()
|*************************************************************************/


void SwCache::Flush( const BYTE )
{
    INCREMENT( nFlushCnt );
    SwCacheObj *pObj = pRealFirst;
    pRealFirst = pFirst = pLast = 0;
    SwCacheObj *pTmp;
    while ( pObj )
    {
#if OSL_DEBUG_LEVEL > 1
        if ( pObj->IsLocked() )
        {
            OSL_ENSURE( TRUE, "Flushing locked objects." );
            if ( !pRealFirst )
            {
                pRealFirst = pFirst = pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 ); pObj->SetPrev( 0 );
                pObj = pTmp;
            }
            else
            {   pLast->SetNext( pObj );
                pObj->SetPrev( pLast );
                pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 );
                pObj = pTmp;
            }
        }
        else
#endif
        {
            pTmp = (SwCacheObj*)pObj;
            pObj = pTmp->GetNext();
            aFreePositions.Insert( pTmp->GetCachePos(), aFreePositions.Count() );
            *(pData + pTmp->GetCachePos()) = (void*)0;
            delete pTmp;
            INCREMENT( nFlushedObjects );
        }
    }
}

/*************************************************************************
|*  SwCache::ToTop()
|*************************************************************************/


void SwCache::ToTop( SwCacheObj *pObj )
{
    INCREMENT( nToTop );

    //Objekt aus der LRU-Kette ausschneiden und am Anfang einfuegen.
    if ( pRealFirst == pObj )   //pFirst wurde vom Aufrufer geprueft!
    {   CHECK;
        return;
    }

    if ( !pRealFirst )
    {   //Der erste wird eingetragen.
        OSL_ENSURE( !pFirst && !pLast, "First not first." );
        pRealFirst = pFirst = pLast = pObj;
        CHECK;
        return;
    }

    //Ausschneiden.
    if ( pObj == pLast )
    {
        OSL_ENSURE( pObj->GetPrev(), "Last but no Prev." );
        pLast = pObj->GetPrev();
        pLast->SetNext( 0 );
    }
    else
    {
        if ( pObj->GetNext() )
            pObj->GetNext()->SetPrev( pObj->GetPrev() );
        if ( pObj->GetPrev() )
            pObj->GetPrev()->SetNext( pObj->GetNext() );
    }

    //Am (virtuellen) Anfang einfuegen.
    if ( pRealFirst == pFirst )
    {
        pRealFirst->SetPrev( pObj );
        pObj->SetNext( pRealFirst );
        pObj->SetPrev( 0 );
        pRealFirst = pFirst = pObj;
        CHECK;
    }
    else
    {
        OSL_ENSURE( pFirst, "ToTop, First ist not RealFirst an Empty." );

        if ( pFirst->GetPrev() )
        {
            pFirst->GetPrev()->SetNext( pObj );
            pObj->SetPrev( pFirst->GetPrev() );
        }
        else
            pObj->SetPrev( 0 );
        pFirst->SetPrev( pObj );
        pObj->SetNext( pFirst );
        pFirst = pObj;
        CHECK;
    }
}

/*************************************************************************
|*  SwCache::Get()
|*************************************************************************/


SwCacheObj *SwCache::Get( const void *pOwner, const USHORT nIndex,
                          const BOOL bToTop )
{
    SwCacheObj *pRet;
    if ( 0 != (pRet = nIndex < Count() ? operator[]( nIndex ) : 0) )
    {
        if ( !pRet->IsOwner( pOwner ) )
            pRet = 0;
        else if ( bToTop && pRet != pFirst )
            ToTop( pRet );
    }

#if OSL_DEBUG_LEVEL > 1
        if ( pRet )
            ++nGetSuccess;
        else
            ++nGetFail;
#endif

    return pRet;
}



SwCacheObj *SwCache::Get( const void *pOwner, const BOOL bToTop )
{
    SwCacheObj *pRet = pRealFirst;
    while ( pRet && !pRet->IsOwner( pOwner ) )
    {
        INCREMENT( nAverageSeekCnt );
        pRet = pRet->GetNext();
    }

    if ( bToTop && pRet && pRet != pFirst )
        ToTop( pRet );

#if OSL_DEBUG_LEVEL > 1
    if ( pRet )
        ++nGetSuccess;
    else
        ++nGetFail;
    ++nGetSeek;
#endif
    return pRet;
}

/*************************************************************************
|*  SwCache::Delete()
|*************************************************************************/


void SwCache::DeleteObj( SwCacheObj *pObj )
{
    CHECK;
    OSL_ENSURE( !pObj->IsLocked(), "SwCache::Delete: Object ist Locked." );
    if ( pObj->IsLocked() )
        return;

    if ( pFirst == pObj )
    {
        if ( pFirst->GetNext() )
            pFirst = pFirst->GetNext();
        else
            pFirst = pFirst->GetPrev();
    }
    if ( pRealFirst == pObj )
        pRealFirst = pRealFirst->GetNext();
    if ( pLast == pObj )
        pLast = pLast->GetPrev();
    if ( pObj->GetPrev() )
        pObj->GetPrev()->SetNext( pObj->GetNext() );
    if ( pObj->GetNext() )
        pObj->GetNext()->SetPrev( pObj->GetPrev() );

    aFreePositions.Insert( pObj->GetCachePos(), aFreePositions.Count() );
    *(pData + pObj->GetCachePos()) = (void*)0;
    delete pObj;

    CHECK;
    if ( Count() > nCurMax &&
         (nCurMax <= (Count() - aFreePositions.Count())) )
    {
        //Falls moeglich wieder verkleinern, dazu muessen allerdings ausreichend
        //Freie Positionen bereitstehen.
        //Unangenehmer Nebeneffekt ist, das die Positionen verschoben werden
        //muessen, und die Eigentuemer der Objekte diese wahrscheinlich nicht
        //wiederfinden werden.
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwCacheObj *pTmpObj = operator[](i);
            if ( !pTmpObj )
            {   SwCacheObjArr::Remove( i, 1 );
                --i;
            }
            else
                pTmpObj->SetCachePos( i );
        }
        aFreePositions.Remove( 0, aFreePositions.Count() );
    }
    CHECK;
}

void SwCache::Delete( const void *pOwner )
{
    INCREMENT( nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, BOOL(FALSE) )) )
        DeleteObj( pObj );
}


/*************************************************************************
|*  SwCache::Insert()
|*************************************************************************/


BOOL SwCache::Insert( SwCacheObj *pNew )
{
    CHECK;
    OSL_ENSURE( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );

    USHORT nPos;//Wird hinter den if's zum setzen am Obj benutzt.
    if ( Count() < nCurMax )
    {
        //Es ist noch Platz frei, also einfach einfuegen.
        INCREMENT( nAppend );
        nPos = Count();
        SwCacheObjArr::C40_INSERT( SwCacheObj, pNew, nPos );
    }
    else if ( aFreePositions.Count() )
    {
        //Es exitieren Platzhalter, also den letzten benutzen.
        INCREMENT( nInsertFree );
        const USHORT nFreePos = aFreePositions.Count() - 1;
        nPos = aFreePositions[ nFreePos ];
        *(pData + nPos) = pNew;
        aFreePositions.Remove( nFreePos );
    }
    else
    {
        INCREMENT( nReplace );
        //Der letzte des LRU fliegt raus.
        SwCacheObj *pObj = pLast;

        while ( pObj && pObj->IsLocked() )
            pObj = pObj->GetPrev();
        if ( !pObj )
        {
            OSL_ENSURE( FALSE, "Cache overflow." );
            return FALSE;
        }

        nPos = pObj->GetCachePos();
        if ( pObj == pLast )
        { OSL_ENSURE( pObj->GetPrev(), "Last but no Prev" );
            pLast = pObj->GetPrev();
            pLast->SetNext( 0 );
        }
        else
        {
            if ( pObj->GetPrev() )
                pObj->GetPrev()->SetNext( pObj->GetNext() );
            if ( pObj->GetNext() )
                pObj->GetNext()->SetPrev( pObj->GetPrev() );
        }
        delete pObj;
        *(pData + nPos) = pNew;
    }
    pNew->SetCachePos( nPos );

    //Anstelle von ToTop, einfach als pFirst einfuegen.
//  ToTop( nPos );
    if ( pFirst )
    {
        if ( pFirst->GetPrev() )
        {   pFirst->GetPrev()->SetNext( pNew );
            pNew->SetPrev( pFirst->GetPrev() );
        }
        pFirst->SetPrev( pNew );
        pNew->SetNext( pFirst );
    }
    else
    { OSL_ENSURE( !pLast, "Last but no First." );
        pLast = pNew;
    }
    if ( pFirst == pRealFirst )
        pRealFirst = pNew;
    pFirst = pNew;

    CHECK;
    return TRUE;
}

/*************************************************************************
|*  SwCache::SetLRUOfst()
|*************************************************************************/


void SwCache::SetLRUOfst( const USHORT nOfst )
{
    if ( !pRealFirst || ((Count() - aFreePositions.Count()) < nOfst) )
        return;

    CHECK;
    pFirst = pRealFirst;
    for ( USHORT i = 0; i < Count() && i < nOfst; ++i )
    {
        if ( pFirst->GetNext() && pFirst->GetNext()->GetNext() )
            pFirst = pFirst->GetNext();
        else
            break;
    }
    CHECK;
}

/*************************************************************************
|*  SwCacheObj::SwCacheObj()
|*************************************************************************/


SwCacheObj::SwCacheObj( const void *pOwn ) :
    pNext( 0 ),
    pPrev( 0 ),
    nCachePos( USHRT_MAX ),
    nLock( 0 ),
    pOwner( pOwn )
{
}



SwCacheObj::~SwCacheObj()
{
}

/*************************************************************************
|*  SwCacheObj::SetLock(), Unlock()
|*************************************************************************/

#if OSL_DEBUG_LEVEL > 1

void SwCacheObj::Lock()
{
    OSL_ENSURE( nLock < UCHAR_MAX, "To many Locks for CacheObject." );
    ++nLock;
}



void SwCacheObj::Unlock()
{
    OSL_ENSURE( nLock, "No more Locks available." );
    --nLock;
}
#endif


SwCacheAccess::~SwCacheAccess()
{
    if ( pObj )
        pObj->Unlock();
}

/*************************************************************************
|*  SwCacheAccess::Get()
|*************************************************************************/

void SwCacheAccess::_Get()
{
    OSL_ENSURE( !pObj, "SwCacheAcces Obj already available." );

    pObj = NewObj();
    if ( !rCache.Insert( pObj ) )
    {
        delete pObj;
        pObj = 0;
    }
    else
        pObj->Lock();
}

/*************************************************************************
|*  SwCacheAccess::IsAvailable()
|*************************************************************************/

BOOL SwCacheAccess::IsAvailable() const
{
    return pObj != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
