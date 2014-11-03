/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <editeng/langitem.hxx>
#include <editeng/charhiddenitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <xmloff/odffields.hxx>

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
#include "IMark.hxx"

typedef tools::SvRef<SwDocShell> SwDocShellRef;

using namespace ::com::sun::star;

/* Implementation of Swdoc-Test class */

class SwDocTest : public test::BootstrapFixture
{
public:
    SwDocTest()
        : m_pDoc(NULL)
    {
    }

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void randomTest();
    void testPageDescName();
    void testFileNameFields();
    void testDocStat();
    void testModelToViewHelper();
    void testSwScanner();
    void testUserPerceivedCharCount();
    void testGraphicAnchorDeletion();
    void testTransliterate();
    void testMarkMove();

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(testTransliterate);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST(testDocStat);
    CPPUNIT_TEST(testModelToViewHelper);
    CPPUNIT_TEST(testSwScanner);
    CPPUNIT_TEST(testUserPerceivedCharCount);
    CPPUNIT_TEST(testGraphicAnchorDeletion);
    CPPUNIT_TEST(testMarkMove);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc *m_pDoc;
    SwDocShellRef m_xDocShRef;
};

void SwDocTest::testPageDescName()
{
    ShellResource aShellResources;

    std::vector<OUString> aResults;

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

//See https://bugs.libreoffice.org/show_bug.cgi?id=32463
void SwDocTest::testFileNameFields()
{
    //Here's a file name with some chars in it that will be %% encoded, when expanding
    //SwFileNameFields we want to restore the original readable filename
    utl::TempFile aTempFile(OUString("demo [name]"));
    aTempFile.EnableKillingFile();

    INetURLObject aTempFileURL(aTempFile.GetURL());
    OUString sFileURL = aTempFileURL.GetMainURL(INetURLObject::NO_DECODE);
    SfxMedium aDstMed(sFileURL, STREAM_STD_READWRITE);

    SfxFilter aFilter(
        OUString("Text"),
        OUString(), 0, 0, OUString(), 0, OUString(),
        OUString("TEXT"), OUString() );
    aDstMed.SetFilter(&aFilter);

    m_xDocShRef->DoSaveAs(aDstMed);
    m_xDocShRef->DoSaveCompleted(&aDstMed);

    const INetURLObject &rUrlObj = m_xDocShRef->GetMedium()->GetURLObject();

    SwFileNameFieldType aNameField(m_pDoc);

    {
        OUString sResult(aNameField.Expand(FF_NAME));
        OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
            true,INetURLObject::DECODE_WITH_CHARSET));
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        OUString sResult(aNameField.Expand(FF_PATHNAME));
        OUString sExpected(rUrlObj.GetFull());
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        OUString sResult(aNameField.Expand(FF_PATH));
        INetURLObject aTemp(rUrlObj);
        aTemp.removeSegment();
        OUString sExpected(aTemp.PathToFileName());
        CPPUNIT_ASSERT_MESSAGE("Expected Readable FileName", sResult == sExpected);
    }

    {
        OUString sResult(aNameField.Expand(FF_NAME_NOEXT));
        OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
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

    OUString sText("Hello World");
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
    OUString sALEF_QAMATS(ALEF_QAMATS, SAL_N_ELEMENTS(ALEF_QAMATS));
    sal_Int32 nGraphemeCount = pBreakIter->getGraphemeCount(sALEF_QAMATS);
    CPPUNIT_ASSERT_MESSAGE("Grapheme Count should be 1", nGraphemeCount == 1);

    //Surrogate pair example, one single unicode code-point (U+1D11E)
    //represented as two code units in UTF-16
    const sal_Unicode GCLEF[] = { 0xD834, 0xDD1E };
    OUString sGCLEF(GCLEF, SAL_N_ELEMENTS(GCLEF));
    sal_Int32 nCount = pBreakIter->getGraphemeCount(sGCLEF);
    CPPUNIT_ASSERT_MESSAGE("Surrogate Pair should be counted as single character", nCount == 1);
}

void SwDocTest::testModelToViewHelper()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    {
        SwFmtFtn aFtn;
        aFtn.SetNumStr(OUString("foo"));

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("AAAAA BBBBB "));
        SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
        sal_Int32 nPos = aPaM.GetPoint()->nContent.GetIndex();
        pTxtNode->InsertItem(aFtn, nPos, nPos);
        m_pDoc->InsertString(aPaM, OUString(" CCCCC "));
        nPos = aPaM.GetPoint()->nContent.GetIndex();
        pTxtNode->InsertItem(aFtn, nPos, nPos);
        m_pDoc->InsertString(aPaM, OUString(" DDDDD"));
        CPPUNIT_ASSERT(pTxtNode->GetTxt().getLength() == (4*5) + 5 + 2);

        //set start of selection to first B
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 6);
        aPaM.SetMark();
        //set end of selection to last C
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 14);
        //set character attribute hidden on range
        SvxCharHiddenItem aHidden(true, RES_CHRATR_HIDDEN);
        m_pDoc->InsertPoolItem(aPaM, aHidden, 0 );
        aPaM.DeleteMark();

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
        aPaM.DeleteMark();

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, PASSTHROUGH);
            OUString sViewText = aModelToViewHelper.getViewText();
            OUString sModelText = pTxtNode->GetTxt();
            CPPUNIT_ASSERT_EQUAL(sModelText, sViewText);
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | EXPANDFOOTNOTE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA BBBBB foo CCCCC foo DDDDD"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                    EXPANDFIELDS | EXPANDFOOTNOTE | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA BBBBB " + OUString(CHAR_ZWSP) + " CCCCC " + OUString(CHAR_ZWSP) + " DDDDD"),
                sViewText);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12),
                aModelToViewHelper.getFootnotePositions()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(20),
                aModelToViewHelper.getFootnotePositions()[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFieldPositions().size());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA BBBBB  CCCCC  DDDDD"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                EXPANDFIELDS | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(OUString("AAAAA BBBBB  CCCCC  DDDDD"),
                sViewText);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFieldPositions().size());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEINVISIBLE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA CCCCC " + OUString(CH_TXTATR_BREAKWORD) + " DDDDD"),
                sViewText);
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEDELETIONS);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAABB " + OUString(CH_TXTATR_BREAKWORD) + " CCCCC " + OUString(CH_TXTATR_BREAKWORD) + " DDDDD"),
                sViewText);
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEINVISIBLE | EXPANDFOOTNOTE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(OUString("AAAAA CCCCC foo DDDDD"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                EXPANDFIELDS | HIDEINVISIBLE | EXPANDFOOTNOTE | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA CCCCC " + OUString(CHAR_ZWSP) + " DDDDD"),
                sViewText);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12),
                aModelToViewHelper.getFootnotePositions()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFieldPositions().size());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEDELETIONS | EXPANDFOOTNOTE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAABB foo CCCCC foo DDDDD"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                EXPANDFIELDS | HIDEDELETIONS | EXPANDFOOTNOTE | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
               OUString("AAAABB " + OUString(CHAR_ZWSP) + " CCCCC " + OUString(CHAR_ZWSP) + " DDDDD"),
               sViewText);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7),
                aModelToViewHelper.getFootnotePositions()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15),
                aModelToViewHelper.getFootnotePositions()[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFieldPositions().size());
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, HIDEINVISIBLE | HIDEDELETIONS);
            OUString sViewText = aModelToViewHelper.getViewText();
            OUStringBuffer aBuffer;
            aBuffer.append("AAAACCCCC ");
            aBuffer.append(CH_TXTATR_BREAKWORD);
            aBuffer.append(" DDDDD");
            CPPUNIT_ASSERT_EQUAL(aBuffer.makeStringAndClear(), sViewText);
        }

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | HIDEINVISIBLE | HIDEDELETIONS | EXPANDFOOTNOTE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(OUString("AAAACCCCC foo DDDDD"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                EXPANDFIELDS | HIDEINVISIBLE | HIDEDELETIONS | EXPANDFOOTNOTE | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(sViewText,
                OUString("AAAACCCCC " + OUString(CHAR_ZWSP) + " DDDDD"));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10),
                aModelToViewHelper.getFootnotePositions()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFieldPositions().size());
        }

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("AAAAA"));
        IDocumentMarkAccess* pMarksAccess = m_pDoc->getIDocumentMarkAccess();
        sw::mark::IFieldmark *pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(
                pMarksAccess->makeNoTextFieldBookmark(aPaM, "test", ODF_FORMDROPDOWN));
        CPPUNIT_ASSERT(pFieldmark);
        uno::Sequence< OUString > vListEntries(1);
        vListEntries[0] = "BBBBB";
        (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] = uno::makeAny(vListEntries);
        (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] = uno::makeAny(sal_Int32(0));
        m_pDoc->InsertString(aPaM, OUString("CCCCC"));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11),
                pTxtNode->GetTxt().getLength());

        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode, EXPANDFIELDS | EXPANDFOOTNOTE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(OUString("AAAAABBBBBCCCCC"), sViewText);
        }
        {
            ModelToViewHelper aModelToViewHelper(*pTxtNode,
                EXPANDFIELDS | EXPANDFOOTNOTE | REPLACEMODE);
            OUString sViewText = aModelToViewHelper.getViewText();
            CPPUNIT_ASSERT_EQUAL(
                OUString("AAAAA" + OUString(CHAR_ZWSP) + "CCCCC"),
                sViewText);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
                aModelToViewHelper.getFootnotePositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                aModelToViewHelper.getFieldPositions().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5),
                aModelToViewHelper.getFieldPositions()[0]);
        }
    }
}

void SwDocTest::testSwScanner()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    CPPUNIT_ASSERT_MESSAGE("Has Text Node", pTxtNode);

    //See https://bugs.libreoffice.org/show_bug.cgi?id=40449
    //See https://bugs.libreoffice.org/show_bug.cgi?id=39365
    //Use a temporary OUString as the arg, as that's the trouble behind
    //fdo#40449 and fdo#39365
    {
        SwScanner aScanner(*pTxtNode,
            OUString("Hello World"),
            0, ModelToViewHelper(), i18n::WordType::DICTIONARY_WORD, 0,
            RTL_CONSTASCII_LENGTH("Hello World"));

        bool bFirstOk = aScanner.NextWord();
        CPPUNIT_ASSERT_MESSAGE("First Token", bFirstOk);
        const OUString &rHello = aScanner.GetWord();
        CPPUNIT_ASSERT_MESSAGE("Should be Hello",
            rHello == "Hello");

        bool bSecondOk = aScanner.NextWord();
        CPPUNIT_ASSERT_MESSAGE("Second Token", bSecondOk);
        const OUString &rWorld = aScanner.GetWord();
        CPPUNIT_ASSERT_MESSAGE("Should be World",
            rWorld == "World");
    }

    //See https://www.libreoffice.org/bugzilla/show_bug.cgi?id=45271
    {
        const sal_Unicode IDEOGRAPHICFULLSTOP_D[] = { 0x3002, 'D' };

        m_pDoc->InsertString(aPaM, OUString(IDEOGRAPHICFULLSTOP_D,
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
        m_pDoc->InsertString(aPaM, OUString(test,
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
    //See https://bugs.libreoffice.org/show_bug.cgi?id=53399
    {
        SwDocStat aDocStat;

        const sal_Unicode aShouldBeThree[] = {
            0x0053, 0x0068, 0x006F, 0x0075, 0x006C, 0x0064, 0x0020,
            0x2018, 0x0062, 0x0065, 0x0020, 0x0074, 0x0068, 0x0072,
            0x0065, 0x0065, 0x2019
        };

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString(aShouldBeThree, SAL_N_ELEMENTS(aShouldBeThree)));
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
        m_pDoc->InsertString(aPaM, OUString(aShouldBeFive, SAL_N_ELEMENTS(aShouldBeFive)));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(aShouldBeFive));
        CPPUNIT_ASSERT_MESSAGE("Should be 5", aDocStat.nWord == 5);
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=49629
    {
        SwDocStat aDocStat;

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("Apple"));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        sal_Int32 nPos = aPaM.GetPoint()->nContent.GetIndex();
        SwFmtFtn aFtn;
        aFtn.SetNumStr(OUString("banana"));
        SwTxtAttr* pTA = pTxtNode->InsertItem(aFtn, nPos, nPos);
        CPPUNIT_ASSERT(pTA);
        CPPUNIT_ASSERT(pTxtNode->Len() == 6); //Apple + 0x02
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("footnote should be expanded", aDocStat.nChar == 11);

        const sal_Int32 nNextPos = aPaM.GetPoint()->nContent.GetIndex();
        CPPUNIT_ASSERT(nNextPos == nPos+1);
        SwFmtRefMark aRef(OUString("refmark"));
        pTA = pTxtNode->InsertItem(aRef, nNextPos, nNextPos);
        CPPUNIT_ASSERT(pTA);

        aDocStat.Reset();
        pTxtNode->SetWordCountDirty(true);
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("refmark anchor should not be counted", aDocStat.nChar == 11);

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("Apple"));

        DateTime aDate(DateTime::SYSTEM);
        SwPostItField aPostIt(
            (SwPostItFieldType*)m_pDoc->GetSysFldType(RES_POSTITFLD), OUString("An Author"),
            OUString("Some Text"), OUString("Initials"), OUString("Name"), aDate );
        m_pDoc->InsertPoolItem(aPaM, SwFmtFld(aPostIt), 0);

        m_pDoc->InsertString(aPaM, OUString("Apple"));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 1);
        CPPUNIT_ASSERT_MESSAGE("postit anchor should effectively not exist", aDocStat.nChar == 10);
        CPPUNIT_ASSERT(pTxtNode->Len() == 11);

        aDocStat.Reset();
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=46757
    {
        SwDocStat aDocStat;

        const char aString[] = "Lorem ipsum";
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString(aString));
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
        //"real underlying text should be the same"
        CPPUNIT_ASSERT_EQUAL(pTxtNode->GetTxt(), OUString(aString));

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

        OUString sLorem = pTxtNode->GetTxt();
        CPPUNIT_ASSERT(sLorem == "Lorem");

        const SwRedlineTbl& rTbl = m_pDoc->GetRedlineTbl();

        SwNodes& rNds = m_pDoc->GetNodes();
        CPPUNIT_ASSERT(rTbl.size() == 1);

        SwNodeIndex* pNodeIdx = rTbl[0]->GetContentIdx();
        CPPUNIT_ASSERT(pNodeIdx);

        pTxtNode = rNds[ pNodeIdx->GetIndex() + 1 ]->GetTxtNode();        //first deleted txtnode
        CPPUNIT_ASSERT(pTxtNode);

        OUString sIpsum = pTxtNode->GetTxt();
        CPPUNIT_ASSERT(sIpsum == " ipsum");

        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len()); //word-counting the text should only count the non-deleted text, and this whole chunk should be ignored
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), aDocStat.nChar);

        // https://bugs.libreoffice.org/show_bug.cgi?id=68347 we do want to count
        // redline *added* text though
        m_pDoc->SetRedlineMode(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_DELETE|nsRedlineMode_t::REDLINE_SHOW_INSERT);
        aPaM.DeleteMark();
        aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), 0);
        m_pDoc->InsertString(aPaM, "redline-new-text ");
        aDocStat.Reset();
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->SetWordCountDirty(true);
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), aDocStat.nWord);
        //redline-new-text Lorem ipsum
        //+++++++++++++++++     ------
        //select start of original text and part of deleted text
        aDocStat.Reset();
        pTxtNode->CountWords(aDocStat, 17, 25);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(5), aDocStat.nChar);
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=38983
    {
        SwDocStat aDocStat;

        OUString sTemplate("ThisXis a test.");

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replace('X', ' '));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 12 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(OUString('X'), OUString(" = ")));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(OUString('X'), OUString(" _ ")));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(OUString('X'), OUString(" -- ")));
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
        //and endash as word separators for word-counting
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
        OUString sChunk(aChunk, SAL_N_ELEMENTS(aChunk));
        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, sTemplate.replaceAll(OUString('X'), sChunk));
        pTxtNode = aPaM.GetNode()->GetTxtNode();
        pTxtNode->CountWords(aDocStat, 0, pTxtNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();
    }
}

//See https://bugs.libreoffice.org/show_bug.cgi?id=40599
void SwDocTest::testGraphicAnchorDeletion()
{
    CPPUNIT_ASSERT_MESSAGE("Expected initial 0 count", m_pDoc->GetDocStat().nChar == 0);

    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    m_pDoc->InsertString(aPaM, OUString("Paragraph 1"));
    m_pDoc->AppendTxtNode(*aPaM.GetPoint());

    m_pDoc->InsertString(aPaM, OUString("graphic anchor>><<graphic anchor"));
    SwNodeIndex nPara2 = aPaM.GetPoint()->nNode;
    m_pDoc->AppendTxtNode(*aPaM.GetPoint());

    m_pDoc->InsertString(aPaM, OUString("Paragraph 3"));

    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetCntntNode(), RTL_CONSTASCII_LENGTH("graphic anchor>>"));

    //Insert a graphic at X of >>X<< in paragraph 2
    SfxItemSet aFlySet(m_pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SwFmtAnchor aAnchor(FLY_AS_CHAR);
    aAnchor.SetAnchor(aPaM.GetPoint());
    aFlySet.Put(aAnchor);
    SwFlyFrmFmt *pFrame = m_pDoc->Insert(aPaM, OUString(), OUString(), NULL, &aFlySet, NULL, NULL);
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
        SvFileStream aPasteDebug(OUString("cppunitDEBUG.html"), STREAM_WRITE|STREAM_TRUNC);
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

static OUString
getRandString()
{
    OUString aText("AAAAA BBBB CCC DD E \n");
    int s = getRand(aText.getLength());
    int j = getRand(aText.getLength() - s);
    OUString aRet(aText.copy(s, j));
    if (!getRand(5))
        aRet += OUString('\n');
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
        SW_MOD()->SetRedlineAuthor(OUString::createFromAscii(authors[0]));

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
                SW_MOD()->SetRedlineAuthor(OUString::createFromAscii(authors[a]));
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
        OStringBuffer aBuffer("nodes-");
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

static OUString
translitTest(SwDoc & rDoc, SwPaM & rPaM, sal_uInt32 const nType)
{
    utl::TransliterationWrapper aTrans(
            ::comphelper::getProcessComponentContext(), nType);
    rDoc.TransliterateText(rPaM, aTrans);
    return rPaM.GetTxt();
}

void SwDocTest::testTransliterate()
{
    // just some simple test to see if it's totally broken
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    m_pDoc->InsertString(aPaM, OUString("foobar"));
    aPaM.SetMark();
    aPaM.GetPoint()->nContent = 0;
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), aPaM.GetTxt());

    CPPUNIT_ASSERT_EQUAL(OUString("FOOBAR"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModules_LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Foobar"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModulesExtra::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("fOOBAR"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModulesExtra::TOGGLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModules_UPPERCASE_LOWERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Foobar"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModulesExtra::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Foobar"),
            translitTest(*m_pDoc, aPaM,
                i18n::TransliterationModules_HIRAGANA_KATAKANA));
}

void SwDocTest::testMarkMove()
{
    IDocumentMarkAccess* pMarksAccess = m_pDoc->getIDocumentMarkAccess();

    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        m_pDoc->InsertString(aPaM, OUString("Paragraph 1"));
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, OUString("Para1"), IDocumentMarkAccess::BOOKMARK);

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("Paragraph 2"));
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, OUString("Para2"), IDocumentMarkAccess::BOOKMARK);

        m_pDoc->AppendTxtNode(*aPaM.GetPoint());
        m_pDoc->InsertString(aPaM, OUString("Paragraph 3"));
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, OUString("Para3"), IDocumentMarkAccess::BOOKMARK);
    }

    // join paragraph 2 and 3 and check
    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -2);
        SwTxtNode* pParaNode2 = dynamic_cast<SwTxtNode*>(&aIdx.GetNode());
        pParaNode2->JoinNext();
    }
    ::sw::mark::IMark* pBM1 = pMarksAccess->findMark("Para1")->get();
    ::sw::mark::IMark* pBM2 = pMarksAccess->findMark("Para2")->get();
    ::sw::mark::IMark* pBM3 = pMarksAccess->findMark("Para3")->get();

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 0 , pBM1->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 11, pBM1->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex(),
        pBM1->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 0 , pBM2->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 11, pBM2->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkStart().nNode.GetIndex(),
        pBM2->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 11, pBM3->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 22, pBM3->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM3->GetMarkStart().nNode.GetIndex(),
        pBM3->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex()+1,
        pBM2->GetMarkStart().nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkStart().nNode.GetIndex(),
        pBM3->GetMarkStart().nNode.GetIndex());

    // cut some text
    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx, aIdx, -1);
        aPaM.GetPoint()->nContent += 5;
        aPaM.GetMark()->nContent += 6;
        m_pDoc->DeleteAndJoin(aPaM);
    }
    pBM1 = pMarksAccess->findMark("Para1")->get();
    pBM2 = pMarksAccess->findMark("Para2")->get();
    pBM3 = pMarksAccess->findMark("Para3")->get();

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 0, pBM1->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 6, pBM1->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex(),
        pBM1->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 6, pBM2->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 12, pBM2->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkStart().nNode.GetIndex(),
        pBM2->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 12, pBM3->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 23, pBM3->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM3->GetMarkStart().nNode.GetIndex(),
        pBM3->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex(),
        pBM2->GetMarkStart().nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkStart().nNode.GetIndex(),
        pBM3->GetMarkStart().nNode.GetIndex());

    // split the paragraph
    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
        SwPosition aPos(aIdx);
        aPos.nContent += 8;
        m_pDoc->SplitNode(aPos, false);
    }
    pBM1 = pMarksAccess->findMark("Para1")->get();
    pBM2 = pMarksAccess->findMark("Para2")->get();
    pBM3 = pMarksAccess->findMark("Para3")->get();

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 0, pBM1->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 6, pBM1->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex(),
        pBM1->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 6, pBM2->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 4, pBM2->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkStart().nNode.GetIndex()+1,
        pBM2->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL((sal_Int32) 4, pBM3->GetMarkStart().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 15, pBM3->GetMarkEnd().nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM3->GetMarkStart().nNode.GetIndex(),
        pBM3->GetMarkEnd().nNode.GetIndex());

    CPPUNIT_ASSERT_EQUAL(
        pBM1->GetMarkStart().nNode.GetIndex(),
        pBM2->GetMarkStart().nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        pBM2->GetMarkEnd().nNode.GetIndex(),
        pBM3->GetMarkEnd().nNode.GetIndex());
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
