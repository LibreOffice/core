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

#include <rtl/ref.hxx>
#include <vector>

namespace sdr::table
{
class Cell;
class TableModel;
class TableRow;
class TableColumn;
class TableRows;
class TableColumns;
typedef rtl::Reference<Cell> CellRef;
typedef rtl::Reference<TableModel> TableModelRef;
typedef rtl::Reference<TableRow> TableRowRef;
typedef rtl::Reference<TableColumn> TableColumnRef;
typedef std::vector<CellRef> CellVector;
typedef std::vector<TableRowRef> RowVector;
typedef std::vector<TableColumnRef> ColumnVector;

template <typename T> class RangeIterator
{
public:
    /** creates an iterator from rStart (including) to rEnd (excluding) if
        bForward is true or from nEnd (excluding to nStart (including).
        rStart must be <= rEnd.
    */
    RangeIterator(const T& rStart, const T& rEnd, bool bForward)
    {
        if (bForward)
        {
            maIter = rStart;
            maEnd = rEnd;
        }
        else
        {
            maIter = rEnd - 1;
            maEnd = rStart - 1;
        }
    }

    /* iterates in the configured direction and returns true if rValue
       now contains a valid position in the range of this iterator */
    bool next(T& rValue)
    {
        if (maIter == maEnd)
            return false;

        rValue = maIter;
        if (maIter < maEnd)
            ++maIter;
        else
            --maIter;
        return true;
    }

private:
    T maEnd;
    T maIter;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
