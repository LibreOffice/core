/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_cache.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:45:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


namespace bridges_urp
{
    template < class t , class tequals >
    class Cache
    {
    public:
        inline Cache ( sal_uInt16 nMaxEntries );
        inline ~Cache();

        // puts the value t into the cache. Returns then entry,
        // that is used for this value.
        inline sal_uInt16 put( const t & );

        // lookup, if there is an entry for this value
        // returns 0xffff, when value cannot be found in the list
        inline sal_uInt16 seek( const t & );

        // resizes the cache, conserving overlapping values
        inline void resize( sal_uInt16 nNewMaxEntries );

        // empties the cache
        inline void clear();
    private:
        t *m_pCache;
        ::std::list< sal_uInt16 > m_lstLeastRecentlyUsed;
        sal_uInt16 m_nMaxEntries;
        sal_uInt16 m_nEntries;
    };
}
