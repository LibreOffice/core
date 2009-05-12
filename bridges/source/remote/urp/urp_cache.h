/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_cache.h,v $
 * $Revision: 1.5 $
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
