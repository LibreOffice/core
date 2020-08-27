/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

namespace
{
using namespace css;

OUStringLiteral const gaDataUrl = u"/vcl/qa/api/data/";

class XGraphicTest : public test::BootstrapFixture
{
public:
    XGraphicTest()
        : BootstrapFixture(true, false)
    {
    }

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

    void testGraphic();
    void testGraphicDescriptor();
    void testGraphicProvider();

    CPPUNIT_TEST_SUITE(XGraphicTest);
    CPPUNIT_TEST(testGraphic);
    CPPUNIT_TEST(testGraphicDescriptor);
    CPPUNIT_TEST(testGraphicProvider);
    CPPUNIT_TEST_SUITE_END();
};

BitmapEx createBitmap()
{
    Bitmap aBitmap(Size(100, 50), 24);
    aBitmap.Erase(COL_LIGHTRED);

    return BitmapEx(aBitmap);
}

void XGraphicTest::testGraphic()
{
    Graphic aGraphic;
    uno::Reference<graphic::XGraphic> xGraphic = aGraphic.GetXGraphic();
}

void XGraphicTest::testGraphicDescriptor()
{
    Graphic aGraphic(createBitmap());
    uno::Reference<graphic::XGraphic> xGraphic = aGraphic.GetXGraphic();
    uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);

    //[property] byte GraphicType;
    sal_Int8 nType;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("GraphicType") >>= nType);
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, nType);

    //[property] string MimeType;
    OUString sMimeType;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("MimeType") >>= sMimeType);
    CPPUNIT_ASSERT_EQUAL(OUString("image/x-vclgraphic"), sMimeType);

    //[optional, property] ::com::sun::star::awt::Size SizePixel;
    awt::Size aSizePixel;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("SizePixel") >>= aSizePixel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aSizePixel.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aSizePixel.Height);

    //[optional, property] ::com::sun::star::awt::Size Size100thMM;
    awt::Size aSize100thMM;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Size100thMM") >>= aSize100thMM);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSize100thMM.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSize100thMM.Height);

    //[optional, property] byte BitsPerPixel;
    sal_Int8 nBitsPerPixel;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("BitsPerPixel") >>= nBitsPerPixel);
    CPPUNIT_ASSERT_EQUAL(sal_Int8(24), nBitsPerPixel);

    //[optional, property] boolean Transparent;
    bool bTransparent;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Transparent") >>= bTransparent);
    CPPUNIT_ASSERT_EQUAL(false, bTransparent);

    //[optional, property] boolean Alpha;
    bool bAlpha;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Alpha") >>= bAlpha);
    CPPUNIT_ASSERT_EQUAL(false, bAlpha);

    //[optional, property] boolean Animated;
    bool bAnimated;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Animated") >>= bAnimated);
    CPPUNIT_ASSERT_EQUAL(false, bAnimated);
}

void XGraphicTest::testGraphicProvider()
{
    OUString aGraphicURL = getFullUrl("TestGraphic.png");

    { // Load lazy
        uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
        uno::Reference<graphic::XGraphicProvider> xGraphicProvider;
        xGraphicProvider.set(graphic::GraphicProvider::create(xContext), uno::UNO_SET_THROW);

        auto aMediaProperties(comphelper::InitPropertySequence({
            { "URL", uno::makeAny(aGraphicURL) },
            { "LazyRead", uno::makeAny(true) },
            { "LoadAsLink", uno::makeAny(false) },
        }));

        uno::Reference<graphic::XGraphic> xGraphic(
            xGraphicProvider->queryGraphic(aMediaProperties));
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

        uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);

        sal_Int8 nType;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("GraphicType") >>= nType);
        CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, nType);

        awt::Size aSizePixel;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("SizePixel") >>= aSizePixel);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Width);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Height);

        bool bLinked;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Linked") >>= bLinked);
        CPPUNIT_ASSERT_EQUAL(false, bLinked);

        OUString sOriginURL;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("OriginURL") >>= sOriginURL);
        CPPUNIT_ASSERT_EQUAL(OUString(), sOriginURL);

        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    }

    { // Load as link
        uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
        uno::Reference<graphic::XGraphicProvider> xGraphicProvider;
        xGraphicProvider.set(graphic::GraphicProvider::create(xContext), uno::UNO_SET_THROW);

        auto aMediaProperties(comphelper::InitPropertySequence({
            { "URL", uno::makeAny(aGraphicURL) },
            { "LazyRead", uno::makeAny(false) },
            { "LoadAsLink", uno::makeAny(true) },
        }));

        uno::Reference<graphic::XGraphic> xGraphic(
            xGraphicProvider->queryGraphic(aMediaProperties));
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);
        CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

        uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);

        sal_Int8 nType;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("GraphicType") >>= nType);
        CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, nType);

        awt::Size aSizePixel;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("SizePixel") >>= aSizePixel);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Width);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Height);

        bool bLinked;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Linked") >>= bLinked);
        CPPUNIT_ASSERT_EQUAL(true, bLinked);

        OUString sOriginURL;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("OriginURL") >>= sOriginURL);
        CPPUNIT_ASSERT_EQUAL(aGraphicURL, sOriginURL);
    }

    { // Load lazy and as link
        uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
        uno::Reference<graphic::XGraphicProvider> xGraphicProvider;
        xGraphicProvider.set(graphic::GraphicProvider::create(xContext), uno::UNO_SET_THROW);

        auto aMediaProperties(comphelper::InitPropertySequence({
            { "URL", uno::makeAny(aGraphicURL) },
            { "LazyRead", uno::makeAny(true) },
            { "LoadAsLink", uno::makeAny(true) },
        }));

        uno::Reference<graphic::XGraphic> xGraphic(
            xGraphicProvider->queryGraphic(aMediaProperties));
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);

        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

        uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);

        sal_Int8 nType;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("GraphicType") >>= nType);
        CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, nType);

        awt::Size aSizePixel;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("SizePixel") >>= aSizePixel);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Width);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aSizePixel.Height);

        bool bLinked;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Linked") >>= bLinked);
        CPPUNIT_ASSERT_EQUAL(true, bLinked);

        OUString sOriginURL;
        CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("OriginURL") >>= sOriginURL);
        CPPUNIT_ASSERT_EQUAL(aGraphicURL, sOriginURL);

        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(XGraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
