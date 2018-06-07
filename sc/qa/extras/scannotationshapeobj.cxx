/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/text/xtext.hxx>
#include <test/drawing/captionshape.hxx>

#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScAnnotationShapeObj : public CalcUnoApiTest, public apitest::XText, public apitest::CaptionShape
{
public:
    ScAnnotationShapeObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<text::XTextContent> getTextContent() override;

    CPPUNIT_TEST_SUITE(ScAnnotationShapeObj);

    // XText
    CPPUNIT_TEST(testInsertRemoveTextContent);

    // CaptionShape
    CPPUNIT_TEST(testCaptionShapeProperties);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
    static uno::Reference<text::XTextContent> mxField;
};

uno::Reference<text::XTextContent> ScAnnotationShapeObj::mxField;

ScAnnotationShapeObj::ScAnnotationShapeObj()
    : CalcUnoApiTest("sc/qa/extras/testdocuments")
{
}

void ScAnnotationShapeObj::setUp()
{
    CalcUnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScAnnotationShapeObj::tearDown()
{
    mxField.clear();
    closeDocument(mxComponent);

    CalcUnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScAnnotationShapeObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);

    // Use cell A1 for this.

    table::CellAddress aNotePos(0, 0, 0);
    Reference<sheet::XSheetAnnotationsSupplier> xAnnosSupp(xSheet, UNO_QUERY_THROW);
    Reference<sheet::XSheetAnnotations> xAnnos(xAnnosSupp->getAnnotations(), UNO_SET_THROW);
    // non-empty string required by note implementation (real text will be added below)
    xAnnos->insertNew(aNotePos, OUString(' '));

    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
    uno::Reference<sheet::XSheetAnnotationAnchor> xAnchor(xCell, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(xAnchor->getAnnotation(), UNO_SET_THROW);
    uno::Reference<text::XSimpleText> xAnnoText(xAnnotation, UNO_QUERY_THROW);
    xAnnoText->setString("ScAnnotationShapeObj");

    uno::Reference<sheet::XSheetAnnotationShapeSupplier> xShapeSupp(xAnnotation, UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xShapeSupp->getAnnotationShape(), UNO_SET_THROW);

    return xShape;
}

uno::Reference<text::XTextContent> ScAnnotationShapeObj::getTextContent()
{
    if (!mxField.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
        mxField.set(xSM->createInstance("com.sun.star.text.TextField.DateTime"), UNO_QUERY_THROW);
    }
    return mxField;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnotationShapeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
