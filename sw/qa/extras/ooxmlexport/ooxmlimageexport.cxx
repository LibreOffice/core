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
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <vcl/bitmapaccess.hxx>
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

DECLARE_OOXMLEXPORT_TEST(testfdo81031, "fdo81031.docx")
{
    // vml image was not rendered
    // As there are also numPicBullets in the file,
    // the fragmentPath was not changed hence relationships were not resolved.

    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> xImage(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic
        = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(381), xBitmap->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(148), xBitmap->getSize().Height);
}

DECLARE_OOXMLEXPORT_TEST(testPositionAndRotation, "position-and-rotation.docx")
{
    // The document should look like: "This line is tricky, <image> because only 'This line is tricky,' is on the left."
    // But the image was pushed down, so it did not break the line into two text portions.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Should be 1559, was -5639
    CPPUNIT_ASSERT(xShape->getPosition().X > 1500);
    // Should be 88, was 473
    CPPUNIT_ASSERT(xShape->getPosition().Y < 100);
}

DECLARE_OOXMLEXPORT_TEST(testAnchorPosition, "anchor-position.docx")
{
    // The problem was that the at-char anchored picture was at the end of the
    // paragraph, so there were only two positions: a Text, then a Frame one.
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(getParagraph(1), 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(getParagraph(1), 3), "TextPortionType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65718, "fdo65718.docx")
{
    // The problem was that the exporter always exported values of "0" for an images distance from text.
    // the actual attributes where 'distT', 'distB', 'distL', 'distR'
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0),
                         getProperty<sal_Int32>(xPropertySet, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0),
                         getProperty<sal_Int32>(xPropertySet, "BottomMargin"));

    // 'getProperty' return 318 (instead of 317.5)
    // I think this is because it returns an integer, instead of a float.
    // The actual exporting to DOCX exports the correct value (114300 = 317.5 * 360)
    // The exporting to DOCX uses the 'SvxLRSpacing' that stores the value in TWIPS (180 TWIPS)
    // However, the 'LeftMargin' property is an integer property that holds that value in 'MM100' (should hold 317.5, but it is 318)
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300),
                         getProperty<sal_Int32>(xPropertySet, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300),
                         getProperty<sal_Int32>(xPropertySet, "RightMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testImageCrop, "ImageCrop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue("GraphicCrop") >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2955), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5477), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2856), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2291), aGraphicCropStruct.Bottom);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106974_int32Crop, "tdf106974_int32Crop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue("GraphicCrop") >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_MESSAGE(OString::number(aGraphicCropStruct.Right).getStr(),
                           sal_Int32(40470) < aGraphicCropStruct.Right);
}

DECLARE_OOXMLEXPORT_TEST(testGIFImageCrop, "test_GIF_ImageCrop.docx")
{
// FIXME why does this fail on Mac?
#if !defined(MACOSX)
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue("GraphicCrop") >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1085), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3651), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(953), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1244), aGraphicCropStruct.Bottom);
#endif
}

DECLARE_OOXMLEXPORT_TEST(testPNGImageCrop, "test_PNG_ImageCrop.docx")
{
// FIXME why does this fail on Mac?
#if !defined(MACOSX)
    /* The problem was image cropping information was not getting saved
    * after roundtrip.
    * Check for presence of cropping parameters in exported file.
    */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue("GraphicCrop") >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1058), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1111), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1164), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), aGraphicCropStruct.Bottom);
#endif
}

DECLARE_OOXMLEXPORT_TEST(testTdf41542_imagePadding, "tdf41542_imagePadding.odt")
{
    // borderlessImage - image WITHOUT BORDERS : simulate padding with -crop
    text::GraphicCrop crop = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT(crop.Left != 0 && crop.Right != 0);
    CPPUNIT_ASSERT(crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right);

    // borderedImage - image WITH BORDERS : simulate padding with -crop
    crop = getProperty<text::GraphicCrop>(getShape(3), "GraphicCrop");
    CPPUNIT_ASSERT(crop.Left != 0 && crop.Right != 0);
    CPPUNIT_ASSERT(crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right);
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeTIF, "fdo77476.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@ContentType='image/tiff']",
                "PartName", "/word/media/image1.tif");
}

DECLARE_OOXMLEXPORT_TEST(testImageNoborder, "image-noborder.docx")
{
    // This was 26; we exported border for an image that had no border.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testPictureWrapPolygon, "picture-wrap-polygon.docx")
{
    // The problem was that the wrap polygon was ignored during export.
    drawing::PointSequenceSequence aSeqSeq
        = getProperty<drawing::PointSequenceSequence>(getShape(1), "ContourPolyPolygon");
    // This was 0: the polygon list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeqSeq.getLength());

    drawing::PointSequence aSeq = aSeqSeq[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aSeq.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testPictureColormodeGrayscale, "picture_colormode_grayscale.docx")
{
    // THe problem was that the grayscale was not exported
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/"
                "pic:blipFill/a:blip/a:grayscl",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testPictureColormodeBlackWhite, "picture_colormode_black_white.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/"
                "pic:blipFill/a:blip/a:biLevel",
                "thresh", "50000");
}

DECLARE_OOXMLEXPORT_TEST(testPictureColormodeWatermark, "picture_colormode_watermark.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/"
                "pic:blipFill/a:blip/a:lum",
                "bright", "70000");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/"
                "pic:blipFill/a:blip/a:lum",
                "contrast", "-70000");
}

DECLARE_OOXMLEXPORT_TEST(testN747461, "n747461.docx")
{
    /*
    The document contains 3 images (Red, Black, Green, in this order), with explicit
    w:relativeHeight (300, 0, 225763766). Check that they are in the right ZOrder
    after they are loaded.
    */
    uno::Reference<drawing::XShape> image1 = getShape(1), image2 = getShape(2),
                                    image3 = getShape(3);
    sal_Int32 zOrder1, zOrder2, zOrder3;
    OUString descr1, descr2, descr3;
    uno::Reference<beans::XPropertySet> imageProperties1(image1, uno::UNO_QUERY);
    imageProperties1->getPropertyValue("ZOrder") >>= zOrder1;
    imageProperties1->getPropertyValue("Description") >>= descr1;
    uno::Reference<beans::XPropertySet> imageProperties2(image2, uno::UNO_QUERY);
    imageProperties2->getPropertyValue("ZOrder") >>= zOrder2;
    imageProperties2->getPropertyValue("Description") >>= descr2;
    uno::Reference<beans::XPropertySet> imageProperties3(image3, uno::UNO_QUERY);
    imageProperties3->getPropertyValue("ZOrder") >>= zOrder3;
    imageProperties3->getPropertyValue("Description") >>= descr3;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), zOrder1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), zOrder2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), zOrder3);
    CPPUNIT_ASSERT_EQUAL(OUString("Black"), descr1);
    CPPUNIT_ASSERT_EQUAL(OUString("Red"), descr2);
    CPPUNIT_ASSERT_EQUAL(OUString("Green"), descr3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf59699, "tdf59699.docx")
{
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    // This was false: the referenced graphic data wasn't imported.
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_OOXMLEXPORT_TEST(testN783638, "n783638.docx")
{
    // The problem was that the margins of inline images were not zero.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "LeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo63685, "fdo63685.docx")
{
    // An inline image's wrapping should be always zero, even if the doc model has a non-zero value.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShape(1), "TopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103544, "tdf103544.docx")
{
    // We have two shapes: a frame and an image
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());

    // Image was lost because of the frame export
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_OOXMLEXPORT_TEST(testTdf106132, "tdf106132.docx")
{
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // This was 250, <wps:bodyPr ... rIns="0" ...> was ignored for an outer shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape, "TextRightDistance"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104115, "tdf104115.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This found 0 nodes: the custom geometry was not written for the Bezier
    // curve -> Word refused to open the document.
    assertXPath(pXmlDoc, "//a:custGeom", 1);
}

DECLARE_OOXMLEXPORT_TEST(testPictureWithSchemeColor, "picture-with-schemecolor.docx")
{
    // At the start of the document, a picture which has a color specified with a color scheme, lost
    // it's color during import.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic
        = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    Graphic aVclGraphic(xGraphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(341L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(181L, aBitmap.GetSizePixel().Height());
    Color aColor(aBitmap.GetPixelColor(120, 30));
    CPPUNIT_ASSERT_EQUAL(aColor, Color(0xb1, 0xc8, 0xdd));
    aColor = aBitmap.GetPixelColor(260, 130);
    CPPUNIT_ASSERT_EQUAL(aColor, Color(0xb1, 0xc8, 0xdd));
}

DECLARE_OOXMLEXPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.docx")
{
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> image(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue("Graphic") >>= graphic;
    uno::Reference<awt::XBitmap> bitmap(graphic, uno::UNO_QUERY);
    Graphic aVclGraphic(graphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(58L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(320L, aBitmap.GetSizePixel().Height());
    Color aColor(aBitmap.GetPixelColor(20, 30));
    CPPUNIT_ASSERT_EQUAL(Color(255, 0xce, 0xce, 0xce), aColor);
}

DECLARE_OOXMLEXPORT_TEST(testGraphicObjectFliph, "graphic-object-fliph.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnEvenPages"));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnOddPages"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
