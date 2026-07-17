/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "sdmodeltestbase.hxx"
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <CustomAnimationEffect.hxx>
#include <anminfo.hxx>
#include <xmloff/SoundReference.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>
#include <sdtiledrenderingtest.hxx>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <osl/process.h>
#include <unotools/saveopt.hxx>

#include <vcl/scheduler.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <svx/svdotable.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace css;
using namespace css::animations;

class SdExportTest2 : public SdModelTestBase
{
public:
    SdExportTest2()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// The sound fixtures below contain nothing but their sound, so a registered
// entry in the link manager means that sound was recognised as an external
// link, the way the document tracks it.
static bool lcl_hasSoundLink(SdDrawDocument& rDoc)
{
    const sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();
    return pLinkManager && !pLinkManager->GetLinks().empty();
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testSwappedOutImageExport)
{
    // Problem was with the swapped out images, which were not swapped in during export.

    std::vector<TestFilter> vFormat{ TestFilter::ODP, TestFilter::PPTX, TestFilter::PPT };

    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file with one image
        createSdImpressDoc("odp/document_with_two_images.odp");
        const OString sFailedMessage
            = "Failed on filter: " + TestFilterNames.at(vFormat[i]).toUtf8();

        // Export the document and import again for a check
        saveAndReload(vFormat[i]);

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testOOoXMLAnimations)
{
    createSdImpressDoc("sxi/ooo41061-1.sxi");

    // FIXME: Error: unexpected attribute "presentation:preset-property"
    skipValidation();

    save(TestFilter::ODP);

    // the problem was that legacy OOoXML animations were lost if store
    // immediately follows load because they were "converted" async by a timer
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//anim:par[@presentation:node-type='timing-root']", 26);
    // currently getting 52 of these without the fix (depends on timing)
    assertXPath(pXmlDoc, "//anim:par", 223);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testBnc480256)
{
    createSdImpressDoc("pptx/bnc480256.pptx");
    // In the document, there are two tables with table background properties.
    // Make sure colors are set properly for individual cells.

    // TODO: If you are working on improving table background support, expect
    // this unit test to fail. In that case, feel free to change the numbers.

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;
    table::BorderLine2 aBorderLine;

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x9bc3ee), nColor);
    xCell->getPropertyValue(u"LeftBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x5597d3), Color(ColorTransparency, aBorderLine.Color));

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xc6ddff), nColor);
    xCell->getPropertyValue(u"TopBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x5597d3), Color(ColorTransparency, aBorderLine.Color));

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT(pTableObj);
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x6bace6), nColor);
    xCell->getPropertyValue(u"LeftBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0xbecfe6), Color(ColorTransparency, aBorderLine.Color));

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4697e0), nColor);

    // This border should be invisible.
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"BottomBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), sal_Int32(aBorderLine.LineWidth));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testUnknownAttributes)
{
    createSdImpressDoc("unknown-attribute.fodp");

    // FIXME: Error: unexpected attribute "foo:non-existent-att"
    skipValidation();

    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/"
                         "style:style[@style:name='gr1']/"
                         "style:graphic-properties[@foo:non-existent-att='bar']");
    // TODO: if the namespace is *known*, the attribute is not preserved, but that seems to be a pre-existing problem, or maybe it's even intentional?
    //    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/style:graphic-properties[@svg:non-existent-att='blah']");
    // this was on style:graphic-properties on the import, but the export moves it to root node which is OK
    assertXPathNSDef(pXmlDoc, "/office:document-content", "foo", "http://example.com/");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTdf80020)
{
    createSdImpressDoc("odp/tdf80020.odp");
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies
            = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xStyleFamily(
            xStyleFamilies->getByName(u"graphics"_ustr), uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"Test Style"_ustr),
                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"text"_ustr, xStyle->getParentStyle());
        saveAndReload(TestFilter::ODP);
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(u"graphics"_ustr),
                                                        uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"Test Style"_ustr),
                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"text"_ustr, xStyle->getParentStyle());
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTdf128985)
{
    createSdImpressDoc("odp/tdf128985.odp");
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies
            = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xStyleFamily(
            xStyleFamilies->getByName(u"LushGreen"_ustr), uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"outline1"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

        sal_Int16 nWritingMode = 0;
        xPropSet->getPropertyValue(u"WritingMode"_ustr) >>= nWritingMode;
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nWritingMode);

        xPropSet->setPropertyValue(u"WritingMode"_ustr, cpo::uno::Any(text::WritingMode2::LR_TB));

        saveAndReload(TestFilter::ODP);
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"LushGreen"_ustr), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"outline1"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue(u"WritingMode"_ustr) >>= nWritingMode;

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nWritingMode);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testLinkedGraphicRT)
{
    // FIXME: PPTX fails
    std::vector<TestFilter> vFormat{ TestFilter::ODP, TestFilter::PPT };
    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file with one image
        createSdImpressDoc("odp/document_with_linked_graphic.odp");

        // allow link updates so the linked graphic is fetched
        {
            SdXImpressDocument* pXImpressDocument
                = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
            CPPUNIT_ASSERT(pXImpressDocument);
            pXImpressDocument->GetDocShell()->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(
                true);
        }

        // Check if the graphic has been imported correctly (before doing the export/import run)
        {
            static constexpr OString sFailedImportMessage
                = "Failed to correctly import the document"_ostr;
            SdXImpressDocument* pXImpressDocument
                = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
            CPPUNIT_ASSERT(pXImpressDocument);
            SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pDoc != nullptr);
            const SdrPage* pPage = pDoc->GetPage(1);
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pPage != nullptr);
            SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pObject != nullptr);
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pObject->IsLinkedGraphic());

            const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedImportMessage.getStr(), int(GraphicType::Bitmap),
                                         int(rGraphicObj.GetGraphic().GetType()));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedImportMessage.getStr(), sal_uLong(864900),
                                         rGraphicObj.GetGraphic().GetSizeBytes());
        }

        // Save and reload
        saveAndReload(vFormat[i]);

        // Check whether graphic imported well after export
        {
            const OString sFailedMessage
                = "Failed on filter: " + TestFilterNames.at(vFormat[i]).toUtf8();

            SdXImpressDocument* pXImpressDocument
                = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
            CPPUNIT_ASSERT(pXImpressDocument);

            // allow link updates on the reloaded document too
            pXImpressDocument->GetDocShell()->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(
                true);
            SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDoc != nullptr);
            const SdrPage* pPage = pDoc->GetPage(1);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pPage != nullptr);
            SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject != nullptr);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject->IsLinkedGraphic());

            const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(GraphicType::Bitmap),
                                         int(rGraphicObj.GetGraphic().GetType()));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_uLong(864900),
                                         rGraphicObj.GetGraphic().GetSizeBytes());
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTdf79082)
{
    createSdImpressDoc("ppt/tdf79082.ppt");
    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // P1 should have 6 tab stops defined
    assertXPathChildren(
        pXmlDoc, "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops", 6);
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[1]",
                "position", u"0cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[2]",
                "position", u"5.08cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[3]",
                "position", u"10.16cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[4]",
                "position", u"15.24cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[5]",
                "position", u"20.32cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[6]",
                "position", u"25.4cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testImageWithSpecialID)
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.

    std::vector<TestFilter> vFormat{ TestFilter::ODP, TestFilter::PPTX, TestFilter::PPT };
    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file
        createSdImpressDoc("odp/images_with_special_IDs.odp");
        const OString sFailedMessage
            = "Failed on filter: " + TestFilterNames.at(vFormat[i]).toUtf8();
        saveAndReload(vFormat[i]);

        // Check whether graphic was exported well
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTdf62176)
{
    createSdImpressDoc("odp/Tdf62176.odp");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin = 0;
    xShape->getPropertyValue(u"ParaLeftMargin"_ustr) >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent = 0;
    xShape->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    CPPUNIT_ASSERT_EQUAL(u"Hello World"_ustr, xParagraph->getString());

    //Saving and Reloading the file
    saveAndReload(TestFilter::ODP);
    uno::Reference<drawing::XDrawPage> xPage2(getPage(0));
    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage2->getCount());

    uno::Reference<beans::XPropertySet> xShape2(getShape(0, xPage2));
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin2 = 0;
    xShape2->getPropertyValue(u"ParaLeftMargin"_ustr) >>= nParaLeftMargin2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin2);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent2 = 0;
    xShape2->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nParaFirstLineIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent2);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph2(getParagraphFromShape(0, xShape2));
    CPPUNIT_ASSERT_EQUAL(u"Hello World"_ustr, xParagraph2->getString());
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testEmbeddedPdf)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    createSdImpressDoc("odp/embedded-pdf.odp");
    saveAndReload(TestFilter::ODP);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"ReplacementGraphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdf)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/sample.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    const SdrPage* pPage = GetPage(1);

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    const SdrObjGroup* pObjGroup = dynamic_cast<const SdrObjGroup*>(pObj);
    CPPUNIT_ASSERT(pObjGroup);
    // Should have exploded to 7 shapes, would be just 1 if not exploded
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), pObjGroup->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfTextPos)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    // Load a PDF, decompose it into Draw shapes, save as FODG and reload.
    // The key assertion is that the text stays on one line (not split into two).
    loadFromFile(u"pdf/textheight1.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    xmlDocUniquePtr pXml = parseLayout();
    sal_Int32 x = getXPath(pXml, "//textarray[1]", "x").toInt32();
    // was 2028 originally
#if !defined _WIN32
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2003, x, 0);
#else
    // need to check why windows appears to be different
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1985, x, 0);
#endif
    sal_Int32 y = getXPath(pXml, "//textarray[1]", "y").toInt32();
    // was 3092 originally, then 3057; now 3073 with font-independent
    // line spacing set during PDF decomposition
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3073, y, 0);

    // Before fix, on reimport this was split over two lines when it
    // should have remained as one line.
    assertXPath(pXml, "//textarray", 1);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfFineDetail)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    // A: an "ſt" ligature is one narrow pdf glyph that expands to two
    // characters. Its run must stay on a single line, not wrap with the
    // trailing glyph dropped below the box.
    //
    // B: hairline strokes should stay visible.
    loadFromFile(u"pdf/ligature-textbox-fit.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    xmlDocUniquePtr pXml = parseLayout();

    // Three "ſt" runs, each on one line. A wrapped run adds a duplicate at
    // the same x and a lower y, taking the count to six.
    assertXPath(pXml, "//textarray[text='ſt']", 3);

    // The corner crop marks are 0.1pt strokes, thinner than a device pixel.
    // They need to come through as explicit hairlines (stroke width 0),
    // not as sub-pixel widths which the draw layer drops and leaves blank
    // corners.
    xmlDocUniquePtr pExported = parseExportedFile();
    OUString aPathStyle = getXPath(pExported, "//draw:path", "style-name");
    assertXPath(pExported, "//style:style[@style:name='" + aPathStyle.toUtf8()
                               + "']/style:graphic-properties[@svg:stroke-width='0cm']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfFont)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/differentfonts.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    xmlDocUniquePtr pXml = parseLayout();
    {
        OUString sItalic = getXPath(pXml, "//font[2]", "italic");
        // was "none" before
        CPPUNIT_ASSERT_EQUAL(u"normal"_ustr, sItalic);
        // check that the others remain as expected
        OUString sFontName = getXPath(pXml, "//font[2]", "name");
        CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, sFontName);
        int nFontHeight = getXPath(pXml, "//font[2]", "height").toInt32();
        CPPUNIT_ASSERT_EQUAL(494, nFontHeight);
    }
#if !defined _WIN32
    //TODO, debug this
    {
        OUString sWeight = getXPath(pXml, "//font[3]", "weight");
        // was "normal" before
        CPPUNIT_ASSERT_EQUAL(u"bold"_ustr, sWeight);
        // check that the others remain as expected
        OUString sFontName = getXPath(pXml, "//font[3]", "name");
        CPPUNIT_ASSERT_EQUAL(u"Liberation Sans"_ustr, sFontName);
        sal_Int32 nFontHeight = getXPath(pXml, "//font[3]", "height").toInt32();
        CPPUNIT_ASSERT_EQUAL(564, nFontHeight);
    }
#endif
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfHindi)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/BasicHindi.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // Check that the English text in here is correct at least

    // ensure the expected content
    assertXPathContent(pXmlDoc,
                       "/office:document/office:body/office:drawing/draw:page/draw:g/draw:frame[3]/"
                       "draw:text-box/text:p[@text:style-name='P4'][1]",
                       u"FIRST-YEAR HINDI COURSE");

    // ensure the expected font name
    assertXPath(pXmlDoc, "/office:document/office:automatic-styles/style:style[@style:name='P4']/"
                         "style:text-properties[@style:font-name='AcademyEngravedLetPlain']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfGrayscaleImageUnderInvisibleTest)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/GrayscaleImageUnderInvisibleTest.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroupShape.is());

    // first shape in the group is the picture
    uno::Reference<beans::XPropertySet> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2582), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(3325), aBitmap.GetSizePixel().Height());

    Color aExpectedColor(ColorAlphaTag::ColorAlpha, 0xFFFFFFFF);

    // Without the fix in place, this test would have failed with
    // - Expected: rgba[ffffffff]
    // - Actual  : rgba[000000ff]
    CPPUNIT_ASSERT_EQUAL(aExpectedColor, aBitmap.GetPixelColor(5, 5));

    // All the other shape in the group are text in front of that picture
    // but with their pdf text mode as Invisible so it is the picture that
    // is seen and the text is hidden. Test a sample text shape here. Without
    // the fix this test would fail as these shapes were visible.
    uno::Reference<beans::XPropertySet> xTextShape(xGroupShape->getByIndex(10), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextShape.is());
    bool bVisible(true);
    xTextShape->getPropertyValue(u"Visible"_ustr) >>= bVisible;
    CPPUNIT_ASSERT_MESSAGE("Shape should be Invisible", !bVisible);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfClippedImages)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/ClippedImages.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroupShape.is());

    uno::Reference<beans::XPropertySet> xGraphicShape1(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphicShape1.is());
    bool bVisible(true);
    xGraphicShape1->getPropertyValue(u"Visible"_ustr) >>= bVisible;
    CPPUNIT_ASSERT_MESSAGE("1st Graphic should be Visible", bVisible);

    // before the fix the clip for this graphic wasn't taken into account so it was visible
    // now it is detected as entirely clipped out and toggled to invisible
    uno::Reference<beans::XPropertySet> xGraphicShape2(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphicShape2.is());
    xGraphicShape2->getPropertyValue(u"Visible"_ustr) >>= bVisible;
    CPPUNIT_ASSERT_MESSAGE("2nd Graphic should be Invisible", !bVisible);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfMissingFontVersion)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/ErrareHumanumEst.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    setImportFilterName(TestFilter::FODG);
    saveAndReload(TestFilter::FODG);

    const SdrPage* pPage = GetPage(1);

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    const SdrObjGroup* pObjGroup = dynamic_cast<const SdrObjGroup*>(pObj);
    CPPUNIT_ASSERT(pObjGroup);
    const SdrTextObj* pTextObj = DynCastSdrTextObj(pObjGroup->GetObj(0));
    OUString sText = pTextObj->GetOutlinerParaObject()->GetTextObject().GetText(0);
    // Without fix this fails to import at all
    CPPUNIT_ASSERT_EQUAL(u"Errare humanum est"_ustr, sText);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfEmbeddedFonts)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/sciencejournalsource.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // The PT Serif embedded font should have been extracted and embedded into the fodg,
    // ensure we have the bold variant
    assertXPath(pXmlDoc, "/office:document/office:font-face-decls/style:font-face[@style:name='PT "
                         "Serif']/svg:font-face-src/svg:font-face-uri[@loext:font-weight='bold' "
                         "and @loext:font-style='normal']/office:binary-data");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfPatternStroke)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/pattern-stroke.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // ensure the stroke color is this redish color, and not gray which is what it
    // defaults to if the stroke pattern isn't taken into account.
    assertXPath(pXmlDoc, "/office:document/office:automatic-styles/style:style[@style:name='gr1']/"
                         "style:graphic-properties[@svg:stroke-color='#ed1b2d']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfPatternFill)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/pattern-fill.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // ensure the stroke color is this redish color, and not gray which is what it
    // defaults to if the stroke pattern isn't taken into account.
    assertXPath(pXmlDoc, "/office:document/office:automatic-styles/style:style[@style:name='gr1']/"
                         "style:graphic-properties[@style:repeat='repeat' and "
                         "@draw:fill-image-width='1.27cm' and @draw:fill-image-height='1.27cm' and "
                         "@draw:fill-image-name='Bitmap_20_1']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testPdfPageMasterOrientation)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/SampleSlideDeck.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // Ensure the page size is landscape. Before fix the master pagesize was
    // portrait Letter so on reimport of the [f]odg the master page size is
    // what is applied to the reloaded pages and was obviously wrong on reload.
    assertXPath(pXmlDoc,
                "/office:document/office:automatic-styles/style:page-layout[@style:name='PM0']/"
                "style:page-layout-properties[@style:print-orientation='landscape']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testExplodedPdfTextShear)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    loadFromFile(u"pdf/textshear.pdf");

    setFilterOptions(u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr);
    save(TestFilter::FODG);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();

    // Ensure the Lato font style is italic, seen as regular before improvement to take
    // text shear into account.
    assertXPath(pXmlDoc,
                "/office:document/office:automatic-styles/style:style[@style:name='P2']/"
                "style:text-properties[@style:font-name='Lato' and @fo:font-style='italic']");
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundLinkDetected)
{
    // A slide-transition sound that references an external file is recognised
    // as a link on load, so it takes part in the link-update decision.
    createSdImpressDoc("odp/transition-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundLinkDetected)
{
    createSdImpressDoc("odp/animation-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundLinkSurvivesODFRoundtrip)
{
    // The transition sound source must come back after an ODF save and reload,
    // not be dropped because the page "Sound" property now carries an object.
    createSdImpressDoc("odp/transition-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::ODP);
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundLinkDetected)
{
    createSdImpressDoc("odp/clickaction-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testSoundLinkAllowedPerLink)
{
    // Allowing one external transition sound through link management marks that
    // page alone allowed and leaves the others blocked.
    createSdImpressDoc("odp/two-sound-links.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();

    SdPage* pPage0 = pDoc->GetSdPage(0, PageKind::Standard);
    SdPage* pPage1 = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pPage0);
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage0->GetSoundLink().isExternalLink());
    CPPUNIT_ASSERT(pPage1->GetSoundLink().isExternalLink());

    // both transition sounds are registered as links, so they appear in link
    // management as two separate entries
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    const sfx2::SvBaseLinks& rLinks = pLinkMgr->GetLinks();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinks.size());

    // neither page is allowed before the user acts
    CPPUNIT_ASSERT(!pPage0->GetSoundLink().isAllowed());
    CPPUNIT_ASSERT(!pPage1->GetSoundLink().isAllowed());

    // updating the link whose source is page 0's sound is the per-link allow
    // action; only page 0 ends up allowed
    for (const tools::SvRef<sfx2::SvBaseLink>& rLink : rLinks)
    {
        OUString aURL;
        sfx2::LinkManager::GetDisplayNames(rLink.get(), nullptr, &aURL);
        if (aURL == pPage0->GetSoundFile())
            rLink->Update();
    }
    CPPUNIT_ASSERT(pPage0->GetSoundLink().isAllowed());
    CPPUNIT_ASSERT(!pPage1->GetSoundLink().isAllowed());
}

static SdAnimationInfo* lcl_getClickActionSoundInfo(SdDrawDocument& rDoc)
{
    for (sal_uInt16 nPage = 0; nPage < rDoc.GetSdPageCount(PageKind::Standard); ++nPage)
    {
        SdPage* pPage = rDoc.GetSdPage(nPage, PageKind::Standard);
        for (size_t nObj = 0; pPage && nObj < pPage->GetObjCount(); ++nObj)
        {
            SdrObject* pObj = pPage->GetObj(nObj);
            SdAnimationInfo* pInfo = pObj ? SdDrawDocument::GetShapeUserData(*pObj) : nullptr;
            if (pInfo && pInfo->meClickAction == css::presentation::ClickAction_SOUND)
                return pInfo;
        }
    }
    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundLinkAllowedPerLink)
{
    // Allowing the click-action sound through link management marks the shape's
    // own SdAnimationInfo allowed, not through a by-URL lookup.
    createSdImpressDoc("odp/clickaction-sound-link.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();

    SdAnimationInfo* pInfo = lcl_getClickActionSoundInfo(*pDoc);
    CPPUNIT_ASSERT(pInfo);

    // the click sound is registered as a link that can be allowed on its own
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!pInfo->mbClickSoundAllowed);

    // updating the link is the per-link allow action
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(pInfo->mbClickSoundAllowed);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundRemoteNotFetched)
{
    // A slide-transition sound pointing outside the document is detected on
    // load, so the link-update infobar is raised. Exporting to PPT without the
    // user allowing the document's links must not fetch the sound. The URL is
    // non-routable, so a fetch attempt would hang this test.
    createSdImpressDoc("odp/transition-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundRemoteNotFetched)
{
    // An animation sound likewise: detected on load, not fetched when
    // exporting without allowing the document's links.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundRemoteNotFetched)
{
    // A click-action sound likewise: detected on load, not fetched when
    // exporting without allowing the document's links.
    createSdImpressDoc("odp/clickaction-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundRemoteNotFetchedOOXML)
{
    // The same must hold on OOXML export, not only binary PPT.
    createSdImpressDoc("odp/transition-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundRemoteNotFetchedOOXML)
{
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPTX);
}

static css::uno::Reference<css::animations::XAnimationNode>
lcl_getPageAnimationNode(const css::uno::Reference<css::lang::XComponent>& xComponent,
                         sal_Int32 nPage)
{
    css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(xComponent,
                                                                    css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::drawing::XDrawPage> xPage(xSupplier->getDrawPages()->getByIndex(nPage),
                                                       css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::animations::XAnimationNodeSupplier> xNodeSupplier(
        xPage, css::uno::UNO_QUERY_THROW);
    return xNodeSupplier->getAnimationNode();
}

static css::uno::Reference<css::animations::XAnimationNode>
lcl_findAudioNode(const css::uno::Reference<css::animations::XAnimationNode>& xNode)
{
    if (!xNode.is())
        return nullptr;
    if (xNode->getType() == css::animations::AnimationNodeType::AUDIO)
        return xNode;
    css::uno::Reference<css::container::XEnumerationAccess> xEnumAccess(xNode, css::uno::UNO_QUERY);
    if (xEnumAccess.is())
    {
        css::uno::Reference<css::container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
        while (xEnum.is() && xEnum->hasMoreElements())
        {
            css::uno::Reference<css::animations::XAnimationNode> xChild(xEnum->nextElement(),
                                                                        css::uno::UNO_QUERY);
            if (auto xFound = lcl_findAudioNode(xChild))
                return xFound;
        }
    }
    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundAllowedForPlayback)
{
    // Allowing the animation-effect sound through link management marks its own
    // audio node's source allowed, so the sound player only plays it once
    // allowed.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    // the external animation sound is registered as a link that can be allowed
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    // updating the link marks the node's own source allowed
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAllowedSoundNotPersisted)
{
    // Allowing an external sound lasts for the session only. After a save and
    // reload the sound is not allowed and its link is registered again, so
    // each session asks the user anew.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    sfx2::LinkManager* pLinkMgr = getSdDocShell()->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());
    pLinkMgr->GetLinks()[0]->Update();
    {
        css::uno::Reference<css::animations::XAudio> xAudio(
            lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAudio.is());
        CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
    }

    saveAndReload(TestFilter::ODP);

    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundInPackageAllowed)
{
    // A sound that lives inside the document package needs no allowing, so its
    // audio node plays without the user acting and it is not a link.
    createSdImpressDoc("odp/animation-sound-package.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));

    // a package sound is not an external reference, so it joins no link
    CPPUNIT_ASSERT(!lcl_hasSoundLink(*pDoc));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundExternalImportPPTX)
{
    // A PPTX animation sound whose relationship points outside the package is
    // imported as an external reference, joins link management like its ODF
    // counterpart, and plays only once allowed. The URL is non-routable, so a
    // fetch attempt would hang this test.
    createSdImpressDoc("pptx/animation-sound-external.pptx");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    // updating the link marks the node's own source allowed
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundEmbeddedImportPPTX)
{
    // A PPTX animation sound embedded in the package becomes package content,
    // so it needs no allowing and is not a link.
    createSdImpressDoc("pptx/animation-sound-embedded.pptx");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
    CPPUNIT_ASSERT(!lcl_hasSoundLink(*pDoc));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testLegacyAnimationSoundNotFetched)
{
    // A legacy animation-effect sound, set through the deprecated shape Sound
    // property the way binary PPT import and the old API populate it, migrates
    // into an audio node whose source is not allowed. So exporting without
    // allowing the links leaves it alone. The URL is non-routable, a fetch
    // attempt would hang.
    createSdImpressDoc();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.TextShape"_ustr), uno::UNO_QUERY_THROW);
    xShapes->add(xShape);

    // an animation effect gives the legacy sound an effect to migrate onto
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    xShapeProps->setPropertyValue(u"Effect"_ustr,
                                  cpo::uno::Any(presentation::AnimationEffect_FADE_FROM_LEFT));
    xShapeProps->setPropertyValue(u"SoundOn"_ustr, cpo::uno::Any(true));
    xShapeProps->setPropertyValue(u"Sound"_ustr,
                                  cpo::uno::Any(u"http://192.0.2.1:12345/animation.wav"_ustr));

    // setting the effect schedules the animation tree on a debounced rebuild
    // timer. Force the export-able representation now, the way the document
    // load path does, so the result does not depend on timer timing.
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    pDoc->GetSdPage(0, PageKind::Standard)->getMainSequence()->getRootNode();

    uno::Reference<css::animations::XAnimationNodeSupplier> xNodeSupplier(xDrawPage,
                                                                          uno::UNO_QUERY_THROW);
    uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(xNodeSupplier->getAnimationNode()), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    // the migrated sound is not allowed, so playback and export leave it alone
    // until the user allows it
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    saveAndReload(TestFilter::PPT);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
