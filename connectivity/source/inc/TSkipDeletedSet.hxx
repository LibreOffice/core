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
#ifndef CONNECTIVITY_SKIPDELETEDSSET_HXX
#define CONNECTIVITY_SKIPDELETEDSSET_HXX

#include "TResultSetHelper.hxx"
#include <rtl/alloc.h>
#include <boost/unordered_map.hpp>
#include <vector>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    /**
        the class OSkipDeletedSet supports a general method to skip deleted rows
    */
    class OOO_DLLPUBLIC_DBTOOLS OSkipDeletedSet
    {
        ::std::vector<sal_Int32>                m_aBookmarksPositions;// vector of iterators to position map, the order is the logical position
        IResultSetHelper*                       m_pHelper;            // used for moving in the resultset
        bool                                    m_bDeletedVisible;

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
                sal_Int32 _nBookmark    the logical position

            @return the mapped position
        */
        sal_Int32   getMappedPosition(sal_Int32 _nBookmark) const;
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
        inline sal_Int32    getLastPosition() const { return m_aBookmarksPositions.size(); }
        inline void SetDeletedVisible(bool _bDeletedVisible) { m_bDeletedVisible = _bDeletedVisible; }
    };
}
#endif // CONNECTIVITY_SKIPDELETEDSSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
