/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

namespace sc_apitest
{
class ScSearchStylesTest : public UnoApiTest
{
public:
    ScSearchStylesTest();

    void setUp() override;

    void testSearchRectangle();
    void testSearchRectangles();

    CPPUNIT_TEST_SUITE(ScSearchStylesTest);
    CPPUNIT_TEST(testSearchRectangle);
    CPPUNIT_TEST(testSearchRectangles);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference<css::sheet::XSpreadsheet> m_xSheet;
    css::uno::Reference<css::view::XSelectionSupplier> m_xSelectionSupplier;

    css::uno::Reference<css::util::XSearchable> getSearchable();
    css::uno::Reference<css::util::XSearchDescriptor>
    createSearchDescriptor(const css::uno::Reference<css::util::XSearchable>& xSearchable,
                           const OUString& sStyleName);
};

ScSearchStylesTest::ScSearchStylesTest()
    : UnoApiTest(u"sc/qa/extras/testdocuments/"_ustr)
{
}

void ScSearchStylesTest::setUp()
{
    UnoApiTest::setUp();

    // Create a Calc document with a 3x3 grid of cells where each column has a different style out
    // of the default Accent 1/2/3 styles

    loadFromURL(u"private:factory/scalc"_ustr);

    css::uno::Reference<css::sheet::XSpreadsheetDocument> xDoc(mxComponent,
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::container::XIndexAccess> xSheets(xDoc->getSheets(),
                                                              css::uno::UNO_QUERY_THROW);
    m_xSheet.set(xSheets->getByIndex(0), css::uno::UNO_QUERY_THROW);

    for (sal_Int32 nCol = 0; nCol < 3; ++nCol)
    {
        for (sal_Int32 nRow = 0; nRow < 3; ++nRow)
        {
            css::uno::Reference<css::table::XCell> xCell = m_xSheet->getCellByPosition(nCol, nRow);
            xCell->setValue(nRow * 3 + nCol);

            css::uno::Reference<css::beans::XPropertySet> xPropertySet(xCell,
                                                                       css::uno::UNO_QUERY_THROW);
            xPropertySet->setPropertyValue(
                u"CellStyle"_ustr, css::uno::Any(u"Accent "_ustr + OUString::number(nCol + 1)));
        }
    }

    // Store a reference to the selection supplier of the controller
    css::uno::Reference<css::frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
    m_xSelectionSupplier.set(xModel->getCurrentController(), css::uno::UNO_QUERY_THROW);
}

namespace
{
OUString getCellRangeAddressString(const css::uno::Any& xCellRangeAny)
{
    css::uno::Reference<css::sheet::XCellRangeAddressable> xCellRangeAddressable;

    CPPUNIT_ASSERT(xCellRangeAny >>= xCellRangeAddressable);
    CPPUNIT_ASSERT(xCellRangeAddressable.is());

    css::table::CellRangeAddress aCellRangeAddress = xCellRangeAddressable->getRangeAddress();

    return OUString::number(aCellRangeAddress.Sheet) + OUStringChar(':')
           + OUString::number(aCellRangeAddress.StartColumn) + OUStringChar(':')
           + OUString::number(aCellRangeAddress.StartRow) + OUStringChar(':')
           + OUString::number(aCellRangeAddress.EndColumn) + OUStringChar(':')
           + OUString::number(aCellRangeAddress.EndRow);
}

OUString getCellAddressString(const css::uno::Reference<css::uno::XInterface>& xCellInterface)
{
    CPPUNIT_ASSERT(xCellInterface.is());

    css::uno::Reference<css::sheet::XCellAddressable> xCellAddressable(xCellInterface,
                                                                       css::uno::UNO_QUERY_THROW);

    css::table::CellAddress aCellAddress = xCellAddressable->getCellAddress();

    return OUString::number(aCellAddress.Sheet) + OUStringChar(':')
           + OUString::number(aCellAddress.Column) + OUStringChar(':')
           + OUString::number(aCellAddress.Row);
}
}

void ScSearchStylesTest::testSearchRectangle()
{
    // Select the topleft-most 3×2 rectangle
    css::uno::Reference<css::sheet::XSheetCellCursor> xCursor = m_xSheet->createCursor();
    xCursor->collapseToSize(3, 2);

    m_xSelectionSupplier->select(css::uno::Any(xCursor));

    css::uno::Reference<css::util::XSearchable> xSearchable = getSearchable();
    css::uno::Reference<css::util::XSearchDescriptor> xDescriptor
        = createSearchDescriptor(xSearchable, u"Accent 1"_ustr);

    // Try with findAll
    css::uno::Reference<css::container::XIndexAccess> xResults = xSearchable->findAll(xDescriptor);

    // xResults is null without the fix for tdf#144299
    CPPUNIT_ASSERT(xResults.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xResults->getCount());

    CPPUNIT_ASSERT_EQUAL(u"0:0:0:0:1"_ustr, getCellRangeAddressString(xResults->getByIndex(0)));

    // Try with findFirst/findNext
    css::uno::Reference<css::uno::XInterface> xResult = xSearchable->findFirst(xDescriptor);
    CPPUNIT_ASSERT_EQUAL(u"0:0:0"_ustr, getCellAddressString(xResult));
    xResult = xSearchable->findNext(xResult, xDescriptor);
    CPPUNIT_ASSERT_EQUAL(u"0:0:1"_ustr, getCellAddressString(xResult));
    CPPUNIT_ASSERT(!xSearchable->findNext(xResult, xDescriptor).is());
}

void ScSearchStylesTest::testSearchRectangles()
{
    // Select the topleft-most 2×1 rectangle and the bottomright-most 2×1 rectangle

    css::uno::Reference<css::lang::XMultiServiceFactory> xMSF(mxComponent,
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::sheet::XSheetCellRangeContainer> xRangeContainer(
        xMSF->createInstance(u"com.sun.star.sheet.SheetCellRanges"_ustr),
        css::uno::UNO_QUERY_THROW);

    css::table::CellRangeAddress a1b1 = { 0, 0, 0, 1, 0 };
    xRangeContainer->addRangeAddress(a1b1, false);
    css::table::CellRangeAddress a3b3 = { 0, 0, 2, 1, 2 };
    xRangeContainer->addRangeAddress(a3b3, false);

    m_xSelectionSupplier->select(css::uno::Any(xRangeContainer));

    css::uno::Reference<css::util::XSearchable> xSearchable = getSearchable();
    css::uno::Reference<css::util::XSearchDescriptor> xDescriptor
        = createSearchDescriptor(xSearchable, u"Accent 1"_ustr);

    // Try with findAll
    css::uno::Reference<css::container::XIndexAccess> xResults = xSearchable->findAll(xDescriptor);

    CPPUNIT_ASSERT(xResults.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xResults->getCount());

    CPPUNIT_ASSERT_EQUAL(u"0:0:0:0:0"_ustr, getCellRangeAddressString(xResults->getByIndex(0)));
    CPPUNIT_ASSERT_EQUAL(u"0:0:2:0:2"_ustr, getCellRangeAddressString(xResults->getByIndex(1)));

    // Try with findFirst/findNext
    css::uno::Reference<css::uno::XInterface> xResult = xSearchable->findFirst(xDescriptor);
    CPPUNIT_ASSERT_EQUAL(u"0:0:0"_ustr, getCellAddressString(xResult));
    xResult = xSearchable->findNext(xResult, xDescriptor);
    CPPUNIT_ASSERT_EQUAL(u"0:0:2"_ustr, getCellAddressString(xResult));
    CPPUNIT_ASSERT(!xSearchable->findNext(xResult, xDescriptor).is());
}

css::uno::Reference<css::util::XSearchable> ScSearchStylesTest::getSearchable()
{
    css::uno::Any xSelection = m_xSelectionSupplier->getSelection();

    css::uno::Reference<css::util::XSearchable> xSearchable;

    CPPUNIT_ASSERT(xSelection >>= xSearchable);
    CPPUNIT_ASSERT(xSearchable.is());

    return xSearchable;
}

css::uno::Reference<css::util::XSearchDescriptor> ScSearchStylesTest::createSearchDescriptor(
    const css::uno::Reference<css::util::XSearchable>& xSearchable, const OUString& sStyleName)
{
    css::uno::Reference<css::util::XSearchDescriptor> xDescriptor
        = xSearchable->createSearchDescriptor();

    xDescriptor->setSearchString(sStyleName);
    xDescriptor->setPropertyValue(u"SearchStyles"_ustr, css::uno::Any(true));

    return xDescriptor;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSearchStylesTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
