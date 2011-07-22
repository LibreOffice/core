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
#include <sal/cppunit.h>

#include <sal/config.h>

#include <osl/file.hxx>
#include <osl/process.h>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>

#include <comphelper/processfactory.hxx>

#include <tools/urlobj.hxx>

#include <unotools/tempfile.hxx>

#include <ucbhelper/contentbroker.hxx>

#include <vcl/svapp.hxx>

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
#include "swcrsr.hxx"
#include "swmodule.hxx"

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;

/* Implementation of Swdoc-Test class */

class SwDocTest : public CppUnit::TestFixture
{
public:
    SwDocTest();
    ~SwDocTest();

    virtual void setUp();
    virtual void tearDown();

    void randomTest();
    void testPageDescName();
    void testFileNameFields();

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    SwDoc *m_pDoc;
    SwDocShellRef m_xDocShRef;
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

    SfxFilter aFilter(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text")),
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TEXT")), rtl::OUString() );
    aDstMed.SetFilter(&aFilter);

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

static int
getRand(int modulus)
{
    if (modulus <= 0)
        return 0;
    return rand() % modulus;
}

static rtl::OUString
getRandString()
{
    static rtl::OUString aText( rtl::OUString::createFromAscii("AAAAA BBBB CCC DD E \n"));
    int s = getRand(aText.getLength());
    int j = getRand(aText.getLength() - s);
    rtl::OUString aRet(aText + s, j);
    if (!getRand(5))
        aRet += rtl::OUString(sal_Unicode('\n'));
//    fprintf (stderr, "rand string '%s'\n", OUStringToOString(aRet, RTL_TEXTENCODING_UTF8).getStr());
    return aRet;
}

#ifdef COMPLEX
static SwPosition
getRandomPosition(SwDoc *pDoc, int nOffset)
{
    SwPaM aPam(pDoc->GetNodes());
    SwCursor aCrs(*aPam.Start(), 0, false);
    for (int sskip = getRand(nOffset); sskip > 0; sskip--)
        aCrs.GoNextSentence();
    aCrs.GoNextCell(getRand(50));
    return *aCrs.GetPoint();
}
#endif

void SwDocTest::randomTest()
{
    CPPUNIT_ASSERT_MESSAGE("SwDoc::IsRedlineOn()", !m_pDoc->IsRedlineOn());
    RedlineMode_t modes[] = {
        nsRedlineMode_t::REDLINE_NONE,
        nsRedlineMode_t::REDLINE_ON,
        nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE | nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_DELETE
    };
    static const char *authors[] = {
        "Jim", "Bob", "JimBobina", "Helga", "Gertrude", "Spagna", "Hurtleweed"
    };

    for( sal_uInt16 rlm = 0; rlm < SAL_N_ELEMENTS(modes); rlm++)
    {
#ifdef COMPLEX
        m_pDoc->ClearDoc();

        // setup redlining
        m_pDoc->SetRedlineMode(modes[rlm]);
        SW_MOD()->SetRedlineAuthor(rtl::OUString::createFromAscii(authors[0]));
#endif

        for( int i = 0; i < 2000; i++ )
        {
#ifdef COMPLEX
            SwPaM aPam(m_pDoc->GetNodes());
            SwCursor aCrs(getRandomPosition(m_pDoc, i/20), 0, false);
            aCrs.SetMark();
            aCrs.GoNextCell(getRand(30));
#else // simple:
            SwNodeIndex nNode( m_pDoc->GetNodes().GetEndOfContent(), -1 );
            SwPaM aCrs( nNode );
#endif

            switch (getRand (i < 50 ? 3 : 6)) {
            // insert ops first
            case 0: {
                if (!m_pDoc->InsertString(aCrs, getRandString()))
                    fprintf (stderr, "failed to insert string !\n");
                break;
            }
            case 1:
                break;
            case 2: { // switch author
                int a = getRand(SAL_N_ELEMENTS(authors));
                SW_MOD()->SetRedlineAuthor(rtl::OUString::createFromAscii(authors[a]));
                break;
            }

#ifdef COMPLEX
            // movement / deletion ops later
            case 3: // deletion
                switch (getRand(6)) {
                case 0:
                    m_pDoc->DelFullPara(aCrs);
                    break;
                case 1:
                    m_pDoc->DeleteRange(aCrs);
                    break;
                case 2:
                    m_pDoc->DeleteAndJoin(aCrs, !!getRand(1));
                    break;
                case 3:
                default:
                    m_pDoc->Overwrite(aCrs, getRandString());
                    break;
                }
                break;
            case 4: { // movement
                IDocumentContentOperations::SwMoveFlags nFlags =
                    (IDocumentContentOperations::SwMoveFlags)
                        (getRand(1) ? // FIXME: puterb this more ?
                         IDocumentContentOperations::DOC_MOVEDEFAULT :
                         IDocumentContentOperations::DOC_MOVEALLFLYS |
                         IDocumentContentOperations::DOC_CREATEUNDOOBJ |
                         IDocumentContentOperations::DOC_MOVEREDLINES |
                         IDocumentContentOperations::DOC_NO_DELFRMS);
                SwPosition aTo(getRandomPosition(m_pDoc, i/10));
                m_pDoc->MoveRange(aCrs, aTo, nFlags);
                break;
            }
#endif
            case 5:
                break;

            // undo / redo ?
            default:
                break;
            }
#ifdef COMPLEX
            SwPosition start(m_pDoc->GetNodes());
            SwPosition end(m_pDoc->GetNodes().GetEndOfContent());
            CheckNodesRange(start.nNode, end.nNode, sal_True);
#endif
        }

#if 0
        fprintf (stderr, "write it !\n");
#ifdef COMPLEX
        SfxFilter aFilter(rtl::OUString::createFromAscii("writer8"),
                          rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
                          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CXML")),
                          rtl::OUString() );
#else
        SfxFilter aFilter(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text")),
                          rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
                          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TEXT")), rtl::OUString() );
#endif
        SfxMedium aDstMed(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///tmp/test.txt")),
                          STREAM_STD_READWRITE, true);
        aDstMed.SetFilter(&aFilter);
        m_xDocShRef->DoSaveAs(aDstMed);
        m_xDocShRef->DoSaveCompleted(&aDstMed);
        m_xDocShRef->DoInitNew(0);
#endif

#ifndef COMPLEX
        return;
#endif
    }
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

    SwGlobals::ensure();
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
