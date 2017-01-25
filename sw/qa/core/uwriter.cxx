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
#include <comphelper/random.hxx>
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
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include "cellfml.hxx"
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
#include <swdll.hxx>
#include "swtypes.hxx"
#include "fmtftn.hxx"
#include "fmtrfmrk.hxx"
#include <fmtinfmt.hxx>
#include <fchrfmt.hxx>
#include "fmtfld.hxx"
#include "redline.hxx"
#include "docary.hxx"
#include "modeltoviewhelper.hxx"
#include "scriptinfo.hxx"
#include "IMark.hxx"
#include "ring.hxx"
#include "calbck.hxx"
#include "pagedesc.hxx"
#include "calc.hxx"

#include <unotbl.hxx>

typedef tools::SvRef<SwDocShell> SwDocShellRef;

using namespace ::com::sun::star;

/* Implementation of Swdoc-Test class */

class SwDocTest : public test::BootstrapFixture
{
public:
    SwDocTest()
        : m_pDoc(nullptr)
    {
    }

    virtual void setUp() override;
    virtual void tearDown() override;

    void randomTest();
    void testPageDescName();
    void testFileNameFields();
    void testDocStat();
    void testModelToViewHelperPassthrough();
    void testModelToViewHelperExpandFieldsExpandFootnote();
    void testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode();
    void testModelToViewHelperExpandFields();
    void testModelToViewHelperExpandFieldsReplaceMode();
    void testModelToViewHelperExpandFieldsHideInvisible();
    void testModelToViewHelperExpandFieldsHideRedlined();
    void testModelToViewHelperExpandFieldsHideInvisibleExpandFootnote();
    void testModelToViewHelperExpandFieldsHideInvisibleExpandFootnoteReplaceMode();
    void testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnote();
    void testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnoteReplaceMode();
    void testModelToViewHelperHideInvisibleHideRedlined();
    void testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnote();
    void testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnoteReplaceMode();
    void testModelToViewHelperExpandFieldsExpandFootnote2();
    void testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode2();
    void testSwScanner();
    void testUserPerceivedCharCount();
    void testMergePortionsDeleteNotSorted();
    void testGraphicAnchorDeletion();
    void testTransliterate();
    void testMarkMove();
    void testFormulas();
    void testIntrusiveRing();
    void testClientModify();
    void test64kPageDescs();
    void testTdf92308();
    void testTableCellComparison();

    CPPUNIT_TEST_SUITE(SwDocTest);

    CPPUNIT_TEST(testTransliterate);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST(testPageDescName);
    CPPUNIT_TEST(testFileNameFields);
    CPPUNIT_TEST(testDocStat);
    CPPUNIT_TEST(testModelToViewHelperPassthrough);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsExpandFootnote);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode);
    CPPUNIT_TEST(testModelToViewHelperExpandFields);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsReplaceMode);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideInvisible);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideRedlined);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideInvisibleExpandFootnote);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideInvisibleExpandFootnoteReplaceMode);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnote);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnoteReplaceMode);
    CPPUNIT_TEST(testModelToViewHelperHideInvisibleHideRedlined);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnote);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnoteReplaceMode);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsExpandFootnote2);
    CPPUNIT_TEST(testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode2);
    CPPUNIT_TEST(testSwScanner);
    CPPUNIT_TEST(testUserPerceivedCharCount);
    CPPUNIT_TEST(testMergePortionsDeleteNotSorted);
    CPPUNIT_TEST(testGraphicAnchorDeletion);
    CPPUNIT_TEST(testMarkMove);
    CPPUNIT_TEST(testFormulas);
    CPPUNIT_TEST(testIntrusiveRing);
    CPPUNIT_TEST(testClientModify);
    CPPUNIT_TEST(test64kPageDescs);
    CPPUNIT_TEST(testTdf92308);
    CPPUNIT_TEST(testTableCellComparison);
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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("GetPageDescName results must be unique", static_cast<size_t>(3), aResults.size());
}

//See https://bugs.libreoffice.org/show_bug.cgi?id=32463
void SwDocTest::testFileNameFields()
{
    //Here's a file name with some chars in it that will be %% encoded, when expanding
    //SwFileNameFields we want to restore the original readable filename
    utl::TempFile aTempFile("demo [name]");
    aTempFile.EnableKillingFile();

    INetURLObject aTempFileURL(aTempFile.GetURL());
    OUString sFileURL = aTempFileURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    SfxMedium aDstMed(sFileURL, StreamMode::STD_READWRITE);

    std::shared_ptr<SfxFilter> pFilter(new SfxFilter(
        "Text",
        OUString(), SfxFilterFlags::NONE, SotClipboardFormatId::NONE, OUString(), 0, OUString(),
        "TEXT", OUString() ));
    aDstMed.SetFilter(pFilter);

    m_xDocShRef->DoSaveAs(aDstMed);
    m_xDocShRef->DoSaveCompleted(&aDstMed);

    const INetURLObject &rUrlObj = m_xDocShRef->GetMedium()->GetURLObject();

    SwFileNameFieldType aNameField(m_pDoc);

    {
        OUString sResult(aNameField.Expand(FF_NAME));
        OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
            true,INetURLObject::DecodeMechanism::WithCharset));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected Readable FileName", sExpected, sResult);
    }

    {
        OUString sResult(aNameField.Expand(FF_PATHNAME));
        OUString sExpected(rUrlObj.GetFull());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected Readable FileName", sExpected, sResult);
    }

    {
        OUString sResult(aNameField.Expand(FF_PATH));
        INetURLObject aTemp(rUrlObj);
        aTemp.removeSegment();
        OUString sExpected(aTemp.PathToFileName());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected Readable FileName", sExpected, sResult);
    }

    {
        OUString sResult(aNameField.Expand(FF_NAME_NOEXT));
        OUString sExpected(rUrlObj.getName(INetURLObject::LAST_SEGMENT,
            true,INetURLObject::DecodeMechanism::WithCharset));
        //Chop off .tmp
        sExpected = sExpected.copy(0, sExpected.getLength() - 4);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected Readable FileName", sExpected, sResult);
    }

    m_xDocShRef->DoInitNew();
}

//See http://lists.freedesktop.org/archives/libreoffice/2011-August/016666.html
//Remove unnecessary parameter to IDocumentStatistics::UpdateDocStat for
//motivation
void SwDocTest::testDocStat()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected initial 0 count", static_cast<sal_uLong>(0), m_pDoc->getIDocumentStatistics().GetDocStat().nChar);

    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    OUString sText("Hello World");
    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sText);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should still be non-updated 0 count", static_cast<sal_uLong>(0), m_pDoc->getIDocumentStatistics().GetDocStat().nChar);

    SwDocStat aDocStat = m_pDoc->getIDocumentStatistics().GetUpdatedDocStat( false, true );
    sal_uLong nLen = static_cast<sal_uLong>(sText.getLength());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should now have updated count", nLen, aDocStat.nChar);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("And cache is updated too", nLen, m_pDoc->getIDocumentStatistics().GetDocStat().nChar);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Grapheme Count should be 1", static_cast<sal_Int32>(1), nGraphemeCount);

    //Surrogate pair example, one single unicode code-point (U+1D11E)
    //represented as two code units in UTF-16
    const sal_Unicode GCLEF[] = { 0xD834, 0xDD1E };
    OUString sGCLEF(GCLEF, SAL_N_ELEMENTS(GCLEF));
    sal_Int32 nCount = pBreakIter->getGraphemeCount(sGCLEF);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Surrogate Pair should be counted as single character", static_cast<sal_Int32>(1), nCount);
}

SwTextNode* getModelToViewTestDocument(SwDoc *pDoc)
{
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    SwFormatFootnote aFootnote;
    aFootnote.SetNumStr("foo");

    pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
    pDoc->getIDocumentContentOperations().InsertString(aPaM, "AAAAA BBBBB ");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    sal_Int32 nPos = aPaM.GetPoint()->nContent.GetIndex();
    pTextNode->InsertItem(aFootnote, nPos, nPos);
    pDoc->getIDocumentContentOperations().InsertString(aPaM, " CCCCC ");
    nPos = aPaM.GetPoint()->nContent.GetIndex();
    pTextNode->InsertItem(aFootnote, nPos, nPos);
    pDoc->getIDocumentContentOperations().InsertString(aPaM, " DDDDD");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>((4*5) + 5 + 2), pTextNode->GetText().getLength());

    //set start of selection to first B
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 6);
    aPaM.SetMark();
    //set end of selection to last C
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 14);
    //set character attribute hidden on range
    SvxCharHiddenItem aHidden(true, RES_CHRATR_HIDDEN);
    pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aHidden );
    aPaM.DeleteMark();

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete|RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on", pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines should be visible", IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    //set start of selection to last A
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 4);
    aPaM.SetMark();
    //set end of selection to second last B
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 9);
    pDoc->getIDocumentContentOperations().DeleteAndJoin(aPaM);    //redline-aware deletion api
    aPaM.DeleteMark();

    return pTextNode;
}

SwTextNode* getModelToViewTestDocument2(SwDoc *pDoc)
{
    getModelToViewTestDocument(pDoc);

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
    pDoc->getIDocumentContentOperations().InsertString(aPaM, "AAAAA");
    IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    sw::mark::IFieldmark *pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(
            pMarksAccess->makeNoTextFieldBookmark(aPaM, "test", ODF_FORMDROPDOWN));
    CPPUNIT_ASSERT(pFieldmark);
    uno::Sequence< OUString > vListEntries { "BBBBB" };
    (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] = uno::makeAny(vListEntries);
    (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] = uno::makeAny(sal_Int32(0));
    pDoc->getIDocumentContentOperations().InsertString(aPaM, "CCCCC");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11),
            pTextNode->GetText().getLength());

    return pTextNode;
}

void SwDocTest::testModelToViewHelperPassthrough()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::PassThrough);
    OUString sViewText = aModelToViewHelper.getViewText();
    OUString sModelText = pTextNode->GetText();
    CPPUNIT_ASSERT_EQUAL(sModelText, sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsExpandFootnote()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields | ExpandMode::ExpandFootnote);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA BBBBB foo CCCCC foo DDDDD"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
            ExpandMode::ExpandFields | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA BBBBB " + OUStringLiteral1(CHAR_ZWSP) + " CCCCC " + OUStringLiteral1(CHAR_ZWSP) + " DDDDD"),
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

void SwDocTest::testModelToViewHelperExpandFields()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA BBBBB  CCCCC  DDDDD"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsReplaceMode()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
        ExpandMode::ExpandFields | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(OUString("AAAAA BBBBB  CCCCC  DDDDD"),
        sViewText);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
        aModelToViewHelper.getFootnotePositions().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
        aModelToViewHelper.getFieldPositions().size());
}

void SwDocTest::testModelToViewHelperExpandFieldsHideInvisible()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::HideInvisible);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA CCCCC " + OUStringLiteral1(CH_TXTATR_BREAKWORD) + " DDDDD"),
        sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideRedlined()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::HideDeletions);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAABB " + OUStringLiteral1(CH_TXTATR_BREAKWORD) + " CCCCC " + OUStringLiteral1(CH_TXTATR_BREAKWORD) + " DDDDD"),
        sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideInvisibleExpandFootnote()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields | ExpandMode::HideInvisible | ExpandMode::ExpandFootnote);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(OUString("AAAAA CCCCC foo DDDDD"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideInvisibleExpandFootnoteReplaceMode()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
        ExpandMode::ExpandFields | ExpandMode::HideInvisible | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA CCCCC " + OUStringLiteral1(CHAR_ZWSP) + " DDDDD"),
        sViewText);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
        aModelToViewHelper.getFootnotePositions().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12),
        aModelToViewHelper.getFootnotePositions()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
        aModelToViewHelper.getFieldPositions().size());
}

void SwDocTest::testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnote()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields | ExpandMode::HideDeletions | ExpandMode::ExpandFootnote);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAABB foo CCCCC foo DDDDD"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideHideRedlinedExpandFootnoteReplaceMode()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
        ExpandMode::ExpandFields | ExpandMode::HideDeletions | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
       OUString("AAAABB " + OUStringLiteral1(CHAR_ZWSP) + " CCCCC " + OUStringLiteral1(CHAR_ZWSP) + " DDDDD"),
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

void SwDocTest::testModelToViewHelperHideInvisibleHideRedlined()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::HideInvisible | ExpandMode::HideDeletions);
    OUString sViewText = aModelToViewHelper.getViewText();
    OUStringBuffer aBuffer;
    aBuffer.append("AAAACCCCC ");
    aBuffer.append(CH_TXTATR_BREAKWORD);
    aBuffer.append(" DDDDD");
    CPPUNIT_ASSERT_EQUAL(aBuffer.makeStringAndClear(), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnote()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields | ExpandMode::HideInvisible | ExpandMode::HideDeletions | ExpandMode::ExpandFootnote);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(OUString("AAAACCCCC foo DDDDD"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsHideInvisibleHideRedlinedExpandFootnoteReplaceMode()
{
    SwTextNode* pTextNode = getModelToViewTestDocument(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
        ExpandMode::ExpandFields | ExpandMode::HideInvisible | ExpandMode::HideDeletions | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(sViewText,
        OUString("AAAACCCCC " + OUStringLiteral1(CHAR_ZWSP) + " DDDDD"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
        aModelToViewHelper.getFootnotePositions().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10),
        aModelToViewHelper.getFootnotePositions()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
        aModelToViewHelper.getFieldPositions().size());
}

void SwDocTest::testModelToViewHelperExpandFieldsExpandFootnote2()
{
    SwTextNode* pTextNode = getModelToViewTestDocument2(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode, ExpandMode::ExpandFields | ExpandMode::ExpandFootnote);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(OUString("AAAAABBBBBCCCCC"), sViewText);
}

void SwDocTest::testModelToViewHelperExpandFieldsExpandFootnoteReplaceMode2()
{
    SwTextNode* pTextNode = getModelToViewTestDocument2(m_pDoc);

    ModelToViewHelper aModelToViewHelper(*pTextNode,
        ExpandMode::ExpandFields | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    OUString sViewText = aModelToViewHelper.getViewText();
    CPPUNIT_ASSERT_EQUAL(
        OUString("AAAAA" + OUStringLiteral1(CHAR_ZWSP) + "CCCCC"),
        sViewText);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0),
        aModelToViewHelper.getFootnotePositions().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
        aModelToViewHelper.getFieldPositions().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5),
        aModelToViewHelper.getFieldPositions()[0]);
}

void SwDocTest::testSwScanner()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();

    CPPUNIT_ASSERT_MESSAGE("Has Text Node", pTextNode);

    //See https://bugs.libreoffice.org/show_bug.cgi?id=40449
    //See https://bugs.libreoffice.org/show_bug.cgi?id=39365
    //Use a temporary OUString as the arg, as that's the trouble behind
    //fdo#40449 and fdo#39365
    {
        SwScanner aScanner(*pTextNode,
            "Hello World",
            nullptr, ModelToViewHelper(), i18n::WordType::DICTIONARY_WORD, 0,
            RTL_CONSTASCII_LENGTH("Hello World"));

        bool bFirstOk = aScanner.NextWord();
        CPPUNIT_ASSERT_MESSAGE("First Token", bFirstOk);
        const OUString &rHello = aScanner.GetWord();
        CPPUNIT_ASSERT_EQUAL(OUString("Hello"), rHello);

        bool bSecondOk = aScanner.NextWord();
        CPPUNIT_ASSERT_MESSAGE("Second Token", bSecondOk);
        const OUString &rWorld = aScanner.GetWord();
        CPPUNIT_ASSERT_EQUAL(OUString("World"), rWorld);
    }

    //See https://www.libreoffice.org/bugzilla/show_bug.cgi?id=45271
    {
        const sal_Unicode IDEOGRAPHICFULLSTOP_D[] = { 0x3002, 'D' };

        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(IDEOGRAPHICFULLSTOP_D,
            SAL_N_ELEMENTS(IDEOGRAPHICFULLSTOP_D)));

        SvxLanguageItem aCJKLangItem( LANGUAGE_CHINESE_SIMPLIFIED, RES_CHRATR_CJK_LANGUAGE );
        SvxLanguageItem aWestLangItem( LANGUAGE_ENGLISH_US, RES_CHRATR_LANGUAGE );
        m_pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aCJKLangItem );
        m_pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aWestLangItem );

        SwDocStat aDocStat;
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(IDEOGRAPHICFULLSTOP_D));

        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), aDocStat.nChar);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), aDocStat.nCharExcludingSpaces);
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
        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(test,
            SAL_N_ELEMENTS(test)));

        SvxLanguageItem aCJKLangItem( LANGUAGE_JAPANESE, RES_CHRATR_CJK_LANGUAGE );
        SvxLanguageItem aWestLangItem( LANGUAGE_ENGLISH_US, RES_CHRATR_LANGUAGE );
        m_pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aCJKLangItem );
        m_pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aWestLangItem );

        SwDocStat aDocStat;
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(test));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("words", static_cast<sal_uLong>(58), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Asian characters and Korean syllables", static_cast<sal_uLong>(43), aDocStat.nAsianWord);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("non-whitespace chars", static_cast<sal_uLong>(105), aDocStat.nCharExcludingSpaces);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("characters", static_cast<sal_uLong>(128), aDocStat.nChar);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=89042
    //See https://bugs.libreoffice.org/show_bug.cgi?id=53399
    {
        SwDocStat aDocStat;

        const sal_Unicode aShouldBeThree[] = {
            0x0053, 0x0068, 0x006F, 0x0075, 0x006C, 0x0064, 0x0020,
            0x2018, 0x0062, 0x0065, 0x0020, 0x0074, 0x0068, 0x0072,
            0x0065, 0x0065, 0x2019
        };

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(aShouldBeThree, SAL_N_ELEMENTS(aShouldBeThree)));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(aShouldBeThree));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(3), aDocStat.nWord);

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

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(aShouldBeFive, SAL_N_ELEMENTS(aShouldBeFive)));
        pTextNode = aPaM.GetNode().GetTextNode();
        aDocStat.Reset();
        pTextNode->CountWords(aDocStat, 0, SAL_N_ELEMENTS(aShouldBeFive));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(5), aDocStat.nWord);
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=49629
    {
        SwDocStat aDocStat;

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Apple");
        pTextNode = aPaM.GetNode().GetTextNode();
        sal_Int32 nPos = aPaM.GetPoint()->nContent.GetIndex();
        SwFormatFootnote aFootnote;
        aFootnote.SetNumStr("banana");
        SwTextAttr* pTA = pTextNode->InsertItem(aFootnote, nPos, nPos);
        CPPUNIT_ASSERT(pTA);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), pTextNode->Len()); //Apple + 0x02
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("footnote should be expanded", static_cast<sal_uLong>(11), aDocStat.nChar);

        const sal_Int32 nNextPos = aPaM.GetPoint()->nContent.GetIndex();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(nPos+1), nNextPos);
        SwFormatRefMark aRef("refmark");
        pTA = pTextNode->InsertItem(aRef, nNextPos, nNextPos);
        CPPUNIT_ASSERT(pTA);

        aDocStat.Reset();
        pTextNode->SetWordCountDirty(true);
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("refmark anchor should not be counted", static_cast<sal_uLong>(11), aDocStat.nChar);

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Apple");

        DateTime aDate(DateTime::SYSTEM);
        SwPostItField aPostIt(
            static_cast<SwPostItFieldType*>(m_pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_POSTITFLD)), "An Author",
            "Some Text", "Initials", "Name", aDate );
        m_pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, SwFormatField(aPostIt));

        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Apple");
        pTextNode = aPaM.GetNode().GetTextNode();
        aDocStat.Reset();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("postit anchor should effectively not exist", static_cast<sal_uLong>(10), aDocStat.nChar);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11), pTextNode->Len());

        aDocStat.Reset();
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=46757
    {
        SwDocStat aDocStat;

        const char aString[] = "Lorem ipsum";
        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, aString);
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(2));

        //turn on red-lining and show changes
        m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete|RedlineFlags::ShowInsert);
        CPPUNIT_ASSERT_MESSAGE("redlining should be on", m_pDoc->getIDocumentRedlineAccess().IsRedlineOn());
        CPPUNIT_ASSERT_MESSAGE("redlines should be visible", IDocumentRedlineAccess::IsShowChanges(m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

        //delete everything except the first word
        aPaM.SetMark(); //set start of selection to current pos
        aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 5);   //set end of selection to fifth char of current node
        m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aPaM);    //redline-aware deletion api
        //"real underlying text should be the same"
        CPPUNIT_ASSERT_EQUAL(pTextNode->GetText(), OUString(aString));

        aDocStat.Reset();
        pTextNode->SetWordCountDirty(true);
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len()); //but word-counting the text should only count the non-deleted text
        CPPUNIT_ASSERT_EQUAL(aDocStat.nWord, static_cast<sal_uLong>(1));

        pTextNode->SetWordCountDirty(true);

        //keep red-lining on but hide changes
        m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
        CPPUNIT_ASSERT_MESSAGE("redlining should be still on", m_pDoc->getIDocumentRedlineAccess().IsRedlineOn());
        CPPUNIT_ASSERT_MESSAGE("redlines should be invisible", !IDocumentRedlineAccess::IsShowChanges(m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

        aDocStat.Reset();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len()); //but word-counting the text should only count the non-deleted text
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), aDocStat.nWord);

        OUString sLorem = pTextNode->GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), sLorem);

        const SwRedlineTable& rTable = m_pDoc->getIDocumentRedlineAccess().GetRedlineTable();

        SwNodes& rNds = m_pDoc->GetNodes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rTable.size());

        SwNodeIndex* pNodeIdx = rTable[0]->GetContentIdx();
        CPPUNIT_ASSERT(pNodeIdx);

        pTextNode = rNds[ pNodeIdx->GetIndex() + 1 ]->GetTextNode();        //first deleted txtnode
        CPPUNIT_ASSERT(pTextNode);

        OUString sIpsum = pTextNode->GetText();
        CPPUNIT_ASSERT_EQUAL(OUString(" ipsum"), sIpsum);

        aDocStat.Reset();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len()); //word-counting the text should only count the non-deleted text, and this whole chunk should be ignored
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), aDocStat.nWord);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), aDocStat.nChar);

        // https://bugs.libreoffice.org/show_bug.cgi?id=68347 we do want to count
        // redline *added* text though
        m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete|RedlineFlags::ShowInsert);
        aPaM.DeleteMark();
        aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 0);
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "redline-new-text ");
        aDocStat.Reset();
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->SetWordCountDirty(true);
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), aDocStat.nWord);
        //redline-new-text Lorem ipsum
        //+++++++++++++++++     ------
        //select start of original text and part of deleted text
        aDocStat.Reset();
        pTextNode->CountWords(aDocStat, 17, 25);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(5), aDocStat.nChar);
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=38983
    {
        SwDocStat aDocStat;

        OUString sTemplate("ThisXis a test.");

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', ' '));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 12 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replaceAll("X", " = "));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replaceAll("X", " _ "));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replaceAll("X", " -- "));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 5 &&
                       aDocStat.nCharExcludingSpaces == 14 &&
                       aDocStat.nChar == 18);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', '_'));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', '-'));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', 0x2012));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', 0x2015));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 3 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        //But default configuration should, msword-alike treat emdash
        //and endash as word separators for word-counting
        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', 0x2013));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replace('X', 0x2014));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 15);
        aDocStat.Reset();

        const sal_Unicode aChunk[] = {' ', 0x2013, ' '};
        OUString sChunk(aChunk, SAL_N_ELEMENTS(aChunk));
        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, sTemplate.replaceAll("X", sChunk));
        pTextNode = aPaM.GetNode().GetTextNode();
        pTextNode->CountWords(aDocStat, 0, pTextNode->Len());
        CPPUNIT_ASSERT(aDocStat.nWord == 4 &&
                       aDocStat.nCharExcludingSpaces == 13 &&
                       aDocStat.nChar == 17);
        aDocStat.Reset();
    }
}

void SwDocTest::testMergePortionsDeleteNotSorted()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "  AABBCC");

    SwCharFormat *const pCharFormat(m_pDoc->MakeCharFormat("foo", nullptr));
    SwFormatCharFormat const charFormat(pCharFormat);

    SwFormatINetFormat const inetFormat("http://example.com", "");

    IDocumentContentOperations & rIDCO(m_pDoc->getIDocumentContentOperations());
    aPaM.SetMark();
    aPaM.GetPoint()->nContent = 2;
    aPaM.GetMark()->nContent = 4;
    rIDCO.InsertPoolItem(aPaM, charFormat);
    aPaM.GetPoint()->nContent = 2;
    aPaM.GetMark()->nContent = 5;
    rIDCO.InsertPoolItem(aPaM, inetFormat);
    aPaM.GetPoint()->nContent = 6;
    aPaM.GetMark()->nContent = 8;
    rIDCO.InsertPoolItem(aPaM, charFormat);
    aPaM.GetPoint()->nContent = 4;
    aPaM.GetMark()->nContent = 6;
    // this triggered an STL assert in SwpHints::MergePortions()
    rIDCO.InsertPoolItem(aPaM, charFormat);
}

//See https://bugs.libreoffice.org/show_bug.cgi?id=40599
void SwDocTest::testGraphicAnchorDeletion()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected initial 0 count", static_cast<sal_uLong>(0), m_pDoc->getIDocumentStatistics().GetDocStat().nChar);

    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Paragraph 1");
    m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());

    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "graphic anchor>><<graphic anchor");
    SwNodeIndex nPara2 = aPaM.GetPoint()->nNode;
    m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());

    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Paragraph 3");

    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), RTL_CONSTASCII_LENGTH("graphic anchor>>"));

    //Insert a graphic at X of >>X<< in paragraph 2
    SfxItemSet aFlySet(m_pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SwFormatAnchor aAnchor(FLY_AS_CHAR);
    aAnchor.SetAnchor(aPaM.GetPoint());
    aFlySet.Put(aAnchor);
    SwFlyFrameFormat *pFrame = m_pDoc->getIDocumentContentOperations().Insert(aPaM, OUString(), OUString(), nullptr, &aFlySet, nullptr, nullptr);
    CPPUNIT_ASSERT_MESSAGE("Expected frame", pFrame != nullptr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be 1 graphic", static_cast<size_t>(1), m_pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    //Delete >X<
    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(),
        RTL_CONSTASCII_LENGTH("graphic anchor>><")+1);
    aPaM.SetMark();
    aPaM.GetPoint()->nNode = nPara2;
    aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), RTL_CONSTASCII_LENGTH("graphic anchor>"));
    m_pDoc->getIDocumentContentOperations().DeleteRange(aPaM);

#ifdef DEBUG_AS_HTML
    {
        SvFileStream aPasteDebug(OUString("cppunitDEBUG.html"), StreamMode::WRITE|StreamMode::TRUNC);
        WriterRef xWrt;
        GetHTMLWriter( String(), String(), xWrt );
        SwWriter aDbgWrt( aPasteDebug, *m_pDoc );
        aDbgWrt.Write( xWrt );
    }
#endif

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be 0 graphics", static_cast<size_t>(0), m_pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    //Now, if instead we swap FLY_AS_CHAR (inline graphic) to FLY_AT_CHAR (anchored to character)
    //and repeat the above, graphic is *not* deleted, i.e. it belongs to the paragraph, not the
    //range to which its anchored, which is annoying.
}

static int
getRand(int modulus)
{
    if (modulus <= 0)
        return 0;
    return comphelper::rng::uniform_int_distribution(0, modulus-1);
}

static OUString
getRandString()
{
    OUString aText("AAAAA BBBB CCC DD E \n");
    int s = getRand(aText.getLength());
    int j = getRand(aText.getLength() - s);
    OUString aRet(aText.copy(s, j));
    if (!getRand(5))
        aRet += "\n";
//    fprintf (stderr, "rand string '%s'\n", OUStringToOString(aRet, RTL_TEXTENCODING_UTF8).getStr());
    return aRet;
}

static SwPosition
getRandomPosition(SwDoc *pDoc, int /* nOffset */)
{
    const SwPosition aPos(pDoc->GetNodes().GetEndOfContent());
    size_t nNodes = aPos.nNode.GetNode().GetIndex() - aPos.nNode.GetNode().StartOfSectionIndex();
    size_t n = comphelper::rng::uniform_size_distribution(0, nNodes);
    SwPaM pam(aPos);
    for (sal_uLong i = 0; i < n; ++i)
    {
        pam.Move(fnMoveBackward, GoInNode);
    }
    return *pam.GetPoint();
}

void SwDocTest::randomTest()
{
    CPPUNIT_ASSERT_MESSAGE("SwDoc::IsRedlineOn()", !m_pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    RedlineFlags modes[] = {
        RedlineFlags::On,
        RedlineFlags::ShowMask,
        RedlineFlags::NONE,
        RedlineFlags::On | RedlineFlags::ShowMask,
        RedlineFlags::On | RedlineFlags::Ignore,
        RedlineFlags::On | RedlineFlags::Ignore | RedlineFlags::ShowMask,
        RedlineFlags::On | RedlineFlags::ShowInsert,
        RedlineFlags::On | RedlineFlags::ShowDelete
    };
    static const char *authors[] = {
        "Jim", "Bob", "JimBobina", "Helga", "Gertrude", "Spagna", "Hurtleweed"
    };

    for( size_t rlm = 0; rlm < SAL_N_ELEMENTS(modes); rlm++ )
    {
        m_pDoc->ClearDoc();

        // setup redlining
        m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(modes[rlm]);
        SW_MOD()->SetRedlineAuthor(OUString::createFromAscii(authors[0]));

        for( int i = 0; i < 2000; i++ )
        {
            SwPaM aPam(m_pDoc->GetNodes());
            SwCursor aCrs(getRandomPosition(m_pDoc, i/20), nullptr);
            aCrs.SetMark();

            switch (getRand (i < 50 ? 3 : 6)) {
            // insert ops first
            case 0: {
                if (!m_pDoc->getIDocumentContentOperations().InsertString(aCrs, getRandString())) {
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
                    m_pDoc->getIDocumentContentOperations().DelFullPara(aCrs);
                    break;
                case 1:
                    m_pDoc->getIDocumentContentOperations().DeleteRange(aCrs);
                    break;
                case 2:
                    m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aCrs, !!getRand(1));
                    break;
                case 3:
                default:
                    m_pDoc->getIDocumentContentOperations().Overwrite(aCrs, getRandString());
                    break;
                }
                break;
            case 4: { // movement
                SwMoveFlags nFlags =
                         getRand(1) // FIXME: puterb this more ?
                         ? SwMoveFlags::DEFAULT
                         : SwMoveFlags::ALLFLYS |
                           SwMoveFlags::CREATEUNDOOBJ |
                           SwMoveFlags::REDLINES |
                           SwMoveFlags::NO_DELFRMS;
                SwPosition aTo(getRandomPosition(m_pDoc, i/10));
                m_pDoc->getIDocumentContentOperations().MoveRange(aCrs, aTo, nFlags);
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
    rDoc.getIDocumentContentOperations().TransliterateText(rPaM, aTrans);
    return rPaM.GetText();
}

void SwDocTest::testTransliterate()
{
    // just some simple test to see if it's totally broken
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "foobar");
    aPaM.SetMark();
    aPaM.GetPoint()->nContent = 0;
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), aPaM.GetText());

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

namespace
{
    class SwTableFormulaTest : public SwTableFormula
    {
        SwTableNode *m_pNode;
    public:
        SwTableFormulaTest(const OUString &rStr, SwTableNode *pNode)
            : SwTableFormula(rStr)
            , m_pNode(pNode)
        {
            m_eNmType = INTRNL_NAME;
        }
        virtual const SwNode* GetNodeOfFormula() const override
        {
            return m_pNode;
        }
    };
}

//tdf#66353 Expression is faulty
void SwDocTest::testFormulas()
{
    SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
    SwPosition aPos(aIdx);

    const SwTable *pTable = m_pDoc->InsertTable(
        SwInsertTableOptions(tabopts::HEADLINE_NO_BORDER, 0), aPos, 1, 3, 0);
    SwTableNode* pTableNode = pTable->GetTableNode();
    SwTableFormulaTest aFormula("<\x12-1,0>+<Table1.A1>", pTableNode);

    aFormula.PtrToBoxNm(pTable);

    CPPUNIT_ASSERT_EQUAL(OUString("<?>+<Table1.?>"), aFormula.GetFormula());

    // tdf#61228: Evaluating non-defined function should return an error
    SwCalc aCalc(*m_pDoc);
    SwSbxValue val = aCalc.Calculate("foobar()");
    CPPUNIT_ASSERT(aCalc.IsCalcError() && val.IsVoidValue() && val.IsDouble());
    CPPUNIT_ASSERT_EQUAL(DBL_MAX, val.GetDouble());
    // Evaluating non-defined variable should return 0 without an error
    val = aCalc.Calculate("foobar");
    CPPUNIT_ASSERT(!aCalc.IsCalcError() && val.IsVoidValue() && val.IsLong());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), val.GetLong());
}

void SwDocTest::testMarkMove()
{
    IDocumentMarkAccess* pMarksAccess = m_pDoc->getIDocumentMarkAccess();

    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Paragraph 1");
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, "Para1", IDocumentMarkAccess::MarkType::BOOKMARK);

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Paragraph 2");
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, "Para2", IDocumentMarkAccess::MarkType::BOOKMARK);

        m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPaM.GetPoint());
        m_pDoc->getIDocumentContentOperations().InsertString(aPaM, "Paragraph 3");
        aPaM.SetMark();
        aPaM.GetMark()->nContent -= aPaM.GetMark()->nContent.GetIndex();
        pMarksAccess->makeMark(aPaM, "Para3", IDocumentMarkAccess::MarkType::BOOKMARK);
    }

    // join paragraph 2 and 3 and check
    {
        SwNodeIndex aIdx(m_pDoc->GetNodes().GetEndOfContent(), -2);
        SwTextNode& rParaNode2 = dynamic_cast<SwTextNode&>(aIdx.GetNode());
        rParaNode2.JoinNext();
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
        m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aPaM);
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
        m_pDoc->getIDocumentContentOperations().SplitNode(aPos, false);
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

namespace
{
    struct TestRing : public sw::Ring<TestRing>
    {
        TestRing() : sw::Ring<TestRing>() {};
        TestRing* GetNext()
            { return GetNextInRing(); }
        TestRing* GetPrev()
            { return GetPrevInRing(); }
        bool lonely() const
            { return unique(); }
#if 0
        void debug()
        {
            SAL_DEBUG("TestRing at: " << this << " prev: " << GetPrev() << " next: " << GetNext());
        }
#endif
    };
}

void SwDocTest::testIntrusiveRing()
{
    TestRing aRing1, aRing2, aRing3, aRing4, aRing5;
    std::vector<TestRing*> vRings;
    vRings.push_back(&aRing1);
    vRings.push_back(&aRing2);
    vRings.push_back(&aRing3);
    vRings.push_back(&aRing4);
    vRings.push_back(&aRing5);
    CPPUNIT_ASSERT_EQUAL(aRing1.GetRingContainer().size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT(aRing1.lonely());
    CPPUNIT_ASSERT(aRing2.lonely());
    CPPUNIT_ASSERT(aRing3.lonely());
    aRing2.MoveTo(&aRing1);
    aRing3.MoveTo(&aRing1);
    CPPUNIT_ASSERT_EQUAL(aRing1.GetRingContainer().size(), static_cast<size_t>(3));
    CPPUNIT_ASSERT_EQUAL(aRing2.GetRingContainer().size(), static_cast<size_t>(3));
    CPPUNIT_ASSERT_EQUAL(aRing3.GetRingContainer().size(), static_cast<size_t>(3));
    CPPUNIT_ASSERT(!aRing1.lonely());
    CPPUNIT_ASSERT(!aRing2.lonely());
    CPPUNIT_ASSERT(!aRing3.lonely());
    aRing5.MoveTo(&aRing4);
    CPPUNIT_ASSERT_EQUAL(aRing4.GetRingContainer().size(), static_cast<size_t>(2));
    aRing4.GetRingContainer().merge(aRing1.GetRingContainer());
    for(TestRing* pRing : vRings)
    {
        CPPUNIT_ASSERT_EQUAL(pRing->GetRingContainer().size(), static_cast<size_t>(5));
    }
    for(std::vector<TestRing*>::iterator ppRing = vRings.begin(); ppRing != vRings.end(); ++ppRing)
    {
        std::vector<TestRing*>::iterator ppNext = ppRing+1;
        if(ppNext==vRings.end())
            ppNext = vRings.begin();
        CPPUNIT_ASSERT_EQUAL((*ppRing)->GetNext(), *ppNext);
        CPPUNIT_ASSERT_EQUAL((*ppNext)->GetPrev(), *ppRing);
    }
    for(TestRing& r: aRing1.GetRingContainer())
    {
        TestRing* pRing = &r;
        CPPUNIT_ASSERT(pRing);
        //pRing->debug();
    }
    const TestRing* pConstRing = &aRing1;
    for(const TestRing& r: pConstRing->GetRingContainer()) // this should fail without r being const
    {
        const TestRing* pRing = &r;
        CPPUNIT_ASSERT(pRing);
    }
    TestRing foo, bar;
    foo.MoveTo(&bar);
    CPPUNIT_ASSERT_EQUAL(&foo, bar.GetNext());
    CPPUNIT_ASSERT_EQUAL(&foo, bar.GetPrev());
    CPPUNIT_ASSERT_EQUAL(&bar, foo.GetNext());
    CPPUNIT_ASSERT_EQUAL(&bar, foo.GetPrev());
    foo.MoveTo(&foo);
    CPPUNIT_ASSERT_EQUAL(&bar, bar.GetNext());
    CPPUNIT_ASSERT_EQUAL(&bar, bar.GetPrev());
    CPPUNIT_ASSERT_EQUAL(&foo, foo.GetNext());
    CPPUNIT_ASSERT_EQUAL(&foo, foo.GetPrev());
}

namespace
{
    struct TestHint final : SfxHint {};
    struct TestModify : SwModify
    {
    };
    struct TestClient : SwClient
    {
        int m_nModifyCount;
        int m_nNotifyCount;
        TestClient() : m_nModifyCount(0), m_nNotifyCount(0) {};
        virtual void Modify( const SfxPoolItem*, const SfxPoolItem*) override
        { ++m_nModifyCount; }
        virtual void SwClientNotify(const SwModify& rModify, const SfxHint& rHint) override
        {
            if(typeid(TestHint) == typeid(rHint))
                ++m_nNotifyCount;
            else
                SwClient::SwClientNotify(rModify, rHint);
        }
    };
    // sad copypasta as tools/rtti.hxxs little brain can't cope with templates
    struct OtherTestClient : SwClient
    {
        int m_nModifyCount;
        OtherTestClient() : m_nModifyCount(0) {};
        virtual void Modify( const SfxPoolItem*, const SfxPoolItem*) override
        { ++m_nModifyCount; }
    };
}
void SwDocTest::testClientModify()
{
    (void) OtherTestClient(); // avoid loplugin:unreffun
    TestModify aMod;
    TestClient aClient1, aClient2;
    OtherTestClient aOtherClient1;
    // test client registration
    CPPUNIT_ASSERT(!aMod.HasWriterListeners());
    CPPUNIT_ASSERT(!aMod.HasOnlyOneListener());
    CPPUNIT_ASSERT_EQUAL(aClient1.GetRegisteredIn(),static_cast<SwModify*>(nullptr));
    CPPUNIT_ASSERT_EQUAL(aClient2.GetRegisteredIn(),static_cast<SwModify*>(nullptr));
    CPPUNIT_ASSERT_EQUAL(aClient2.GetRegisteredIn(),static_cast<SwModify*>(nullptr));
    aMod.Add(&aClient1);
    CPPUNIT_ASSERT(aMod.HasWriterListeners());
    CPPUNIT_ASSERT(aMod.HasOnlyOneListener());
    aMod.Add(&aClient2);
    CPPUNIT_ASSERT_EQUAL(aClient1.GetRegisteredIn(),static_cast<SwModify*>(&aMod));
    CPPUNIT_ASSERT_EQUAL(aClient2.GetRegisteredIn(),static_cast<SwModify*>(&aMod));
    CPPUNIT_ASSERT(aMod.HasWriterListeners());
    CPPUNIT_ASSERT(!aMod.HasOnlyOneListener());
    // test broadcast
    aMod.ModifyBroadcast(nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nModifyCount,1);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nModifyCount,1);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nNotifyCount,0);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nNotifyCount,0);
    aMod.ModifyBroadcast(nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nModifyCount,2);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nModifyCount,2);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nNotifyCount,0);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nNotifyCount,0);
    // test notify
    {
        TestHint aHint;
        aMod.CallSwClientNotify(aHint);
        CPPUNIT_ASSERT_EQUAL(aClient1.m_nModifyCount,2);
        CPPUNIT_ASSERT_EQUAL(aClient2.m_nModifyCount,2);
        CPPUNIT_ASSERT_EQUAL(aClient1.m_nNotifyCount,1);
        CPPUNIT_ASSERT_EQUAL(aClient2.m_nNotifyCount,1);
    }
    // test typed iteration
    CPPUNIT_ASSERT(typeid(aClient1) != typeid(OtherTestClient));
    {
        SwIterator<OtherTestClient,SwModify> aIter(aMod);
        for(OtherTestClient* pClient = aIter.First(); pClient ; pClient = aIter.Next())
            CPPUNIT_ASSERT(false);
    }
    {
        int nCount = 0;
        SwIterator<TestClient,SwModify> aIter(aMod);
        for(TestClient* pClient = aIter.First(); pClient ; pClient = aIter.Next())
        {
            CPPUNIT_ASSERT_EQUAL(pClient->m_nModifyCount,2);
            ++nCount;
        }
        CPPUNIT_ASSERT_EQUAL(nCount,2);
    }
    aMod.Add(&aOtherClient1);
    CPPUNIT_ASSERT_EQUAL(aOtherClient1.m_nModifyCount,0);
    {
        int nCount = 0;
        SwIterator<TestClient,SwModify> aIter(aMod);
        for(TestClient* pClient = aIter.First(); pClient ; pClient = aIter.Next())
        {
            CPPUNIT_ASSERT_EQUAL(pClient->m_nModifyCount,2);
            ++nCount;
        }
        CPPUNIT_ASSERT_EQUAL(nCount,2);
    }
    CPPUNIT_ASSERT_EQUAL(aOtherClient1.m_nModifyCount,0);
    aMod.Remove(&aOtherClient1);
    CPPUNIT_ASSERT_EQUAL(aClient1.GetRegisteredIn(),static_cast<SwModify*>(&aMod));
    CPPUNIT_ASSERT_EQUAL(aClient2.GetRegisteredIn(),static_cast<SwModify*>(&aMod));
    CPPUNIT_ASSERT_EQUAL(aOtherClient1.GetRegisteredIn(),static_cast<SwModify*>(nullptr));
    // test client self-deregistration during iteration
    {
        int nCount = 0;
        SwIterator<TestClient,SwModify> aIter(aMod);
        for(TestClient* pClient = aIter.First(); pClient ; pClient = aIter.Next())
        {
            aMod.Remove(pClient);
            ++nCount;
        }
        CPPUNIT_ASSERT_EQUAL(nCount,2);
    }
    CPPUNIT_ASSERT_EQUAL(aClient1.GetRegisteredIn(), static_cast<SwModify*>(nullptr));
    CPPUNIT_ASSERT_EQUAL(aClient2.GetRegisteredIn(), static_cast<SwModify*>(nullptr));
    {
        int nCount = 0;
        SwIterator<TestClient,SwModify> aIter(aMod);
        for(TestClient* pClient = aIter.First(); pClient ; pClient = aIter.Next())
        {
            CPPUNIT_ASSERT(false);
        }
        CPPUNIT_ASSERT_EQUAL(nCount,0);
    }
    aMod.ModifyBroadcast(nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nModifyCount,2);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nModifyCount,2);
    CPPUNIT_ASSERT_EQUAL(aClient1.m_nNotifyCount,1);
    CPPUNIT_ASSERT_EQUAL(aClient2.m_nNotifyCount,1);
}

void SwDocTest::test64kPageDescs()
{
    size_t nPageDescCount = 65536; // USHRT_MAX + 1

    for (size_t i = 0; i < nPageDescCount; ++i)
    {
        OUString aName = "Page" + OUString::number(i);
        m_pDoc->MakePageDesc( aName );
    }

    size_t nCount = m_pDoc->GetPageDescCnt();
    // +1 because Writer always creates a dummy page desc
    // in a new SwDoc
    CPPUNIT_ASSERT_EQUAL( nPageDescCount + 1, nCount );

    const SwPageDesc &rDesc = m_pDoc->GetPageDesc( nPageDescCount );
    SwPageDesc &rZeroDesc = m_pDoc->GetPageDesc( 0 );
    CPPUNIT_ASSERT_EQUAL( OUString("Page65535"), rDesc.GetName() );

    SwPageDesc aDesc( rDesc );
    const OUString aChanged("Changed01");
    aDesc.SetName( aChanged );
    m_pDoc->ChgPageDesc( nPageDescCount, aDesc );

    size_t nPos;
    SwPageDesc *pDesc = m_pDoc->FindPageDesc( aChanged, &nPos );
    CPPUNIT_ASSERT( pDesc != nullptr );
    CPPUNIT_ASSERT_EQUAL( nPageDescCount, nPos );

    // check if we didn't mess up PageDesc at pos 0
    // (happens with 16bit int overflow)
    OUString aZeroName = rZeroDesc.GetName();
    rZeroDesc = m_pDoc->GetPageDesc( 0 );
    CPPUNIT_ASSERT_EQUAL( aZeroName, rZeroDesc.GetName() );

    m_pDoc->DelPageDesc( aChanged, nPos );
    pDesc = m_pDoc->FindPageDesc( aChanged, &nPos );
    // not there anymore
    CPPUNIT_ASSERT( !pDesc );
    CPPUNIT_ASSERT_EQUAL( std::numeric_limits<size_t>::max(), nPos);

    // check if PageDesc at pos 0 is still there
    pDesc = m_pDoc->FindPageDesc( aZeroName, &nPos );
    CPPUNIT_ASSERT( pDesc != nullptr );
    CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), nPos );
}

void SwDocTest::testTdf92308()
{
    CPPUNIT_ASSERT_EQUAL(m_pDoc->HasInvisibleContent(), false);
}

void SwDocTest::testTableCellComparison()
{
    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellsByColFirst("A1", "Z1") );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByColFirst("Z1", "A1") );
    CPPUNIT_ASSERT_EQUAL(  0, sw_CompareCellsByColFirst("A1", "A1") );

    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByColFirst("A2", "A1") );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByColFirst("Z3", "A2") );
    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellsByColFirst("A3", "Z1") );

    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellsByRowFirst("A1", "Z1") );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByRowFirst("Z1", "A1") );
    CPPUNIT_ASSERT_EQUAL(  0, sw_CompareCellsByRowFirst("A1", "A1") );

    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByRowFirst("A2", "A1") );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByRowFirst("Z3", "A2") );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellsByRowFirst("A3", "Z1") );

    CPPUNIT_ASSERT_EQUAL(  0, sw_CompareCellRanges("A1", "A1", "A1", "A1", true) );
    CPPUNIT_ASSERT_EQUAL(  0, sw_CompareCellRanges("A1", "Z1", "A1", "Z1", true) );
    CPPUNIT_ASSERT_EQUAL(  0, sw_CompareCellRanges("A1", "Z1", "A1", "Z1", false) );

    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellRanges("A1", "Z1", "B1", "Z1", true) );
    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellRanges("A1", "Z1", "A2", "Z2", false) );
    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellRanges("A1", "Z1", "A2", "Z2", true) );
    CPPUNIT_ASSERT_EQUAL( -1, sw_CompareCellRanges("A1", "Z1", "A6", "Z2", true) );

    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellRanges("B1", "Z1", "A1", "Z1", true) );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellRanges("A2", "Z2", "A1", "Z1", false) );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellRanges("A2", "Z2", "A1", "Z1", true) );
    CPPUNIT_ASSERT_EQUAL( +1, sw_CompareCellRanges("A6", "Z2", "A1", "Z1", true) );

    OUString rCell1 = OUString("A1");
    OUString rCell2 = OUString("C5");

    sw_NormalizeRange(rCell1, rCell2);
    CPPUNIT_ASSERT_EQUAL( OUString("A1"), rCell1 );
    CPPUNIT_ASSERT_EQUAL( OUString("C5"), rCell2 );

    sw_NormalizeRange(rCell2, rCell1);
    CPPUNIT_ASSERT_EQUAL( OUString("C5"), rCell1 );
    CPPUNIT_ASSERT_EQUAL( OUString("A1"), rCell2 );

    rCell1 = OUString("A5");
    rCell2 = OUString("C1");

    sw_NormalizeRange(rCell1, rCell2);
    CPPUNIT_ASSERT_EQUAL( OUString("A1"), rCell1 );
    CPPUNIT_ASSERT_EQUAL( OUString("C5"), rCell2 );

    sw_NormalizeRange(rCell2, rCell1);
    CPPUNIT_ASSERT_EQUAL( OUString("C5"), rCell1 );
    CPPUNIT_ASSERT_EQUAL( OUString("A1"), rCell2 );

    CPPUNIT_ASSERT_EQUAL( OUString(), sw_GetCellName(-1, -1) );
}

void SwDocTest::setUp()
{
    BootstrapFixture::setUp();

    SwGlobals::ensure();
    m_pDoc = new SwDoc;
    m_xDocShRef = new SwDocShell(m_pDoc, SfxObjectCreateMode::EMBEDDED);
    m_xDocShRef->DoInitNew();
}

void SwDocTest::tearDown()
{
    m_pDoc = nullptr; // deleted by DoClose()
    m_xDocShRef->DoClose();
    m_xDocShRef.clear();

    BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDocTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
