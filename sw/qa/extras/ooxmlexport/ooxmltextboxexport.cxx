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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
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

DECLARE_OOXMLEXPORT_TEST(testFdo80997, "fdo80997.docx")
{
    // The problem was that the DOCX exporter not able to export text behind textbox, if textbox has a wrap property.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1, " text");
}

DECLARE_OOXMLEXPORT_TEST(testMsoPosition, "bnc884615-mso-position.docx")
{
    if (xmlDocPtr doc = parseExport("word/footer1.xml"))
    {
        // We write the frames out in different order than they were read, so check it's the correct
        // textbox first by checking width. These tests may need reordering if that gets fixed.
        OUString style1 = getXPath(
            doc, "/w:ftr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT(style1.indexOf(";width:531pt;") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-vertical-relative:page") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-horizontal-relative:page") >= 0);
        OUString style2 = getXPath(
            doc, "/w:ftr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT(style2.indexOf(";width:549pt;") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-vertical-relative:text") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal:center") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal-relative:text") >= 0);
        OUString style3 = getXPath(
            doc, "/w:ftr/w:p/w:r[5]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT(style3.indexOf(";width:36pt;") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-horizontal-relative:text") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-vertical-relative:text") >= 0);
    }

    xmlDocPtr doc = parseExport("word/header1.xml");
    if (!doc)
        return;

    OUString style1
        = getXPath(doc, "/w:hdr/w:p/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
    CPPUNIT_ASSERT(style1.indexOf(";width:335.75pt;") >= 0);
    CPPUNIT_ASSERT(style1.indexOf(";mso-position-horizontal-relative:page") >= 0);
    CPPUNIT_ASSERT(style1.indexOf(";mso-position-vertical-relative:page") >= 0);
    OUString style2
        = getXPath(doc, "/w:hdr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
    CPPUNIT_ASSERT(style2.indexOf(";width:138.15pt;") >= 0);
    CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal-relative:page") >= 0);
    CPPUNIT_ASSERT(style2.indexOf(";mso-position-vertical-relative:page") >= 0);
    OUString style3
        = getXPath(doc, "/w:hdr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
    CPPUNIT_ASSERT(style3.indexOf(";width:163.8pt;") >= 0);
    CPPUNIT_ASSERT(style3.indexOf(";mso-position-horizontal-relative:page") >= 0);
    CPPUNIT_ASSERT(style3.indexOf(";mso-position-vertical-relative:page") >= 0);
}

DECLARE_OOXMLEXPORT_TEST(testTextFrames, "textframes.odt")
{
    // The frames were simply missing, so let's check if all 3 frames were imported back.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTextFrameBorders, "textframe-borders.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, a TextFrame is created by the VML import.
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), getProperty<sal_Int32>(xFrame, "BackColor"));

        table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xFrame, "TopBorder");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), aBorder.Color);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), aBorder.LineWidth);

        table::ShadowFormat aShadowFormat
            = getProperty<table::ShadowFormat>(xFrame, "ShadowFormat");
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadowFormat.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(48), aShadowFormat.ShadowWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aShadowFormat.Color);
    }
    else
    {
        // After export and import, the result is a shape.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), getProperty<sal_Int32>(xShape, "FillColor"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), getProperty<sal_Int32>(xShape, "LineColor"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), getProperty<sal_Int32>(xShape, "LineWidth"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, "ShadowXDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, "ShadowYDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), getProperty<sal_Int32>(xShape, "ShadowColor"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTextframeGradient, "textframe-gradient.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x666666), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    // Left / right margin was incorrect: the attribute was missing and we
    // didn't have the right default (had 0 instead of the below one).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, "RightMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo48557, "fdo48557.odt")
{
    // Inner margins of the textframe wasn't exported.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextLeftDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextRightDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextUpperDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextLowerDistance"));
}

DECLARE_OOXMLEXPORT_TEST(testN822175, "n822175.odt")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    // Was text::WrapTextMode_THROUGH, due to missing Surround handling in the exporter.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL,
                         getProperty<text::WrapTextMode>(xFrame, "Surround"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo58577, "fdo58577.odt")
{
    // The second frame was simply missing, so let's check if both frames were imported back.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testNestedTextFrames, "nested-text-frames.odt")
{
    // First problem was LO crashed during export (crash test)

    // Second problem was LO made file corruption, writing out nested text boxes, which can't be handled by Word.
    // Test that all three exported text boxes are anchored to the same paragraph and not each other.
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(3), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testfdo79540, "fdo79540.docx")
{
    /* Issue was, <w:drawing> was getting written inside <w:drawing>.
    * So postpone the writing of Inner Drawing tag.
    * MS Office does not allow nesting of drawing tags.
    */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Ensure that two separate w:drawing tags are written after the code changes.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing",
                1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing",
                1);
}

DECLARE_OOXMLEXPORT_TEST(test76734_2K7, "test76734_2K7.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]",
                "Requires", "wps");
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_backgroundTextbox, "tdf108973_backgroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", false,
                                 getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_foregroundTextbox, "tdf108973_foregroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", true,
                                 getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testfdo78300, "fdo78300.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r[1]/w:drawing[1]",
                0);
}

DECLARE_OOXMLEXPORT_TEST(testBnc884615, "bnc884615.docx")
{
    // The problem was that the shape in the header wasn't in the background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo80894, "TextFrameRotation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Rotation value was not roundtripped for textframe.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
                "rot", "16200000");
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTDF87348, "tdf87348_linkedTextboxes.docx")
{
    int followCount = 0;
    int precedeCount = 0;
    if (!parseDump("/root/page/body/txt/anchored/fly[1]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[1]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[2]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[2]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[3]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[3]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[4]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[4]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[5]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[5]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[6]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[6]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[7]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[7]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[8]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[8]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[9]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[9]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[10]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[10]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[11]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[11]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[12]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[12]/txt", "precede").isEmpty())
        precedeCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[13]/txt", "follow").isEmpty())
        followCount++;
    if (!parseDump("/root/page/body/txt/anchored/fly[13]/txt", "precede").isEmpty())
        precedeCount++;
    //there should be 4 chains/13 linked textboxes (set of 5, set of 3, set of 3, set of 2)
    //that means 9 NEXT links and 9 PREV links.
    //however, the current implementation adds leftover shapes, so can't go on exact numbers
    //  (unknown number of flys, unknown order of leftovers)
    CPPUNIT_ASSERT((followCount >= 6) && (precedeCount >= 6));
}
#endif

DECLARE_OOXMLEXPORT_TEST(testfdo78904, "fdo78904.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0),
                             getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testN592908_Frame, "n592908-frame.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, eValue);
}

DECLARE_OOXMLEXPORT_TEST(testFdo55381, "fdo55381.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), xCursor->getPage());
    //TODO: frames not located on the correct pages
}

DECLARE_OOXMLEXPORT_TEST(testTdf103982, "tdf103982.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    sal_Int32 nDistB = getXPath(pXmlDoc, "//wp:anchor", "distB").toInt32();
    // This was -260350, which is not a valid value for an unsigned type.
    CPPUNIT_ASSERT(nDistB >= 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112446_frameStyle, "tdf112446_frameStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf100075, "tdf100075.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);

    // There are two frames in document
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Ensure that frame#1 height is more that frame#2: if no hRul attribute
    // defined, MS Word will use hRul=auto if height is not defined,
    // and hRul=atLeast if height is provided. So frame#1 should be higher
    CPPUNIT_ASSERT(getProperty<sal_Int32>(xFrame1, "Height")
                   > getProperty<sal_Int32>(xFrame2, "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testLibreOfficeHang, "frame-wrap-auto.docx")
{
    // fdo#72775
    // This was text::WrapTextMode_NONE.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_DYNAMIC,
                         getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc875718, "bnc875718.docx")
{
    // The frame in the footer must not accidentally end up in the document body.
    // The easiest way for this to test I've found is checking that
    // xray ThisComponent.TextFrames.GetByIndex( index ).Anchor.Text.ImplementationName
    // is not SwXBodyText but rather SwXHeadFootText
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    for (int i = 0; i < xIndexAccess->getCount(); ++i)
    {
        uno::Reference<text::XTextFrame> frame(xIndexAccess->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> range(frame->getAnchor(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> text(range->getText(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("SwXHeadFootText"), text->getImplementationName());
    }
    // Also check that the footer contents are not in the body text.
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> text(textDocument->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), text->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113399, "tdf113399.doc")
{
    // 0 padding was not preserved
    // In LO 0 is the default, but in OOXML format the default is 254 / 127
    uno::Reference<beans::XPropertySet> xPropSet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nPaddingValue;
    xPropSet->getPropertyValue("TextLeftDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextRightDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextUpperDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextLowerDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
