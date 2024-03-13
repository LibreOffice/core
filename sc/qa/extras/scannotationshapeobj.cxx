/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/drawing/captionshape.hxx>
#include <test/drawing/xgluepointssupplier.hxx>
#include <test/drawing/xshape.hxx>
#include <test/drawing/xshapedescriptor.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/text/xtext.hxx>
#include <test/text/xtextrange.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScAnnotationShapeObj : public UnoApiTest,
                             public apitest::CaptionShape,
                             public apitest::XGluePointsSupplier,
                             public apitest::XShape,
                             public apitest::XShapeDescriptor,
                             public apitest::XSimpleText,
                             public apitest::XText,
                             public apitest::XTextRange
{
public:
    ScAnnotationShapeObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<text::XTextContent> getTextContent() override;

    CPPUNIT_TEST_SUITE(ScAnnotationShapeObj);

    // CaptionShape
    CPPUNIT_TEST(testCaptionShapeProperties);

    // XGluePointsSupplier
    CPPUNIT_TEST(testGetGluePoints);

    // XShape
    CPPUNIT_TEST(testGetSetSize);
    CPPUNIT_TEST(testGetSetPosition);

    // XShapeDescriptor
    CPPUNIT_TEST(testGetShapeType);

    // XSimpleText
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertControlCharacter);
    CPPUNIT_TEST(testInsertString);

    // XText
    CPPUNIT_TEST(testInsertRemoveTextContent);

    // XTextRange
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetText);

    CPPUNIT_TEST_SUITE_END();

private:
    static uno::Reference<text::XTextContent> m_xField;
};

uno::Reference<text::XTextContent> ScAnnotationShapeObj::m_xField;

ScAnnotationShapeObj::ScAnnotationShapeObj()
    : UnoApiTest("sc/qa/extras/testdocuments")
    , XShapeDescriptor("com.sun.star.drawing.CaptionShape")
{
}

void ScAnnotationShapeObj::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScAnnotationShapeObj::tearDown()
{
    m_xField.clear();
    UnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScAnnotationShapeObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    // Use cell A1 for this.
    table::CellAddress aNotePos(0, 0, 0);
    uno::Reference<sheet::XSheetAnnotationsSupplier> xAnnosSupp(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotations> xAnnos(xAnnosSupp->getAnnotations(),
                                                    uno::UNO_SET_THROW);
    // non-empty string required by note implementation (real text will be added below)
    xAnnos->insertNew(aNotePos, OUString(' '));

    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
    uno::Reference<sheet::XSheetAnnotationAnchor> xAnchor(xCell, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(xAnchor->getAnnotation(),
                                                        uno::UNO_SET_THROW);
    uno::Reference<text::XSimpleText> xAnnoText(xAnnotation, uno::UNO_QUERY_THROW);
    xAnnoText->setString("ScAnnotationShapeObj");

    uno::Reference<sheet::XSheetAnnotationShapeSupplier> xShapeSupp(xAnnotation,
                                                                    uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xShapeSupp->getAnnotationShape(), uno::UNO_SET_THROW);

    return xShape;
}

uno::Reference<text::XTextContent> ScAnnotationShapeObj::getTextContent()
{
    if (!m_xField.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);
        m_xField.set(xSM->createInstance("com.sun.star.text.TextField.DateTime"),
                     uno::UNO_QUERY_THROW);
    }
    return m_xField;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnotationShapeObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
