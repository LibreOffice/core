/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/graph.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/fodfexport/data/", "OpenDocument Text Flat XML")
    {
    }
};

DECLARE_FODFEXPORT_TEST(testTdf113696, "tdf113696.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test that an image which is written in svm format (image/x-vclgraphic)
    // is accompanied by a png fallback graphic.
    if (xmlDocUniquePtr pXmlDoc = parseExportedFile())
    {
        assertXPath(pXmlDoc, "/office:document/office:body/office:text/text:p/draw:frame/"
                             "draw:image[@draw:mime-type='image/x-vclgraphic']");
        assertXPath(pXmlDoc, "/office:document/office:body/office:text/text:p/draw:frame/"
                             "draw:image[@draw:mime-type='image/png']");
    }
}

DECLARE_FODFEXPORT_TEST(testTdf113696WriterImage, "tdf113696-writerimage.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Same as testTdf113696, but with a writer image instead of a draw image
    // (they use different code paths).
    if (xmlDocUniquePtr pXmlDoc = parseExportedFile())
    {
        assertXPath(pXmlDoc, "/office:document/office:body/office:text/text:p/draw:frame/"
                             "draw:image[@draw:mime-type='image/x-vclgraphic']");
        assertXPath(pXmlDoc, "/office:document/office:body/office:text/text:p/draw:frame/"
                             "draw:image[@draw:mime-type='image/png']");
    }
}

DECLARE_FODFEXPORT_TEST(testSvgImageRoundtrip, "SvgImageTest.fodt")
{
    // Related to tdf#123396

    // We should have one image (shape)
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // Get the shape and extract the Graphic
    uno::Reference<drawing::XShape> xShape = getShape(1);
    uno::Reference<beans::XPropertySet> XPropertySet(xShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(XPropertySet.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    XPropertySet->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);

    // The graphic should be SVG - so should contain a VectorGraphicData
    auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);

    // The VectorGraphicData type should be SVG
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Svg, pVectorGraphicData->getType());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
