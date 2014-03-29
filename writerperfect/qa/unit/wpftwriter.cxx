/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/theDesktop.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <test/bootstrapfixture.hxx>

#include <unotest/filters-test.hxx>

namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace document = com::sun::star::document;
namespace frame = com::sun::star::frame;
namespace io = com::sun::star::io;
namespace lang = com::sun::star::lang;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

namespace
{

class WpftWriterFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(WpftWriterFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    virtual bool load(const OUString &, const OUString &rURL, const OUString &,
        unsigned int, unsigned int, unsigned int);

    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath);

    void impl_detectFilterName(uno::Sequence<beans::PropertyValue> &rDescriptor, const rtl::OUString &rTypeName);

private:
    uno::Reference<frame::XDesktop2> m_xDesktop;
    uno::Reference<ucb::XSimpleFileAccess> m_xFileAccess;
    uno::Reference<document::XFilter> m_xFilter;
    uno::Reference<container::XNameAccess> m_xTypeMap;
};

void WpftWriterFilterTest::setUp()
{
    test::BootstrapFixture::setUp();

    m_xDesktop = frame::theDesktop::get(m_xContext);
    m_xFileAccess = ucb::SimpleFileAccess::create(m_xContext);

    const uno::Reference<document::XTypeDetection> xTypeDetection(
            m_xFactory->createInstanceWithContext("com.sun.star.document.TypeDetection", m_xContext),
            uno::UNO_QUERY_THROW);
    m_xTypeMap.set(xTypeDetection, uno::UNO_QUERY_THROW);
}

void WpftWriterFilterTest::tearDown()
{
    test::BootstrapFixture::tearDown();

    m_xDesktop->terminate();
}

void WpftWriterFilterTest::test()
{
    doTest("com.sun.star.comp.Writer.AbiWordImportFilter", "/writerperfect/qa/unit/data/libabw/");
    doTest("org.libreoffice.comp.Writer.EBookImportFilter", "/writerperfect/qa/unit/data/libe-book/");
    doTest("com.sun.star.comp.Writer.MSWorksImportFilter", "/writerperfect/qa/unit/data/libwps/");
    doTest("com.sun.star.comp.Writer.MWAWImportFilter", "/writerperfect/qa/unit/data/libmwaw/");
    doTest("com.sun.star.comp.Writer.WordPerfectImportFilter", "/writerperfect/qa/unit/data/libwpd/");
}

bool WpftWriterFilterTest::load(const OUString &, const OUString &rURL, const OUString &,
    unsigned int, unsigned int, unsigned int)
{
    const uno::Reference<lang::XComponent> xDoc(
            m_xDesktop->loadComponentFromURL("private:factory/swriter", "_blank", 0, uno::Sequence<beans::PropertyValue>()),
            uno::UNO_QUERY_THROW);

    bool result = false;

    try
    {
        const uno::Reference<document::XImporter> xImporter(m_xFilter, uno::UNO_QUERY_THROW);

        xImporter->setTargetDocument(xDoc);

        uno::Sequence<beans::PropertyValue> aDescriptor(2);
        aDescriptor[0].Name = "URL";
        aDescriptor[0].Value <<= rURL;

        const uno::Reference<io::XInputStream> xInputStream(m_xFileAccess->openFileRead(rURL), uno::UNO_QUERY_THROW);
        aDescriptor[1].Name = "InputStream";
        aDescriptor[1].Value <<= xInputStream;

        const uno::Reference<document::XExtendedFilterDetection> xDetector(m_xFilter, uno::UNO_QUERY_THROW);

        const rtl::OUString aTypeName(xDetector->detect(aDescriptor));
        if (aTypeName.isEmpty())
            throw lang::IllegalArgumentException();

        impl_detectFilterName(aDescriptor, aTypeName);

        result = m_xFilter->filter(aDescriptor);
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    xDoc->dispose();

    return result;
}

void WpftWriterFilterTest::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    testDir(OUString(), getURLFromSrc(rPath), OUString());
}

void WpftWriterFilterTest::impl_detectFilterName(uno::Sequence<beans::PropertyValue> &rDescriptor, const rtl::OUString &rTypeName)
{
    const sal_Int32 nDescriptorLen = rDescriptor.getLength();

    for (sal_Int32 n = 0; nDescriptorLen != n; ++n)
    {
        if ("FilterName" == rDescriptor[n].Name)
            return;
    }

    uno::Sequence<beans::PropertyValue> aTypes;
    if (m_xTypeMap->getByName(rTypeName) >>= aTypes)
    {
        for (sal_Int32 n = 0; aTypes.getLength() != n; ++n)
        {
            rtl::OUString aFilterName;
            if (("PreferredFilter" == aTypes[n].Name) && (aTypes[n].Value >>= aFilterName))
            {
                rDescriptor.realloc(nDescriptorLen + 1);
                rDescriptor[nDescriptorLen].Name = "FilterName";
                rDescriptor[nDescriptorLen].Value <<= aFilterName;
                return;
            }
        }
    }

    throw container::NoSuchElementException();
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftWriterFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
