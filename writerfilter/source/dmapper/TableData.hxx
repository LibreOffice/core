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

#include <com/sun/star/text/XTextRange.hpp>

#include "PropertyMap.hxx"

#include <vector>

namespace writerfilter::dmapper
{

/**
   Class containing the data to describe a table cell.
 */
class CellData final : public virtual SvRefBase
{
    /**
       Handle to start of cell.
    */
    css::uno::Reference<css::text::XTextRange> mStart;

    /**
       Handle to end of cell.
    */
    css::uno::Reference<css::text::XTextRange> mEnd;

    /**
       Pointer to properties of cell.
    */
    TablePropertyMapPtr mpProps;

    bool mbOpen;

    sal_uInt32 m_nGridSpan; ///< number of grid columns in the parent table's table grid which this cell defines

public:
    typedef tools::SvRef<CellData> Pointer_t;

    CellData(css::uno::Reference<css::text::XTextRange> const & start, TablePropertyMapPtr pProps)
    : mStart(start), mEnd(start), mpProps(pProps), mbOpen(true)
        , m_nGridSpan(1)
    {
    }

    /**
       Set the end handle of a cell.

       @param end     the end handle of the cell
    */
    void setEnd(css::uno::Reference<css::text::XTextRange> const & end) { mEnd = end; mbOpen = false; }

    /**
       Adds properties to the cell.

       @param pProps      the properties to add
     */
    void insertProperties(TablePropertyMapPtr pProps)
    {
        if( mpProps )
            mpProps->InsertProps(pProps.get());
        else
            mpProps = pProps;
    }

    /**
       Return start handle of the cell.
     */
    const css::uno::Reference<css::text::XTextRange>& getStart() const { return mStart; }

    /**
       Return end handle of the cell.
    */
    const css::uno::Reference<css::text::XTextRange>& getEnd() const { return mEnd; }

    /**
       Return properties of the cell.
     */
    const TablePropertyMapPtr& getProperties() const { return mpProps; }

    bool isOpen() const { return mbOpen; }

    sal_uInt32 getGridSpan() const { return m_nGridSpan; }
    void setGridSpan( sal_uInt32 nSpan ) { m_nGridSpan = nSpan; }
};

/**
   Class to handle data of a table row.
 */
class RowData final : public virtual SvRefBase
{
    typedef ::std::vector<CellData::Pointer_t> Cells;

    /**
       the cell data of the row
     */
    Cells mCells;

    /**
       the properties of the row
    */
    mutable TablePropertyMapPtr mpProperties;

    sal_uInt32 m_nGridBefore; ///< number of grid columns in the parent table's table grid which must be skipped before the contents of this table row are added to the parent table
    sal_uInt32 m_nGridAfter; ///< number of grid columns in the parent table's table grid which shall be left after the last cell in the table row

public:
    typedef tools::SvRef<RowData> Pointer_t;

    RowData()
        : m_nGridBefore(0)
        , m_nGridAfter(0)
    {
    }

    RowData(const RowData& rRowData)
    : SvRefBase(), mCells(rRowData.mCells), mpProperties(rRowData.mpProperties)
        , m_nGridBefore(rRowData.m_nGridBefore)
        , m_nGridAfter(rRowData.m_nGridAfter)
    {
    }

    /**
       Add a cell to the row.

       @param start     the start handle of the cell
       @param end       the end handle of the cell
       @param pProps    the properties of the cell
       @param bAddBefore true: add an empty cell at beginning of the row for gridBefore
     */
    void addCell(const css::uno::Reference<css::text::XTextRange>& start, TablePropertyMapPtr pProps, bool bAddBefore = false)
    {
        CellData::Pointer_t pCellData(new CellData(start, pProps));
        if (bAddBefore)
        {
            mCells.insert(mCells.begin(), pCellData);
            mCells[0]->setEnd(start);
        }
        else
            mCells.push_back(pCellData);
    }

    void endCell(const css::uno::Reference<css::text::XTextRange>& end)
    {
        if (mCells.size() > 0)
            mCells.back()->setEnd(end);
    }

    bool isCellOpen() const
    {
        return mCells.size() > 0 && mCells.back()->isOpen();
    }

    /**
       Add properties to the row.

       @param pProperties   the properties to set
     */
    void insertProperties(TablePropertyMapPtr pProperties)
    {
        if( pProperties )
        {
            if( !mpProperties )
                mpProperties = pProperties;
            else
                mpProperties->InsertProps(pProperties.get());
        }
    }

    /**
        Add properties to the last cell of the row.
     */
    void insertCellProperties(TablePropertyMapPtr pProps)
    {
        if (!mCells.empty())
            mCells.back()->insertProperties(pProps);
    }

    /**
       Return number of cells in the row.
    */
    unsigned int getCellCount() const
    {
        return mCells.size();
    }

    /**
       Return start handle of a cell in the row.

       @param i      index of the cell
     */
    const css::uno::Reference<css::text::XTextRange>& getCellStart(unsigned int i) const
    {
        return mCells[i]->getStart();
    }

    /**
        Return end handle of a cell in the row.

        @param i     index of the cell
    */
    const css::uno::Reference<css::text::XTextRange>& getCellEnd(unsigned int i) const
    {
        return mCells[i]->getEnd();
    }

    /**
       Return the properties of a cell in the row.

       @param i      index of the cell
     */
    TablePropertyMapPtr const & getCellProperties(unsigned int i) const
    {
        return mCells[i]->getProperties();
    }

    /**
       Return properties of the row.
     */
    const TablePropertyMapPtr& getProperties() const
    {
        return mpProperties;
    }

    sal_uInt32 getGridBefore() const { return m_nGridBefore; }
    void setGridBefore(sal_uInt32 nSkipGrids) { m_nGridBefore = nSkipGrids; }
    sal_uInt32 getGridAfter() const { return m_nGridAfter; }
    void setGridAfter(sal_uInt32 nSkipGrids) { m_nGridAfter = nSkipGrids; }
    sal_uInt32 getGridSpan(sal_uInt32 i) { return mCells[i]->getGridSpan(); }
    std::vector< sal_uInt32 > getGridSpans()
    {
        std::vector< sal_uInt32 > nRet;
        for (auto const& aCell: mCells)
            nRet.push_back(aCell->getGridSpan());
        return nRet;
    }
    void setCurrentGridSpan(sal_uInt32 nSpan, bool bFirstCell = false)
    {
        if ( mCells.size() )
        {
            if ( bFirstCell )
                mCells.front()->setGridSpan(nSpan);
            else
                mCells.back()->setGridSpan(nSpan);
        }
    }
};

/**
   Class that holds the data of a table.
 */
class TableData : public virtual SvRefBase
{
    typedef RowData::Pointer_t RowPointer_t;
    typedef ::std::vector<RowPointer_t> Rows;

    /**
       the data of the rows of the table
    */
    Rows mRows;

    /**
        pointer to the data of the current row (while building up the table data).
    */
    RowPointer_t mpRow;

    /**
       depth of the current table in a hierarchy of tables
     */
    unsigned int mnDepth;

    /**
       initialize mpRow
     */
    void newRow() { mpRow = RowPointer_t(new RowData()); }

public:
    typedef tools::SvRef<TableData> Pointer_t;

    explicit TableData(unsigned int nDepth) : mnDepth(nDepth) { newRow(); }

    /**
       End the current row.

       Sets properties of the current row and pushes the row to the
       back of the rows currently contained in the table.

       @param pProperties    properties of the row to be ended
     */
    void endRow(TablePropertyMapPtr pProperties)
    {
        mpRow->insertProperties(pProperties);
        mRows.push_back(mpRow);
        newRow();
    }

    /**
       Add a cell to the current row.

       @param start   start handle of the cell
       @param end     end handle of the cell
       @param pProps  properties of the cell
     */
    void addCell(const css::uno::Reference<css::text::XTextRange>& start, TablePropertyMapPtr pProps)
    {
        mpRow->addCell(start, pProps);
    }

    /**
        End the current cell of the current row.

        @parm end    end handle of the cell
     */
    void endCell(const css::uno::Reference<css::text::XTextRange>& end)
    {
        mpRow->endCell(end);
    }

    /**
        Return if the current cell of the current row is open.
     */
    bool isCellOpen() const
    {
        return mpRow->isCellOpen();
    }

    /**
        Insert properties to the current cell of the current row.

        @param pProps   the properties to add
     */
    void insertCellProperties(TablePropertyMapPtr pProps)
    {
        mpRow->insertCellProperties(pProps);
    }

    /**
       Return number of rows in the table.
     */
    unsigned int getRowCount() const
    {
        return mRows.size();
    }

    /**
       Return depth of table in surrounding table hierarchy.
    */
    unsigned int getDepth() const
    {
        return mnDepth;
    }

    /**
       Return row data of a certain row.

       @param i     index of the row
    */
    RowPointer_t const & getRow(unsigned int i) const
    {
        return mRows[i];
    }

    const RowPointer_t& getCurrentRow() const
    {
        return mpRow;
    }
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
