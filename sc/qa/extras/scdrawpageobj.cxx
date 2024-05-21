/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/helper/shape.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/drawing/xshapegrouper.hxx>
#include <test/drawing/xshapes.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScDrawPageObj : public UnoApiTest,
                      public apitest::XElementAccess,
                      public apitest::XIndexAccess,
                      public apitest::XServiceInfo,
                      public apitest::XShapeGrouper,
                      public apitest::XShapes
{
public:
    ScDrawPageObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScDrawPageObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XShapeGrouper
    CPPUNIT_TEST(testGroup);
    CPPUNIT_TEST(testUngroup);

    // XShapes
    CPPUNIT_TEST(testAddRemove);

    CPPUNIT_TEST_SUITE_END();
};

ScDrawPageObj::ScDrawPageObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<drawing::XShape>::get())
    , XIndexAccess(2)
    , XServiceInfo(u"ScPageObj"_ustr, u"com.sun.star.sheet.SpreadsheetDrawPage"_ustr)
{
}

uno::Reference<uno::XInterface> ScDrawPageObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDP(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    xDP->insertNewByIndex(0);
    xDP->insertNewByIndex(1);

    uno::Reference<drawing::XDrawPage> xDrawPage(xDP->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XShape> xRectangle0(
        apitest::helper::shape::createRectangle(mxComponent, 7500, 5000, 5000, 3500),
        uno::UNO_SET_THROW);
    xShapes->add(xRectangle0);
    uno::Reference<drawing::XShape> xRectangle1(
        apitest::helper::shape::createRectangle(mxComponent, 5000, 5000, 5000, 5500),
        uno::UNO_SET_THROW);
    xShapes->add(xRectangle1);

    // needed for XShapeGrouper tests
    setDrawPage(xDrawPage);
    // needed for XShapes tests
    setShape(apitest::helper::shape::createLine(mxComponent, 7500, 10000, 5000, 3500));
    return xDrawPage;
}

void ScDrawPageObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDrawPageObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
