/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

#include <rtl/ref.hxx>

#include <DocumentHandlerForOdt.hxx>
#include <ImportFilter.hxx>
#include "WpftFilterFixture.hxx"
#include "WpftLoader.hxx"
#include "wpftimport.hxx"

namespace
{

namespace uno = css::uno;

class TextImportFilter : public writerperfect::ImportFilter<OdtGenerator>
{
public:
    explicit TextImportFilter(const uno::Reference< uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdtGenerator>(rxContext) {}

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString &ServiceName) override;
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() override;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, rtl::OUString &rTypeName) override;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdtGenerator &rGenerator, utl::MediaDescriptor &rDescriptor) override;

    static void generate(librevenge::RVNGTextInterface &rDocument);
};

bool TextImportFilter::doImportDocument(librevenge::RVNGInputStream &, OdtGenerator &rGenerator, utl::MediaDescriptor &)
{
    TextImportFilter::generate(rGenerator);
    return true;
}

bool TextImportFilter::doDetectFormat(librevenge::RVNGInputStream &, rtl::OUString &rTypeName)
{
    rTypeName = "WpftDummyText";
    return true;
}

// XServiceInfo
rtl::OUString SAL_CALL TextImportFilter::getImplementationName()
{
    return OUString("org.libreoffice.comp.Wpft.QA.TextImportFilter");
}

sal_Bool SAL_CALL TextImportFilter::supportsService(const rtl::OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< rtl::OUString > SAL_CALL TextImportFilter::getSupportedServiceNames()
{
    return {"com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection"};
}

void TextImportFilter::generate(librevenge::RVNGTextInterface &rDocument)
{
    using namespace librevenge;

    rDocument.startDocument(RVNGPropertyList());
    rDocument.openPageSpan(RVNGPropertyList());
    rDocument.openParagraph(RVNGPropertyList());
    rDocument.openSpan(RVNGPropertyList());
    rDocument.insertText("My hovercraft is full of eels.");
    rDocument.closeSpan();
    rDocument.closeParagraph();
    rDocument.closePageSpan();
    rDocument.endDocument();
}

}

namespace
{

class TextImportTest : public writerperfect::test::WpftFilterFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(TextImportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void TextImportTest::test()
{
    using namespace css;

    rtl::Reference<TextImportFilter> xFilter {new TextImportFilter(m_xContext)};
    writerperfect::test::WpftLoader aLoader(createDummyInput(), xFilter.get(), "private:factory/swriter", m_xDesktop, m_xContext);

    uno::Reference<text::XTextDocument> xDoc(aLoader.getDocument(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<container::XEnumerationAccess> xParaAccess(xDoc->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParaAccess.is());
    uno::Reference<container::XEnumeration> xParas = xParaAccess->createEnumeration();
    CPPUNIT_ASSERT(xParas.is());
    CPPUNIT_ASSERT(xParas->hasMoreElements());
    uno::Reference<container::XEnumerationAccess> xPortionsAccess(xParas->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortionsAccess.is());
    uno::Reference<container::XEnumeration> xPortions = xPortionsAccess->createEnumeration();
    CPPUNIT_ASSERT(xPortions.is());
    CPPUNIT_ASSERT(xPortions->hasMoreElements());
    uno::Reference<beans::XPropertySet> xPortionProps(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortionProps.is());
    rtl::OUString aPortionType;
    CPPUNIT_ASSERT(xPortionProps->getPropertyValue("TextPortionType") >>= aPortionType);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("Text"), aPortionType);
    uno::Reference<text::XTextRange> xPortion(xPortionProps, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortion.is());
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("My hovercraft is full of eels."), xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());
    CPPUNIT_ASSERT(!xParas->hasMoreElements());
}

CPPUNIT_TEST_SUITE_REGISTRATION(TextImportTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
