/*************************************************************************
 *
 *  $RCSfile: TSkipDeletedSet.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-05 07:03:02 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t nSize,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void * pMem,void* _pHint ) SAL_THROW( () )
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

