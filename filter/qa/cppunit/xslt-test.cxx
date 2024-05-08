/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <condition_variable>
#include <mutex>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <sal/log.hxx>

#include <rtl/ref.hxx>

#include <osl/file.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/xslt/XSLTTransformer.hpp>

#include <cppuhelper/implbase.hxx>

#include <test/bootstrapfixture.hxx>


using namespace ::com::sun::star;


namespace {

class XsltFilterTest
    : public test::BootstrapFixture
{
public:
    void testXsltCopyOld();
    void testXsltCopyNew();

    CPPUNIT_TEST_SUITE(XsltFilterTest);
    CPPUNIT_TEST(testXsltCopyOld);
    CPPUNIT_TEST(testXsltCopyNew);
    CPPUNIT_TEST_SUITE_END();
};

class Listener : public ::cppu::WeakImplHelper<io::XStreamListener>
{
public:
    Listener() : m_bDone(false) {}

    void wait() {
        std::unique_lock<std::mutex> g(m_mutex);
        m_cond.wait(g, [this]() { return m_bDone; });
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_bDone;

    virtual void SAL_CALL disposing(const lang::EventObject&) noexcept override {}
    virtual void SAL_CALL started() noexcept override {}
    virtual void SAL_CALL closed() noexcept override { notifyDone(); }
    virtual void SAL_CALL terminated() noexcept override { notifyDone(); }
    virtual void SAL_CALL error(const uno::Any& e) override
    {
        notifyDone(); // set on error too, otherwise main thread waits forever
        SAL_WARN("filter.xslt", e);
        CPPUNIT_FAIL("exception while in XSLT");
    }

    void notifyDone() {
        std::scoped_lock<std::mutex> g(m_mutex);
        m_bDone = true;
        m_cond.notify_all();
    }
};

void XsltFilterTest::testXsltCopyNew()
{
    OUString tempDirURL;
    osl_getTempDirURL(&tempDirURL.pData);
    oslFileHandle tempFile;
    OUString tempURL;
    osl::File::RC rc = osl::File::createTempFile(nullptr, &tempFile, &tempURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, rc);
    osl_closeFile(tempFile); // close it so xSFA can open it on WNT

    OUString source(
            m_directories.getURLFromSrc(u"/filter/source/xsltfilter/xsltfilter.component"));
    uno::Sequence<uno::Any> args{
        uno::Any(beans::NamedValue(u"StylesheetURL"_ustr,
            uno::Any(m_directories.getURLFromSrc(u"/filter/qa/cppunit/data/xslt/copy.xslt")))),
        uno::Any(beans::NamedValue(u"SourceURL"_ustr, uno::Any(source))),
        uno::Any(beans::NamedValue(u"TargetURL"_ustr, uno::Any(tempURL))),
        uno::Any(beans::NamedValue(u"SourceBaseURL"_ustr,
            uno::Any(m_directories.getURLFromSrc(u"/filter/source/xsltfilter/")))),
        uno::Any(beans::NamedValue(u"TargetBaseURL"_ustr, uno::Any(tempDirURL))),
        uno::Any(beans::NamedValue(u"SystemType"_ustr, uno::Any(OUString()))),
        uno::Any(beans::NamedValue(u"PublicType"_ustr, uno::Any(OUString())))
    };

    uno::Reference<ucb::XSimpleFileAccess3> xSFA =
        ucb::SimpleFileAccess::create(getComponentContext());

    uno::Reference<io::XInputStream> xIn = xSFA->openFileRead(source);
    uno::Reference<io::XOutputStream> xOut = xSFA->openFileWrite(tempURL);

    rtl::Reference<Listener> xListener = new Listener();

    uno::Reference<xml::xslt::XXSLTTransformer> xXslt(
            xml::xslt::XSLTTransformer::create(getComponentContext(), args));

    xXslt->addListener(xListener);
    xXslt->setInputStream(xIn);
    xXslt->setOutputStream(xOut);

    xXslt->start();

    xListener->wait();

    xIn->closeInput();
    xOut->closeOutput();

    osl::File foo(tempURL); // apparently it's necessary to open it again?
    foo.open(osl_File_OpenFlag_Read);
    sal_uInt64 size(0);
    foo.getSize(size);
    CPPUNIT_ASSERT(size > 1000); // check that something happened
    foo.close();
    osl_removeFile(tempURL.pData);
}

void XsltFilterTest::testXsltCopyOld()
{
    OUString tempDirURL;
    osl_getTempDirURL(&tempDirURL.pData);
    oslFileHandle tempFile;
    OUString tempURL;
    osl::File::RC rc = osl::File::createTempFile(nullptr, &tempFile, &tempURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, rc);
    osl_closeFile(tempFile); // close it so xSFA can open it on WNT

    OUString source(
            m_directories.getURLFromSrc(u"/filter/source/xsltfilter/xsltfilter.component"));
    uno::Sequence<uno::Any> args{
        uno::Any(beans::NamedValue(u"StylesheetURL"_ustr,
            uno::Any(m_directories.getURLFromSrc(u"/filter/qa/cppunit/data/xslt/copy.xslt")))),
        uno::Any(beans::NamedValue(u"SourceURL"_ustr, uno::Any(source))),
        uno::Any(beans::NamedValue(u"TargetURL"_ustr, uno::Any(tempURL))),
        uno::Any(beans::NamedValue(u"SourceBaseURL"_ustr,
            uno::Any(m_directories.getURLFromSrc(u"/filter/source/xsltfilter/")))),
        uno::Any(beans::NamedValue(u"TargetBaseURL"_ustr, uno::Any(tempDirURL))),
        uno::Any(beans::NamedValue(u"SystemType"_ustr, uno::Any(OUString()))),
        uno::Any(beans::NamedValue(u"PublicType"_ustr, uno::Any(OUString())))
    };

    uno::Reference<ucb::XSimpleFileAccess3> xSFA =
        ucb::SimpleFileAccess::create(getComponentContext());

    uno::Reference<io::XInputStream> xIn = xSFA->openFileRead(source);
    uno::Reference<io::XOutputStream> xOut = xSFA->openFileWrite(tempURL);

    rtl::Reference<Listener> xListener = new Listener();

    uno::Reference<xml::xslt::XXSLTTransformer> xXslt(
        getMultiServiceFactory()->createInstance(
            u"com.sun.star.comp.documentconversion.LibXSLTTransformer"_ustr),
        uno::UNO_QUERY_THROW);

    uno::Reference<lang::XInitialization> xInit(xXslt, uno::UNO_QUERY_THROW);
    xInit->initialize(args);
    xXslt->addListener(xListener);
    xXslt->setInputStream(xIn);
    xXslt->setOutputStream(xOut);

    xXslt->start();

    xListener->wait();

    xIn->closeInput();
    xOut->closeOutput();

    osl::File foo(tempURL); // apparently it's necessary to open it again?
    foo.open(osl_File_OpenFlag_Read);
    sal_uInt64 size(0);
    foo.getSize(size);
    CPPUNIT_ASSERT(size > 1000); // check that something happened
    foo.close();
    osl_removeFile(tempURL.pData);
}

CPPUNIT_TEST_SUITE_REGISTRATION(XsltFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
