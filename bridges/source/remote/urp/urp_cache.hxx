/*************************************************************************
 *
 *  $RCSfile: urp_cache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-09-29 08:42:05 $
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
#include <stdio.h>
#include <list>

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
    inline void Cache < t , tequals >::resize( sal_Int32 nNewMaxEntries )
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
