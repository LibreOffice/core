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

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>

#include <comphelper/processfactory.hxx>

#include <i18npool/mslangid.hxx>

#include <tools/urlobj.hxx>

#include <unotools/tempfile.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <vcl/svapp.hxx>

#include <ucbhelper/contentbroker.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include "init.hxx"
#include "swtypes.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "shellres.hxx"
#include "docufld.hxx"

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

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

    void recursiveScan(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData, int nExpected);
    bool load(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData);

    bool testLoad(const rtl::OUString &rFilter,
        const rtl::OUString &rUserData,
        const rtl::OUString &rURL);

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
    uno::Reference<uno::XInterface> m_xWriterComponent;
    ::rtl::OUString m_aSrcRoot;
};

bool FiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData)
{
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rUserData, rtl::OUString() );

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium aSrcMed(rURL, STREAM_STD_READ, true);
    aSrcMed.SetFilter(&aFilter);
    return xDocShRef->DoLoad(&aSrcMed);
}

void FiltersTest::recursiveScan(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData, int nExpected)
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
            recursiveScan(rFilter, sURL, rUserData, nExpected);
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
            bool bRes = load(rFilter, sURL, rUserData);
            sal_uInt32 nEndTime = osl_getGlobalTimer();
            if (nExpected == indeterminate)
            {
                fprintf(stderr, "pass/fail was %d (%d ms)\n", bRes, nEndTime-nStartTime);
                continue;
            }
            CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == nExpected);
        }
    }
    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.close());
}

bool FiltersTest::testLoad(const rtl::OUString &rFilter,
    const rtl::OUString &rUserData,
    const rtl::OUString &rURL)
{
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rUserData, rtl::OUString() );

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium aSrcMed(rURL, STREAM_STD_READ, true);
    aSrcMed.SetFilter(&aFilter);
    return xDocShRef->DoLoad(&aSrcMed);
}

void FiltersTest::testCVEs()
{
    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Staroffice XML (Writer)")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/xml/pass")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CXML")), true);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Staroffice XML (Writer)")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/xml/fail")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CXML")), false);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/rtf/pass")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")), true);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/rtf/fail")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")), false);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/rtf/indeterminate")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")), indeterminate);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/ww8/pass")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")), true);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/ww8/fail")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")), false);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")), m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/clone/writer/sw/qa/core/data/ww8/indeterminate")), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")), indeterminate);
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

    InitVCL(xSM);

    //This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xWriterComponent =
        xSM->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.TextDocument")));
    CPPUNIT_ASSERT_MESSAGE("no writer component!", m_xWriterComponent.is());

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
