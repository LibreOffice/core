/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetoperation.hxx>

#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSheetOperation::testClearContents()
{
    uno::Reference<sheet::XSheetOperation> xSheetOperation(init(), UNO_QUERY_THROW);

    xSheetOperation->clearContents(sheet::CellFlags::VALUE | sheet::CellFlags::FORMULA);

    double sum = xSheetOperation->computeFunction(sheet::GeneralFunction_SUM);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to clean contents", 0.0, sum);
}

void XSheetOperation::testComputeFunction()
{
    uno::Reference<sheet::XSheetOperation> xSheetOperation(init(), UNO_QUERY_THROW);

    double count = xSheetOperation->computeFunction(sheet::GeneralFunction_COUNT);
    CPPUNIT_ASSERT_MESSAGE("Unable to compute function", count >= 0.0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
