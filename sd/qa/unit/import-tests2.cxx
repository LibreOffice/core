/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sdpage.hxx>

#include "sdmodeltestbase.hxx"

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>

#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdooitm.hxx>
#include <animations/animationnodehelper.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/graphicmimetype.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/dibtools.hxx>

using namespace ::com::sun::star;

class SdImportTest2 : public SdModelTestBase
{
public:
    virtual void setUp() override;

    void testTdf93868();
    void testTdf95932();
    void testTdf99030();
    void testTdf49561();
    void testTdf103473();
    void testAoo124143();
    void testTdf103567();
    void testTdf103792();
    void testTdf103876();
    void testTdf79007();
    void testTdf119649();
    void testTdf118776();
    void testTdf129686();
    void testTdf104015();
    void testTdf104201();
    void testTdf103477();
    void testTdf104445();
    void testTdf105150();
    void testTdf105150PPT();
    void testTdf123684();
    void testTdf100926();
    void testTdf89064();
    void testTdf108925();
    void testTdf109067();
    void testTdf109187();
    void testTdf108926();
    void testTdf100065();
    void testTdf90626();
    void testTdf138148();
    void testTdf114488();
    void testTdf134174();
    void testTdf134210();
    void testTdf114913();
    void testTdf114821();
    void testTdf115394();
    void testTdf115394PPT();
    void testTdf51340();
    void testTdf116899();
    void testTdf77747();
    void testTdf116266();
    void testTdf128684();
    void testShapeGlowEffectPPTXImpoer();
    void testShapeBlurPPTXImport();
    void testMirroredGraphic();
    void testGreysScaleGraphic();
    void testTdf134210CropPosition();

    bool checkPattern(sd::DrawDocShellRef const& rDocRef, int nShapeNumber,
                      std::vector<sal_uInt8>& rExpected);
    void testPatternImport();
    void testPptCrop();
    void testTdf120028();
    void testDescriptionImport();
    void testTdf83247();
    void testTdf47365();
    void testTdf122899();
    void testOOXTheme();
    void testCropToShape();
    void testTdf127964();
    void testTdf106638();
    void testTdf113198();
    void testTdf49856();
    void testTdf103347();
    void testHyperlinksOnShapes();
    void testTdf112209();
    void testTdf128596();
    void testDefaultTabStop();

    CPPUNIT_TEST_SUITE(SdImportTest2);

    CPPUNIT_TEST(testTdf93868);
    CPPUNIT_TEST(testTdf95932);
    CPPUNIT_TEST(testTdf99030);
    CPPUNIT_TEST(testTdf49561);
    CPPUNIT_TEST(testTdf103473);
    CPPUNIT_TEST(testAoo124143);
    CPPUNIT_TEST(testTdf103567);
    CPPUNIT_TEST(testTdf103792);
    CPPUNIT_TEST(testTdf103876);
    CPPUNIT_TEST(testTdf79007);
    CPPUNIT_TEST(testTdf119649);
    CPPUNIT_TEST(testTdf118776);
    CPPUNIT_TEST(testTdf129686);
    CPPUNIT_TEST(testTdf104015);
    CPPUNIT_TEST(testTdf104201);
    CPPUNIT_TEST(testTdf103477);
    CPPUNIT_TEST(testTdf104445);
    CPPUNIT_TEST(testTdf105150);
    CPPUNIT_TEST(testTdf105150PPT);
    CPPUNIT_TEST(testTdf123684);
    CPPUNIT_TEST(testTdf100926);
    CPPUNIT_TEST(testPatternImport);
    CPPUNIT_TEST(testTdf89064);
    CPPUNIT_TEST(testTdf108925);
    CPPUNIT_TEST(testTdf109067);
    CPPUNIT_TEST(testTdf109187);
    CPPUNIT_TEST(testTdf108926);
    CPPUNIT_TEST(testTdf100065);
    CPPUNIT_TEST(testTdf90626);
    CPPUNIT_TEST(testTdf138148);
    CPPUNIT_TEST(testTdf114488);
    CPPUNIT_TEST(testTdf134174);
    CPPUNIT_TEST(testTdf134210);
    CPPUNIT_TEST(testTdf114913);
    CPPUNIT_TEST(testTdf114821);
    CPPUNIT_TEST(testTdf115394);
    CPPUNIT_TEST(testTdf115394PPT);
    CPPUNIT_TEST(testTdf51340);
    CPPUNIT_TEST(testTdf116899);
    CPPUNIT_TEST(testTdf77747);
    CPPUNIT_TEST(testTdf116266);
    CPPUNIT_TEST(testPptCrop);
    CPPUNIT_TEST(testTdf120028);
    CPPUNIT_TEST(testDescriptionImport);
    CPPUNIT_TEST(testTdf83247);
    CPPUNIT_TEST(testTdf47365);
    CPPUNIT_TEST(testTdf122899);
    CPPUNIT_TEST(testOOXTheme);
    CPPUNIT_TEST(testCropToShape);
    CPPUNIT_TEST(testTdf127964);
    CPPUNIT_TEST(testTdf106638);
    CPPUNIT_TEST(testTdf128684);
    CPPUNIT_TEST(testTdf113198);
    CPPUNIT_TEST(testTdf49856);
    CPPUNIT_TEST(testShapeGlowEffectPPTXImpoer);
    CPPUNIT_TEST(testShapeBlurPPTXImport);
    CPPUNIT_TEST(testMirroredGraphic);
    CPPUNIT_TEST(testGreysScaleGraphic);
    CPPUNIT_TEST(testTdf134210CropPosition);
    CPPUNIT_TEST(testTdf103347);
    CPPUNIT_TEST(testHyperlinksOnShapes);
    CPPUNIT_TEST(testTdf112209);
    CPPUNIT_TEST(testTdf128596);
    CPPUNIT_TEST(testDefaultTabStop);

    CPPUNIT_TEST_SUITE_END();
};

void SdImportTest2::setUp()
{
    SdModelTestBase::setUp();
    mxDesktop.set(frame::Desktop::create(getComponentContext()));
}

void SdImportTest2::testTdf93868()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf93868.pptx"), PPTX);

    const SdrPage* pPage = &(GetPage(1, xDocShRef)->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(5), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(
        drawing::FillStyle_SOLID,
        dynamic_cast<const XFillStyleItem&>(pPage->GetObj(0)->GetMergedItem(XATTR_FILLSTYLE))
            .GetValue());
    CPPUNIT_ASSERT_EQUAL(
        drawing::FillStyle_GRADIENT,
        dynamic_cast<const XFillStyleItem&>(pPage->GetObj(1)->GetMergedItem(XATTR_FILLSTYLE))
            .GetValue());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf95932()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf95932.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrObject* const pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT(pObj);

    const XFillStyleItem& rStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
    const XFillColorItem& rColorItem
        = dynamic_cast<const XFillColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR));
    CPPUNIT_ASSERT_EQUAL(Color(0x76bf3d), rColorItem.GetColorValue());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf99030()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf99030.pptx"), PPTX);

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    Color nFillColor;
    uno::Any aAny = xPropSet->getPropertyValue("Background");
    if (aAny.hasValue())
    {
        uno::Reference<beans::XPropertySet> xBackgroundPropSet;
        aAny >>= xBackgroundPropSet;
        xBackgroundPropSet->getPropertyValue("FillColor") >>= nFillColor;
    }
    CPPUNIT_ASSERT_EQUAL(Color(0x676A55), nFillColor);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf49561()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf49561.ppt"), PPT);

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(getShape(3, xPage));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    float fCharHeight = 0;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue("CharHeight") >>= fCharHeight);
    CPPUNIT_ASSERT_EQUAL(12.f, fCharHeight);

    OUString aCharFontName;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue("CharFontName") >>= aCharFontName);
    CPPUNIT_ASSERT_EQUAL(OUString("Stencil"), aCharFontName);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103473()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf103473.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* const pObj = dynamic_cast<SdrTextObj* const>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);
    ::tools::Rectangle aRect = pObj->GetGeoRect();
    CPPUNIT_ASSERT_EQUAL(tools::Long(3629), aRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4431), aRect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(8353), aRect.Right());
    CPPUNIT_ASSERT_EQUAL(tools::Long(9155), aRect.Bottom());

    xDocShRef->DoClose();
}

void SdImportTest2::testAoo124143()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/ooo124143-1.odg"), ODG);

    uno::Reference<beans::XPropertySet> const xImage(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<drawing::XGluePointsSupplier> const xGPS(xImage, uno::UNO_QUERY);
    uno::Reference<container::XIdentifierAccess> const xGluePoints(xGPS->getGluePoints(),
                                                                   uno::UNO_QUERY);

    uno::Sequence<sal_Int32> const ids(xGluePoints->getIdentifiers());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), ids.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ids[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ids[1]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), ids[2]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), ids[3]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), ids[4]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ids[5]);

    // interesting ones are custom 4, 5
    drawing::GluePoint2 glue4;
    xGluePoints->getByIdentifier(4) >>= glue4;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2470), glue4.Position.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1810), glue4.Position.Y);

    drawing::GluePoint2 glue5;
    xGluePoints->getByIdentifier(5) >>= glue5;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2975), glue5.Position.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2165), glue5.Position.Y);

    // now check connectors
    uno::Reference<beans::XPropertySet> const xEllipse(getShapeFromPage(1, 0, xDocShRef));
    uno::Reference<beans::XPropertySet> const xConn1(getShapeFromPage(2, 0, xDocShRef));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         xConn1->getPropertyValue("StartGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn1->getPropertyValue("StartShape").get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4),
                         xConn1->getPropertyValue("EndGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage, xConn1->getPropertyValue("EndShape").get<uno::Reference<beans::XPropertySet>>());
    uno::Reference<beans::XPropertySet> const xConn2(getShapeFromPage(3, 0, xDocShRef));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         xConn2->getPropertyValue("StartGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn2->getPropertyValue("StartShape").get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),
                         xConn2->getPropertyValue("EndGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage, xConn2->getPropertyValue("EndShape").get<uno::Reference<beans::XPropertySet>>());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103567()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf103567.odp"), ODP);
    for (int i = 0; i < 4; ++i)
    {
        uno::Reference<beans::XPropertySet> const xShape(getShapeFromPage(i, 0, xDocShRef));
        uno::Reference<document::XEventsSupplier> const xEventsSupplier(xShape, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> const xEvents(xEventsSupplier->getEvents());
        OString const msg("shape " + OString::number(i) + ": ");

        CPPUNIT_ASSERT(xEvents->hasByName("OnClick"));
        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName("OnClick") >>= props;
        comphelper::SequenceAsHashMap const map(props);
        {
            auto iter(map.find("EventType"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no EventType").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), OUString("Presentation"),
                                         iter->second.get<OUString>());
        }
        {
            auto iter(map.find("ClickAction"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no ClickAction").getStr(), iter != map.end());
            if (i % 2 == 0)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::presentation::ClickAction_DOCUMENT,
                                             iter->second.get<css::presentation::ClickAction>());
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::presentation::ClickAction_NEXTPAGE,
                                             iter->second.get<css::presentation::ClickAction>());
            }
        }
        if (i % 2 == 0)
        {
            auto iter(map.find("Bookmark"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no Bookmark").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), OUString("http://example.com/"),
                                         iter->second.get<OUString>());
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103792()
{
    // Title text shape on the actual slide contained no text neither a placeholder text.
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf103792.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pObj);
    CPPUNIT_ASSERT_MESSAGE("Not a text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to add Title"), aEdit.GetText(0));

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103876()
{
    // Title text shape's placeholder text did not inherit the corresponding text properties
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf103876.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    // Check paragraph alignment
    sal_Int16 nParaAdjust = 0;
    xShape->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    // Check character color
    Color nCharColor;
    xShape->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), nCharColor);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf79007()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf79007.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 0, xDocShRef));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape1.is());

    // Check we map mso washout to our watermark
    drawing::ColorMode aColorMode1;
    xShape1->getPropertyValue("GraphicColorMode") >>= aColorMode1;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_WATERMARK, aColorMode1);

    sal_Int16 nContrast1;
    xShape1->getPropertyValue("AdjustContrast") >>= nContrast1;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast1);

    sal_Int16 nLuminance1;
    xShape1->getPropertyValue("AdjustLuminance") >>= nLuminance1;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance1);

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 0, xDocShRef));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape2.is());

    // Check we map mso grayscale to our grayscale
    drawing::ColorMode aColorMode2;
    xShape2->getPropertyValue("GraphicColorMode") >>= aColorMode2;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_GREYS, aColorMode2);

    sal_Int16 nContrast2;
    xShape2->getPropertyValue("AdjustContrast") >>= nContrast2;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast2);

    sal_Int16 nLuminance2;
    xShape2->getPropertyValue("AdjustLuminance") >>= nLuminance2;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance2);

    uno::Reference<beans::XPropertySet> xShape3(getShapeFromPage(2, 0, xDocShRef));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape3.is());

    // Check we map mso black/white to our black/white
    drawing::ColorMode aColorMode3;
    xShape3->getPropertyValue("GraphicColorMode") >>= aColorMode3;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_MONO, aColorMode3);

    sal_Int16 nContrast3;
    xShape3->getPropertyValue("AdjustContrast") >>= nContrast3;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast3);

    sal_Int16 nLuminance3;
    xShape3->getPropertyValue("AdjustLuminance") >>= nLuminance3;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance3);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf119649()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf119649.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0, xDocShRef));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    CPPUNIT_ASSERT_EQUAL(OUString("default_color("), xRun->getString());

    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharColor);

    xRun.set(getRunFromParagraph(1, xParagraph));

    // Without the fix in place, this test would have failed with
    // - Expected: colored_text
    // - Actual  : colored_text)
    CPPUNIT_ASSERT_EQUAL(OUString("colored_text"), xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xCE181E), nCharColor);

    xRun.set(getRunFromParagraph(2, xParagraph));
    CPPUNIT_ASSERT_EQUAL(OUString(")"), xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharColor);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf118776()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf118776.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    sal_Int16 nTransparency = 0;
    xPropSet->getPropertyValue("CharTransparence") >>= nTransparency;

    // Import noFill color as 99% transparency
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(99), nTransparency);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf129686()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf129686.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    sal_Int16 nTransparency = 0;
    xPropSet->getPropertyValue("CharTransparence") >>= nTransparency;

    // 0 = no transparency, 100 (default) = completely transparent (unless COL_AUTO)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nTransparency);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf104015()
{
    // Shape fill, line and effect properties were not inherited from master slide shape
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104015.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject* const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    // Should have a red fill color
    {
        const XFillStyleItem& rStyleItem
            = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem
            = dynamic_cast<const XFillColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), rColorItem.GetColorValue());
    }
    // Should have a blue line
    {
        const XLineStyleItem& rStyleItem
            = dynamic_cast<const XLineStyleItem&>(pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem
            = dynamic_cast<const XLineColorItem&>(pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x0000FF), rColorItem.GetColorValue());
    }
    // Should have some shadow
    {
        const SdrOnOffItem& rShadowItem
            = dynamic_cast<const SdrOnOffItem&>(pObj->GetMergedItem(SDRATTR_SHADOW));
        CPPUNIT_ASSERT(rShadowItem.GetValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf104201()
{
    // Group shape properties did not overwrite the child shapes' fill
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104201.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);

    // First shape has red fill, but this should be overwritten by green group fill
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem
            = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem
            = dynamic_cast<const XFillColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x00FF00), rColorItem.GetColorValue());
    }
    // Second shape has blue fill, but this should be overwritten by green group fill
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem
            = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem
            = dynamic_cast<const XFillColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x00FF00), rColorItem.GetColorValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103477()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf103477.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);

    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(6));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's color is wrong!", Color(0x000000),
                                 pNumFmt->GetNumRule().GetLevel(1).GetBulletColor());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf105150()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf105150.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(1);
    auto& rFillStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    // This was drawing::FillStyle_NONE, <p:sp useBgFill="1"> was ignored when
    // the slide didn't have an explicit background fill.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf123684()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf123684.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the shape's fill was FillStyle_SOLID, making the text of the shape unreadable.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf105150PPT()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/ppt/tdf105150.ppt"), PPT);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(1);
    // This was drawing::FillStyle_NONE, the shape's mso_fillBackground was
    // ignored when the slide didn't have an explicit background fill.
    auto& rFillStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf104445()
{
    // Extra bullets were added to the first shape
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104445.pptx"), PPTX);

    // First shape should not have bullet
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference<beans::XPropertySet> xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference<container::XIndexAccess> xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for (beans::PropertyValue const& rProp : std::as_const(aBulletProps))
        {
            if (rProp.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(sal_Int16(style::NumberingType::NUMBER_NONE),
                                     rProp.Value.get<sal_Int16>());
            if (rProp.Name == "LeftMargin")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<sal_Int32>());
        }
    }
    // Second shape should have bullet set
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0, xDocShRef));
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference<beans::XPropertySet> xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference<container::XIndexAccess> xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for (beans::PropertyValue const& rProp : std::as_const(aBulletProps))
        {
            if (rProp.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(sal_Int16(style::NumberingType::CHAR_SPECIAL),
                                     rProp.Value.get<sal_Int16>());
            if (rProp.Name == "LeftMargin")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(635), rProp.Value.get<sal_Int32>());
        }
    }
    xDocShRef->DoClose();
}

namespace
{
bool checkPatternValues(std::vector<sal_uInt8>& rExpected, Bitmap& rBitmap)
{
    bool bResult = true;

    const Color aFGColor(0xFF0000);
    const Color aBGColor(0xFFFFFF);

    Bitmap::ScopedReadAccess pAccess(rBitmap);
    for (tools::Long y = 0; y < pAccess->Height(); ++y)
    {
        Scanline pScanline = pAccess->GetScanline(y);
        for (tools::Long x = 0; x < pAccess->Width(); ++x)
        {
            Color aColor = pAccess->GetPixelFromData(pScanline, x);
            sal_uInt8 aValue = rExpected[y * 8 + x];

            if (aValue == 1 && aColor != aFGColor)
                bResult = false;
            else if (aValue == 0 && aColor != aBGColor)
                bResult = false;
        }
    }

    return bResult;
}

} // end anonymous namespace

bool SdImportTest2::checkPattern(sd::DrawDocShellRef const& rDocRef, int nShapeNumber,
                                 std::vector<sal_uInt8>& rExpected)
{
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(nShapeNumber, 0, rDocRef));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape.is());

    Bitmap aBitmap;
    if (xShape.is())
    {
        uno::Any aBitmapAny = xShape->getPropertyValue("FillBitmap");
        uno::Reference<awt::XBitmap> xBitmap;
        if (aBitmapAny >>= xBitmap)
        {
            uno::Sequence<sal_Int8> aBitmapSequence(xBitmap->getDIB());
            SvMemoryStream aBitmapStream(aBitmapSequence.getArray(), aBitmapSequence.getLength(),
                                         StreamMode::READ);
            ReadDIB(aBitmap, aBitmapStream, true);
        }
    }
    CPPUNIT_ASSERT_EQUAL(tools::Long(8), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(8), aBitmap.GetSizePixel().Height());
    return checkPatternValues(rExpected, aBitmap);
}

/* Test checks that importing a PPT file with all supported fill patterns is
 * correctly imported as a tiled fill bitmap with the expected pattern.
 */
void SdImportTest2::testPatternImport()
{
    sd::DrawDocShellRef xDocRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/ppt/FillPatterns.ppt"), PPT);

    std::vector<sal_uInt8> aExpectedPattern1 = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPattern2 = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPattern3 = {
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPattern4 = {
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    };
    std::vector<sal_uInt8> aExpectedPattern5 = {
        1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0,
        0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern6 = {
        1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0,
        1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern7 = {
        1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0,
        1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern8 = {
        1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0,
        1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern9 = {
        0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1,
        1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1,
        1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern10 = {
        0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern11 = {
        1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    std::vector<sal_uInt8> aExpectedPattern12 = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine1 = {
        1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine2 = {
        0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,
        0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine3 = {
        1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
        1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0,
        0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine4 = {
        0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1,
        0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine5 = {
        1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine6 = {
        1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
        1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine7 = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
        0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine8 = {
        1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine9 = {
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine10 = {
        1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine11 = {
        1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
        0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
        1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine12 = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    CPPUNIT_ASSERT_MESSAGE("Pattern1 - 5%", checkPattern(xDocRef, 0, aExpectedPattern1));
    CPPUNIT_ASSERT_MESSAGE("Pattern2 - 10%", checkPattern(xDocRef, 1, aExpectedPattern2));
    CPPUNIT_ASSERT_MESSAGE("Pattern3 - 20%", checkPattern(xDocRef, 2, aExpectedPattern3));
    CPPUNIT_ASSERT_MESSAGE("Pattern4 - 25%", checkPattern(xDocRef, 3, aExpectedPattern4));
    CPPUNIT_ASSERT_MESSAGE("Pattern5 - 30%", checkPattern(xDocRef, 4, aExpectedPattern5));
    CPPUNIT_ASSERT_MESSAGE("Pattern6 - 40%", checkPattern(xDocRef, 5, aExpectedPattern6));
    CPPUNIT_ASSERT_MESSAGE("Pattern7 - 50%", checkPattern(xDocRef, 6, aExpectedPattern7));
    CPPUNIT_ASSERT_MESSAGE("Pattern8 - 60%", checkPattern(xDocRef, 7, aExpectedPattern8));
    CPPUNIT_ASSERT_MESSAGE("Pattern9 - 70%", checkPattern(xDocRef, 8, aExpectedPattern9));
    CPPUNIT_ASSERT_MESSAGE("Pattern10 - 75%", checkPattern(xDocRef, 9, aExpectedPattern10));
    CPPUNIT_ASSERT_MESSAGE("Pattern11 - 80%", checkPattern(xDocRef, 10, aExpectedPattern11));
    CPPUNIT_ASSERT_MESSAGE("Pattern12 - 90%", checkPattern(xDocRef, 11, aExpectedPattern12));

    CPPUNIT_ASSERT_MESSAGE("Pattern13 - Light downward diagonal",
                           checkPattern(xDocRef, 12, aExpectedPatternLine1));
    CPPUNIT_ASSERT_MESSAGE("Pattern14 - Light upward diagonal",
                           checkPattern(xDocRef, 13, aExpectedPatternLine2));
    CPPUNIT_ASSERT_MESSAGE("Pattern15 - Dark downward diagonal",
                           checkPattern(xDocRef, 14, aExpectedPatternLine3));
    CPPUNIT_ASSERT_MESSAGE("Pattern16 - Dark upward diagonal",
                           checkPattern(xDocRef, 15, aExpectedPatternLine4));
    CPPUNIT_ASSERT_MESSAGE("Pattern17 - Wide downward diagonal",
                           checkPattern(xDocRef, 16, aExpectedPatternLine5));
    CPPUNIT_ASSERT_MESSAGE("Pattern18 - Wide upward diagonal",
                           checkPattern(xDocRef, 17, aExpectedPatternLine6));

    CPPUNIT_ASSERT_MESSAGE("Pattern19 - Light vertical",
                           checkPattern(xDocRef, 18, aExpectedPatternLine7));
    CPPUNIT_ASSERT_MESSAGE("Pattern20 - Light horizontal",
                           checkPattern(xDocRef, 19, aExpectedPatternLine8));
    CPPUNIT_ASSERT_MESSAGE("Pattern21 - Narrow vertical",
                           checkPattern(xDocRef, 20, aExpectedPatternLine9));
    CPPUNIT_ASSERT_MESSAGE("Pattern22 - Narrow horizontal",
                           checkPattern(xDocRef, 21, aExpectedPatternLine10));
    CPPUNIT_ASSERT_MESSAGE("Pattern23 - Dark vertical",
                           checkPattern(xDocRef, 22, aExpectedPatternLine11));
    CPPUNIT_ASSERT_MESSAGE("Pattern24 - Dark horizontal",
                           checkPattern(xDocRef, 23, aExpectedPatternLine12));

    // TODO: other patterns in the test document

    xDocRef->DoClose();
}

void SdImportTest2::testTdf100926()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf100926.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    sal_Int32 nRotation = 0;
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nRotation);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nRotation);

    xCell.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRotation);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf89064()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf89064.pptx"), PPTX);
    uno::Reference<presentation::XPresentationPage> xPage(getPage(0, xDocShRef),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xNotesPage(xPage->getNotesPage(), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xNotesPage->getCount());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf108925()
{
    // Test document contains bulleting with too small bullet size (1%) which breaks the lower constraint
    // So it should be converted to the lowest allowed value (25%).
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf108925.odp"), ODP);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();

    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(25), pNumFmt->GetNumRule().GetLevel(0).GetBulletRelSize());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf109067()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf109067.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    awt::Gradient gradient;
    CPPUNIT_ASSERT(xShape->getPropertyValue("FillGradient") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(450), gradient.Angle);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf109187()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf109187.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xArrow1(getShapeFromPage(0, 0, xDocShRef),
                                                uno::UNO_SET_THROW);
    awt::Gradient aGradient1;
    CPPUNIT_ASSERT(xArrow1->getPropertyValue("FillGradient") >>= aGradient1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2250), aGradient1.Angle);
    uno::Reference<beans::XPropertySet> xArrow2(getShapeFromPage(1, 0, xDocShRef),
                                                uno::UNO_SET_THROW);
    awt::Gradient aGradient2;
    CPPUNIT_ASSERT(xArrow2->getPropertyValue("FillGradient") >>= aGradient2);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1350), aGradient2.Angle);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf108926()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf108926.ppt"), PPT);
    uno::Reference<presentation::XPresentationPage> xPage(getPage(0, xDocShRef),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xNotesPage(xPage->getNotesPage(), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xNotesPage->getCount());

    // Second object should be imported as an empty presentation shape
    uno::Reference<beans::XPropertySet> xPresentationShape(xNotesPage->getByIndex(1),
                                                           uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPresentationShape.is());
    bool bIsEmptyPresObject = false;
    xPresentationShape->getPropertyValue("IsEmptyPresentationObject") >>= bIsEmptyPresObject;
    CPPUNIT_ASSERT(bIsEmptyPresObject);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf100065()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf100065.pptx"), PPTX);

    uno::Reference<container::XIndexAccess> xGroupShape1(getShapeFromPage(0, 0, xDocShRef),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape1(xGroupShape1->getByIndex(1), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle1;
    CPPUNIT_ASSERT(xShape1->getPropertyValue("RotateAngle") >>= nAngle1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nAngle1);

    uno::Reference<container::XIndexAccess> xGroupShape2(getShapeFromPage(1, 0, xDocShRef),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape2(xGroupShape2->getByIndex(0), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle2;
    CPPUNIT_ASSERT(xShape2->getPropertyValue("RotateAngle") >>= nAngle2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18000), nAngle2);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf90626()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf90626.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    for (int i = 0; i < 4; i++)
    {
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(i).GetItem(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT(pNumFmt);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(372),
                                     pNumFmt->GetNumRule().GetLevel(0).GetGraphicSize().getHeight(),
                                     tools::Long(1));
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf138148()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf138148.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    for (int i = 0; i < 2; i++)
    {
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(i).GetItem(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT(pNumFmt);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(444),
                                     pNumFmt->GetNumRule().GetLevel(0).GetGraphicSize().getHeight(),
                                     tools::Long(1));

        // Without the fix in place, this test would have failed with
        // - Expected: 148
        // - Actual  : 444
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(148),
                                     pNumFmt->GetNumRule().GetLevel(0).GetGraphicSize().getWidth(),
                                     tools::Long(1));
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf114488()
{
    // This doc has two images - one WMF and the other PNG (fallback image).
    // When loading this doc, the WMF image should be preferred over the PNG image.
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odg/tdf114488.fodg"), FODG);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    OUString sMimeType(comphelper::GraphicMimeTypeHelper::GetMimeTypeForXGraphic(xGraphic));
    CPPUNIT_ASSERT_EQUAL(OUString("image/x-wmf"), sMimeType);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf134174()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf134174.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x8b21db), aBitmap.GetPixelColor(0, 0));

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf134210()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf134210.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x60563e), aBitmap.GetPixelColor(0, 0));

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf114913()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf114913.pptx"), PPTX);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(GetPage(1, xDocShRef)->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const SvxNumBulletItem* pItem
        = pTxtObj->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0).GetItem(
            EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(tools::Long(692),
                         pItem->GetNumRule().GetLevel(0).GetGraphicSize().getHeight());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf114821()
{
    css::uno::Any aAny;
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf114821.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0, xDocShRef));
    aAny = xPropSet->getPropertyValue("Model");
    CPPUNIT_ASSERT_MESSAGE("The shape doesn't have the property", aAny.hasValue());

    uno::Reference<chart::XChartDocument> xChartDoc;
    aAny >>= xChartDoc;
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    uno::Reference<chart2::XChartDocument> xChart2Doc(xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    uno::Reference<chart2::XCoordinateSystemContainer> xBCooSysCnt(xChart2Doc->getFirstDiagram(),
                                                                   uno::UNO_QUERY);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> aCooSysSeq(
        xBCooSysCnt->getCoordinateSystems());
    uno::Reference<chart2::XChartTypeContainer> xCTCnt(aCooSysSeq[0], uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeriesContainer> xDSCnt(xCTCnt->getChartTypes()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load data series", xDSCnt.is());
    uno::Sequence<uno::Reference<chart2::XDataSeries>> aSeriesSeq(xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", static_cast<sal_Int32>(1),
                                 aSeriesSeq.getLength());

    // These Labels have custom position, so the exported LabelPlacement (reference point) by MSO is OUTSIDE/OUTEND
    // Check the first label
    const css::uno::Reference<css::beans::XPropertySet>& rPropSet0(
        aSeriesSeq[0]->getDataPointByIndex(0));
    CPPUNIT_ASSERT(rPropSet0.is());
    sal_Int32 aPlacement;
    rPropSet0->getPropertyValue("LabelPlacement") >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(css::chart::DataLabelPlacement::OUTSIDE, aPlacement);

    // Check the second label
    const css::uno::Reference<css::beans::XPropertySet>& rPropSet1(
        aSeriesSeq[0]->getDataPointByIndex(1));
    CPPUNIT_ASSERT(rPropSet1.is());
    rPropSet1->getPropertyValue("LabelPlacement") >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(css::chart::DataLabelPlacement::OUTSIDE, aPlacement);

    // Check the third label
    const css::uno::Reference<css::beans::XPropertySet>& rPropSet2(
        aSeriesSeq[0]->getDataPointByIndex(2));
    CPPUNIT_ASSERT(rPropSet2.is());
    rPropSet2->getPropertyValue("LabelPlacement") >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(css::chart::DataLabelPlacement::OUTSIDE, aPlacement);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf115394()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf115394.pptx"), PPTX);
    double fTransitionDuration;

    // Slow in MS formats
    SdPage* pPage1 = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    // Medium in MS formats
    SdPage* pPage2 = xDocShRef->GetDoc()->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Fast in MS formats
    SdPage* pPage3 = xDocShRef->GetDoc()->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Custom values
    SdPage* pPage4 = xDocShRef->GetDoc()->GetSdPage(3, PageKind::Standard);
    fTransitionDuration = pPage4->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.25, fTransitionDuration);

    SdPage* pPage5 = xDocShRef->GetDoc()->GetSdPage(4, PageKind::Standard);
    fTransitionDuration = pPage5->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(4.25, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf115394PPT()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf115394.ppt"), PPT);
    double fTransitionDuration;

    // Fast
    SdPage* pPage1 = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Medium
    SdPage* pPage2 = xDocShRef->GetDoc()->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Slow
    SdPage* pPage3 = xDocShRef->GetDoc()->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf51340()
{
    // Line spacing was not inherited from upper levels (slide layout, master slide)
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf51340.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0, xDocShRef));

    // First paragraph has a 90% line spacing set on master slide
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    css::style::LineSpacing aSpacing;
    xPropSet->getPropertyValue("ParaLineSpacing") >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(90), aSpacing.Height);

    // Second paragraph has a 125% line spacing set on slide layout
    xParagraph.set(getParagraphFromShape(1, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaLineSpacing") >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(125), aSpacing.Height);

    // Third paragraph has a 70% line spacing set directly on normal slide (master slide property is overridden)
    xParagraph.set(getParagraphFromShape(2, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaLineSpacing") >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(70), aSpacing.Height);

    // Fourth paragraph has a 190% line spacing set directly on normal slide (slide layout property is overridden)
    xParagraph.set(getParagraphFromShape(3, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaLineSpacing") >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(190), aSpacing.Height);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf116899()
{
    // This is a PPT created in Impress and roundtripped in PP, the key times become [1, -1] in PP,
    //  a time of -1 (-1000) in PPT means key times have to be distributed evenly between 0 and 1
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf116899.ppt"), PPT);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<animations::XAnimationNodeSupplier> xAnimNodeSupplier(xPage,
                                                                         uno::UNO_QUERY_THROW);
    uno::Reference<animations::XAnimationNode> xRootNode(xAnimNodeSupplier->getAnimationNode());
    std::vector<uno::Reference<animations::XAnimationNode>> aAnimVector;
    anim::create_deep_vector(xRootNode, aAnimVector);
    uno::Reference<animations::XAnimate> xNode(aAnimVector[8], uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of key times in the animation node isn't 2.",
                                 static_cast<sal_Int32>(2), xNode->getKeyTimes().getLength());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "First key time in the animation node isn't 0, key times aren't normalized.", 0.,
        xNode->getKeyTimes()[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Second key time in the animation node isn't 1, key times aren't normalized.", 1.,
        xNode->getKeyTimes()[1]);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf77747()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf77747.ppt"), PPT);
    CPPUNIT_ASSERT(xDocShRef.is());
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(GetPage(1, xDocShRef)->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const SvxNumBulletItem* pNumFmt
        = pTxtObj->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0).GetItem(
            EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's suffix is wrong!", OUString("-"),
                                 pNumFmt->GetNumRule().GetLevel(0).GetSuffix());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's numbering type is wrong!", SVX_NUM_NUMBER_HEBREW,
                                 pNumFmt->GetNumRule().GetLevel(0).GetNumberingType());

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf116266()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf116266.odp"), ODP);
    SdDrawDocument* pDoc = xDocShRef->GetDoc();
    sfx2::LinkManager* rLinkManager = pDoc->GetLinkManager();
    // The document contains one SVG stored as a link.
    CPPUNIT_ASSERT_EQUAL(size_t(1), rLinkManager->GetLinks().size());

    xDocShRef->DoClose();
}

void SdImportTest2::testPptCrop()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/crop.ppt"), PPT);

    uno::Reference<beans::XPropertySet> xPropertySet(
        getShapeFromPage(/*nShape=*/1, /*nPage=*/0, xDocShRef));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue("GraphicCrop") >>= aCrop;
    // These were all 0, lazy-loading broke cropping.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Top);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Bottom);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Right);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf120028()
{
    // Check that the text shape has 4 columns.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf120028.pptx"), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    uno::Reference<text::XTextColumns> xCols(xShape->getPropertyValue("TextColumns"),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4), xCols->getColumnCount());
    uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(0)), xColProps->getPropertyValue("AutomaticDistance"));

    // Check font size in the shape.
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    double fCharHeight = 0;
    xPropSet->getPropertyValue("CharHeight") >>= fCharHeight;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.5, fCharHeight, 1E-12);
    // 13.5 * 86% is approx. 11.6 (the correct scaled font size)
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(86)), xShape->getPropertyValue("TextFitToSizeScale"));

    xDocShRef->DoClose();
}

void SdImportTest2::testDescriptionImport()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/altdescription.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xPropertySet(
        getShapeFromPage(/*nShape=*/2, /*nPage=*/0, xDocShRef));
    OUString sDesc;

    xPropertySet->getPropertyValue("Description") >>= sDesc;

    CPPUNIT_ASSERT_EQUAL(OUString("We Can Do It!"), sDesc);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf83247()
{
    auto GetPause = [this](const OUString& sSrc, sal_Int32 nFormat) {
        sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(sSrc), nFormat);
        uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(
            xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPresentationProps(
            xPresentationSupplier->getPresentation(), uno::UNO_QUERY_THROW);

        auto retVal = xPresentationProps->getPropertyValue("Pause");
        xDocShRef->DoClose();
        return retVal.get<sal_Int32>();
    };

    // 1. Check that presentation:pause attribute is imported correctly
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), GetPause("/sd/qa/unit/data/odp/loopPause10.odp", ODP));

    // 2. ODF compliance: if presentation:pause attribute is absent, it must be treated as 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetPause("/sd/qa/unit/data/odp/loopNoPause.odp", ODP));

    // 3. Import PPT: pause should be 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetPause("/sd/qa/unit/data/ppt/loopNoPause.ppt", PPT));
}

void SdImportTest2::testTdf47365()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/loopNoPause.pptx"), PPTX);
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    const bool bEndlessVal = xPresentationProps->getPropertyValue("IsEndless").get<bool>();
    const sal_Int32 nPauseVal = xPresentationProps->getPropertyValue("Pause").get<sal_Int32>();

    // Check that we import "loop" attribute of the presentation, and don't introduce any pauses
    CPPUNIT_ASSERT(bEndlessVal);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPauseVal);

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf122899()
{
    // tdf122899 FILEOPEN: ppt: old kind arc from MS Office 97 is broken
    // Error was, that the path coordinates of a mso_sptArc shape were read as sal_Int16
    // although they are unsigned 16 bit. This leads to wrong positions of start and end
    // point and results to a huge shape width in the test document.
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/ppt/tdf122899_Arc_90_to_91_clockwise.ppt"),
        PPT);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    awt::Rectangle aFrameRect;
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
    // original width is 9cm, add some tolerance
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(9020), aFrameRect.Width);

    xDocShRef->DoClose();
}

void SdImportTest2::testOOXTheme()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/ooxtheme.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xPropSet(xDocShRef->GetDoc()->getUnoModel(),
                                                 uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBag;
    xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;

    bool bTheme = false;
    for (beans::PropertyValue const& prop : std::as_const(aGrabBag))
    {
        if (prop.Name == "OOXTheme")
        {
            bTheme = true;
            uno::Reference<xml::dom::XDocument> aThemeDom;
            CPPUNIT_ASSERT(prop.Value >>= aThemeDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aThemeDom); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(bTheme); // Grab Bag has all the expected elements

    xDocShRef->DoClose();
}

void SdImportTest2::testCropToShape()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/crop-to-shape.pptx"), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString{ "com.sun.star.drawing.CustomShape" }, xShape->getShapeType());
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    css::drawing::FillStyle fillStyle;
    xShapeProps->getPropertyValue("FillStyle") >>= fillStyle;
    CPPUNIT_ASSERT_EQUAL(css::drawing::FillStyle_BITMAP, fillStyle);
    css::drawing::BitmapMode bitmapmode;
    xShapeProps->getPropertyValue("FillBitmapMode") >>= bitmapmode;
    CPPUNIT_ASSERT_EQUAL(css::drawing::BitmapMode_STRETCH, bitmapmode);
}

void SdImportTest2::testTdf127964()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf127964.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rFillStyleItem.GetValue());

    auto& rFillColorItem
        = dynamic_cast<const XFillColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4294967295
    // - Actual  : 5210557
    // i.e. instead of transparent (which then got rendered as white), the shape fill color was
    // blue.
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, rFillColorItem.GetColorValue());
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf106638()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf106638.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xPara(getParagraphFromShape(1, xShape));
    uno::Reference<text::XText> xText = xPara->getText();
    uno::Reference<text::XTextCursor> xTextCursor
        = xText->createTextCursorByRange(xPara->getStart());
    uno::Reference<beans::XPropertySet> xPropSet(xTextCursor, uno::UNO_QUERY_THROW);
    OUString aCharFontName;
    CPPUNIT_ASSERT(xTextCursor->goRight(1, true));
    // First character U+f0fe that uses Wingding
    xPropSet->getPropertyValue("CharFontName") >>= aCharFontName;
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), aCharFontName);

    // The rest characters that do not use Wingding.
    CPPUNIT_ASSERT(xTextCursor->goRight(45, true));
    xPropSet->getPropertyValue("CharFontName") >>= aCharFontName;
    CPPUNIT_ASSERT(aCharFontName != "Wingdings");
}

void SdImportTest2::testTdf128684()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf128684.pptx"), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());
    uno::Any aAny = xShape->getPropertyValue("CustomShapeGeometry");
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Sequence<beans::PropertyValue> aProps;
    CPPUNIT_ASSERT(aAny >>= aProps);
    sal_Int32 nRotateAngle = 0;
    for (const auto& rProp : std::as_const(aProps))
    {
        if (rProp.Name == "TextPreRotateAngle")
        {
            rProp.Value >>= nRotateAngle;
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-90), nRotateAngle);
}

void SdImportTest2::testTdf113198()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf113198.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    sal_Int16 nParaAdjust = -1;
    xShape->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

void SdImportTest2::testTdf49856()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/ppt/tdf49856.ppt"), PPT);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(2).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    const sal_UCS4 aBullet = pNumFmt->GetNumRule().GetLevel(0).GetBulletChar();
    CPPUNIT_ASSERT_EQUAL(OUString("More level 2"), aEdit.GetText(2));
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x2022), aBullet);

    xDocShRef->DoClose();
}

void SdImportTest2::testShapeGlowEffectPPTXImpoer()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-glow-effect.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("GlowEffectRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 139700 EMU = 388.0556 mm/100
    Color nColor;
    xShape->getPropertyValue("GlowEffectColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue("GlowEffectTransparency") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nTransparency);
}

void SdImportTest2::testShapeBlurPPTXImport()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-blur-effect.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);

    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("ShadowBlur") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 584200EMU=46pt - 139700EMU = 388Hmm = 11pt
}

void SdImportTest2::testMirroredGraphic()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/mirrored-graphic.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x4f4955), aBitmap.GetPixelColor(0, 0));
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf134210CropPosition()
{
    // We are testing crop position of bitmap in custom shapes. We should see only green with proper fix.
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/crop-position.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x81d41a), aBitmap.GetPixelColor(0, 0));
    xDocShRef->DoClose();
}

void SdImportTest2::testGreysScaleGraphic()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/greysscale-graphic.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x3c3c3c), aBitmap.GetPixelColor(0, 0));
    xDocShRef->DoClose();
}

void SdImportTest2::testTdf103347()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf103347.pptx"), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xNamed1->getName());

    uno::Reference<drawing::XDrawPage> xPage2(xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed2(xPage2, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello (2)"), xNamed2->getName());

    uno::Reference<drawing::XDrawPage> xPage3(xDoc->getDrawPages()->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed3(xPage3, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello (3)"), xNamed3->getName());

    xDocShRef->DoClose();
}

void SdImportTest2::testHyperlinksOnShapes()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf144616.pptx"), PPTX);

    for (sal_Int32 i = 0; i < 7; i++)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(i, 0, xDocShRef));
        uno::Reference<document::XEventsSupplier> xEventsSupplier(xShape, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xEvents(xEventsSupplier->getEvents());

        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName("OnClick") >>= props;
        comphelper::SequenceAsHashMap map(props);
        auto iter(map.find("ClickAction"));
        switch (i)
        {
            case 0:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_FIRSTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 1:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_LASTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 2:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_NEXTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 3:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_PREVPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 4:
            {
                auto iter2(map.find("Bookmark"));
                CPPUNIT_ASSERT_EQUAL(OUString("Second slide"), iter2->second.get<OUString>());
            }
            break;
            case 5:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_STOPPRESENTATION,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 6:
            {
                auto iter1(map.find("Bookmark"));
                CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/"),
                                     iter1->second.get<OUString>());
            }
            break;
            default:
                break;
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf112209()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112209.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:132 G:132 B:132 A:0
    // - Actual  : Color: R:21 G:170 B:236 A:0
    // i.e. the image color was blue instead of grey.
    CPPUNIT_ASSERT_EQUAL(Color(0x848484), aBitmap.GetPixelColor(0, 0));

    xDocShRef->DoClose();
}

void SdImportTest2::testTdf128596()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf128596.pptx"), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef),
                                               uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    css::drawing::BitmapMode bitmapmode;
    xShape->getPropertyValue("FillBitmapMode") >>= bitmapmode;
    CPPUNIT_ASSERT_EQUAL(css::drawing::BitmapMode_REPEAT, bitmapmode);

    xDocShRef->DoClose();
}

void SdImportTest2::testDefaultTabStop()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/deftabstop.pptx"), PPTX);
    SdDrawDocument* pDoc = xDocShRef->GetDoc();
    sal_Int32 nDefTab = pDoc->GetDefaultTabulator();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), nDefTab);

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
