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
#ifndef _LRU_CACHE_HXX_
#define _LRU_CACHE_HXX_

// __CACHE_DIAGNOSE forces cache size to 4 and works only for OUString keys
//  #define __CACHE_DIAGNOSE 1

#include <osl/mutex.hxx>
#include "rtl/ustring.hxx"

#include <boost/unordered_map.hpp>

/** Implementation of a least recently used (lru) cache.
    <br>
    @author Daniel Boelzle
*/
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
class LRU_Cache
{
    struct CacheEntry
    {
        t_Key               aKey;
        t_Val               aVal;
        CacheEntry *        pPred;
        CacheEntry *        pSucc;
    };
    typedef ::boost::unordered_map< t_Key, CacheEntry *, t_KeyHash, t_KeyEqual > t_Key2Element;

    mutable ::osl::Mutex        _aCacheMutex;
    sal_Int32                   _nCachedElements;
    t_Key2Element               _aKey2Element;

    CacheEntry *                _pBlock;
    mutable CacheEntry *        _pHead;
    mutable CacheEntry *        _pTail;
    inline void toFront( CacheEntry * pEntry ) const;

public:
    /** Constructor:
        <br>
        @param nCachedElements number of elements to be cached; default param set to 128
    */
    inline LRU_Cache( sal_Int32 nCachedElements = 128 );
    /** Destructor: releases all cached elements and keys.
        <br>
    */
    inline ~LRU_Cache();

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
    /** Tests whether a value is cached for given key.
        <br>
        @param rKey a key
        @return true, if value is cached
    */
    inline sal_Bool hasValue( const t_Key & rKey ) const;
    /** Clears the cache, thus releasing all cached elements and keys.
        <br>
    */
    inline void clear();
};
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::LRU_Cache( sal_Int32 nCachedElements )
#ifdef __CACHE_DIAGNOSE
    : _nCachedElements( 4 )
#else
    : _nCachedElements( nCachedElements )
#endif
    , _pBlock( 0 )
{
    if (_nCachedElements > 0)
    {
        _pBlock = new CacheEntry[_nCachedElements];
        _pHead  = _pBlock;
        _pTail  = _pBlock + _nCachedElements -1;
        for ( sal_Int32 nPos = _nCachedElements; nPos--; )
        {
            _pBlock[nPos].pPred = _pBlock + nPos -1;
            _pBlock[nPos].pSucc = _pBlock + nPos +1;
        }
    }
}
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::~LRU_Cache()
{
    delete [] _pBlock;
}
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::toFront( CacheEntry * pEntry ) const
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
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline sal_Bool LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::hasValue( const t_Key & rKey ) const
{
    ::osl::MutexGuard aGuard( _aCacheMutex );
    const typename t_Key2Element::const_iterator iFind( _aKey2Element.find( rKey ) );
    return (iFind != _aKey2Element.end());
}
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline t_Val LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::getValue( const t_Key & rKey ) const
{
    ::osl::MutexGuard aGuard( _aCacheMutex );
    const typename t_Key2Element::const_iterator iFind( _aKey2Element.find( rKey ) );
    if (iFind != _aKey2Element.end())
    {
        CacheEntry * pEntry = (*iFind).second;
        toFront( pEntry );
#ifdef __CACHE_DIAGNOSE
        OSL_TRACE( "> retrieved element \"" );
        OSL_TRACE( "%s", ::rtl::OUStringToOString( pEntry->aKey, RTL_TEXTENCODING_ASCII_US ).getStr() );
        OSL_TRACE( "\" from cache <\n" );
#endif
        return pEntry->aVal;
    }
    return t_Val();
}
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::setValue(
    const t_Key & rKey, const t_Val & rValue )
{
    ::osl::MutexGuard aGuard( _aCacheMutex );
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
                OSL_TRACE( "> kicking element \"" );
                OSL_TRACE( "%s", ::rtl::OUStringToOString( pEntry->aKey, RTL_TEXTENCODING_ASCII_US ).getStr() );
                OSL_TRACE( "\" from cache <\n" );
            }
#endif
            _aKey2Element.erase( pEntry->aKey );
            _aKey2Element[ pEntry->aKey = rKey ] = pEntry;
        }
        else
        {
            pEntry = (*iFind).second;
#ifdef __CACHE_DIAGNOSE
            OSL_TRACE( "> replacing element \"" );
            OSL_TRACE( "%s", ::rtl::OUStringToOString( pEntry->aKey, RTL_TEXTENCODING_ASCII_US ).getStr() );
            OSL_TRACE( "\" in cache <\n" );
#endif
        }
        pEntry->aVal = rValue;
        toFront( pEntry );
    }
}
//__________________________________________________________________________________________________
template< class t_Key, class t_Val, class t_KeyHash, class t_KeyEqual >
inline void LRU_Cache< t_Key, t_Val, t_KeyHash, t_KeyEqual >::clear()
{
    ::osl::MutexGuard aGuard( _aCacheMutex );
    _aKey2Element.clear();
    for ( sal_Int32 nPos = _nCachedElements; nPos--; )
    {
        _pBlock[nPos].aKey = t_Key();
        _pBlock[nPos].aVal = t_Val();
    }
    _nCachedElements = 0;
#ifdef __CACHE_DIAGNOSE
    OSL_TRACE( "> cleared cache <\n" );
#endif
}

//==================================================================================================
struct FctHashOUString : public ::std::unary_function< const ::rtl::OUString &, size_t >
{
    size_t operator()( const ::rtl::OUString & rKey ) const
        { return rKey.hashCode(); }
};

/** Template instance for OUString keys, Any values.<br>
*/
typedef LRU_Cache< ::rtl::OUString, ::com::sun::star::uno::Any,
                   FctHashOUString, ::std::equal_to< ::rtl::OUString > >
    LRU_CacheAnyByOUString;


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
