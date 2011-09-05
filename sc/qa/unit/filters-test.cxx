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

#include "docsh.hxx"

const int indeterminate = 2;

#define ODS_FORMAT_TYPE 50331943

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
    ScDocShellRef load(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData, sal_uLong nFormatType = 0);

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testODSs();

    CPPUNIT_TEST_SUITE(FiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testODSs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    uno::Reference<uno::XInterface> m_xCalcComponent;
    ::rtl::OUString m_aSrcRoot;
};

ScDocShellRef FiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData, sal_uLong nFormatType)
{
    sal_uInt32 nFormat = 0;
    if (nFormatType)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), nFormatType, nFormat, rtl::OUString(), 0, rtl::OUString(),
        rUserData, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    aFilter.SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocShRef = new ScDocShell;
    SfxMedium aSrcMed(rURL, STREAM_STD_READ, true);
    aSrcMed.SetFilter(&aFilter);
    if (!xDocShRef->DoLoad(&aSrcMed))
        // load failed.
        xDocShRef.Clear();

    return xDocShRef;
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
            bool bRes = load(rFilter, sURL, rUserData).Is();
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
    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/qpro/pass")), rtl::OUString(), true);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/qpro/fail")), rtl::OUString(), false);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/qpro/indeterminate")), rtl::OUString(), indeterminate);

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYLK")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/slk/pass")), rtl::OUString(), true);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYLK")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/slk/fail")), rtl::OUString(), false);

    recursiveScan(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYLK")),
        m_aSrcRoot + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/slk/indeterminate")), rtl::OUString(), indeterminate);

}

void FiltersTest::testODSs()
{
    rtl::OUString aString1(RTL_CONSTASCII_USTRINGPARAM("calc8"));
    rtl::OUString aString2 = m_aSrcRoot + rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data/ods/named-ranges-global.ods"));

    ScDocShellRef xDocSh = load( aString1, aString2 , rtl::OUString(), ODS_FORMAT_TYPE);

    CPPUNIT_ASSERT_MESSAGE("Failed to load named-ranges-global.ods.", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Global1")));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    pDoc->GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Global1 should reference Sheet1.A1", aValue == 1);
    pRangeData = pDoc->GetRangeName(0)->findByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local1")));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    pDoc->GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", aValue == 3);
    pRangeData = pDoc->GetRangeName(1)->findByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local2")));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    pDoc->GetValue(1,1,1,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 should reference Sheet2.A2", aValue == 7);
    //check for correct results for the remaining formulas
    pDoc->GetValue(1,1,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=global2 should be 2", aValue == 2);
    pDoc->GetValue(1,3,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=local2 should be 4", aValue == 4);
    pDoc->GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=SUM(global3) should be 10", aValue == 10);
    pDoc->GetValue(1,0,1,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", aValue == 5);
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

    //This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xCalcComponent =
        xSM->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.SpreadsheetDocument")));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());

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
