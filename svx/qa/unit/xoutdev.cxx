/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sal/types.h>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>

using namespace com::sun::star;

class XOutdevTest : public UnoApiTest
{
public:
    XOutdevTest()
        : UnoApiTest("svx/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(XOutdevTest, testPdfGraphicExport)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // Import the graphic.
    Graphic aGraphic;
    OUString aURL = createFileURL(u"graphic.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = maTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "pdf", eFlags);

    // Assert that the output looks like a PDF.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 5);
    sal_uInt8 sFirstBytes[5];
    pStream->ReadBytes(sFirstBytes, 5);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('%'), sFirstBytes[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('D'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('F'), sFirstBytes[3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('-'), sFirstBytes[4]);
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testTdf60684)
{
    Graphic aGraphic;
    OUString aURL = createFileURL(u"tdf60684.jpg");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = maTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "png", eFlags);

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 4);
    sal_uInt8 sFirstBytes[4];
    pStream->ReadBytes(sFirstBytes, 4);

    //Checks if the file's header matches a PNG's expected header
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('N'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('G'), sFirstBytes[3]);
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testFillColorThemeUnoApi)
{
    // Given an empty Impress document with a (title) shape:
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    // When setting the theme index of the shape's fill color:
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    // Set theme color
    {
        model::ThemeColor aThemeColor;
        aThemeColor.setType(model::ThemeColorType::Accent1);
        aThemeColor.addTransformation({ model::TransformationType::LumMod, 2000 });
        aThemeColor.addTransformation({ model::TransformationType::LumOff, 8000 });
        xShape->setPropertyValue("FillColorThemeReference",
                                 uno::Any(model::theme::createXThemeColor(aThemeColor)));
    }

    // Then make sure the value we read back is the expected one:
    {
        uno::Reference<util::XThemeColor> xThemeColor;
        CPPUNIT_ASSERT(xShape->getPropertyValue("FillColorThemeReference") >>= xThemeColor);
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                             aThemeColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), aThemeColor.getTransformations()[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff,
                             aThemeColor.getTransformations()[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), aThemeColor.getTransformations()[1].mnValue);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
