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

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

#include <editeng/langitem.hxx>
#include <editeng/charhiddenitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include "breakit.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "docstat.hxx"
#include "docufld.hxx"
#include "fmtanchr.hxx"
#include "init.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "shellres.hxx"
#include "swcrsr.hxx"
#include "swscanner.hxx"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "fmtftn.hxx"
#include "fmtrfmrk.hxx"
#include "fmtfld.hxx"
#include "redline.hxx"
#include "docary.hxx"
#include "modeltoviewhelper.hxx"
#include "scriptinfo.hxx"

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
    void testModelToViewHelper();
    void testSwScanner();
    void testUserPerceivedCharCount();
    void testGraphicAnchorDeletion();

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST(testDocStat);
    CPPUNIT_TEST(testModelToViewHelper);
    CPPUNIT_TEST(testSwScanner);
    CPPUNIT_TEST(testUserPerceivedCharCount);
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

//See https://bugs.freedesktop.org/show_bug.cgi?id=32463
void SwDocTest::testFileNameFields()
{
    //Here's a file name with some chars in it that will be %% encoded, when expanding
    //SwFileNameFields we want to restore the original readable filename
    utl::TempFile aTempFile(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("demo [name]")));
    aTempFile.EnableKillingFile();

    INetURLObject aTempFileURL(aTempFile.GetURL());
    String sFileURL = aTempFileURL.GetMainURL(INetURLObject::NO_DECODE);
    SfxMedium aDstMed(sFileURL, STREAM_STD_READWRITE);

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

//For UI character counts we should follow UAX#29 and display the user
//perceived characters, not the number of codepoints, nor the number of code
//units http://unicode.org/reports/tr29/
void SwDocTest::testUserPerceivedCharCount()
{
    SwBreakIt *pBreakIter = SwBreakIt::Get();

    //Grapheme example, two different unicode code-points perceived by the user as a single
    //glyph
    const sal_Unicode ALEF_QAMATS [] = { 0x05D0, 0x05B8 };
    ::rtl::OUString sALEF_QAMATS(ALEF_QAMATS, SAL_N_ELEMENTS(ALEF_QAMATS));
    sal_Int32 nGraphemeCount = pBreakIter->getGraphemeCount(sALEF_QAMATS);
    CPPUNIT_ASSERT_MESSAGE("Grapheme Count should be 1", nGraphemeCount == 1);

    //Surrogate pair example, one single unicode code-point (U+1D11E)
    //represented as two code units in UTF-16
    const sal_Unicode GCLEF[] = { 0xD834, 0xDD1E };
    ::rtl::OUString sGCLEF(GCLEF, SAL_N_ELEMENTS(GCLEF));
    sal_Int32 nCount = pBreakIter->getGraphemeCount(sGCLEF);
    CPPUNIT_ASSERT_MESSAGE("Surrogate Pair should be counted as single character", nCount == 1);
}

void SwDocTest::testModelToViewHelper()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    {
        SwFmtFtn aFtn;
        aFtn.SetNumStr(rtl::OUString("foo"));

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString("AAAAA BBBBB "));
        SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
        xub_StrLen nPos = aPaM.GetPoint()->nContent.GetIndex();
        pTxtNode->InsertItem(aFtn, nPos, nPos);
        m_pDoc->InsertString(aPaM, rtl::OUString(" CCCCC "));
        nPos = aPaM.GetPoint()->nContent.GetIndex();
        pTxtNode->InsertItem(aFtn, nPos, nPos);
        m_pDoc->InsertString(aPaM, rtl::OUString(" DDDDD"));
        CPPUNIT_ASSERT(pTxtNode->GetTxt().Len() == (4*5) + 5 + 2);

        //set start of selection to first B
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 6);
        aPaM.SetMark();
        //set end of selection to last C
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 14);
        //set character attribute hidden on range
        SvxCharHiddenItem aHidden(true, RES_CHRATR_HIDDEN);
        m_pDoc->InsertPoolItem(aPaM, aHidden, 0 );

        //turn on red-lining and show changes
        m_pDoc->SetRedlineMode(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_DELETE|nsRedlineMode_t::REDLINE_SHOW_INSERT);
        CPPUNIT_ASSERT_MESSAGE("redlining should be on", m_pDoc->IsRedlineOn());
        CPPUNIT_ASSERT_MESSAGE("redlines should be visible", IDocumentRedlineAccess::IsShowChanges(m_pDoc->GetRedlineMode()));

        //set start of selection to last A
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 4);
        aPaM.SetMark();
        //set end of selection to second last B
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 9);
        m_pDoc->DeleteAndJoin(aPaM);    //redline-aware deletion api

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, PASSTHROUGH);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            rtl::OUString sModelText = pTxtNode->GetTxt();
            CPPUNIT_ASSERT(sViewText == sModelText);
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT(sViewText == "AAAAA BBBBB foo CCCCC foo DDDDD");
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEINVISIBLE);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            rtl::OUStringBuffer aBuffer;
            aBuffer.append("AAAAA CCCCC ");
            aBuffer.append(CH_TXTATR_BREAKWORD);
            aBuffer.append(" DDDDD");
            CPPUNIT_ASSERT(sViewText == aBuffer.makeStringAndClear());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEREDLINED);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            rtl::OUStringBuffer aBuffer;
            aBuffer.append("AAAABB ");
            aBuffer.append(CH_TXTATR_BREAKWORD);
            aBuffer.append(" CCCCC ");
            aBuffer.append(CH_TXTATR_BREAKWORD);
            aBuffer.append(" DDDDD");
            CPPUNIT_ASSERT(sViewText == aBuffer.makeStringAndClear());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEINVISIBLE);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT(sViewText == "AAAAA CCCCC foo DDDDD");
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEREDLINED);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT(sViewText == "AAAABB foo CCCCC foo DDDDD");
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEINVISIBLE | HIDEREDLINED);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            rtl::OUStringBuffer aBuffer;
            aBuffer.append("AAAACCCCC ");
            aBuffer.append(CH_TXTATR_BREAKWORD);
            aBuffer.append(" DDDDD");
            CPPUNIT_ASSERT(sViewText == aBuffer.makeStringAndClear());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEINVISIBLE | HIDEREDLINED);
            rtl::OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT(sViewText == "AAAACCCCC foo DDDDD");
        }
    }
}

void SwDocTest::testSwScanner()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    CPPUNIT_ASSERT_MESSAGE("Has Text Node", pTxtNode);

    //See https://bugs.freedesktop.org/show_bug.cgi?id=40449
    //See https://bugs.freedesktop.org/show_bug.cgi?id=39365
    //Use a temporary rtl::OUString as the arg, as that's the trouble behind
    //fdo#40449 and fdo#39365
    {
        SwScanner aScanner(*pTxtNode,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hello World")),
            0, ModelToViewHelper(), i18n::WordType::DICTIONARY_WORD, 0,
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

    //See https://www.libreoffice.org/bugzilla/show_bug.cgi?id=45271
    {
        const sal_Unicode IDEOGRAPHICFULLSTOP_D[] = { 0x3002, 'D' };

        m_pDoc->InsertString(aPaM, rtl::OUString(IDEOGRAPHICFULLSTOP_D,
            SAL_N_ELEMENTS(IDEOGRAPHICFULLSTOP_D)));

        SvxLanguageItem aCJKLangItem( LANGUAGE_CHINESE_SIMPLIFIED, RES_CHRATR_CJK_LANGUAGE );
        SvxLanguageItem aWestLangItem( LANGUAGE_ENGLISH_US, RES_CHRATR_LANGUAGE );
        m_pDoc->InsertPoolItem(aPaM, aCJKLangItem, 0 );
        m_pDoc->InsertPoolItem(aPaM, aWestLangItem, 0 );

        SwDocStat aDocStat;
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(IDEOGRAPHICFULLSTOP_D));

        CPPUNIT_ASSERT_MESSAGE("Should be 2", aDocStat.nChar == 2);
        CPPUNIT_ASSERT_MESSAGE("Should be 2", aDocStat.nCharExcludingSpaces == 2);
    }
    {
        const sal_Unicode test[] =
        {
            0x3053, 0x306E, 0x65E5, 0x672C, 0x8A9E, 0x306F, 0x6B63, 0x3057,
            0x304F, 0x6570, 0x3048, 0x3089, 0x308C, 0x308B, 0x3067, 0x3057,
            0x3087, 0x3046, 0x304B, 0x3002, 0x0041, 0x006E, 0x0064, 0x0020,
            0x006C, 0x0065, 0x0074, 0x0027, 0x0073, 0x0020, 0x0074, 0x0068,
            0x0072, 0x006F, 0x0077, 0x0020, 0x0073, 0x006F, 0x006D, 0x0065,
            0x0020, 0x0045, 0x006E, 0x0067, 0x006C, 0x0069, 0x0073, 0x0068,
            0x0020, 0x0069, 0x006E, 0x0020, 0x0074, 0x006F, 0x0020, 0x006D,
            0x0061, 0x006B, 0x0065, 0x0020, 0x0069, 0x0074, 0x0020, 0x0069,
            0x006E, 0x0074, 0x0065, 0x0072, 0x0065, 0x0073, 0x0074, 0x0069,
            0x006E, 0x0067, 0x002E, 0x0020, 0x0020, 0x305D, 0x3057, 0x3066,
            0x3001, 0x307E, 0x305F, 0x65E5, 0x672C, 0x8A9E, 0x3000, 0x3000,
            0x3067, 0x3082, 0x4ECA, 0x56DE, 0x306F, 0x7A7A, 0x767D, 0x3092,
            0x3000, 0x3000, 0x5165, 0x308C, 0x307E, 0x3057, 0x305F, 0x3002,
            0x0020, 0x0020, 0x0053, 0x006F, 0x0020, 0x0068, 0x006F, 0x0077,
            0x0020, 0x0064, 0x006F, 0x0065, 0x0073, 0x0020, 0x0074, 0x0068,
            0x0069, 0x0073, 0x0020, 0x0064, 0x006F, 0x003F, 0x0020, 0x0020
        };
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString(test,
            SAL_N_ELEMENTS(test)));

        SvxLanguageItem aCJKLangItem( LANGUAGE_JAPANESE, RES_CHRATR_CJK_LANGUAGE );
        SvxLanguageItem aWestLangItem( LANGUAGE_ENGLISH_US, RES_CHRATR_LANGUAGE );
        m_pDoc->InsertPoolItem(aPaM, aCJKLangItem, 0 );
        m_pDoc->InsertPoolItem(aPaM, aWestLangItem, 0 );

        SwDocStat aDocStat;
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(test));
        CPPUNIT_ASSERT_MESSAGE("58 words", aDocStat.nWord == 58);
        CPPUNIT_ASSERT_MESSAGE("43 Asian characters and Korean syllables", aDocStat.nAsianWord == 43);
        CPPUNIT_ASSERT_MESSAGE("105 non-whitespace chars", aDocStat.nCharExcludingSpaces == 105);
        CPPUNIT_ASSERT_MESSAGE("128 characters", aDocStat.nChar == 128);
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=89042
    //See https://bugs.freedesktop.org/show_bug.cgi?id=53399
    {
        SwDocStat aDocStat;

        const sal_Unicode aShouldBeThree[] = {
            0x0053, 0x0068, 0x006F, 0x0075, 0x006C, 0x0064, 0x0020,
            0x2018, 0x0062, 0x0065, 0x0020, 0x0074, 0x0068, 0x0072,
            0x0065, 0x0065, 0x2019
        };

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString(aShouldBeThree, SAL_N_ELEMENTS(aShouldBeThree)));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(aShouldBeThree));
        CPPUNIT_ASSERT_MESSAGE("Should be 3", aDocStat.nWord == 3);

        const sal_Unicode aShouldBeFive[] = {
            // f    r       e       n       c       h       space
            0x0046, 0x0072, 0x0065, 0x006E, 0x0063, 0x0068, 0x0020,
            // <<   nbsp    s       a       v       o       i
            0x00AB, 0x00A0, 0x0073, 0x0061, 0x0076, 0x006F, 0x0069,
            // r    nnbsp   c       a       l       c       u
            0x0072, 0x202f, 0x0063, 0x0061, 0x006C, 0x0063, 0x0075,
            // l    e       r       idspace >>
            0x006C, 0x0065, 0x0072, 0x3000, 0x00BB
        };

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString(aShouldBeFive, SAL_N_ELEMENTS(aShouldBeFive)));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(aShouldBeFive));
        CPPUNIT_ASSERT_MESSAGE("Should be 5", aDocStat.nWord == 5);
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=49629
    {
        SwDocStat aDocStat;

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString("Apple"));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        xub_StrLen nPos = aPaM.GetPoint()->nContent.GetIndex();
        SwFmtFtn aFtn;
        aFtn.SetNumStr(rtl::OUString("banana"));
        SwTxtAttr* pTA = pTxtNode->InsertItem(aFtn, nPos, nPos);
        CPPUNIT_ASSERT(pTA);
        CPPUNIT_ASSERT(pTxtNode->Len() == 6); //Apple + 0x02
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("footnote should be expanded", aDocStat.nChar == 11);

        xub_StrLen nNextPos = aPaM.GetPoint()->nContent.GetIndex();
        CPPUNIT_ASSERT(nNextPos == nPos+1);
        SwFmtRefMark aRef(rtl::OUString("refmark"));
        pTA = pTxtNode->InsertItem(aRef, nNextPos, nNextPos);
        CPPUNIT_ASSERT(pTA);

        aDocStat.Reset();
        pTxtNode->SetWordCountDirty(true);
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("refmark anchor should not be counted", aDocStat.nChar == 11);

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString("Apple"));

        DateTime aDate(DateTime::SYSTEM);
        SwPostItField aPostIt(
            (SwPostItFieldType*)m_pDoc->GetSysFldType(RES_POSTITFLD), rtl::OUString("An Author"),
            rtl::OUString("Some Text"), rtl::OUString("Initials"), OUString("Name"), aDate );
        m_pDoc->InsertPoolItem(aPaM, SwFmtFld(aPostIt), 0);

        m_pDoc->InsertString(aPaM, rtl::OUString("Apple"));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("postit anchor should effectively not exist", aDocStat.nChar == 10);
        CPPUNIT_ASSERT(pTxtNode->Len() == 11);

        aDocStat.Reset();
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=46757
    {
        SwDocStat aDocStat;

        const char aString[] = "Lorem ipsum";
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, rtl::OUString(aString));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(2));

        //turn on red-lining and show changes
        m_pDoc->SetRedlineMode(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_DELETE|nsRedlineMode_t::REDLINE_SHOW_INSERT);
        CPPUNIT_ASSERT_MESSAGE("redlining should be on", m_pDoc->IsRedlineOn());
        CPPUNIT_ASSERT_MESSAGE("redlines should be visible", IDocumentRedlineAccess::IsShowChanges(m_pDoc->GetRedlineMode()));

        //delete everything except the first word
        aPaM.SetMark(); //set start of selection to current pos
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 5);   //set end of selection to fifth char of current node
        m_pDoc->DeleteAndJoin(aPaM);    //redline-aware deletion api
        CPPUNIT_ASSERT_MESSAGE("real underlying text should be the same", pTxtNode->GetTxt().EqualsAscii(aString));

        aDocStat.Reset();
        pTxtNode->SetWordCountDirty(true);
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len()); //but word-counting the text should only count the non-deleted text
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(1));

        pTxtNode->SetWordCountDirty(true);

        //keep red-lining on but hide changes
        m_pDoc->SetRedlineMode(nsRedlineMode_t::REDLINE_ON);
        CPPUNIT_ASSERT_MESSAGE("redlining should be still on", m_pDoc->IsRedlineOn());
        CPPUNIT_ASSERT_MESSAGE("redlines should be invisible", !IDocumentRedlineAccess::IsShowChanges(m_pDoc->GetRedlineMode()));

        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len()); //but word-counting the text should only count the non-deleted text
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(1));

        rtl::OUString sLorem = pTxtNode->GetTxt();
        CPPUNIT_ASSERT(sLorem == "Lorem");

        const SwRedlineTbl& rTbl = m_pDoc->GetRedlineTbl();

        SwNodes& rNds = m_pDoc->GetNodes();
        CPPUNIT_ASSERT(rTbl.size() == 1);

        SwNodeIndex* pNodeIdx = rTbl[0]->GetContentIdx();
        CPPUNIT_ASSERT(pNodeIdx);

        pTxtNode = rNds[ pNodeIdx->GetIndex() + 1 ]->GetTxtNode();        //first deleted txtnode
        CPPUNIT_ASSERT(pTxtNode);

        rtl::OUString sIpsum = pTxtNode->GetTxt();
        CPPUNIT_ASSERT(sIpsum == " ipsum");

        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len()); //word-counting the text should only count the non-deleted text, and this whole chunk should be ignored
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(0));
        CPPUNIT_ASSERT_EQUAL(aDocStat.nChar, static_cast<sal_uLong>(0));
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=38983
    {
        SwDocStat aDocStat;

        rtl::OUString sTemplate("ThisXis a test.");

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', ' '));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 12 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(rtl::OUString('X'), rtl::OUString(" = ")));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(rtl::OUString('X'), rtl::OUString(" _ ")));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(rtl::OUString('X'), rtl::OUString(" -- ")));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 14 &&
                       aDocStat.nChar == 18);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', '_'));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', '-'));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', 0x2012));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', 0x2015));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        //But default configuration should, msword-alike treak emdash
        //and endash as word seperators for word-counting
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', 0x2013));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', 0x2014));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        const sal_Unicode aChunk[] = {' ', 0x2013, ' '};
        rtl::OUString sChunk(aChunk, SAL_N_ELEMENTS(aChunk));
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(rtl::OUString('X'), sChunk));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();
    }
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=40599
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
    rtl::OUString aText("AAAAA BBBB CCC DD E \n");
    int s = getRand(aText.getLength());
    int j = getRand(aText.getLength() - s);
    rtl::OUString aRet(aText.copy(s, j));
    if (!getRand(5))
        aRet += rtl::OUString(sal_Unicode('\n'));
//    fprintf (stderr, "rand string '%s'\n", OUStringToOString(aRet, RTL_TEXTENCODING_UTF8).getStr());
    return aRet;
}

static SwPosition
getRandomPosition(SwDoc *pDoc, int /* nOffset */)
{
    const SwPosition aPos(pDoc->GetNodes().GetEndOfContent());
	sal_uLong nNodes = aPos.nNode.GetNode().GetIndex() - aPos.nNode.GetNode().StartOfSectionIndex();
	sal_uLong n = (rand() * nNodes) / RAND_MAX;
	SwPaM pam(aPos);
	for (sal_uLong i = 0; i < n; ++i) {
		pam.Move(fnMoveBackward, fnGoNode);
	}
    return *pam.GetPoint();
}

void SwDocTest::randomTest()
{
    CPPUNIT_ASSERT_MESSAGE("SwDoc::IsRedlineOn()", !m_pDoc->IsRedlineOn());
    RedlineMode_t modes[] = {
        nsRedlineMode_t::REDLINE_ON,
        nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_NONE,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE | nsRedlineMode_t::REDLINE_SHOW_MASK,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT,
        nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_DELETE
    };
    static const char *authors[] = {
        "Jim", "Bob", "JimBobina", "Helga", "Gertrude", "Spagna", "Hurtleweed"
    };

    for( sal_uInt16 rlm = 0; rlm < SAL_N_ELEMENTS(modes); rlm++ )
    {
        m_pDoc->ClearDoc();

        // setup redlining
        m_pDoc->SetRedlineMode(modes[rlm]);
        SW_MOD()->SetRedlineAuthor(rtl::OUString::createFromAscii(authors[0]));

        for( int i = 0; i < 2000; i++ )
        {
            SwPaM aPam(m_pDoc->GetNodes());
            SwCursor aCrs(getRandomPosition(m_pDoc, i/20), 0, false);
            aCrs.SetMark();

            switch (getRand (i < 50 ? 3 : 6)) {
            // insert ops first
            case 0: {
                if (!m_pDoc->InsertString(aCrs, getRandString())) {
//                    fprintf (stderr, "failed to insert string !\n");
                }
                break;
            }
            case 1:
                break;
            case 2: { // switch author
                int a = getRand(SAL_N_ELEMENTS(authors));
                SW_MOD()->SetRedlineAuthor(rtl::OUString::createFromAscii(authors[a]));
                break;
            }

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

            case 5:
                break;

            // undo / redo ?
            default:
                break;
            }
        }

// Debug / verify the produced document has real content
#if 0
        rtl::OStringBuffer aBuffer("nodes-");
        aBuffer.append(sal_Int32(rlm));
        aBuffer.append(".xml");

        xmlTextWriterPtr writer;
        writer = xmlNewTextWriterFilename( aBuffer.makeStringAndClear().getStr(), 0 );
        xmlTextWriterStartDocument( writer, NULL, NULL, NULL );
        m_pDoc->dumpAsXml(writer);
        xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
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
