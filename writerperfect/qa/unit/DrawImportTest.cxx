/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

#include <rtl/ref.hxx>

#include "DocumentHandlerForOdg.hxx"
#include "ImportFilter.hxx"
#include "WpftFilterFixture.hxx"
#include "WpftLoader.hxx"
#include "wpftimport.hxx"

namespace
{

namespace uno = css::uno;

class DrawImportFilter : public writerperfect::ImportFilter<OdgGenerator>
{
public:
    explicit DrawImportFilter(const uno::Reference< uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdgGenerator>(rxContext) {}

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString &ServiceName) override;
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() override;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, rtl::OUString &rTypeName) override;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &rDescriptor) override;

    static void generate(librevenge::RVNGDrawingInterface &rDocument);
};

bool DrawImportFilter::doImportDocument(librevenge::RVNGInputStream &, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    DrawImportFilter::generate(rGenerator);
    return true;
}

bool DrawImportFilter::doDetectFormat(librevenge::RVNGInputStream &, rtl::OUString &rTypeName)
{
    rTypeName = "WpftDummyText";
    return true;
}

// XServiceInfo
rtl::OUString SAL_CALL DrawImportFilter::getImplementationName()
{
    return OUString("org.libreoffice.comp.Wpft.QA.DrawImportFilter");
}

sal_Bool SAL_CALL DrawImportFilter::supportsService(const rtl::OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< rtl::OUString > SAL_CALL DrawImportFilter::getSupportedServiceNames()
{
    return {"com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection"};
}

void DrawImportFilter::generate(librevenge::RVNGDrawingInterface &rDocument)
{
    using namespace librevenge;

    rDocument.startDocument(RVNGPropertyList());
    RVNGPropertyList aProps;
    aProps.insert("svg:width", 800);
    aProps.insert("svg:height", 600);
    rDocument.startPage(aProps);
    aProps.clear();
    aProps.insert("svg:x", 50);
    aProps.insert("svg:y", 50);
    aProps.insert("svg:width", 200);
    aProps.insert("svg:height", 100);
    rDocument.startTextObject(aProps);
    rDocument.openParagraph(RVNGPropertyList());
    rDocument.openSpan(RVNGPropertyList());
    rDocument.insertText("My hovercraft is full of eels.");
    rDocument.closeSpan();
    rDocument.closeParagraph();
    rDocument.endTextObject();
    rDocument.endPage();
    rDocument.endDocument();
}

}

namespace
{

class DrawImportTest : public writerperfect::test::WpftFilterFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(DrawImportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void DrawImportTest::test()
{
    using namespace css;

    rtl::Reference<DrawImportFilter> xFilter{new DrawImportFilter(m_xContext)};
    writerperfect::test::WpftLoader aLoader(createDummyInput(), xFilter.get(), "private:factory/sdraw", m_xDesktop, m_xContext);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(aLoader.getDocument(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPages> xPages = xDoc->getDrawPages();
    CPPUNIT_ASSERT(xPages.is());
    auto aPage = xPages->getByIndex(0);
    uno::Reference<beans::XPropertySet> xPageProps;
    CPPUNIT_ASSERT(aPage >>= xPageProps);
    CPPUNIT_ASSERT(xPageProps.is());
    sal_Int32 nProp = 0;
    CPPUNIT_ASSERT(xPageProps->getPropertyValue("Height") >>= nProp);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(600), nProp);
    CPPUNIT_ASSERT(xPageProps->getPropertyValue("Width") >>= nProp);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(800), nProp);
    uno::Reference<drawing::XShapes> xShapes(xPageProps, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());
    auto aShape = xShapes->getByIndex(0);
    uno::Reference<drawing::XShapeDescriptor> xShapeDesc;
    CPPUNIT_ASSERT(aShape >>= xShapeDesc);
    CPPUNIT_ASSERT(xShapeDesc.is());
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("com.sun.star.drawing.TextShape"), xShapeDesc->getShapeType());
    uno::Reference<text::XText> xText(xShapeDesc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("My hovercraft is full of eels."), xText->getString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(DrawImportTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
