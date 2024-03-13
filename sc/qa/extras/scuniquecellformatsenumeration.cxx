/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <sal/types.h>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
namespace
{
struct RGBColor
{
    int m_nRed;
    int m_nGreen;
    int m_nBlue;

    RGBColor(int nRed, int nGreen, int nBlue)
        : m_nRed(nRed)
        , m_nGreen(nGreen)
        , m_nBlue(nBlue)
    {
    }

    sal_Int32 hashCode() const { return (255 << 24) | (m_nRed << 16) | (m_nGreen << 8) | m_nBlue; }
};
}

class ScUniqueCellFormatsEnumeration : public UnoApiTest, public apitest::XEnumeration
{
public:
    ScUniqueCellFormatsEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScUniqueCellFormatsEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();

private:
    void changeColor(const uno::Reference<sheet::XSpreadsheet>& xSheet, const OUString& sRangeName,
                     const RGBColor& rgb);
};

ScUniqueCellFormatsEnumeration::ScUniqueCellFormatsEnumeration()
    : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScUniqueCellFormatsEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    changeColor(xSheet0, "A1:A5", RGBColor(0, 255, 0));
    changeColor(xSheet0, "A6:B10", RGBColor(255, 0, 0));
    changeColor(xSheet0, "B1:B6", RGBColor(0, 0, 255));
    changeColor(xSheet0, "B7", RGBColor(0, 255, 0));
    changeColor(xSheet0, "B8:B10", RGBColor(0, 0, 255));
    changeColor(xSheet0, "C1:C10", RGBColor(0, 0, 255));
    changeColor(xSheet0, "D1:D10", RGBColor(0, 255, 0));

    uno::Reference<sheet::XUniqueCellFormatRangesSupplier> xUCFRS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> xEA(xUCFRS->getUniqueCellFormatRanges(),
                                                      uno::UNO_QUERY_THROW);
    return xEA->createEnumeration();
}

void ScUniqueCellFormatsEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScUniqueCellFormatsEnumeration::changeColor(const uno::Reference<sheet::XSpreadsheet>& xSheet,
                                                 const OUString& sRangeName, const RGBColor& rgb)
{
    uno::Reference<table::XCellRange> xCellRange(xSheet->getCellRangeByName(sRangeName),
                                                 uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xCellRange, uno::UNO_QUERY_THROW);

    sal_Int32 nColor = 16777216 + rgb.hashCode();
    uno::Any aValue;
    aValue <<= nColor;
    xPropertySet->setPropertyValue("CellBackColor", aValue);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScUniqueCellFormatsEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
