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
#include <tools/color.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>

#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdooitm.hxx>
#include <animations/animationnodehelper.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/Gradient.hpp>
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
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
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
    SdImportTest2()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }

protected:
    bool checkPattern(int nShapeNumber, std::vector<sal_uInt8>& rExpected);
};

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf157285)
{
    createSdImpressDoc("pptx/tdf157285.pptx");

    uno::Reference<drawing::XShape> xShape1(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    sal_Int32 nHeight1 = xShape1->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 2541
    // Actual  : placeholder height: 3435
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2541), nHeight1);

    uno::Reference<drawing::XShape> xShape2(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    sal_Int32 nHeight2 = xShape2->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 1169
    // Actual  : placeholder height: 11303
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1169), nHeight2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf152186)
{
    createSdImpressDoc("pptx/tdf152186.pptx");
    saveAndReload("Impress MS PowerPoint 2007 XML");

    bool bHasShadow = bool();
    const SdrPage* pPage = GetPage(1);
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(i, 0));
        CPPUNIT_ASSERT(xShape->getPropertyValue("Shadow") >>= bHasShadow);
        CPPUNIT_ASSERT(!bHasShadow);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf93868)
{
    createSdImpressDoc("pptx/tdf93868.pptx");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(5), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID,
                         pPage->GetObj(0)->GetMergedItem(XATTR_FILLSTYLE).GetValue());

    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         pPage->GetObj(1)->GetMergedItem(XATTR_FILLSTYLE).GetValue());
    CPPUNIT_ASSERT_EQUAL(true,
                         pPage->GetObj(1)->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND).GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf95932)
{
    createSdImpressDoc("pptx/tdf95932.pptx");

    const SdrPage* pPage = GetPage(1);
    SdrObject* const pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT(pObj);

    const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
    const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
    CPPUNIT_ASSERT_EQUAL(Color(0x76bf3d), rColorItem.GetColorValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf99030)
{
    createSdImpressDoc("pptx/tdf99030.pptx");

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf49561)
{
    createSdImpressDoc("ppt/tdf49561.ppt");

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103473)
{
    createSdImpressDoc("pptx/tdf103473.pptx");

    const SdrPage* pPage = GetPage(1);
    SdrTextObj* const pObj = dynamic_cast<SdrTextObj* const>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);
    ::tools::Rectangle aRect = pObj->GetGeoRect();
    CPPUNIT_ASSERT_EQUAL(tools::Long(3629), aRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4431), aRect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(8353), aRect.Right());
    CPPUNIT_ASSERT_EQUAL(tools::Long(9155), aRect.Bottom());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testAoo124143)
{
    createSdDrawDoc("odg/ooo124143-1.odg");

    uno::Reference<beans::XPropertySet> const xImage(getShapeFromPage(0, 0));
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
    uno::Reference<beans::XPropertySet> const xEllipse(getShapeFromPage(1, 0));
    uno::Reference<beans::XPropertySet> const xConn1(getShapeFromPage(2, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         xConn1->getPropertyValue("StartGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn1->getPropertyValue("StartShape").get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4),
                         xConn1->getPropertyValue("EndGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage, xConn1->getPropertyValue("EndShape").get<uno::Reference<beans::XPropertySet>>());
    uno::Reference<beans::XPropertySet> const xConn2(getShapeFromPage(3, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         xConn2->getPropertyValue("StartGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn2->getPropertyValue("StartShape").get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),
                         xConn2->getPropertyValue("EndGluePointIndex").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage, xConn2->getPropertyValue("EndShape").get<uno::Reference<beans::XPropertySet>>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103567)
{
    createSdImpressDoc("odp/tdf103567.odp");
    for (int i = 0; i < 4; ++i)
    {
        uno::Reference<beans::XPropertySet> const xShape(getShapeFromPage(i, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103792)
{
    // Title text shape on the actual slide contained no text neither a placeholder text.

    createSdImpressDoc("pptx/tdf103792.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT_MESSAGE("Not a text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to add Title"), aEdit.GetText(0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf148685)
{
    createSdImpressDoc("pptx/tdf148685.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    CPPUNIT_ASSERT_EQUAL(OUString("TEXT "), xRun->getString());

    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharUnderlineColor;
    xPropSet->getPropertyValue("CharUnderlineColor") >>= nCharUnderlineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xA1467E), nCharUnderlineColor);

    xRun.set(getRunFromParagraph(1, xParagraph));

    CPPUNIT_ASSERT_EQUAL(OUString("TE"), xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue("CharUnderlineColor") >>= nCharUnderlineColor;

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B:255 A:255
    // - Actual  : Color: R:161 G:70 B:126 A:0
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharUnderlineColor);

    xRun.set(getRunFromParagraph(2, xParagraph));
    CPPUNIT_ASSERT_EQUAL(OUString("XT"), xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue("CharUnderlineColor") >>= nCharUnderlineColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharUnderlineColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103876)
{
    // Title text shape's placeholder text did not inherit the corresponding text properties

    createSdImpressDoc("pptx/tdf103876.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Check paragraph alignment
    sal_Int16 nParaAdjust = 0;
    xShape->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    // Check character color
    Color nCharColor;
    xShape->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf79007)
{
    createSdImpressDoc("pptx/tdf79007.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 0));
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

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 0));
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

    uno::Reference<beans::XPropertySet> xShape3(getShapeFromPage(2, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf119649)
{
    createSdImpressDoc("pptx/tdf119649.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf118776)
{
    createSdImpressDoc("pptx/tdf118776.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    sal_Int16 nTransparency = 0;
    xPropSet->getPropertyValue("CharTransparence") >>= nTransparency;

    // Import noFill color as 99% transparency
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(99), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf129686)
{
    createSdImpressDoc("pptx/tdf129686.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    sal_Int16 nTransparency = 0;
    xPropSet->getPropertyValue("CharTransparence") >>= nTransparency;

    // 0 = no transparency, 100 (default) = completely transparent (unless COL_AUTO)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf104015)
{
    // Shape fill, line and effect properties were not inherited from master slide shape

    createSdImpressDoc("pptx/tdf104015.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject* const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    // Should have a red fill color
    {
        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, rColorItem.GetColorValue());
    }
    // Should have a blue line
    {
        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, rColorItem.GetColorValue());
    }
    // Should have some shadow
    {
        const SdrOnOffItem& rShadowItem = pObj->GetMergedItem(SDRATTR_SHADOW);
        CPPUNIT_ASSERT(rShadowItem.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf104201)
{
    // Group shape properties did not overwrite the child shapes' fill

    createSdImpressDoc("pptx/tdf104201.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);

    // First shape has red fill, but this should be overwritten by green group fill
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, rColorItem.GetColorValue());
    }
    // Second shape has blue fill, but this should be overwritten by green group fill
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, rColorItem.GetColorValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103477)
{
    createSdImpressDoc("pptx/tdf103477.pptx");

    const SdrPage* pPage = GetPage(1);

    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(6));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's color is wrong!", COL_BLACK,
                                 pNumFmt->GetNumRule().GetLevel(1).GetBulletColor());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf105150)
{
    createSdImpressDoc("pptx/tdf105150.pptx");
    const SdrPage* pPage = GetPage(1);
    const SdrObject* pObj = pPage->GetObj(1);
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    // This was drawing::FillStyle_NONE, <p:sp useBgFill="1"> was ignored when
    // the slide didn't have an explicit background fill.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
    auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
    CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf123684)
{
    createSdImpressDoc("pptx/tdf123684.pptx");
    const SdrPage* pPage = GetPage(1);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the shape's fill was FillStyle_SOLID, making the text of the shape unreadable.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf105150PPT)
{
    createSdImpressDoc("ppt/tdf105150.ppt");
    const SdrPage* pPage = GetPage(1);
    const SdrObject* pObj = pPage->GetObj(1);
    // This was drawing::FillStyle_NONE, the shape's mso_fillBackground was
    // ignored when the slide didn't have an explicit background fill.
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
    auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
    CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf104445)
{
    // Extra bullets were added to the first shape

    createSdImpressDoc("pptx/tdf104445.pptx");

    // First shape should not have bullet
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference<beans::XPropertySet> xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference<container::XIndexAccess> xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for (beans::PropertyValue const& rProp : aBulletProps)
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
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference<beans::XPropertySet> xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference<container::XIndexAccess> xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for (beans::PropertyValue const& rProp : aBulletProps)
        {
            if (rProp.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(sal_Int16(style::NumberingType::CHAR_SPECIAL),
                                     rProp.Value.get<sal_Int16>());
            if (rProp.Name == "LeftMargin")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(635), rProp.Value.get<sal_Int32>());
        }
    }
}

namespace
{
bool checkPatternValues(std::vector<sal_uInt8>& rExpected, Bitmap& rBitmap)
{
    bool bResult = true;

    const Color aFGColor(COL_LIGHTRED);
    const Color aBGColor(COL_WHITE);

    BitmapScopedReadAccess pAccess(rBitmap);
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

bool SdImportTest2::checkPattern(int nShapeNumber, std::vector<sal_uInt8>& rExpected)
{
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(nShapeNumber, 0));
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
CPPUNIT_TEST_FIXTURE(SdImportTest2, testPatternImport)
{
    createSdImpressDoc("ppt/FillPatterns.ppt");

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

    CPPUNIT_ASSERT_MESSAGE("Pattern1 - 5%", checkPattern(0, aExpectedPattern1));
    CPPUNIT_ASSERT_MESSAGE("Pattern2 - 10%", checkPattern(1, aExpectedPattern2));
    CPPUNIT_ASSERT_MESSAGE("Pattern3 - 20%", checkPattern(2, aExpectedPattern3));
    CPPUNIT_ASSERT_MESSAGE("Pattern4 - 25%", checkPattern(3, aExpectedPattern4));
    CPPUNIT_ASSERT_MESSAGE("Pattern5 - 30%", checkPattern(4, aExpectedPattern5));
    CPPUNIT_ASSERT_MESSAGE("Pattern6 - 40%", checkPattern(5, aExpectedPattern6));
    CPPUNIT_ASSERT_MESSAGE("Pattern7 - 50%", checkPattern(6, aExpectedPattern7));
    CPPUNIT_ASSERT_MESSAGE("Pattern8 - 60%", checkPattern(7, aExpectedPattern8));
    CPPUNIT_ASSERT_MESSAGE("Pattern9 - 70%", checkPattern(8, aExpectedPattern9));
    CPPUNIT_ASSERT_MESSAGE("Pattern10 - 75%", checkPattern(9, aExpectedPattern10));
    CPPUNIT_ASSERT_MESSAGE("Pattern11 - 80%", checkPattern(10, aExpectedPattern11));
    CPPUNIT_ASSERT_MESSAGE("Pattern12 - 90%", checkPattern(11, aExpectedPattern12));

    CPPUNIT_ASSERT_MESSAGE("Pattern13 - Light downward diagonal",
                           checkPattern(12, aExpectedPatternLine1));
    CPPUNIT_ASSERT_MESSAGE("Pattern14 - Light upward diagonal",
                           checkPattern(13, aExpectedPatternLine2));
    CPPUNIT_ASSERT_MESSAGE("Pattern15 - Dark downward diagonal",
                           checkPattern(14, aExpectedPatternLine3));
    CPPUNIT_ASSERT_MESSAGE("Pattern16 - Dark upward diagonal",
                           checkPattern(15, aExpectedPatternLine4));
    CPPUNIT_ASSERT_MESSAGE("Pattern17 - Wide downward diagonal",
                           checkPattern(16, aExpectedPatternLine5));
    CPPUNIT_ASSERT_MESSAGE("Pattern18 - Wide upward diagonal",
                           checkPattern(17, aExpectedPatternLine6));

    CPPUNIT_ASSERT_MESSAGE("Pattern19 - Light vertical", checkPattern(18, aExpectedPatternLine7));
    CPPUNIT_ASSERT_MESSAGE("Pattern20 - Light horizontal", checkPattern(19, aExpectedPatternLine8));
    CPPUNIT_ASSERT_MESSAGE("Pattern21 - Narrow vertical", checkPattern(20, aExpectedPatternLine9));
    CPPUNIT_ASSERT_MESSAGE("Pattern22 - Narrow horizontal",
                           checkPattern(21, aExpectedPatternLine10));
    CPPUNIT_ASSERT_MESSAGE("Pattern23 - Dark vertical", checkPattern(22, aExpectedPatternLine11));
    CPPUNIT_ASSERT_MESSAGE("Pattern24 - Dark horizontal", checkPattern(23, aExpectedPatternLine12));

    // TODO: other patterns in the test document
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf100926)
{
    createSdImpressDoc("pptx/tdf100926.pptx");
    const SdrPage* pPage = GetPage(1);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf89064)
{
    createSdImpressDoc("pptx/tdf89064.pptx");
    uno::Reference<presentation::XPresentationPage> xPage(getPage(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xNotesPage(xPage->getNotesPage(), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xNotesPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf108925)
{
    // Test document contains bulleting with too small bullet size (1%) which breaks the lower constraint
    // So it should be converted to the lowest allowed value (25%).

    createSdImpressDoc("odp/tdf108925.odp");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();

    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(25), pNumFmt->GetNumRule().GetLevel(0).GetBulletRelSize());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf109067)
{
    createSdImpressDoc("pptx/tdf109067.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    awt::Gradient gradient;
    CPPUNIT_ASSERT(xShape->getPropertyValue("FillGradient") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(450), gradient.Angle);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf109187)
{
    createSdImpressDoc("pptx/tdf109187.pptx");
    uno::Reference<beans::XPropertySet> xArrow1(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    awt::Gradient aGradient1;
    CPPUNIT_ASSERT(xArrow1->getPropertyValue("FillGradient") >>= aGradient1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2250), aGradient1.Angle);
    uno::Reference<beans::XPropertySet> xArrow2(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
    awt::Gradient aGradient2;
    CPPUNIT_ASSERT(xArrow2->getPropertyValue("FillGradient") >>= aGradient2);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1350), aGradient2.Angle);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf108926)
{
    createSdImpressDoc("pptx/tdf108926.ppt");
    uno::Reference<presentation::XPresentationPage> xPage(getPage(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xNotesPage(xPage->getNotesPage(), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xNotesPage->getCount());

    // Second object should be imported as an empty presentation shape
    uno::Reference<beans::XPropertySet> xPresentationShape(xNotesPage->getByIndex(1),
                                                           uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPresentationShape.is());
    bool bIsEmptyPresObject = false;
    xPresentationShape->getPropertyValue("IsEmptyPresentationObject") >>= bIsEmptyPresObject;
    CPPUNIT_ASSERT(bIsEmptyPresObject);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf100065)
{
    createSdImpressDoc("pptx/tdf100065.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape1(getShapeFromPage(0, 0),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape1(xGroupShape1->getByIndex(1), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle1;
    CPPUNIT_ASSERT(xShape1->getPropertyValue("RotateAngle") >>= nAngle1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nAngle1);

    uno::Reference<container::XIndexAccess> xGroupShape2(getShapeFromPage(1, 0),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape2(xGroupShape2->getByIndex(0), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle2;
    CPPUNIT_ASSERT(xShape2->getPropertyValue("RotateAngle") >>= nAngle2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18000), nAngle2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf90626)
{
    createSdImpressDoc("pptx/tdf90626.pptx");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(1));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf138148)
{
    createSdImpressDoc("pptx/tdf138148.pptx");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf114488)
{
    // This doc has two images - one WMF and the other PNG (fallback image).
    // When loading this doc, the WMF image should be preferred over the PNG image.

    createSdDrawDoc("odg/tdf114488.fodg");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    OUString sMimeType(comphelper::GraphicMimeTypeHelper::GetMimeTypeForXGraphic(xGraphic));
    CPPUNIT_ASSERT_EQUAL(OUString("image/x-wmf"), sMimeType);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf134174)
{
    createSdImpressDoc("pptx/tdf134174.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x8b21db), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf134210)
{
    createSdImpressDoc("pptx/tdf134210.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x605741), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf114913)
{
    createSdImpressDoc("pptx/tdf114913.pptx");
    SdrTextObj* pTxtObj = DynCastSdrTextObj(GetPage(1)->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const SvxNumBulletItem* pItem
        = pTxtObj->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0).GetItem(
            EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(tools::Long(692),
                         pItem->GetNumRule().GetLevel(0).GetGraphicSize().getHeight());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf114821)
{
    css::uno::Any aAny;

    createSdImpressDoc("pptx/tdf114821.pptx");

    uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf115394)
{
    createSdImpressDoc("pptx/tdf115394.pptx");
    double fTransitionDuration;

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Slow in MS formats
    SdPage* pPage1 = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    // Medium in MS formats
    SdPage* pPage2 = pDoc->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Fast in MS formats
    SdPage* pPage3 = pDoc->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Custom values
    SdPage* pPage4 = pDoc->GetSdPage(3, PageKind::Standard);
    fTransitionDuration = pPage4->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.25, fTransitionDuration);

    SdPage* pPage5 = pDoc->GetSdPage(4, PageKind::Standard);
    fTransitionDuration = pPage5->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(4.25, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf115394PPT)
{
    createSdImpressDoc("ppt/tdf115394.ppt");
    double fTransitionDuration;

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Fast
    SdPage* pPage1 = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Medium
    SdPage* pPage2 = pDoc->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Slow
    SdPage* pPage3 = pDoc->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf51340)
{
    // Line spacing was not inherited from upper levels (slide layout, master slide)

    createSdImpressDoc("pptx/tdf51340.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf116899)
{
    // This is a PPT created in Impress and roundtripped in PP, the key times become [1, -1] in PP,
    //  a time of -1 (-1000) in PPT means key times have to be distributed evenly between 0 and 1

    createSdImpressDoc("ppt/tdf116899.ppt");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf77747)
{
    createSdImpressDoc("ppt/tdf77747.ppt");
    SdrTextObj* pTxtObj = DynCastSdrTextObj(GetPage(1)->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const SvxNumBulletItem* pNumFmt
        = pTxtObj->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0).GetItem(
            EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's suffix is wrong!", OUString("-"),
                                 pNumFmt->GetNumRule().GetLevel(0).GetSuffix());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's numbering type is wrong!", SVX_NUM_NUMBER_HEBREW,
                                 pNumFmt->GetNumRule().GetLevel(0).GetNumberingType());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf116266)
{
    createSdImpressDoc("odp/tdf116266.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sfx2::LinkManager* rLinkManager = pDoc->GetLinkManager();
    // The document contains one SVG stored as a link.
    CPPUNIT_ASSERT_EQUAL(size_t(1), rLinkManager->GetLinks().size());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testPptCrop)
{
    createSdImpressDoc("ppt/crop.ppt");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/1, /*nPage=*/0));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue("GraphicCrop") >>= aCrop;
    // These were all 0, lazy-loading broke cropping.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Top);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Bottom);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Right);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf149206)
{
    // Check that the image is cropped
    createSdImpressDoc("pptx/tdf149206.pptx");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/0, /*nPage=*/0));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue("GraphicCrop") >>= aCrop;

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Top);

    // Without the fix in place, this test would have failed with
    // - Expected: 5937
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5937), aCrop.Bottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Right);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf149785)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf149785.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf149985)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf149985.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf150770)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf150770.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDoc->getDrawPages()->getCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf120028)
{
    // Check that the text shape has 4 columns.
    createSdImpressDoc("pptx/tdf120028.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
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

    double fTextSclale = 0.0;
    xShape->getPropertyValue("TextFitToSizeScale") >>= fTextSclale;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(92.0, fTextSclale, 1E1);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testDescriptionImport)
{
    createSdImpressDoc("pptx/altdescription.pptx");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/2, /*nPage=*/0));
    OUString sDesc;

    xPropertySet->getPropertyValue("Description") >>= sDesc;

    CPPUNIT_ASSERT_EQUAL(OUString("We Can Do It!"), sDesc);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf83247)
{
    auto GetPause = [this](const OUString& sSrc) {
        loadFromFile(sSrc);
        uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPresentationProps(
            xPresentationSupplier->getPresentation(), uno::UNO_QUERY_THROW);

        auto retVal = xPresentationProps->getPropertyValue("Pause");
        return retVal.get<sal_Int32>();
    };

    // 1. Check that presentation:pause attribute is imported correctly
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), GetPause("odp/loopPause10.odp"));

    // 2. ODF compliance: if presentation:pause attribute is absent, it must be treated as 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetPause("odp/loopNoPause.odp"));

    // 3. Import PPT: pause should be 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetPause("ppt/loopNoPause.ppt"));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf47365)
{
    createSdImpressDoc("pptx/loopNoPause.pptx");
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    const bool bEndlessVal = xPresentationProps->getPropertyValue("IsEndless").get<bool>();
    const sal_Int32 nPauseVal = xPresentationProps->getPropertyValue("Pause").get<sal_Int32>();

    // Check that we import "loop" attribute of the presentation, and don't introduce any pauses
    CPPUNIT_ASSERT(bEndlessVal);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPauseVal);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf122899)
{
    // tdf122899 FILEOPEN: ppt: old kind arc from MS Office 97 is broken
    // Error was, that the path coordinates of a mso_sptArc shape were read as sal_Int16
    // although they are unsigned 16 bit. This leads to wrong positions of start and end
    // point and results to a huge shape width in the test document.
    createSdImpressDoc("ppt/tdf122899_Arc_90_to_91_clockwise.ppt");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testOOXTheme)
{
    createSdImpressDoc("pptx/ooxtheme.pptx");

    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBag;
    xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;

    bool bTheme = false;
    for (beans::PropertyValue const& prop : aGrabBag)
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testCropToShape)
{
    createSdImpressDoc("pptx/crop-to-shape.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf127964)
{
    createSdImpressDoc("pptx/tdf127964.pptx");
    {
        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
        auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
        CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
    }

    saveAndReload("impress8");

    {
        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
        auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
        CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf48083)
{
    createSdImpressDoc("ppt/tdf48083.ppt");
    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    const SdrObject* pObj = pPage->GetObj(1);
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
    auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
    CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf106638)
{
    createSdImpressDoc("pptx/tdf106638.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
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

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf128684)
{
    createSdImpressDoc("pptx/tdf128684.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(0, xPage));
    CPPUNIT_ASSERT(xShapeProperties.is());
    // Check text direction.
    sal_Int16 eWritingMode(text::WritingMode2::LR_TB);
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName("WritingMode"))
        xShapeProperties->getPropertyValue("WritingMode") >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::TB_RL90), eWritingMode);
    // Check shape rotation
    sal_Int32 nRotateAngle = 0;
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName("RotateAngle"))
        xShapeProperties->getPropertyValue("RotateAngle") >>= nRotateAngle;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), nRotateAngle);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf113198)
{
    createSdImpressDoc("pptx/tdf113198.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int16 nParaAdjust = -1;
    xShape->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf49856)
{
    createSdImpressDoc("ppt/tdf49856.ppt");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(2).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    const sal_UCS4 aBullet = pNumFmt->GetNumRule().GetLevel(0).GetBulletChar();
    CPPUNIT_ASSERT_EQUAL(OUString("More level 2"), aEdit.GetText(2));
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x2022), aBullet);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testShapeGlowEffectPPTXImpoer)
{
    createSdImpressDoc("pptx/shape-glow-effect.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
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

CPPUNIT_TEST_FIXTURE(SdImportTest2, testShapeBlurPPTXImport)
{
    createSdImpressDoc("pptx/shape-blur-effect.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);

    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("ShadowBlur") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 584200EMU=46pt - 139700EMU = 388Hmm = 11pt
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testMirroredGraphic)
{
    createSdImpressDoc("pptx/mirrored-graphic.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x4f4955), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf134210CropPosition)
{
    // We are testing crop position of bitmap in custom shapes. We should see only green with proper fix.

    createSdImpressDoc("pptx/crop-position.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x81d41a), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testGreysScaleGraphic)
{
    createSdImpressDoc("pptx/greysscale-graphic.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(Color(0x3c3c3c), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103347)
{
    createSdImpressDoc("pptx/tdf103347.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xNamed1->getName());

    uno::Reference<drawing::XDrawPage> xPage2(xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed2(xPage2, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello (2)"), xNamed2->getName());

    uno::Reference<drawing::XDrawPage> xPage3(xDoc->getDrawPages()->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed3(xPage3, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello (3)"), xNamed3->getName());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testHyperlinksOnShapes)
{
    createSdImpressDoc("pptx/tdf144616.pptx");

    for (sal_Int32 i = 0; i < 7; i++)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(i, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf112209)
{
    createSdImpressDoc("pptx/tdf112209.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
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
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf128596)
{
    createSdImpressDoc("pptx/tdf128596.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    css::drawing::BitmapMode bitmapmode;
    xShape->getPropertyValue("FillBitmapMode") >>= bitmapmode;
    CPPUNIT_ASSERT_EQUAL(css::drawing::BitmapMode_REPEAT, bitmapmode);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testDefaultTabStop)
{
    createSdImpressDoc("pptx/deftabstop.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    sal_Int32 nDefTab = pDoc->GetDefaultTabulator();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), nDefTab);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testCropToZero)
{
    // Must not crash because of division by zero
    // Also must not fail assertions because of passing negative value to CropQuotientsFromSrcRect
    createSdImpressDoc("pptx/croppedTo0.pptx");
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf144092TableHeight)
{
    createSdImpressDoc("pptx/tdf144092-tableHeight.pptx");

    uno::Reference<drawing::XShape> xTableShape(getShapeFromPage(0, 0), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7606
    // - Actual  : 4595
    // i.e. the table height wasn't corrected by expanding less than minimum sized rows.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7606), xTableShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf89928BlackWhiteThreshold)
{
    // A slide with two graphics, one with color HSV{0,0,74%} and one with HSV{0,0,76%}
    // where both have an applied 75% Black/White Color Effect.

    createSdImpressDoc("pptx/tdf89928-blackWhiteEffectThreshold.pptx");

    // First graphic should appear black
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShape->getPropertyValue("Graphic") >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        Graphic aGraphic(xGraphic);
        BitmapEx aBitmap(aGraphic.GetBitmapEx());

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Color: R:0 G:0 B:0 A:0
        // - Actual  : Color: R:189 G:189 B:189 A:0
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x000000), aBitmap.GetPixelColor(0, 0));
    }

    // Second graphic should appear white
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShape->getPropertyValue("Graphic") >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        Graphic aGraphic(xGraphic);
        BitmapEx aBitmap(aGraphic.GetBitmapEx());

        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xFFFFFF), aBitmap.GetPixelColor(0, 0));
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf151547TransparentWhiteText)
{
    createSdImpressDoc("pptx/tdf151547-transparent-white-text.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:255 G:255 B:254 A:255
    // - Actual  : Color: R:255 G:255 B:255 A:255
    // i.e. fully transparent white text color was interpreted as COL_AUTO
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xFFFFFFFE), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf149961AutofitIndentation)
{
    createSdImpressDoc("pptx/tdf149961-autofitIndentation.pptx");

    const SdrPage* pPage = GetPage(1);

    {
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT(pNumFmt);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(3175), pNumFmt->GetNumRule().GetLevel(0).GetAbsLSpace());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-3175),
                             pNumFmt->GetNumRule().GetLevel(0).GetFirstLineOffset());
    }

    {
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(1));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT(pNumFmt);

        // Spacing doesn't change when it is scaled
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3175), pNumFmt->GetNumRule().GetLevel(0).GetAbsLSpace());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-3175),
                             pNumFmt->GetNumRule().GetLevel(0).GetFirstLineOffset());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf149588TransparentSolidFill)
{
    createSdImpressDoc("pptx/tdf149588_transparentSolidFill.pptx");
    saveAndReload("Impress MS PowerPoint 2007 XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(6, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:99 G:99 B:99 A   51  (T:204)
    // - Actual  : Color: R:99 G:99 B:99 A: 255  (T:  0)
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xCC636363), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testOverflowBehaviorClip)
{
    createSdImpressDoc("odp/style-overflow-behavior-clip.fodp");
    {
        uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 0
        CPPUNIT_ASSERT_EQUAL(true,
                             xPropSet->getPropertyValue("TextClipVerticalOverflow").get<bool>());
    }

    saveAndReload("impress8");
    {
        uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
        CPPUNIT_ASSERT_EQUAL(true,
                             xPropSet->getPropertyValue("TextClipVerticalOverflow").get<bool>());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testIndentDuplication)
{
    createSdImpressDoc("pptx/formatting-bullet-indent.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(2, 0));

    uno::Reference<beans::XPropertySet> const xParagraph1(getParagraphFromShape(0, xShape),
                                                          uno::UNO_QUERY_THROW);
    sal_Int32 nIndent1;
    xParagraph1->getPropertyValue("ParaFirstLineIndent") >>= nIndent1;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2500), nIndent1);

    uno::Reference<beans::XPropertySet> const xParagraph2(getParagraphFromShape(1, xShape),
                                                          uno::UNO_QUERY_THROW);
    sal_Int32 nIndent2;
    xParagraph2->getPropertyValue("ParaFirstLineIndent") >>= nIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nIndent2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, test_srcRect_smallNegBound)
{
    // Given a cropped custom shape, with a srcRect having a small negative value in one of bounds
    createSdImpressDoc("pptx/tdf153008-srcRect-smallNegBound.pptx");

    uno::Reference<graphic::XGraphic> xGraphic(
        getShapeFromPage(0, 0)->getPropertyValue("FillBitmap"), uno::UNO_QUERY_THROW);

    BitmapEx aBitmap(Graphic(xGraphic).GetBitmapEx());

    // Properly cropped bitmap should have black pixels close to left edge, near vertical center.
    // Before the fix, the gear was distorted, and this area was white.
    auto yMiddle = aBitmap.GetSizePixel().Height() / 2;
    auto x5Percent = aBitmap.GetSizePixel().Width() / 20;
    CPPUNIT_ASSERT(aBitmap.GetPixelColor(x5Percent, yMiddle).IsDark());
    // Just in case, check that the corner is bright (it is in fact yellow)
    CPPUNIT_ASSERT(aBitmap.GetPixelColor(0, 0).IsBright());
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf153012)
{
    // Given a chart with a data point with solid fill of "Background 1, Darker 15%" color,
    // where the 'bg1' is mapped in the slide to "dk1", but in the chart to "lt1":
    createSdImpressDoc("pptx/chart_pt_color_bg1.pptx");

    uno::Reference<chart2::XChartDocument> xChart2Doc(
        getShapeFromPage(0, 0)->getPropertyValue("Model"), uno::UNO_QUERY_THROW);

    uno::Reference<chart2::XCoordinateSystemContainer> xCooSysCnt(xChart2Doc->getFirstDiagram(),
                                                                  uno::UNO_QUERY_THROW);

    uno::Reference<chart2::XChartTypeContainer> xCTCnt(xCooSysCnt->getCoordinateSystems()[0],
                                                       uno::UNO_QUERY_THROW);

    uno::Reference<chart2::XDataSeriesContainer> xDSCnt(xCTCnt->getChartTypes()[0],
                                                        uno::UNO_QUERY_THROW);

    uno::Sequence<uno::Reference<chart2::XDataSeries>> aSeriesSeq(xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeriesSeq.getLength());

    css::uno::Reference<css::beans::XPropertySet> xPropSet1(aSeriesSeq[0]->getDataPointByIndex(1),
                                                            uno::UNO_SET_THROW);
    Color aFillColor;
    xPropSet1->getPropertyValue("FillColor") >>= aFillColor;
    // The color must arrive correctly. Without the fix, it would fail:
    // - Expected: rgba[d9d9d9ff]
    // - Actual  : rgba[000000ff]
    CPPUNIT_ASSERT_EQUAL(Color(0xd9d9d9), aFillColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testMasterSlides)
{
    createSdImpressDoc("pptx/master-slides.pptx");
    uno::Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xMasterPages(xMasterPagesSupplier->getMasterPages());
    CPPUNIT_ASSERT(xMasterPages.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), xMasterPages->getCount());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
