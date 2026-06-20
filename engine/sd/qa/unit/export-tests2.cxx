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
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
