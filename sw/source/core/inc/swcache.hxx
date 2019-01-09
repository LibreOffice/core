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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWCACHE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWCACHE_HXX

/**
 * Here, we manage pointers in a simple PtrArray to objects.
 * These objects are created (using new) in cache access classes; they are
 * destroyed by the cache.
 *
 * One can access these objects by array index or by searching in the array.
 * If you access it by index, managing the index is the responsibility of
 * the cache user.
 *
 * The cached objects are derrived from the base class SwCacheObj.
 * In it, the cache objects are doubly-linked which allows for the use of
 * an LRU algorithm.
 *
 * The LRU algorithm can be changed in the base class, by setting a virtual
 * First Pointer. It can be set to the first real one plus an offset.
 * By doing so we can protect the start area of the cache and make sure we
 * don't mess up the cache during some special operations.
 * E.g.: the Idle Handler should not destroy the cache for the visible area.
 *
 * The cache can be grown and shrunk in size.
 * E.g.: The cache for FormatInfo is grown for every new Shell and shrunk
 * when destroying them.
 */

#include <memory>
#include <vector>

#include <rtl/ustring.hxx>

class SwCacheObj;

class SwCache
{
    std::vector<std::unique_ptr<SwCacheObj>> m_aCacheObjects;
    std::vector<sal_uInt16> m_aFreePositions; /// Free positions for the Insert if the maximum has not been reached
                                            /// Every time an object is deregistered, its position is added here
    SwCacheObj *m_pRealFirst;                 /// _ALWAYS_ the real first LRU
    SwCacheObj *m_pFirst;                     /// The virtual first
    SwCacheObj *m_pLast;

    sal_uInt16 m_nCurMax;                     // Maximum of accepted objects

    void DeleteObj( SwCacheObj *pObj );

#ifdef DBG_UTIL
    OString m_aName;
    long m_nAppend;           /// number of entries appended
    long m_nInsertFree;       /// number of entries inserted on freed position
    long m_nReplace;          /// number of LRU replacements
    long m_nGetSuccess;
    long m_nGetFail;
    long m_nToTop;            /// number of reordering (LRU)
    long m_nDelete;           /// number of explicit deletes
    long m_nGetSeek;          /// number of gets without index
    long m_nAverageSeekCnt;   /// number of seeks for all gets without index
    long m_nFlushCnt;         /// number of flush calls
    long m_nFlushedObjects;
    long m_nIncreaseMax;      /// number of cache size increases
    long m_nDecreaseMax;      /// number of cache size decreases

    void Check();
#endif

public:

// Only add sal_uInt8!!!
#ifdef DBG_UTIL
    SwCache( const sal_uInt16 nInitSize, const OString &rNm );
#else
    SwCache( const sal_uInt16 nInitSize );
#endif
    /// The dtor will free all objects still in the vector
    ~SwCache();

    void Flush();

    //bToTop == false -> No LRU resorting!
    SwCacheObj *Get( const void *pOwner, const bool bToTop = true );
    SwCacheObj *Get( const void *pOwner, const sal_uInt16 nIndex,
                     const bool bToTop = true );
    void ToTop( SwCacheObj *pObj );

    bool Insert( SwCacheObj *pNew );
    void Delete( const void *pOwner );

    void SetLRUOfst( const sal_uInt16 nOfst );  /// nOfst determines how many are not to be touched
    void ResetLRUOfst() { m_pFirst = m_pRealFirst; }

    void IncreaseMax( const sal_uInt16 nAdd );
    void DecreaseMax( const sal_uInt16 nSub );
    sal_uInt16 GetCurMax() const { return m_nCurMax; }
    SwCacheObj *First() { return m_pRealFirst; }
    static inline SwCacheObj *Next( SwCacheObj *pCacheObj);
    SwCacheObj* operator[](sal_uInt16 nIndex) { return m_aCacheObjects[nIndex].get(); }
    sal_uInt16 size() { return m_aCacheObjects.size(); }
};

/// Safely manipulate the cache
class SwSaveSetLRUOfst
{
    SwCache &rCache;
public:
    SwSaveSetLRUOfst( SwCache &rC, const sal_uInt16 nOfst )
        : rCache( rC )          { rCache.SetLRUOfst( nOfst );  }

    ~SwSaveSetLRUOfst()         { rCache.ResetLRUOfst(); }
};

/**
 * The Cache object base class
 * Users of the Cache must derrive a class from the SwCacheObj and store
 * their payload there
 */
class SwCacheObj
{
    friend class SwCache;   /// Can do everything

    SwCacheObj *m_pNext;      /// For the LRU chaining
    SwCacheObj *m_pPrev;

    sal_uInt16 m_nCachePos;   /// Position in the Cache array

    sal_uInt8       m_nLock;

    SwCacheObj *GetNext() { return m_pNext; }
    SwCacheObj *GetPrev() { return m_pPrev; }
    void SetNext( SwCacheObj *pNew )  { m_pNext = pNew; }
    void SetPrev( SwCacheObj *pNew )  { m_pPrev = pNew; }

    void   SetCachePos( const sal_uInt16 nNew ) { m_nCachePos = nNew; }

protected:
    const void *m_pOwner;

public:

    SwCacheObj( const void *pOwner );
    virtual ~SwCacheObj();

    const void *GetOwner() const { return m_pOwner; }
    inline bool IsOwner( const void *pNew ) const;

    sal_uInt16 GetCachePos() const { return m_nCachePos; }

    bool IsLocked() const { return 0 != m_nLock; }

#ifdef DBG_UTIL
    void Lock();
    void Unlock();
#else
    void Lock() { ++m_nLock; }
    void Unlock() { --m_nLock; }
#endif
};

/**
 * Access class for the Cache
 *
 * The Cache object is created in the ctor.
 * If the Cache does not return one, the member is set to 0 and one is
 * created on the Get() and added to the Cache (if possible).
 * Cache users must derrive a class from SwCacheAccess in order to
 * guarantee type safety. The base class should always be called for the
 * Get(). A derrived Get() should only ever guarantee type safety.
 * Cache objects are always locked for the instance's life time.
 */
class SwCacheAccess
{
    SwCache &m_rCache;

    void Get_();

protected:
    SwCacheObj *m_pObj;
    const void *m_pOwner; /// Can be use in NewObj

    virtual SwCacheObj *NewObj() = 0;

    inline SwCacheObj *Get();

    inline SwCacheAccess( SwCache &rCache, const void *pOwner, bool bSeek );
    inline SwCacheAccess( SwCache &rCache, const void* nCacheId, const sal_uInt16 nIndex );

public:
    virtual ~SwCacheAccess();

    /// Shorthand for those who know that they did not override isAvailable()
    /// FIXME: wtf?
    bool IsAvail() const { return m_pObj != nullptr; }
};


inline bool SwCacheObj::IsOwner( const void *pNew ) const
{
    return m_pOwner && m_pOwner == pNew;
}

inline SwCacheObj *SwCache::Next( SwCacheObj *pCacheObj)
{
    if ( pCacheObj )
        return pCacheObj->GetNext();
    else
        return nullptr;
}

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void *pOwn, bool bSeek ) :
    m_rCache( rC ),
    m_pObj( nullptr ),
    m_pOwner( pOwn )
{
    if ( bSeek && m_pOwner && nullptr != (m_pObj = m_rCache.Get( m_pOwner )) )
        m_pObj->Lock();
}

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void* nCacheId,
                              const sal_uInt16 nIndex ) :
    m_rCache( rC ),
    m_pObj( nullptr ),
    m_pOwner( nCacheId )
{
    if ( m_pOwner && nullptr != (m_pObj = m_rCache.Get( m_pOwner, nIndex )) )
        m_pObj->Lock();
}

inline SwCacheObj *SwCacheAccess::Get()
{
    if ( !m_pObj )
        Get_();
    return m_pObj;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
