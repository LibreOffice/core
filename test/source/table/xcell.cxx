/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/cppunitasserthelper.hxx>
#include <test/table/xcell.hxx>

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellContentType.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

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
    CPPUNIT_ASSERT_MESSAGE("Successfully able to get Error",
                           (nCorrectFormula == 0) && (nIncorrectFormula != 0));
}

void XCell::testGetFormula()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    OUString aFormula = "=2+2";

    xCell->setFormula(aFormula);

    OUString aFormula2 = xCell->getFormula();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to get Formula", aFormula, aFormula2);
}

void XCell::testGetType()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    table::CellContentType aType;
    table::CellContentType aType2 = xCell->getType();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to get Type", typeid(aType).name(),
                                 typeid(aType2).name());
}

void XCell::testGetValue()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    bool aResult = true;
    xCell->getValue();
    CPPUNIT_ASSERT_MESSAGE("Successfully able to get Value", aResult);
}

void XCell::testSetFormula()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    OUString aFormula = "=2/6";

    xCell->setFormula(aFormula);

    OUString aFormula2 = xCell->getFormula();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to set Formula", aFormula, aFormula2);
}

void XCell::testSetValue()
{
    uno::Reference<table::XCell> xCell(init(), UNO_QUERY_THROW);
    double nInValue = 222.555;

    xCell->setValue(nInValue);
    double nCellValue = xCell->getValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to set Value", nInValue, nCellValue);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
