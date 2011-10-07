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

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>

#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include "init.hxx"
#include "swtypes.hxx"
#include "docstat.hxx"
#include "doc.hxx"
#include "ndtxt.hxx"
#include "docsh.hxx"
#include "shellres.hxx"
#include "docufld.hxx"
#include "fmtanchr.hxx"
#include "swscanner.hxx"
#include "swcrsr.hxx"
#include "swmodule.hxx"
#include "shellio.hxx"

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;

/* Implementation of Swdoc-Test class */

class SwDocTest : public test::BootstrapFixture
{
public:
    virtual void setUp();
    virtual void tearDown();

    void randomTest();
    void testPageDescName();
    void testFileNameFields();
    void testDocStat();
    void testSwScanner();
    void testGraphicAnchorDeletion();

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST(testDocStat);
    CPPUNIT_TEST(testSwScanner);
    CPPUNIT_TEST(testGraphicAnchorDeletion);
    CPPUNIT_TEST_SUITE_END();

private:
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

//See http://lists.freedesktop.org/archives/libreoffice/2011-August/016666.html
//Remove unnecessary parameter to IDocumentStatistics::UpdateDocStat for
//motivation
void SwDocTest::testDocStat()
{
    CPPUNIT_ASSERT_MESSAGE("Expected initial 0 count", m_pDoc->GetDocStat().nChar == 0);

    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    rtl::OUString sText(RTL_CONSTASCII_USTRINGPARAM("Hello World"));
    m_pDoc->InsertString(aPaM, sText);

    CPPUNIT_ASSERT_MESSAGE("Should still be non-updated 0 count", m_pDoc->GetDocStat().nChar == 0);

    SwDocStat aDocStat = m_pDoc->GetUpdatedDocStat();
    sal_uLong nLen = static_cast<sal_uLong>(sText.getLength());

    CPPUNIT_ASSERT_MESSAGE("Should now have updated count", aDocStat.nChar == nLen);

    CPPUNIT_ASSERT_MESSAGE("And cache is updated too", m_pDoc->GetDocStat().nChar == nLen);
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=40449 for motivation
void SwDocTest::testSwScanner()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    const SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    CPPUNIT_ASSERT_MESSAGE("Has Text Node", pTxtNode);

    //Use a temporary rtl::OUString as the arg, as that's the trouble behind
    //fdo#40449 and fdo#39365
    SwScanner aScanner(*pTxtNode,
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hello World")),
        0, 0, i18n::WordType::DICTIONARY_WORD, 0,
        RTL_CONSTASCII_LENGTH("Hello World"));

    bool bFirstOk = aScanner.NextWord();
    CPPUNIT_ASSERT_MESSAGE("First Token", bFirstOk);
    const rtl::OUString &rHello = aScanner.GetWord();
    CPPUNIT_ASSERT_MESSAGE("Should be Hello",
        rHello.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Hello")));

    bool bSecondOk = aScanner.NextWord();
    CPPUNIT_ASSERT_MESSAGE("Second Token", bSecondOk);
    const rtl::OUString &rWorld = aScanner.GetWord();
    CPPUNIT_ASSERT_MESSAGE("Should be World",
        rWorld.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("World")));
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=40599 for motivation
void SwDocTest::testGraphicAnchorDeletion()
{
    CPPUNIT_ASSERT_MESSAGE("Expected initial 0 count", m_pDoc->GetDocStat().nChar == 0);

    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    m_pDoc->InsertString(aPaM, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Paragraph 1")));
    m_pDoc->AppendTxtNode(*aPaM.GetPoint());

    m_pDoc->InsertString(aPaM, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("graphic anchor>><<graphic anchor")));
    SwNodeIndex nPara2 = aPaM.GetPoint()->nNode;
    m_pDoc->AppendTxtNode(*aPaM.GetPoint());

    m_pDoc->InsertString(aPaM, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Paragraph 3")));

    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), RTL_CONSTASCII_LENGTH("graphic anchor>>"));

    //Insert a graphic at X of >>X<< in paragraph 2
    SfxItemSet aFlySet(m_pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SwFmtAnchor aAnchor(FLY_AS_CHAR);
    aAnchor.SetAnchor(aPaM.GetPoint());
    aFlySet.Put(aAnchor);
    SwFlyFrmFmt *pFrame = m_pDoc->Insert(aPaM, rtl::OUString(), rtl::OUString(), NULL, &aFlySet, NULL, NULL);
    CPPUNIT_ASSERT_MESSAGE("Expected frame", pFrame != NULL);

    CPPUNIT_ASSERT_MESSAGE("Should be 1 graphic", m_pDoc->GetFlyCount(FLYCNTTYPE_GRF) == 1);

    //Delete >X<
    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(),
        RTL_CONSTASCII_LENGTH("graphic anchor>><")+1);
    aPaM.SetMark();
    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), RTL_CONSTASCII_LENGTH("graphic anchor>"));
    m_pDoc->DeleteRange(aPaM);

#ifdef DEBUG_AS_HTML
    {
        SvFileStream aPasteDebug(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "cppunitDEBUG.html")), STREAM_WRITE|STREAM_TRUNC);
        WriterRef xWrt;
        GetHTMLWriter( String(), String(), xWrt );
        SwWriter aDbgWrt( aPasteDebug, *m_pDoc );
        aDbgWrt.Write( xWrt );
    }
#endif

    CPPUNIT_ASSERT_MESSAGE("Should be 0 graphics", m_pDoc->GetFlyCount(FLYCNTTYPE_GRF) == 0);

    //Now, if instead we swap FLY_AS_CHAR (inline graphic) to FLY_AT_CHAR (anchored to character)
    //and repeat the above, graphic is *not* deleted, i.e. it belongs to the paragraph, not the
    //range to which its anchored, which is annoying.
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
    static rtl::OUString aText(RTL_CONSTASCII_USTRINGPARAM(
        "AAAAA BBBB CCC DD E \n"));
    int s = getRand(aText.getLength());
    int j = getRand(aText.getLength() - s);
    rtl::OUString aRet(aText.copy(s, j));
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

    for( sal_uInt16 rlm = 0;
         rlm < SAL_N_ELEMENTS(modes);
#ifdef COMPLEX // otherwise it returns at end of loop, so avoid "unreachable code" warning
         rlm++
#endif
        )
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

void SwDocTest::setUp()
{
    BootstrapFixture::setUp();

    SwGlobals::ensure();
    m_pDoc = new SwDoc;
    m_xDocShRef = new SwDocShell(m_pDoc, SFX_CREATE_MODE_EMBEDDED);
    m_xDocShRef->DoInitNew(0);
}

void SwDocTest::tearDown()
{
    m_xDocShRef.Clear();
    delete m_pDoc;

    BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDocTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
