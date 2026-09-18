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

#pragma once

#include <controls/table/tabletypes.hxx>
#include <tools/gen.hxx>

namespace svt::table
{


    class TableControl;


    //= TableGeometry

    class TableGeometry
    {
    protected:
        const TableControl&    m_rControl;
        const tools::Rectangle&            m_rBoundaries;
        tools::Rectangle                   m_aRect;

    protected:
        TableGeometry(
                const TableControl& _rControl,
                const tools::Rectangle& _rBoundaries
            )
            :m_rControl( _rControl )
            ,m_rBoundaries( _rBoundaries )
            ,m_aRect( _rBoundaries )
        {
        }

    public:
        // attribute access
        const TableControl&    getControl() const      { return m_rControl; }

        // status
        const tools::Rectangle&    getRect() const { return m_aRect; }
        bool                isValid() const { return !m_aRect.GetIntersection( m_rBoundaries ).IsEmpty(); }
    };


    //= TableRowGeometry

    class TableRowGeometry final : public TableGeometry
    {
    public:
        TableRowGeometry(
            TableControl const & _rControl,
            tools::Rectangle const & _rBoundaries,
            sal_Int32 const _nRow,
            bool const i_allowVirtualRows = false
                // allow rows >= getRowCount()?
        );

        // status
        sal_Int32 getRow() const { return m_nRowPos; }
        // operations
        bool                moveDown();

    private:
        void    impl_initRect();
        bool impl_isValidRow(sal_Int32 const i_row) const;

        sal_Int32 m_nRowPos;
        bool    m_bAllowVirtualRows;
    };


    //= TableColumnGeometry

    class TableColumnGeometry final : public TableGeometry
    {
    public:
        TableColumnGeometry(TableControl const& _rControl,
                            tools::Rectangle const& _rBoundaries, sal_Int32 const _nCol);

        // status
        sal_Int32 getCol() const { return m_nColPos; }
        // operations
        bool                moveRight();

    private:
        void    impl_initRect();
        bool impl_isValidColumn(sal_Int32 const i_column) const;

        sal_Int32 m_nColPos;
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
                TableControl const & _rControl,
                tools::Rectangle const & _rBoundaries,
                sal_Int32 const _nCol,
                sal_Int32 const _nRow
            )
            :m_aRow( _rControl, _rBoundaries, _nRow, false/*allowVirtualCells*/ )
            ,m_aCol( _rControl, _rBoundaries, _nCol )
        {
        }

        TableCellGeometry(
                const TableRowGeometry& _rRow,
                sal_Int32 _nCol
            )
            :m_aRow( _rRow )
            ,m_aCol( _rRow.getControl(), _rRow.getRect(), _nCol )
        {
        }

        tools::Rectangle   getRect() const     { return m_aRow.getRect().GetIntersection( m_aCol.getRect() ); }
        sal_Int32 getColumn() const { return m_aCol.getCol(); }
        bool        isValid() const     { return !getRect().IsEmpty(); }

        bool        moveRight()     {return m_aCol.moveRight(); }
    };


} // namespace svt::table



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
