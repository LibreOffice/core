/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>



#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

#include <comphelper/sequenceashashmap.hxx>

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
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testDmlShapeTitle, "dml-shape-title.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), getProperty<OUString>(getShape(1), "Title"));
    CPPUNIT_ASSERT_EQUAL(OUString("Description"), getProperty<OUString>(getShape(1), "Description"));
}

DECLARE_OOXMLEXPORT_TEST(testDmlZorder, "dml-zorder.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This was "0": causing that in Word, the second shape was on top, while in the original odt the first shape is on top.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor", "relativeHeight", "2");
}

DECLARE_OOXMLEXPORT_TEST(testDmlShapeRelsize, "dml-shape-relsize.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Relative size wasn't exported all, then relativeFrom was "page", not "margin".
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp14:sizeRelH", "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testDmlPictureInTextframe, "dml-picture-in-textframe.docx")
{
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp14:sizeRelH", "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testDmlTextshape, "dml-textshape.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::TOP, getProperty<sal_Int16>(xGroup, "VertOrient"));
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was drawing::FillStyle_NONE.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xShape, "FillStyle"));
    // This was drawing::LineStyle_NONE.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape, "LineStyle"));

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    // This was wrap="none".
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:bodyPr", "wrap", "square");

    xShape.set(xGroup->getByIndex(3), uno::UNO_QUERY);
    OUString aType = comphelper::SequenceAsHashMap(getProperty<beans::PropertyValues>(xShape, "CustomShapeGeometry"))["Type"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-bentConnector3"), aType);
    // Connector was incorrectly shifted towards the top left corner, X was 552, Y was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4018), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-4487), xShape->getPosition().Y);

    xShape.set(xGroup->getByIndex(5), uno::UNO_QUERY);
    // This was incorrectly shifted towards the top of the page, Y was 106.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-4727), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDmlTextshapeB, "dml-textshapeB.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(3), uno::UNO_QUERY);
    // Connector was incorrectly shifted towards the top left corner, X was 192, Y was -5743.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3778), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-5064), xShape->getPosition().Y);

    xShape.set(xGroup->getByIndex(5), uno::UNO_QUERY);
    // This was incorrectly shifted towards the top of the page, Y was -5011.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-4713), xShape->getPosition().Y);
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
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(getShape(1), "CustomShapeGeometry");
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::MOVETO), aSegments[0].Command );

    // (a:cubicBezTo)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CURVETO), aSegments[1].Command );

    // Coordinates
    sal_Int32 nLength = 16;
    CPPUNIT_ASSERT_EQUAL(nLength, aPairs.getLength());
    std::pair<sal_Int32,sal_Int32> aCoordinates[] =
    {
        std::pair<sal_Int32,sal_Int32>(9084, 0),
        std::pair<sal_Int32,sal_Int32>(6734, 689),
        std::pair<sal_Int32,sal_Int32>(4489, 893),
        std::pair<sal_Int32,sal_Int32>(2633, 893),
        std::pair<sal_Int32,sal_Int32>(1631, 893),
        std::pair<sal_Int32,sal_Int32>(733, 830),
        std::pair<sal_Int32,sal_Int32>(0, 752),
        std::pair<sal_Int32,sal_Int32>(987, 908),
        std::pair<sal_Int32,sal_Int32>(2274, 1034),
        std::pair<sal_Int32,sal_Int32>(3756, 1034),
        std::pair<sal_Int32,sal_Int32>(5357, 1034),
        std::pair<sal_Int32,sal_Int32>(7183, 877),
        std::pair<sal_Int32,sal_Int32>(9084, 423),
        std::pair<sal_Int32,sal_Int32>(9084, 0),
        std::pair<sal_Int32,sal_Int32>(9084, 0),
        std::pair<sal_Int32,sal_Int32>(9084, 0)
    };

    for( int i = 0; i < nLength; ++i )
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
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 2nd frame's context is adjusted to the center
    xFrame.set(getShapeByName("Rectangle 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 3rd frame's context is adjusted to the bottom
    xFrame.set(getShapeByName("Rectangle 3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillBitmapCrop, "dml-shape-fillbitmapcrop.docx")
{
    // Test the new GraphicCrop property which is introduced to define
    // cropping of shapes filled with a picture in stretch mode.

    // 1st shape has some cropping
    text::GraphicCrop aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL( sal_Int32(mbExported ? 454 : 455 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(mbExported ? 367 : 368 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(mbExported ? -454 : -455 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(mbExported ? -367 : -368 ), aGraphicCropStruct.Bottom );

    // 2nd shape has no cropping
    aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Bottom );

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
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill",
            0);

    // check shape style has been exported
    assertXPath(pXmlDoc,
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "2");
    assertXPath(pXmlDoc,
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeParaSpacing, "dml-groupshape-paraspacing.docx")
{
    // Paragraph spacing (top/bottom margin and line spacing) inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();

    // 1st paragraph has 1.5x line spacing but it has no spacing before/after.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    style::LineSpacing aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(150), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 2nd paragraph has double line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(2, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 3rd paragraph has 24 pt line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(3, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::MINIMUM), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(847), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 4th paragraph has 1.75x line spacing but it has no spacing before/after.
    xRun.set(getRun(getParagraphOfText(4, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(175), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 5th paragraph has margins which are defined by w:beforeLines and w:afterLines.
    xRun.set(getRun(getParagraphOfText(5, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(741), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 6th paragraph has margins which are defined by w:before and w:after.
    xRun.set(getRun(getParagraphOfText(6, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    // 7th paragraph has auto paragraph margins a:afterAutospacing and a:beforeAutospacing, which means margins must be ignored.
    xRun.set(getRun(getParagraphOfText(7, xText),1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTableFloatingMargins, "table-floating-margins.docx")
{
    // In case the table had custom left cell margin, the horizontal position was still incorrect (too small, -199).
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    sal_Int32 nHoriOrientPosition = getProperty<sal_Int32>(xFrame, "HoriOrientPosition");
    CPPUNIT_ASSERT(nHoriOrientPosition < sal_Int32(-495));
    // These were 0 as well, due to lack of import.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xFrame, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xFrame, "BottomMargin"));

    // Paragraph bottom margin wasn't 0 in the A1 cell of the floating table.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "after", "0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69636, "fdo69636.docx")
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
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox", "style").match("mso-layout-flow-alt:bottom-to-top"));
    // drawingML
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr", "vert", "vert270");
}

DECLARE_OOXMLEXPORT_TEST(testVMLData, "TestVMLData.docx")
{
    // The problem was exporter was exporting vml data for shape in w:rPr element.
    // vml data should not come under w:rPr element.
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape", "stroked").match("f"));
}

DECLARE_OOXMLEXPORT_TEST(testImageData, "image_data.docx")
{
    // The problem was exporter was exporting v:imagedata data for shape in w:pict as v:fill w element.

    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape/v:imagedata", "detectmouseclick").match("t"));
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
    aXPos[0] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset").toInt32();
    aXPos[1] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset").toInt32();
    aXPos[2] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset").toInt32();
    aXPos[3] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:posOffset").toInt32();

    aYPos[0] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset").toInt32();
    aYPos[1] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset").toInt32();
    aYPos[2] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset").toInt32();
    aYPos[3] = getXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/wp:positionV/wp:posOffset").toInt32();

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
    aHSize[0] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cx").toInt32();
    aHSize[1] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cx").toInt32();
    aHSize[2] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cx").toInt32();
    aHSize[3] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cx").toInt32();

    aVSize[0] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cy").toInt32();
    aVSize[1] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cy").toInt32();
    aVSize[2] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cy").toInt32();
    aVSize[3] = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/wp:extent", "cy").toInt32();

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
    aStyles[0] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:rect", "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:165pt;width:283.4pt;height:141.7pt;rotation:285"
    aStyles[1] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect", "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:255"
    aStyles[2] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Fallback/w:pict/v:rect", "style");
    // original is: "position:absolute;margin-left:97.5pt;margin-top:164.9pt;width:283.4pt;height:141.7pt;rotation:105"
    aStyles[3] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Fallback/w:pict/v:rect", "style");
    // original is: "position:absolute;margin-left:97.55pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:75"

    //check the size and position of each of the shapes
    for( int i = 0; i < 4; ++i )
    {
        CPPUNIT_ASSERT(!aStyles[i].isEmpty());

        sal_Int32 nextTokenPos = 0;
        do
        {
            OUString aStyleCommand = aStyles[i].getToken( 0, ';', nextTokenPos );
            CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

            OUString aStyleCommandName  = aStyleCommand.getToken( 0, ':' );
            OUString aStyleCommandValue = aStyleCommand.getToken( 1, ':' );

            if( aStyleCommandName == "margin-left" )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(97.6, fValue, 0.1);
            }
            else if( aStyleCommandName == "margin-top" )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(165.0, fValue, 0.2);
            }
            else if( aStyleCommandName == "width" )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(283.4, fValue, 0.1);
            }
            else if( aStyleCommandName == "height" )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(141.7, fValue, 0.1);
            }

        } while( nextTokenPos != -1 );
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:prstGeom",
                "prst", "rect");
    // 'adj1' was 'adj', which is not valid for bentConnector3.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[9]/wps:spPr/a:prstGeom/a:avLst/a:gd",
                "name", "adj1");
}

DECLARE_OOXMLEXPORT_TEST(testBehinddoc, "behinddoc.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This was "0", shape was in the foreground.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor", "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(testSmartArtAnchoredInline, "fdo73227.docx")
{
    /* Given file contains 3 DrawingML objects as 1Picture,1SmartArt and 1Shape.
     * Check for SmartArt.
    *  SmartArt should get written as "Floating Object" i.e. inside <wp:anchor> tag.
    *  Also check for value of attribute "id" of <wp:docPr>. It should be unique for
    *  all 3 DrawingML objects in a document.
    */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr","id","1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr","name","Diagram1");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr","id","2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr","name","10-Point Star 3");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr","id","3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr","name","Picture 1");
}

DECLARE_OOXMLEXPORT_TEST(testFdo65833, "fdo65833.docx")
{
    // The "editas" attribute for vml group shape was not preserved.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:group", "editas", "canvas");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73247, "fdo73247.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
                "rot", "1969200");
}

DECLARE_OOXMLEXPORT_TEST(testFdo70942, "fdo70942.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "ellipse");
}

DECLARE_OOXMLEXPORT_TEST(testDrawinglayerPicPos, "drawinglayer-pic-pos.docx")
{
    // The problem was that the position of the picture was incorrect, it was shifted towards the bottom right corner.
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    OString aXPath("/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:xfrm/a:off");
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
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef",
            "idx", "2");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr/a:shade",
            "val", "50000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef",
            "idx", "0");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef/a:schemeClr",
            "val", "accent1");

    // check shape style hasn't been overwritten
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill",
            0);
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill",
            0);

    // check direct theme assignments have been preserved
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr",
            "val", "accent6");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr",
            "val", "accent3");

    // check color transformations applied to theme colors have been preserved
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumOff",
            "val", "60000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr/a:lumMod",
            "val", "50000");

    // check direct color assignments have been preserved
    OUString sFillColor = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:srgbClr",
            "val");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00b050), sFillColor.toInt32(16));
    sal_Int32 nLineColor = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:srgbClr",
            "val").toInt32(16);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), nLineColor);

    // check direct line type assignments have been preserved
    sal_Int32 nLineWidth = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln",
            "w").toInt32();
    CPPUNIT_ASSERT(abs(63500 - nLineWidth) < 1000); //some rounding errors in the conversion ooxml -> libo -> ooxml are tolerated
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:miter",
            1);
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash",
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
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape1, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape2, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH,  getProperty<drawing::LineStyle>(xShape3, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND, getProperty<drawing::LineJoint>(xShape1, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND, getProperty<drawing::LineJoint>(xShape2, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_MITER, getProperty<drawing::LineJoint>(xShape3, "LineJoint"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO73546, "FDO73546.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:hdr/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor", "distL","0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69616, "fdo69616.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    // VML
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[1]/mc:AlternateContent/mc:Fallback/w:pict/v:group", "coordorigin").match("696,725"));
}

DECLARE_OOXMLEXPORT_TEST(testAlignForShape,"Shape.docx")
{
    //fdo73545:Shape Horizontal and vertical orientation is wrong
    //The wp:align tag is missing after roundtrip
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:align","1");
}

DECLARE_OOXMLEXPORT_TEST(testLineStyle_DashType, "LineStyle_DashType.docx")
{
    /* DOCX contatining Shape with LineStyle as Dash Type should get preserved inside
     * an XMl tag <a:prstDash> with value "dash", "sysDot", "lgDot", etc.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "lgDashDotDot");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "lgDashDot");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "lgDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "dashDot");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "dash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "sysDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "sysDot");
}

DECLARE_OOXMLEXPORT_TEST(testGradientFillPreservation, "gradient-fill-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // check rgb colors for every step in the gradient of the first shape
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr",
            "val", "ffff00");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr",
            "val", "ffff33");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[3]/a:srgbClr",
            "val", "ff0000");

    // check theme colors for every step in the gradient of the second shape
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='0']/a:schemeClr",
            "val", "accent5");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='100000']/a:schemeClr",
            "val", "accent1");

    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:srgbClr/a:alpha",
            "val", "20000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr/a:tint",
            "val", "44500");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr/a:satMod",
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:stroke", "dashstyle", "dash");
}

DECLARE_OOXMLEXPORT_TEST(testFdo74110,"fdo74110.docx")
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]",
                "prst", "rect");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/a:avLst[1]/a:gd[1]",0);
}

DECLARE_OOXMLEXPORT_TEST(testOuterShdw,"testOuterShdw.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:effectLst[1]/a:outerShdw[1]", "dist", "1041400");
}

DECLARE_OOXMLEXPORT_TEST(testExtentValue, "fdo74605.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    sal_Int32 nX = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent", "cx").toInt32();
    // This was negative.
    CPPUNIT_ASSERT(nX >= 0);
}

// part of tdf#93676, word gives the frame in the exported .docx a huge height,
// because its exported with 255% height percentage from a 255 HeightPercent
// settings, but 255 is a special flag that the value is synced to the
// other dimension.
DECLARE_OOXMLEXPORT_TEST(testSyncedRelativePercent, "tdf93676-1.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // check no explicit pctHeight has been exported, all we care
    // about at this point is that it's not 255000
    assertXPath(pXmlDoc, "//wp14:pctHeight", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
