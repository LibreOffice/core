/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/xtablecolumns.hxx>

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XTableColumns::testInsertByIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCR(m_xSheet, uno::UNO_QUERY_THROW);

    // insert one column at position one
    xTC->insertByIndex(1, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(3, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 1)));

    // insert one column at position zero
    xTC->insertByIndex(0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(3, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(4, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(5, 1)));

    // insert two columns at position zero
    xTC->insertByIndex(0, 2);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(3, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(5, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(6, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(7, 1)));
}

void XTableColumns::testInsertByIndexWithNegativeIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_THROW(xTC->insertByIndex(-1, 1), uno::RuntimeException);
}

void XTableColumns::testInsertByIndexWithNoColumn()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_THROW(xTC->insertByIndex(0, 0), uno::RuntimeException);
}

void XTableColumns::testInsertByIndexWithOutOfBoundIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xTC->insertByIndex(xTC->getCount(), 1), uno::RuntimeException);
}

void XTableColumns::testRemoveByIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCR(m_xSheet, uno::UNO_QUERY_THROW);

    xTC->insertByIndex(1, 1); // insert one column at position one
    xTC->insertByIndex(0, 1); // insert one column at position zero
    xTC->insertByIndex(0, 2); // insert two columns at position zero

    // remove two columns at position zero
    xTC->removeByIndex(0, 2);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(3, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(4, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(5, 1)));

    // remove one column at position zero
    xTC->removeByIndex(0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(3, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(4, 1)));

    // remove one column at position one
    xTC->removeByIndex(1, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("0a"), getCellText(xCR->getCellByPosition(0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0b"), getCellText(xCR->getCellByPosition(0, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1a"), getCellText(xCR->getCellByPosition(1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1b"), getCellText(xCR->getCellByPosition(1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2a"), getCellText(xCR->getCellByPosition(2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2b"), getCellText(xCR->getCellByPosition(2, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getCellText(xCR->getCellByPosition(3, 1)));
}

void XTableColumns::testRemoveByIndexWithNegativeIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_THROW(xTC->removeByIndex(-1, 1), uno::RuntimeException);
}

void XTableColumns::testRemoveByIndexWithNoColumn()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_THROW(xTC->removeByIndex(0, 0), uno::RuntimeException);
}

void XTableColumns::testRemoveByIndexWithOutOfBoundIndex()
{
    uno::Reference<table::XTableColumns> xTC(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xTC->removeByIndex(xTC->getCount(), 1), uno::RuntimeException);
}

const OUString XTableColumns::getCellText(const uno::Reference<table::XCell>& r_xCell)
{
    uno::Reference<text::XSimpleText> xST(r_xCell, uno::UNO_QUERY_THROW);
    return xST->getString();
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
