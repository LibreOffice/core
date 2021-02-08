/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/table/xcell.hxx>

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellContentType.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCell::testGetError()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    const sal_Int32 nCorrectFormula = xCell->getError();
    xCell->setFormula("=sqrt(-2)");
    const sal_Int32 nIncorrectFormula = xCell->getError();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to get Error", sal_Int32(0), nCorrectFormula);
    CPPUNIT_ASSERT_MESSAGE("Successfully able to get Error", (nIncorrectFormula != 0));
}

void XCell::testGetType()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    bool aResult = true;

    if (xCell->getType() == table::CellContentType_EMPTY)
        aResult &= true;
    else if (xCell->getType() == table::CellContentType_VALUE)
        aResult &= true;
    else if (xCell->getType() == table::CellContentType_TEXT)
        aResult &= true;
    else if (xCell->getType() == table::CellContentType_FORMULA)
        aResult &= true;
    else
        aResult = false;

    CPPUNIT_ASSERT_MESSAGE("Successfully able to get Type", aResult);
}

void XCell::testSetGetFormula()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    OUString aFormula = "=2+2";

    xCell->setFormula(aFormula);

    OUString aFormula2 = xCell->getFormula();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to set and get Formula", aFormula, aFormula2);
}

void XCell::testSetGetValue()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    double nInValue = 222.555;

    xCell->setValue(nInValue);

    double nCellValue = xCell->getValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to set and get Value", nInValue, nCellValue);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
