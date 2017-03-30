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

#ifndef INCLUDED_SVTOOLS_SOURCE_TABLE_TABLEGEOMETRY_HXX
#define INCLUDED_SVTOOLS_SOURCE_TABLE_TABLEGEOMETRY_HXX

#include <svtools/table/tabletypes.hxx>

#include <tools/gen.hxx>


namespace svt { namespace table
{


    class TableControl_Impl;


    //= TableGeometry

    class TableGeometry
    {
    protected:
        const TableControl_Impl&    m_rControl;
        const tools::Rectangle&            m_rBoundaries;
        tools::Rectangle                   m_aRect;

    protected:
        TableGeometry(
                const TableControl_Impl& _rControl,
                const tools::Rectangle& _rBoundaries
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
        const tools::Rectangle&    getRect() const { return m_aRect; }
        bool                isValid() const { return !m_aRect.GetIntersection( m_rBoundaries ).IsEmpty(); }
    };


    //= TableRowGeometry

    class TableRowGeometry : public TableGeometry
    {
    protected:
        RowPos  m_nRowPos;
        bool    m_bAllowVirtualRows;

    public:
        TableRowGeometry(
            TableControl_Impl const & _rControl,
            tools::Rectangle const & _rBoundaries,
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


    //= TableColumnGeometry

    class TableColumnGeometry : public TableGeometry
    {
    protected:
        ColPos  m_nColPos;

    public:
        TableColumnGeometry(
            TableControl_Impl const & _rControl,
            tools::Rectangle const & _rBoundaries,
            ColPos const _nCol
        );

        // status
        ColPos              getCol() const  { return m_nColPos; }
        // operations
        bool                moveRight();

    private:
        void    impl_initRect();
        bool    impl_isValidColumn( ColPos const i_column ) const;
    };


    //= TableCellGeometry

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
                tools::Rectangle const & _rBoundaries,
                ColPos const _nCol,
                RowPos const _nRow
            )
            :m_aRow( _rControl, _rBoundaries, _nRow, false/*allowVirtualCells*/ )
            ,m_aCol( _rControl, _rBoundaries, _nCol )
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

        tools::Rectangle   getRect() const     { return m_aRow.getRect().GetIntersection( m_aCol.getRect() ); }
        ColPos      getColumn() const   { return m_aCol.getCol(); }
        bool        isValid() const     { return !getRect().IsEmpty(); }

        bool        moveRight()     {return m_aCol.moveRight(); }
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_SOURCE_TABLE_TABLEGEOMETRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
