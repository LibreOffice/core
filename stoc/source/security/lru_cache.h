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
#ifndef _STOC_SEC_LRU_CACHE_H_
#define _STOC_SEC_LRU_CACHE_H_

#include <boost/unordered_map.hpp>

// __CACHE_DIAGNOSE works only for OUString keys
#ifdef __CACHE_DIAGNOSE
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#endif


namespace stoc_sec
{

/** Implementation of a least recently used (lru) cache.
*/
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
class lru_cache
{
    struct Entry
    {
        t_key m_key;
        t_val m_val;
        Entry * m_pred;
        Entry * m_succ;
    };
    typedef ::boost::unordered_map< t_key, Entry *, t_hashKey, t_equalKey > t_key2element;
    t_key2element m_key2element;
    ::std::size_t m_size;

    Entry * m_block;
    mutable Entry * m_head;
    mutable Entry * m_tail;
    inline void toFront( Entry * entry ) const SAL_THROW( () );

public:
    /** Default Ctor.  Does not cache.
    */
    inline lru_cache() SAL_THROW( () );
    /** Ctor.

        @param size number of elements to be cached; default param set to 128
    */
    inline lru_cache( ::std::size_t size ) SAL_THROW( () );

    /** Destructor: releases all cached elements and keys.
    */
    inline ~lru_cache() SAL_THROW( () );

    /** Retrieves a pointer to value in cache.  Returns 0, if none was found.

        @param key a key
        @return pointer to value or 0
    */
    inline t_val const * lookup( t_key const & key ) const SAL_THROW( () );

    /** Sets a value to be cached for given key.

        @param key a key
        @param val a value
    */
    inline void set( t_key const & key, t_val const & val ) SAL_THROW( () );

    /** Tests whether a value is cached for given key.

        @param key a key
        @return true, if value is cached
    */
    inline bool has( t_key const & key ) const SAL_THROW( () );

    /** Clears the cache, releasing all cached elements and keys.
    */
    inline void clear() SAL_THROW( () );

    /** Sets the number of elements to be cached.  This will clear previous entries.

        @param cacheSize number of elements to be cached
    */
    inline void setSize( ::std::size_t size ) SAL_THROW( () );
};
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::setSize(
    ::std::size_t size ) SAL_THROW( () )
{
    m_key2element.clear();
    delete [] m_block;
    m_block = 0;
    m_size = size;

    if (0 < m_size)
    {
        m_block = new Entry[ m_size ];
        m_head = m_block;
        m_tail = m_block + m_size -1;
        for ( ::std::size_t nPos = m_size; nPos--; )
        {
            m_block[ nPos ].m_pred = m_block + nPos -1;
            m_block[ nPos ].m_succ = m_block + nPos +1;
        }
    }
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lru_cache(
    ::std::size_t size ) SAL_THROW( () )
    : m_size( 0 )
    , m_block( 0 )
{
    setSize( size );
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lru_cache() SAL_THROW( () )
    : m_size( 0 )
    , m_block( 0 )
{
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::~lru_cache()
    SAL_THROW( () )
{
    delete [] m_block;
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::toFront(
    Entry * entry ) const SAL_THROW( () )
{
    if (entry != m_head)
    {
        // cut out element
        if (entry == m_tail)
        {
            m_tail = entry->m_pred;
        }
        else
        {
            entry->m_succ->m_pred = entry->m_pred;
            entry->m_pred->m_succ = entry->m_succ;
        }
        // push to front
        m_head->m_pred = entry;
        entry->m_succ = m_head;
        m_head = entry;
    }
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline bool lru_cache< t_key, t_val, t_hashKey, t_equalKey >::has(
    t_key const & key ) const SAL_THROW( () )
{
    typename t_key2element::const_iterator const iFind( m_key2element.find( key ) );
    return (iFind != m_key2element.end());
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline t_val const * lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lookup(
    t_key const & key ) const SAL_THROW( () )
{
    if (0 < m_size)
    {
        typename t_key2element::const_iterator const iFind( m_key2element.find( key ) );
        if (iFind != m_key2element.end())
        {
            Entry * entry = iFind->second;
            toFront( entry );
#ifdef __CACHE_DIAGNOSE
            ::rtl::OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> retrieved element \"") );
            buf.append( entry->m_key );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" from cache") );
            ::rtl::OString str( ::rtl::OUStringToOString(
                buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "%s", str.getStr() );
#endif
            return &entry->m_val;
        }
    }
    return 0;
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::set(
    t_key const & key, t_val const & val ) SAL_THROW( () )
{
    if (0 < m_size)
    {
        typename t_key2element::const_iterator const iFind( m_key2element.find( key ) );

        Entry * entry;
        if (iFind == m_key2element.end())
        {
            entry = m_tail; // erase last element
#ifdef __CACHE_DIAGNOSE
            if (entry->m_key.getLength())
            {
                ::rtl::OUStringBuffer buf( 48 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> kicking element \"") );
                buf.append( entry->m_key );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" from cache") );
                ::rtl::OString str( ::rtl::OUStringToOString(
                    buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
                OSL_TRACE( "%s", str.getStr() );
            }
#endif
            m_key2element.erase( entry->m_key );
            entry->m_key = key;
            ::std::pair< typename t_key2element::iterator, bool > insertion(
                m_key2element.insert( typename t_key2element::value_type( key, entry ) ) );
#ifdef __CACHE_DIAGNOSE
            OSL_ENSURE( insertion.second, "### inserting new cache entry failed?!" );
#endif
        }
        else
        {
            entry = iFind->second;
#ifdef __CACHE_DIAGNOSE
            ::rtl::OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> replacing element \"") );
            buf.append( entry->m_key );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" in cache") );
            ::rtl::OString str( ::rtl::OUStringToOString(
                buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "%s", str.getStr() );
#endif
        }
        entry->m_val = val;
        toFront( entry );
    }
}
//__________________________________________________________________________________________________
template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::clear() SAL_THROW( () )
{
    m_key2element.clear();
    for ( ::std::size_t nPos = m_size; nPos--; )
    {
        m_block[ nPos ].m_key = t_key();
        m_block[ nPos ].m_val = t_val();
    }
#ifdef __CACHE_DIAGNOSE
    OSL_TRACE( "> cleared cache\n" );
#endif
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
