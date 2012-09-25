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

#include <rtl/strbuf.hxx>
#include <swcache.hxx>
#include <limits.h>     // USHRT_MAX

#ifdef DBG_UTIL
void SwCache::Check()
{
    if ( !pRealFirst )
        return;

    // consistency check
    SAL_WARN_IF( pLast->GetNext(), "sw", "Last but not last." );
    SAL_WARN_IF( pRealFirst->GetPrev(), "sw", "First but not first." );
    sal_uInt16 nCnt = 0;
    bool bFirstFound = false;
    SwCacheObj *pObj = pRealFirst;
    SwCacheObj *pRekursive = pObj;
    while ( pObj )
    {
        // the object must be found also when moving backwards
        SwCacheObj *pTmp = pLast;
        while ( pTmp && pTmp != pObj )
            pTmp = pTmp->GetPrev();
        SAL_WARN_IF( !pTmp, "sw", "Objekt not found." );

        ++nCnt;
        if ( pObj == pFirst )
            bFirstFound = true;
        if ( !pObj->GetNext() )
            SAL_WARN_IF( pObj != pLast, "sw", "Last not Found." );
        pObj = pObj->GetNext();
        SAL_WARN_IF( pObj == pRekursive, "sw", "Recursion in SwCache." );
    }
    SAL_WARN_IF( !bFirstFound, "sw", "First not Found." );
    SAL_WARN_IF( nCnt + aFreePositions.size() != size(), "sw", "Lost Chain." );
    SAL_WARN_IF(
        size() == nCurMax && nCurMax != aFreePositions.size() + nCnt, "sw",
        "Lost FreePositions." );
}

#define INCREMENT( nVar )   ++nVar
#define CHECK Check();

#else
#define INCREMENT( nVar )
#define CHECK
#endif


SwCache::SwCache( const sal_uInt16 nInitSize
#ifdef DBG_UTIL
    , const rtl::OString &rNm
#endif
    ) :
    m_aCacheObjects(),
    pRealFirst( 0 ),
    pFirst( 0 ),
    pLast( 0 ),
    nCurMax( nInitSize )
#ifdef DBG_UTIL
    , m_aName( rNm )
    , m_nAppend( 0 )
    , m_nInsertFree( 0 )
    , m_nReplace( 0 )
    , m_nGetSuccess( 0 )
    , m_nGetFail( 0 )
    , m_nToTop( 0 )
    , m_nDelete( 0 )
    , m_nGetSeek( 0 )
    , m_nAverageSeekCnt( 0 )
    , m_nFlushCnt( 0 )
    , m_nFlushedObjects( 0 )
    , m_nIncreaseMax( 0 )
    , m_nDecreaseMax( 0 )
#endif
{
    m_aCacheObjects.reserve( (sal_uInt8)nInitSize );
}

SwCache::~SwCache()
{
#ifdef DBG_UTIL
    {
        rtl::OStringBuffer sOut(m_aName);

        sOut.append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of new entries:                 ")).
            append(static_cast<sal_Int32>(m_nAppend)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of insert on free places:       ")).
            append(static_cast<sal_Int32>(m_nInsertFree)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of replacements:                ")).
            append(static_cast<sal_Int32>(m_nReplace)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of successful Get's:            ")).
            append(static_cast<sal_Int32>(m_nGetSuccess)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of failed Get's:                ")).
            append(static_cast<sal_Int32>(m_nGetFail)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number or reordering (LRU):            ")).
            append(static_cast<sal_Int32>(m_nToTop)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of suppressions:                ")).
            append(static_cast<sal_Int32>(m_nDelete)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Get's without Index:         ")).
            append(static_cast<sal_Int32>(m_nGetSeek)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Seek for Get without Index:  ")).
            append(static_cast<sal_Int32>(m_nAverageSeekCnt)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Flush calls:                 " )).
            append(static_cast<sal_Int32>(m_nFlushCnt)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of flushed objects:             ")).
            append(static_cast<sal_Int32>(m_nFlushedObjects)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Cache expansions:            ")).
            append(static_cast<sal_Int32>(m_nIncreaseMax)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Cache reductions:            ")).
            append(static_cast<sal_Int32>(m_nDecreaseMax)).
            append('\n');

        OSL_TRACE(sOut.getStr());
    }
    Check();
#endif

    for(SwCacheObjArr::const_iterator it = m_aCacheObjects.begin(); it != m_aCacheObjects.end(); ++it)
        delete *it;
}

void SwCache::Flush( const sal_uInt8 )
{
    INCREMENT( m_nFlushCnt );
    SwCacheObj *pObj = pRealFirst;
    pRealFirst = pFirst = pLast = 0;
    SwCacheObj *pTmp;
    while ( pObj )
    {
#ifdef DBG_UTIL
        if ( pObj->IsLocked() )
        {
            OSL_FAIL( "Flushing locked objects." );
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
            aFreePositions.push_back( pTmp->GetCachePos() );
            m_aCacheObjects[pTmp->GetCachePos()] = NULL;
            delete pTmp;
            INCREMENT( m_nFlushedObjects );
        }
    }
}

void SwCache::ToTop( SwCacheObj *pObj )
{
    INCREMENT( m_nToTop );

    // cut object out of chain and insert at beginning
    if ( pRealFirst == pObj )   // pFirst was checked by caller
    {
        CHECK;
        return;
    }

    if ( !pRealFirst )
    {
        // the first will be inserted
        OSL_ENSURE( !pFirst && !pLast, "First not first." );
        pRealFirst = pFirst = pLast = pObj;
        CHECK;
        return;
    }

    // cut
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

    // paste at the (virtual) beginning
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

SwCacheObj *SwCache::Get( const void *pOwner, const sal_uInt16 nIndex,
                          const sal_Bool bToTop )
{
    SwCacheObj *pRet;
    if ( 0 != (pRet = nIndex < m_aCacheObjects.size() ? m_aCacheObjects[ nIndex ] : 0) )
    {
        if ( !pRet->IsOwner( pOwner ) )
            pRet = 0;
        else if ( bToTop && pRet != pFirst )
            ToTop( pRet );
    }

#ifdef DBG_UTIL
        if ( pRet )
            ++m_nGetSuccess;
        else
            ++m_nGetFail;
#endif

    return pRet;
}

SwCacheObj *SwCache::Get( const void *pOwner, const sal_Bool bToTop )
{
    SwCacheObj *pRet = pRealFirst;
    while ( pRet && !pRet->IsOwner( pOwner ) )
    {
        INCREMENT( m_nAverageSeekCnt );
        pRet = pRet->GetNext();
    }

    if ( bToTop && pRet && pRet != pFirst )
        ToTop( pRet );

#ifdef DBG_UTIL
    if ( pRet )
        ++m_nGetSuccess;
    else
        ++m_nGetFail;
    ++m_nGetSeek;
#endif
    return pRet;
}

void SwCache::DeleteObj( SwCacheObj *pObj )
{
    CHECK;
    OSL_ENSURE( !pObj->IsLocked(), "SwCache::Delete: object is locked." );
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

    aFreePositions.push_back( pObj->GetCachePos() );
    m_aCacheObjects[pObj->GetCachePos()] = NULL;
    delete pObj;

    CHECK;
    if ( m_aCacheObjects.size() > nCurMax &&
         (nCurMax <= (m_aCacheObjects.size() - aFreePositions.size())) )
    {
        // Shrink if possible.To do so we need enough free positions.
        // Unpleasent side effect: positions will be moved and the owner of
        // these might not find them afterwards
        for ( sal_uInt16 i = 0; i < m_aCacheObjects.size(); ++i )
        {
            SwCacheObj *pTmpObj = m_aCacheObjects[i];
            if ( !pTmpObj )
            {   m_aCacheObjects.erase( m_aCacheObjects.begin() + i );
                --i;
            }
            else
            {
                pTmpObj->SetCachePos( i );
            }
        }
        aFreePositions.clear();
    }
    CHECK;
}

void SwCache::Delete( const void *pOwner )
{
    INCREMENT( m_nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, sal_Bool(sal_False) )) )
        DeleteObj( pObj );
}

sal_Bool SwCache::Insert( SwCacheObj *pNew )
{
    CHECK;
    OSL_ENSURE( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );

    sal_uInt16 nPos;
    if ( m_aCacheObjects.size() < nCurMax )
    {
        // there is still space; insert directly
        INCREMENT( m_nAppend );
        nPos = m_aCacheObjects.size();
        m_aCacheObjects.push_back(pNew);
    }
    else if ( !aFreePositions.empty() )
    {
        // there are placeholders; use the last of those
        INCREMENT( m_nInsertFree );
        const sal_uInt16 nFreePos = aFreePositions.size() - 1;
        nPos = aFreePositions[ nFreePos ];
        m_aCacheObjects[nPos] = pNew;
        aFreePositions.erase( aFreePositions.begin() + nFreePos );
    }
    else
    {
        INCREMENT( m_nReplace );
        // the last of the LRU has to go
        SwCacheObj *pObj = pLast;

        while ( pObj && pObj->IsLocked() )
            pObj = pObj->GetPrev();
        if ( !pObj )
        {
            OSL_FAIL( "Cache overflow." );
            return sal_False;
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
        m_aCacheObjects[nPos] = pNew;
    }
    pNew->SetCachePos( nPos );

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
    return sal_True;
}

void SwCache::SetLRUOfst( const sal_uInt16 nOfst )
{
    if ( !pRealFirst || ((m_aCacheObjects.size() - aFreePositions.size()) < nOfst) )
        return;

    CHECK;
    pFirst = pRealFirst;
    for ( sal_uInt16 i = 0; i < m_aCacheObjects.size() && i < nOfst; ++i )
    {
        if ( pFirst->GetNext() && pFirst->GetNext()->GetNext() )
            pFirst = pFirst->GetNext();
        else
            break;
    }
    CHECK;
}

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

#ifdef DBG_UTIL
void SwCacheObj::Lock()
{
    OSL_ENSURE( nLock < UCHAR_MAX, "Too many Locks for CacheObject." );
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
    {
        pObj->Lock();
    }
}

sal_Bool SwCacheAccess::IsAvailable() const
{
    return pObj != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
