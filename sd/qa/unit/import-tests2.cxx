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

#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdooitm.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/graphicmimetype.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/dibtools.hxx>
#include <sdresid.hxx>
#include <stlpool.hxx>
#include <strings.hrc>

using namespace ::com::sun::star;

class SdImportTest2 : public SdModelTestBase
{
public:
    SdImportTest2()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

protected:
    bool checkPattern(int nShapeNumber, std::vector<sal_uInt8>& rExpected);
};

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf157529)
{
    createSdImpressDoc("pptx/tdf157529.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 0));
    CPPUNIT_ASSERT(xShape1.is());
    sal_Int16 nTransparence1;
    xShape1->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence1;

    // Without the fix in place, this test would have failed with
    // Expected: transparence value: 100%
    // Actual  : transparence value: 0%
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), nTransparence1);

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT(xShape2.is());
    sal_Int16 nTransparence2;
    xShape2->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence2;

    // Without the fix in place, this test would have failed with
    // Expected: transparence value: 100%
    // Actual  : transparence value: 0%
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), nTransparence2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf160490)
{
    createSdImpressDoc("pptx/tdf160490.pptx");

    uno::Reference<drawing::XShape> xShape1(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    sal_Int32 nHeight1 = xShape1->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 3726
    // Actual  : placeholder height: 3476
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3726), nHeight1);

    uno::Reference<drawing::XShape> xShape2(getShapeFromPage(0, 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    sal_Int32 nHeight2 = xShape2->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 3365
    // Actual  : placeholder height: 3116
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3365), nHeight2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf165321)
{
    createSdImpressDoc("pptx/tdf165321.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape(getShapeFromPage(0, 0),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape.is());
    // Without the fix in place, this test would have failed with
    // Expected: shape height: 3597
    // Actual  : shape height: 3
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3597), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6592), xShape->getSize().Width);

    xShape.set(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape.is());
    // Without the fix in place, this test would have failed with
    // Expected: shape height: 3597
    // Actual  : shape height: 3
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3597), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6402), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf165341)
{
    createSdImpressDoc("pptx/tdf165341.pptx");

    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xProp(xShape, uno::UNO_QUERY);
    drawing::TextHorizontalAdjust eHori;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eHori);
    drawing::TextVerticalAdjust eVert;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextVerticalAdjust"_ustr) >>= eVert);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust::TextHorizontalAdjust_CENTER, eHori);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust::TextVerticalAdjust_TOP, eVert);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf157285)
{
    createSdImpressDoc("pptx/tdf157285.pptx");

    uno::Reference<drawing::XShape> xShape1(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    sal_Int32 nHeight1 = xShape1->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 2565
    // Actual  : placeholder height: 3435
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2565), nHeight1);

    uno::Reference<drawing::XShape> xShape2(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    sal_Int32 nHeight2 = xShape2->getSize().Height;

    // Without the fix in place, this test would have failed with
    // Expected: placeholder height: 1180
    // Actual  : placeholder height: 11303
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1180), nHeight2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf152186)
{
    createSdImpressDoc("pptx/tdf152186.pptx");
    saveAndReload(TestFilter::PPTX);

    bool bHasShadow = bool();
    const SdrPage* pPage = GetPage(1);
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(i, 0));
        CPPUNIT_ASSERT(xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow);
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
    uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    if (aAny.hasValue())
    {
        uno::Reference<beans::XPropertySet> xBackgroundPropSet;
        aAny >>= xBackgroundPropSet;
        xBackgroundPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
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
    CPPUNIT_ASSERT(xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fCharHeight);
    CPPUNIT_ASSERT_EQUAL(12.f, fCharHeight);

    OUString aCharFontName;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue(u"CharFontName"_ustr) >>= aCharFontName);
    CPPUNIT_ASSERT_EQUAL(u"Stencil"_ustr, aCharFontName);
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
                         xConn1->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn1->getPropertyValue(u"StartShape"_ustr).get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4),
                         xConn1->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage,
        xConn1->getPropertyValue(u"EndShape"_ustr).get<uno::Reference<beans::XPropertySet>>());
    uno::Reference<beans::XPropertySet> const xConn2(getShapeFromPage(3, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         xConn2->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xEllipse,
        xConn2->getPropertyValue(u"StartShape"_ustr).get<uno::Reference<beans::XPropertySet>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),
                         xConn2->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        xImage,
        xConn2->getPropertyValue(u"EndShape"_ustr).get<uno::Reference<beans::XPropertySet>>());
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

        CPPUNIT_ASSERT(xEvents->hasByName(u"OnClick"_ustr));
        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName(u"OnClick"_ustr) >>= props;
        comphelper::SequenceAsHashMap const map(props);
        {
            auto iter(map.find(u"EventType"_ustr));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no EventType").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), u"Presentation"_ustr,
                                         iter->second.get<OUString>());
        }
        {
            auto iter(map.find(u"ClickAction"_ustr));
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
            auto iter(map.find(u"Bookmark"_ustr));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no Bookmark").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), u"http://example.com/"_ustr,
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
    CPPUNIT_ASSERT_EQUAL(u"Click to add Title"_ustr, aEdit.GetText(0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf148685)
{
    createSdImpressDoc("pptx/tdf148685.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    CPPUNIT_ASSERT_EQUAL(u"TEXT "_ustr, xRun->getString());

    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharUnderlineColor;
    xPropSet->getPropertyValue(u"CharUnderlineColor"_ustr) >>= nCharUnderlineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xA1467E), nCharUnderlineColor);

    xRun.set(getRunFromParagraph(1, xParagraph));

    CPPUNIT_ASSERT_EQUAL(u"TE"_ustr, xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue(u"CharUnderlineColor"_ustr) >>= nCharUnderlineColor;

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B:255 A:255
    // - Actual  : Color: R:161 G:70 B:126 A:0
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharUnderlineColor);

    xRun.set(getRunFromParagraph(2, xParagraph));
    CPPUNIT_ASSERT_EQUAL(u"XT"_ustr, xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue(u"CharUnderlineColor"_ustr) >>= nCharUnderlineColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharUnderlineColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf103876)
{
    // Title text shape's placeholder text did not inherit the corresponding text properties

    createSdImpressDoc("pptx/tdf103876.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Check paragraph alignment
    sal_Int16 nParaAdjust = 0;
    xShape->getPropertyValue(u"ParaAdjust"_ustr) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    // Check character color
    Color nCharColor;
    xShape->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf79007)
{
    createSdImpressDoc("pptx/tdf79007.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 0));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape1.is());

    // Check we map mso washout to our watermark
    drawing::ColorMode aColorMode1;
    xShape1->getPropertyValue(u"GraphicColorMode"_ustr) >>= aColorMode1;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_WATERMARK, aColorMode1);

    sal_Int16 nContrast1;
    xShape1->getPropertyValue(u"AdjustContrast"_ustr) >>= nContrast1;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast1);

    sal_Int16 nLuminance1;
    xShape1->getPropertyValue(u"AdjustLuminance"_ustr) >>= nLuminance1;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance1);

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape2.is());

    // Check we map mso grayscale to our grayscale
    drawing::ColorMode aColorMode2;
    xShape2->getPropertyValue(u"GraphicColorMode"_ustr) >>= aColorMode2;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_GREYS, aColorMode2);

    sal_Int16 nContrast2;
    xShape2->getPropertyValue(u"AdjustContrast"_ustr) >>= nContrast2;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast2);

    sal_Int16 nLuminance2;
    xShape2->getPropertyValue(u"AdjustLuminance"_ustr) >>= nLuminance2;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance2);

    uno::Reference<beans::XPropertySet> xShape3(getShapeFromPage(2, 0));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape3.is());

    // Check we map mso black/white to our black/white
    drawing::ColorMode aColorMode3;
    xShape3->getPropertyValue(u"GraphicColorMode"_ustr) >>= aColorMode3;
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_MONO, aColorMode3);

    sal_Int16 nContrast3;
    xShape3->getPropertyValue(u"AdjustContrast"_ustr) >>= nContrast3;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nContrast3);

    sal_Int16 nLuminance3;
    xShape3->getPropertyValue(u"AdjustLuminance"_ustr) >>= nLuminance3;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), nLuminance3);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf119649)
{
    createSdImpressDoc("pptx/tdf119649.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    CPPUNIT_ASSERT_EQUAL(u"default_color("_ustr, xRun->getString());

    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nCharColor);

    xRun.set(getRunFromParagraph(1, xParagraph));

    // Without the fix in place, this test would have failed with
    // - Expected: colored_text
    // - Actual  : colored_text)
    CPPUNIT_ASSERT_EQUAL(u"colored_text"_ustr, xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xCE181E), nCharColor);

    xRun.set(getRunFromParagraph(2, xParagraph));
    CPPUNIT_ASSERT_EQUAL(u")"_ustr, xRun->getString());

    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
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
    xPropSet->getPropertyValue(u"CharTransparence"_ustr) >>= nTransparency;

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
    xPropSet->getPropertyValue(u"CharTransparence"_ustr) >>= nTransparency;

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

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf164640)
{
    createSdImpressDoc();

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    SdStyleSheetPool* const pPool(pDoc->GetSdStyleSheetPool());

    for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
    {
        OString aMsg = "Fails on level " + OString::number(nLevel);

        OUString aStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " " + OUString::number(nLevel));
        SfxStyleSheetBase* pStyleSheet = pPool->Find(aStyleName, SfxStyleFamily::Pseudo);
        CPPUNIT_ASSERT(pStyleSheet);

        const SvxNumBulletItem& rNumFmt = pStyleSheet->GetItemSet().Get(EE_PARA_NUMBULLET);
        sal_UCS4 aBullet1 = rNumFmt.GetNumRule().GetLevel(0).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet1);

        sal_UCS4 aBullet2 = rNumFmt.GetNumRule().GetLevel(1).GetBulletChar();
        // Without the fix in place, this test would have failed with
        // - Expected: 8211
        // - Actual  : 9679
        // - Fails on level 2
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x2013), aBullet2);

        sal_UCS4 aBullet3 = rNumFmt.GetNumRule().GetLevel(2).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet3);

        sal_UCS4 aBullet4 = rNumFmt.GetNumRule().GetLevel(3).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x2013), aBullet4);

        sal_UCS4 aBullet5 = rNumFmt.GetNumRule().GetLevel(4).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet5);

        sal_UCS4 aBullet6 = rNumFmt.GetNumRule().GetLevel(5).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet6);

        sal_UCS4 aBullet7 = rNumFmt.GetNumRule().GetLevel(6).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet7);

        sal_UCS4 aBullet8 = rNumFmt.GetNumRule().GetLevel(7).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet8);

        sal_UCS4 aBullet9 = rNumFmt.GetNumRule().GetLevel(8).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet9);

        sal_UCS4 aBullet10 = rNumFmt.GetNumRule().GetLevel(9).GetBulletChar();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_UCS4(0x25CF), aBullet10);
    }
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
        xPropSet->getPropertyValue(u"NumberingRules"_ustr) >>= xNumRule;
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
        xPropSet->getPropertyValue(u"NumberingRules"_ustr) >>= xNumRule;
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
bool checkPatternValues(std::vector<sal_uInt8>& rExpected, const Bitmap& rBitmap)
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
        uno::Any aBitmapAny = xShape->getPropertyValue(u"FillBitmap"_ustr);
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
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nRotation);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nRotation);

    xCell.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
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
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"FillGradient"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(450), gradient.Angle);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf109187)
{
    createSdImpressDoc("pptx/tdf109187.pptx");
    uno::Reference<beans::XPropertySet> xArrow1(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    awt::Gradient aGradient1;
    CPPUNIT_ASSERT(xArrow1->getPropertyValue(u"FillGradient"_ustr) >>= aGradient1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2250), aGradient1.Angle);
    uno::Reference<beans::XPropertySet> xArrow2(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
    awt::Gradient aGradient2;
    CPPUNIT_ASSERT(xArrow2->getPropertyValue(u"FillGradient"_ustr) >>= aGradient2);
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
    xPresentationShape->getPropertyValue(u"IsEmptyPresentationObject"_ustr) >>= bIsEmptyPresObject;
    CPPUNIT_ASSERT(bIsEmptyPresObject);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf100065)
{
    createSdImpressDoc("pptx/tdf100065.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape1(getShapeFromPage(0, 0),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape1(xGroupShape1->getByIndex(1), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle1;
    CPPUNIT_ASSERT(xShape1->getPropertyValue(u"RotateAngle"_ustr) >>= nAngle1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nAngle1);

    uno::Reference<container::XIndexAccess> xGroupShape2(getShapeFromPage(1, 0),
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape2(xGroupShape2->getByIndex(0), uno::UNO_QUERY_THROW);
    sal_Int32 nAngle2;
    CPPUNIT_ASSERT(xShape2->getPropertyValue(u"RotateAngle"_ustr) >>= nAngle2);
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
    xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    OUString sMimeType(comphelper::GraphicMimeTypeHelper::GetMimeTypeForXGraphic(xGraphic));
    CPPUNIT_ASSERT_EQUAL(u"image/x-wmf"_ustr, sMimeType);
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf134174)
{
    createSdImpressDoc("pptx/tdf134174.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(Color(0x8b21db), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest2, testTdf134210)
{
    createSdImpressDoc("pptx/tdf134210.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
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
    aAny = xPropSet->getPropertyValue(u"Model"_ustr);
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
    const css::uno::Reference<css::beans::XPropertySet> aPropSet0(
        aSeriesSeq[0]->getDataPointByIndex(0));
    CPPUNIT_ASSERT(aPropSet0.is());
    sal_Int32 aPlacement;
    aPropSet0->getPropertyValue(u"LabelPlacement"_ustr) >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(css::chart::DataLabelPlacement::OUTSIDE, aPlacement);

    // Check the second label
    const css::uno::Reference<css::beans::XPropertySet> aPropSet1(
        aSeriesSeq[0]->getDataPointByIndex(1));
    CPPUNIT_ASSERT(aPropSet1.is());
    aPropSet1->getPropertyValue(u"LabelPlacement"_ustr) >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(css::chart::DataLabelPlacement::OUTSIDE, aPlacement);

    // Check the third label
    const css::uno::Reference<css::beans::XPropertySet> aPropSet2(
        aSeriesSeq[0]->getDataPointByIndex(2));
    CPPUNIT_ASSERT(aPropSet2.is());
    aPropSet2->getPropertyValue(u"LabelPlacement"_ustr) >>= aPlacement;
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
