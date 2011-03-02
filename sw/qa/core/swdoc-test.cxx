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
 *       Thorsten Behrens <tbehrens@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Thorsten Behrens <tbehrens@novell.com>
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

/* Implementation of Swdoc-Test class */

class SwDocTest : public CppUnit::TestFixture
{
public:
    SwDocTest();
    ~SwDocTest();

    virtual void setUp();
    virtual void tearDown();

    bool testLoad(const rtl::OUString &rFilter, const rtl::OUString &rURL);

    void randomTest();
    void testPageDescName();
    void testFileNameFields();

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    SwDoc *m_pDoc;
    SwDocShellRef m_xDocShRef;
    ::rtl::OUString m_aPWDURL;
};

void SwDocTest::testPageDescName()
{
    ShellResource aShellResources;

    std::vector<rtl::OUString> aResults;

    //These names must be unique for each different combination, otherwise
    //duplicate page description names may exist, which will causes lookup
    //by name to be incorrect, and so the corresponding export to .odt
    aResults.push_back(aShellResources.GetPageDescName(1, ShellResource::NORMAL_PAGE));
    aResults.push_back(aShellResources.GetPageDescName(1, ShellResource::FIRST_PAGE));
    aResults.push_back(aShellResources.GetPageDescName(1, ShellResource::FOLLOW_PAGE));

    std::sort(aResults.begin(), aResults.end());
    aResults.erase(std::unique(aResults.begin(), aResults.end()), aResults.end());

    CPPUNIT_ASSERT_MESSAGE("GetPageDescName results must be unique", aResults.size() == 3);
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=32463 for motivation
void SwDocTest::testFileNameFields()
{
    //Here's a file name with some chars in it that will be %% encoded, when expanding
    //SwFileNameFields we want to restore the original readable filename
    utl::TempFile aTempFile(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("demo [name]")));
    aTempFile.EnableKillingFile();

    INetURLObject aTempFileURL(aTempFile.GetURL());
    String sFileURL = aTempFileURL.GetMainURL(INetURLObject::NO_DECODE);
    SfxMedium aDstMed(sFileURL, STREAM_STD_READWRITE, true);

    m_xDocShRef->DoSaveAs(aDstMed);
    m_xDocShRef->DoSaveCompleted(&aDstMed);

    const INetURLObject &rUrlObj = m_xDocShRef->GetMedium()->GetURLObject();

    SwFileNameFieldType aNameField(m_pDoc);

    {
        rtl::OUString sResult(aNameField.Expand(FF_NAME));
        rtl::OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
            true,INetURLObject::DECODE_WITH_CHARSET));
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        rtl::OUString sResult(aNameField.Expand(FF_PATHNAME));
        rtl::OUString sExpected(rUrlObj.GetFull());
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        rtl::OUString sResult(aNameField.Expand(FF_PATH));
        INetURLObject aTemp(rUrlObj);
        aTemp.removeSegment();
        rtl::OUString sExpected(aTemp.PathToFileName());
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        rtl::OUString sResult(aNameField.Expand(FF_NAME_NOEXT));
        rtl::OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
            true,INetURLObject::DECODE_WITH_CHARSET));
        //Chop off .tmp
        sExpected = sExpected.copy(0, sExpected.getLength() - 4);
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    m_xDocShRef->DoInitNew(0);
}

bool SwDocTest::testLoad(const rtl::OUString &rFilter, const rtl::OUString &rURL)
{
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rtl::OUString(), rtl::OUString() );

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium aSrcMed(rURL, STREAM_STD_READ, true);
    aSrcMed.SetFilter(&aFilter);
    return xDocShRef->DoLoad(&aSrcMed);
}

void SwDocTest::testCVEs()
{
//To-Do: I know this works on Linux, please check if this test works under
//windows and enable it if so
#ifndef WNT
    bool bResult;

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice XML (Writer)")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2006-3117-1.sxw")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2006-3117 regression", bResult == false);
#endif
}

void SwDocTest::randomTest()
{
    CPPUNIT_ASSERT_MESSAGE("SwDoc::IsRedlineOn()", !m_pDoc->IsRedlineOn());
}

SwDocTest::SwDocTest()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();

    uno::Reference<lang::XMultiServiceFactory> xSM(m_xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL(xSM);

    SwDLL::Init();

    oslProcessError err = osl_getProcessWorkingDir(&m_aPWDURL.pData);
    CPPUNIT_ASSERT_MESSAGE("no PWD!", err == osl_Process_E_None);

    ErrorHandler::RegisterDisplay(&aWndFunc);
}

void SwDocTest::setUp()
{
    m_pDoc = new SwDoc;
    m_xDocShRef = new SwDocShell(m_pDoc, SFX_CREATE_MODE_EMBEDDED);
    m_xDocShRef->DoInitNew(0);
}

SwDocTest::~SwDocTest()
{
}

void SwDocTest::tearDown()
{
    m_xDocShRef.Clear();
    delete m_pDoc;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDocTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
