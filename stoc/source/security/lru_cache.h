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
#ifndef INCLUDED_STOC_SOURCE_SECURITY_LRU_CACHE_H
#define INCLUDED_STOC_SOURCE_SECURITY_LRU_CACHE_H

#include <unordered_map>

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
    typedef std::unordered_map< t_key, Entry *, t_hashKey, t_equalKey > t_key2element;
    t_key2element m_key2element;
    ::std::size_t m_size;

    Entry * m_block;
    mutable Entry * m_head;
    mutable Entry * m_tail;
    inline void toFront( Entry * entry ) const;

public:
    /** Default Ctor.  Does not cache.
    */
    inline lru_cache();
    /** Ctor.

        @param size number of elements to be cached; default param set to 128
    */
    explicit inline lru_cache( ::std::size_t size );

    /** Destructor: releases all cached elements and keys.
    */
    inline ~lru_cache();

    /** Retrieves a pointer to value in cache.  Returns 0, if none was found.

        @param key a key
        @return pointer to value or 0
    */
    inline t_val const * lookup( t_key const & key ) const;

    /** Sets a value to be cached for given key.

        @param key a key
        @param val a value
    */
    inline void set( t_key const & key, t_val const & val );

    /** Clears the cache, releasing all cached elements and keys.
    */
    inline void clear();

    /** Sets the number of elements to be cached.  This will clear previous entries.

        @param cacheSize number of elements to be cached
    */
    inline void setSize( ::std::size_t size );
};

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::setSize(
    ::std::size_t size )
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

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lru_cache(
    ::std::size_t size )
    : m_size( 0 )
    , m_block( 0 )
    , m_tail( 0 )
{
    setSize( size );
}

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lru_cache()
    : m_size( 0 )
    , m_block( 0 )
    , m_head( 0 )
    , m_tail( 0 )
{
}

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline lru_cache< t_key, t_val, t_hashKey, t_equalKey >::~lru_cache()
{
    delete [] m_block;
}

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::toFront(
    Entry * entry ) const
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

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline t_val const * lru_cache< t_key, t_val, t_hashKey, t_equalKey >::lookup(
    t_key const & key ) const
{
    if (0 < m_size)
    {
        typename t_key2element::const_iterator const iFind( m_key2element.find( key ) );
        if (iFind != m_key2element.end())
        {
            Entry * entry = iFind->second;
            toFront( entry );
#ifdef __CACHE_DIAGNOSE
            OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> retrieved element \"") );
            buf.append( entry->m_key );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" from cache") );
            OString str( OUStringToOString(
                buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "%s", str.getStr() );
#endif
            return &entry->m_val;
        }
    }
    return 0;
}

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::set(
    t_key const & key, t_val const & val )
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
                OUStringBuffer buf( 48 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> kicking element \"") );
                buf.append( entry->m_key );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" from cache") );
                OString str( OUStringToOString(
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
            (void) insertion; // avoid warnings
        }
        else
        {
            entry = iFind->second;
#ifdef __CACHE_DIAGNOSE
            OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> replacing element \"") );
            buf.append( entry->m_key );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" in cache") );
            OString str( OUStringToOString(
                buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "%s", str.getStr() );
#endif
        }
        entry->m_val = val;
        toFront( entry );
    }
}

template< typename t_key, typename t_val, typename t_hashKey, typename t_equalKey >
inline void lru_cache< t_key, t_val, t_hashKey, t_equalKey >::clear()
{
    m_key2element.clear();
    for ( ::std::size_t nPos = m_size; nPos--; )
    {
        m_block[ nPos ].m_key = t_key();
        m_block[ nPos ].m_val = t_val();
    }
#ifdef __CACHE_DIAGNOSE
    OSL_TRACE( "> cleared cache" );
#endif
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
