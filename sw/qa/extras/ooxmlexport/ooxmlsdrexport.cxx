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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <comphelper/sequenceashashmap.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
    }
};

#if 1
#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

// For testing during development of a tast, you want to use
// DECLARE_OOXMLEXPORT_TEST_ONLY, and change the above to #if 0
// Of course, don't forget to set back to #if 1 when you are done :-)
#else
#define DECLARE_OOXMLEXPORT_TEST_ONLY(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

#undef DECLARE_OOXMLEXPORT_TEST
#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) class disabled##TestName : public Test { void disabled(); }; void disabled##TestName::disabled()
#endif

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
    if (!m_bExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), m_aTempFile.GetURL());
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
    CPPUNIT_ASSERT(getShape(1)->getSize().Height > 21000);

    // This was around 0: relative size of 0% was imported as 0, not "fall back to absolute size".
    CPPUNIT_ASSERT(getShape(2)->getSize().Height > 300);
}

DECLARE_OOXMLEXPORT_TEST(testDMLTextFrameVertAdjust, "dml-textframe-vertadjust.docx")
{
    // DOCX textboxes with text are imported as text frames but in Writer text frames did not have
    // TextVerticalAdjust attribute so far.

    // 1st frame's context is adjusted to the top
    uno::Reference<beans::XPropertySet> xFrame(getTextFrameByName("Rectangle 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 2nd frame's context is adjusted to the center
    xFrame.set(getTextFrameByName("Rectangle 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 3rd frame's context is adjusted to the bottom
    xFrame.set(getTextFrameByName("Rectangle 3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillBitmapCrop, "dml-shape-fillbitmapcrop.docx")
{
    // Test the new GraphicCrop property which is introduced to define
    // cropping of shapes filled with a picture in stretch mode.

    // 1st shape has some cropping
    text::GraphicCrop aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? 454 : 455 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? 367 : 368 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? -454 : -455 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? -367 : -368 ), aGraphicCropStruct.Bottom );

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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xGroup.set(xChildGroup, uno::UNO_QUERY);
    xChildGroup.set(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -1856 : -1858), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xChildGroup.set(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 14028 : 14025), xChildGroup->getPosition().Y);
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

#endif

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
