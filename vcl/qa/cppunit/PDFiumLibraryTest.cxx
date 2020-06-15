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

    CPPUNIT_TEST_SUITE(PDFiumLibraryTest);
    CPPUNIT_TEST(testDocument);
    CPPUNIT_TEST(testPages);
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

CPPUNIT_TEST_SUITE_REGISTRATION(PDFiumLibraryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
