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
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <config_features.h>
#include <unotools/fltrcfg.hxx>
#include <comphelper/propertysequence.hxx>
#include <string>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (OString(filename) == "smartart.docx" || OString(filename) == "strict-smartart.docx")
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([]() { SvtFilterOptions::Get().SetSmartArt2Shape(false); }));
            SvtFilterOptions::Get().SetSmartArt2Shape(true);
            return pResetter;
        }
        return nullptr;
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
    /// Copy&paste helper.
    bool paste(const OUString& rFilename, const uno::Reference<text::XTextRange>& xTextRange)
    {
        uno::Reference<document::XFilter> xFilter(
            m_xSFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"),
            uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(
            m_directories.getURLFromSrc("/sw/qa/extras/ooxmlexport/data/") + rFilename,
            StreamMode::READ);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
            { "InputStream", uno::makeAny(xStream) },
            { "InputMode", uno::makeAny(true) },
            { "TextInsertModeRange", uno::makeAny(xTextRange) },
        }));
        return xFilter->filter(aDescriptor);
    }
};

#if 0
class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
    * Blacklist handling
    */

protected:
};
#endif

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

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // 1st problem: last character was missing
    CPPUNIT_ASSERT_EQUAL(OUString("SAMPLE"), xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(xShape, "CustomShapeGeometry");
    bool bFound = false;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "TextPath")
            bFound = true;
    // 2nd problem: v:textpath wasn't imported
    CPPUNIT_ASSERT_EQUAL(true, bFound);

    // 3rd problem: rotation angle was 315, not 45.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45 * 100), getProperty<sal_Int32>(xShape, "RotateAngle"));

    // 4th problem: mso-position-vertical-relative:margin was ignored, VertOrientRelation was text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "VertOrientRelation"));

    // These problems were in the exporter
    // The textpath wasn't semi-transparent.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));
    // The textpath had a stroke.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE,
                         getProperty<drawing::LineStyle>(xShape, "LineStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testWatermark_2, "watermark.docx")
{
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);

    sal_Int32 nHeight = xShape->getSize().Height;

    // Rounding errors
    sal_Int32 nDifference = 5150 - nHeight;
    std::stringstream ss;
    ss << "Difference: " << nDifference << " TotalHeight: " << nHeight;
#ifndef _WIN32
    // FIXME why does this sometimes fail?
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference <= 4);
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference >= -4);
#endif
}

DECLARE_OOXMLEXPORT_TEST(testWatermarkFont, "watermark-font.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TestFont"), xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    OUString aFont;
    float nFontSize;

    // Check font family
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("CharFontName") >>= aFont);
    CPPUNIT_ASSERT_EQUAL(OUString("DejaVu Serif"), aFont);

    // Check font size
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("CharHeight") >>= nFontSize);
    CPPUNIT_ASSERT_EQUAL(float(72), nFontSize);
}

DECLARE_OOXMLEXPORT_TEST(testFdo66688, "fdo66688.docx")
{
    // The problem was that TextFrame imported and exported the wrong value for transparency
    // (was stored as 'FillTransparence' instead of 'BackColorTransparency'
    uno::Reference<beans::XPropertySet> xFrame(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(80), getProperty<sal_Int32>(xFrame, "FillTransparence"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc581614, "bnc581614.doc")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66929, "fdo66929.docx")
{
    // The problem was that the default 'inset' attribute of the 'textbox' node was exported incorrectly.
    // A node like '<v:textbox inset="0">' was exported back as '<v:textbox inset="0pt,0pt,0pt,0pt">'
    // This is wrong because the original node denotes a specific 'left' inset, and a default 'top','right','bottom' inset
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // VML import -> TextFrame
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xFrame, "LeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xFrame, "TopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(254), getProperty<sal_Int32>(xFrame, "RightBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127),
                             getProperty<sal_Int32>(xFrame, "BottomBorderDistance"));
    }
    else
    {
        // drawingML import -> shape with TextBox
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "TextLeftDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, "TextUpperDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(254), getProperty<sal_Int32>(xShape, "TextRightDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, "TextLowerDistance"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo67737, "fdo67737.docx")
{
    // The problem was that imported shapes did not import and render the 'flip:x' and 'flip:y' attributes
    uno::Reference<drawing::XShape> xArrow = getShape(1);
    uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(xArrow, "CustomShapeGeometry");
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "MirroredY")
        {
            CPPUNIT_ASSERT_EQUAL(true, rProp.Value.get<bool>());
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find MirroredY=true property");
}

DECLARE_OOXMLEXPORT_TEST(testTransparentShadow, "transparent-shadow.docx")
{
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    sal_Int32 nShadowColor = getProperty<sal_Int32>(xPicture, "ShadowColor");
    sal_Int16 nShadowTransparence = getProperty<sal_Int16>(xPicture, "ShadowTransparence");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x808080), nShadowColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), nShadowTransparence);
}

DECLARE_OOXMLEXPORT_TEST(NoFillAttrInImagedata, "NoFillAttrInImagedata.docx")
{
    //problem was that type and color2 which are v:fill attributes were written in 'v:imagedata'
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathNoAttribute(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:imagedata",
        "type");
    assertXPathNoAttribute(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:imagedata",
        "color2");
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxGradientAngle, "fdo65295.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xIndexAccess->getCount());

    // Angle of frame#1 is 135 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame1, "FillStyle"));
    awt::Gradient aGradient1 = getProperty<awt::Gradient>(xFrame1, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(135 * 10), aGradient1.Angle);

    // Angle of frame#2 is 180 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame2, "FillStyle"));
    awt::Gradient aGradient2 = getProperty<awt::Gradient>(xFrame2, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(180 * 10), aGradient2.Angle);

    // Angle of frame#3 is  90 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame3(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame3, "FillStyle"));
    awt::Gradient aGradient3 = getProperty<awt::Gradient>(xFrame3, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(90 * 10), aGradient3.Angle);

    // Angle of frame#4 is 225 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame4(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame4, "FillStyle"));
    awt::Gradient aGradient4 = getProperty<awt::Gradient>(xFrame4, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(225 * 10), aGradient4.Angle);

    // Angle of frame#5 is 270 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame5(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame5, "FillStyle"));
    awt::Gradient aGradient5 = getProperty<awt::Gradient>(xFrame5, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270 * 10), aGradient5.Angle);

    // Angle of frame#6 is 315 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame6(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame6, "FillStyle"));
    awt::Gradient aGradient6 = getProperty<awt::Gradient>(xFrame6, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(315 * 10), aGradient6.Angle);

    // Angle of frame#7 is   0 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame7(xIndexAccess->getByIndex(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame7, "FillStyle"));
    awt::Gradient aGradient7 = getProperty<awt::Gradient>(xFrame7, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0 * 10), aGradient7.Angle);

    // Angle of frame#8 is  45 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame8(xIndexAccess->getByIndex(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame8, "FillStyle"));
    awt::Gradient aGradient8 = getProperty<awt::Gradient>(xFrame8, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(45 * 10), aGradient8.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTriangle, "ooxml-triangle.docx")
{
    // The problem was that ooxml-triangle shape type wasn't handled by VML
    // export (only isosceles-triangle), leading to a missing shape.
    getShape(1);
}

DECLARE_OOXMLEXPORT_TEST(testMce, "mce.docx")
{
    // The shape is red in Word2007, green in Word2010. Check that our import follows the later.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x9bbb59), getProperty<sal_Int32>(getShape(1), "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testRelorientation, "relorientation.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // This was text::RelOrientation::FRAME, when handling relativeFrom=page, align=right
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));

    uno::Reference<drawing::XShapes> xGroup(xShape, uno::UNO_QUERY);
    // This resulted in lang::IndexOutOfBoundsException, as nested groupshapes weren't handled.
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"),
                         xShapeDescriptor->getShapeType());

    // Right after import we get a rounding error: 8662 vs 8664.
    if (mbExported)
    {
        uno::Reference<drawing::XShape> xYear(xGroup->getByIndex(1), uno::UNO_QUERY);
        // This was 2, due to incorrect handling of parent transformations inside DML groupshapes.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), xYear->getSize().Width);
    }
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTextbox, "groupshape-textbox.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // The VML export lost text on textboxes inside groupshapes.
    // The DML export does not, make sure it stays that way.
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xShape->getString());
    // This was 16, i.e. inheriting doc default char height didn't work.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapePicture, "groupshape-picture.docx")
{
    // Picture in the groupshape got lost, groupshape had only one child.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(1),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GraphicObjectShape"),
                         xShapeDescriptor->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testAutofit, "autofit.docx")
{
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "TextAutoGrowHeight"));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "TextAutoGrowHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxPictureFill, "textbox_picturefill.docx")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    auto xBitmap = getProperty<uno::Reference<awt::XBitmap>>(xFrame, "FillBitmap");
    CPPUNIT_ASSERT(xBitmap.is());
    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    CPPUNIT_ASSERT(aGraphic);
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0L);
    CPPUNIT_ASSERT_EQUAL(447L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(528L, aGraphic.GetSizePixel().Height());
}

DECLARE_OOXMLEXPORT_TEST(testMsoSpt180, "mso-spt180.docx")
{
    if (!mbExported)
        return;

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty<uno::Sequence<beans::PropertyValue>>(
        xGroup->getByIndex(0), "CustomShapeGeometry");
    OUString aType;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "Type")
            aType = aProps[i].Value.get<OUString>();
    // This was exported as borderCallout90, which is an invalid drawingML preset shape string.
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-borderCallout1"), aType);
}

DECLARE_OOXMLEXPORT_TEST(testPageRelSize, "pagerelsize.docx")
{
    // First shape: width is relative from page, but not height.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));

    // Second shape: height is relative from page, but not height.
    xShape = getShape(2);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
}

DECLARE_OOXMLEXPORT_TEST(testRelSizeRound, "rel-size-round.docx")
{
    // This was 9: 9.8 was imported as 9 instead of being rounded to 10.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), getProperty<sal_Int16>(getShape(1), "RelativeHeight"));
}

#if 0
// Currently LibreOffice exports custom geometry for this up arrow, not preset shape.
// When LibreOffice can export preset shape with correct modifiers, then this test can be re-enabled.

DECLARE_OOXMLEXPORT_TEST(testFdo76016, "fdo76016.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf83227, "tdf83227.docx")
{
    // Bug document contains a rotated image, which is handled as a draw shape (not as a Writer image) on export.
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/media/image1.png")));
    // This was also true, image was written twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/image2.png")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103001, "tdf103001.docx")
{
    // The same image is featured in the header and in the body text, make sure
    // the header relation is still written, even when caching is enabled.
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // This failed: header reused the RelId of the body text, even if RelIds
    // are local to their stream.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/_rels/header1.xml.rels"));
}

DECLARE_OOXMLEXPORT_TEST(testNoDuplicateAttributeExport, "duplicate-east-asia.odt")
{
    // File asserting while saving in LO.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testFDO77117, "fdo77117.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This checks textbox textrun size of font which is in group shape.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testPageBreakInFirstPara, "fdo77727.docx")
{
    /* Break to next page was not exported if it is in first paragraph of the section.
    * Now after fix , LO writes Next Page Break and also preserves <w:br> tag.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78284, "fdo78284.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/word/media/"
                "OOXDiagramDataRels1_0.png']",
                "ContentType", "image/png");
}

DECLARE_OOXMLEXPORT_TEST(testKDE302504, "kde302504-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//v:shape", "ID", "KoPathShape");
}

DECLARE_OOXMLEXPORT_TEST(testKDE216114, "kde216114-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:pict", 1);
}

DECLARE_OOXMLEXPORT_TEST(testDmlShapeTitle, "dml-shape-title.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), getProperty<OUString>(getShape(1), "Title"));
    CPPUNIT_ASSERT_EQUAL(OUString("Description"),
                         getProperty<OUString>(getShape(1), "Description"));
}

DECLARE_OOXMLEXPORT_TEST(testDmlZorder, "dml-zorder.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This was "0": causing that in Word, the second shape was on top, while in the original odt the first shape is on top.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor",
                "relativeHeight", "2");
}

DECLARE_OOXMLEXPORT_TEST(testDmlShapeRelsize, "dml-shape-relsize.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Relative size wasn't exported all, then relativeFrom was "page", not "margin".
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "wp14:sizeRelH",
                "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testDmlPictureInTextframe, "dml-picture-in-textframe.docx")
{
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/media/image1.gif")));
    // This was also true, image was written twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/image2.gif")));
}

DECLARE_OOXMLEXPORT_TEST(testDmlGroupshapeRelsize, "dml-groupshape-relsize.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Relative size wasn't imported.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "wp14:sizeRelH",
                "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testDmlTextshape, "dml-textshape.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::TOP, getProperty<sal_Int16>(xGroup, "VertOrient"));
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was drawing::FillStyle_NONE.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID,
                         getProperty<drawing::FillStyle>(xShape, "FillStyle"));
    // This was drawing::LineStyle_NONE.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID,
                         getProperty<drawing::LineStyle>(xShape, "LineStyle"));

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    // This was wrap="none".
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:bodyPr",
                "wrap", "square");

    xShape.set(xGroup->getByIndex(3), uno::UNO_QUERY);
    OUString aType = comphelper::SequenceAsHashMap(
                         getProperty<beans::PropertyValues>(xShape, "CustomShapeGeometry"))["Type"]
                         .get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-bentConnector3"), aType);
    // Connector was incorrectly shifted towards the top left corner, X was 552, Y was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4018), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-4487), xShape->getPosition().Y);

    xShape.set(xGroup->getByIndex(5), uno::UNO_QUERY);
    // This was incorrectly shifted towards the top of the page, Y was 106.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-4727), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDMLSolidfillAlpha, "dml-solidfill-alpha.docx")
{
    // Problem was that the transparency was not exported (a:alpha).
    // RGB color (a:srgbClr)
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(70), getProperty<sal_Int16>(xShape, "FillTransparence"));

    // Theme color (a:schemeClr)
    xShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, "FillTransparence"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLCustomGeometry, "dml-customgeometry-cubicbezier.docx")
{
    // The problem was that a custom shape was not exported.
    uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            rProp.Value >>= aPairs;
        else if (rProp.Name == "Segments")
            rProp.Value >>= aSegments;
    }

    // (a:moveTo)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::MOVETO),
                         aSegments[0].Command);

    // (a:cubicBezTo)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CURVETO),
                         aSegments[1].Command);

    // Coordinates
    sal_Int32 nLength = 16;
    CPPUNIT_ASSERT_EQUAL(nLength, aPairs.getLength());
    std::pair<sal_Int32, sal_Int32> aCoordinates[] = {
        std::pair<sal_Int32, sal_Int32>(9084, 0),    std::pair<sal_Int32, sal_Int32>(6734, 689),
        std::pair<sal_Int32, sal_Int32>(4489, 893),  std::pair<sal_Int32, sal_Int32>(2633, 893),
        std::pair<sal_Int32, sal_Int32>(1631, 893),  std::pair<sal_Int32, sal_Int32>(733, 830),
        std::pair<sal_Int32, sal_Int32>(0, 752),     std::pair<sal_Int32, sal_Int32>(987, 908),
        std::pair<sal_Int32, sal_Int32>(2274, 1034), std::pair<sal_Int32, sal_Int32>(3756, 1034),
        std::pair<sal_Int32, sal_Int32>(5357, 1034), std::pair<sal_Int32, sal_Int32>(7183, 877),
        std::pair<sal_Int32, sal_Int32>(9084, 423),  std::pair<sal_Int32, sal_Int32>(9084, 0),
        std::pair<sal_Int32, sal_Int32>(9084, 0),    std::pair<sal_Int32, sal_Int32>(9084, 0)
    };

    for (int i = 0; i < nLength; ++i)
    {
        CPPUNIT_ASSERT(abs(aCoordinates[i].first - aPairs[i].First.Value.get<sal_Int32>()) < 20);
        CPPUNIT_ASSERT(abs(aCoordinates[i].second - aPairs[i].Second.Value.get<sal_Int32>()) < 20);
    }
}

DECLARE_OOXMLEXPORT_TEST(testDmlRectangleRelsize, "dml-rectangle-relsize.docx")
{
    // This was around 19560, as we did not read wp14:pctHeight for
    // drawinglayer shapes and the fallback data was invalid.
    OString aMessage("Height is only " + OString::number(getShape(1)->getSize().Height));
    CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), getShape(1)->getSize().Height >= 20967);

    // This was around 0: relative size of 0% was imported as 0, not "fall back to absolute size".
    CPPUNIT_ASSERT(getShape(2)->getSize().Height > 300);
}

DECLARE_OOXMLEXPORT_TEST(testDMLTextFrameVertAdjust, "dml-textframe-vertadjust.docx")
{
    // DOCX textboxes with text are imported as text frames but in Writer text frames did not have
    // TextVerticalAdjust attribute so far.

    // 1st frame's context is adjusted to the top
    uno::Reference<beans::XPropertySet> xFrame(getShapeByName("Rectangle 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP,
                         getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 2nd frame's context is adjusted to the center
    xFrame.set(getShapeByName("Rectangle 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER,
                         getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 3rd frame's context is adjusted to the bottom
    xFrame.set(getShapeByName("Rectangle 3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM,
                         getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillBitmapCrop, "dml-shape-fillbitmapcrop.docx")
{
    // Test the new GraphicCrop property which is introduced to define
    // cropping of shapes filled with a picture in stretch mode.

    // 1st shape has some cropping
    text::GraphicCrop aGraphicCropStruct
        = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 454 : 455), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 367 : 368), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -454 : -455), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -367 : -368), aGraphicCropStruct.Bottom);

    // 2nd shape has no cropping
    aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Bottom);
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillPattern, "dml-shape-fillpattern.docx")
{
    // Hatching was ignored by the export.

    // 1st shape: light horizontal pattern (ltHorz)
    drawing::Hatch aHatch = getProperty<drawing::Hatch>(getShape(1), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x99FF66), aHatch.Color);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 2nd shape: horizontal pattern (horz)
    aHatch = getProperty<drawing::Hatch>(getShape(2), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 3rd shape: light vertical pattern (ltVert)
    aHatch = getProperty<drawing::Hatch>(getShape(3), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 4th shape: vertical pattern (vert)
    aHatch = getProperty<drawing::Hatch>(getShape(4), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 5th shape: light upward diagonal pattern (ltUpDiag)
    aHatch = getProperty<drawing::Hatch>(getShape(5), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 6th shape: wide upward diagonal pattern (wdUpDiag)
    aHatch = getProperty<drawing::Hatch>(getShape(6), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 7th shape: light downward diagonal pattern (ltDnDiag)
    aHatch = getProperty<drawing::Hatch>(getShape(7), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1350), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 8th shape: wide downward diagonal pattern (wdDnDiag)
    aHatch = getProperty<drawing::Hatch>(getShape(8), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1350), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    // 9th shape: small grid pattern (smGrid)
    aHatch = getProperty<drawing::Hatch>(getShape(9), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    // 10th shape: large grid pattern (lgGrid)
    aHatch = getProperty<drawing::Hatch>(getShape(10), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    // 11th shape: small checker board pattern (smCheck)
    aHatch = getProperty<drawing::Hatch>(getShape(11), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    // 12th shape: outlined diamond pattern (openDmnd)
    aHatch = getProperty<drawing::Hatch>(getShape(12), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeChildPosition, "dml-groupshape-childposition.docx")
{
    // Problem was parent transformation was ignored fully, but translate component
    // which specify the position must be also applied for children of the group.

    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xChildGroup(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -2120 : -2122), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 11336 : 11333), xChildGroup->getPosition().Y);

    xGroup.set(xChildGroup, uno::UNO_QUERY);
    xChildGroup.set(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -1856 : -1858), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 11336 : 11333), xChildGroup->getPosition().Y);

    xChildGroup.set(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -2120 : -2122), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 14026 : 14023), xChildGroup->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDMLGradientFillTheme, "dml-gradientfill-theme.docx")
{
    // Problem was when a fill gradient was imported from a theme, (fillRef in ooxml)
    // not just the theme was written out but the explicit values too
    // Besides the duplication of values it causes problems with writing out
    // <a:schemeClr val="phClr"> into document.xml, while phClr can be used just for theme definitions.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // check no explicit gradFill has been exported
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill",
                0);

    // check shape style has been exported
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
                "idx", "2");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
                "val", "accent1");
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeParaSpacing, "dml-groupshape-paraspacing.docx")
{
    // Paragraph spacing (top/bottom margin and line spacing) inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();

    // 1st paragraph has 1.5x line spacing but it has no spacing before/after.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText), 1);
    style::LineSpacing aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(150), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 2nd paragraph has double line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(2, xText), 1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 3rd paragraph has 24 pt line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(3, xText), 1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::MINIMUM), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(847), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 4th paragraph has 1.75x line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(4, xText), 1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(175), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 5th paragraph has margins which are defined by w:beforeLines and w:afterLines.
    xRun.set(getRun(getParagraphOfText(5, xText), 1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(741), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 6th paragraph has margins which are defined by w:before and w:after.
    xRun.set(getRun(getParagraphOfText(6, xText), 1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 7th paragraph has auto paragraph margins a:afterAutospacing and a:beforeAutospacing, which means margins must be ignored.
    xRun.set(getRun(getParagraphOfText(7, xText), 1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69636, "fdo69636.docx")
{
    // The problem was that the mso-layout-flow-alt:bottom-to-top VML shape property wasn't handled for sw text frames.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xPropertySet->getPropertyValue("CustomShapeGeometry"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-270),
                         aCustomShapeGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testFdo69636_2, "fdo69636.docx")
{
    /*
    * The problem was that the exporter didn't mirror the workaround of the
    * importer, regarding the btLr text frame direction: the
    * mso-layout-flow-alt property was completely missing in the output.
    */
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    // VML
    CPPUNIT_ASSERT(
        getXPath(
            pXmlDoc,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox",
            "style")
            .match("mso-layout-flow-alt:bottom-to-top"));
    // drawingML
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:bodyPr",
                "vert", "vert270");
}

DECLARE_OOXMLEXPORT_TEST(testFdo70838, "fdo70838.docx")
{
    // The problem was that VMLExport::Commit didn't save the correct width and height,
    // and ImplEESdrWriter::ImplFlipBoundingBox made a mistake calculating the position

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // Check DML document

    sal_Int32 aXPos[4], aYPos[4];
    aXPos[0]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset")
              .toInt32();
    aXPos[1]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset")
              .toInt32();
    aXPos[2]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset")
              .toInt32();
    aXPos[3]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset")
              .toInt32();

    aYPos[0]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset")
              .toInt32();
    aYPos[1]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset")
              .toInt32();
    aYPos[2]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset")
              .toInt32();
    aYPos[3]
        = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/"
                                        "mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset")
              .toInt32();

    // certain degree of error is tolerated due to rounding in unit conversions
    CPPUNIT_ASSERT(abs(1239520 - aXPos[0]) < 1000);
    CPPUNIT_ASSERT(abs(1239520 - aXPos[1]) < 1000);
    CPPUNIT_ASSERT(abs(1238250 - aXPos[2]) < 1000);
    CPPUNIT_ASSERT(abs(1238885 - aXPos[3]) < 1000);

    CPPUNIT_ASSERT(abs(2095500 - aYPos[0]) < 1000);
    CPPUNIT_ASSERT(abs(2094865 - aYPos[1]) < 1000);
    CPPUNIT_ASSERT(abs(2094230 - aYPos[2]) < 1000);
    CPPUNIT_ASSERT(abs(2094865 - aYPos[3]) < 1000);

    sal_Int32 aHSize[4], aVSize[4];
    aHSize[0] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cx")
                    .toInt32();
    aHSize[1] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cx")
                    .toInt32();
    aHSize[2] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cx")
                    .toInt32();
    aHSize[3] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cx")
                    .toInt32();

    aVSize[0] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cy")
                    .toInt32();
    aVSize[1] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cy")
                    .toInt32();
    aVSize[2] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cy")
                    .toInt32();
    aVSize[3] = getXPath(pXmlDocument,
                         "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/"
                         "wp:anchor/wp:extent",
                         "cy")
                    .toInt32();

    // certain degree of error is tolerated due to rounding in unit conversions
    CPPUNIT_ASSERT(abs(3599280 - aHSize[0]) < 1000);
    CPPUNIT_ASSERT(abs(3599280 - aHSize[1]) < 1000);
    CPPUNIT_ASSERT(abs(3599280 - aHSize[2]) < 1000);
    CPPUNIT_ASSERT(abs(3599280 - aHSize[3]) < 1000);

    CPPUNIT_ASSERT(abs(1799640 - aVSize[0]) < 1000);
    CPPUNIT_ASSERT(abs(1799640 - aVSize[1]) < 1000);
    CPPUNIT_ASSERT(abs(1799640 - aVSize[2]) < 1000);
    CPPUNIT_ASSERT(abs(1799640 - aVSize[3]) < 1000);

    // Check VML document

    // get styles of the four shapes
    OUString aStyles[4];
    aStyles[0] = getXPath(
        pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:rect",
        "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:165pt;width:283.4pt;height:141.7pt;rotation:285"
    aStyles[1] = getXPath(
        pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect",
        "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:255"
    aStyles[2] = getXPath(
        pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Fallback/w:pict/v:rect",
        "style");
    // original is: "position:absolute;margin-left:97.5pt;margin-top:164.9pt;width:283.4pt;height:141.7pt;rotation:105"
    aStyles[3] = getXPath(
        pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Fallback/w:pict/v:rect",
        "style");
    // original is: "position:absolute;margin-left:97.55pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:75"

    //check the size and position of each of the shapes
    for (int i = 0; i < 4; ++i)
    {
        CPPUNIT_ASSERT(!aStyles[i].isEmpty());

        sal_Int32 nextTokenPos = 0;
        do
        {
            OUString aStyleCommand = aStyles[i].getToken(0, ';', nextTokenPos);
            CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

            OUString aStyleCommandName = aStyleCommand.getToken(0, ':');
            OUString aStyleCommandValue = aStyleCommand.getToken(1, ':');

            if (aStyleCommandName == "margin-left")
            {
                float fValue = aStyleCommandValue.getToken(0, 'p').toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(97.6, fValue, 0.1);
            }
            else if (aStyleCommandName == "margin-top")
            {
                float fValue = aStyleCommandValue.getToken(0, 'p').toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(165.0, fValue, 0.2);
            }
            else if (aStyleCommandName == "width")
            {
                float fValue = aStyleCommandValue.getToken(0, 'p').toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(283.4, fValue, 0.1);
            }
            else if (aStyleCommandName == "height")
            {
                float fValue = aStyleCommandValue.getToken(0, 'p').toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(141.7, fValue, 0.1);
            }

        } while (nextTokenPos != -1);
    }

    // Check shape objects

    awt::Point aPos[4];
    aPos[0] = getShape(1)->getPosition();
    aPos[1] = getShape(2)->getPosition();
    aPos[2] = getShape(3)->getPosition();
    aPos[3] = getShape(4)->getPosition();

    // certain degree of error is tolerated due to rounding in unit conversions
    CPPUNIT_ASSERT(abs(4734 - aPos[0].X) < 10);
    CPPUNIT_ASSERT(abs(4734 - aPos[1].X) < 10);
    CPPUNIT_ASSERT(abs(4731 - aPos[2].X) < 10);
    CPPUNIT_ASSERT(abs(4733 - aPos[3].X) < 10);

    CPPUNIT_ASSERT(abs(2845 - aPos[0].Y) < 10);
    CPPUNIT_ASSERT(abs(2843 - aPos[1].Y) < 10);
    CPPUNIT_ASSERT(abs(2842 - aPos[2].Y) < 10);
    CPPUNIT_ASSERT(abs(2843 - aPos[3].Y) < 10);
}

DECLARE_OOXMLEXPORT_TEST(testFdo73215, "fdo73215.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // 'rect' was 'pictureFrame', which isn't valid.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:prstGeom",
                "prst", "rect");
    // 'adj1' was 'adj', which is not valid for bentConnector3.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[9]/wps:spPr/a:prstGeom/a:avLst/a:gd",
                "name", "adj1");
}

DECLARE_OOXMLEXPORT_TEST(testBehinddoc, "behinddoc.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This was "0", shape was in the foreground.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor",
                "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(testFdo65833, "fdo65833.docx")
{
    // The "editas" attribute for vml group shape was not preserved.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:group",
                "editas", "canvas");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73247, "fdo73247.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
                "rot", "1969200");
}

DECLARE_OOXMLEXPORT_TEST(testFdo70942, "fdo70942.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "ellipse");
}

DECLARE_OOXMLEXPORT_TEST(testDrawinglayerPicPos, "drawinglayer-pic-pos.docx")
{
    // The problem was that the position of the picture was incorrect, it was shifted towards the bottom right corner.
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    OString aXPath("/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:xfrm/a:off");
    // This was 720.
    assertXPath(pXmlDocument, aXPath, "x", "0");
    // This was 1828800.
    assertXPath(pXmlDocument, aXPath, "y", "0");
}

DECLARE_OOXMLEXPORT_TEST(testShapeThemePreservation, "shape-theme-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // check shape style has been preserved
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
                "idx", "1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
                "val", "accent1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
                "idx", "1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
                "val", "accent1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
                "idx", "1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
                "val", "accent1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef",
                "idx", "2");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr",
                "val", "accent1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr/a:shade",
                "val", "50000");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef",
                "idx", "0");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef/a:schemeClr",
                "val", "accent1");

    // check shape style hasn't been overwritten
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill",
                0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill",
                0);

    // check direct theme assignments have been preserved
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr",
                "val", "accent6");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr",
                "val", "accent3");

    // check color transformations applied to theme colors have been preserved
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumMod",
                "val", "40000");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumOff",
                "val", "60000");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr/a:lumMod",
                "val", "50000");

    // check direct color assignments have been preserved
    OUString sFillColor
        = getXPath(pXmlDocument,
                   "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:srgbClr",
                   "val");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00b050), sFillColor.toInt32(16));
    sal_Int32 nLineColor
        = getXPath(pXmlDocument,
                   "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:srgbClr",
                   "val")
              .toInt32(16);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), nLineColor);

    // check direct line type assignments have been preserved
    sal_Int32 nLineWidth
        = getXPath(pXmlDocument,
                   "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln",
                   "w")
              .toInt32();
    CPPUNIT_ASSERT(
        abs(63500 - nLineWidth)
        < 1000); //some rounding errors in the conversion ooxml -> libo -> ooxml are tolerated
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:miter",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:custDash",
                1);

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);
    uno::Reference<drawing::XShape> xShape3 = getShape(3);

    // check colors are properly applied to shapes on import
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xShape1, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xfcd5b5), getProperty<sal_Int32>(xShape2, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00b050), getProperty<sal_Int32>(xShape3, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x3a5f8b), getProperty<sal_Int32>(xShape1, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f6228), getProperty<sal_Int32>(xShape2, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(xShape3, "LineColor"));

    // check line properties are properly applied to shapes on import
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID,
                         getProperty<drawing::LineStyle>(xShape1, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID,
                         getProperty<drawing::LineStyle>(xShape2, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH,
                         getProperty<drawing::LineStyle>(xShape3, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND,
                         getProperty<drawing::LineJoint>(xShape1, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND,
                         getProperty<drawing::LineJoint>(xShape2, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_MITER,
                         getProperty<drawing::LineJoint>(xShape3, "LineJoint"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69616, "fdo69616.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    // VML
    CPPUNIT_ASSERT(getXPath(pXmlDoc,
                            "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[1]/"
                            "mc:AlternateContent/mc:Fallback/w:pict/v:group",
                            "coordorigin")
                       .match("696,725"));
}

DECLARE_OOXMLEXPORT_TEST(testAlignForShape, "Shape.docx")
{
    //fdo73545:Shape Horizontal and vertical orientation is wrong
    //The wp:align tag is missing after roundtrip
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionH/wp:align",
                "1");
}

DECLARE_OOXMLEXPORT_TEST(testLineStyle_DashType, "LineStyle_DashType.docx")
{
    /* DOCX contatining Shape with LineStyle as Dash Type should get preserved inside
    * an XMl tag <a:prstDash> with value "dash".
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash",
                "val", "dash");
}

DECLARE_OOXMLEXPORT_TEST(testGradientFillPreservation, "gradient-fill-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // check rgb colors for every step in the gradient of the first shape
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr",
                "val", "ffff00");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr",
                "val", "ffff33");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[3]/a:srgbClr",
                "val", "ff0000");

    // check theme colors for every step in the gradient of the second shape
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='0']/"
                "a:schemeClr",
                "val", "accent5");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/"
                "a:schemeClr",
                "val", "accent1");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='100000']/"
                "a:schemeClr",
                "val", "accent1");

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/"
                "a:srgbClr/a:alpha",
                "val", "20000");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/"
                "a:schemeClr/a:tint",
                "val", "44500");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/"
                "a:schemeClr/a:satMod",
                "val", "160000");
}

DECLARE_OOXMLEXPORT_TEST(testLineStyle_DashType_VML, "LineStyle_DashType_VML.docx")
{
    /* DOCX contatining "Shape with text inside" having Line Style as "Dash Type" should get
    * preserved inside an XML tag <v:stroke> with attribute dashstyle having value "dash".
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:stroke",
        "dashstyle", "dash");
}

DECLARE_OOXMLEXPORT_TEST(testFdo74110, "fdo74110.docx")
{
    /*
    The File contains word art which is being exported as shape and the mapping is defaulted to
    shape type rect since the actual shape type(s) is/are commented out for some reason.
    The actual shape type(s) has/have adjustment value(s) where as rect does not have adjustment value.
    Hence the following test case.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/"
                "wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]",
                "prst", "rect");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/"
                "wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/"
                "a:avLst[1]/a:gd[1]",
                0);
}

DECLARE_OOXMLEXPORT_TEST(testChildNodesOfCubicBezierTo, "FDO74774.docx")
{
    /* Number of children required by cubicBexTo is 3 of type "pt".
    While exporting, sometimes the child nodes are less than 3.
    The test case ensures that there are 3 child nodes of type "pt"
    for cubicBexTo
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice/"
                         "w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/"
                         "wps:wsp[3]/wps:spPr[1]/a:custGeom[1]/a:pathLst[1]/a:path[1]/"
                         "a:cubicBezTo[2]/a:pt[3]");
}

DECLARE_OOXMLEXPORT_TEST(testMSwordHang, "test_msword_hang.docx")
{
    // fdo#74771:
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r[2]/"
                "w:drawing/wp:inline",
                "distT", "0");
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeThemeFont, "groupshape-theme-font.docx")
{
    // Font was specified using a theme reference, which wasn't handled.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText), 1);
    // This was Calibri.
    CPPUNIT_ASSERT_EQUAL(OUString("Cambria"), getProperty<OUString>(xRun, "CharFontName"));
}

DECLARE_OOXMLEXPORT_TEST(testAnchorIdForWP14AndW14, "AnchorId.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:inline",
        "anchorId", "78735EFD");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect",
                "anchorId", "78735EFD");
}

DECLARE_OOXMLEXPORT_TEST(testDkVert, "dkvert.docx")
{
    // <a:pattFill prst="dkVert"> was exported as ltVert.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 50.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<drawing::Hatch>(xShape, "FillHatch").Distance);
}

DECLARE_OOXMLEXPORT_TEST(testTextWatermark, "textWatermark.docx")
{
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'PowerPlusWaterMarkObject' string in it
    xmlDocPtr pXmlHeader1 = parseExport("word/header1.xml");
    if (!pXmlHeader1)
        return;

    assertXPath(pXmlHeader1, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]", "id",
                "PowerPlusWaterMarkObject93701316");

    //The second problem was that Word uses also "o:spid"
    const OUString& sSpid
        = getXPath(pXmlHeader1, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]", "spid");
    CPPUNIT_ASSERT(!sSpid.isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testPictureWatermark, "pictureWatermark.docx")
{
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'WordPictureWaterMarkObject' string in it

    xmlDocPtr pXmlHeader1 = parseExport("word/header1.xml");
    if (!pXmlHeader1)
        return;

    // Check the watermark ID
    assertXPath(
        pXmlHeader1,
        "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:shape[1]", "id",
        "WordPictureWatermark11962361");
}

DECLARE_OOXMLEXPORT_TEST(testFdo76249, "fdo76249.docx")
{
    /*
    * The Locked Canvas is imported correctly, but while exporting
    * the drawing element is exported inside a textbox. However a the drawing has to exported
    * as a Locked Canvas inside a text-box for the RT file to work in MS Word, as drawing elements
    * are not allowed inside the textboxes.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/"
                "w:pict[1]/v:rect[1]/v:textbox[1]/w:txbxContent[1]/w:p[1]/w:r[1]/"
                "mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/"
                "a:graphicData[1]/lc:lockedCanvas[1]",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testShapeEffectPreservation, "shape-effect-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // first shape with outer shadow, rgb color
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "algn", "tl");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "blurRad", "50800");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "dir", "2700000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "dist", "37674");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "rotWithShape", "0");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr",
        "val", "000000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/"
                "a:srgbClr/a:alpha",
                "val", "40000");

    // second shape with outer shadow, scheme color
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "algn", "tl");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "blurRad", "114300");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "dir", "2700000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "dist", "203137");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "rotWithShape", "0");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr",
        "val", "accent1");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/"
                "a:schemeClr/a:lumMod",
                "val", "40000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/"
                "a:schemeClr/a:lumOff",
                "val", "60000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/"
                "a:schemeClr/a:alpha",
                "val", "40000");

    // third shape with inner shadow, rgb color
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
                "blurRad", "63500");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
                "dir", "16200000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
                "dist", "50800");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
        "val", "ffff00");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/"
                "a:srgbClr/a:alpha",
                "val", "50000");

    // 4th shape with soft edge
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge",
                "rad", "127000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge/*",
                0); // should not be present

    // 5th shape with glow effect, scheme color
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow",
                "rad", "101600");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr",
                "val", "accent2");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/"
                "a:satMod",
                "val", "175000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/a:alpha",
        "val", "40000");

    // 6th shape with reflection
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "blurRad", "6350");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "stA", "50000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "endA", "300");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "endPos", "55500");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection/*",
                0); // should not be present

    // 7th shape with several effects: glow, inner shadow and reflection
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow",
                "rad", "63500");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr",
                "val", "accent2");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/*",
        2);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
                "blurRad", "63500");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
                "dir", "2700000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
        "val", "000000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/"
                "a:srgbClr/a:alpha",
                "val", "50000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "blurRad", "6350");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
                "stA", "52000");
}

DECLARE_OOXMLEXPORT_TEST(testShape3DEffectPreservation, "shape-3d-effect-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // first shape: extrusion and shift on z, rotated camera with zoom, rotated light rig
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
                "prst", "perspectiveRelaxedModerately");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
                "zoom", "150000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
                "lat", "19490639");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
                "lon", "0");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
                "rev", "12900001");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
                "rig", "threePt");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
                "dir", "t");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
                "lat", "0");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
                "lon", "0");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
                "rev", "4800000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "extrusionH", "63500");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "z", "488950");

    // second shape: extrusion with theme color, no camera or light rotation, metal material
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
                "prst", "isometricLeftDown");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
                0);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
                "rig", "threePt");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
                "dir", "t");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
                0);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "extrusionH", "25400");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "prstMaterial", "metal");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr",
        "val", "accent5");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/"
                "a:schemeClr/a:lumMod",
                "val", "40000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/"
                "a:schemeClr/a:lumOff",
                "val", "60000");

    // third shape: colored contour and top and bottom bevel, plastic material
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "contourW", "50800");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "prstMaterial", "plastic");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
                "w", "139700");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
                "h", "88900");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
                "prst", "cross");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
                "h", "88900");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
                "prst", "relaxedInset");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:contourClr/a:srgbClr",
                "val", "3333ff");

    // fourth shape: wireframe
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
                "prstMaterial", "legacyWireframe");
}

DECLARE_OOXMLEXPORT_TEST(testPictureEffectPreservation, "picture-effects-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // first picture: glow effect with theme color and transformations, 3d rotation and extrusion
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow",
                "rad", "228600");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:schemeClr",
                "val", "accent1");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:schemeClr/*",
        2);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:camera",
                "prst", "isometricRightUp");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:lightRig",
                "rig", "threePt");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d",
                "extrusionH", "76200");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d/a:extrusionClr/a:srgbClr",
        "val", "92d050");

    // second picture: shadow and reflection effects
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw",
                "dir", "8076614");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr",
        "val", "000000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/"
                "a:srgbClr/a:alpha",
                "val", "40000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection",
                "dir", "5400000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection/*",
                0); // should not be present

    // third picture: soft edge effect
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:softEdge",
                "rad", "63500");
}

DECLARE_OOXMLEXPORT_TEST(testPictureArtisticEffectPreservation,
                         "picture-artistic-effects-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    xmlDocPtr pRelsDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc || !pRelsDoc)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // 1st picture: marker effect
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticMarker",
        "trans", "14000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticMarker",
        "size", "80");

    OUString sEmbedId1
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    OUString sFile
        = getXPath(pRelsDoc, OUStringToOString(sXmlPath, RTL_TEXTENCODING_UTF8), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 2nd picture: pencil grayscale
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticPencilGrayscale",
        "trans", "15000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticPencilGrayscale",
        "pencilSize", "66");

    OUString sEmbedId2
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId2);

    // 3rd picture: pencil sketch
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticPencilSketch",
        "trans", "7000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticPencilSketch",
        "pressure", "17");

    OUString sEmbedId3
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId3);

    // 4th picture: light screen
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticLightScreen",
        "trans", "13000");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticLightScreen",
        "gridSize", "1");

    OUString sEmbedId4
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId4 + "']";
    sFile = getXPath(pRelsDoc, OUStringToOString(sXmlPath, RTL_TEXTENCODING_UTF8), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 5th picture: watercolor sponge
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticWatercolorSponge",
        "brushSize", "4");

    OUString sEmbedId5
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId5);

    // 6th picture: photocopy (no attributes)
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
        "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/"
        "a14:imgEffect/"
        "a14:artisticPhotocopy",
        1);

    OUString sEmbedId6
        = getXPath(pXmlDoc,
                   "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                   "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/"
                   "a14:imgProps/a14:imgLayer",
                   "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId6);

    // no redundant wdp files saved
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/hdphoto3.wdp")));
}

DECLARE_OOXMLEXPORT_TEST(fdo77719, "fdo77719.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/"
                "a:custGeom[1]",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testNestedAlternateContent, "nestedAlternateContent.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // We check alternateContent  could not contains alternateContent (i.e. nested alternateContent)
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[2]/"
                "wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[2]/mc:AlternateContent[1]",
                0);
}

#if 0
// Currently LibreOffice exports custom geometry for this hexagon, not preset shape.
// When LibreOffice can export preset shapes with correct modifiers, then this test can be re-enabled.

DECLARE_OOXMLEXPORT_TEST(test76317, "test76317.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc) return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]", "prst", "hexagon");
}

#endif

DECLARE_OOXMLEXPORT_TEST(test76317_2K10, "test76317_2K10.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/"
                "a:prstGeom[1]/a:avLst[1]/a:gd[1]",
                "name", "adj");
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_backgroundShape, "tdf77219_backgroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", false,
                                 getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_foregroundShape, "tdf77219_foregroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", true,
                                 getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testPresetShape, "preset-shape.docx")
{
    // Document contains a flowChartMultidocument preset shape, our date for that shape wasn't correct.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xPropertySet->getPropertyValue("CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"]);
    uno::Sequence<awt::Size> aSubViewSize((aPath["SubViewSize"]).get<uno::Sequence<awt::Size>>());

    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aSubViewSize[0].Height);
}

DECLARE_OOXMLEXPORT_TEST(testSdtAndShapeOverlapping, "ShapeOverlappingWithSdt.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt[1]/w:sdtContent[1]/w:r[1]/w:t[1]");
}

DECLARE_OOXMLEXPORT_TEST(testLockedCanvas, "fdo78658.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for lockedCanvas tag
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/lc:lockedCanvas",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testAbsolutePositionOffsetValue, "fdo78432.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    xmlXPathObjectPtr pXmlObjs[6];
    pXmlObjs[0] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[1] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[2] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[3] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[4] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[5] = getXPathNode(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/"
                                        "mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/"
                                        "wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    for (sal_Int32 index = 0; index < 6; ++index)
    {
        CPPUNIT_ASSERT(pXmlObjs[index]->nodesetval != nullptr);
        xmlNodePtr pXmlNode = pXmlObjs[index]->nodesetval->nodeTab[0];
        OUString contents = OUString::createFromAscii(
            reinterpret_cast<const char*>((pXmlNode->children[0]).content));
        CPPUNIT_ASSERT(contents.toInt64() <= SAL_MAX_INT32);
        xmlXPathFreeObject(pXmlObjs[index]);
    }
}

DECLARE_OOXMLEXPORT_TEST(testfdo78663, "fdo78663.docx")
{
    /*     * A 2007 word art tool is enclosed in a 2010 drawing toolWithin a file,
    * Originally the file has the following xml tag hierarchy.
    *
    * <p> <r> <ac> <drawing> <txbx> <txbxContent> <pict><shapetype> <shape> ...</shape></shapetype> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
    *  After RT :
    * <p> <r> <ac> <drawing> <txbx> <txbxContent> <pict><shapetype> <shape> <textbox><txbxContent> ... </txbxContent></textbox></shape></shapetype> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
    * MSO doesn't allow nesting of txbxContent tags.
    * As the text of the wordart tool is written in the tag <v:textpath string="Welcome to... "History is fun and informative"/>
    * We shouldn't repeat it again in <shapetype><shape> <textbox><txbxContent>
    * */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict/v:shape/"
                "v:path",
                1);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/"
                "v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path",
                1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo79256, "fdo79256.docx")
{
    /* Corruption issue containing Line Style with Long Dashes and Dots
    * After RT checking the Dash Length value. Dash Length value should not be greater than 2147483.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    const sal_Int32 maxLimit = 2147483;
    sal_Int32 d = getXPath(pXmlDoc,
                           "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                           "w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/"
                           "wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]",
                           "d")
                      .toInt32();
    CPPUNIT_ASSERT(d <= maxLimit);
}

DECLARE_OOXMLEXPORT_TEST(testDashedLinePreset, "dashed_line_preset.docx")
{
    /* Make sure that preset line is exported correctly as "1000th of a percent".
    * This test-file has a PRESET dash-line which will be converted by LO import
    * to a custom-dash (dash-dot-dot). This test-case makes sure that the exporter
    * outputs the correct values.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "d", "800000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "sp", "300000");
}

DECLARE_OOXMLEXPORT_TEST(testDashedLine_CustDash1000thOfPercent,
                         "dashed_line_custdash_1000th_of_percent.docx")
{
    /* Make sure that preset line is exported correctly as "1000th of a percent".
    * This test-file has a CUSTOM dash-line that is defined as '1000th of a percent'.
    * This should be imported by LO as-is, and exported back with the same values.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "d", "800000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "sp", "300000");
}

DECLARE_OOXMLEXPORT_TEST(testDashedLine_CustDashPercentage, "dashed_line_custdash_percentage.docx")
{
    /* Make sure that preset line is exported correctly as "1000th of a percent".
    * This test-file has a CUSTOM dash-line that is defined as percentages.
    * This should be imported by LO as '1000th of a percent', and exported back
    * as '1000th of a percent'.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "d", "800000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[1]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[2]",
                "sp", "300000");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "d", "100000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                "wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/"
                "a:custDash[1]/a:ds[3]",
                "sp", "300000");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79591, "fdo79591.docx")
{
    /* Values set for docPr name and shape ID attributes
    * in RT file were not valid as per UTF-8 encoding format
    * and hence was showing RT document as corrupt with error
    * message "invalid character"
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr",
        "name", "");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape", "ID",
                "");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80895, "fdo80895.docx")
{
    // DML shapes in header and footer were not getting rendered in LO and the same were not preserved after RT.
    // In actual there was a shape but because of fetching wrong theme for header.xml or footer.xml
    // resultant shape was with <a:noFill/> prop in <wps:spPr> hence was not visible.
    // Checking there is a shape in header without <a:noFill/> element.

    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:noFill",
                0);
    assertXPath(pXmlDoc,
                "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:ln/a:noFill",
                0);

    // Check for fallback (required for MSO-2007)
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect",
                "fillcolor", "#4f81bd");
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:fill",
                "type", "solid");
}

DECLARE_OOXMLEXPORT_TEST(testWrapTightThrough, "wrap-tight-through.docx")
{
    // These were wrapSquare without a wrap polygon before.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // The first shape should be wrapThrough with a wrap polygon (was wrapSquare).
        assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapThrough/wp:wrapPolygon/wp:start", "x",
                    "-1104");
        // The second shape should be wrapTight with a wrap polygon (was wrapSquare).
        assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapTight/wp:wrapPolygon/wp:start", "y",
                    "792");
    }
}

DECLARE_OOXMLEXPORT_TEST(testExportShadow, "bnc637947.odt")
{
    // The problem was that shadows of shapes from non-OOXML origin were not exported to DrawingML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw",
                "dist", "109865");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw",
                "dir", "634411");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/"
                "a:srgbClr",
                "val", "000000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/"
                "a:srgbClr/a:alpha",
                "val", "38000");
}

DECLARE_OOXMLEXPORT_TEST(testExportAdjustmentValue, "tdf91429.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd",
                "fmla", "val 50000");
}

DECLARE_OOXMLEXPORT_TEST(testFlipAndRotateCustomShape, "flip_and_rotate.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // there should be no flipH and flipV attributes in this case
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "flipH");
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "flipV");
    // check rotation angle
    assertXPath(pXmlDoc, "//a:xfrm", "rot", "13500000");
    // check the first few coordinates of the polygon
#ifndef MACOSX /* Retina-related rounding roundtrip error
    * hard to smooth out due to the use of string compare
    * instead of number */
#if !defined(_WIN32)
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "x", "2351");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "y", "3171");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt", "x", "1695");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt", "y", "3171");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[3]/a:pt", "x", "1695");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[3]/a:pt", "y", "1701");
#endif
#endif
}

DECLARE_OOXMLEXPORT_TEST(testN751054, "n751054.docx")
{
    text::TextContentAnchorType eValue
        = getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType");
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
}

DECLARE_OOXMLEXPORT_TEST(testN751117, "n751117.docx")
{
    // First shape: the end should be an arrow, should be rotated and should be flipped.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("LineEndName") >>= aValue;
    CPPUNIT_ASSERT(aValue.indexOf("Arrow") != -1);

    // Rotating & Flipping will cause the angle to change from 90 degrees to 270 degrees
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(270 * 100), nValue);

    uno::Reference<drawing::XShape> xShape(xPropertySet, uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());
    CPPUNIT_ASSERT(aActualSize.Width > 0);

    // The second shape should be a line
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.LineShape"));
}

DECLARE_OOXMLEXPORT_TEST(testN705956_1, "n705956-1.docx")
{
    /*
    Get the first image in the document and check it's the one image in the document.
    It should be also anchored inline (as character) and be inside a groupshape.
    image = ThisComponent.DrawPage.getByIndex(0)
    graphic = image(0).Graphic
    xray graphic.Size
    xray image.AnchorType
    */
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), drawPage->getCount());
    uno::Reference<drawing::XShapes> shapes(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> image;
    shapes->getByIndex(0) >>= image;
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue("Graphic") >>= graphic;
    uno::Reference<awt::XBitmap> bitmap(graphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(120), bitmap->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(106), bitmap->getSize().Height);
    text::TextContentAnchorType anchorType;
    imageProperties->getPropertyValue("AnchorType") >>= anchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, anchorType);
}

DECLARE_OOXMLEXPORT_TEST(testN705956_2, "n705956-2.docx")
{
    /*
    <v:shapetype> must be global, reachable even from <v:shape> inside another <w:pict>
    image = ThisComponent.DrawPage.getByIndex(0)
    xray image.FillColor
    */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    sal_Int32 fillColor;
    imageProperties->getPropertyValue("FillColor") >>= fillColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xc0504d), fillColor);
}

DECLARE_OOXMLEXPORT_TEST(testN779834, "n779834.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testTDF91260, "tdf91260.docx")
{
    /*
    * textbox can't extend beyond the page bottom
    * solution: shrinking textbox (its text frame) height, if needed
    */
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame->getString().startsWith("Lorem ipsum"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3454), getProperty<sal_Int32>(xFrame, "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testShadow, "imgshadow.docx")
{
    /*
    * The problem was that drop shadows on inline images were not being
    * imported and applied.
    */
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(2), uno::UNO_QUERY);

    bool bShadow = getProperty<bool>(xPropertySet, "Shadow");
    CPPUNIT_ASSERT(bShadow);

    sal_Int32 nShadowXDistance = getProperty<sal_Int32>(xPropertySet, "ShadowXDistance");
    CPPUNIT_ASSERT(nShadowXDistance != 0);
}

DECLARE_OOXMLEXPORT_TEST(testN792778, "n792778.docx")
{
    /*
    * The problem was that the importer didn't handle complex groupshapes with groupshapes, textboxes and graphics inside.
    *
    * xray ThisComponent.DrawPage.Count ' 1 groupshape
    * xray ThisComponent.DrawPage(0).Count ' 2 sub-groupshapes
    * xray ThisComponent.DrawPage(0).getByIndex(0).Count ' first sub-groupshape: 1 pic
    * xray ThisComponent.DrawPage(0).getByIndex(1).Count ' second sub-groupshape: 1 pic
    * xray ThisComponent.DrawPage(0).getByIndex(0).getByIndex(0).Position.Y ' 11684, the vertical position of the shapes were also wrong
    * xray ThisComponent.DrawPage(0).getByIndex(1).getByIndex(0).Position.Y ' 11684
    */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                      uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    uno::Reference<drawing::XShapes> xGroupShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupShape->getCount());

    uno::Reference<drawing::XShapes> xInnerGroupShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xInnerGroupShape->getCount());

    uno::Reference<drawing::XShape> xInnerShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11684), xInnerShape->getPosition().Y);

    xInnerGroupShape.set(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xInnerGroupShape->getCount());

    xInnerShape.set(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11684), xInnerShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeSmarttag, "groupshape-smarttag.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    // First run of shape text was missing due to the w:smartTag wrapper around it.
    CPPUNIT_ASSERT_EQUAL(OUString("Box 2"), xShape->getString());

    // Font size of the shape text was 10.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xShape->getText(), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testN592908_Picture, "n592908-picture.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, eValue);
}

DECLARE_OOXMLEXPORT_TEST(testFdo46361, "fdo46361.docx")
{
    uno::Reference<container::XIndexAccess> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    // This was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP,
                         getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    // This was LEFT.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    // This was black, not green.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x008000),
                         getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
    // \n char was missing due to unhandled w:br.
    CPPUNIT_ASSERT_EQUAL(
        OUString("text\ntext"),
        uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY)->getString());
    // \n chars were missing, due to unhandled multiple w:p tags.
    CPPUNIT_ASSERT_EQUAL(
        OUString("text\ntext\n"),
        uno::Reference<text::XTextRange>(xGroupShape->getByIndex(2), uno::UNO_QUERY)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeRotation, "groupshape-rotation.docx")
{
    // Rotation on groupshapes wasn't handled at all by the VML importer.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc780044Spacing, "bnc780044_spacing.docx")
{
    // The document has global w:spacing in styles.xml , and local w:spacing in w:pPr, which however
    // only applied to text runs, not to as-character pictures. So the picture made the line higher.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testVmlTextVerticalAdjust, "vml-text-vertical-adjust.docx")
{
    uno::Reference<drawing::XShapes> xOuterGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xInnerGroupShape(xOuterGroupShape->getByIndex(0),
                                                      uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    // Was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP,
                         getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103389, "tdf103389.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // No geometry was exported for the second canvas
    // Check both canvases' geometry
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "rect");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "rect");
}

DECLARE_OOXMLEXPORT_TEST(testTdf84678, "tdf84678.docx")
{
    // This was 0, left margin inside a shape+text wasn't imported from DOCX.
    // 360000 EMU, but layout uses twips.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(567),
        parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds", "left").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103573, "tdf103573.docx")
{
    // Relative positions to the left or right margin (MS Word naming) was not handled.
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER,
                                 nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to left page border",
                                 text::RelOrientation::PAGE_LEFT, nValue);

    xShapeProperties.set(getShape(2), uno::UNO_QUERY);
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER,
                                 nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to right page border",
                                 text::RelOrientation::PAGE_RIGHT, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107104, "tdf107104.docx")
{
    CPPUNIT_ASSERT(getShape(1)->getSize().Width > 0);
    // This failed: the second arrow was invisible because it had zero width.
    CPPUNIT_ASSERT(getShape(2)->getSize().Width > 0);
}

DECLARE_OOXMLEXPORT_TEST(testWatermarkShape, "watermark-shapetype.docx")
{
    uno::Reference<drawing::XShape> xShape1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    uno::Reference<beans::XPropertySet> xPropertySet2(xShape2, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xPropertySet1->getPropertyValue("TextAutoGrowHeight"),
                         xPropertySet2->getPropertyValue("TextAutoGrowHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4),
                         xGroup->getCount()); // 3 rectangles and an arrow in the group

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nValue(0);
    xPropertySet->getPropertyValue("FillColor") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), nValue); // If fill color is right, theme import is OK

    uno::Reference<text::XTextRange> xTextRange(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Sample"), xTextRange->getString()); // Shape has text

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextRange->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("ParaAdjust") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_CENTER),
                         nValue); // Paragraph properties are imported
}

DECLARE_OOXMLEXPORT_TEST(testWpsOnly, "wps-only.docx")
{
    // Document has wp:anchor, not wp:inline, so handle it accordingly.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    text::TextContentAnchorType eValue
        = getProperty<text::TextContentAnchorType>(xShape, "AnchorType");
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_PARAGRAPH, eValue);

    // Check position, it was 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(671830), xShape->getPosition().X);

    // Left margin should be 0, as margins are not relevant for <wp:wrapNone/>.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "LeftMargin"));
    // Wrap type was PARALLEL.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(xShape, "Surround"));
    // Confirm that the deprecated (incorrectly spelled) _THROUGHT also matches
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT,
                         getProperty<text::WrapTextMode>(xShape, "Surround"));

    // This should be in front of text.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "Opaque"));
    // And this should be behind the document.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testWpgOnly, "wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was nearly 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(548005), xShape->getPosition().X);
}

DECLARE_OOXMLEXPORT_TEST(testWpgNested, "wpg-nested.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0),
                                                               uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to lack of handling of groupshapes inside groupshapes.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"),
                         xShapeDescriptor->getShapeType());

    // This failed, the right edge of the shape was outside the page
    // boundaries.
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "/root/page[2]/infos/bounds", "left").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "/root/page[2]/infos/bounds", "width").toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/SwAnchoredDrawObject/bounds", "left")
              .toInt32();
    sal_Int32 nShapeWidth
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/SwAnchoredDrawObject/bounds", "width")
              .toInt32();
    // Make sure the shape is within the page bounds.
    CPPUNIT_ASSERT_GREATEREQUAL(nShapeLeft + nShapeWidth, nPageLeft + nPageWidth);
}

DECLARE_OOXMLEXPORT_TEST(textboxWpgOnly, "textbox-wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // The relativeFrom attribute was ignored for groupshapes, i.e. these were text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    // Make sure the shape is not in the background, as we have behindDoc="0" in the doc.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "Opaque"));

    // The 3 paragraphs on the rectangles inside the groupshape ended up in the
    // body text, make sure we don't have multiple paragraphs there anymore.
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs()); // was 4

    // Character escapement was enabled by default, this was 58.
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(100),
        getProperty<sal_Int32>(getRun(getParagraphOfText(1, xText), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testMceWpg, "mce-wpg.docx")
{
    // Make sure that we read the primary branch, if wpg is requested as a feature.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    // This was VML1.
    getParagraphOfText(1, xText, "DML1");
}

DECLARE_OOXMLEXPORT_TEST(testMceNested, "mce-nested.docx")
{
    // Vertical position of the shape was incorrect due to incorrect nested mce handling.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // positionV's posOffset from the bugdoc, was 0.
    CPPUNIT_ASSERT(6985 <= getProperty<sal_Int32>(xShape, "VertOrientPosition"));
    // This was -1 (default), make sure the background color is set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xShape, "FillColor"));

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(getShape(2), uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to incorrect handling of wpg elements after a wps textbox.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"),
                         xShapeDescriptor->getShapeType());

    // Now check the top right textbox.
    uno::Reference<container::XIndexAccess> xGroup(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "[Year]");
    CPPUNIT_ASSERT_EQUAL(48.f, getProperty<float>(getRun(xParagraph, 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff),
                         getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getRun(xParagraph, 1), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        drawing::TextVerticalAdjust_BOTTOM,
        getProperty<drawing::TextVerticalAdjust>(xGroup->getByIndex(1), "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo70457, "fdo70457.docx")
{
    // The document contains a rotated bitmap
    // It must be imported as a XShape object with the proper rotation value

    // Check: there is one shape in the doc
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());

    // Check: the angle of the shape is 45º
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4500), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupshapeSdt, "dml-groupshape-sdt.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    // The text in the groupshape was missing due to the w:sdt and w:sdtContent wrapper around it.
    CPPUNIT_ASSERT_EQUAL(
        OUString("sdt and sdtContent inside groupshape"),
        uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testDmlCharheightDefault, "dml-charheight-default.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 16: the first run of the second para incorrectly inherited the char height of the first para.
    CPPUNIT_ASSERT_EQUAL(
        11.f,
        getProperty<float>(getRun(getParagraphOfText(2, xShape->getText()), 1), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeCapitalization, "dml-groupshape-capitalization.docx")
{
    // Capitalization inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    // 2nd line is written with uppercase letters
    CPPUNIT_ASSERT_EQUAL(
        style::CaseMap::UPPERCASE,
        getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), "CharCaseMap"));
    // 3rd line has no capitalization
    CPPUNIT_ASSERT_EQUAL(
        style::CaseMap::NONE,
        getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), "CharCaseMap"));
    // 4th line has written with small capitals
    CPPUNIT_ASSERT_EQUAL(
        style::CaseMap::SMALLCAPS,
        getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), "CharCaseMap"));
    // 5th line has no capitalization
    CPPUNIT_ASSERT_EQUAL(
        style::CaseMap::NONE,
        getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), "CharCaseMap"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeRunFonts, "dml-groupshape-runfonts.docx")
{
    // Fonts defined by w:rFonts was not imported and so the font specified by a:fontRef was used.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"),
                         getProperty<OUString>(xRun, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("MS Mincho"), getProperty<OUString>(xRun, "CharFontNameAsian"));
}

DECLARE_OOXMLEXPORT_TEST(testSmartartStrict, "strict-smartart.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    // This was 0, SmartArt was visually missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xGroup->getCount()); // 3 ellipses + 3 arrows
}

DECLARE_OOXMLEXPORT_TEST(testFdo74401, "fdo74401.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // The triangle (second child) was a TextShape before, so it was shown as a rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.CustomShape"), xShape->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testInlineGroupshape, "inline-groupshape.docx")
{
    // Inline groupshape was in the background, so it was hidden sometimes by other shapes.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTrackedchanges, "groupshape-trackedchanges.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Shape text was completely missing, ensure inserted text is available.
    CPPUNIT_ASSERT_EQUAL(OUString(" Inserted"), xShape->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo80555, "fdo80555.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Shape was wrongly placed at X=0, Y=0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3318), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(247), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testTdf87924, "tdf87924.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // This was -270, the text rotation angle was set when it should not be rotated.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90153, "tdf90153.docx")
{
    // This was at-para, so the line-level VertOrientRelation was lost, resulting in an incorrect vertical position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89165, "tdf89165.docx")
{
    // This must not hang in layout
}

DECLARE_OOXMLEXPORT_TEST(testTdf95777, "tdf95777.docx")
{
    // This must not fail on open
}

DECLARE_OOXMLEXPORT_TEST(testTdf97371, "tdf97371.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pShape = pPage->GetObj(0);
    SdrObject* pTextBox = pPage->GetObj(1);
    long nDiff = std::abs(pShape->GetSnapRect().Top() - pTextBox->GetSnapRect().Top());
    // The top of the two shapes were 410 and 3951, now it should be 3950 and 3951.
    CPPUNIT_ASSERT(nDiff < 10);
}

DECLARE_OOXMLEXPORT_TEST(testSignatureLineShape, "signature-line-all-props-set.docx")
{
    uno::Reference<drawing::XShape> xSignatureLineShape = getShape(1);
    uno::Reference<beans::XPropertySet> xPropSet(xSignatureLineShape, uno::UNO_QUERY);

    bool bIsSignatureLine;
    xPropSet->getPropertyValue("IsSignatureLine") >>= bIsSignatureLine;
    CPPUNIT_ASSERT_EQUAL(true, bIsSignatureLine);

    bool bShowSignDate;
    xPropSet->getPropertyValue("SignatureLineShowSignDate") >>= bShowSignDate;
    CPPUNIT_ASSERT_EQUAL(true, bShowSignDate);

    bool bCanAddComment;
    xPropSet->getPropertyValue("SignatureLineCanAddComment") >>= bCanAddComment;
    CPPUNIT_ASSERT_EQUAL(true, bCanAddComment);

    OUString aSignatureLineId;
    xPropSet->getPropertyValue("SignatureLineId") >>= aSignatureLineId;
    CPPUNIT_ASSERT_EQUAL(OUString("{0EBE47D5-A1BD-4C9E-A52E-6256E5C345E9}"), aSignatureLineId);

    OUString aSuggestedSignerName;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerName") >>= aSuggestedSignerName;
    CPPUNIT_ASSERT_EQUAL(OUString("John Doe"), aSuggestedSignerName);

    OUString aSuggestedSignerTitle;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerTitle") >>= aSuggestedSignerTitle;
    CPPUNIT_ASSERT_EQUAL(OUString("Farmer"), aSuggestedSignerTitle);

    OUString aSuggestedSignerEmail;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerEmail") >>= aSuggestedSignerEmail;
    CPPUNIT_ASSERT_EQUAL(OUString("john@thefarmers.com"), aSuggestedSignerEmail);

    OUString aSigningInstructions;
    xPropSet->getPropertyValue("SignatureLineSigningInstructions") >>= aSigningInstructions;
    CPPUNIT_ASSERT_EQUAL(OUString("Check the machines!"), aSigningInstructions);
}

DECLARE_OOXMLEXPORT_TEST(testTdf113183, "tdf113183.docx")
{
    // The horizontal positioning of the star shape affected the positioning of
    // the triangle one, so the triangle was outside the page frame.
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "/root/page[1]/infos/bounds", "left").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "/root/page[1]/infos/bounds", "width").toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "left")
              .toInt32();
    sal_Int32 nShapeWidth
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "width")
              .toInt32();
    // Make sure the second triangle shape is within the page bounds (with ~1px tolerance).
    CPPUNIT_ASSERT_GREATEREQUAL(nShapeLeft + nShapeWidth, nPageLeft + nPageWidth + 21);
}

DECLARE_OOXMLEXPORT_TEST(testTdf113258, "tdf113258.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354, "tdf104354.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a text frame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
    // still 494 in the second paragraph
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(xShape->getEnd(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116976, "tdf116976.docx")
{
    // This was 0, relative size of shape after bitmap was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(40),
                         getProperty<sal_Int16>(getShape(1), "RelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116985, "tdf116985.docx")
{
    // Body frame width is 10800, 40% is the requested relative width, with 144
    // spacing to text on the left/right side.  So ideal width would be 4032,
    // was 3431. Allow one pixel tolerance, though.
    sal_Int32 nWidth
        = parseDump("/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT(nWidth > 4000);
}

DECLARE_OOXMLEXPORT_TEST(testWatermarkTrim, "tdf114308.docx")
{
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);

    // Rounding errors
    sal_Int32 nHeight = xShape->getSize().Height;
    sal_Int32 nDifference = 8729 - nHeight;
    std::stringstream ss;
    ss << "Difference: " << nDifference << " TotalHeight: " << nHeight;
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference <= 4);
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference >= -4);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
