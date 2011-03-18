/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef SVTOOLS_TABLEGEOMETRY_HXX
#define SVTOOLS_TABLEGEOMETRY_HXX

#include "svtools/table/tabletypes.hxx"

#include <tools/gen.hxx>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    class TableControl_Impl;

    //====================================================================
    //= TableGeometry
    //====================================================================
    class TableGeometry
    {
    protected:
        const TableControl_Impl&    m_rControl;
        const Rectangle&            m_rBoundaries;
        Rectangle                   m_aRect;

    protected:
        TableGeometry(
                const TableControl_Impl& _rControl,
                const Rectangle& _rBoundaries
            )
            :m_rControl( _rControl )
            ,m_rBoundaries( _rBoundaries )
            ,m_aRect( _rBoundaries )
        {
        }

    public:
        // attribute access
        const TableControl_Impl&    getControl() const      { return m_rControl; }

        // status
        const Rectangle&    getRect() const { return m_aRect; }
        bool                isValid() const { return !m_aRect.GetIntersection( m_rBoundaries ).IsEmpty(); }
    };

    //====================================================================
    //= TableRowGeometry
    //====================================================================
    class TableRowGeometry : public TableGeometry
    {
    protected:
        RowPos  m_nRowPos;
        bool    m_bAllowVirtualRows;

    public:
        TableRowGeometry(
            TableControl_Impl const & _rControl,
            Rectangle const & _rBoundaries,
            RowPos const _nRow,
            bool const i_allowVirtualRows = false
                // allow rows >= getRowCount()?
        );

        // status
        RowPos              getRow() const  { return m_nRowPos; }
        // operations
        bool                moveDown();

    private:
        void    impl_initRect();
        bool    impl_isValidRow( RowPos const i_row ) const;
    };

    //====================================================================
    //= TableColumnGeometry
    //====================================================================
    class TableColumnGeometry : public TableGeometry
    {
    protected:
        ColPos  m_nColPos;
        bool    m_bAllowVirtualColumns;

    public:
        TableColumnGeometry(
            TableControl_Impl const & _rControl,
            Rectangle const & _rBoundaries,
            ColPos const _nCol,
            bool const i_allowVirtualColumns = false
        );

        // status
        ColPos              getCol() const  { return m_nColPos; }
        // operations
        bool                moveRight();

    private:
        void    impl_initRect();
        bool    impl_isValidColumn( ColPos const i_column ) const;
    };

    //====================================================================
    //= TableCellGeometry
    //====================================================================
    /** a helper representing geometry information of a cell
    */
    class TableCellGeometry
    {
    private:
        TableRowGeometry    m_aRow;
        TableColumnGeometry m_aCol;

    public:
        TableCellGeometry(
                TableControl_Impl const & _rControl,
                Rectangle const & _rBoundaries,
                ColPos const _nCol,
                RowPos const _nRow,
                bool const i_alllowVirtualCells = false
            )
            :m_aRow( _rControl, _rBoundaries, _nRow, i_alllowVirtualCells )
            ,m_aCol( _rControl, _rBoundaries, _nCol, i_alllowVirtualCells )
        {
        }

        TableCellGeometry(
                const TableRowGeometry& _rRow,
                ColPos _nCol
            )
            :m_aRow( _rRow )
            ,m_aCol( _rRow.getControl(), _rRow.getRect(), _nCol )
        {
        }

        inline  Rectangle   getRect() const     { return m_aRow.getRect().GetIntersection( m_aCol.getRect() ); }
        inline  RowPos      getRow() const      { return m_aRow.getRow(); }
        inline  ColPos      getColumn() const   { return m_aCol.getCol(); }
        inline  bool        isValid() const     { return !getRect().IsEmpty(); }

        inline  bool        moveDown()      {return m_aRow.moveDown(); }
        inline  bool        moveRight()     {return m_aCol.moveRight(); }
    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_TABLEGEOMETRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
