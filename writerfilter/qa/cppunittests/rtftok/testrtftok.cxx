/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/cppunit.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XFilter.hpp>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/svapp.hxx>
#include <ucbhelper/contentbroker.hxx>

using namespace ::com::sun::star;

class RtfTest : public CppUnit::TestFixture
{
public:
    RtfTest();
    ~RtfTest();

    virtual void setUp();
    virtual void tearDown();

    void recursiveScan(const rtl::OUString &rURL, bool bExpected);
    bool load(const rtl::OUString &rURL);
    void test();

    CPPUNIT_TEST_SUITE(RtfTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    uno::Reference<lang::XMultiServiceFactory> m_xMSF;
    uno::Reference<document::XFilter> m_xFilter;

    ::rtl::OUString m_aSrcRoot;
    int m_nLoadedDocs;
};

RtfTest::RtfTest()
    : m_aSrcRoot(RTL_CONSTASCII_USTRINGPARAM("file://")),
    m_nLoadedDocs(0)
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xMSF = uno::Reference<lang::XMultiServiceFactory>(m_xFactory, uno::UNO_QUERY_THROW);
    m_xFilter = uno::Reference< document::XFilter >(m_xMSF->createInstance(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.RtfFilter"))),
        uno::UNO_QUERY_THROW);

    const char* pSrcRoot = getenv( "SRC_ROOT" );
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != NULL && pSrcRoot[0] != 0);

#ifdef WNT
    if (pSrcRoot[1] == ':')
        m_aSrcRoot += rtl::OUString::createFromAscii( "/" );
#endif
    m_aSrcRoot += rtl::OUString::createFromAscii( pSrcRoot );

    // Without these we're crashing
    comphelper::setProcessServiceFactory(m_xMSF);
    InitVCL(m_xMSF);

    // initialise UCB-Broker
    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    bool bInitUcb = ucbhelper::ContentBroker::initialize(m_xMSF, aUcbInitSequence);
    CPPUNIT_ASSERT_MESSAGE("Should be able to initialize UCB", bInitUcb);

    uno::Reference<ucb::XContentProviderManager> xUcb =
        ucbhelper::ContentBroker::get()->getContentProviderManagerInterface();
    uno::Reference<ucb::XContentProvider> xFileProvider(m_xMSF->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.FileContentProvider"))), uno::UNO_QUERY);
    xUcb->registerContentProvider(xFileProvider, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file")), sal_True);
}

RtfTest::~RtfTest()
{
    DeInitVCL();
}

void RtfTest::setUp()
{
}

void RtfTest::tearDown()
{
}

bool RtfTest::load(const rtl::OUString &rURL)
{
    ++m_nLoadedDocs;
    uno::Sequence< beans::PropertyValue > aDescriptor(1);
    aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
    aDescriptor[0].Value <<= rURL;
    return m_xFilter->filter(aDescriptor);
}

void RtfTest::recursiveScan(const rtl::OUString &rURL, bool bExpected)
{
    osl::Directory aDir(rURL);

    CPPUNIT_ASSERT_MESSAGE("failed to open directory", osl::FileBase::E_None == aDir.open());
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        rtl::OUString sURL = aFileStatus.getFileURL();
        if (aFileStatus.getFileType() == osl::FileStatus::Directory)
            recursiveScan(sURL, bExpected);
        else
        {
            //ignore .files
            sal_Int32 nLastSlash = sURL.lastIndexOf('/');
            if ((nLastSlash != -1) && (nLastSlash+1 < sURL.getLength()) &&
                    (sURL.getStr()[nLastSlash+1] == '.'))
                continue;

            bool bRes = load(sURL);
            rtl::OString aRes(rtl::OUStringToOString(sURL, osl_getThreadTextEncoding()));
            CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == bExpected);
        }
    }
    CPPUNIT_ASSERT_MESSAGE("failed to close directory", osl::FileBase::E_None == aDir.close());
}

void RtfTest::test()
{
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/filters/writerfilter/qa/cppunittests/rtftok/data/pass")), true);
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/filters/writerfilter/qa/cppunittests/rtftok/data/fail")), false);

    printf("Rtf: tested %d files\n", m_nLoadedDocs);
}

CPPUNIT_TEST_SUITE_REGISTRATION(RtfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
