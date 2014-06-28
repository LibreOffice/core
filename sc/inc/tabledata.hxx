/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_TABLEDATA_HXX
#define INCLUDED_SC_TABLEDATA_HXX

#include <rtl/ustring.hxx>
#include "scdllapi.h"

#include "address.hxx"

#include <vector>

struct SC_DLLPUBLIC ScTableDataColumn
{
    OUString maName;
    size_t mnId;

    ScTableDataColumn();
};

struct SC_DLLPUBLIC ScTableData
{
    bool mbHeaderRow;
    bool mbTotalRow;
    bool mbBandedRows;
    bool mbBandedColumns;
    bool mbFirstColumn;
    bool mbLastColumn;

    OUString maStyleName;
    OUString maName;

    ScRange maRange;

    std::vector<ScTableDataColumn> maColumnData;

    ScTableData();
};

struct SC_DLLPUBLIC ScTableStyle
{
    OUString maWholeTable;
    OUString maFirstColumnStripe;
    OUString maSecondColumnStripe;
    OUString maFirstRowStripe;
    OUString maSecondRowStripe;
    OUString maFirstColumn;
    OUString maLastColumn;
    OUString maHeaderRow;
    OUString maTotalRow;
    OUString maFirstHeaderCell;
    OUString maLastHeaderCell;
    OUString maFirstTotalCell;
    OUString maLastTotalCell;

    size_t mnFirstRowStripSize;
    size_t mnSecondRowStripeSize;
    size_t mnFirstColumnStripeSize;
    size_t mnSecondColumnStripeSize;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
