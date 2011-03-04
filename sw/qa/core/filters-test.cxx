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
#include "precompiled_sw.hxx"

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#endif

#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>

#include <osl/file.hxx>
#include <osl/process.h>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/contentbroker.hxx>

#include "init.hxx"
#include "swtypes.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "shellres.hxx"
#include "docufld.hxx"

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;

static USHORT aWndFunc(Window *, USHORT, const String &, const String &)
{
    return ERRCODE_BUTTON_OK;
}

/* Implementation of Filters test */

class FiltersTest : public CppUnit::TestFixture
{
public:
    FiltersTest();
    ~FiltersTest();

    virtual void setUp();
    virtual void tearDown();

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
    ::rtl::OUString m_aPWDURL;
};

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
//To-Do: I know this works on Linux, please check if this test works under
//windows and enable it if so
#ifndef WNT
    bool bResult;

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice XML (Writer)")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CXML")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2006-3117-1.sxw")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2006-3117 regression", bResult == false);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2007-0245-1.rtf")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2007-0245 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2009-0200-1.doc")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2009-0200 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2009-0201-1.doc")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2009-0201 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2010-3451-1.rtf")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2010-3451 regression", bResult == false);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rich Text Format")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RTF")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2010-3452-1.rtf")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2010-3452 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2010-3453-1.doc")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2010-3453 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Word 97")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2010-3454-1.doc")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2010-3454 regression", bResult == true);
#endif
}

FiltersTest::FiltersTest()
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


    InitVCL(xSM);

    //This is a bit of a fudge, we do this to ensure that SwDLL::Init, which is
    //a private symbol to us, gets called
    m_xWriterComponent =
        xSM->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.TextDocument")));
    CPPUNIT_ASSERT_MESSAGE("no PWD!", m_xWriterComponent.is());

    ErrorHandler::RegisterDisplay(&aWndFunc);

    oslProcessError err = osl_getProcessWorkingDir(&m_aPWDURL.pData);
    CPPUNIT_ASSERT_MESSAGE("no PWD!", err == osl_Process_E_None);
}

void FiltersTest::setUp()
{
}

FiltersTest::~FiltersTest()
{
}

void FiltersTest::tearDown()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(FiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
