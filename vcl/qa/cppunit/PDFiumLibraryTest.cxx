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
#include <unotools/datetime.hxx>

#include <com/sun/star/util/DateTime.hpp>

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
    void testPageObjects();
    void testAnnotationsMadeInEvince();
    void testAnnotationsMadeInAcrobat();
    void testAnnotationsDifferentTypes();
    void testTools();

    CPPUNIT_TEST_SUITE(PDFiumLibraryTest);
    CPPUNIT_TEST(testDocument);
    CPPUNIT_TEST(testPages);
    CPPUNIT_TEST(testPageObjects);
    CPPUNIT_TEST(testAnnotationsMadeInEvince);
    CPPUNIT_TEST(testAnnotationsMadeInAcrobat);
    CPPUNIT_TEST(testAnnotationsDifferentTypes);
    CPPUNIT_TEST(testTools);
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

void PDFiumLibraryTest::testPageObjects()
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

    CPPUNIT_ASSERT_EQUAL(12, pPage->getObjectCount());

    auto pPageObject = pPage->getObject(0);
    auto pTextPage = pPage->getTextPage();

    CPPUNIT_ASSERT_EQUAL(1, pPageObject->getType()); // FPDF_PAGEOBJ_TEXT

    CPPUNIT_ASSERT_EQUAL(OUString("The quick, brown fox jumps over a lazy dog. DJs flock by when "
                                  "MTV ax quiz prog. Junk MTV quiz "),
                         pPageObject->getText(pTextPage));

    CPPUNIT_ASSERT_EQUAL(12.0, pPageObject->getFontSize());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"), pPageObject->getFontName());
    CPPUNIT_ASSERT_EQUAL(0, pPageObject->getTextRenderMode()); // FPDF_TEXTRENDERMODE_FILL
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getFillColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getStrokeColor());

    CPPUNIT_ASSERT_EQUAL(true, pPageObject->getMatrix().isIdentity());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(057.01, pPageObject->getBounds().getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(721.51, pPageObject->getBounds().getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(539.48, pPageObject->getBounds().getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(732.54, pPageObject->getBounds().getMaxY(), 1E-2);
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
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Text, pAnnotation->getSubType());

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("Annotation test"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pPopupAnnotation->getSubType());

        OUString sDateTimeString
            = pAnnotation->getString(vcl::pdf::constDictionaryKeyModificationDate);
        CPPUNIT_ASSERT_EQUAL(OUString("D:20200612201322+02'00"), sDateTimeString);
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pAnnotation->getSubType());
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
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Text, pAnnotation->getSubType());

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("YEEEY"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pPopupAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(2);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Text, pAnnotation->getSubType());

        OUString aPopupString = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
        CPPUNIT_ASSERT_EQUAL(OUString("quikee"), aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(OUString("Note"), aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(3, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pPopupAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(3);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pAnnotation->getSubType());
    }
}

void PDFiumLibraryTest::testAnnotationsDifferentTypes()
{
    OUString aURL = getFullUrl("PangramWithMultipleTypeOfAnnotations.pdf");
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
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::FreeText, pAnnotation->getSubType());
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Line, pAnnotation->getSubType());
    }
}

void PDFiumLibraryTest::testTools()
{
    OUString sConverted = vcl::pdf::convertPdfDateToISO8601("D:20200612201322+02'00");

    css::util::DateTime aDateTime;
    CPPUNIT_ASSERT(utl::ISO8601parseDateTime(sConverted, aDateTime));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2020), aDateTime.Year);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), aDateTime.Month);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), aDateTime.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), aDateTime.Hours);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(13), aDateTime.Minutes);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(22), aDateTime.Seconds);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aDateTime.NanoSeconds);
    CPPUNIT_ASSERT_EQUAL(false, bool(aDateTime.IsUTC));
}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFiumLibraryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
