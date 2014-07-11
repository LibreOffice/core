/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(WNT)

#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequenceasvector.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1256), xShape->getPosition().Y);

    xShape.set(xGroup->getByIndex(5), uno::UNO_QUERY);
    // This was incorrectly shifted towards the top of the page, Y was 106.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1016), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDMLSolidfillAlpha, "dml-solidfill-alpha.docx")
{
    // Problem was that the transparency was not exported (a:alpha).
    // RGB color (a:srgbClr)
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);;
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
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(450, 44),
        std::pair<sal_Int32,sal_Int32>(300, 57),
        std::pair<sal_Int32,sal_Int32>(176, 57),
        std::pair<sal_Int32,sal_Int32>(109, 57),
        std::pair<sal_Int32,sal_Int32>(49, 53),
        std::pair<sal_Int32,sal_Int32>(0, 48),
        std::pair<sal_Int32,sal_Int32>(66, 58),
        std::pair<sal_Int32,sal_Int32>(152, 66),
        std::pair<sal_Int32,sal_Int32>(251, 66),
        std::pair<sal_Int32,sal_Int32>(358, 66),
        std::pair<sal_Int32,sal_Int32>(480, 56),
        std::pair<sal_Int32,sal_Int32>(607, 27),
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(607, 0)
    };

    for( int i = 0; i < nLength; ++i )
    {
        CPPUNIT_ASSERT_EQUAL(aCoordinates[i].first, aPairs[i].First.Value.get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(aCoordinates[i].second, aPairs[i].Second.Value.get<sal_Int32>());
    }
}

DECLARE_OOXMLEXPORT_TEST(testDmlRectangleRelsize, "dml-rectangle-relsize.docx")
{
    // This was around 19560, as we did not read wp14:pctHeight for
    // drawinglayer shapes and the fallback data was invalid.
    OString aMessage("Height is only " + OString::number(getShape(1)->getSize().Height));
    CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), getShape(1)->getSize().Height >= 20973);

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
    // Problem was parent transformation was ingnored fully, but translate component
    // which specify the position must be also applied for children of the group.

    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xChildGroup(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xGroup.set(xChildGroup, uno::UNO_QUERY);
    xChildGroup.set(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -1856 : -1858), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xChildGroup.set(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(mbExported ? 14028 : 14025), xChildGroup->getPosition().Y);
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "after", "0");
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
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "stroked").match("f"));
}

DECLARE_OOXMLEXPORT_TEST(testImageData, "image_data.docx")
{
    // The problem was exporter was exporting v:imagedata data for shape in w:pict as v:fill w element.

    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:imagedata", "detectmouseclick").match("t"));
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

            if( aStyleCommandName.equals( "margin-left" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(97.6, fValue, 0.1);
            }
            else if( aStyleCommandName.equals( "margin-top" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(165.0, fValue, 0.2);
            }
            else if( aStyleCommandName.equals( "width" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_DOUBLES_EQUAL(283.4, fValue, 0.1);
            }
            else if( aStyleCommandName.equals( "height" ) )
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
    /* Given file conatins 3 DrawingML objects as 1Picture,1SmartArt and 1Shape.
     * Check for SmartArt.
    *  SmartArt shoould get written as "Floating Object" i.e. inside <wp:anchor> tag.
    *  Also check for value of attribute "id" of <wp:docPr> . It should be unique for
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr","name","Picture");
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
    CPPUNIT_ASSERT_EQUAL(sFillColor.toInt32(16), sal_Int32(0x00b050));
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
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:custDash",
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
     * an XMl tag <a:prstDash> with value "dash".
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "dash");
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
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:effectLst[1]/a:outerShdw[1]", "dist", "57811035");
}

DECLARE_OOXMLEXPORT_TEST(testExtentValue, "fdo74605.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent","cx","0");
}

DECLARE_OOXMLEXPORT_TEST( testChildNodesOfCubicBezierTo, "FDO74774.docx")
{
    /* Number of children required by cubicBexTo is 3 of type "pt".
       While exporting, sometimes the child nodes are less than 3.
       The test case ensures that there are 3 child nodes of type "pt"
       for cubicBexTo
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath( pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[3]/wps:spPr[1]/a:custGeom[1]/a:pathLst[1]/a:path[1]/a:cubicBezTo[2]/a:pt[3]");
}

DECLARE_OOXMLEXPORT_TEST(testMSwordHang,"test_msword_hang.docx")
{
    // fdo#74771:
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r[2]/w:drawing/wp:inline", "distT", "0");
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeThemeFont, "groupshape-theme-font.docx")
{
    // Font was specified using a theme reference, which wasn't handled.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    // This was Calibri.
    CPPUNIT_ASSERT_EQUAL(OUString("Cambria"), getProperty<OUString>(xRun, "CharFontName"));
}

DECLARE_OOXMLEXPORT_TEST(testAnchorIdForWP14AndW14, "AnchorId.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:inline", "anchorId", "78735EFD");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "anchorId", "78735EFD");


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

    assertXPath(pXmlHeader1, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]","id","PowerPlusWaterMarkObject93701316");
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
    assertXPath(pXmlHeader1, "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:rect[1]","id","WordPictureWatermark11962361");
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
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:rect[1]/v:textbox[1]/w:txbxContent[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/lc:lockedCanvas[1]",1);
}

DECLARE_OOXMLEXPORT_TEST(testFdo76979, "fdo76979.docx")
{
    // The problem was that black was exported as "auto" fill color, resulting in well-formed, but invalid XML.
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
       return;
    // This was "auto", not "FFFFFF".
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr", "val", "FFFFFF");
}

DECLARE_OOXMLEXPORT_TEST(testShapeEffectPreservation, "shape-effect-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // first shape with outer shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "algn", "tl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "blurRad", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dist", "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "rotWithShape", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha",
            "val", "40000");

    // second shape with outer shadow, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "algn", "tl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "blurRad", "114300");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dist", "203200");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "rotWithShape", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumOff",
            "val", "60000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:alpha",
            "val", "40000");

    // third shape with inner shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "blurRad", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dir", "16200000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dist", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
            "val", "ffff00");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha",
            "val", "50000");

    // 4th shape with soft edge
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge",
            "rad", "127000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge/*",
            0 ); // should not be present

    // 5th shape with glow effect, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow",
            "rad", "101600");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr",
            "val", "accent2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/a:satMod",
            "val", "175000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/a:alpha",
            "val", "40000");

    // 6th shape with reflection
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "blurRad", "6350");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "stA", "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "endA", "300");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "endPos", "55500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection/*",
            0 ); // should not be present

    // 7th shape with several effects: glow, inner shadow and reflection
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow",
            "rad", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr",
            "val", "accent2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:schemeClr/*",
            2);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "blurRad", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha",
            "val", "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "blurRad", "6350");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "stA", "52000");
}

DECLARE_OOXMLEXPORT_TEST(testShape3DEffectPreservation, "shape-3d-effect-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // first shape: extrusion and shift on z, rotated camera with zoom, rotated light rig
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "prst", "perspectiveRelaxedModerately");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "zoom", "150000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "lat", "19490639");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "lon", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "rev", "12900001");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "dir", "t");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "lat", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "lon", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "rev", "4800000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "extrusionH", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "z", "488950");

    // second shape: extrusion with theme color, no camera or light rotation, metal material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "prst", "isometricLeftDown");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "dir", "t");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            0);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "extrusionH", "25400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "metal");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr",
            "val", "accent5");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumOff",
            "val", "60000");

    // third shape: colored countour and top and bottom bevel, plastic material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "contourW", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "plastic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "w", "139700");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "h", "88900");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "prst", "cross");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
            "h", "88900");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
            "prst", "relaxedInset");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:contourClr/a:srgbClr",
            "val", "3333ff");

    // fourth shape: wireframe
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "legacyWireframe");
}

DECLARE_OOXMLEXPORT_TEST(testPictureEffectPreservation, "picture-effects-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // first picture: glow effect with theme color and transformations, 3d rotation and extrusion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow",
            "rad", "228600");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:schemeClr/*",
            2);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:camera",
            "prst", "isometricRightUp");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d",
            "extrusionH", "76200");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d/a:extrusionClr/a:srgbClr",
            "val", "92d050");

    // second picture: shadow and reflection effects
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw",
            "dir", "8100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection",
            "dir", "5400000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection/*",
            0 ); // should not be present

    // third picture: soft edge effect
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:softEdge",
            "rad", "63500");
}

DECLARE_OOXMLEXPORT_TEST(testPictureArtisticEffectPreservation, "picture-artistic-effects-preservation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    xmlDocPtr pRelsDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc || !pRelsDoc)
       return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    // 1st picture: marker effect
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker",
            "trans", "14000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker",
            "size", "80");

    OUString sEmbedId1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    OUString sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 2nd picture: pencil grayscale
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale",
            "trans", "15000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale",
            "pencilSize", "66");

    OUString sEmbedId2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId2);

    // 3rd picture: pencil sketch
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch",
            "trans", "7000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch",
            "pressure", "17");

    OUString sEmbedId3 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId3);

    // 4th picture: light screen
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen",
            "trans", "13000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen",
            "gridSize", "1");

    OUString sEmbedId4 = getXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId4 + "']";
    sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 5th picture: watercolor sponge
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticWatercolorSponge",
            "brushSize", "4");

    OUString sEmbedId5 = getXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId5);

    // 6th picture: photocopy (no attributes)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPhotocopy", 1);

    OUString sEmbedId6 = getXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
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
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:custGeom[1]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testNestedAlternateContent, "nestedAlternateContent.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
     if (!pXmlDoc)
        return;
    // We check alternateContent  could not contains alternateContent (i.e. nested alternateContent)
    assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[2]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[2]/mc:AlternateContent[1]",0);
}

DECLARE_OOXMLEXPORT_TEST(test76317, "test76317.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc) return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]", "prst", "hexagon");
}

DECLARE_OOXMLEXPORT_TEST(fdo76591, "fdo76591.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]", "relativeHeight", "3");
}

DECLARE_OOXMLEXPORT_TEST(test76317_2K10, "test76317_2K10.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc) return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/a:avLst[1]/a:gd[1]", "name", "adj");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77122, "LinkedTextBoxes.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    //ensure that the text box links are preserved.
    assertXPath(pXmlDoc, "//wps:txbx[1]", "id", "1");
    assertXPath(pXmlDoc, "//wps:linkedTxbx[1]", "id", "1");
}

DECLARE_OOXMLEXPORT_TEST(test76734_2K7, "test76734_2K7.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]", "Requires", "wps");
}

DECLARE_OOXMLEXPORT_TEST(test77219, "test77219.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[6]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]", "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(testPresetShape, "preset-shape.docx")
{
    // Document contains a flowChartMultidocument preset shape, our date for that shape wasn't correct.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"]);
    comphelper::SequenceAsVector<awt::Size> aSubViewSize(aPath["SubViewSize"]);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aSubViewSize[0].Height);
}

DECLARE_OOXMLEXPORT_TEST(testFdo76101, "fdo76101.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");

    if (!pXmlDoc)
       return;
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:styles/w:style");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(4091 >= xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testSdtAndShapeOverlapping,"ShapeOverlappingWithSdt.docx")
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
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[2]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/lc:lockedCanvas[1]", 1);
}

DECLARE_OOXMLEXPORT_TEST(fdo78474, "fdo78474.docx")
{
    xmlDocPtr pXmlDoc1 = parseExport("word/document.xml");
    if (!pXmlDoc1) return;
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc1, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]", "embed", "rId2");

    xmlDocPtr pXmlDoc2 = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc2) return;
    assertXPath(pXmlDoc2,"/rels:Relationships/rels:Relationship[2]","Id","rId2");
}

DECLARE_OOXMLEXPORT_TEST(testAbsolutePositionOffsetValue,"fdo78432.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    sal_Int32 IntMax = SAL_MAX_INT32;

    xmlXPathObjectPtr pXmlObjs[6];
    pXmlObjs[0] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[1] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[2] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[3] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[4] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[5] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    for(sal_Int32 index = 0; index<6; ++index)
    {
        CPPUNIT_ASSERT(pXmlObjs[index]->nodesetval != 0);
        xmlNodePtr pXmlNode = pXmlObjs[index]->nodesetval->nodeTab[0];
        OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
        CPPUNIT_ASSERT( contents.toInt64() <= IntMax );
        xmlXPathFreeObject(pXmlObjs[index]);
    }
}

DECLARE_OOXMLEXPORT_TEST(testfdo78300,"fdo78300.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r[1]/w:drawing[1]",
                0);
}

DECLARE_OOXMLEXPORT_TEST(testWordArtWithinDraingtool, "testWordArtWithinDraingtool.docx")
{
/*   * Within a file, there is a 2007 wordArt enclosed in a drawing tool
     * LO was exporting it as below:
     * Sample XML as in Original file:
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <pict> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
     *  After RT :
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <drawing> <txbx> <txbxContent> ..  </txbxContent></txbx> </drawing> .. </txbxContent></txbx> </drawing> </ac> </r> </p>
     *  Expected : As there is nesting of a 2007 Word Art within a draing tool, then can be separated in two different runs.
     * */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent",1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape",1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",1);
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
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path",1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path",1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",1);
}

DECLARE_OOXMLEXPORT_TEST(testFdo78957, "fdo78957.docx")
{
    xmlDocPtr pXmlHeader = parseExport("word/header2.xml");

    if(!pXmlHeader)
        return;

    const sal_Int64 IntMax = SAL_MAX_INT32;
    sal_Int64 cx = 0, cy = 0;
    cx = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]","cx").toInt64();
    cy = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]","cy").toInt64();
    //  Here we check the values of extent width & height
    CPPUNIT_ASSERT(cx <= IntMax );
    CPPUNIT_ASSERT(cy >= 0 );
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
    sal_Int32 d = getXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]","d").toInt32();
    CPPUNIT_ASSERT(d <= maxLimit );
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

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "d" , "800000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "sp", "300000");
}

DECLARE_OOXMLEXPORT_TEST(testDashedLine_CustDash1000thOfPercent, "dashed_line_custdash_1000th_of_percent.docx")
{
    /* Make sure that preset line is exported correctly as "1000th of a percent".
     * This test-file has a CUSTOM dash-line that is defined as '1000th of a percent'.
     * This should be imported by LO as-is, and exported back with the same values.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "d" , "800000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "sp", "300000");
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

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "d" , "800000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "sp", "300000");
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRoundedCorners, "textbox-rounded-corners.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    comphelper::SequenceAsHashMap aCustomShapeGeometry = comphelper::SequenceAsHashMap(getProperty< uno::Sequence<beans::PropertyValue> >(xShape, "CustomShapeGeometry"));

    // Test that the shape is a rounded rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-roundRect"), aCustomShapeGeometry["Type"].get<OUString>());

    // The shape text should start with a table, with "a" in its A1 cell.
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1, xText), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xCell->getString());
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

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr", "name", "");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape", "ID", "");
}

DECLARE_OOXMLEXPORT_TEST(testBnc884615, "bnc884615.docx")
{
    // The problem was that the shape in the header wasn't in the background.
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(getShape(1), "Opaque")));
}

DECLARE_OOXMLEXPORT_TEST(testFdo80894, "TextFrameRotation.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // Rotation value was not roundtripped for textframe.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
    "rot","16200000");
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
