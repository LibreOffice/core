/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <unotest/bootstrapfixturebase.hxx>
#include <unotest/directories.hxx>

#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>

class PDFiumLibraryTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc("/vcl/qa/cppunit/data/") + sFileName;
    }

    void testDocument();
    void testPages();
    void testAnnotationsMadeInEvince();
    void testAnnotationsMadeInAcrobat();

    CPPUNIT_TEST_SUITE(PDFiumLibraryTest);
    CPPUNIT_TEST(testDocument);
    CPPUNIT_TEST(testPages);
    CPPUNIT_TEST(testAnnotationsMadeInEvince);
    CPPUNIT_TEST(testAnnotationsMadeInAcrobat);
    CPPUNIT_TEST_SUITE_END();
};

void PDFiumLibraryTest::testDocument()
{
    OUString aURL = getFullUrl("Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    const void* pData = pVectorGraphicData->getVectorGraphicDataArray().getConstArray();
    int nLength = pVectorGraphicData->getVectorGraphicDataArrayLength();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    CPPUNIT_ASSERT(pPdfium);
    auto pDocument = pPdfium->openDocument(pData, nLength);
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto aSize = pDocument->getPageSize(0);
    CPPUNIT_ASSERT_EQUAL(612.0, aSize.getX());
    CPPUNIT_ASSERT_EQUAL(792.0, aSize.getY());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);
}

void PDFiumLibraryTest::testPages()
{
    OUString aURL = getFullUrl("Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    const void* pData = pVectorGraphicData->getVectorGraphicDataArray().getConstArray();
    int nLength = pVectorGraphicData->getVectorGraphicDataArrayLength();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument = pPdfium->openDocument(pData, nLength);
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);
}

void PDFiumLibraryTest::testAnnotationsMadeInEvince()
{
    OUString aURL = getFullUrl("PangramWithAnnotations.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    const void* pData = pVectorGraphicData->getVectorGraphicDataArray().getConstArray();
    int nLength = pVectorGraphicData->getVectorGraphicDataArrayLength();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument = pPdfium->openDocument(pData, nLength);
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    CPPUNIT_ASSERT_EQUAL(2, pPage->getAnnotationCount());

    {
        auto pAnnotation = pPage->getAnnotation(0);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(1, pAnnotation->getSubType()); // FPDF_ANNOT_TEXT

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("Annotation test"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(16, pPopupAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(16, pAnnotation->getSubType()); // FPDF_ANNOT_POPUP
    }
}

void PDFiumLibraryTest::testAnnotationsMadeInAcrobat()
{
    OUString aURL = getFullUrl("PangramAcrobatAnnotations.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    const void* pData = pVectorGraphicData->getVectorGraphicDataArray().getConstArray();
    int nLength = pVectorGraphicData->getVectorGraphicDataArrayLength();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument = pPdfium->openDocument(pData, nLength);
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    CPPUNIT_ASSERT_EQUAL(4, pPage->getAnnotationCount());

    {
        auto pAnnotation = pPage->getAnnotation(0);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(1, pAnnotation->getSubType()); // FPDF_ANNOT_TEXT

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("YEEEY"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(16, pPopupAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(16, pAnnotation->getSubType()); // FPDF_ANNOT_POPUP
    }

    {
        auto pAnnotation = pPage->getAnnotation(2);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(1, pAnnotation->getSubType()); // FPDF_ANNOT_TEXT

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("Note"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(3, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(16, pPopupAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(3);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(16, pAnnotation->getSubType()); // FPDF_ANNOT_POPUP
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFiumLibraryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
