/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testDrawingmlFlipv, "drawingml-flipv.docx")
{
    // The problem was that the shape had vertical flip only, but then we added rotation as well on export.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "rot");
}

DECLARE_OOXMLEXPORT_TEST(testRot90Fliph, "rot90-fliph.docx")
{
    // The problem was that a shape rotation of 90° got turned into 270° after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipH", "1");
        // This was 16200000 (270 * 60000).
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "5400000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testRot180Flipv, "rot180-flipv.docx")
{
    // 180° rotation got lost after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipV", "1");
        // This attribute was completely missing.
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "10800000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testRot270Flipv, "rot270-flipv.docx")
{
    // 270° rotation got turned into 90° after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipV", "1");
        // This was 5400000.
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "16200000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testWpsCharColor, "wps-char-color.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was -1, i.e. the character color was default (-1), not white.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff),
                         getProperty<sal_Int32>(xShape->getStart(), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRightEdge, "textbox-right-edge.docx")
{
    // I'm fairly sure this is not specific to DOCX, but the doc model created
    // by the ODF import doesn't trigger this bug, so let's test this here
    // instead of uiwriter.
    int nShapeLeft = parseDump("//SwAnchoredDrawObject/bounds", "left").toInt32();
    int nShapeWidth = parseDump("//SwAnchoredDrawObject/bounds", "width").toInt32();
    int nTextboxLeft = parseDump("//fly/infos/bounds", "left").toInt32();
    int nTextboxWidth = parseDump("//fly/infos/bounds", "width").toInt32();
    // This is a rectangle, make sure the right edge of the textbox is still
    // inside the draw shape.
    CPPUNIT_ASSERT(nShapeLeft + nShapeWidth >= nTextboxLeft + nTextboxWidth);
}

DECLARE_OOXMLEXPORT_TEST(testEffectExtentMargin, "effectextent-margin.docx")
{
    // This was 318, i.e. oox::drawingml::convertEmuToHmm(114300), effectExtent
    // wasn't part of the margin, leading to the fly not taking enough space
    // around itself.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300 + 95250),
                         getProperty<sal_Int32>(getShape(1), "LeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testCropPixel, "crop-pixel.docx")
{
    // If map mode of the graphic is in pixels, then we used to handle original
    // size of the graphic as mm100, but it was in pixels.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This is 17667 in the original document, was 504666 (so the image
        // become invisible), now is around 19072.
        CPPUNIT_ASSERT(getXPath(pXmlDoc, "//a:srcRect", "l").toInt32() <= 22452);
    }
}

DECLARE_OOXMLEXPORT_TEST(testEffectExtent, "effect-extent.docx")
{
    // The problem was that in case there were no shadows on the picture, we
    // wrote a <wp:effectExtent> full or zeros.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // E.g. this was 0.
        assertXPath(pXmlDoc, "//wp:effectExtent", "l", "114300");
}

DECLARE_OOXMLEXPORT_TEST(testEffectExtentInline, "effect-extent-inline.docx")
{
    // The problem was that in case there was inline rotated picture, we
    // wrote a <wp:effectExtent> full or zeros.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // E.g. this was 0.
        assertXPath(pXmlDoc, "//wp:effectExtent", "l", "609600");
        assertXPath(pXmlDoc, "//wp:effectExtent", "r", "590550");
        assertXPath(pXmlDoc, "//wp:effectExtent", "t", "590550");
        assertXPath(pXmlDoc, "//wp:effectExtent", "b", "571500");
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
