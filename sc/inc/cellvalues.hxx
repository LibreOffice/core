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

#include <address.hxx>

class ScColumn;
struct ScRefCellValue;

namespace sc {

struct CellValuesImpl;
struct CellTextAttr;

/**
 * Think of this as a mini-ScColumn like storage that only stores cell
 * values in a column.
 */
class CellValues
{
    CellValuesImpl* mpImpl;

    CellValues( const CellValues& ); // disabled
    CellValues& operator= ( const CellValues& ); // disabled

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

    void transferTo( ScColumn& rCol, SCROW nRow );
    void copyTo( ScColumn& rCol, SCROW nRow ) const;

    void assign( const std::vector<double>& rVals );
    void append( ScRefCellValue& rVal, const CellTextAttr* pAttr );

    size_t size() const;

private:
    void copyCellsTo( ScColumn& rCol, SCROW nRow ) const;
    void copyCellTextAttrsTo( ScColumn& rCol, SCROW nRow ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
