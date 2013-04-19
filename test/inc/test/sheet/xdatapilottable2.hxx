/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XDataPilotTable2.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/uno/Sequence.h>

#include "test/testdllapi.hxx"

#include <vector>

using namespace com::sun::star;

namespace apitest {

class OOO_DLLPUBLIC_TEST XDataPilotTable2
{
public:
    void testGetPositionData();
    void testGetDrillDownData();
    void testGetOutputRangeByType();
    void testInsertDrillDownSheet();
    virtual ~XDataPilotTable2();

    virtual uno::Reference< uno::XInterface > initDP2() = 0;
    virtual uno::Reference< uno::XInterface > getSheets() = 0;

protected:

private:
    bool checkDrillDownSheetContent(uno::Reference< sheet::XSpreadsheet >, uno::Sequence< uno::Sequence < uno::Any > > aData);

    void getOutputRanges(uno::Reference< sheet::XDataPilotTable2 >);
    void buildDataFields(uno::Reference< sheet::XDataPilotTable2 >);
    void buildResultCells(uno::Reference< sheet::XDataPilotTable2 >);

    std::vector< table::CellAddress > maResultCells;
    std::vector< sal_Int32 > maDataFieldDims;
    table::CellRangeAddress maRangeWhole;
    table::CellRangeAddress maRangeTable;
    table::CellRangeAddress maRangeResult;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
