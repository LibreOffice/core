/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <crsskip.hxx>
#include <shellio.hxx>
#include <expfld.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <fmtclds.hxx>

#include "UndoManager.hxx"

static const char* DATA_DIRECTORY = "/sw/qa/extras/uiwriter/data/";

class SwUiWriterTest : public SwModelTestBase
{

public:
    void testReplaceForward();
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    void testReplaceBackward();
    void testFdo69893();
    void testFdo70807();
    void testImportRTF();
    void testExportRTF();
    void testFdo75110();
    void testFdo75898();
    void testFdo74981();
    void testCp1000071();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST(testFdo69893);
    CPPUNIT_TEST(testFdo70807);
    CPPUNIT_TEST(testImportRTF);
    CPPUNIT_TEST(testExportRTF);
    CPPUNIT_TEST(testFdo75110);
    CPPUNIT_TEST(testFdo75898);
    CPPUNIT_TEST(testFdo74981);
    CPPUNIT_TEST(testCp1000071);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = 0);
};

SwDoc* SwUiWriterTest::createDoc(const char* pName)
{
    if (!pName)
        pName = "empty.odt";
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    return pTxtDoc->GetDocShell()->GetDoc();
}

//Replacement tests

static void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), end);
}

static const OUString ORIGINAL_REPLACE_CONTENT("toto titi tutu");
static const OUString EXPECTED_REPLACE_CONTENT("toto toto tutu");

void SwUiWriterTest::testReplaceForward()
{
    SwDoc* pDoc = createDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, ORIGINAL_REPLACE_CONTENT);

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testFdo75110()
{
    SwDoc* pDoc = createDoc("fdo75110.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll();
    // The problem was that SwEditShell::DeleteSel() what this Delete() invokes took the wrong selection...
    pWrtShell->Delete();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    // ... so this Undo() call resulted in a crash.
    rUndoManager.Undo();
}

void SwUiWriterTest::testFdo75898()
{
    SwDoc* pDoc = createDoc("fdo75898.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->InsertRow(1, true);
    pWrtShell->InsertRow(1, true);

    // Now check if the table has 3 lines.
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    SwTableNode* pTableNode = pShellCrsr->Start()->nNode.GetNode().FindTableNode();
    // This was 1, when doing the same using the UI, Writer even crashed.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pTableNode->GetTable().GetTabLines().size());
}

void SwUiWriterTest::testReplaceBackward()
{
    SwDoc* pDoc = createDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));
    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testFdo69893()
{
    SwDoc* pDoc = createDoc("fdo69893.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll();

    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
    // Selection did not include the para after table, this was "B1".
    CPPUNIT_ASSERT_EQUAL(OUString("Para after table."), rEnd.GetTxt());
}

void SwUiWriterTest::testFdo70807()
{
    load(DATA_DIRECTORY, "fdo70807.odt");

    uno::Reference<container::XIndexAccess> stylesIter(getStyles("PageStyles"), uno::UNO_QUERY);

    for (sal_Int32 i = 0; i < stylesIter->getCount(); ++i)
    {
        uno::Reference<style::XStyle> xStyle(stylesIter->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xName(xStyle, uno::UNO_QUERY);

        sal_Bool expectedUsedStyle = sal_False;
        sal_Bool expectedUserDefined = sal_False;

        OUString styleName(xName->getName());

        // just these styles are user defined styles
        if (styleName == "pagestyle1" || styleName == "pagestyle2")
            expectedUserDefined = sal_True;

        // just these styles are used in the document
        if (styleName == "Right Page" || styleName == "pagestyle1" || styleName == "pagestyle2")
            expectedUsedStyle = sal_True;

        CPPUNIT_ASSERT_EQUAL(expectedUserDefined, xStyle->isUserDefined());
        CPPUNIT_ASSERT_EQUAL(expectedUsedStyle, xStyle->isInUse());
    }
}

void SwUiWriterTest::testImportRTF()
{
    // Insert "foobar" and position the cursor between "foo" and "bar".
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("foobar");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);

    // Insert the RTF at the cursor position.
    OString aData = "{\\rtf1 Hello world!\\par}";
    SvMemoryStream aStream(const_cast<sal_Char*>(aData.getStr()), aData.getLength(), STREAM_READ);
    SwReader aReader(aStream, OUString(), OUString(), *pWrtShell->GetCrsr());
    Reader* pRTFReader = SwReaderWriter::GetReader(READER_WRITER_RTF);
    CPPUNIT_ASSERT(pRTFReader != 0);
    CPPUNIT_ASSERT_EQUAL(sal_uLong(0), aReader.Read(*pRTFReader));

    sal_uLong nIndex = pWrtShell->GetCrsr()->GetNode()->GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("fooHello world!"), static_cast<SwTxtNode*>(pDoc->GetNodes()[nIndex - 1])->GetTxt());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), static_cast<SwTxtNode*>(pDoc->GetNodes()[nIndex])->GetTxt());
}

void SwUiWriterTest::testExportRTF()
{
    // Insert "aaabbbccc" and select "bbb".
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("aaabbbccc");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 3, /*bBasicCall=*/false);

    // Create the clipboard document.
    boost::shared_ptr<SwDoc> pClpDoc(new SwDoc());
    pClpDoc->SetClipBoard(true);
    pWrtShell->Copy(pClpDoc.get());

    // And finally export it as RTF.
    WriterRef xWrt;
    SwReaderWriter::GetWriter("RTF", OUString(), xWrt);
    SvMemoryStream aStream;
    SwWriter aWrt(aStream, *pClpDoc);
    aWrt.Write(xWrt);

    OString aData(static_cast<const sal_Char*>(aStream.GetBuffer()), aStream.GetSize());
    CPPUNIT_ASSERT(aData.startsWith("{\\rtf1"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aData.indexOf("aaa"));
    CPPUNIT_ASSERT(aData.indexOf("bbb") != -1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aData.indexOf("ccc"));
}

void SwUiWriterTest::testFdo74981()
{
    // create a document with an input field
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInputField aField((SwInputFieldType*)pWrtShell->GetFldType(0, RES_INPUTFLD), OUString("foo"), OUString("bar"), 0, 0);
    pWrtShell->Insert(aField);

    // expect hints
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwTxtNode* pTxtNode = aIdx.GetNode().GetTxtNode();
    CPPUNIT_ASSERT(pTxtNode->HasHints());

    // go to the begin of the paragraph and split this node
    pWrtShell->Left(CRSR_SKIP_CHARS, false, 100, false);
    pWrtShell->SplitNode();

    // expect only the second paragraph to have hints
    aIdx = SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    pTxtNode = aIdx.GetNode().GetTxtNode();
    CPPUNIT_ASSERT(pTxtNode->HasHints());
    aIdx--;
    pTxtNode = aIdx.GetNode().GetTxtNode();
    CPPUNIT_ASSERT(!pTxtNode->HasHints());
}

void SwUiWriterTest::testCp1000071()
{
    SwDoc* pDoc = createDoc("cp1000071.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
    CPPUNIT_ASSERT_EQUAL( size_t( 2 ), rTbl.size());
    sal_uLong redlineStart0NodeIndex = rTbl[ 0 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart0Index = rTbl[ 0 ]->Start()->nContent.GetIndex();
    sal_uLong redlineEnd0NodeIndex = rTbl[ 0 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd0Index = rTbl[ 0 ]->End()->nContent.GetIndex();
    sal_uLong redlineStart1NodeIndex = rTbl[ 1 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart1Index = rTbl[ 1 ]->Start()->nContent.GetIndex();
    sal_uLong redlineEnd1NodeIndex = rTbl[ 1 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd1Index = rTbl[ 1 ]->End()->nContent.GetIndex();

    // Change the document layout to be 2 columns, and then undo.
    pWrtShell->SelAll();
    SwSectionData section(CONTENT_SECTION, pWrtShell->GetUniqueSectionName());
    SfxItemSet set( pDoc->GetDocShell()->GetPool(), RES_COL, RES_COL, 0 );
    SwFmtCol col;
    col.Init( 2, 0, 10000 );
    set.Put( col );
    pWrtShell->InsertSection( section, &set );
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    // Check that redlines are the same like at the beginning.
    CPPUNIT_ASSERT_EQUAL( size_t( 2 ), rTbl.size());
    CPPUNIT_ASSERT_EQUAL( redlineStart0NodeIndex, rTbl[ 0 ]->Start()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart0Index, rTbl[ 0 ]->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0NodeIndex, rTbl[ 0 ]->End()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0Index, rTbl[ 0 ]->End()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1NodeIndex, rTbl[ 1 ]->Start()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1Index, rTbl[ 1 ]->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1NodeIndex, rTbl[ 1 ]->End()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1Index, rTbl[ 1 ]->End()->nContent.GetIndex());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
