/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/objectpositioning/data/";

/// Covers sw/source/core/objectpositioning/ fixes.
class SwCoreObjectpositioningTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreObjectpositioningTest, testOverlapCrash)
{
    // Load a document with 2 images.
    load(DATA_DIRECTORY, "overlap-crash.odt");

    // Change their anchor type to to-char.
    uno::Reference<beans::XPropertySet> xShape1(getShape(1), uno::UNO_QUERY);
    xShape1->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<beans::XPropertySet> xShape2(getShape(1), uno::UNO_QUERY);
    xShape2->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert a new paragraph at the start.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    // Without the accompanying fix in place, this test would have crashed.
    pWrtShell->SplitNode();
}

CPPUNIT_TEST_FIXTURE(SwCoreObjectpositioningTest, testVertPosFromBottom)
{
    // Create a document, insert a shape and position it 1cm above the bottom of the body area.
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("AnchorType",
                                  uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProps->setPropertyValue("VertOrient", uno::makeAny(text::VertOrientation::NONE));
    xShapeProps->setPropertyValue("VertOrientRelation",
                                  uno::makeAny(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    xShapeProps->setPropertyValue("VertOrientPosition",
                                  uno::makeAny(static_cast<sal_Int32>(-11000)));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Verify that the distance between the body and anchored object bottom is indeed around 1cm.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32();
    sal_Int32 nAnchoredBottom
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "bottom").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 564
    // - Actual  : 9035
    // i.e. the vertical position was from-top, not from-bottom.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(564), nBodyBottom - nAnchoredBottom);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
