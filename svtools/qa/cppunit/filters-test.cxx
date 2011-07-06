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
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/cppunit.h>
#include <sal/config.h>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/filter.hxx>

const int indeterminate = 2;

using namespace ::com::sun::star;

/* Implementation of Filters test */

class FiltersTest : public CppUnit::TestFixture
{
public:
    FiltersTest();
    ~FiltersTest();

    virtual void setUp();
    virtual void tearDown();

    void recursiveScan(const rtl::OUString &rURL, int nExpected);
    bool load(const rtl::OUString &rURL);

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(FiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    ::rtl::OUString m_aSrcRoot;
};

bool FiltersTest::load(const rtl::OUString &rURL)
{
    GraphicFilter aGraphicFilter(false);
    SvFileStream aFileStream(rURL, STREAM_READ);
    Graphic aGraphic;
    return aGraphicFilter.ImportGraphic(aGraphic, rURL, aFileStream) == 0;
}

void FiltersTest::recursiveScan(const rtl::OUString &rURL, int nExpected)
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
            recursiveScan(sURL, nExpected);
        else
        {
            sal_Int32 nLastSlash = sURL.lastIndexOf('/');

            //ignore .files
            if (
                 (nLastSlash != -1) && (nLastSlash+1 < sURL.getLength()) &&
                 (sURL.getStr()[nLastSlash+1] == '.')
               )
            {
                continue;
            }

            rtl::OString aRes(rtl::OUStringToOString(sURL,
                osl_getThreadTextEncoding()));
            if (nExpected == indeterminate)
            {
                fprintf(stderr, "loading %s\n", aRes.getStr());
            }
            sal_uInt32 nStartTime = osl_getGlobalTimer();
            bool bRes = load(sURL);
            sal_uInt32 nEndTime = osl_getGlobalTimer();
            if (nExpected == indeterminate)
            {
                fprintf(stderr, "pass/fail was %d (%"SAL_PRIuUINT32" ms)\n",
                    bRes, nEndTime-nStartTime);
                continue;
            }
            CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == nExpected);
        }
    }
    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.close());
}

void FiltersTest::testCVEs()
{
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/wmf/pass")), true);
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/wmf/fail")), false);
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/wmf/indeterminate")), 2);

    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/emf/pass")), true);
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/emf/fail")), false);
    recursiveScan(m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/libs-gui/svtools/qa/cppunit/data/emf/indeterminate")), 2);
}

FiltersTest::FiltersTest()
    : m_aSrcRoot(RTL_CONSTASCII_USTRINGPARAM("file://"))
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();

    uno::Reference<lang::XMultiServiceFactory> xSM(m_xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);
#if 0
    // initialise UCB-Broker
    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    bool bInitUcb = ucbhelper::ContentBroker::initialize(xSM, aUcbInitSequence);
    CPPUNIT_ASSERT_MESSAGE("Should be able to initialize UCB", bInitUcb);

    uno::Reference<ucb::XContentProviderManager> xUcb =
        ucbhelper::ContentBroker::get()->getContentProviderManagerInterface();
    uno::Reference<ucb::XContentProvider> xFileProvider(xSM->createInstance(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.FileContentProvider"))), uno::UNO_QUERY);
    xUcb->registerContentProvider(xFileProvider, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file")), sal_True);

    // force locale (and resource files loaded) to en-US
    const LanguageType eLang=LANGUAGE_ENGLISH_US;

    rtl::OUString aLang, aCountry;
    MsLangId::convertLanguageToIsoNames(eLang, aLang, aCountry);
    lang::Locale aLocale(aLang, aCountry, rtl::OUString());
    ResMgr::SetDefaultLocale( aLocale );

    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetUILocaleConfigString(
        MsLangId::convertLanguageToIsoString( eLang ) );
#endif

    InitVCL(xSM);

    const char* pSrcRoot = getenv( "SRC_ROOT" );
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != NULL && pSrcRoot[0] != 0);

#ifdef WNT
    if (pSrcRoot[1] == ':')
        m_aSrcRoot += rtl::OUString::createFromAscii( "/" );
#endif
    m_aSrcRoot += rtl::OUString::createFromAscii( pSrcRoot );
}

void FiltersTest::setUp()
{
}

FiltersTest::~FiltersTest()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void FiltersTest::tearDown()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(FiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
