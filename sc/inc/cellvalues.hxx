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

namespace sc {

struct CellValuesImpl;

class CellValues
{
    CellValuesImpl* mpImpl;

    CellValues( const CellValues& ); // disabled
    CellValues& operator= ( const CellValues& ); // disabled

public:
    CellValues();
    ~CellValues();

    void transferFrom( ScColumn& rCol, SCROW nRow, size_t nLen );
    void copyTo( ScColumn& rCol, SCROW nRow ) const;

    void assign( const std::vector<double>& rVals );

    size_t size() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
