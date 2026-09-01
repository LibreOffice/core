/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <test/unoapi_test.hxx>
#include <tools/color.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <officecfg/Office/Common.hxx>
#include <test/commontesttools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/unoapi.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>
#include <xmloff/autolayout.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>
#include <SlideSectionManager.hxx>
#include <unomodel.hxx>

using namespace css;
using namespace css::uno;

class SdOOXMLExportTest5 : public SdModelTestBase
{
public:
    SdOOXMLExportTest5()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testDeduplicateMasters)
{
    createSdImpressDoc("pptx/onemaster-twolayouts.pptx");
    saveAndReload(TestFilter::PPTX);

    // Check that the document still has one master and two layouts
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/presentation.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId"_ostr, 1);
    pXmlDocContent = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId"_ostr, 2);

    // Check that both background colors have been preserved
    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDoc->getMasterPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    cpo::uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    Color nColor;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue(u"FillColor"_ustr) >>= nColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x0E2841), nColor);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getMasterPages()->getByIndex(1),
                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet1(xPage1, uno::UNO_QUERY);
    aAny = xPropSet1->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    aAny >>= aXBackgroundPropSet;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue(u"FillColor"_ustr) >>= nColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testInvalidAttributeValuesInSrcRect)
{
    createSdImpressDoc("odp/invalidAttributeValuesInSrcRect.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 24
    // - validation error in OOXML export: Errors: 24
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testInvalidBuAutoNumEnumValue)
{
    createSdImpressDoc("odp/invalidBuAutoNumEnumValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testInvalidCxAndCyAttribute)
{
    createSdImpressDoc("odp/invalidCxAndCyAttribute.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testBadPercentageValue)
{
    createSdImpressDoc("odp/badPercentageValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testInvalidPrstDashEnumValue)
{
    createSdImpressDoc("odp/invalidPrstDashEnumValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 113
    // - validation error in OOXML export: Errors: 113
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testConvertWithMasterDeduplication)
{
    createSdImpressDoc("odp/dupmastermultlayouts.odp");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // For each slide check that it's layout exists
    for (int i = 1; i <= 4; ++i)
    {
        xmlDocUniquePtr pXmlDocRels
            = parseExport("ppt/slides/_rels/slide" + OUString::number(i) + ".xml.rels");

        assertXPath(
            pXmlDocRels,
            "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])"_ostr);
        // the relative target e.g. "../slideLayouts/slideLayout2.xml"
        OUString sRelativeLayoutPath = getXPathContent(
            pXmlDocRels,
            "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])/@Target"_ostr);

        // Check that the referenced slideLayout files exist
        // Without the accompanying fix in place, this test would have failed with:
        // equality assertion failed
        // - Expected: 1
        // - Actual  : 0
        // i.e. the referenced slideLayout file was missing on export.
        OUString sSlideLayoutName = sRelativeLayoutPath.getToken(2, '/');
        CPPUNIT_ASSERT_EQUAL(true,
                             xNameAccess->hasByName("ppt/slideLayouts/" + sSlideLayoutName));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTdf162571HorzAnchor)
{
    createSdImpressDoc("pptx/tdf165261.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xProp(xShape, uno::UNO_QUERY);

    drawing::TextHorizontalAdjust eHori;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eHori);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust::TextHorizontalAdjust_LEFT, eHori);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTdf165261HorzAnchor)
{
    createSdImpressDoc("pptx/tdf165261.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xProp(xShape, uno::UNO_QUERY);
    drawing::TextHorizontalAdjust eHori;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eHori);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust::TextHorizontalAdjust_CENTER, eHori);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCustomPromptTexts)
{
    createSdImpressDoc("pptx/tdf163239.pptx");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage1 = GetPage(1);
    {
        // subtitle placeholder text
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::Text,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!",
                                     u"Click to edit customized Master Subtitle style"_ustr, aText);

        auto xShapeProps(getShapeFromPage(0, 0));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!",
                                     u"Click to edit customized Master Subtitle style"_ustr, aText);
    }

    {
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(1));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::TitleText,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!", u"Custom Title 1"_ustr, aText);

        auto xShapeProps(getShapeFromPage(1, 0));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!", u"Custom Title 1"_ustr,
                                     aText);
    }

    const SdrPage* pPage2 = GetPage(3);
    {
        // body placeholder text
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage2->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::OutlineText,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!", u"Text placeholder"_ustr, aText);

        auto xShapeProps(getShapeFromPage(0, 1));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!", u"Text placeholder"_ustr,
                                     aText);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testPresObjTextStyles)
{
    createSdImpressDoc("pptx/tdf163239_v2.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    // 1st slide
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"Title Slide"_ustr), uno::UNO_QUERY);
    {
        // Title style
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"title"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(60.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    }
    // 2nd slide
    xStyleFamily.set(xStyleFamilies->getByName(u"Title and Content"_ustr), uno::UNO_QUERY);
    {
        // Title style
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"title"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(44.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline1 style
        xStyle.set(xStyleFamily->getByName(u"outline1"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(28.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline2 style
        xStyle.set(xStyleFamily->getByName(u"outline2"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline5 style
        xStyle.set(xStyleFamily->getByName(u"outline5"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(18.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTextStylesXML)
{
    createSdImpressDoc("pptx/tdf163239_v2.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slideLayouts/slideLayout1.xml"_ustr);

    assertXPath(pXmlDocRels, "/p:sldLayout/p:cSld/p:spTree/p:sp[1]/p:txBody/a:lstStyle/a:lvl1pPr",
                "algn", u"ctr");
    assertXPath(pXmlDocRels,
                "/p:sldLayout/p:cSld/p:spTree/p:sp[1]/p:txBody/a:lstStyle/a:lvl1pPr/a:defRPr", "sz",
                u"6000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTextAlignLeft)
{
    createSdImpressDoc("pptx/presLeftAlign.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr", "algn", u"l");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testSubtitleNoBullets)
{
    createSdImpressDoc("odp/tdf170166.odp");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage1 = GetPage(1);
    {
        // subtitle placeholder object
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::Text,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
        // Without a fix, it will fail with numbering type: SVX_NUM_CHAR_SPECIAL
        CPPUNIT_ASSERT(pNumFmt);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's numbering type is wrong!", SVX_NUM_NUMBER_NONE,
                                     pNumFmt->GetNumRule().GetLevel(0).GetNumberingType());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testOLEObjectAnimationTarget)
{
    createSdImpressDoc("pptx/tdf169088.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString sOleId = getXPath(
        pXmlDoc, "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");
    const OUString sPicId = getXPath(pXmlDoc,
                                     "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/"
                                     "a:graphicData/p:oleObj/p:pic/p:nvPicPr/p:cNvPr",
                                     "id");

    // Check OLE id is different from pic id
    CPPUNIT_ASSERT_MESSAGE("OLE id is same as pic id", sOleId != sPicId);

    // Check animation target spid matches OLE object id
    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt",
                "spid", sOleId);

    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animEffect/"
                "p:cBhvr/p:tgtEl/p:spTgt",
                "spid", sOleId);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testtdf169496_hidden_graphic)
{
    createSdImpressDoc("pptx/tdf169496_hidden_graphic.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Graphic 5 is hidden and Graphic 4 is visible, but their order might change in the XML
    // Without the fix the hidden attribute wasn't exported
    OUString sName1
        = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr", "name");
    OUString sName2
        = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr", "name");
    if (sName1 == "Graphic 5" && sName2 == "Graphic 4")
    {
        OUString aHidden
            = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 3 is supposed to be hidden", true, bHidden);

        assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr",
                               "hidden");
    }
    else if (sName1 == "Graphic 4" && sName2 == "Graphic 5")
    {
        assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr",
                               "hidden");

        OUString aHidden
            = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 4 is supposed to be hidden", true, bHidden);
    }
    else
        CPPUNIT_FAIL("Names of graphics is incorrect");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testHiddenGroupShape)
{
    createSdImpressDoc("pptx/hidden_group_shape.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:grpSp/p:nvGrpSpPr/p:cNvPr", "hidden", u"1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testtdf169825_vertical_layouts)
{
    createSdImpressDoc("pptx/tdf169825_vertical_layouts.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the SlidePersist::getLayoutFromValueToken() part of the patch,
    // this and the next layout types would both be exported as blank
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTx");

    pXmlDocRels = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    sLayoutRelRelative = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTx");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testtdf169825_vertical_layouts_from_scratch)
{
    createSdImpressDoc();

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xPages = xDoc->getDrawPages();
    uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPageSet(xPage, uno::UNO_QUERY_THROW);
    xPageSet->setPropertyValue(
        u"Layout"_ustr,
        cpo::uno::Any(static_cast<sal_Int32>(AutoLayout::AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT)));

    uno::Reference<drawing::XDrawPage> xPage2(xPages->insertNewByIndex(1), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPageSet2(xPage2, uno::UNO_QUERY_THROW);
    xPageSet2->setPropertyValue(
        u"Layout"_ustr, cpo::uno::Any(static_cast<sal_Int32>(AutoLayout::AUTOLAYOUT_VTITLE_VCONTENT)));
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "objTx"
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTxOverChart");

    pXmlDocRels = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    sLayoutRelRelative = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "picTx"
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTx");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testtdf169825_layout_type)
{
    createSdImpressDoc("odp/tdf169825_layout_type.odp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "vertTitleAndTxOverChart"
    // while in the original ODP this was - still possibly bogus - notes layout
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"blank");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testtdf170102_layout_type)
{
    createSdImpressDoc("odp/tdf170102_layout_type.odp");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    sal_Int32 nMPCount = xDoc->getMasterPages()->getCount();
    // test roughly the same thing in document and in XML: no drawing object in master page/layout
    for (sal_Int32 i = 0; i < nMPCount; i++)
    {
        uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(i),
                                                 uno::UNO_QUERY_THROW);
        sal_Int32 nObjCount = xPage->getCount();
        for (sal_Int32 j = 0; j < nObjCount; j++)
        {
            uno::Reference<lang::XServiceInfo> xShapeInfo(xPage->getByIndex(j),
                                                          uno::UNO_QUERY_THROW);
            // without the fix in place there would be a graphic shape placeholder
            CPPUNIT_ASSERT_MESSAGE(
                "Unexpected graphic object shape in exported master page",
                !xShapeInfo->supportsService(u"com.sun.star.drawing.GraphicObjectShape"_ustr));
        }

        xmlDocUniquePtr pXmlDocLayout = parseExport(u"ppt/slideLayouts/slideLayout"_ustr
                                                    + OUString::number(i + 1) + u".xml"_ustr);
        CPPUNIT_ASSERT(pXmlDocLayout);
        // without the fix in place there would be a pic layout element for the placeholder
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Unexpected \"pic\" element in exported PPTX slide layout", 0,
            countXPathNodes(pXmlDocLayout, "/p:sldLayout/p:cSld/p:spTree/p:pic"));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testFooterIdxConsistency)
{
    createSdImpressDoc("pptx/multiplelayoutfooter.pptx");
    save(TestFilter::PPTX);

    // slide1 points to slideLayout2
    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pRelsDoc,
                "/rels:Relationships/rels:Relationship[@Type='http://"
                "schemas.openxmlformats.org/officeDocument/2006/"
                "relationships/slideLayout']",
                "Target", u"../slideLayouts/slideLayout2.xml");

    // And they agree on the idx of the footer:
    xmlDocUniquePtr pSlide = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString aFtrIdx = getXPath(pSlide, "//p:sp/p:nvSpPr/p:nvPr/p:ph", "idx");
    xmlDocUniquePtr pLayout = parseExport(u"ppt/slideLayouts/slideLayout2.xml"_ustr);
    assertXPath(pLayout, "//p:sp/p:nvSpPr/p:nvPr/p:ph", "idx", aFtrIdx);
    // Without the fix it fails with:
    // - Expected: 1
    // - Actual  : 2
    // - In <>, attribute 'idx' of '//p:sp/p:nvSpPr/p:nvPr/p:ph' incorrect value.
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTdf169559)
{
    createSdImpressDoc("pptx/tdf169524.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "/p:sldMaster/p:cSld/p:spTree/p:sp[2]/p:txBody/a:lstStyle/a:lvl1pPr/a:spcAft/a:spcPts",
        "val", u"1701");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testOmitCanvasSlideExport)
{
    createSdImpressDoc("odp/canvas-slide.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);

    // the document has 2 pages - one canvas page, and one normal page
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT(pDoc->HasCanvasPage());

    save(TestFilter::PPTX);

    // Verify that the canvas slide was omitted from the export
    // It should have one master slide, and one slide
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/presentation.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId", 1);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldIdLst/p:sldId", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTdf114443_PPSX)
{
    createSdImpressDoc("ppsx/tdf114443-6.ppsx");
    save(TestFilter::PPSX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    // Without the fix, this would have been:
    //   application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml
    // which means the file loses its slideshow/autoplay nature and PowerPoint rejects it.
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testConnectorShapeAnimationTarget)
{
    createSdImpressDoc("pptx/connector-shape-animations.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString sConnectorId = getXPath(
        pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn", "id");

    const OUString sShapeId = getXPath(
        pXmlDoc, "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");

    // Check connector and shape both have the same ID
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Connector and the shape doesn't have the same ID", sConnectorId,
                                 sShapeId);

    // Check animation spid matches ShapeId
    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt",
                "spid", sShapeId);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testChartExExport)
{
    createSdImpressDoc("pptx/forum-mso-de-138303.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    const OString sPath1 = "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Fallback"_ostr;
    assertXPath(pDoc, sPath1 + "/xdr:sp"_ostr, 0);
    assertXPath(pDoc, sPath1 + "/p:sp"_ostr, 1);
    assertXPath(pDoc, sPath1 + "/p:sp/p:nvSpPr/p:nvPr"_ostr, 1);

    pDoc = parseExport(u"ppt/charts/chartEx1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/cx:chartSpace/c:date1904", 0);
}

// 1. Open PPTX with sections, re-export to PPTX, verify sections preserved
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testSlideSectionsPPTXImport)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section", 3);
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 7);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 2);

    // GUIDs preserved
    for (sal_Int32 i = 1; i <= 3; ++i)
    {
        OUString sId = getXPath(pXmlDoc, sPath + "/p14:section[" + OString::number(i) + "]", "id");
        CPPUNIT_ASSERT(!sId.isEmpty());
        CPPUNIT_ASSERT(sId.startsWith("{"));
    }

    // Slide IDs non-zero
    OUString sSldId = getXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId[1]", "id");
    CPPUNIT_ASSERT(sSldId.toInt32() > 0);
}

// 2. Export PPTX with sections to ODP
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testSlideSectionsPPTXToODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    static constexpr OString sPath
        = "/office:document-content/office:body/office:presentation/loext:section-list"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:section", 3);
    assertXPath(pXmlDoc, sPath + "/loext:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/loext:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/loext:section[3]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/loext:section[1]/loext:section-slide", 4);
    assertXPath(pXmlDoc, sPath + "/loext:section[2]/loext:section-slide", 7);
    assertXPath(pXmlDoc, sPath + "/loext:section[3]/loext:section-slide", 2);
}

// 3. Open ODP with sections (PPTX -> ODP -> reload ODP)
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testSlideSectionsODPImport)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    // Verify the in-memory model loaded from ODP
    auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);
}

// 4. Export ODP with sections to PPTX
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testSlideSectionsODPToPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section", 3);
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 7);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 2);

    // Slide IDs non-zero after ODP round-trip
    OUString sSldId = getXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId[1]", "id");
    CPPUNIT_ASSERT(sSldId.toInt32() > 0);

    // GUIDs survive ODP round-trip (generated if missing)
    for (sal_Int32 i = 1; i <= 3; ++i)
    {
        OUString sId = getXPath(pXmlDoc, sPath + "/p14:section[" + OString::number(i) + "]", "id");
        CPPUNIT_ASSERT(!sId.isEmpty());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16078_placeholderKeepsItsOutline)
{
    // Given a layout whose picture placeholder is clipped to an outline of two <a:path> elements:
    // a frame with two holes, one of them round, and a second path holding a triangle that crosses
    // the round one:
    createSdImpressDoc("pptx/custgeom-placeholder.pptx");
    save(TestFilter::PPTX);

    // The outline is written back as the placeholder's own geometry. Without it the placeholder
    // saved as a plain rectangle, and its fill then hid both holes and what showed through.
    xmlDocUniquePtr pLayout = parseExportedLayoutNamed(u"Picture placeholder, clipped outline");
    OString aPath
        = "//p:sp[p:nvSpPr/p:nvPr/p:ph/@type='pic']/p:spPr/a:custGeom/a:pathLst/a:path"_ostr;
    assertXPath(pLayout, aPath, 1);
    // Three contours in one path, which is what subtracts the holes from the frame - the same three
    // the ODF side writes. The second path is not one of them: it is filled rather than cut out, so
    // the triangle it holds took a piece of the round hole back instead of adding a contour.
    assertXPath(pLayout, aPath + "/a:moveTo", 3);
    assertXPath(pLayout, aPath + "/a:close", 3);
    // Four sides each for the two rectangles, the closing one included.
    assertXPath(pLayout, aPath + "/a:lnTo", 8);
    // What is left of the round hole is still curves, not a polygon flattened out of them.
    assertXPath(pLayout, aPath + "/a:cubicBezTo", 6);
    assertXPath(pLayout, aPath + "/a:cubicBezTo[1]/a:pt", 3);
    // The frame's own corner, in the path's coordinates.
    assertXPath(pLayout, aPath + "/a:moveTo[1]/a:pt", "x", u"0");
    assertXPath(pLayout, aPath + "/a:moveTo[1]/a:pt", "y", u"0");

    // The preset rectangle it used to write would have claimed the whole frame.
    assertXPath(pLayout, "//p:sp[p:nvSpPr/p:nvPr/p:ph/@type='pic']/p:spPr/a:prstGeom", 0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16080_masterKeepsItsPlaceholders)
{
    // Given a deck of one master and eleven layouts, so one Impress master page stands for the
    // whole group:
    createSdImpressDoc("pptx/master-and-eleven-layouts.pptx");
    save(TestFilter::PPTX);

    // The master part carries its own placeholders. Without the fix it held a shape tree with
    // nothing in it, so PowerPoint offered an empty slide master: nothing to inherit from, and
    // an edit meant for every layout had to be repeated on each of them.
    xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    static constexpr OString aTree("/p:sldMaster/p:cSld/p:spTree"_ostr);
    assertXPath(pMaster, aTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='title']", 1);
    assertXPath(pMaster, aTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='dt']", 1);
    assertXPath(pMaster, aTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='ftr']", 1);
    assertXPath(pMaster, aTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='sldNum']", 1);

    // The body comes from the first layout that has one, since the page standing for the group is
    // the Title Slide layout and has none. That layout inherits the master's geometry, so the
    // placeholder lands where the master put it rather than where a layout would.
    assertXPath(pMaster, aTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='body']", 1);
    assertXPath(pMaster, aTree + "/p:sp[p:nvSpPr/p:nvPr/p:ph/@type='body']/p:spPr/a:xfrm/a:off",
                "y", u"1825560");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16080_layoutShapesStayOnTheirLayout)
{
    // Given a deck whose first layout paints the whole slide blue, and a slide on another layout,
    // which is therefore white:
    createSdImpressDoc("pptx/layout-own-background.pptx");
    saveAndReload(TestFilter::PPTX);

    utl::TempFileNamed aPng;
    aPng.EnableKillingFile();
    cpo::uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"PixelWidth"_ustr, sal_Int32(64)),
        comphelper::makePropertyValue(u"PixelHeight"_ustr, sal_Int32(36))
    };
    cpo::uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"URL"_ustr, aPng.GetURL()),
        comphelper::makePropertyValue(u"FilterName"_ustr, u"PNG"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData)
    };
    auto xExporter = drawing::GraphicExportFilter::create(getComponentContext());
    xExporter->setSourceDocument(getPage(0).queryThrow<lang::XComponent>());
    xExporter->filter(aDescriptor);

    SvFileStream aStream(aPng.GetURL(), StreamMode::READ);
    Bitmap aSlide = vcl::PngImageReader(aStream).read();
    CPPUNIT_ASSERT_EQUAL(Size(64, 36), aSlide.GetSizePixel());

    // The slide is still white. An imported master and layout collapse onto one Impress master
    // page, so the page standing for the group holds the first layout's rectangle - and writing
    // that page's shapes into the master part painted it under every slide of the deck. Sample
    // the side margins, which no placeholder of this layout reaches.
    // Without the fix in place, this test would have failed with
    // - Expected: rgba[ffffffff]
    // - Actual  : rgba[0000ffff]
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aSlide.GetPixelColor(2, 18));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aSlide.GetPixelColor(61, 18));

    // The layout it belongs to still paints it, which is what a slide of that layout inherits.
    xmlDocUniquePtr pLayout = parseExportedLayoutNamed(u"Title Slide");
    assertXPath(pLayout,
                "/p:sldLayout/p:cSld/p:spTree/p:sp[p:nvSpPr/p:cNvPr/@name='Blue background']"
                "/p:spPr/a:solidFill/a:srgbClr",
                "val", u"0000FF");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16080_masterKeepsWhatItsLayoutsShare)
{
    // Given a deck of one master that draws a rule of its own across the top of the slide, in
    // segments of colour, and seven layouts that draw nothing:
    createSdImpressDoc("pptx/tdf157740.pptx");
    saveAndReload(TestFilter::PPTX);

    // The rule is on the slide master, so PowerPoint edits it once for every layout. The import
    // gives each of the seven Impress master pages a copy, and the master used to get none.
    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pMaster,
                "/p:sldMaster/p:cSld/p:spTree/p:grpSp[p:nvGrpSpPr/p:cNvPr/@name='Group 1']", 1);

    // It moved there rather than being written seven more times: a layout holds the three
    // placeholders it inherits and nothing else.
    xmlDocUniquePtr pLayout = parseExportedLayoutNamed(u"Title, Content over Content");
    static constexpr OString aLayoutTree("/p:sldLayout/p:cSld/p:spTree"_ostr);
    assertXPath(pLayout, aLayoutTree + "/*[not(self::p:nvGrpSpPr) and not(self::p:grpSpPr)]", 3);
    assertXPath(pLayout, aLayoutTree + "/p:sp/p:nvSpPr/p:nvPr/p:ph", 3);

    // And the slide still shows the rule, wherever it is written
    utl::TempFileNamed aPng;
    aPng.EnableKillingFile();
    cpo::uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"PixelWidth"_ustr, sal_Int32(1920)),
        comphelper::makePropertyValue(u"PixelHeight"_ustr, sal_Int32(1080))
    };
    cpo::uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"URL"_ustr, aPng.GetURL()),
        comphelper::makePropertyValue(u"FilterName"_ustr, u"PNG"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData)
    };
    auto xExporter = drawing::GraphicExportFilter::create(getComponentContext());
    xExporter->setSourceDocument(getPage(0).queryThrow<lang::XComponent>());
    xExporter->filter(aDescriptor);

    SvFileStream aStream(aPng.GetURL(), StreamMode::READ);
    Bitmap aSlide = vcl::PngImageReader(aStream).read();
    CPPUNIT_ASSERT_EQUAL(Size(1920, 1080), aSlide.GetSizePixel());
    // 90EBCD is accent5 of the deck's theme, the fill of the rule's second segment
    CPPUNIT_ASSERT_EQUAL(Color(0x90EBCD), aSlide.GetPixelColor(200, 5));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16082_layoutKeepsSubtitle)
{
    // Given a deck whose Title Slide layout carries a subtitle placeholder:
    createSdImpressDoc("pptx/master-and-eleven-layouts.pptx");
    save(TestFilter::PPTX);

    // The layout keeps it. Without the fix every layout of a master holding more than one lost its
    // subtitle, so a slide made from the layout afterwards had nowhere to put one.
    xmlDocUniquePtr pLayout = parseExportedLayoutNamed(u"Title Slide");
    assertXPath(pLayout, "/p:sldLayout/p:cSld/p:spTree/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='subTitle']",
                1);

    // The slide master still gets none, which is what PowerPoint refuses the file over.
    xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pMaster, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:nvSpPr/p:nvPr/p:ph[@type='subTitle']",
                0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testCool16079_dateTimeField)
{
    // Given a deck whose date placeholders hold an automatically updated date:
    createSdImpressDoc("pptx/master-and-eleven-layouts.pptx");
    save(TestFilter::PPTX);

    // The placeholder keeps a field. Without the fix it held our own display string as literal
    // text, so PowerPoint showed "<date/time>" where the original showed a date. The type is the
    // one that leaves the format to the reader, which is what a field holding none asks for - the
    // alias PowerPoint writes here stands for a fixed MM/DD/YYYY.
    xmlDocUniquePtr pLayout = parseExportedLayoutNamed(u"Title Slide");
    assertXPath(pLayout, "//p:sp[p:nvSpPr/p:nvPr/p:ph/@type='dt']/p:txBody/a:p/a:fld", "type",
                u"datetime");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest5, testTdf166401_textGivenToAPicturePlaceholder)
{
    // Given a slide whose picture placeholder is given text without any editing, which is what a
    // script does and what leaves the placeholder standing for itself rather than for the text:
    createSdImpressDoc("pptx/picture-placeholder-custom-prompt.pptx");
    getShapeFromPage(0, 0).queryThrow<text::XTextRange>()->setString(
        u"Given to a picture placeholder"_ustr);
    save(TestFilter::PPTX);

    // The placeholder is written with its text. It holds no image, so writing it as a picture wrote
    // nothing at all and the text went with the shape.
    xmlDocUniquePtr pSlide = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString aPlaceholder(
        "/p:sld/p:cSld/p:spTree/p:sp[p:nvSpPr/p:nvPr/p:ph/@type='pic']"_ostr);
    assertXPath(pSlide, aPlaceholder, 1);
    assertXPathContent(pSlide, aPlaceholder + "/p:txBody/a:p/a:r/a:t",
                       u"Given to a picture placeholder");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
