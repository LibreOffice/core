/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xchild.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xsheetannotation.hxx>
#include <test/sheet/xsheetannotationshapesupplier.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/text/xtextrange.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScAnnontationObj : public CalcUnoApiTest,
                         public apitest::XChild,
                         public apitest::XServiceInfo,
                         public apitest::XSheetAnnotation,
                         public apitest::XSheetAnnotationShapeSupplier,
                         public apitest::XSimpleText,
                         public apitest::XTextRange
{
public:
    ScAnnontationObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<sheet::XSheetAnnotation> getAnnotation(table::CellAddress&) override;

    CPPUNIT_TEST_SUITE(ScAnnontationObj);

    // XChild
    CPPUNIT_TEST(testGetSetParent);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XSheetAnnotation
    CPPUNIT_TEST(testGetPosition);
    CPPUNIT_TEST(testGetAuthor);
    CPPUNIT_TEST(testGetDate);
    CPPUNIT_TEST(testGetIsVisible);
    CPPUNIT_TEST(testSetIsVisible);

    // XSheetAnnotationShapeSupplier
    CPPUNIT_TEST(testGetAnnotationShape);

    // XSimpleText
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);

    // XTextRange
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetText);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScAnnontationObj::ScAnnontationObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XServiceInfo("ScAnnotationObj", "com.sun.star.sheet.CellAnnotation")
{
}

uno::Reference<sheet::XSheetAnnotation>
ScAnnontationObj::getAnnotation(table::CellAddress& xCellAddress)
{
    // get the sheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(xCellAddress.Sheet),
                                               uno::UNO_QUERY_THROW);

    // get the cell
    uno::Reference<table::XCell> xCell(
        xSheet->getCellByPosition(xCellAddress.Column, xCellAddress.Row), uno::UNO_QUERY_THROW);

    // get the annotation from cell
    uno::Reference<sheet::XSheetAnnotationAnchor> xAnnotationAnchor(xCell, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotation> xSheetAnnotation(xAnnotationAnchor->getAnnotation(),
                                                             uno::UNO_QUERY_THROW);

    return xSheetAnnotation;
}

uno::Reference<uno::XInterface> ScAnnontationObj::init()
{
    // tested annotation is in sheet 0 cell C2
    table::CellAddress aCellAddress;
    aCellAddress.Sheet = 0;
    aCellAddress.Row = 1;
    aCellAddress.Column = 2;

    return getAnnotation(aCellAddress);
}

void ScAnnontationObj::setUp()
{
    CalcUnoApiTest::setUp();

    // get the test file
    OUString aFileURL;
    createFileURL("ScAnnotationObj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);
}

void ScAnnontationObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
