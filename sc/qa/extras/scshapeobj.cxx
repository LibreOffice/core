/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/drawing/xshapedescriptor.hxx>
#include <test/lang/xcomponent.hxx>
#include <test/sheet/shape.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScShapeObj : public CalcUnoApiTest,
                   public apitest::Shape,
                   public apitest::XComponent,
                   public apitest::XShapeDescriptor
{
public:
    ScShapeObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXSheetDocument() override;
    virtual void triggerDesktopTerminate() override{};
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScShapeObj);

    // Shape
    CPPUNIT_TEST(testShapePropertiesAnchor);
    CPPUNIT_TEST(testShapePropertiesPosition);

    // XComponent
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testRemoveEventListener);

    // XShapeDescriptor
    CPPUNIT_TEST(testGetShapeType);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScShapeObj::ScShapeObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XShapeDescriptor("com.sun.star.drawing.RectangleShape")
{
}

uno::Reference<uno::XInterface> ScShapeObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(
        xMSF->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY_THROW);
    xShape->setPosition(awt::Point(5000, 3500));
    xShape->setSize(awt::Size(7500, 5000));

    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDPS->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
    xShapes->add(xShape);

    for (auto i = 0; i < 10; i++)
    {
        xShape->setPosition(awt::Point(5000, 3500));
        xShape->setSize(awt::Size(7510 + 10 * i, 5010 + 10 * i));
        xShapes->add(xShape);
    }

    return xShape;
}

uno::Reference<uno::XInterface> ScShapeObj::getXSheetDocument()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    return xDoc;
}

void ScShapeObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScShapeObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScShapeObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
