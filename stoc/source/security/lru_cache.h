/*************************************************************************
 *
 *  $RCSfile: lru_cache.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:09:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _STOC_SEC_LRU_CACHE_H_
#define _STOC_SEC_LRU_CACHE_H_

#include <hash_map>

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
    typedef ::std::hash_map< t_key, Entry *, t_hashKey, t_equalKey > t_key2element;
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
            OSL_TRACE( str.getStr() );
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
                OSL_TRACE( str.getStr() );
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
            OSL_TRACE( str.getStr() );
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
