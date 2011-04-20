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
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
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

using namespace ::com::sun::star;

namespace
{
    class LotusWordProTest : public ::CppUnit::TestFixture
    {
    public:
        LotusWordProTest();

        virtual void setUp();
        virtual void tearDown();

        void recursiveScan(const rtl::OUString &rURL, bool bExpected);
        bool load(const rtl::OUString &rURL);
        void test();

        CPPUNIT_TEST_SUITE(LotusWordProTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    private:
        uno::Reference<uno::XComponentContext> m_xContext;
        uno::Reference<lang::XMultiComponentFactory> m_xFactory;
        uno::Reference<lang::XMultiServiceFactory> m_xMSF;
        uno::Reference<document::XFilter> m_xFilter;

        ::rtl::OUString m_aPWDURL;
        int m_nLoadedDocs;
    };

    LotusWordProTest::LotusWordProTest() : m_nLoadedDocs(0)
    {
        m_xContext = cppu::defaultBootstrap_InitialComponentContext();
        m_xFactory = m_xContext->getServiceManager();
        m_xMSF = uno::Reference<lang::XMultiServiceFactory>(m_xFactory, uno::UNO_QUERY_THROW);
        m_xFilter = uno::Reference< document::XFilter >(m_xMSF->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.LotusWordProImportFilter"))),
            uno::UNO_QUERY_THROW);

        oslProcessError err = osl_getProcessWorkingDir(&m_aPWDURL.pData);
        CPPUNIT_ASSERT_MESSAGE("no PWD!", err == osl_Process_E_None);

        //Without this we're crashing because callees are using
        //getProcessServiceFactory.  In general those should be removed in favour
        //of retaining references to the root ServiceFactory as its passed around
        comphelper::setProcessServiceFactory(m_xMSF);

        //Lotus Import filter pokes at printers :-(
        InitVCL(m_xMSF);
    }

    void LotusWordProTest::setUp()
    {
    }

    void LotusWordProTest::tearDown()
    {
    }

    bool LotusWordProTest::load(const rtl::OUString &rURL)
    {
        uno::Sequence< beans::PropertyValue > aDescriptor(1);
        aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
        aDescriptor[0].Value <<= rURL;
        sal_Bool bRet = m_xFilter->filter(aDescriptor);

        ++m_nLoadedDocs;

        return bRet;
    }

    void LotusWordProTest::recursiveScan(const rtl::OUString &rURL, bool bExpected)
    {
        osl::Directory aDir(rURL);

        CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
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
                bool bRes = load(sURL);
                rtl::OString aRes(rtl::OUStringToOString(sURL, osl_getThreadTextEncoding()));
                CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == bExpected);
            }
        }
        CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.close());
    }

    void LotusWordProTest::test()
    {
        recursiveScan(m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/qa/cppunit/data/pass")), true);
        recursiveScan(m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/qa/cppunit/data/fail/")), false);

        printf("LotusWordPro: tested %d files\n", m_nLoadedDocs);
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(LotusWordProTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
