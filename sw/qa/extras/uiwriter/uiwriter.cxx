/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <crsskip.hxx>
#include <shellio.hxx>
#include <expfld.hxx>
#include <drawdoc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <view.hxx>
#include <hhcwrp.hxx>
#include <postithelper.hxx>
#include <PostItMgr.hxx>
#include <SidebarWin.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include "UndoManager.hxx"

static const char* DATA_DIRECTORY = "/sw/qa/extras/uiwriter/data/";

class SwUiWriterTest : public SwModelTestBase
{

public:
    void testReplaceForward();
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    void testReplaceBackward();
    void testRedlineFrame();
    void testFdo69893();
    void testFdo70807();
    void testImportRTF();
    void testExportRTF();
    void testFdo75110();
    void testFdo75898();
    void testFdo74981();
    void testShapeTextboxSelect();
    void testShapeTextboxDelete();
    void testCp1000071();
    void testShapeTextboxVertadjust();
    void testShapeTextboxAutosize();
    void testFdo82191();
    void testCommentedWord();
    void testChineseConversionBlank();
    void testChineseConversionNonChineseText();
    void testChineseConversionTraditionalToSimplified();
    void testChineseConversionSimplifiedToTraditional();
    void testFdo85554();
    void testBookmarkUndo();
    void testCp1000115();
    void testDde();
    void testTdf89720();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST(testRedlineFrame);
    CPPUNIT_TEST(testFdo69893);
    CPPUNIT_TEST(testFdo70807);
    CPPUNIT_TEST(testImportRTF);
    CPPUNIT_TEST(testExportRTF);
    CPPUNIT_TEST(testFdo75110);
    CPPUNIT_TEST(testFdo75898);
    CPPUNIT_TEST(testFdo74981);
    CPPUNIT_TEST(testShapeTextboxSelect);
    CPPUNIT_TEST(testShapeTextboxDelete);
    CPPUNIT_TEST(testCp1000071);
    CPPUNIT_TEST(testShapeTextboxVertadjust);
    CPPUNIT_TEST(testShapeTextboxAutosize);
    CPPUNIT_TEST(testFdo82191);
    CPPUNIT_TEST(testCommentedWord);
    CPPUNIT_TEST(testChineseConversionBlank);
    CPPUNIT_TEST(testChineseConversionNonChineseText);
    CPPUNIT_TEST(testChineseConversionTraditionalToSimplified);
    CPPUNIT_TEST(testChineseConversionSimplifiedToTraditional);
    CPPUNIT_TEST(testFdo85554);
    CPPUNIT_TEST(testBookmarkUndo);
    CPPUNIT_TEST(testCp1000115);
    CPPUNIT_TEST(testDde);
    CPPUNIT_TEST(testTdf89720);

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

    pDoc->getIDocumentContentOperations().InsertString(aPaM, ORIGINAL_REPLACE_CONTENT);

    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testRedlineFrame()
{
    SwDoc * pDoc(createDoc("redlineFrame.fodt"));
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // there is exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    sal_uInt16 nMode = pWrtShell->GetRedlineMode();
    CPPUNIT_ASSERT(nMode & nsRedlineMode_t::REDLINE_SHOW_DELETE);

    // hide delete redlines
    pWrtShell->SetRedlineMode(nMode & ~nsRedlineMode_t::REDLINE_SHOW_DELETE);

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    pWrtShell->SetRedlineMode(nMode); // show again

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());
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

    pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString("toto titi tutu"));
    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testFdo69893()
{
    SwDoc* pDoc = createDoc("fdo69893.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll(); // A1 is empty -> selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.

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

        bool expectedUsedStyle = false;
        bool expectedUserDefined = false;

        OUString styleName(xName->getName());

        // just these styles are user defined styles
        if (styleName == "pagestyle1" || styleName == "pagestyle2")
            expectedUserDefined = true;

        // just these styles are used in the document
        if (styleName == "Right Page" || styleName == "pagestyle1" || styleName == "pagestyle2")
            expectedUsedStyle = true;

        CPPUNIT_ASSERT_EQUAL(expectedUserDefined, bool(xStyle->isUserDefined()));
        CPPUNIT_ASSERT_EQUAL(expectedUsedStyle, bool(xStyle->isInUse()));
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

    sal_uLong nIndex = pWrtShell->GetCrsr()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("fooHello world!"), pDoc->GetNodes()[nIndex - 1]->GetTxtNode()->GetTxt());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), pDoc->GetNodes()[nIndex]->GetTxtNode()->GetTxt());
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

    //Amusingly eventually there was a commit id with "ccc" in it, and so the rtf contained
    //{\*\generator LibreOfficeDev/4.4.0.0.alpha0$Linux_X86_64 LibreOffice_project/f70664ccc6837f2cc21a29bb4f44e41e100efe6b}
    //so the test fell over. so strip the generator tag
    sal_Int32 nGeneratorStart = aData.indexOf("{\\*\\generator ");
    CPPUNIT_ASSERT(nGeneratorStart != -1);
    sal_Int32 nGeneratorEnd = aData.indexOf('}', nGeneratorStart + 1);
    CPPUNIT_ASSERT(nGeneratorEnd != -1);
    aData = aData.replaceAt(nGeneratorStart, nGeneratorEnd-nGeneratorStart+1, "");

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

void SwUiWriterTest::testShapeTextboxSelect()
{
    SwDoc* pDoc = createDoc("shape-textbox.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(RES_FLYFRMFMT, static_cast<RES_FMT>(pTextBox->GetFmt()->Which()));

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwDrawContact* pShape = static_cast<SwDrawContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // And finally make sure the shape got selected, not just the textbox itself.
    CPPUNIT_ASSERT_EQUAL(RES_DRAWFRMFMT, static_cast<RES_FMT>(pShape->GetFmt()->Which()));
}

void SwUiWriterTest::testShapeTextboxDelete()
{
    SwDoc* pDoc = createDoc("shape-textbox.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    size_t nActual = pPage->GetObjCount();
    // Two objects on the draw page: the shape and its textbox.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), nActual);

    pWrtShell->DelSelectedObj();
    nActual = pPage->GetObjCount();
    // Both (not only the shape) should be removed by now (the textbox wasn't removed, so this was 1).
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), nActual);
}

void SwUiWriterTest::testCp1000071()
{
    SwDoc* pDoc = createDoc("cp1000071.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
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

void SwUiWriterTest::testShapeTextboxVertadjust()
{
    SwDoc* pDoc = createDoc("shape-textbox-vertadjust.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwFrmFmt* pFmt = static_cast<SwContact*>(pObject->GetUserCall())->GetFmt();
    // This was SDRTEXTVERTADJUST_TOP.
    CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_CENTER, pFmt->GetTextVertAdjust().GetValue());
}

void SwUiWriterTest::testShapeTextboxAutosize()
{
    SwDoc* pDoc = createDoc("shape-textbox-autosize.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pFirst = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(OUString("1st"), pFirst->GetName());

    SdrObject* pSecond = pPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL(OUString("2nd"), pSecond->GetName());

    // Shape -> textbox synchronization was missing, the second shape had the
    // same height as the first, even though the first contained 1 paragraph
    // and the other 2 ones.
    CPPUNIT_ASSERT(pFirst->GetSnapRect().getHeight() < pSecond->GetSnapRect().getHeight());
}

void SwUiWriterTest::testFdo82191()
{
    SwDoc* pDoc = createDoc("fdo82191.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    std::set<const SwFrmFmt*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
    // Make sure we have a single draw shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), SwTextBoxHelper::getCount(pPage, aTextBoxes));

    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrObject* pObject = pPage->GetObj(0);
    // Select it, then copy and paste.
    pWrtShell->SelectObj(Point(), 0, pObject);
    pWrtShell->Copy(&aClipboard);
    pWrtShell->Paste(&aClipboard);

    aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
    // This was one: the textbox of the shape wasn't copied.
    CPPUNIT_ASSERT_EQUAL(size_t(2), aTextBoxes.size());
}

void SwUiWriterTest::testCommentedWord()
{
    // This word is commented. <- string in document
    // 123456789 <- character positions
    SwDoc* pDoc = createDoc("commented-word.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Select the word.
    pWrtShell->SelWrd();

    // Make sure that not only the word, but its comment anchor is also selected.
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    // This was 9, only "word", not "word<anchor character>" was selected.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pShellCrsr->End()->nContent.GetIndex());

    // Test that getAnchor() points to "word", not to an empty string.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("word"), xField->getAnchor()->getString());
}


// Chinese conversion tests

static const OUString CHINESE_TRADITIONAL_CONTENT(sal_Unicode(0x9F8D));
static const OUString CHINESE_SIMPLIFIED_CONTENT(sal_Unicode(0x9F99));
static const OUString NON_CHINESE_CONTENT ("Hippopotamus");

// Tests that a blank document is still blank after conversion
void SwUiWriterTest::testChineseConversionBlank()
{

    // Given
    SwDoc* pDoc = createDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    const uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, NULL,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();


    // Then
    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    CPPUNIT_ASSERT_EQUAL(OUString(), pTxtNode->GetTxt());

}

// Tests that non Chinese text is unchanged after conversion
void SwUiWriterTest::testChineseConversionNonChineseText()
{

    // Given
    SwDoc* pDoc = createDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    const uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    pDoc->getIDocumentContentOperations().InsertString(aPaM, NON_CHINESE_CONTENT);


    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, NULL,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();


    // Then
    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    CPPUNIT_ASSERT_EQUAL(NON_CHINESE_CONTENT, pTxtNode->GetTxt());

}

// Tests conversion of traditional Chinese characters to simplified Chinese
void SwUiWriterTest::testChineseConversionTraditionalToSimplified()
{

    // Given
    SwDoc* pDoc = createDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    const uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    pDoc->getIDocumentContentOperations().InsertString(aPaM, CHINESE_TRADITIONAL_CONTENT);


    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, NULL,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();


    // Then
    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    CPPUNIT_ASSERT_EQUAL(CHINESE_SIMPLIFIED_CONTENT, pTxtNode->GetTxt());

}

// Tests conversion of simplified Chinese characters to traditional Chinese
void SwUiWriterTest::testChineseConversionSimplifiedToTraditional()
{

    // Given
    SwDoc* pDoc = createDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    const uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    pDoc->getIDocumentContentOperations().InsertString(aPaM, CHINESE_SIMPLIFIED_CONTENT);


    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_SIMPLIFIED, LANGUAGE_CHINESE_TRADITIONAL, NULL,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();


    // Then
    SwTxtNode* pTxtNode = aPaM.GetNode().GetTxtNode();
    CPPUNIT_ASSERT_EQUAL(CHINESE_TRADITIONAL_CONTENT, pTxtNode->GetTxt());

}

void SwUiWriterTest::testFdo85554()
{
    // Load the document, it contains one shape with a textbox.
    load("/sw/qa/extras/uiwriter/data/", "fdo85554.odt");

    // Add a second shape to the document.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    xDrawPage->add(xShape);

    // Save it and load it back.
    reload("writer8", "fdo85554.odt");

    xDrawPageSupplier.set(mxComponent, uno::UNO_QUERY);
    xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, we lost a shape on export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());
}

void SwUiWriterTest::testBookmarkUndo()
{
    SwDoc* pDoc = createDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    SwPaM aPaM( SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1) );

    pMarkAccess->makeMark(aPaM, OUString("Mark"), IDocumentMarkAccess::BOOKMARK);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark("Mark");
    CPPUNIT_ASSERT(ppBkmk != pMarkAccess->getAllMarksEnd());

    pMarkAccess->renameMark(ppBkmk->get(), "Mark_");
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") != pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") == pMarkAccess->getAllMarksEnd());
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());

    pMarkAccess->deleteMark( pMarkAccess->findMark("Mark_") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
}

void SwUiWriterTest::testCp1000115()
{
    createDoc("cp1000115.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/root/page[2]/body/tab/row/cell[2]/txt");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    // This was 1: the long paragraph in the B1 cell did flow over to the
    // second page, so there was only one paragraph in the second cell of the
    // second page.
    CPPUNIT_ASSERT_EQUAL(2, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent, const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}

void SwUiWriterTest::testDde()
{
    // Type asdf and copy it.
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("asdf");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    uno::Sequence<beans::PropertyValue> aPropertyValues;
    lcl_dispatchCommand(mxComponent, ".uno:Copy", aPropertyValues);

    // Go before the selection and paste as a DDE link.
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    aPropertyValues = uno::Sequence<beans::PropertyValue>(1);
    aPropertyValues[0].Name = "SelectedFormat";
    aPropertyValues[0].Value = uno::makeAny(static_cast<sal_uInt32>(SOT_FORMATSTR_ID_LINK));
    lcl_dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    // Make sure that the document starts with a field now, and its expanded string value contains asdf.
    const uno::Reference< text::XTextRange > xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    CPPUNIT_ASSERT(xField->getString().endsWith("asdf"));
}

void SwUiWriterTest::testTdf89720()
{
#ifndef MACOSX
    SwDoc* pDoc = createDoc("tdf89720.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (SwSidebarItem* pItem : *pPostItMgr)
    {
        if (pItem->pPostIt->IsFollow())
            // This was non-0: reply comments had a text range overlay,
            // resulting in unexpected dark color.
            CPPUNIT_ASSERT(!pItem->pPostIt->TextRange());
    }
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
