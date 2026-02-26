/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <tools/color.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>

#include <svx/svdotext.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <animations/animationnodehelper.hxx>

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <sfx2/linkmgr.hxx>
#include <sdresid.hxx>
#include <stlpool.hxx>
#include <strings.hrc>

using namespace ::com::sun::star;

class SdImportTest4 : public SdModelTestBase
{
public:
    SdImportTest4()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf51340)
{
    // Line spacing was not inherited from upper levels (slide layout, master slide)

    createSdImpressDoc("pptx/tdf51340.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

    // First paragraph has a 90% line spacing set on master slide
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    css::style::LineSpacing aSpacing;
    xPropSet->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(90), aSpacing.Height);

    // Second paragraph has a 125% line spacing set on slide layout
    xParagraph.set(getParagraphFromShape(1, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(125), aSpacing.Height);

    // Third paragraph has a 70% line spacing set directly on normal slide (master slide property is overridden)
    xParagraph.set(getParagraphFromShape(2, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(70), aSpacing.Height);

    // Fourth paragraph has a 190% line spacing set directly on normal slide (slide layout property is overridden)
    xParagraph.set(getParagraphFromShape(3, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(css::style::LineSpacingMode::PROP, aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(190), aSpacing.Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf116899)
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

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf77747)
{
    createSdImpressDoc("ppt/tdf77747.ppt");
    SdrTextObj* pTxtObj = DynCastSdrTextObj(GetPage(1)->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const SvxNumBulletItem* pNumFmt
        = pTxtObj->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0).GetItem(
            EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's suffix is wrong!", u"-"_ustr,
                                 pNumFmt->GetNumRule().GetLevel(0).GetSuffix());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's numbering type is wrong!", SVX_NUM_NUMBER_HEBREW,
                                 pNumFmt->GetNumRule().GetLevel(0).GetNumberingType());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf116266)
{
    createSdImpressDoc("odp/tdf116266.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sfx2::LinkManager* rLinkManager = pDoc->GetLinkManager();
    // The document contains one SVG stored as a link.
    CPPUNIT_ASSERT_EQUAL(size_t(1), rLinkManager->GetLinks().size());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testPptCrop)
{
    createSdImpressDoc("ppt/crop.ppt");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/1, /*nPage=*/0));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue(u"GraphicCrop"_ustr) >>= aCrop;
    // These were all 0, lazy-loading broke cropping.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Top);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Bottom);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aCrop.Right);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf149206)
{
    // Check that the image is cropped
    createSdImpressDoc("pptx/tdf149206.pptx");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/0, /*nPage=*/0));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue(u"GraphicCrop"_ustr) >>= aCrop;

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Top);

    // Without the fix in place, this test would have failed with
    // - Expected: 5937
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5937), aCrop.Bottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Right);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testtdf163852)
{
    // Check that the svg image is cropped
    createSdImpressDoc("pptx/tdf163852.pptx");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/1, /*nPage=*/0));
    text::GraphicCrop aCrop;
    xPropertySet->getPropertyValue(u"GraphicCrop"_ustr) >>= aCrop;

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Top);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aCrop.Right);
    // Without the fix in place, this test would have failed with
    // - Expected: 702
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(702), aCrop.Bottom);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf149785)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf149785.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf149985)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf149985.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf166030)
{
    // Without the fix in place, no bullet spacing was applied
    createSdImpressDoc("ppt/tdf166030.ppt");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(1).GetItem(EE_PARA_NUMBULLET);
    const SvxNumberFormat& rFmt = pNumFmt->GetNumRule().GetLevel(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(953), rFmt.GetAbsLSpace());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf150770)
{
    // Without the fix in place, this test would have failed to load the file
    createSdImpressDoc("pptx/tdf150770.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDoc->getDrawPages()->getCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf120028)
{
    // Check that the text shape has 4 columns.
    createSdImpressDoc("pptx/tdf120028.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    uno::Reference<text::XTextColumns> xCols(xShape->getPropertyValue(u"TextColumns"_ustr),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4), xCols->getColumnCount());
    uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(0)),
                         xColProps->getPropertyValue(u"AutomaticDistance"_ustr));

    // Check font size in the shape.
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    double fCharHeight = 0;
    xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fCharHeight;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.5, fCharHeight, 1E-12);

    double fFontScale = 0.0;
    xShape->getPropertyValue(u"TextFitToSizeFontScale"_ustr) >>= fFontScale;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(92.0, fFontScale, 1E1);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testDescriptionImport)
{
    createSdImpressDoc("pptx/altdescription.pptx");

    uno::Reference<beans::XPropertySet> xPropertySet(getShapeFromPage(/*nShape=*/2, /*nPage=*/0));
    OUString sDesc;

    xPropertySet->getPropertyValue(u"Description"_ustr) >>= sDesc;

    CPPUNIT_ASSERT_EQUAL(u"We Can Do It!"_ustr, sDesc);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf83247_odp_loopPause10)
{
    // Check that presentation:pause attribute is imported correctly
    loadFromFile(u"odp/loopPause10.odp");
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    auto retVal = xPresentationProps->getPropertyValue(u"Pause"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), retVal.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf83247_odp_loopNoPause)
{
    // ODF compliance: if presentation:pause attribute is absent, it must be treated as 0
    loadFromFile(u"odp/loopNoPause.odp");
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    auto retVal = xPresentationProps->getPropertyValue(u"Pause"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), retVal.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf83247_ppt_loopNoPause)
{
    // Import PPT: pause should be 0
    loadFromFile(u"ppt/loopNoPause.ppt");
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    auto retVal = xPresentationProps->getPropertyValue(u"Pause"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), retVal.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf47365)
{
    createSdImpressDoc("pptx/loopNoPause.pptx");
    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    const bool bEndlessVal = xPresentationProps->getPropertyValue(u"IsEndless"_ustr).get<bool>();
    const sal_Int32 nPauseVal
        = xPresentationProps->getPropertyValue(u"Pause"_ustr).get<sal_Int32>();

    // Check that we import "loop" attribute of the presentation, and don't introduce any pauses
    CPPUNIT_ASSERT(bEndlessVal);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPauseVal);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf122899)
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

CPPUNIT_TEST_FIXTURE(SdImportTest4, testOOXTheme)
{
    createSdImpressDoc("pptx/ooxtheme.pptx");

    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBag;
    xPropSet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;

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

CPPUNIT_TEST_FIXTURE(SdImportTest4, testCropToShape)
{
    createSdImpressDoc("pptx/crop-to-shape.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShape->getShapeType());
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    css::drawing::FillStyle fillStyle;
    xShapeProps->getPropertyValue(u"FillStyle"_ustr) >>= fillStyle;
    CPPUNIT_ASSERT_EQUAL(css::drawing::FillStyle_BITMAP, fillStyle);
    css::drawing::BitmapMode bitmapmode;
    xShapeProps->getPropertyValue(u"FillBitmapMode"_ustr) >>= bitmapmode;
    CPPUNIT_ASSERT_EQUAL(css::drawing::BitmapMode_STRETCH, bitmapmode);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf127964)
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

    saveAndReload(TestFilter::ODP);

    {
        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
        auto& rFillBackgroundItem = pObj->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND);
        CPPUNIT_ASSERT_EQUAL(true, rFillBackgroundItem.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf48083)
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

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf106638)
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
    xPropSet->getPropertyValue(u"CharFontName"_ustr) >>= aCharFontName;
    CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, aCharFontName);

    // The rest characters that do not use Wingding.
    CPPUNIT_ASSERT(xTextCursor->goRight(45, true));
    xPropSet->getPropertyValue(u"CharFontName"_ustr) >>= aCharFontName;
    CPPUNIT_ASSERT(aCharFontName != "Wingdings");
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf128684)
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
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName(u"WritingMode"_ustr))
        xShapeProperties->getPropertyValue(u"WritingMode"_ustr) >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::TB_RL90), eWritingMode);
    // Check shape rotation
    sal_Int32 nRotateAngle = 0;
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName(u"RotateAngle"_ustr))
        xShapeProperties->getPropertyValue(u"RotateAngle"_ustr) >>= nRotateAngle;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), nRotateAngle);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf113198)
{
    createSdImpressDoc("pptx/tdf113198.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int16 nParaAdjust = -1;
    xShape->getPropertyValue(u"ParaAdjust"_ustr) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf49856)
{
    createSdImpressDoc("ppt/tdf49856.ppt");
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("No text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(2).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    const sal_UCS4 aBullet = pNumFmt->GetNumRule().GetLevel(0).GetBulletChar();
    CPPUNIT_ASSERT_EQUAL(u"More level 2"_ustr, aEdit.GetText(2));
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x2022), aBullet);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testShapeGlowEffectPPTXImpoer)
{
    createSdImpressDoc("pptx/shape-glow-effect.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 139700 EMU = 388.0556 mm/100
    Color nColor;
    xShape->getPropertyValue(u"GlowEffectColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue(u"GlowEffectTransparency"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testShapeTextGlowEffectPPTXImport)
{
    createSdImpressDoc("pptx/shape-text-glow-effect.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"GlowTextEffectRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(706), nRadius); // 20 pt = 706 mm/100
    Color nColor;
    xShape->getPropertyValue(u"GlowTextEffectColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4EA72E), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue(u"GlowTextEffectTransparency"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testShapeBlurPPTXImport)
{
    createSdImpressDoc("pptx/shape-blur-effect.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    bool bHasShadow = false;
    xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);

    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"ShadowBlur"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 584200EMU=46pt - 139700EMU = 388Hmm = 11pt
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testMirroredGraphic)
{
    createSdImpressDoc("pptx/mirrored-graphic.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(Color(0x4f4955), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf134210CropPosition)
{
    // We are testing crop position of bitmap in custom shapes. We should see only green with proper fix.

    createSdImpressDoc("pptx/crop-position.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(Color(0x81d41a), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testGreysScaleGraphic)
{
    createSdImpressDoc("pptx/greysscale-graphic.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(Color(0x3c3c3c), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf103347)
{
    createSdImpressDoc("pptx/tdf103347.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xNamed1->getName());

    uno::Reference<drawing::XDrawPage> xPage2(xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed2(xPage2, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Hello (2)"_ustr, xNamed2->getName());

    uno::Reference<drawing::XDrawPage> xPage3(xDoc->getDrawPages()->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed3(xPage3, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Hello (3)"_ustr, xNamed3->getName());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testHyperlinksOnShapes)
{
    createSdImpressDoc("pptx/tdf144616.pptx");

    for (sal_Int32 i = 0; i < 7; i++)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(i, 0));
        uno::Reference<document::XEventsSupplier> xEventsSupplier(xShape, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xEvents(xEventsSupplier->getEvents());

        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName(u"OnClick"_ustr) >>= props;
        comphelper::SequenceAsHashMap map(props);
        auto iter(map.find(u"ClickAction"_ustr));
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
                auto iter2(map.find(u"Bookmark"_ustr));
                CPPUNIT_ASSERT_EQUAL(u"Second slide"_ustr, iter2->second.get<OUString>());
            }
            break;
            case 5:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_STOPPRESENTATION,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 6:
            {
                auto iter1(map.find(u"Bookmark"_ustr));
                CPPUNIT_ASSERT_EQUAL(u"http://www.example.com/"_ustr,
                                     iter1->second.get<OUString>());
            }
            break;
            default:
                break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf112209)
{
    createSdImpressDoc("pptx/tdf112209.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:132 G:132 B:132 A:0
    // - Actual  : Color: R:21 G:170 B:236 A:0
    // i.e. the image color was blue instead of grey.
    CPPUNIT_ASSERT_EQUAL(Color(0x848484), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf128596)
{
    createSdImpressDoc("pptx/tdf128596.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    css::drawing::BitmapMode bitmapmode;
    xShape->getPropertyValue(u"FillBitmapMode"_ustr) >>= bitmapmode;
    CPPUNIT_ASSERT_EQUAL(css::drawing::BitmapMode_REPEAT, bitmapmode);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf96389_deftabstopISO29500)
{
    createSdImpressDoc("potx/tdf96389_deftabstopISO29500.potx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    sal_Int32 nDefTab = pDoc->GetDefaultTabulator();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), nDefTab);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf96389_deftabstopECMA)
{
    createSdImpressDoc("potx/tdf96389_deftabstopECMA.potx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    sal_Int32 nDefTab = pDoc->GetDefaultTabulator();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), nDefTab);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testCropToZero)
{
    // Must not crash because of division by zero
    // Also must not fail assertions because of passing negative value to CropQuotientsFromSrcRect
    createSdImpressDoc("pptx/croppedTo0.pptx");
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf144092TableHeight)
{
    createSdImpressDoc("pptx/tdf144092-tableHeight.pptx");

    uno::Reference<drawing::XShape> xTableShape(getShapeFromPage(0, 0), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7885
    // - Actual  : 4595
    // i.e. the table height wasn't corrected by expanding less than minimum sized rows.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7885), xTableShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf89928BlackWhiteThreshold)
{
    // A slide with two graphics, one with color HSV{0,0,74%} and one with HSV{0,0,76%}
    // where both have an applied 75% Black/White Color Effect.

    createSdImpressDoc("pptx/tdf89928-blackWhiteEffectThreshold.pptx");

    // First graphic should appear black
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        Graphic aGraphic(xGraphic);
        Bitmap aBitmap(aGraphic.GetBitmap());

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Color: R:0 G:0 B:0 A:0
        // - Actual  : Color: R:189 G:189 B:189 A:0
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x000000), aBitmap.GetPixelColor(0, 0));
    }

    // Second graphic should appear white
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        Graphic aGraphic(xGraphic);
        Bitmap aBitmap(aGraphic.GetBitmap());

        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xFFFFFF), aBitmap.GetPixelColor(0, 0));
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf151547TransparentWhiteText)
{
    createSdImpressDoc("pptx/tdf151547-transparent-white-text.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:255 G:255 B:254 A:255
    // - Actual  : Color: R:255 G:255 B:255 A:255
    // i.e. fully transparent white text color was interpreted as COL_AUTO
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xFFFFFFFE), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf149961AutofitIndentation)
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

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf149588TransparentSolidFill)
{
    createSdImpressDoc("pptx/tdf149588_transparentSolidFill.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(6, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:99 G:99 B:99 A   51  (T:204)
    // - Actual  : Color: R:99 G:99 B:99 A: 255  (T:  0)
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xCC636363), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testOverflowBehaviorClip)
{
    createSdImpressDoc("odp/style-overflow-behavior-clip.fodp");
    {
        uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 0
        CPPUNIT_ASSERT_EQUAL(
            true, xPropSet->getPropertyValue(u"TextClipVerticalOverflow"_ustr).get<bool>());
    }

    saveAndReload(TestFilter::ODP);
    {
        uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
        CPPUNIT_ASSERT_EQUAL(
            true, xPropSet->getPropertyValue(u"TextClipVerticalOverflow"_ustr).get<bool>());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testIndentDuplication)
{
    createSdImpressDoc("pptx/formatting-bullet-indent.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(2, 0));

    uno::Reference<beans::XPropertySet> const xParagraph1(getParagraphFromShape(0, xShape),
                                                          uno::UNO_QUERY_THROW);
    sal_Int32 nIndent1;
    xParagraph1->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nIndent1;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2500), nIndent1);

    uno::Reference<beans::XPropertySet> const xParagraph2(getParagraphFromShape(1, xShape),
                                                          uno::UNO_QUERY_THROW);
    sal_Int32 nIndent2;
    xParagraph2->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nIndent2);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, test_srcRect_smallNegBound)
{
    // Given a cropped custom shape, with a srcRect having a small negative value in one of bounds
    createSdImpressDoc("pptx/tdf153008-srcRect-smallNegBound.pptx");

    uno::Reference<graphic::XGraphic> xGraphic(
        getShapeFromPage(0, 0)->getPropertyValue(u"FillBitmap"_ustr), uno::UNO_QUERY_THROW);

    Bitmap aBitmap(Graphic(xGraphic).GetBitmap());

    // Properly cropped bitmap should have black pixels close to left edge, near vertical center.
    // Before the fix, the gear was distorted, and this area was white.
    auto yMiddle = aBitmap.GetSizePixel().Height() / 2;
    auto x5Percent = aBitmap.GetSizePixel().Width() / 20;
    CPPUNIT_ASSERT(aBitmap.GetPixelColor(x5Percent, yMiddle).IsDark());
    // Just in case, check that the corner is bright (it is in fact yellow)
    CPPUNIT_ASSERT(aBitmap.GetPixelColor(0, 0).IsBright());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf153012)
{
    // Given a chart with a data point with solid fill of "Background 1, Darker 15%" color,
    // where the 'bg1' is mapped in the slide to "dk1", but in the chart to "lt1":
    createSdImpressDoc("pptx/chart_pt_color_bg1.pptx");

    uno::Reference<chart2::XChartDocument> xChart2Doc(
        getShapeFromPage(0, 0)->getPropertyValue(u"Model"_ustr), uno::UNO_QUERY_THROW);

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
    xPropSet1->getPropertyValue(u"FillColor"_ustr) >>= aFillColor;
    // The color must arrive correctly. Without the fix, it would fail:
    // - Expected: rgba[d9d9d9ff]
    // - Actual  : rgba[000000ff]
    CPPUNIT_ASSERT_EQUAL(Color(0xd9d9d9), aFillColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testMasterSlides)
{
    createSdImpressDoc("pptx/master-slides.pptx");
    uno::Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xMasterPages(xMasterPagesSupplier->getMasterPages());
    CPPUNIT_ASSERT(xMasterPages.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), xMasterPages->getCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf161023)
{
    // Given a shape with three paragraphs (18pt), all directly assigned a smaller font (10pt)
    createSdImpressDoc("odp/tdf161023.fodp");
    auto shape = getShapeFromPage(0, 0);

    // 1st paragraph, not empty
    {
        auto paragraph(getParagraphFromShape(0, shape));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, paragraph->getString());
        auto run(getRunFromParagraph(0, paragraph));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fCharHeight;
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRunFromParagraph(1, paragraph), container::NoSuchElementException);
    }

    // Empty 2nd paragraph, consisting of a single span: this span was treated as "paragraph mark"
    {
        auto paragraph(getParagraphFromShape(1, shape));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, paragraph->getString());
        auto run(getRunFromParagraph(0, paragraph));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fCharHeight;
        // Without the fix, this would fail with
        // - Expected: 10
        // - Actual  : 18
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRunFromParagraph(1, paragraph), container::NoSuchElementException);
    }

    // 3rd paragraph, not empty
    {
        auto paragraph(getParagraphFromShape(2, shape));
        CPPUNIT_ASSERT_EQUAL(u"c"_ustr, paragraph->getString());
        auto run(getRunFromParagraph(0, paragraph));
        CPPUNIT_ASSERT_EQUAL(u"c"_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fCharHeight;
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRunFromParagraph(1, paragraph), container::NoSuchElementException);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf161430)
{
    // Without the bug fix this opens with the classic solid 'blue' background used in "Outline 1"
    // as seen in slide 3
    createSdImpressDoc("odp/tdf161430.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    SdStyleSheetPool* const pPool(pDoc->GetSdStyleSheetPool());

    OUString aStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
    SfxStyleSheetBase* pStyleSheet = pPool->Find(aStyleName, SfxStyleFamily::Pseudo);
    CPPUNIT_ASSERT(pStyleSheet);

    const XFillStyleItem& rFillStyle = pStyleSheet->GetItemSet().Get(XATTR_FILLSTYLE);
    drawing::FillStyle eXFS = rFillStyle.GetValue();
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, eXFS);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, tdf158512)
{
    // First shape on first slide should have no fill to avoid hiding background
    createSdImpressDoc("pptx/tdf158512.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         pPage->GetObj(0)->GetMergedItem(XATTR_FILLSTYLE).GetValue());
    CPPUNIT_ASSERT_EQUAL(false,
                         pPage->GetObj(0)->GetMergedItem(XATTR_FILLUSESLIDEBACKGROUND).GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf169524)
{
    createSdImpressDoc("pptx/tdf169524.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> const xParagraph(getParagraphFromShape(8, xShape),
                                                         uno::UNO_QUERY_THROW);
    sal_Int32 nLeftMargin;
    xParagraph->getPropertyValue(u"ParaLeftMargin"_ustr) >>= nLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nLeftMargin);
}

CPPUNIT_TEST_FIXTURE(SdImportTest4, testTdf168109)
{
    createSdImpressDoc("odp/tdf168109.fodp");

    // Slide 1
    {
        auto xPage = getPage(0).queryThrow<presentation::XPresentationPage>();
        auto xNotesPage = xPage->getNotesPage();
        auto xThumbnail = getShape(0, xNotesPage);
        auto xDescriptor = xThumbnail.queryThrow<drawing::XShapeDescriptor>();

        CPPUNIT_ASSERT_EQUAL(u"com.sun.star.presentation.PageShape"_ustr,
                             xDescriptor->getShapeType());
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(1)),
                             xThumbnail->getPropertyValue(u"PageNumber"_ustr));
    }

    // Slide 2
    {
        auto xPage = getPage(1).queryThrow<presentation::XPresentationPage>();
        auto xNotesPage = xPage->getNotesPage();
        auto xThumbnail = getShape(0, xNotesPage);
        auto xDescriptor = xThumbnail.queryThrow<drawing::XShapeDescriptor>();

        CPPUNIT_ASSERT_EQUAL(u"com.sun.star.presentation.PageShape"_ustr,
                             xDescriptor->getShapeType());
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(2)),
                             xThumbnail->getPropertyValue(u"PageNumber"_ustr));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
