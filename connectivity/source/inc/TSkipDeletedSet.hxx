/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TSkipDeletedSet.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:58:03 $
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
#ifndef CONNECTIVITY_SKIPDELETEDSSET_HXX
#define CONNECTIVITY_SKIPDELETEDSSET_HXX

#ifndef CONNECTIVITY_TRESULTSETHELPER_HXX
#include "TResultSetHelper.hxx"
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#include <map>
#include <vector>

namespace connectivity
{
    /**
        the class OSkipDeletedSet supports a general method to skip deleted rows
    */
    class OSkipDeletedSet
    {
        typedef ::std::map<sal_Int32,sal_Int32> TInt2IntMap;
        TInt2IntMap                             m_aBookmarks;         // map from postion to logical position
        ::std::vector<TInt2IntMap::iterator>    m_aBookmarksPositions;// vector of iterators to position map, the order is the logical position
        IResultSetHelper*                       m_pHelper;            // used for moving in the resultset

        sal_Bool    moveAbsolute(sal_Int32 _nOffset,sal_Bool _bRetrieveData);
    public:
        OSkipDeletedSet(IResultSetHelper* _pHelper);
        ~OSkipDeletedSet();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }

        /**
            skipDeleted moves the resultset to the position defined by the parameters
            it garantees that the row isn't deleted
                @param
                    IResultSetHelper::Movement  _eCursorPosition        in which direction the resultset should be moved
                    sal_Int32                   _nOffset                the position relativ to the movement
                    sal_Bool                    _bRetrieveData          is true when the current row should be filled which data
                @return
                    true when the movement was successful otherwise false
        */
        sal_Bool    skipDeleted(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData);
        /**
            clear the map and the vector used in this class
        */
        void        clear();
        /**
            getMappedPosition returns the mapped position of a logical position
            @param
                sal_Int32 _nPos the logical position

            @return the mapped position
        */
        sal_Int32   getMappedPosition(sal_Int32 _nPos) const;
        /**
            insertNewPosition adds a new position to the map
            @param
                sal_Int32 _nPos the logical position
        */
        void        insertNewPosition(sal_Int32 _nPos);
        /**
            deletePosition deletes this position from the map and decrement all following positions
            @param
                sal_Int32 _nPos the logical position
        */
        void        deletePosition(sal_Int32 _nPos);
        /**
            getLastPosition returns the last position
            @return the last position
        */
        sal_Int32   getLastPosition() const { return m_aBookmarksPositions.size(); }
    };
}
#endif // CONNECTIVITY_SKIPDELETEDSSET_HXX

