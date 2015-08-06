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
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <tools/errcode.hxx>
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
#include <postithelper.hxx>
#include <PostItMgr.hxx>
#include <SidebarWin.hxx>
#include "com/sun/star/text/XDefaultNumberingProvider.hpp"
#include "com/sun/star/awt/FontUnderline.hpp"

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>

#include "UndoManager.hxx"
#include <textsh.hxx>
#include <frmmgr.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "com/sun/star/util/XNumberFormatTypes.hpp"
#include "com/sun/star/util/NumberFormat.hpp"
#include "com/sun/star/util/XNumberFormatsSupplier.hpp"
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include "com/sun/star/util/SearchAlgorithms.hpp"
#include "com/sun/star/i18n/TransliterationModulesExtra.hpp"
#include "com/sun/star/sdbcx/XTablesSupplier.hpp"
#include "com/sun/star/text/XParagraphCursor.hpp"
#include "com/sun/star/util/XPropertyReplace.hpp"
#include "com/sun/star/awt/FontStrikeout.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/text/XTextField.hpp"
#include "com/sun/star/text/TextMarkupType.hpp"
#include <osl/file.hxx>
#include <paratr.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

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
    void testDefaultsOfOutlineNumbering();
    void testdelofTableRedlines();
    void testXFlatParagraph();
    void testTdf81995();
    void testExportToPicture();
    void testTdf77340();
    void testTdf79236();
    void testTextSearch();
    void testTdf69282();
    void testTdf69282WithMirror();
    void testTdf78742();
    void testUnoParagraph();
    void testSearchWithTransliterate();
    void testTdf80663();
    void testTdf57197();
    void testTdf90808();
    void testTdf75137();
    void testTdf83798();
    void testPropertyDefaults();
    void testTableBackgroundColor();
    void testTdf88899();
    void testTdf90362();
    void testUndoCharAttribute();
    void testTdf86639();
    void testTdf90883TableBoxGetCoordinates();
    void testEmbeddedDataSource();
    void testUnoCursorPointer();
    void testTextTableCellNames();
    void testShapeAnchorUndo();
    void testDde();
    void testTdf89954();
    void testTdf89720();
    void testTdf88986();

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
    CPPUNIT_TEST(testDefaultsOfOutlineNumbering);
    CPPUNIT_TEST(testdelofTableRedlines);
    CPPUNIT_TEST(testXFlatParagraph);
    CPPUNIT_TEST(testTdf81995);
    CPPUNIT_TEST(testExportToPicture);
    CPPUNIT_TEST(testTdf77340);
    CPPUNIT_TEST(testTdf79236);
    CPPUNIT_TEST(testTextSearch);
    CPPUNIT_TEST(testTdf69282);
    CPPUNIT_TEST(testTdf69282WithMirror);
    CPPUNIT_TEST(testTdf78742);
    CPPUNIT_TEST(testUnoParagraph);
    CPPUNIT_TEST(testSearchWithTransliterate);
    CPPUNIT_TEST(testTdf80663);
    CPPUNIT_TEST(testTdf57197);
    CPPUNIT_TEST(testTdf90808);
    CPPUNIT_TEST(testTdf75137);
    CPPUNIT_TEST(testTdf83798);
    CPPUNIT_TEST(testPropertyDefaults);
    CPPUNIT_TEST(testTableBackgroundColor);
    CPPUNIT_TEST(testTdf88899);
    CPPUNIT_TEST(testTdf90362);
    CPPUNIT_TEST(testUndoCharAttribute);
    CPPUNIT_TEST(testTdf86639);
    CPPUNIT_TEST(testTdf90883TableBoxGetCoordinates);
    CPPUNIT_TEST(testEmbeddedDataSource);
    CPPUNIT_TEST(testUnoCursorPointer);
    CPPUNIT_TEST(testTextTableCellNames);
    CPPUNIT_TEST(testShapeAnchorUndo);
    CPPUNIT_TEST(testDde);
    CPPUNIT_TEST(testTdf89954);
    CPPUNIT_TEST(testTdf89720);
    CPPUNIT_TEST(testTdf88986);
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

void SwUiWriterTest::testDefaultsOfOutlineNumbering()
{
    uno::Reference<text::XDefaultNumberingProvider> xDefNum(m_xSFactory->createInstance("com.sun.star.text.DefaultNumberingProvider"), uno::UNO_QUERY);
    com::sun::star::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    uno::Sequence<beans::PropertyValues> aPropVal(xDefNum->getDefaultContinuousNumberingLevels(alocale));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aPropVal.getLength());
    for(int i=0;i<aPropVal.getLength();i++)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aPropVal[i].getLength());
        for(int j=0;j<aPropVal[i].getLength();j++)
        {
            uno::Any aAny = (aPropVal[i])[j].Value;
            if((aPropVal[i])[j].Name == "Prefix" || (aPropVal[i])[j].Name == "Suffix" || (aPropVal[i])[j].Name == "Transliteration")
                CPPUNIT_ASSERT_EQUAL(OUString("string"), aAny.getValueTypeName());
            else if((aPropVal[i])[j].Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(OUString("short"), aAny.getValueTypeName());
            else if((aPropVal[i])[j].Name == "NatNum")
                CPPUNIT_ASSERT_EQUAL(OUString("short"), aAny.getValueTypeName());
                //It is expected to be long but right now its short !error!
            else
                CPPUNIT_FAIL("Property Name not matched");
        }
    }
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

void SwUiWriterTest::testXFlatParagraph()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting some text in the document
    pWrtShell->Insert("This is sample text");
    pWrtShell->SplitNode();
    pWrtShell->Insert("This is another sample text");
    pWrtShell->SplitNode();
    pWrtShell->Insert("This is yet another sample text");
    //retrieving the XFlatParagraphs
    uno::Reference<text::XFlatParagraphIteratorProvider> xFPIP(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFlatParagraphIterator> xFPIterator(xFPIP->getFlatParagraphIterator(sal_Int32(text::TextMarkupType::SPELLCHECK), true));
    uno::Reference<text::XFlatParagraph> xFlatPara(xFPIterator->getFirstPara());
    CPPUNIT_ASSERT_EQUAL(OUString("This is sample text"), xFlatPara->getText());
    //checking modified status
    CPPUNIT_ASSERT(!xFlatPara->isModified());
    //checking "checked" staus, modifying it and asserting the changes
    CPPUNIT_ASSERT(!xFlatPara->isChecked(sal_Int32(text::TextMarkupType::SPELLCHECK)));
    xFlatPara->setChecked((sal_Int32(text::TextMarkupType::SPELLCHECK)), true);
    CPPUNIT_ASSERT(xFlatPara->isChecked(sal_Int32(text::TextMarkupType::SPELLCHECK)));
    //getting other XFlatParagraphs and asserting their contents
    uno::Reference<text::XFlatParagraph> xFlatPara2(xFPIterator->getParaAfter(xFlatPara));
    CPPUNIT_ASSERT_EQUAL(OUString("This is another sample text"), xFlatPara2->getText());
    uno::Reference<text::XFlatParagraph> xFlatPara3(xFPIterator->getParaAfter(xFlatPara2));
    CPPUNIT_ASSERT_EQUAL(OUString("This is yet another sample text"), xFlatPara3->getText());
    uno::Reference<text::XFlatParagraph> xFlatPara4(xFPIterator->getParaBefore(xFlatPara3));
    CPPUNIT_ASSERT_EQUAL(xFlatPara2->getText(), xFlatPara4->getText());
    //changing the attributes of last para
    uno::Sequence<beans::PropertyValue> aDescriptor =
    {
        beans::PropertyValue("CharWeight", sal_Int32(0), uno::makeAny(float(com::sun::star::awt::FontWeight::BOLD)), beans::PropertyState_DIRECT_VALUE)
    };
    xFlatPara3->changeAttributes(sal_Int32(0), sal_Int32(5), aDescriptor);
    //checking Language Portions
    uno::Sequence<::sal_Int32> aLangPortions(xFlatPara4->getLanguagePortions());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aLangPortions.getLength());
    //examining Language of text
    com::sun::star::lang::Locale alocale = xFlatPara4->getLanguageOfText(sal_Int32(0), sal_Int32(4));
    CPPUNIT_ASSERT_EQUAL(OUString("en"), alocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), alocale.Country);
    //examining Primary Language of text
    com::sun::star::lang::Locale aprimarylocale = xFlatPara4->getPrimaryLanguageOfText(sal_Int32(0), sal_Int32(20));
    CPPUNIT_ASSERT_EQUAL(OUString("en"), aprimarylocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), aprimarylocale.Country);
}

void SwUiWriterTest::testTdf81995()
{
    uno::Reference<text::XDefaultNumberingProvider> xDefNum(m_xSFactory->createInstance("com.sun.star.text.DefaultNumberingProvider"), uno::UNO_QUERY);
    com::sun::star::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    uno::Sequence<uno::Reference<container::XIndexAccess>> aIndexAccess(xDefNum->getDefaultOutlineNumberings(alocale));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aIndexAccess.getLength());
    for(int i=0;i<aIndexAccess.getLength();i++)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aIndexAccess[i]->getCount());
        for(int j=0;j<aIndexAccess[i]->getCount();j++)
        {
            uno::Sequence<beans::PropertyValue> aProps;
            aIndexAccess[i]->getByIndex(j) >>= aProps;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aProps.getLength());
            for(int k=0;k<aProps.getLength();k++)
            {
                const beans::PropertyValue& rProp = aProps[k];
                uno::Any aAny = rProp.Value;
                if(rProp.Name == "Prefix" || rProp.Name == "Suffix" || rProp.Name == "BulletChar" || rProp.Name == "BulletFontName" || rProp.Name == "Transliteration")
                    CPPUNIT_ASSERT_EQUAL(OUString("string"), aAny.getValueTypeName());
                else if(rProp.Name == "NumberingType" || rProp.Name == "ParentNumbering" || rProp.Name == "Adjust")
                    CPPUNIT_ASSERT_EQUAL(OUString("short"), aAny.getValueTypeName());
                else if(rProp.Name == "LeftMargin" || rProp.Name == "SymbolTextDistance" || rProp.Name == "FirstLineOffset" || rProp.Name == "NatNum")
                    CPPUNIT_ASSERT_EQUAL(OUString("long"), aAny.getValueTypeName());
                else
                    CPPUNIT_FAIL("Property Name not matched");
            }
        }
    }
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

void SwUiWriterTest::testTdf77340()
{
    createDoc();
    //Getting some paragraph style in our document
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xFactory->createInstance("com.sun.star.style.ParagraphStyle"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(OUString("ParaBackColor"), uno::makeAny(sal_Int32(0xFF00FF)));
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess(xSupplier->getStyleFamilies());
    uno::Reference<container::XNameContainer> xNameCont;
    xNameAccess->getByName("ParagraphStyles") >>= xNameCont;
    xNameCont->insertByName(OUString("myStyle"), uno::makeAny(xStyle));
    CPPUNIT_ASSERT_EQUAL(OUString("myStyle"), xStyle->getName());
    //Setting the properties with proper values
    xPropSet->setPropertyValue(OUString("PageDescName"), uno::makeAny(OUString("First Page")));
    xPropSet->setPropertyValue(OUString("PageNumberOffset"), uno::makeAny(sal_Int16(3)));
    //Getting the properties and checking that they have proper values
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(OUString("First Page")), xPropSet->getPropertyValue(OUString("PageDescName")));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_Int16(3)), xPropSet->getPropertyValue(OUString("PageNumberOffset")));
}

void SwUiWriterTest::testTdf79236()
{
    SwDoc* pDoc = createDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //Getting some paragraph style
    SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName(OUString("Text Body"));
    const SwAttrSet& attrSet = pTextFormat->GetAttrSet();
    SfxItemSet* itemSet = attrSet.Clone();
    sal_uInt16 initialCount = itemSet->Count();
    SvxAdjustItem AdjustItem = attrSet.GetAdjust(true);
    SvxAdjust initialAdjust = AdjustItem.GetAdjust();
    //By default the adjust is LEFT
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_LEFT, initialAdjust);
    //Changing the adjust to RIGHT
    AdjustItem.SetAdjust(SVX_ADJUST_RIGHT);
    //Checking whether the change is made or not
    SvxAdjust modifiedAdjust = AdjustItem.GetAdjust();
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_RIGHT, modifiedAdjust);
    //Modifying the itemset, putting *one* item
    itemSet->Put(AdjustItem);
    //The count should increment by 1
    sal_uInt16 modifiedCount = itemSet->Count();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(initialCount + 1), modifiedCount);
    //Setting the updated item set on the style
    pDoc->ChgFormat(*pTextFormat, *itemSet);
    //Checking the Changes
    SwTextFormatColl* pTextFormat2 = pDoc->FindTextFormatCollByName(OUString("Text Body"));
    const SwAttrSet& attrSet2 = pTextFormat2->GetAttrSet();
    const SvxAdjustItem& AdjustItem2 = attrSet2.GetAdjust(true);
    SvxAdjust Adjust2 = AdjustItem2.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_RIGHT, Adjust2);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat3 = pDoc->FindTextFormatCollByName(OUString("Text Body"));
    const SwAttrSet& attrSet3 = pTextFormat3->GetAttrSet();
    const SvxAdjustItem& AdjustItem3 = attrSet3.GetAdjust(true);
    SvxAdjust Adjust3 = AdjustItem3.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_LEFT, Adjust3);
    //Redo the changes
    rUndoManager.Redo();
    SwTextFormatColl* pTextFormat4 = pDoc->FindTextFormatCollByName(OUString("Text Body"));
    const SwAttrSet& attrSet4 = pTextFormat4->GetAttrSet();
    const SvxAdjustItem& AdjustItem4 = attrSet4.GetAdjust(true);
    SvxAdjust Adjust4 = AdjustItem4.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_RIGHT, Adjust4);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat5 = pDoc->FindTextFormatCollByName(OUString("Text Body"));
    const SwAttrSet& attrSet5 = pTextFormat5->GetAttrSet();
    const SvxAdjustItem& AdjustItem5 = attrSet5.GetAdjust(true);
    SvxAdjust Adjust5 = AdjustItem5.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_LEFT, Adjust5);
}

void SwUiWriterTest::testTextSearch()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createDoc();
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCrsr();
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCrsr, "Hello World This is a test");
    // Use cursor to select part of text
    for (int i = 0; i < 10; i++) {
        pCrsr->Move(fnMoveBackward);
    }
    pCrsr->SetMark();
    for(int i = 0; i < 4; i++) {
        pCrsr->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("This"), pCrsr->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCrsr, aWeightItem);
    //making another selection of text
    for (int i = 0; i < 7; i++) {
        pCrsr->Move(fnMoveBackward);
    }
    pCrsr->SetMark();
    for(int i = 0; i < 5; i++) {
        pCrsr->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), pCrsr->GetText());
    // Apply a "Bold" attribute to selection
    rIDCO.InsertPoolItem(*pCrsr, aWeightItem);
    //Performing Search Operation and also covering the UNO coverage for setProperty
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes(xSearch->createSearchDescriptor(), uno::UNO_QUERY);
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);
    //setting some properties
    uno::Sequence<beans::PropertyValue> aDescriptor =
    {
        beans::PropertyValue("CharWeight", sal_Int32(0), uno::makeAny(float(com::sun::star::awt::FontWeight::BOLD)), beans::PropertyState_DIRECT_VALUE)
    };
    xProp->setSearchAttributes(aDescriptor);
    //receiving the defined properties and asserting them with expected values, covering UNO
    uno::Sequence<beans::PropertyValue> aPropVal2(xProp->getSearchAttributes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPropVal2.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("CharWeight"), aPropVal2[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(com::sun::star::awt::FontWeight::BOLD)), aPropVal2[0].Value);
    //specifying the search attributes
    uno::Reference<beans::XPropertySet> xPropSet(xSearchDes, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(OUString("SearchWords"), uno::makeAny(true));
    xPropSet->setPropertyValue(OUString("SearchCaseSensitive"), uno::makeAny(true));
    //this will search all the BOLD words
    uno::Reference<container::XIndexAccess> xIndex(xSearch->findAll(xSearchDes));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndex->getCount());
    //Replacing the searched string via XReplaceable
    uno::Reference<util::XReplaceable> xReplace(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XReplaceDescriptor> xReplaceDes(xReplace->createReplaceDescriptor(), uno::UNO_QUERY);
    uno::Reference<util::XPropertyReplace> xProp2(xReplaceDes, uno::UNO_QUERY);
    xProp2->setReplaceAttributes(aDescriptor);
    //checking that the proper attributes are there or not
    uno::Sequence<beans::PropertyValue> aRepProp(xProp2->getReplaceAttributes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRepProp.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("CharWeight"), aRepProp[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(com::sun::star::awt::FontWeight::BOLD)), aRepProp[0].Value);
    //setting strings for replacement
    xReplaceDes->setSearchString("test");
    xReplaceDes->setReplaceString("task");
    //checking the replaceString
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xReplaceDes->getReplaceString());
    //this will replace *normal*test to *bold*task
    sal_Int32 ReplaceCount = xReplace->replaceAll(xReplaceDes);
    //There should be only 1 replacement since there is only one occurrence of "test" in the document
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    //Now performing search again for BOLD words, count should be 3 due to replacement
    uno::Reference<container::XIndexAccess> xIndex2(xReplace->findAll(xSearchDes));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndex2->getCount());
}

void SwUiWriterTest::testTdf69282()
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

void SwUiWriterTest::testTdf78742()
{
    //testing with service type and any .ods file
    OUString path = getURLFromSrc(DATA_DIRECTORY) + "calc-data-source.ods";
    SfxMedium aMedium(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher(OUString("com.sun.star.text.TextDocument"));
    const SfxFilter* pFilter = nullptr;
    sal_uInt32 filter = aMatcher.DetectFilter(aMedium, &pFilter, true);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_IO_ABORT, filter);
    //it should not return any Filter
    CPPUNIT_ASSERT(!pFilter);
    //testing without service type and any .ods file
    SfxMedium aMedium2(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher2;
    const SfxFilter* pFilter2 = nullptr;
    sal_uInt32 filter2 = aMatcher2.DetectFilter(aMedium2, &pFilter2, true);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_CLASS_NONE, filter2);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("calc8"), pFilter2->GetFilterName());
    //testing with service type and any .odt file
    OUString path2 = getURLFromSrc(DATA_DIRECTORY) + "fdo69893.odt";
    SfxMedium aMedium3(path2, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher3(OUString("com.sun.star.text.TextDocument"));
    const SfxFilter* pFilter3 = nullptr;
    sal_uInt32 filter3 = aMatcher3.DetectFilter(aMedium3, &pFilter3, true);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_CLASS_NONE, filter3);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("writer8"), pFilter3->GetFilterName());
}

void SwUiWriterTest::testUnoParagraph()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting some text content in the document
    pWrtShell->Insert("This is initial text in paragraph one");
    pWrtShell->SplitNode();
    //Inserting second paragraph
    pWrtShell->Insert("This is initial text in paragraph two");
    //now testing the SwXParagraph
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextDocument->getText());
    uno::Reference<container::XEnumerationAccess> xParaAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPara(xParaAccess->createEnumeration());
    //getting first paragraph
    uno::Reference<text::XTextContent> xFirstParaContent(xPara->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFirstPara(xFirstParaContent, uno::UNO_QUERY);
    //testing the initial text
    CPPUNIT_ASSERT_EQUAL(OUString("This is initial text in paragraph one"), xFirstPara->getString());
    //changing the text content in first paragraph
    xFirstPara->setString("This is modified text in paragraph one");
    //testing the changes
    CPPUNIT_ASSERT_EQUAL(OUString("This is modified text in paragraph one"), xFirstPara->getString());
    //getting second paragraph
    uno::Reference<text::XTextContent> xSecondParaContent(xPara->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xSecondPara(xSecondParaContent, uno::UNO_QUERY);
    //testing the initial text
    CPPUNIT_ASSERT_EQUAL(OUString("This is initial text in paragraph two"), xSecondPara->getString());
    //changing the text content in second paragraph
    xSecondPara->setString("This is modified text in paragraph two");
    //testing the changes
    CPPUNIT_ASSERT_EQUAL(OUString("This is modified text in paragraph two"), xSecondPara->getString());
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

void SwUiWriterTest::testTdf80663()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 2x2 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the first row
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->SelTableRow(); //selects the first row
    pWrtShell->DeleteRow();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the second row
    pWrtShell->GoNextCell(); //moves the cursor to next cell
    pWrtShell->SelTableRow(); //selects the second row
    pWrtShell->DeleteRow();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the first column
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->SelTableCol(); //selects first column
    pWrtShell->DeleteCol();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the second column
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->GoNextCell(); //moves the cursor to next cell
    pWrtShell->SelTableCol(); //selects second column
    pWrtShell->DeleteCol();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
}

void SwUiWriterTest::testTdf57197()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 1x1 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    pWrtShell->InsertTable(TableOpt, 1, 1);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one row before the existing row
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->InsertRow(1, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Insering one row after the existing row
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->InsertRow(1, true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one column before the existing column
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->InsertCol(1, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
     //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one column after the existing column
    pWrtShell->SttDoc(); //moves the cursor to the start of Doc
    pWrtShell->InsertCol(1, true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
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

void SwUiWriterTest::testPropertyDefaults()
{
    createDoc();
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xInterface(xFact->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyState> xPropState(xInterface, uno::UNO_QUERY);
    //testing CharFontName from style::CharacterProperties
    //getting property default
    uno::Any aCharFontName = xPropState->getPropertyDefault(OUString("CharFontName"));
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharFontName")), aCharFontName);
    //changing the default value
    xPropSet->setPropertyValue(OUString("CharFontName"), uno::makeAny(OUString("Symbol")));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(OUString("Symbol")), xPropSet->getPropertyValue(OUString("CharFontName")));
    //resetting the value to default
    xPropState->setPropertyToDefault(OUString("CharFontName"));
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharFontName")), aCharFontName);
    //testing CharHeight from style::CharacterProperties
    //getting property default
    uno::Any aCharHeight = xPropState->getPropertyDefault(OUString("CharHeight"));
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharHeight")), aCharHeight);
    //changing the default value
    xPropSet->setPropertyValue(OUString("CharHeight"), uno::makeAny(float(14)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(14)), xPropSet->getPropertyValue(OUString("CharHeight")));
    //resetting the value to default
    xPropState->setPropertyToDefault(OUString("CharHeight"));
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharHeight")), aCharHeight);
    //testing CharWeight from style::CharacterProperties
    uno::Any aCharWeight = xPropSet->getPropertyValue(OUString("CharWeight"));
    //changing the default value
    xPropSet->setPropertyValue(OUString("CharWeight"), uno::makeAny(float(awt::FontWeight::BOLD)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(awt::FontWeight::BOLD)), xPropSet->getPropertyValue(OUString("CharWeight")));
    //resetting the value to default
    xPropState->setPropertyToDefault(OUString("CharWeight"));
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharWeight")), aCharWeight);
    //testing CharUnderline from style::CharacterProperties
    uno::Any aCharUnderline = xPropSet->getPropertyValue(OUString("CharUnderline"));
    //changing the default value
    xPropSet->setPropertyValue(OUString("CharUnderline"), uno::makeAny(sal_Int16(awt::FontUnderline::SINGLE)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_Int16(awt::FontUnderline::SINGLE)), xPropSet->getPropertyValue(OUString("CharUnderline")));
    //resetting the value to default
    xPropState->setPropertyToDefault(OUString("CharUnderline"));
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue(OUString("CharUnderline")), aCharUnderline);
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

void SwUiWriterTest::testTdf88899()
{
    createDoc();
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Reference<beans::XPropertyContainer> xUserProps(xProps->getUserDefinedProperties(), uno::UNO_QUERY);
    com::sun::star::util::DateTime aDateTime = {sal_uInt32(1234567), sal_uInt16(3), sal_uInt16(3), sal_uInt16(3), sal_uInt16(10), sal_uInt16(11), sal_uInt16(2014), true};
    xUserProps->addProperty("dateTime", sal_Int16(beans::PropertyAttribute::OPTIONAL), uno::makeAny(aDateTime));
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xTextField(xFact->createInstance("com.sun.star.text.textfield.docinfo.Custom"), uno::UNO_QUERY);
    //Setting Name Property
    uno::Reference<beans::XPropertySet> xPropSet(xTextField, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(OUString("Name"), uno::makeAny(OUString("dateTime")));
    //Setting NumberFormat
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XNumberFormatTypes> xNumFormat(xNumberFormatsSupplier->getNumberFormats(), uno::UNO_QUERY);
    com::sun::star::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    sal_Int16 key = xNumFormat->getStandardFormat(util::NumberFormat::DATETIME, alocale);
    xPropSet->setPropertyValue(OUString("NumberFormat"), uno::makeAny(sal_Int16(key)));
    //Inserting Text Content
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextRange->getText(), uno::UNO_QUERY);
    xText->insertTextContent(xTextRange, xTextField, true);
    //Retrieving the contents for verification
    CPPUNIT_ASSERT_EQUAL(OUString("11/10/14 03:03 AM"), xTextField->getPresentation(false));
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
    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::LINK))}
    });
    lcl_dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    // Make sure that the document starts with a field now, and its expanded string value contains asdf.
    const uno::Reference< text::XTextRange > xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    CPPUNIT_ASSERT(xField->getString().endsWith("asdf"));
}

void SwUiWriterTest::testTdf89954()
{
    SwDoc* pDoc = createDoc("tdf89954.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 't', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'e', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 's', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 't', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '.', 0);

    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    // Placeholder character for the comment anchor was ^A (CH_TXTATR_BREAKWORD), not <fff9> (CH_TXTATR_INWORD).
    // As a result, autocorrect did not turn the 't' input into 'T'.
    OUString aExpected("Tes\xef\xbf\xb9t. Test.", 14, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aNodeIndex.GetNode().GetTextNode()->GetText());
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

void SwUiWriterTest::testTdf88986()
{
    // Create a text shell.
    SwDoc* pDoc = createDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwTextShell aShell(*pView);

    // Create the item set that is normally passed to the insert frame dialog.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwFlyFrmAttrMgr aMgr(true, pWrtShell, FRMMGR_TYPE_TEXT);
    SfxItemSet aSet = aShell.CreateInsertFrameItemSet(aMgr);

    // This was missing along with the gradient and other tables.
    CPPUNIT_ASSERT(aSet.HasItem(SID_COLOR_TABLE));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
