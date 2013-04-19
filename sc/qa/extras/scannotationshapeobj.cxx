/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/text/xtext.hxx>

#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#define NUMBER_OF_TESTS 1

namespace sc_apitest {

class ScAnnotationShapeObj : public UnoApiTest, apitest::XText
{
public:
    ScAnnotationShapeObj();


    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference<uno::XInterface> init();
    virtual uno::Reference<text::XTextContent> getTextContent();

    CPPUNIT_TEST_SUITE(ScAnnotationShapeObj);

    // XText
    CPPUNIT_TEST(testInsertRemoveTextContent);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
    static uno::Reference<text::XTextContent> mxField;
};

sal_Int32 ScAnnotationShapeObj::nTest = 0;
uno::Reference<lang::XComponent> ScAnnotationShapeObj::mxComponent;
uno::Reference<text::XTextContent> ScAnnotationShapeObj::mxField;

ScAnnotationShapeObj::ScAnnotationShapeObj() {}

void ScAnnotationShapeObj::setUp()
{
    ++nTest;
    UnoApiTest::setUp();
}

void ScAnnotationShapeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        mxField.clear();
        closeDocument(mxComponent);
    }

    UnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScAnnotationShapeObj::init()
{
#ifdef FIXME_REMOVE_WHEN_RE_BASE_COMPLETE
    if (!mxComponent.is())
        // Load an empty document.
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);

    // Use cell A1 for this.
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
    uno::Reference<sheet::XSheetAnnotationAnchor> xAnchor(xCell, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotation> xAnnotation = xAnchor->getAnnotation();
    uno::Reference<text::XSimpleText> xAnnoText(xAnnotation, UNO_QUERY_THROW);
    xAnnoText->setString("ScAnnotationShapeObj");

    uno::Reference<sheet::XSheetAnnotationShapeSupplier> xShapeSupp(xAnnotation, UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape = xShapeSupp->getAnnotationShape();

    return xShape;
#else
    return uno::Reference<drawing::XShape>();
#endif
}

uno::Reference<text::XTextContent> ScAnnotationShapeObj::getTextContent()
{
#ifdef FIXME_REMOVE_WHEN_RE_BASE_COMPLETE
    if (!mxField.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
        mxField.set(xSM->createInstance("com.sun.star.text.TextField.DateTime"), UNO_QUERY_THROW);
    }
#endif
    return mxField;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnotationShapeObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
