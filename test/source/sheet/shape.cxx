/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/shape.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void Shape::testShapePropertiesAnchor()
{
    uno::Reference<beans::XPropertySet> xShape(init(), UNO_QUERY_THROW);
    uno::Any aNewValue;

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getXSheetDocument(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Sequence<OUString> sheetNames = xSheets->getElementNames();
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByName(sheetNames[0]), UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell(xSheet->getCellByPosition(0, 0), UNO_SET_THROW);

    // Shape should be anchored to sheet by default
    uno::Reference<sheet::XSpreadsheet> xSheetGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Anchor (XSpreadsheet)",
                           xShape->getPropertyValue(u"Anchor"_ustr) >>= xSheetGet);

    // Anchor the shape to a cell
    aNewValue <<= xCell;
    xShape->setPropertyValue(u"Anchor"_ustr, aNewValue);
    uno::Reference<table::XCell> xCellGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Anchor (XCell)",
                           xShape->getPropertyValue(u"Anchor"_ustr) >>= xCellGet);

    // Shape should not resize with cell by default
    bool bIsResizeWithCell = {};
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ResizeWithCell"_ustr) >>= bIsResizeWithCell);
    CPPUNIT_ASSERT_MESSAGE("Shape should not resize with the cell", !bIsResizeWithCell);

    xShape->setPropertyValue(u"ResizeWithCell"_ustr, uno::Any(true));
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ResizeWithCell"_ustr) >>= bIsResizeWithCell);
    CPPUNIT_ASSERT_MESSAGE("Shape should resize with the cell", bIsResizeWithCell);

    // Anchoring to a different cell should keep the "ResizeWithCell" attribute
    uno::Reference<table::XCell> xCell2(xSheet->getCellByPosition(1, 2), UNO_SET_THROW);
    aNewValue <<= xCell2;
    xShape->setPropertyValue(u"Anchor"_ustr, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ResizeWithCell"_ustr) >>= bIsResizeWithCell);
    CPPUNIT_ASSERT_MESSAGE("ResizeWithCell should still be set", bIsResizeWithCell);

    // Now anchor to sheet again
    aNewValue <<= xSheet;
    xShape->setPropertyValue(u"Anchor"_ustr, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"Anchor"_ustr) >>= xSheetGet);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Anchor (XSpreadsheet)",
                           xShape->getPropertyValue(u"Anchor"_ustr) >>= xSheetGet);

    // Setting ResizeWithCell while anchored to page should not have any effect
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ResizeWithCell"_ustr) >>= bIsResizeWithCell);
    CPPUNIT_ASSERT_MESSAGE("ResizeWithCell should be false for sheet anchored shapes",
                           !bIsResizeWithCell);
    xShape->setPropertyValue(u"ResizeWithCell"_ustr, uno::Any(true));
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ResizeWithCell"_ustr) >>= bIsResizeWithCell);
    CPPUNIT_ASSERT_MESSAGE("ResizeWithCell should be unchangeable for sheet anchored shapes",
                           !bIsResizeWithCell);
}

void Shape::testShapePropertiesPosition()
{
    uno::Reference<beans::XPropertySet> xShape(init(), UNO_QUERY_THROW);
    uno::Any aNewValue;

    sal_Int32 nHoriOrientPositionGet = 0;
    sal_Int32 nHoriOrientPositionSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HoriOrientPosition",
                           xShape->getPropertyValue(u"HoriOrientPosition"_ustr)
                           >>= nHoriOrientPositionGet);

    aNewValue <<= nHoriOrientPositionGet + 42;
    xShape->setPropertyValue(u"HoriOrientPosition"_ustr, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"HoriOrientPosition"_ustr) >>= nHoriOrientPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue HoriOrientPosition",
                                 nHoriOrientPositionGet + 42, nHoriOrientPositionSet);

    sal_Int32 nVertOrientPositionGet = 0;
    sal_Int32 nVertOrientPositionSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue VertOrientPosition",
                           xShape->getPropertyValue(u"VertOrientPosition"_ustr)
                           >>= nVertOrientPositionGet);

    aNewValue <<= nVertOrientPositionGet + 42;
    xShape->setPropertyValue(u"VertOrientPosition"_ustr, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"VertOrientPosition"_ustr) >>= nVertOrientPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue VertOrientPosition",
                                 nVertOrientPositionGet + 42, nVertOrientPositionSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
