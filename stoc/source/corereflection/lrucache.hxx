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

#pragma once

// __CACHE_DIAGNOSE forces cache size to 4 and works only for OUString keys
//  #define __CACHE_DIAGNOSE 1

#include <rtl/ustring.hxx>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace com::sun::star::uno { class Any; }

/** Implementation of a least recently used (lru) cache.
    <br>
*/
template< class t_Key, class t_Val, class t_KeyHash >
class LRU_Cache
{
    struct CacheEntry
    {
        t_Key               aKey;
        t_Val               aVal;
        CacheEntry *        pPred;
        CacheEntry *        pSucc;
    };
    typedef std::unordered_map< t_Key, CacheEntry *, t_KeyHash > t_Key2Element;

    mutable std::mutex        _aCacheMutex;
    sal_Int32                   _nCachedElements;
    t_Key2Element               _aKey2Element;

    std::unique_ptr<CacheEntry[]> _pBlock;
    mutable CacheEntry *        _pHead;
    mutable CacheEntry *        _pTail;
    inline void toFront( CacheEntry * pEntry ) const;

public:
    /** Constructor:
        <br>
        @param nCachedElements number of elements to be cached; default param set to 128
    */
    explicit inline LRU_Cache();

    /** Retrieves a value from the cache. Returns default constructed value,
        if none was found.
        <br>
        @param rKey a key
        @return value
    */
    inline t_Val getValue( const t_Key & rKey ) const;
    /** Sets a value to be cached for given key.
        <br>
        @param rKey a key
        @param rValue a value
    */
    inline void setValue( const t_Key & rKey, const t_Val & rValue );
    /** Clears the cache, thus releasing all cached elements and keys.
        <br>
    */
    inline void clear();
};

template< class t_Key, class t_Val, class t_KeyHash >
inline LRU_Cache< t_Key, t_Val, t_KeyHash >::LRU_Cache()
#ifdef __CACHE_DIAGNOSE
    : _nCachedElements( 4 )
#else
    : _nCachedElements( 256 )
#endif
    , _pBlock( nullptr )
    , _pHead( nullptr )
    , _pTail( nullptr )
{
    _pBlock.reset(new CacheEntry[_nCachedElements]);
    _pHead = _pBlock.get();
    _pTail = _pBlock.get() + _nCachedElements - 1;
    for (sal_Int32 nPos = _nCachedElements; nPos--;)
    {
        _pBlock[nPos].pPred = _pBlock.get() + nPos - 1;
        _pBlock[nPos].pSucc = _pBlock.get() + nPos + 1;
    }
}

template< class t_Key, class t_Val, class t_KeyHash >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash >::toFront( CacheEntry * pEntry ) const
{
    if (pEntry != _pHead)
    {
        // cut out element
        if (pEntry == _pTail)
        {
            _pTail = pEntry->pPred;
        }
        else
        {
            pEntry->pSucc->pPred = pEntry->pPred;
            pEntry->pPred->pSucc = pEntry->pSucc;
        }
        // push to front
        _pHead->pPred = pEntry;
        pEntry->pSucc = _pHead;
        _pHead        = pEntry;
    }
}

template< class t_Key, class t_Val, class t_KeyHash >
inline t_Val LRU_Cache< t_Key, t_Val, t_KeyHash >::getValue( const t_Key & rKey ) const
{
    std::scoped_lock aGuard( _aCacheMutex );
    const typename t_Key2Element::const_iterator iFind( _aKey2Element.find( rKey ) );
    if (iFind != _aKey2Element.end())
    {
        CacheEntry * pEntry = (*iFind).second;
        toFront( pEntry );
#ifdef __CACHE_DIAGNOSE
        SAL_INFO("stoc.corerefl", "> retrieved element \"" );
        SAL_INFO("stoc.corerefl", "" << pEntry->aKey);
        SAL_INFO("stoc.corerefl", "\" from cache <" );
#endif
        return pEntry->aVal;
    }
    return t_Val();
}

template< class t_Key, class t_Val, class t_KeyHash >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash >::setValue(
    const t_Key & rKey, const t_Val & rValue )
{
    std::scoped_lock aGuard( _aCacheMutex );
    if (_nCachedElements > 0)
    {
        const typename t_Key2Element::const_iterator iFind( _aKey2Element.find( rKey ) );

        CacheEntry * pEntry;
        if (iFind == _aKey2Element.end())
        {
            pEntry = _pTail; // erase last element
#ifdef __CACHE_DIAGNOSE
            if (pEntry->aKey.getLength())
            {
                SAL_INFO("stoc.corerefl", "> kicking element \"" );
                SAL_INFO("stoc.corerefl", "" << pEntry->aKey);
                SAL_INFO("stoc.corerefl", "\" from cache <" );
            }
#endif
            _aKey2Element.erase( pEntry->aKey );
            pEntry->aKey = rKey;
            _aKey2Element[ rKey ] = pEntry;
        }
        else
        {
            pEntry = (*iFind).second;
#ifdef __CACHE_DIAGNOSE
            SAL_INFO("stoc.corerefl", "> replacing element \"" );
            SAL_INFO("stoc.corerefl", "" << pEntry->aKey);
            SAL_INFO("stoc.corerefl", "\" in cache <" );
#endif
        }
        pEntry->aVal = rValue;
        toFront( pEntry );
    }
}

template< class t_Key, class t_Val, class t_KeyHash >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash >::clear()
{
    std::scoped_lock aGuard( _aCacheMutex );
    _aKey2Element.clear();
    for ( sal_Int32 nPos = _nCachedElements; nPos--; )
    {
        _pBlock[nPos].aKey = t_Key();
        _pBlock[nPos].aVal = t_Val();
    }
    _nCachedElements = 0;
#ifdef __CACHE_DIAGNOSE
    SAL_INFO("stoc.corerefl", "> cleared cache <" );
#endif
}


/** Template instance for OUString keys, Any values.<br>
*/
typedef LRU_Cache< OUString, css::uno::Any, OUStringHash >
    LRU_CacheAnyByOUString;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
