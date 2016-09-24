/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CELLVALUES_HXX
#define INCLUDED_SC_INC_CELLVALUES_HXX

#include "address.hxx"
#include <global.hxx>
#include <memory>

class ScColumn;

namespace svl {

class SharedString;

}

namespace sc {

struct CellValuesImpl;

struct CellValueSpan
{
    SCROW mnRow1;
    SCROW mnRow2;

    CellValueSpan( SCROW nRow1, SCROW nRow2 );
};

/**
 * Think of this as a mini-ScColumn like storage that only stores cell
 * values in a column.
 */
class CellValues
{
    std::unique_ptr<CellValuesImpl> mpImpl;

    CellValues( const CellValues& ) = delete;
    CellValues& operator= ( const CellValues& ) = delete;

public:
    CellValues();
    ~CellValues();

    /**
     * Transfer values from specified column.  The transferred segment in the
     * source column becomes empty after this call.
     *
     * @param rCol source column to transfer values from.
     * @param nRow top row position in the source column.
     * @param nLen length of the segment to transfer.
     */
    void transferFrom( ScColumn& rCol, SCROW nRow, size_t nLen );

    void copyTo( ScColumn& rCol, SCROW nRow ) const;
    void swapNonEmpty( ScColumn& rCol );

    void assign( const std::vector<double>& rVals );

    size_t size() const;

    void reset( size_t nSize );
    void setValue( size_t nRow, double fVal );
    void setValue( size_t nRow, const svl::SharedString& rStr );

    void swap( CellValues& r );

    std::vector<CellValueSpan> getNonEmptySpans() const;

private:
    void copyCellsTo( ScColumn& rCol, SCROW nRow ) const;
    void copyCellTextAttrsTo( ScColumn& rCol, SCROW nRow ) const;
};

/**
 * Stores cell values for multiple tables.
 */
class TableValues
{
    struct Impl;

    std::unique_ptr<Impl> mpImpl;

    TableValues( const TableValues& ) = delete;
    TableValues& operator= ( const TableValues& ) = delete;

public:

    TableValues();
    TableValues( const ScRange& rRange );
    ~TableValues();

    const ScRange& getRange() const;

    /**
     * Swap the entire column.
     */
    void swap( SCTAB nTab, SCCOL nCol, CellValues& rColValue );

    /**
     * Swap non-empty blocks with the column storage.
     */
    void swapNonEmpty( SCTAB nTab, SCCOL nCol, ScColumn& rCol );

    std::vector<CellValueSpan> getNonEmptySpans( SCTAB nTab, SCCOL nCol ) const;

    void swap( TableValues& rOther );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
