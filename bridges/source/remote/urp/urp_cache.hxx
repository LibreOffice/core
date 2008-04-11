/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_cache.hxx,v $
 * $Revision: 1.7 $
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
#include <stdio.h>
#include <list>
#include <algorithm>
#include <rtl/ustring.hxx>

#include "urp_threadid.hxx"
#include "urp_cache.h"

namespace bridges_urp
{

    template < class t , class tequals >
    inline Cache< t , tequals >::Cache( sal_uInt16 nMaxEntries  ) :
        m_pCache( new t[nMaxEntries] ),
        m_nMaxEntries( nMaxEntries ),
        m_nEntries( 0 )
    {

    }

    template < class t , class tequals >
    inline Cache< t , tequals >::~Cache( )
    {
        delete [] m_pCache;
    }


    template < class t , class tequals >
    inline sal_uInt16 Cache< t , tequals >::put( const t & value )
    {
        if( ! m_nMaxEntries )
        {
            return 0xffff;
        }
        sal_uInt16 nEntry = 0xffff;
        if( m_nEntries < m_nMaxEntries )
        {
            // cache has still empty places
            m_pCache[m_nEntries] = value;
            nEntry = m_nEntries;
            m_nEntries ++;

            // add it to the cache
            m_lstLeastRecentlyUsed.push_front( nEntry );
        }
        else
        {
            // cache is full, remove an element and insert the new one
            nEntry = m_lstLeastRecentlyUsed.back();
            m_lstLeastRecentlyUsed.pop_back();
            m_lstLeastRecentlyUsed.push_front( nEntry );

            m_pCache[nEntry] = value;
        }
        return nEntry;
    }

    template < class t , class tequals >
    inline sal_uInt16 Cache< t , tequals >::seek( const t & value )
    {
        for( ::std::list< sal_uInt16 >::iterator ii = m_lstLeastRecentlyUsed.begin() ;
             ii != m_lstLeastRecentlyUsed.end() ;
             ++ ii )
        {
            if( value == m_pCache[*ii] )
            {
                sal_uInt16 nEntry = *ii;
                m_lstLeastRecentlyUsed.erase( ii );
                m_lstLeastRecentlyUsed.push_front( nEntry );
                return nEntry;
            }
        }
        return 0xffff;
    }

    // helper predicate for element removal
    template < class t >
    struct PredicateOverMax
    {
        t m_;
        inline PredicateOverMax( const t &value ) : m_(value)
            {}
        sal_Int32 operator () ( const t &value  )  const
            { return value >= m_; }
    };

    template < class t, class tequals >
    inline void Cache < t , tequals >::resize( sal_uInt16 nNewMaxEntries )
    {
        if( 0 == nNewMaxEntries )
        {
            m_lstLeastRecentlyUsed.clear();
            delete [] m_pCache;
            m_pCache = 0;
            m_nMaxEntries = 0;
        }
        else
        {
            // allocate
            t *pNew = new t[nNewMaxEntries];
            sal_Int32 nMin = nNewMaxEntries < m_nMaxEntries ? nNewMaxEntries : m_nMaxEntries;

            // copy
            for( sal_Int32 i = 0; i < nMin ; i ++ )
            {
                pNew[i] = m_pCache[i];
            }
            // delete
            delete [] m_pCache;

            // assign
            m_pCache = pNew;

            // remove overlapping lru cache entries
            ::std::remove_if(m_lstLeastRecentlyUsed.begin(),
                             m_lstLeastRecentlyUsed.end(),
                             PredicateOverMax< sal_Int32 > ( nMin ) );
        }
        m_nMaxEntries = nNewMaxEntries;
        m_nEntries = m_nEntries < m_nMaxEntries ?
                     m_nEntries : m_nMaxEntries;
    }

    template < class t, class tequals >
    inline void Cache < t, tequals >:: clear()
    {
        for( sal_Int32 i = 0; i < m_nMaxEntries ; i ++ )
        {
            m_pCache[i] = t();
        }
        m_lstLeastRecentlyUsed.clear();
        m_nEntries = 0;
    }
}
