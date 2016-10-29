/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95031, "tdf95031.docx")
{
    // This was 494, in-numbering paragraph's automating spacing was handled as visible spacing, while it should not.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103389, "tdf103389.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // No geometry was exported for the second canvas
    // Check both canvases' geometry
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", "rect");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", "rect");
}

DECLARE_OOXMLEXPORT_TEST(testTdf84678, "tdf84678.docx")
{
    // This was 0, left margin inside a shape+text wasn't imported from DOCX.
    // 360000 EMU, but layout uses twips.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(567), parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds", "left").toInt32());

}

DECLARE_OOXMLEXPORT_TEST(testTdf103544, "tdf103544.docx")
{
    // We have two shapes: a frame and an image
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());

    // Image was lost because of the frame export
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103573, "tdf103573.docx")
{
    // Relative positions to the left or right margin (MS Word naming) was not handled.
    uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to left page border", text::RelOrientation::PAGE_LEFT, nValue);

    xShapeProperties.set( getShape(2), uno::UNO_QUERY );
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to right page border", text::RelOrientation::PAGE_RIGHT, nValue);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
