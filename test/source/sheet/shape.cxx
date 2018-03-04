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
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void Shape::testShapeProperties()
{
    uno::Reference<beans::XPropertySet> xShape(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "Anchor";
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getXSheetDocument(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Sequence<OUString> sheetNames = xSheets->getElementNames();
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByName(sheetNames[0]), UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell(xSheet->getCellByPosition(0, 0), UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheetGet;
    uno::Reference<table::XCell> xCellGet;

    if (xShape->getPropertyValue(propName) >>= xSheetGet)
    {
        uno::Reference<sheet::XSpreadsheet> xSheetSet;
        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Anchor (XSpreadsheet)",
                               xShape->getPropertyValue(propName) >>= xSheetGet);

        aNewValue <<= xSheet;
        xShape->setPropertyValue(propName, aNewValue);
        CPPUNIT_ASSERT(xShape->getPropertyValue(propName) >>= xSheetSet);
        // TODO: Find a way to compare sheet::XSpreadsheet objects
        //CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Anchor (XSpreadsheet)",
        //xSheet.get(), xSheetSet.get());
    }
    else if (xShape->getPropertyValue(propName) >>= xCellGet)
    {
        uno::Reference<table::XCell> xCellSet;
        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Anchor (XCell)",
                               xShape->getPropertyValue(propName) >>= xCellGet);

        aNewValue <<= xCell;
        xShape->setPropertyValue(propName, aNewValue);
        CPPUNIT_ASSERT(xShape->getPropertyValue(propName) >>= xCellSet);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Anchor (XCell)", xCell, xCellSet);
    }

    propName = "HoriOrientPosition";
    sal_Int32 nHoriOrientPositionGet = 0;
    sal_Int32 nHoriOrientPositionSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HoriOrientPosition",
                           xShape->getPropertyValue(propName) >>= nHoriOrientPositionGet);

    aNewValue <<= nHoriOrientPositionGet + 42;
    xShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(propName) >>= nHoriOrientPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue HoriOrientPosition",
                                 nHoriOrientPositionGet + 42, nHoriOrientPositionSet);

    propName = "VertOrientPosition";
    sal_Int32 nVertOrientPositionGet = 0;
    sal_Int32 nVertOrientPositionSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue VertOrientPosition",
                           xShape->getPropertyValue(propName) >>= nVertOrientPositionGet);

    aNewValue <<= nVertOrientPositionGet + 42;
    xShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xShape->getPropertyValue(propName) >>= nVertOrientPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue VertOrientPosition",
                                 nVertOrientPositionGet + 42, nVertOrientPositionSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
