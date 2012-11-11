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

#ifndef INCLUDED_TABLE_DATA
#define INCLUDED_TABLE_DATA

#include <resourcemodel/WW8ResourceModel.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>

namespace writerfilter
{

template <typename T, typename PropertiesPointer>
/**
   Class containing the data to describe a table cell.
 */
class CellData
{
    /**
       Handle to start of cell.
    */
    T mStart;

    /**
       Handle to end of cell.
    */
    T mEnd;

    /**
       Pointer to properties of cell.
    */
    PropertiesPointer mpProps;

    bool mbOpen;

public:
    typedef boost::shared_ptr<CellData> Pointer_t;

    CellData(T start, PropertiesPointer pProps)
    : mStart(start), mEnd(start), mpProps(pProps), mbOpen(true)
    {
    }

    virtual ~CellData() {}

    /**
       Set the start handle of the cell.

       @param start    the start handle of the cell
    */
    void setStart(T start) { mStart = start; }

    /**
       Set the end handle of a cell.

       @param end     the end handle of the cell
    */
    void setEnd(T end) { mEnd = end; mbOpen = false; }

    /**
       Set the properties of the cell.

       @param pProps      the properties of the cell
    */
    void setProperties(PropertiesPointer pProps) { mpProps = pProps; }

    /**
       Adds properties to the cell.

       @param pProps      the properties to add
     */
    void insertProperties(PropertiesPointer pProps)
    {
        if( mpProps.get() )
            mpProps->InsertProps(pProps);
        else
            mpProps = pProps;
    }

    /**
       Return start handle of the cell.
     */
    const T & getStart() { return mStart; }

    /**
       Return end handle of the cell.
    */
    const T & getEnd() { return mEnd; }

    /**
       Return properties of the cell.
     */
    PropertiesPointer getProperties() { return mpProps; }

    bool isOpen() const { return mbOpen; }
};

template <typename T, typename PropertiesPointer>
/**
   Class to handle data of a table row.
 */
class RowData
{
    typedef typename CellData<T, PropertiesPointer>::Pointer_t
        CellDataPointer_t;
    typedef ::std::vector<CellDataPointer_t> Cells;

    /**
       the cell data of the row
     */
    Cells mCells;

    /**
       the properties of the row
    */
    mutable PropertiesPointer mpProperties;

public:
    typedef boost::shared_ptr<RowData <T, PropertiesPointer> > Pointer_t;

    RowData() {}

    RowData(const RowData<T, PropertiesPointer> & rRowData)
    : mCells(rRowData.mCells), mpProperties(rRowData.mpProperties)
    {
    }

    virtual ~RowData() {}

    /**
       Add a cell to the row.

       @param start     the start handle of the cell
       @param end       the end handle of the cell
       @param pProps    the properties of the cell
     */
    void addCell(const T & start, PropertiesPointer pProps)
    {
        CellDataPointer_t pCellData
            (new CellData<T, PropertiesPointer>(start, pProps));
        mCells.push_back(pCellData);
    }

    void endCell(const T & end)
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
    void insertProperties(PropertiesPointer pProperties)
    {
        if( pProperties.get() )
        {
            if( !mpProperties.get() )
                mpProperties = pProperties;
            else
                mpProperties->InsertProps(pProperties);
        }
    }

    /**
       Add properties to a cell of the row.

       @param i          index of the cell
       @param pProps     the properties to add
     */
    void insertCellProperties(unsigned int i, PropertiesPointer pProps)
    {
        mCells[i]->insertProperties(pProps);
    }

    /**
        Add properties to the last cell of the row.
     */
    void insertCellProperties(PropertiesPointer pProps)
    {
        if (! mCells.empty())
            mCells.back()->insertProperties(pProps);
    }

    /**
       Return number of cells in the row.
    */
    unsigned int getCellCount()
    {
        return mCells.size();
    }

    /**
       Return start handle of a cell in the row.

       @param i      index of the cell
     */
    const T & getCellStart(unsigned int i) const
    {
        return mCells[i]->getStart();
    }

    /**
        Return end handle of a cell in the row.

        @param i     index of the cell
    */
    const T & getCellEnd(unsigned int i) const
    {
        return mCells[i]->getEnd();
    }

    /**
       Return the properties of a cell in the row.

       @param i      index of the cell
     */
    PropertiesPointer getCellProperties(unsigned int i) const
    {
        return mCells[i]->getProperties();
    }

    /**
       Return properties of the row.
     */
    PropertiesPointer getProperties()
    {
        return mpProperties;
    }

    /**
       Clear the row data.
     */
    void clear()
    {
        mCells.clear();
        mpProperties.reset();
    }
};

template <typename T, typename PropertiesPointer>
/**
   Class that holds the data of a table.
 */
class TableData
{
    typedef typename RowData<T, PropertiesPointer>::Pointer_t RowPointer_t;
    typedef ::std::vector<RowPointer_t> Rows;

    /**
       the table properties
     */
    PropertiesPointer mpTableProps;

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
    void newRow() { mpRow = RowPointer_t(new RowData<T, PropertiesPointer>()); }

public:
    typedef boost::shared_ptr<TableData <T, PropertiesPointer> > Pointer_t;

    TableData(unsigned int nDepth) : mnDepth(nDepth) { newRow(); }
    ~TableData() {}

    /**
       End the current row.

       Sets properties of the current row and pushes the row to the
       back of the rows currently contained in the table.

       @param pProperties    properties of the row to be ended
     */
    void endRow(PropertiesPointer pProperties)
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
    void addCell(const T & start, PropertiesPointer pProps)
    {
        mpRow->addCell(start, pProps);
    }

    /**
        End the current cell of the current row.

        @parm end    end handle of the cell
     */
    void endCell(const T & end)
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
    void insertCellProperties(PropertiesPointer pProps)
    {
        mpRow->insertCellProperties(pProps);
    }

    /**
       Add properties to a cell of the current row.

       @param i       index of the cell
       @param pProps  properties to add
     */
    void insertCellProperties(unsigned int i, PropertiesPointer pProps)
    {
        mpRow->insertCellProperties(i, pProps);
    }

    void insertTableProperties( PropertiesPointer pProps )
    {
        if ( mpTableProps.get( ) )
            mpTableProps->insert( pProps );
        else
            mpTableProps = pProps;
    }

    /**
      Return the table properties.
     */
    PropertiesPointer getTableProperties( )
    {
        return mpTableProps;
    }

    /**
       Return number of rows in the table.
     */
    unsigned int getRowCount()
    {
        return mRows.size();
    }

    /**
       Return depth of table in surrounding table hierarchy.
    */
    unsigned int getDepth()
    {
        return mnDepth;
    }

    /**
       Return row data of a certain row.

       @param i     index of the row
    */
    const RowPointer_t getRow(unsigned int i) const
    {
        return mRows[i];
    }
};

}

#endif // INCLUDED_TABLE_DATA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
