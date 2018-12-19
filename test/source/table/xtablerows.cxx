/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/xtablerows.hxx>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XTableRows::testInsertByIndex()
{
    uno::Reference<table::XCellRange> xCellRange(getXCellRange(), uno::UNO_QUERY_THROW);
    xCellRange->getCellByPosition(0, 0)->setValue(17);
    xCellRange->getCellByPosition(0, 1)->setValue(15);

    uno::Reference<table::XTableRows> xTableRows(init(), uno::UNO_QUERY_THROW);
    xTableRows->removeByIndex(0, 1);

    xTableRows->insertByIndex(0, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(xCellRange->getCellByPosition(0, 1)->getValue(), 15.0, 0.1);

    CPPUNIT_ASSERT_THROW(xTableRows->insertByIndex(-1, 1), uno::RuntimeException);
}

void XTableRows::testRemoveByIndex()
{
    uno::Reference<table::XCellRange> xCellRange(getXCellRange(), uno::UNO_QUERY_THROW);
    xCellRange->getCellByPosition(0, 0)->setValue(17);
    xCellRange->getCellByPosition(0, 1)->setValue(15);

    uno::Reference<table::XTableRows> xTableRows(init(), uno::UNO_QUERY_THROW);
    xTableRows->removeByIndex(0, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(xCellRange->getCellByPosition(0, 0)->getValue(), 15.0, 0.1);

    CPPUNIT_ASSERT_THROW(xTableRows->removeByIndex(-1, 1), uno::RuntimeException);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
