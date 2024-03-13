/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/container/xnamed.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScStyleObj : public UnoApiTest, public apitest::XNamed, public apitest::XPropertySet
{
public:
    ScStyleObj();

    virtual void setUp() override;

    virtual uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(ScStyleObj);

    // XNamed
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    CPPUNIT_TEST_SUITE_END();
};

ScStyleObj::ScStyleObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XNamed("ScStyleObj")
    , XPropertySet({ "BottomBorder",
                     "BottomBorder2",
                     "BottomBorderComplexColor",
                     "CellProtection",
                     "CharLocale",
                     "CharLocaleAsian",
                     "CharLocaleComplex",
                     "CharPosture",
                     "CharPostureAsian",
                     "CharPostureComplex",
                     "DiagonalBLTR",
                     "DiagonalBLTR2",
                     "DiagonalTLBR",
                     "DiagonalTLBR2",
                     "HoriJustify",
                     "LeftBorder",
                     "LeftBorder2",
                     "LeftBorderComplexColor",
                     "NumberFormat",
                     "Orientation",
                     "RightBorder",
                     "RightBorder2",
                     "RightBorderComplexColor",
                     "ShadowFormat",
                     "TableBorder",
                     "TopBorder",
                     "TopBorder2",
                     "TopBorderComplexColor",
                     "UserDefinedAttributes",
                     "CellBackgroundComplexColor" })
{
}

uno::Reference<uno::XInterface> ScStyleObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<style::XStyleFamiliesSupplier> xSFS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA_StyleFamilies(xSFS->getStyleFamilies(),
                                                             uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA_StyleFamilies(xNA_StyleFamilies,
                                                              uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA_StyleFamily(xIA_StyleFamilies->getByIndex(0),
                                                           uno::UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<style::XStyle> xStyle(xMSF->createInstance("com.sun.star.style.CellStyle"),
                                         uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameContainer> xNC(xNA_StyleFamily, uno::UNO_QUERY_THROW);
    if (xNC->hasByName("ScStyleObj"))
    {
        xNC->removeByName("ScStyleObj");
    }
    xNC->insertByName("ScStyleObj", uno::Any(xStyle));

    uno::Reference<container::XIndexAccess> xIA_sheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA_sheets->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell = xSheet0->getCellByPosition(2, 3);
    uno::Reference<beans::XPropertySet> xPS(xCell, uno::UNO_QUERY_THROW);
    xPS->setPropertyValue("CellStyle", uno::Any(xStyle->getName()));

    return xStyle;
}

void ScStyleObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScStyleObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
