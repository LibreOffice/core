/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

CPPUNIT_PLUGIN_IMPLEMENT();

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
