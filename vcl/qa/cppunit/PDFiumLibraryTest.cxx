/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string_view>

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
#include <vcl/pdfread.hxx>
#include <vcl/BitmapReadAccess.hxx>

class PDFiumLibraryTest : public test::BootstrapFixtureBase
{
protected:
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/data/") + sFileName;
    }
};

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testDocument)
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    CPPUNIT_ASSERT(pPdfium);
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto aSize = pDocument->getPageSize(0);
    CPPUNIT_ASSERT_EQUAL(612.0, aSize.getWidth());
    CPPUNIT_ASSERT_EQUAL(792.0, aSize.getHeight());
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testPages)
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testPageObjects)
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    CPPUNIT_ASSERT_EQUAL(12, pPage->getObjectCount());

    auto pPageObject = pPage->getObject(0);
    auto pTextPage = pPage->getTextPage();

    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Text, pPageObject->getType());

    CPPUNIT_ASSERT_EQUAL(u"The quick, brown fox jumps over a lazy dog. DJs flock by when "
                         "MTV ax quiz prog. Junk MTV quiz "_ustr,
                         pPageObject->getText(pTextPage));

    CPPUNIT_ASSERT_EQUAL(12.0, pPageObject->getFontSize());
    CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, pPageObject->getFontName());
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFTextRenderMode::Fill, pPageObject->getTextRenderMode());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getFillColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getStrokeColor());

    basegfx::B2DHomMatrix aMatrix = pPageObject->getMatrix();
    // Ignore translation, ensure there is no rotate/scale.
    aMatrix.set(0, 2, 0);
    aMatrix.set(1, 2, 0);
    CPPUNIT_ASSERT_EQUAL(true, aMatrix.isIdentity());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(057.01, pPageObject->getBounds().getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(721.51, pPageObject->getBounds().getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(539.48, pPageObject->getBounds().getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(732.54, pPageObject->getBounds().getMaxY(), 1E-2);
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testAnnotationsMadeInEvince)
{
    OUString aURL = getFullUrl(u"PangramWithAnnotations.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
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
        CPPUNIT_ASSERT_EQUAL(u"quikee"_ustr, aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Annotation test"_ustr, aContentsString);

        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyPopup));
        auto pPopupAnnotation = pAnnotation->getLinked(vcl::pdf::constDictionaryKeyPopup);
        CPPUNIT_ASSERT(pPopupAnnotation);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationIndex(pPopupAnnotation));
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pPopupAnnotation->getSubType());

        OUString sDateTimeString
            = pAnnotation->getString(vcl::pdf::constDictionaryKeyModificationDate);
        CPPUNIT_ASSERT_EQUAL(u"D:20200612201322+02'00"_ustr, sDateTimeString);
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup, pAnnotation->getSubType());
    }
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testAnnotationsMadeInAcrobat)
{
    OUString aURL = getFullUrl(u"PangramAcrobatAnnotations.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
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
        CPPUNIT_ASSERT_EQUAL(u"quikee"_ustr, aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"YEEEY"_ustr, aContentsString);

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
        CPPUNIT_ASSERT_EQUAL(u"quikee"_ustr, aPopupString);

        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Note"_ustr, aContentsString);

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

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testFormFields)
{
    // Given a document with a form field that looks like plain text:
    OUString aURL = getFullUrl(u"form-fields.pdf");
    SvFileStream aFileStream(aURL, StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFileStream);
    aMemory.Seek(0);

    // When rendering its first (and only) page to a bitmap:
    std::vector<BitmapEx> aBitmaps;
    int nRet = vcl::RenderPDFBitmaps(aMemory.GetData(), aMemory.GetSize(), aBitmaps);
    CPPUNIT_ASSERT(nRet);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aBitmaps.size());

    // Then make sure the bitmap contains that text:
    Bitmap aBitmap = aBitmaps[0].GetBitmap();
    BitmapReadAccess aAccess(aBitmap);
    Size aSize = aBitmap.GetSizePixel();
    std::set<sal_uInt32> aColors;
    for (tools::Long y = 0; y < aSize.Height(); ++y)
    {
        for (tools::Long x = 0; x < aSize.Width(); ++x)
        {
            aColors.insert(static_cast<sal_uInt32>(aAccess.GetPixel(y, x)));
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 1
    // - Actual  : 1
    // i.e. at least black text and white background is expected (possibly more, due to
    // anti-aliasing), but nothing was rendered.
    CPPUNIT_ASSERT_GREATER(static_cast<size_t>(1), aColors.size());
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testAnnotationsDifferentTypes)
{
    OUString aURL = getFullUrl(u"PangramWithMultipleTypeOfAnnotations.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    CPPUNIT_ASSERT_EQUAL(6, pPage->getAnnotationCount());

    {
        auto pAnnotation = pPage->getAnnotation(0);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::FreeText, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Inline Note"_ustr, aContentsString);
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(true, aLineGeometry.empty());
    }

    {
        auto pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Ink, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Freehand Text"_ustr, aContentsString);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnotation->getInkStrokes().size());
        auto const aInkStrokes = pAnnotation->getInkStrokes();
        auto const& aPoints = aInkStrokes[0];
        CPPUNIT_ASSERT_EQUAL(size_t(74), aPoints.size());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, pAnnotation->getBorderWidth(), 1E-2);
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(true, aLineGeometry.empty());
    }

    {
        auto pAnnotation = pPage->getAnnotation(2);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Line, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Line Text"_ustr, aContentsString);
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(false, aLineGeometry.empty());
    }

    {
        auto pAnnotation = pPage->getAnnotation(3);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Polygon, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        CPPUNIT_ASSERT_EQUAL(true, pAnnotation->hasKey("Vertices"_ostr));
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Polygon Text"_ustr, aContentsString);
        auto const aVertices = pAnnotation->getVertices();
        CPPUNIT_ASSERT_EQUAL(size_t(3), aVertices.size());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, pAnnotation->getBorderWidth(), 1E-2);
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(true, aLineGeometry.empty());
    }

    {
        auto pAnnotation = pPage->getAnnotation(4);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Circle, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Ellipse Text"_ustr, aContentsString);
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(true, aLineGeometry.empty());
    }

    {
        auto pAnnotation = pPage->getAnnotation(5);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Square, pAnnotation->getSubType());
        CPPUNIT_ASSERT_EQUAL(0, pAnnotation->getObjectCount());
        OUString aContentsString = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
        CPPUNIT_ASSERT_EQUAL(u"Rectangle Text"_ustr, aContentsString);
        CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xE0, 0x00), pAnnotation->getColor());
        CPPUNIT_ASSERT_EQUAL(false, pAnnotation->hasKey(vcl::pdf::constDictionaryKeyInteriorColor));
        auto const aLineGeometry = pAnnotation->getLineGeometry();
        CPPUNIT_ASSERT_EQUAL(true, aLineGeometry.empty());
    }
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testAnnotationsFreeText)
{
    OUString aURL = getFullUrl(u"Annotations_Adobe_FreeText.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);

    std::vector<vcl::PDFGraphicResult> aResults;
    CPPUNIT_ASSERT_EQUAL(size_t(1), vcl::ImportPDFUnloaded(aURL, aResults));

    vcl::PDFGraphicResult& rResult = aResults[0];

    Graphic aGraphic = rResult.GetGraphic();
    aGraphic.makeAvailable();

    OUString aDefaultStyle;
    OUString aRichContent;

    {
        auto pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

        auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

        auto pPdfium = vcl::pdf::PDFiumLibrary::get();
        auto pDocument
            = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
        CPPUNIT_ASSERT(pDocument);

        CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

        auto pPage = pDocument->openPage(0);
        CPPUNIT_ASSERT(pPage);

        CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());

        auto pAnnotation = pPage->getAnnotation(0);
        CPPUNIT_ASSERT(pAnnotation);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::FreeText, pAnnotation->getSubType());

        aDefaultStyle = pAnnotation->getString(vcl::pdf::constDictionaryKey_DefaultStyle);
        CPPUNIT_ASSERT_EQUAL(false, aDefaultStyle.isEmpty());

        aRichContent = pAnnotation->getString(vcl::pdf::constDictionaryKey_RichContent);
        CPPUNIT_ASSERT_EQUAL(false, aRichContent.isEmpty());
    }

    auto const& rAnnotations = rResult.GetAnnotations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rAnnotations.size());

    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::FreeText, rAnnotations[0].meSubType);

    auto* pMarker
        = static_cast<vcl::pdf::PDFAnnotationMarkerFreeText*>(rAnnotations[0].mpMarker.get());

    CPPUNIT_ASSERT_EQUAL(aDefaultStyle, pMarker->maDefaultStyle);
    CPPUNIT_ASSERT_EQUAL(aRichContent, pMarker->maRichContent);
}

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testTools)
{
    OUString sConverted = vcl::pdf::convertPdfDateToISO8601(u"D:20200612201322+02'00");

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

CPPUNIT_TEST_FIXTURE(PDFiumLibraryTest, testStructureTree)
{
    OUString aURL = getFullUrl(u"StructureTreeExampleDocument.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    auto pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    auto& rDataContainer = pVectorGraphicData->getBinaryDataContainer();

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    CPPUNIT_ASSERT(pPdfium);

    auto pDocument
        = pPdfium->openDocument(rDataContainer.getData(), rDataContainer.getSize(), OString());
    CPPUNIT_ASSERT(pDocument);

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getPageCount());

    auto pPage = pDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    auto pTree = pPage->getStructureTree();
    CPPUNIT_ASSERT(pTree);
    CPPUNIT_ASSERT_EQUAL(1, pTree->getNumberOfChildren());

    // Check the structure
    {
        auto pChildDocument = pTree->getChild(0);
        CPPUNIT_ASSERT(pChildDocument);
        CPPUNIT_ASSERT_EQUAL(5, pChildDocument->getNumberOfChildren());

        CPPUNIT_ASSERT_EQUAL(u""_ustr, pChildDocument->getAltText());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pChildDocument->getActualText());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pChildDocument->getID());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pChildDocument->getLang());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pChildDocument->getTitle());
        CPPUNIT_ASSERT_EQUAL(u"Document"_ustr, pChildDocument->getType());
        CPPUNIT_ASSERT_EQUAL(u"StructElem"_ustr, pChildDocument->getObjectType());

        {
            auto pThis = pChildDocument->getChild(0);
            CPPUNIT_ASSERT(pThis);
            CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pThis->getType());
            CPPUNIT_ASSERT_EQUAL(1, pThis->getNumberOfChildren());
            CPPUNIT_ASSERT_EQUAL(0, pThis->getChildMarkedContentID(0));
        }

        {
            auto pThis = pChildDocument->getChild(1);
            CPPUNIT_ASSERT(pThis);
            CPPUNIT_ASSERT_EQUAL(u"H1"_ustr, pThis->getType());
            CPPUNIT_ASSERT_EQUAL(2, pThis->getNumberOfChildren());
            CPPUNIT_ASSERT_EQUAL(1, pThis->getChildMarkedContentID(0));
            CPPUNIT_ASSERT_EQUAL(2, pThis->getChildMarkedContentID(1));
        }

        {
            auto pThis = pChildDocument->getChild(2);
            CPPUNIT_ASSERT(pThis);
            CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pThis->getType());
            CPPUNIT_ASSERT_EQUAL(13, pThis->getNumberOfChildren());
            CPPUNIT_ASSERT_EQUAL(3, pThis->getChildMarkedContentID(0));
            {
                auto pChild = pThis->getChild(1);
                CPPUNIT_ASSERT_EQUAL(u"Code"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(4, pChild->getChildMarkedContentID(0));

                // Check getParent
                auto pThis2 = pChild->getParent();
                CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pThis2->getType());
                CPPUNIT_ASSERT_EQUAL(13, pThis2->getNumberOfChildren());
            }
            CPPUNIT_ASSERT_EQUAL(5, pThis->getChildMarkedContentID(2));
            CPPUNIT_ASSERT_EQUAL(6, pThis->getChildMarkedContentID(3));
            {
                auto pChild = pThis->getChild(4);
                CPPUNIT_ASSERT_EQUAL(u"Span"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(7, pChild->getChildMarkedContentID(0));
            }
            CPPUNIT_ASSERT_EQUAL(8, pThis->getChildMarkedContentID(5));
            CPPUNIT_ASSERT_EQUAL(9, pThis->getChildMarkedContentID(6));
            {
                auto pChild = pThis->getChild(7);
                CPPUNIT_ASSERT_EQUAL(u"Span"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(10, pChild->getChildMarkedContentID(0));
            }
            CPPUNIT_ASSERT_EQUAL(11, pThis->getChildMarkedContentID(8));
            CPPUNIT_ASSERT_EQUAL(12, pThis->getChildMarkedContentID(9));
            {
                auto pChild = pThis->getChild(10);
                CPPUNIT_ASSERT_EQUAL(u"Span"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(13, pChild->getChildMarkedContentID(0));
            }
            CPPUNIT_ASSERT_EQUAL(14, pThis->getChildMarkedContentID(11));
            {
                auto pChild = pThis->getChild(12);
                CPPUNIT_ASSERT_EQUAL(u"Span"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(15, pChild->getChildMarkedContentID(0));
            }
        }

        {
            auto pThis = pChildDocument->getChild(3);
            CPPUNIT_ASSERT(pThis);
            CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pThis->getType());
            CPPUNIT_ASSERT_EQUAL(4, pThis->getNumberOfChildren());
            CPPUNIT_ASSERT_EQUAL(16, pThis->getChildMarkedContentID(0));
            {
                auto pChild = pThis->getChild(1);
                CPPUNIT_ASSERT_EQUAL(u"Quote"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(17, pChild->getChildMarkedContentID(0));
            }
            CPPUNIT_ASSERT_EQUAL(18, pThis->getChildMarkedContentID(2));
            {
                auto pChild = pThis->getChild(3);
                // Rectangle
                CPPUNIT_ASSERT_EQUAL(u"Div"_ustr, pChild->getType());
                CPPUNIT_ASSERT_EQUAL(u"Only Text! - The Alt Text!"_ustr, pChild->getAltText());
                CPPUNIT_ASSERT_EQUAL(20, pChild->getChildMarkedContentID(0));
                {
                    // Text in rectangle
                    auto pRectangleElement = pChild->getChild(1);
                    CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pRectangleElement->getType());
                    CPPUNIT_ASSERT_EQUAL(21, pRectangleElement->getChildMarkedContentID(0));
                }
            }
        }

        {
            auto pThis = pChildDocument->getChild(4);
            CPPUNIT_ASSERT(pThis);
            CPPUNIT_ASSERT_EQUAL(u"P"_ustr, pThis->getType());
            CPPUNIT_ASSERT_EQUAL(1, pThis->getNumberOfChildren());
            CPPUNIT_ASSERT_EQUAL(19, pThis->getChildMarkedContentID(0));
        }

        {
            auto pThis = pChildDocument->getChild(5);
            CPPUNIT_ASSERT(!pThis);
        }
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
