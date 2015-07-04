/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
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
#include <swacorr.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <charatr.hxx>
#include <editeng/acorrcfg.hxx>
#include <unotools/streamwrap.hxx>
#include <test/mtfxmldump.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <unotbl.hxx>
#include <pagedesc.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>

#include "UndoManager.hxx"

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include "com/sun/star/util/SearchAlgorithms.hpp"
#include "com/sun/star/i18n/TransliterationModulesExtra.hpp"
#include "com/sun/star/sdbcx/XTablesSupplier.hpp"
#include "com/sun/star/text/XParagraphCursor.hpp"
#include <osl/file.hxx>

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
    void testAutoCorr();
    void testMergeDoc();
    void testCreatePortions();
    void testBookmarkUndo();
    void testFdo85876();
    void testFdo87448();
    void testTdf68183();
    void testCp1000115();
    void testTdf63214();
    void testTdf90003();
    void testTdf51741();
    void testdelofTableRedlines();
    void testExportToPicture();
    void testTdf69282();
    void testTdf69282WithMirror();
    void testSearchWithTransliterate();
    void testTdf90808();
    void testTdf75137();
    void testTdf83798();
    void testTableBackgroundColor();
    void testTdf90362();
    void testUndoCharAttribute();
    void testTdf86639();
    void testTdf90883TableBoxGetCoordinates();
    void testEmbeddedDataSource();
    void testUnoCursorPointer();
    void testTextTableCellNames();
    void testShapeAnchorUndo();

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
    CPPUNIT_TEST(testAutoCorr);
    CPPUNIT_TEST(testMergeDoc);
    CPPUNIT_TEST(testCreatePortions);
    CPPUNIT_TEST(testBookmarkUndo);
    CPPUNIT_TEST(testFdo85876);
    CPPUNIT_TEST(testFdo87448);
    CPPUNIT_TEST(testTdf68183);
    CPPUNIT_TEST(testCp1000115);
    CPPUNIT_TEST(testTdf63214);
    CPPUNIT_TEST(testTdf90003);
    CPPUNIT_TEST(testTdf51741);
    CPPUNIT_TEST(testdelofTableRedlines);
    CPPUNIT_TEST(testExportToPicture);
    CPPUNIT_TEST(testTdf69282);
    CPPUNIT_TEST(testTdf69282WithMirror);
    CPPUNIT_TEST(testSearchWithTransliterate);
    CPPUNIT_TEST(testTdf90808);
    CPPUNIT_TEST(testTdf75137);
    CPPUNIT_TEST(testTdf83798);
    CPPUNIT_TEST(testTableBackgroundColor);
    CPPUNIT_TEST(testTdf90362);
    CPPUNIT_TEST(testUndoCharAttribute);
    CPPUNIT_TEST(testTdf86639);
    CPPUNIT_TEST(testTdf90883TableBoxGetCoordinates);
    CPPUNIT_TEST(testEmbeddedDataSource);
    CPPUNIT_TEST(testUnoCursorPointer);
    CPPUNIT_TEST(testTextTableCellNames);
    CPPUNIT_TEST(testShapeAnchorUndo);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = 0);
};

SwDoc* SwUiWriterTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

//Replacement tests

static void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetContentNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetContentNode(), end);
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

    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTextNode->GetText());
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
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTextNode->GetText());
}

void SwUiWriterTest::testFdo69893()
{
    SwDoc* pDoc = createDoc("fdo69893.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll(); // A1 is empty -> selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.

    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCrsr->End()->nNode.GetNode());
    // Selection did not include the para after table, this was "B1".
    CPPUNIT_ASSERT_EQUAL(OUString("Para after table."), rEnd.GetText());
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
    SvMemoryStream aStream(const_cast<sal_Char*>(aData.getStr()), aData.getLength(), StreamMode::READ);
    SwReader aReader(aStream, OUString(), OUString(), *pWrtShell->GetCrsr());
    Reader* pRTFReader = SwReaderWriter::GetReader(READER_WRITER_RTF);
    CPPUNIT_ASSERT(pRTFReader != 0);
    CPPUNIT_ASSERT_EQUAL(sal_uLong(0), aReader.Read(*pRTFReader));

    sal_uLong nIndex = pWrtShell->GetCrsr()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("fooHello world!"), pDoc->GetNodes()[nIndex - 1]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), pDoc->GetNodes()[nIndex]->GetTextNode()->GetText());
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
    SwInputField aField(static_cast<SwInputFieldType*>(pWrtShell->GetFieldType(0, RES_INPUTFLD)), OUString("foo"), OUString("bar"), 0, 0);
    pWrtShell->Insert(aField);

    // expect hints
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
    CPPUNIT_ASSERT(pTextNode->HasHints());

    // go to the begin of the paragraph and split this node
    pWrtShell->Left(CRSR_SKIP_CHARS, false, 100, false);
    pWrtShell->SplitNode();

    // expect only the second paragraph to have hints
    aIdx = SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    pTextNode = aIdx.GetNode().GetTextNode();
    CPPUNIT_ASSERT(pTextNode->HasHints());
    --aIdx;
    pTextNode = aIdx.GetNode().GetTextNode();
    CPPUNIT_ASSERT(!pTextNode->HasHints());
}

void SwUiWriterTest::testShapeTextboxSelect()
{
    SwDoc* pDoc = createDoc("shape-textbox.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(RES_FLYFRMFMT, static_cast<RES_FMT>(pTextBox->GetFormat()->Which()));

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwDrawContact* pShape = static_cast<SwDrawContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // And finally make sure the shape got selected, not just the textbox itself.
    CPPUNIT_ASSERT_EQUAL(RES_DRAWFRMFMT, static_cast<RES_FMT>(pShape->GetFormat()->Which()));
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

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL( size_t( 2 ), rTable.size());
    sal_uLong redlineStart0NodeIndex = rTable[ 0 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart0Index = rTable[ 0 ]->Start()->nContent.GetIndex();
    sal_uLong redlineEnd0NodeIndex = rTable[ 0 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd0Index = rTable[ 0 ]->End()->nContent.GetIndex();
    sal_uLong redlineStart1NodeIndex = rTable[ 1 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart1Index = rTable[ 1 ]->Start()->nContent.GetIndex();
    sal_uLong redlineEnd1NodeIndex = rTable[ 1 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd1Index = rTable[ 1 ]->End()->nContent.GetIndex();

    // Change the document layout to be 2 columns, and then undo.
    pWrtShell->SelAll();
    SwSectionData section(CONTENT_SECTION, pWrtShell->GetUniqueSectionName());
    SfxItemSet set( pDoc->GetDocShell()->GetPool(), RES_COL, RES_COL, 0 );
    SwFormatCol col;
    col.Init( 2, 0, 10000 );
    set.Put( col );
    pWrtShell->InsertSection( section, &set );
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    // Check that redlines are the same like at the beginning.
    CPPUNIT_ASSERT_EQUAL( size_t( 2 ), rTable.size());
    CPPUNIT_ASSERT_EQUAL( redlineStart0NodeIndex, rTable[ 0 ]->Start()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart0Index, rTable[ 0 ]->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0NodeIndex, rTable[ 0 ]->End()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0Index, rTable[ 0 ]->End()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1NodeIndex, rTable[ 1 ]->Start()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1Index, rTable[ 1 ]->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1NodeIndex, rTable[ 1 ]->End()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1Index, rTable[ 1 ]->End()->nContent.GetIndex());
}

void SwUiWriterTest::testShapeTextboxVertadjust()
{
    SwDoc* pDoc = createDoc("shape-textbox-vertadjust.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwFrameFormat* pFormat = static_cast<SwContact*>(pObject->GetUserCall())->GetFormat();
    // This was SDRTEXTVERTADJUST_TOP.
    CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_CENTER, pFormat->GetTextVertAdjust().GetValue());
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
    std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
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
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(), pTextNode->GetText());

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
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(NON_CHINESE_CONTENT, pTextNode->GetText());

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
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(CHINESE_SIMPLIFIED_CONTENT, pTextNode->GetText());

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
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(CHINESE_TRADITIONAL_CONTENT, pTextNode->GetText());

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

void SwUiWriterTest::testAutoCorr()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    const sal_Unicode cIns = ' ';

    //Normal AutoCorrect
    pWrtShell->Insert("tset");
    pWrtShell->AutoCorrect(corr, cIns);
    sal_uLong nIndex = pWrtShell->GetCrsr()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("Test "), static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    //AutoCorrect with change style to bolt
    pWrtShell->Insert("Bolt");
    pWrtShell->AutoCorrect(corr, cIns);
    nIndex = pWrtShell->GetCrsr()->GetNode().GetIndex();
    const uno::Reference< text::XTextRange > xRun = getRun(getParagraph(1), 2);
    CPPUNIT_ASSERT_EQUAL(OUString("Bolt"), xRun->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));

    //AutoCorrect inserts Table with 2 rows and 3 columns
    pWrtShell->Insert("4xx");
    pWrtShell->AutoCorrect(corr, cIns);
    const uno::Reference< text::XTextTable > xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
}

void SwUiWriterTest::testMergeDoc()
{
    SwDoc* const pDoc1(createDoc("merge-change1.odt"));

    auto xDoc2Component(loadFromDesktop(
            getURLFromSrc(DATA_DIRECTORY) + "merge-change2.odt",
            "com.sun.star.text.TextDocument"));
    auto pxDoc2Document(
            dynamic_cast<SwXTextDocument *>(xDoc2Component.get()));
    CPPUNIT_ASSERT(pxDoc2Document);
    SwDoc* const pDoc2(pxDoc2Document->GetDocShell()->GetDoc());

    SwEditShell* const pEditShell(pDoc1->GetEditShell());
    pEditShell->MergeDoc(*pDoc2);

    // accept all redlines
    while(pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    CPPUNIT_ASSERT_EQUAL(7, getParagraphs());
    getParagraph(1, "Para One: Two Three Four Five");
    getParagraph(2, "Para Two: One Three Four Five");
    getParagraph(3, "Para Three: One Two Four Five");
    getParagraph(4, "Para Four: One Two Three Four Five");
    getParagraph(5, "Para Six: One Three Four Five");
    getParagraph(6, "");
    getParagraph(7, "");
}

void SwUiWriterTest::testCreatePortions()
{
    createDoc("uno-cycle.odt");
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xText(xBookmarksSupplier->getBookmarks()->getByName("Mark"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xTextCursor(xText->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextCursor.is());

    uno::Reference<container::XEnumerationAccess> xParagraph(
            xTextCursor->createEnumeration()->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    // This looped forever in lcl_CreatePortions
    xParagraph->createEnumeration();
}

void SwUiWriterTest::testBookmarkUndo()
{
    SwDoc* pDoc = createDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    SwPaM aPaM( SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1) );

    pMarkAccess->makeMark(aPaM, OUString("Mark"), IDocumentMarkAccess::MarkType::BOOKMARK);
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

static void lcl_setWeight(SwWrtShell* pWrtShell, FontWeight aWeight)
{
    SvxWeightItem aWeightItem(aWeight, EE_CHAR_WEIGHT);
    SvxScriptSetItem aScriptSetItem(SID_ATTR_CHAR_WEIGHT, pWrtShell->GetAttrPool());
    aScriptSetItem.PutItemForScriptType(SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX, aWeightItem);
    pWrtShell->SetAttrSet(aScriptSetItem.GetItemSet());
}

void SwUiWriterTest::testFdo85876()
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    lcl_setWeight(pWrtShell, WEIGHT_BOLD);
    pWrtShell->Insert("test");
    lcl_setWeight(pWrtShell, WEIGHT_NORMAL);
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->Up(false);
    pWrtShell->Insert("test");
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getParagraph(1)));
        CPPUNIT_ASSERT(xCursor.is());
        xCursor->collapseToStart();
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
    {
        auto xCursor(xText->createTextCursorByRange(getParagraph(2)));
        CPPUNIT_ASSERT(xCursor.is());
        xCursor->collapseToStart();
        // this used to be BOLD too with fdo#85876
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }
}

void SwUiWriterTest::testFdo87448()
{
    createDoc("fdo87448.odt");

    // Save the first shape to a metafile.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(comphelper::getProcessComponentContext());
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor =
    {
        beans::PropertyValue("OutputStream", sal_Int32(0), uno::makeAny(xOutputStream), beans::PropertyState_DIRECT_VALUE),
        beans::PropertyValue("FilterName", sal_Int32(0), uno::makeAny(OUString("SVM")), beans::PropertyState_DIRECT_VALUE)
    };
    xGraphicExporter->filter(aDescriptor);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Read it back and dump it as an XML file.
    Graphic aGraphic;
    ReadGraphic(aStream, aGraphic);
    const GDIMetaFile& rMetaFile = aGraphic.GetGDIMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(rMetaFile);

    // The first polyline in the document has a number of points to draw arcs,
    // the last one jumps back to the start, so we call "end" the last but one.
    sal_Int32 nFirstEnd = getXPath(pXmlDoc, "/metafile/polyline[1]/point[last()-1]", "x").toInt32();
    // The second polyline has a different start point, but the arc it draws
    // should end at the ~same position as the first polyline.
    sal_Int32 nSecondEnd = getXPath(pXmlDoc, "/metafile/polyline[2]/point[last()]", "x").toInt32();

    // nFirstEnd was 6023 and nSecondEnd was 6648, now they should be much closer, e.g. nFirstEnd = 6550, nSecondEnd = 6548
    OString aMsg = "nFirstEnd is " + OString::number(nFirstEnd) + ", nSecondEnd is " + OString::number(nSecondEnd);
    // Assert that the difference is less than half point.
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), abs(nFirstEnd - nSecondEnd) < 10);
}

void SwUiWriterTest::testTdf68183()
{
    // First disable RSID and check if indeed no such attribute is inserted.
    SwDoc* pDoc = createDoc();
    SW_MOD()->GetModuleConfig()->SetStoreRsid(false);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert2("X");

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(false, pTextNode->GetSwAttrSet().HasItem(RES_PARATR_RSID));

    // Then enable storing of RSID and make sure that the attribute is inserted.
    SW_MOD()->GetModuleConfig()->SetStoreRsid(true);

    pWrtShell->DelToStartOfLine();
    pWrtShell->Insert2("X");

    CPPUNIT_ASSERT_EQUAL(true, pTextNode->GetSwAttrSet().HasItem(RES_PARATR_RSID));
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

void SwUiWriterTest::testTdf63214()
{
    //This is a crash test
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    pWrtShell->Insert("V");
    {   //limiting the lifetime of SwPaM with a nested scope
        //the shell cursor are automatically adjusted when nodes are deleted, but the shell doesn't know about an SwPaM on the stack
        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        SwPaM aPaM( SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1) );
        aPaM.SetMark();
        aPaM.Move(fnMoveForward, fnGoContent);
        //Inserting a crossRefBookmark
        pMarkAccess->makeMark(aPaM, OUString("Bookmark"), IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    }
    //moving cursor to the end of paragraph
    pWrtShell->EndPara();
    //inserting paragraph break
    pWrtShell->SplitNode();
    rUndoManager.Undo();
    rUndoManager.Redo();
}

void SwUiWriterTest::testTdf90003()
{
    createDoc("tdf90003.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // This was 1: an unexpected fly portion was created, resulting in too
    // large x position for the empty paragraph marker.
    assertXPath(pXmlDoc, "//Special[@nType='POR_FLY']", 0);
}

void SwUiWriterTest::testTdf51741()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    SwPaM aPaM( SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1) );
    //Modification 1
    pMarkAccess->makeMark(aPaM, OUString("Mark"), IDocumentMarkAccess::MarkType::BOOKMARK);
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    //Modification 2
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    //Modification 3
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark("Mark");
    CPPUNIT_ASSERT(ppBkmk != pMarkAccess->getAllMarksEnd());
    //Modification 4
    pMarkAccess->renameMark(ppBkmk->get(), "Mark_");
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());
    //Modification 5
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") != pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") == pMarkAccess->getAllMarksEnd());
    //Modification 6
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());
    //Modification 7
    pMarkAccess->deleteMark( pMarkAccess->findMark("Mark_") );
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    //Modification 8
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    //Modification 9
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
}

void SwUiWriterTest::testdelofTableRedlines()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    const SwTable& tbl = pWrtShell->InsertTable(TableOpt, 1, 3);
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    uno::Sequence<beans::PropertyValue> aDescriptor;
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(tbl.GetTableBox(OUString("A1")))), OUString("TableCellInsert"), aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(tbl.GetTableBox(OUString("B1")))), OUString("TableCellInsert"), aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(tbl.GetTableBox(OUString("C1")))), OUString("TableCellInsert"), aDescriptor);
    IDocumentRedlineAccess& pDocRed = pDoc->getIDocumentRedlineAccess();
    SwExtraRedlineTable& redtbl = pDocRed.GetExtraRedlineTable();
    redtbl.DeleteAllTableRedlines(pDoc, tbl, false, sal_uInt16(USHRT_MAX));
    CPPUNIT_ASSERT(redtbl.IsEmpty());
}

void SwUiWriterTest::testExportToPicture()
{
    createDoc();
    uno::Sequence<beans::PropertyValue> aFilterData =
    {
        beans::PropertyValue("PixelWidth", sal_Int32(0), uno::makeAny(sal_Int32(610)), beans::PropertyState_DIRECT_VALUE),
        beans::PropertyValue("PixelHeight", sal_Int32(0), uno::makeAny(sal_Int32(610)), beans::PropertyState_DIRECT_VALUE)
    };
    uno::Sequence<beans::PropertyValue> aDescriptor =
    {
        beans::PropertyValue("FilterName", sal_Int32(0), uno::makeAny(OUString("writer_png_Export")), beans::PropertyState_DIRECT_VALUE),
        beans::PropertyValue("FilterData", sal_Int32(0), uno::makeAny(aFilterData), beans::PropertyState_DIRECT_VALUE)
    };
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    bool extchk = aTempFile.IsValid();
    CPPUNIT_ASSERT_EQUAL(true, extchk);
    osl::File tmpFile(aTempFile.GetURL());
    tmpFile.open(sal_uInt32(osl_File_OpenFlag_Read));
    sal_uInt64 val;
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, tmpFile.getSize(val));
    CPPUNIT_ASSERT(val > 100);
    aTempFile.EnableKillingFile();
}

void SwUiWriterTest::testTdf69282()
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc* source = pTextDoc->GetDocShell()->GetDoc();
    uno::Reference<lang::XComponent> xSourceDoc(mxComponent, uno::UNO_QUERY);
    mxComponent.clear();
    SwDoc* target = createDoc();
    SwPageDesc* sPageDesc = source->MakePageDesc(OUString("SourceStyle"));
    SwPageDesc* tPageDesc = target->MakePageDesc(OUString("TargetStyle"));
    sPageDesc->ChgFirstShare(false);
    CPPUNIT_ASSERT(!sPageDesc->IsFirstShared());
    SwFrameFormat& sMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    sMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    sMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc(OUString("SourceStyle"), *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& tMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& tLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& tFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& tFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem hMasterFormatSpace = tMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hMasterFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hMasterFormatSpace.GetRight());
    const SvxLRSpaceItem hLeftFormatSpace = tLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hLeftFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hLeftFormatSpace.GetRight());
    const SvxLRSpaceItem hFirstMasterFormatSpace = tFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hFirstMasterFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hFirstMasterFormatSpace.GetRight());
    const SvxLRSpaceItem hFirstLeftFormatSpace = tFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hFirstLeftFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hFirstLeftFormatSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem vMasterFormatSpace = tMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vMasterFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vMasterFormatSpace.GetLower());
    const SvxULSpaceItem vLeftFormatSpace = tLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vLeftFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vLeftFormatSpace.GetLower());
    const SvxULSpaceItem vFirstMasterFormatSpace = tFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vFirstMasterFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vFirstMasterFormatSpace.GetLower());
    const SvxULSpaceItem vFirstLeftFormatSpace = tFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vFirstLeftFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vFirstLeftFormatSpace.GetLower());
    xSourceDoc->dispose();
}

void SwUiWriterTest::testTdf69282WithMirror()
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* source = pTextDoc->GetDocShell()->GetDoc();
    uno::Reference<lang::XComponent> xSourceDoc(mxComponent, uno::UNO_QUERY);
    mxComponent.clear();
    SwDoc* target = createDoc();
    SwPageDesc* sPageDesc = source->MakePageDesc(OUString("SourceStyle"));
    SwPageDesc* tPageDesc = target->MakePageDesc(OUString("TargetStyle"));
    //Enabling Mirror
    sPageDesc->SetUseOn(nsUseOnPage::PD_MIRROR);
    SwFrameFormat& sMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    sMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    sMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc(OUString("SourceStyle"), *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& tMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& tLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& tFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& tFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem hMasterFormatSpace = tMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hMasterFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hMasterFormatSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem hLeftFormatSpace = tLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hLeftFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hLeftFormatSpace.GetRight());
    const SvxLRSpaceItem hFirstMasterFormatSpace = tFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hFirstMasterFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hFirstMasterFormatSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem hFirstLeftFormatSpace = tFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), hFirstLeftFormatSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), hFirstLeftFormatSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem vMasterFormatSpace = tMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vMasterFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vMasterFormatSpace.GetLower());
    const SvxULSpaceItem vLeftFormatSpace = tLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vLeftFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vLeftFormatSpace.GetLower());
    const SvxULSpaceItem vFirstMasterFormatSpace = tFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vFirstMasterFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vFirstMasterFormatSpace.GetLower());
    const SvxULSpaceItem vFirstLeftFormatSpace = tFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), vFirstLeftFormatSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), vFirstLeftFormatSpace.GetLower());
    xSourceDoc->dispose();
}

void SwUiWriterTest::testSearchWithTransliterate()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    pDoc->getIDocumentContentOperations().InsertString(aPaM,"This is paragraph one");
    pWrtShell->SplitNode();
    aIdx = SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    aPaM = SwPaM(aIdx);
    pDoc->getIDocumentContentOperations().InsertString(aPaM,"This is Other PARAGRAPH");
    com::sun::star::util::SearchOptions SearchOpt;
    SearchOpt.algorithmType = com::sun::star::util::SearchAlgorithms_ABSOLUTE;
    SearchOpt.searchFlag = 0x00000001;
    SearchOpt.searchString = "other";
    SearchOpt.replaceString.clear();
    SearchOpt.changedChars = 0;
    SearchOpt.deletedChars = 0;
    SearchOpt.insertedChars = 0;
    SearchOpt.transliterateFlags = com::sun::star::i18n::TransliterationModulesExtra::IGNORE_DIACRITICS_CTL;
    //transliteration option set so that at least one of the search strings is not found
    sal_uLong case1 = pWrtShell->SearchPattern(SearchOpt,true,DOCPOS_START,DOCPOS_END,FND_IN_BODY,false);
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(true);
    CPPUNIT_ASSERT_EQUAL(OUString(""),pShellCrsr->GetText());
    CPPUNIT_ASSERT_EQUAL(0,(int)case1);
    SearchOpt.searchString = "paragraph";
    SearchOpt.transliterateFlags = com::sun::star::i18n::TransliterationModulesExtra::IGNORE_KASHIDA_CTL;
    //transliteration option set so that all search strings are found
    sal_uLong case2 = pWrtShell->SearchPattern(SearchOpt,true,DOCPOS_START,DOCPOS_END,FND_IN_BODY,false);
    pShellCrsr = pWrtShell->getShellCrsr(true);
    CPPUNIT_ASSERT_EQUAL(OUString("paragraph"),pShellCrsr->GetText());
    CPPUNIT_ASSERT_EQUAL(1,(int)case2);
}

void SwUiWriterTest::testTdf90808()
{
    createDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextRange->getText(), uno::UNO_QUERY);
    uno::Reference<text::XParagraphCursor> xCrsr(xText->createTextCursor(), uno::UNO_QUERY);
    //inserting text into document so that the paragraph is not empty
    xText->setString(OUString("Hello World!"));
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    //creating bookmark 1
    uno::Reference<text::XTextContent> type1bookmark1(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xName1(type1bookmark1, uno::UNO_QUERY);
    xName1->setName("__RefHeading__1");
    //moving cursor to the starting of paragraph
    xCrsr->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCrsr, type1bookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> type1bookmark2(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xName2(type1bookmark2, uno::UNO_QUERY);
    xName2->setName("__RefHeading__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCrsr->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCrsr, type1bookmark2, true), com::sun::star::lang::IllegalArgumentException);
    //now testing for __RefNumPara__
    //creating bookmark 1
    uno::Reference<text::XTextContent> type2bookmark1(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xName3(type2bookmark1, uno::UNO_QUERY);
    xName3->setName("__RefNumPara__1");
    //moving cursor to the starting of paragraph
    xCrsr->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCrsr, type2bookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> type2bookmark2(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xName4(type2bookmark2, uno::UNO_QUERY);
    xName4->setName("__RefNumPara__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCrsr->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCrsr, type2bookmark2, true), com::sun::star::lang::IllegalArgumentException);
}

void SwUiWriterTest::testTdf75137()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(true);
    pWrtShell->InsertFootnote(OUString("This is first footnote"));
    sal_uLong firstIndex = pShellCrsr->GetNode().GetIndex();
    pShellCrsr->GotoFootnoteAnchor();
    pWrtShell->InsertFootnote(OUString("This is second footnote"));
    pWrtShell->Up(false, 1, false);
    sal_uLong secondIndex = pShellCrsr->GetNode().GetIndex();
    pWrtShell->Down(false, 1, false);
    sal_uLong thirdIndex = pShellCrsr->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(firstIndex, thirdIndex);
    CPPUNIT_ASSERT(firstIndex != secondIndex);
}

void SwUiWriterTest::testTdf83798()
{
    SwDoc* pDoc = createDoc("tdf83798.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase, nullptr);
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCrsr();
    pCrsr->SetMark();
    pCrsr->Move(fnMoveForward, fnGoNode);
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents"), pCrsr->GetText());
    pCrsr->DeleteMark();
    pCrsr->SetMark();
    pCrsr->Move(fnMoveForward, fnGoContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pCrsr->GetText());
    pCrsr->DeleteMark();
    pCrsr->Move(fnMoveForward, fnGoNode);
    pCrsr->SetMark();
    pCrsr->Move(fnMoveForward, fnGoContent);
    pCrsr->Move(fnMoveForward, fnGoContent);
    pCrsr->Move(fnMoveForward, fnGoContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1.A"), pCrsr->GetText());
    pCrsr->DeleteMark();
    pCrsr->Move(fnMoveForward, fnGoNode);
    pCrsr->SetMark();
    pCrsr->Move(fnMoveForward, fnGoContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pCrsr->GetText());
    pCrsr->DeleteMark();
    pCrsr->Move(fnMoveForward, fnGoNode);
    pCrsr->SetMark();
    pCrsr->Move(fnMoveForward, fnGoContent);
    pCrsr->Move(fnMoveForward, fnGoContent);
    pCrsr->Move(fnMoveForward, fnGoContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2.A"), pCrsr->GetText());
    pCrsr->DeleteMark();
}

void SwUiWriterTest::testTableBackgroundColor()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    pWrtShell->InsertTable(TableOpt, 3, 3); //Inserting Table
    //Checking Rows and Columns of Inserted Table
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    pWrtShell->SttDoc();
    pWrtShell->SelTableRow(); //Selecting First Row
    pWrtShell->ClearMark();
    //Modifying the color of Table Box
    Color colour = sal_Int32(0xFF00FF);
    pWrtShell->SetBoxBackground(SvxBrushItem(colour, sal_Int16(RES_BACKGROUND)));
    //Checking cells for background color only A1 should be modified
    uno::Reference<table::XCell> xCell;
   xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF00FF), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("A2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("A3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
}

void SwUiWriterTest::testTdf90362()
{
    // First check if the end of the second paragraph is indeed protected.
    SwDoc* pDoc = createDoc("tdf90362.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara();
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->HasReadonlySel());

    // Then enable ignoring of protected areas and make sure that this time the cursor is read-write.
    pWrtShell->Up(/*bSelect=*/false);
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetIgnoreProtectedArea(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
}

void SwUiWriterTest::testUndoCharAttribute()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createDoc();
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCrsr();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCrsr, "This will be bolded");
    // Position of word                   9876543210
    // Use cursor to select part of text
    pCrsr->SetMark();
    for (int i = 0; i < 9; i++) {
        pCrsr->Move(fnMoveBackward);
    }
    // Check that correct text was selected
    CPPUNIT_ASSERT_EQUAL(OUString("be bolded"), pCrsr->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCrsr, aWeightItem);
    SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT);
    // Adds selected text's attributes to aSet
    pCrsr->GetNode().GetTextNode()->GetAttr(aSet, 10, 19);
    SfxPoolItem const * aPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    SfxPoolItem& ampPoolItem = aWeightItem;
    // Check that bold is active on the selection; checks if it's in aSet
    CPPUNIT_ASSERT_EQUAL((*aPoolItem == ampPoolItem), true);
    // Invoke Undo
    rUndoManager.Undo();
    // Check that bold is no longer active
    aSet.ClearItem(RES_CHRATR_WEIGHT);
    pCrsr->GetNode().GetTextNode()->GetAttr(aSet, 10, 19);
    aPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL((*aPoolItem == ampPoolItem), false);
}

void SwUiWriterTest::testTdf86639()
{
    SwDoc* pDoc = createDoc("tdf86639.rtf");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextFormatColl* pColl = pDoc->FindTextFormatCollByName("Heading");
    pWrtShell->SetTextFormatColl(pColl);
    OUString aExpected = pColl->GetAttrSet().GetFont().GetFamilyName();
    // This was Calibri, should be Liberation Sans.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

void SwUiWriterTest::testTdf90883TableBoxGetCoordinates()
{
    SwDoc* pDoc = createDoc("tdf90883.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(true);
    SwSelBoxes aBoxes;
    ::GetTableSel( *pWrtShell, aBoxes );
    CPPUNIT_ASSERT_EQUAL( 2, (int)aBoxes.size() );
    Point pos ( aBoxes[0]->GetCoordinates() );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pos.X() );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pos.Y() );
    pos = aBoxes[1]->GetCoordinates();
    CPPUNIT_ASSERT_EQUAL( 1, (int)pos.X() );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pos.Y() );
}

void SwUiWriterTest::testEmbeddedDataSource()
{
    // Initially no data source.
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(xComponentContext);
    CPPUNIT_ASSERT(!xDatabaseContext->hasByName("calc-data-source"));

    // Load: should have a component and a data source, too.
    load(DATA_DIRECTORY, "embedded-data-source.odt");
    CPPUNIT_ASSERT(mxComponent.is());
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1.
    uno::Reference<sdbc::XDataSource> xDataSource(xDatabaseContext->getByName("calc-data-source"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    uno::Reference<sdbcx::XTablesSupplier> xConnection(xDataSource->getConnection("", ""), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables(xConnection->getTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));

    // Reload: should still have a component and a data source, too.
    reload("writer8", "embedded-data-source.odt");
    CPPUNIT_ASSERT(mxComponent.is());
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1 after saving to a different directory.
    xDataSource.set(xDatabaseContext->getByName("calc-data-source"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    xConnection.set(xDataSource->getConnection("", ""), uno::UNO_QUERY);
    xTables.set(xConnection->getTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));

    // Close: should not have a data source anymore.
    mxComponent->dispose();
    mxComponent.clear();
    CPPUNIT_ASSERT(!xDatabaseContext->hasByName("calc-data-source"));

    // Now open again the saved result, and instead of 'save as', just 'save'.
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->store();
}

void SwUiWriterTest::testUnoCursorPointer()
{
    auto xDocComponent(loadFromDesktop("private:factory/swriter",
            "com.sun.star.text.TextDocument"));
    auto pxDocDocument(
            dynamic_cast<SwXTextDocument *>(xDocComponent.get()));
    CPPUNIT_ASSERT(pxDocDocument);
    SwDoc* const pDoc(pxDocDocument->GetDocShell()->GetDoc());
    std::unique_ptr<SwNodeIndex> xIdx(new SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1));
    std::unique_ptr<SwPosition> xPos(new SwPosition(*xIdx));
    sw::UnoCursorPointer pCursor(pDoc->CreateUnoCrsr(*xPos));
    CPPUNIT_ASSERT(static_cast<bool>(pCursor));
    xPos.reset(); // we need to kill the SwPosition before disposing
    xIdx.reset(); // we need to kill the SwNodeIndex before disposing
    xDocComponent->dispose();
    CPPUNIT_ASSERT(!static_cast<bool>(pCursor));
}

void SwUiWriterTest::testTextTableCellNames()
{
    sal_Int32 nCol, nRow2;
    SwXTextTable::GetCellPosition( OUString("z1"), nCol, nRow2);
    CPPUNIT_ASSERT(nCol == 51);
    SwXTextTable::GetCellPosition( OUString("AA1"), nCol, nRow2);
    CPPUNIT_ASSERT(nCol == 52);
    SwXTextTable::GetCellPosition( OUString("AB1"), nCol, nRow2);
    CPPUNIT_ASSERT(nCol == 53);
    SwXTextTable::GetCellPosition( OUString("BB1"), nCol, nRow2);
    CPPUNIT_ASSERT(nCol == 105);
}

void SwUiWriterTest::testShapeAnchorUndo()
{
    SwDoc* pDoc = createDoc("draw-anchor-undo.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Rectangle aOrigLogicRect(pObject->GetLogicRect());

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.StartUndo(UNDO_START, NULL);

    pWrtShell->SelectObj(Point(), 0, pObject);

    pWrtShell->GetDrawView()->MoveMarkedObj(Size(100, 100), false);
    pWrtShell->ChgAnchor(0, true, true);

    rUndoManager.EndUndo(UNDO_END, NULL);

    CPPUNIT_ASSERT(aOrigLogicRect != pObject->GetLogicRect());

    rUndoManager.Undo();

    CPPUNIT_ASSERT(aOrigLogicRect == pObject->GetLogicRect());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
