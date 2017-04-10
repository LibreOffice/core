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

#include <swcache.hxx>

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

#include <limits.h>

#ifdef DBG_UTIL
void SwCache::Check()
{
    if ( !m_pRealFirst )
        return;

    // consistency check
    SAL_WARN_IF( m_pLast->GetNext(), "sw.core", "Last but not last." );
    SAL_WARN_IF( m_pRealFirst->GetPrev(), "sw.core", "First but not first." );
    sal_uInt16 nCnt = 0;
    bool bFirstFound = false;
    SwCacheObj *pObj = m_pRealFirst;
    SwCacheObj *const pOldRealFirst = m_pRealFirst;
    while ( pObj )
    {
        // the object must be found also when moving backwards
        SwCacheObj *pTmp = m_pLast;
        while ( pTmp && pTmp != pObj )
            pTmp = pTmp->GetPrev();
        SAL_WARN_IF( !pTmp, "sw.core", "Object not found." );

        ++nCnt;
        if ( pObj == m_pFirst )
            bFirstFound = true;
        if ( !pObj->GetNext() )
            SAL_WARN_IF( pObj != m_pLast, "sw.core", "Last not Found." );
        pObj = pObj->GetNext();
        SAL_WARN_IF(pObj == pOldRealFirst, "sw.core", "Recursion in SwCache.");
    }
    SAL_WARN_IF( !bFirstFound, "sw.core", "First not Found." );
    SAL_WARN_IF( nCnt + m_aFreePositions.size() != size(), "sw.core", "Lost Chain." );
    SAL_WARN_IF(
        size() == m_nCurMax && m_nCurMax != m_aFreePositions.size() + nCnt, "sw.core",
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
    , const OString &rNm
#endif
    ) :
    m_aCacheObjects(),
    m_pRealFirst( nullptr ),
    m_pFirst( nullptr ),
    m_pLast( nullptr ),
    m_nCurMax( nInitSize )
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
    m_aCacheObjects.reserve( nInitSize );
}

SwCache::~SwCache()
{
#ifdef DBG_UTIL
    SAL_INFO(
        "sw.core",
        m_aName << "; number of new entries: " << m_nAppend
            << "; number of insert on free places: " << m_nInsertFree
            << "; number of replacements: " << m_nReplace
            << "; number of successful Get's: " << m_nGetSuccess
            << "; number of failed Get's: " << m_nGetFail
            << "; number or reordering (LRU): " << m_nToTop
            << "; number of suppressions: " << m_nDelete
            << "; number of Get's without Index: " << m_nGetSeek
            << "; number of Seek for Get without Index: " << m_nAverageSeekCnt
            << "; number of Flush calls: " << m_nFlushCnt
            << "; number of flushed objects: " << m_nFlushedObjects
            << "; number of Cache expansions: " << m_nIncreaseMax
            << "; number of Cache reductions: " << m_nDecreaseMax);
    Check();
#endif

    for(SwCacheObjArr::const_iterator it = m_aCacheObjects.begin(); it != m_aCacheObjects.end(); ++it)
        delete *it;
}

void SwCache::Flush()
{
    INCREMENT( m_nFlushCnt );
    SwCacheObj *pObj = m_pRealFirst;
    m_pRealFirst = m_pFirst = m_pLast = nullptr;
    SwCacheObj *pTmp;
    while ( pObj )
    {
#ifdef DBG_UTIL
        if ( pObj->IsLocked() )
        {
            OSL_FAIL( "Flushing locked objects." );
            if ( !m_pRealFirst )
            {
                m_pRealFirst = m_pFirst = m_pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( nullptr ); pObj->SetPrev( nullptr );
                pObj = pTmp;
            }
            else
            {   m_pLast->SetNext( pObj );
                pObj->SetPrev( m_pLast );
                m_pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( nullptr );
                pObj = pTmp;
            }
        }
        else
#endif
        {
            pTmp = pObj;
            pObj = pTmp->GetNext();
            m_aFreePositions.push_back( pTmp->GetCachePos() );
            m_aCacheObjects[pTmp->GetCachePos()] = nullptr;
            delete pTmp;
            INCREMENT( m_nFlushedObjects );
        }
    }
}

void SwCache::ToTop( SwCacheObj *pObj )
{
    INCREMENT( m_nToTop );

    // cut object out of chain and insert at beginning
    if ( m_pRealFirst == pObj )   // pFirst was checked by caller
    {
        CHECK;
        return;
    }

    if ( !m_pRealFirst )
    {
        // the first will be inserted
        OSL_ENSURE( !m_pFirst && !m_pLast, "First not first." );
        m_pRealFirst = m_pFirst = m_pLast = pObj;
        CHECK;
        return;
    }

    // cut
    if ( pObj == m_pLast )
    {
        OSL_ENSURE( pObj->GetPrev(), "Last but no Prev." );
        m_pLast = pObj->GetPrev();
        m_pLast->SetNext( nullptr );
    }
    else
    {
        if ( pObj->GetNext() )
            pObj->GetNext()->SetPrev( pObj->GetPrev() );
        if ( pObj->GetPrev() )
            pObj->GetPrev()->SetNext( pObj->GetNext() );
    }

    // paste at the (virtual) beginning
    if ( m_pRealFirst == m_pFirst )
    {
        m_pRealFirst->SetPrev( pObj );
        pObj->SetNext( m_pRealFirst );
        pObj->SetPrev( nullptr );
        m_pRealFirst = m_pFirst = pObj;
        CHECK;
    }
    else
    {
        OSL_ENSURE( m_pFirst, "ToTop, First is not RealFirst an Empty." );

        if ( m_pFirst->GetPrev() )
        {
            m_pFirst->GetPrev()->SetNext( pObj );
            pObj->SetPrev( m_pFirst->GetPrev() );
        }
        else
            pObj->SetPrev( nullptr );
        m_pFirst->SetPrev( pObj );
        pObj->SetNext( m_pFirst );
        m_pFirst = pObj;
        CHECK;
    }
}

SwCacheObj *SwCache::Get( const void *pOwner, const sal_uInt16 nIndex,
                          const bool bToTop )
{
    SwCacheObj *pRet;
    if ( nullptr != (pRet = (nIndex < m_aCacheObjects.size()) ? m_aCacheObjects[ nIndex ] : nullptr) )
    {
        if ( !pRet->IsOwner( pOwner ) )
            pRet = nullptr;
        else if ( bToTop && pRet != m_pFirst )
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

SwCacheObj *SwCache::Get( const void *pOwner, const bool bToTop )
{
    SwCacheObj *pRet = m_pRealFirst;
    while ( pRet && !pRet->IsOwner( pOwner ) )
    {
        INCREMENT( m_nAverageSeekCnt );
        pRet = pRet->GetNext();
    }

    if ( bToTop && pRet && pRet != m_pFirst )
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

    if ( m_pFirst == pObj )
    {
        if ( m_pFirst->GetNext() )
            m_pFirst = m_pFirst->GetNext();
        else
            m_pFirst = m_pFirst->GetPrev();
    }
    if ( m_pRealFirst == pObj )
        m_pRealFirst = m_pRealFirst->GetNext();
    if ( m_pLast == pObj )
        m_pLast = m_pLast->GetPrev();
    if ( pObj->GetPrev() )
        pObj->GetPrev()->SetNext( pObj->GetNext() );
    if ( pObj->GetNext() )
        pObj->GetNext()->SetPrev( pObj->GetPrev() );

    m_aFreePositions.push_back( pObj->GetCachePos() );
    m_aCacheObjects[pObj->GetCachePos()] = nullptr;
    delete pObj;

    CHECK;
    if ( m_aCacheObjects.size() > m_nCurMax &&
         (m_nCurMax <= (m_aCacheObjects.size() - m_aFreePositions.size())) )
    {
        // Shrink if possible.To do so we need enough free positions.
        // Unpleasant side effect: positions will be moved and the owner of
        // these might not find them afterwards
        for ( size_t i = 0; i < m_aCacheObjects.size(); ++i )
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
        m_aFreePositions.clear();
    }
    CHECK;
}

void SwCache::Delete( const void *pOwner )
{
    INCREMENT( m_nDelete );
    SwCacheObj *pObj;
    if ( nullptr != (pObj = Get( pOwner, false )) )
        DeleteObj( pObj );
}

bool SwCache::Insert( SwCacheObj *pNew )
{
    CHECK;
    OSL_ENSURE( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );

    sal_uInt16 nPos;
    if ( m_aCacheObjects.size() < m_nCurMax )
    {
        // there is still space; insert directly
        INCREMENT( m_nAppend );
        nPos = m_aCacheObjects.size();
        m_aCacheObjects.push_back(pNew);
    }
    else if ( !m_aFreePositions.empty() )
    {
        // there are placeholders; use the last of those
        INCREMENT( m_nInsertFree );
        const sal_uInt16 nFreePos = m_aFreePositions.size() - 1;
        nPos = m_aFreePositions[ nFreePos ];
        m_aCacheObjects[nPos] = pNew;
        m_aFreePositions.erase( m_aFreePositions.begin() + nFreePos );
    }
    else
    {
        INCREMENT( m_nReplace );
        // the last of the LRU has to go
        SwCacheObj *pObj = m_pLast;

        while ( pObj && pObj->IsLocked() )
            pObj = pObj->GetPrev();
        if ( !pObj )
        {
            OSL_FAIL( "Cache overflow." );
            return false;
        }

        nPos = pObj->GetCachePos();
        if ( pObj == m_pLast )
        { OSL_ENSURE( pObj->GetPrev(), "Last but no Prev" );
            m_pLast = pObj->GetPrev();
            m_pLast->SetNext( nullptr );
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

    if ( m_pFirst )
    {
        if ( m_pFirst->GetPrev() )
        {   m_pFirst->GetPrev()->SetNext( pNew );
            pNew->SetPrev( m_pFirst->GetPrev() );
        }
        m_pFirst->SetPrev( pNew );
        pNew->SetNext( m_pFirst );
    }
    else
    { OSL_ENSURE( !m_pLast, "Last but no First." );
        m_pLast = pNew;
    }
    if ( m_pFirst == m_pRealFirst )
        m_pRealFirst = pNew;
    m_pFirst = pNew;

    CHECK;
    return true;
}

void SwCache::SetLRUOfst( const sal_uInt16 nOfst )
{
    if ( !m_pRealFirst || ((m_aCacheObjects.size() - m_aFreePositions.size()) < nOfst) )
        return;

    CHECK;
    m_pFirst = m_pRealFirst;
    for ( sal_uInt16 i = 0; i < m_aCacheObjects.size() && i < nOfst; ++i )
    {
        if ( m_pFirst->GetNext() && m_pFirst->GetNext()->GetNext() )
            m_pFirst = m_pFirst->GetNext();
        else
            break;
    }
    CHECK;
}

SwCacheObj::SwCacheObj( const void *pOwn ) :
    m_pNext( nullptr ),
    m_pPrev( nullptr ),
    m_nCachePos( USHRT_MAX ),
    m_nLock( 0 ),
    m_pOwner( pOwn )
{
}

SwCacheObj::~SwCacheObj()
{
}

#ifdef DBG_UTIL
void SwCacheObj::Lock()
{
    OSL_ENSURE( m_nLock < UCHAR_MAX, "Too many Locks for CacheObject." );
    ++m_nLock;
}

void SwCacheObj::Unlock()
{
    OSL_ENSURE( m_nLock, "No more Locks available." );
    --m_nLock;
}
#endif

SwCacheAccess::~SwCacheAccess()
{
    if ( m_pObj )
        m_pObj->Unlock();
}

void SwCacheAccess::Get_()
{
    OSL_ENSURE( !m_pObj, "SwCacheAcces Obj already available." );

    m_pObj = NewObj();
    if ( !m_rCache.Insert( m_pObj ) )
    {
        delete m_pObj;
        m_pObj = nullptr;
    }
    else
    {
        m_pObj->Lock();
    }
}

bool SwCacheAccess::IsAvailable() const
{
    return m_pObj != nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
