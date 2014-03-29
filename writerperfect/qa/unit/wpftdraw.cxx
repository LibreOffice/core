/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/theDesktop.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <test/bootstrapfixture.hxx>

#include <unotest/filters-test.hxx>

namespace beans = com::sun::star::beans;
namespace document = com::sun::star::document;
namespace frame = com::sun::star::frame;
namespace io = com::sun::star::io;
namespace lang = com::sun::star::lang;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

namespace
{

class WpftDrawFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(WpftDrawFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    virtual bool load(const OUString &, const OUString &rURL, const OUString &,
        unsigned int, unsigned int, unsigned int);

    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath);

private:
    uno::Reference<frame::XDesktop2> m_xDesktop;
    uno::Reference<ucb::XSimpleFileAccess> m_xFileAccess;
    uno::Reference<document::XFilter> m_xFilter;
};

void WpftDrawFilterTest::setUp()
{
    test::BootstrapFixture::setUp();

    m_xDesktop = frame::theDesktop::get(m_xContext);
    m_xFileAccess = ucb::SimpleFileAccess::create(m_xContext);
}

void WpftDrawFilterTest::tearDown()
{
    test::BootstrapFixture::tearDown();

    m_xDesktop->terminate();
}

void WpftDrawFilterTest::test()
{
    doTest("com.sun.star.comp.Draw.CDRImportFilter", "/writerperfect/qa/unit/data/libcdr/");
    doTest("com.sun.star.comp.Draw.CMXImportFilter", "/writerperfect/qa/unit/data/libcdr-cmx/");
    doTest("com.sun.star.comp.Draw.FreehandImportFilter", "/writerperfect/qa/unit/data/libfreehand/");
    doTest("com.sun.star.comp.Draw.MSPUBImportFilter", "/writerperfect/qa/unit/data/libmspub/");
    doTest("com.sun.star.comp.Draw.VisioImportFilter", "/writerperfect/qa/unit/data/libvisio/");
    doTest("com.sun.star.comp.Draw.WPGImportFilter", "/writerperfect/qa/unit/data/libwpg/");
}

bool WpftDrawFilterTest::load(const OUString &, const OUString &rURL, const OUString &,
    unsigned int, unsigned int, unsigned int)
{
    const uno::Reference<lang::XComponent> xDoc(
            m_xDesktop->loadComponentFromURL("private:factory/simpress", "_blank", 0, uno::Sequence<beans::PropertyValue>()),
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

        result = m_xFilter->filter(aDescriptor);
    }
    catch (const uno::Exception &)
    {
        xDoc->dispose();
        throw;
    }

    xDoc->dispose();

    return result;
}

void WpftDrawFilterTest::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    testDir(OUString(), getURLFromSrc(rPath), OUString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftDrawFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
