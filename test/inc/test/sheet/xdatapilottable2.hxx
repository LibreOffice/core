/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
