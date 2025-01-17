/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <vcl/errcode.hxx>
#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
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
#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagedesc.hxx>
#include <postithelper.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include "com/sun/star/text/XDefaultNumberingProvider.hpp"
#include "com/sun/star/awt/FontUnderline.hpp"

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <i18nutil/transliteration.hxx>
#include <i18nutil/searchopt.hxx>
#include <reffld.hxx>
#include <txatbase.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <unofldmid.h>
#include "UndoManager.hxx"
#include <textsh.hxx>
#include <frmmgr.hxx>
#include <tblafmt.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "com/sun/star/util/XNumberFormatTypes.hpp"
#include "com/sun/star/util/NumberFormat.hpp"
#include "com/sun/star/util/XNumberFormatsSupplier.hpp"
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include "com/sun/star/util/SearchAlgorithms.hpp"
#include "com/sun/star/sdbcx/XTablesSupplier.hpp"
#include "com/sun/star/text/XParagraphCursor.hpp"
#include "com/sun/star/util/XPropertyReplace.hpp"
#include "com/sun/star/awt/FontStrikeout.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/text/XTextField.hpp"
#include "com/sun/star/text/TextMarkupType.hpp"
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <o3tl/make_unique.hxx>
#include <osl/file.hxx>
#include <paratr.hxx>
#include <drawfont.hxx>
#include <txtfrm.hxx>
#include <txttypes.hxx>
#include <SwPortionHandler.hxx>
#include <hyp.hxx>
#include <swdtflvr.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/seqstream.hxx>
#include <sfx2/classificationhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/configurationhelper.hxx>
#include <editeng/unolingu.hxx>
#include <config_features.h>
#include <sfx2/watermarkitem.hxx>

static const char* const DATA_DIRECTORY = "/sw/qa/extras/uiwriter/data/";

class SwUiWriterTest : public SwModelTestBase
{

public:
    void testReplaceForward();
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    void testReplaceBackward();
    void testRedlineFrame();
    void testBookmarkCopy();
    void testFdo69893();
    void testFdo70807();
    void testImportRTF();
    void testExportRTF();
    void testDOCXAutoTextEmpty();
    void testDOCXAutoTextMultiple();
    void testDOTMAutoText();
    void testDOCXAutoTextGallery();
    void testWatermarkDOCX();
    void testTdf67238();
    void testFdo75110();
    void testFdo75898();
    void testFdo74981();
    void testTdf98512();
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
    void testTextCursorInvalidation();
    void testTdf68183();
    void testCp1000115();
    void testTdf63214();
    void testTdf90003();
    void testTdf51741();
    void testDefaultsOfOutlineNumbering();
    void testDeleteTableRedlines();
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
    void testTdf72788();
    void testTdf60967();
    void testSearchWithTransliterate();
    void testTdf73660();
    void testNewDocModifiedState();
    void testTdf77342();
    void testTdf63553();
    void testTdf74230();
    void testTdf74363();
    void testTdf80663();
    void testTdf57197();
    void testTdf90808();
    void testTdf97601();
    void testTdf75137();
    void testTdf83798();
    void testTdf89714();
    void testPropertyDefaults();
    void testTableBackgroundColor();
    void testTdf88899();
    void testTdf90362();
    void testUndoDelAsCharTdf107512();
    void testUndoCharAttribute();
    void testUndoDelAsChar();
    void testTdf86639();
    void testTdf90883TableBoxGetCoordinates();
    void testEmbeddedDataSource();
    void testUnoCursorPointer();
    void testUnicodeNotationToggle();
    void testTextTableCellNames();
    void testShapeAnchorUndo();
#if HAVE_FEATURE_UI
    void testDde();
#endif
    void testDocModState();
    void testTdf94804();
    void testTdf34957();
    void testTdf89954();
    void testTdf89720();
    void testTdf88986();
    void testTdf87922();
    void testTdf77014();
    void testTdf92648();
    void testTdf103978_backgroundTextShape();
    void testTdf96515();
    void testTdf96943();
    void testTdf96536();
    void testTdf96479();
    void testTdf96961();
    void testTdf88453();
    void testTdf88453Table();
    void testClassificationPaste();
    void testSmallCaps();
    void testTdf98987();
    void testTdf99004();
    void testTdf84695();
    void testTdf84695NormalChar();
    void testTdf84695Tab();
    void testTableStyleUndo();
    void testRedlineCopyPaste();
    void testRedlineParam();
    void testRedlineViewAuthor();
    void testTdf78727();
    void testRedlineTimestamp();
    void testCursorWindows();
    void testLandscape();
    void testTdf95699();
    void testTdf104032();
    void testTdf104440();
    void testTdf104425();
    void testTdf104814();
    void testTdf66405();
    void testTdf35021_tabOverMarginDemo();
    void testTdf106701_tabOverMarginAutotab();
    void testTdf104492();
    void testTdf107025();
    void testTdf107362();
    void testTdf105417();
    void testTdf105625();
    void testTdf106736();

    void testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc);
    void testTdf113877_blank_bold_on();
    void testTdf113877_blank_bold_off();

    void testMsWordCompTrailingBlanks();
    void testCreateDocxAnnotation();
    void testTdf107976();
    void testTdf113790();
    void testTdf114536();
    void testTdf113877();
    void testTdf113877NoMerge();
    void testTdf113877_default_style();
    void testTdf113877_Standard_style();
    void testPlaceholderHTMLPaste();
    void testPlaceholderHTMLInsert();
    void testPlaceholderHTMLPasteStyleOverride();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST(testRedlineFrame);
    CPPUNIT_TEST(testBookmarkCopy);
    CPPUNIT_TEST(testFdo69893);
    CPPUNIT_TEST(testFdo70807);
    CPPUNIT_TEST(testImportRTF);
    CPPUNIT_TEST(testExportRTF);
    CPPUNIT_TEST(testDOCXAutoTextEmpty);
    CPPUNIT_TEST(testDOCXAutoTextMultiple);
    CPPUNIT_TEST(testDOTMAutoText);
    CPPUNIT_TEST(testDOCXAutoTextGallery);
    CPPUNIT_TEST(testWatermarkDOCX);
    CPPUNIT_TEST(testTdf67238);
    CPPUNIT_TEST(testFdo75110);
    CPPUNIT_TEST(testFdo75898);
    CPPUNIT_TEST(testFdo74981);
    CPPUNIT_TEST(testTdf98512);
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
    CPPUNIT_TEST(testTextCursorInvalidation);
    CPPUNIT_TEST(testTdf68183);
    CPPUNIT_TEST(testCp1000115);
    CPPUNIT_TEST(testTdf63214);
    CPPUNIT_TEST(testTdf90003);
    CPPUNIT_TEST(testTdf51741);
    CPPUNIT_TEST(testDefaultsOfOutlineNumbering);
    CPPUNIT_TEST(testDeleteTableRedlines);
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
    CPPUNIT_TEST(testTdf72788);
    CPPUNIT_TEST(testTdf60967);
    CPPUNIT_TEST(testSearchWithTransliterate);
    CPPUNIT_TEST(testTdf73660);
    CPPUNIT_TEST(testNewDocModifiedState);
    CPPUNIT_TEST(testTdf77342);
    CPPUNIT_TEST(testTdf63553);
    CPPUNIT_TEST(testTdf74230);
    CPPUNIT_TEST(testTdf74363);
    CPPUNIT_TEST(testTdf80663);
    CPPUNIT_TEST(testTdf57197);
    CPPUNIT_TEST(testTdf90808);
    CPPUNIT_TEST(testTdf97601);
    CPPUNIT_TEST(testTdf75137);
    CPPUNIT_TEST(testTdf83798);
    CPPUNIT_TEST(testTdf89714);
    CPPUNIT_TEST(testPropertyDefaults);
    CPPUNIT_TEST(testTableBackgroundColor);
    CPPUNIT_TEST(testTdf88899);
    CPPUNIT_TEST(testTdf90362);
    CPPUNIT_TEST(testUndoDelAsCharTdf107512);
    CPPUNIT_TEST(testUndoCharAttribute);
    CPPUNIT_TEST(testUndoDelAsChar);
    CPPUNIT_TEST(testTdf86639);
    CPPUNIT_TEST(testTdf90883TableBoxGetCoordinates);
    CPPUNIT_TEST(testEmbeddedDataSource);
    CPPUNIT_TEST(testUnoCursorPointer);
    CPPUNIT_TEST(testUnicodeNotationToggle);
    CPPUNIT_TEST(testTextTableCellNames);
    CPPUNIT_TEST(testShapeAnchorUndo);
#if HAVE_FEATURE_UI
    CPPUNIT_TEST(testDde);
#endif
    CPPUNIT_TEST(testDocModState);
    CPPUNIT_TEST(testTdf94804);
    CPPUNIT_TEST(testTdf34957);
    CPPUNIT_TEST(testTdf89954);
    CPPUNIT_TEST(testTdf89720);
    CPPUNIT_TEST(testTdf88986);
    CPPUNIT_TEST(testTdf87922);
    CPPUNIT_TEST(testTdf77014);
    CPPUNIT_TEST(testTdf92648);
    CPPUNIT_TEST(testTdf103978_backgroundTextShape);
    CPPUNIT_TEST(testTdf96515);
    CPPUNIT_TEST(testTdf96943);
    CPPUNIT_TEST(testTdf96536);
    CPPUNIT_TEST(testTdf96479);
    CPPUNIT_TEST(testTdf96961);
    CPPUNIT_TEST(testTdf88453);
    CPPUNIT_TEST(testTdf88453Table);
    CPPUNIT_TEST(testClassificationPaste);
    CPPUNIT_TEST(testSmallCaps);
    CPPUNIT_TEST(testTdf98987);
    CPPUNIT_TEST(testTdf99004);
    CPPUNIT_TEST(testTdf84695);
    CPPUNIT_TEST(testTdf84695NormalChar);
    CPPUNIT_TEST(testTdf84695Tab);
    CPPUNIT_TEST(testTableStyleUndo);
    CPPUNIT_TEST(testRedlineCopyPaste);
    CPPUNIT_TEST(testRedlineParam);
    CPPUNIT_TEST(testRedlineViewAuthor);
    CPPUNIT_TEST(testTdf78727);
    CPPUNIT_TEST(testRedlineTimestamp);
    CPPUNIT_TEST(testCursorWindows);
    CPPUNIT_TEST(testLandscape);
    CPPUNIT_TEST(testTdf95699);
    CPPUNIT_TEST(testTdf104032);
    CPPUNIT_TEST(testTdf104440);
    CPPUNIT_TEST(testTdf104425);
    CPPUNIT_TEST(testTdf104814);
    CPPUNIT_TEST(testTdf66405);
    CPPUNIT_TEST(testTdf35021_tabOverMarginDemo);
    CPPUNIT_TEST(testTdf106701_tabOverMarginAutotab);
    CPPUNIT_TEST(testTdf104492);
    CPPUNIT_TEST(testTdf107025);
    CPPUNIT_TEST(testTdf107362);
    CPPUNIT_TEST(testTdf105417);
    CPPUNIT_TEST(testTdf105625);
    CPPUNIT_TEST(testTdf106736);
    CPPUNIT_TEST(testTdf113877_blank_bold_on);
    CPPUNIT_TEST(testTdf113877_blank_bold_off);
    CPPUNIT_TEST(testMsWordCompTrailingBlanks);
    CPPUNIT_TEST(testCreateDocxAnnotation);
    CPPUNIT_TEST(testTdf107976);
    CPPUNIT_TEST(testTdf113790);
    CPPUNIT_TEST(testTdf114536);
    CPPUNIT_TEST(testTdf113877);
    CPPUNIT_TEST(testTdf113877NoMerge);
    CPPUNIT_TEST(testTdf113877_default_style);
    CPPUNIT_TEST(testTdf113877_Standard_style);
    CPPUNIT_TEST(testPlaceholderHTMLPaste);
    CPPUNIT_TEST(testPlaceholderHTMLInsert);
    CPPUNIT_TEST(testPlaceholderHTMLPasteStyleOverride);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
    std::unique_ptr<SwTextBlocks> readDOCXAutotext(const OUString& sFileName, bool bEmpty = false);
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

std::unique_ptr<SwTextBlocks> SwUiWriterTest::readDOCXAutotext(const OUString& sFileName, bool bEmpty)
{
    OUString rURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + sFileName;

    SfxMedium aSrcMed(rURL, StreamMode::STD_READ);
    SwDoc* pDoc = createDoc();

    SwReader aReader(aSrcMed, rURL, pDoc);
    Reader* pDOCXReader = SwReaderWriter::GetDOCXReader();
    auto pGlossary = o3tl::make_unique<SwTextBlocks>(rURL);

    CPPUNIT_ASSERT(pDOCXReader != nullptr);
    CPPUNIT_ASSERT_EQUAL(!bEmpty, aReader.ReadGlossaries(*pDOCXReader, *pGlossary, false));

    return pGlossary;
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
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTextNode->GetText());
}

void SwUiWriterTest::testRedlineFrame()
{
    SwDoc * pDoc(createDoc("redlineFrame.fodt"));
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // there is exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    RedlineFlags nMode = pWrtShell->GetRedlineFlags();
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    // hide delete redlines
    pWrtShell->SetRedlineFlags(nMode & ~RedlineFlags::ShowDelete);

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    pWrtShell->SetRedlineFlags(nMode); // show again

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());
}

void SwUiWriterTest::testBookmarkCopy()
{
    SwDoc * pDoc(createDoc());

    // add text and bookmark
    IDocumentMarkAccess & rIDMA(*pDoc->getIDocumentMarkAccess());
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwCursor aPaM(SwPosition(aIdx), nullptr);
    rIDCO.InsertString(aPaM, "foo");
    rIDCO.SplitNode(*aPaM.GetPoint(), false);
    rIDCO.InsertString(aPaM, "bar");
    aPaM.SetMark();
    aPaM.MovePara(GoCurrPara, fnParaStart);
    rIDMA.makeMark(aPaM, "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
            ::sw::mark::InsertMode::New);
    aPaM.Exchange();
    aPaM.DeleteMark();
    rIDCO.SplitNode(*aPaM.GetPoint(), false);
    rIDCO.InsertString(aPaM, "baz");

    // copy range
    rIDCO.SplitNode(*aPaM.GetPoint(), false);
    SwPosition target(*aPaM.GetPoint());
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.SetMark();
    aPaM.SttEndDoc(true/*start*/);
    aPaM.Move(fnMoveForward, GoInContent); // partially select 1st para

    rIDCO.CopyRange(aPaM, target, /*bCopyAll=*/false, /*bCheckPos=*/true);

    // check bookmark was copied to correct position
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rIDMA.getBookmarksCount());
    for (auto it(rIDMA.getBookmarksBegin()); it != rIDMA.getBookmarksEnd(); ++it)
    {
        OUString markText(SwPaM((*it)->GetMarkPos(), (*it)->GetOtherMarkPos()).GetText());
        CPPUNIT_ASSERT_EQUAL(OUString("bar"), markText);
    }

    // copy 2nd time, such that bCanMoveBack is false in CopyImpl
    SwPaM aCopyPaM(*aPaM.GetMark(), *aPaM.GetPoint());
    aPaM.SttEndDoc(true/*start*/);
    rIDCO.SplitNode(*aPaM.GetPoint(), false);
    aPaM.SttEndDoc(true/*start*/);

    rIDCO.CopyRange(aCopyPaM, *aPaM.GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true);

    // check bookmark was copied to correct position
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getBookmarksCount());
    for (auto it(rIDMA.getBookmarksBegin()); it != rIDMA.getBookmarksEnd(); ++it)
    {
        OUString markText(SwPaM((*it)->GetMarkPos(), (*it)->GetOtherMarkPos()).GetText());
        CPPUNIT_ASSERT_EQUAL(OUString("bar"), markText);
    }
}

void SwUiWriterTest::testTdf67238()
{
    //create a new writer document
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //insert a 3X3 table in the newly created document
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    const SwTable& rTable = pWrtShell->InsertTable(TableOpt, 3, 3);
    //checking for the rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    //selecting the table
    pWrtShell->SttDoc();
    pWrtShell->SelTable();
    //making the table protected
    pWrtShell->ProtectCells();
    //checking each cell's protection, it should be protected
    CPPUNIT_ASSERT(((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    //undo the changes, make cells [un]protected
    rUndoManager.Undo();
    //checking each cell's protection, it should be [un]protected
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    //redo the changes, make cells protected
    rUndoManager.Redo();
    //checking each cell's protection, it should be protected
    CPPUNIT_ASSERT(((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    //moving the cursor to the starting of the document
    pWrtShell->SttDoc();
    //making the table [un]protected
    pWrtShell->SelTable();
    pWrtShell->UnProtectCells();
    //checking each cell's protection, it should be [un]protected
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    //undo the changes, make cells protected
    rUndoManager.Undo();
    //checking each cell's protection, it should be protected
    CPPUNIT_ASSERT(((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    //redo the changes, make cells [un]protected
    rUndoManager.Redo();
    //checking each cell's protection, it should be [un]protected
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("A3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("B3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C1"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C2"))->GetFrameFormat()->GetProtect()).IsContentProtected());
    CPPUNIT_ASSERT(!((rTable.GetTableBox("C3"))->GetFrameFormat()->GetProtect()).IsContentProtected());
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
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    SwTableNode* pTableNode = pShellCursor->Start()->nNode.GetNode().FindTableNode();
    // This was 1, when doing the same using the UI, Writer even crashed.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pTableNode->GetTable().GetTabLines().size());
}

void SwUiWriterTest::testReplaceBackward()
{
    SwDoc* pDoc = createDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "toto titi tutu");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

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

    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->nNode.GetNode());
    // Selection did not include the para after table, this was "B1".
    CPPUNIT_ASSERT_EQUAL(OUString("Para after table."), rEnd.GetText());
}

void SwUiWriterTest::testFdo70807()
{
    load(DATA_DIRECTORY, "fdo70807.odt");

    uno::Reference<container::XIndexAccess> xStylesIter(getStyles("PageStyles"), uno::UNO_QUERY);

    for (sal_Int32 i = 0; i < xStylesIter->getCount(); ++i)
    {
        uno::Reference<style::XStyle> xStyle(xStylesIter->getByIndex(i), uno::UNO_QUERY);
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
    SwReader aReader(aStream, OUString(), OUString(), *pWrtShell->GetCursor());
    Reader* pRTFReader = SwReaderWriter::GetRtfReader();
    CPPUNIT_ASSERT(pRTFReader != nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_uLong(0), aReader.Read(*pRTFReader));

    sal_uLong nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
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
    std::shared_ptr<SwDoc> pClpDoc(new SwDoc());
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
    // Ensure there's no extra newline
    CPPUNIT_ASSERT(aData.endsWith("bbb}" SAL_NEWLINE_STRING "}"));
}

void SwUiWriterTest::testDOCXAutoTextEmpty()
{
    // file contains normal content but no AutoText
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext("autotext-empty.dotx", true);
    CPPUNIT_ASSERT(pGlossary != nullptr);
}

void SwUiWriterTest::testDOCXAutoTextMultiple()
{
    // file contains three AutoText entries
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext("autotext-multiple.dotx");

    // check entries count
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)3, pGlossary->GetCount());

    // check names of entries, sorted order
    CPPUNIT_ASSERT_EQUAL(OUString("Anothercomplex"), pGlossary->GetLongName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Multiple"), pGlossary->GetLongName(1));
    CPPUNIT_ASSERT_EQUAL(OUString("Second  Autotext"), pGlossary->GetLongName(2));

    // check if previously loaded content is correct (eg. doesn't contain title)
    SwDoc* pDoc = pGlossary->GetDoc();
    CPPUNIT_ASSERT(pDoc != nullptr);

    SwNodeIndex aDocEnd(pDoc->GetNodes().GetEndOfContent());
    SwNodeIndex aStart(*aDocEnd.GetNode().StartOfSectionNode(), 1);

    CPPUNIT_ASSERT(aStart < aDocEnd);

    // first line
    SwNode& rNode = aStart.GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Another "), rNode.GetTextNode()->GetText());

    // last line
    SwNodeIndex aLast(*aDocEnd.GetNode().EndOfSectionNode(), -1);
    SwNode& rLastNode = aLast.GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("complex"), rLastNode.GetTextNode()->GetText());
}

void SwUiWriterTest::testDOTMAutoText()
{
    // this is dotm file difference is that in the dotm
    // there are no empty paragraphs at the end of each entry
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext("autotext-dotm.dotm");

    SwDoc* pDoc = pGlossary->GetDoc();
    CPPUNIT_ASSERT(pDoc != nullptr);

    // check if content is correct
    SwNodeIndex aDocEnd(pDoc->GetNodes().GetEndOfContent());
    SwNodeIndex aStart(*aDocEnd.GetNode().StartOfSectionNode(), 1);
    SwNode& rNode = aStart.GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("paragraph"), rNode.GetTextNode()->GetText());
}

void SwUiWriterTest::testDOCXAutoTextGallery()
{
    // this file contains one AutoText entry and other
    // entries which are not AutoText (have different "gallery" value)
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext("autotext-gallery.dotx");

    SwDoc* pDoc = pGlossary->GetDoc();
    CPPUNIT_ASSERT(pDoc != nullptr);

    // check entries count
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)1, pGlossary->GetCount());

    // check entry name (if not contains gallery type)
    CPPUNIT_ASSERT_EQUAL(OUString("Multiple"), pGlossary->GetLongName(0));
}

void SwUiWriterTest::testWatermarkDOCX()
{
    SwDoc* const pDoc = createDoc("watermark.docx");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    const SfxPoolItem* pItem;
    SfxItemState eState = pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher()->QueryState(SID_WATERMARK, pItem);

    CPPUNIT_ASSERT(eState >= SfxItemState::DEFAULT);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL((unsigned short)SID_WATERMARK, pItem->Which());

    const SfxWatermarkItem* pWatermark = static_cast<const SfxWatermarkItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomWatermark"), pWatermark->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("DejaVu Sans Light"), pWatermark->GetFont());
    CPPUNIT_ASSERT_EQUAL((sal_Int16)45, pWatermark->GetAngle());
    CPPUNIT_ASSERT_EQUAL((sal_uInt32)0x548dd4, pWatermark->GetColor());
    CPPUNIT_ASSERT_EQUAL((sal_Int16)50, pWatermark->GetTransparency());
}

void SwUiWriterTest::testFdo74981()
{
    // create a document with an input field
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInputField aField(static_cast<SwInputFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::Input)), "foo", "bar", 0, 0);
    pWrtShell->Insert(aField);

    {
        // expect hints
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
        CPPUNIT_ASSERT(pTextNode->HasHints());
    }

    // go to the begin of the paragraph and split this node
    pWrtShell->Left(CRSR_SKIP_CHARS, false, 100, false);
    pWrtShell->SplitNode();

    {
        // expect only the second paragraph to have hints
        SwNodeIndex aIdx(SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1));
        SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
        CPPUNIT_ASSERT(pTextNode->HasHints());
        --aIdx;
        pTextNode = aIdx.GetNode().GetTextNode();
        CPPUNIT_ASSERT(!pTextNode->HasHints());
    }
}

void SwUiWriterTest::testTdf98512()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInputFieldType *const pType(static_cast<SwInputFieldType*>(
                pWrtShell->GetFieldType(0, SwFieldIds::Input)));
    SwInputField aField1(pType, "foo", "bar", INP_TXT, 0);
    pWrtShell->Insert(aField1);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwInputField aField2(pType, "baz", "quux", INP_TXT, 0);
    pWrtShell->Insert(aField2);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SetMark();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    OUString const expected1(
        OUStringLiteral1(CH_TXT_ATR_INPUTFIELDSTART) + "foo" + OUStringLiteral1(CH_TXT_ATR_INPUTFIELDEND));
    OUString const expected2(
        OUStringLiteral1(CH_TXT_ATR_INPUTFIELDSTART) + "baz" + OUStringLiteral1(CH_TXT_ATR_INPUTFIELDEND)
        + expected1);
    CPPUNIT_ASSERT_EQUAL(expected2, pWrtShell->getShellCursor(false)->GetText());
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SetMark();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    CPPUNIT_ASSERT_EQUAL(expected1, pWrtShell->getShellCursor(false)->GetText());
    rUndoManager.Redo();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SetMark();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    CPPUNIT_ASSERT_EQUAL(expected2, pWrtShell->getShellCursor(false)->GetText());
    rUndoManager.Undo();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SetMark();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    CPPUNIT_ASSERT_EQUAL(expected1, pWrtShell->getShellCursor(false)->GetText());
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
    CPPUNIT_ASSERT_EQUAL( SwRedlineTable::size_type( 2 ), rTable.size());
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
    CPPUNIT_ASSERT_EQUAL( SwRedlineTable::size_type( 2 ), rTable.size());
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
    // Make sure we have a single draw shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), SwTextBoxHelper::getCount(pPage));

    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrObject* pObject = pPage->GetObj(0);
    // Select it, then copy and paste.
    pWrtShell->SelectObj(Point(), 0, pObject);
    pWrtShell->Copy(&aClipboard);
    pWrtShell->Paste(&aClipboard);

    // This was one: the textbox of the shape wasn't copied.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), SwTextBoxHelper::getCount(pDoc));
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
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This was 9, only "word", not "word<anchor character>" was selected.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pShellCursor->End()->nContent.GetIndex());

    // Test that getAnchor() points to "word", not to an empty string.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("word"), xField->getAnchor()->getString());
}


// Chinese conversion tests

static const sal_Unicode CHINESE_TRADITIONAL_CONTENT(0x9F8D);
static const sal_Unicode CHINESE_SIMPLIFIED_CONTENT(0x9F99);
static const OUStringLiteral NON_CHINESE_CONTENT("Hippopotamus");

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
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, nullptr,
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
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, nullptr,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();

    // Then
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(NON_CHINESE_CONTENT), pTextNode->GetText());

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
    pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(CHINESE_TRADITIONAL_CONTENT));

    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_CHINESE_SIMPLIFIED, nullptr,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();

    // Then
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(CHINESE_SIMPLIFIED_CONTENT), pTextNode->GetText());

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
    pDoc->getIDocumentContentOperations().InsertString(aPaM, OUString(CHINESE_SIMPLIFIED_CONTENT));

    // When
    SwHHCWrapper aWrap( pView, xContext, LANGUAGE_CHINESE_SIMPLIFIED, LANGUAGE_CHINESE_TRADITIONAL, nullptr,
                        i18n::TextConversionOption::CHARACTER_BY_CHARACTER, false,
                        true, false, false );
    aWrap.Convert();

    // Then
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(CHINESE_TRADITIONAL_CONTENT), pTextNode->GetText());

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
    sal_uLong nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("Test "), static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    //AutoCorrect with change style to bolt
    pWrtShell->Insert("Bolt");
    pWrtShell->AutoCorrect(corr, cIns);
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
            m_directories.getURLFromSrc(DATA_DIRECTORY) + "merge-change2.odt",
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

    xDoc2Component->dispose();
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

    pMarkAccess->makeMark(aPaM, "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
            ::sw::mark::InsertMode::New);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark("Mark");
    CPPUNIT_ASSERT(ppBkmk != pMarkAccess->getAllMarksEnd());

    pMarkAccess->renameMark(ppBkmk->get(), "Mark_");
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd()));
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") != pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark_") == pMarkAccess->getAllMarksEnd()));
    rUndoManager.Redo();
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd()));
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
    sal_Int32 nFirstEnd = getXPath(pXmlDoc, "(//polyline)[1]/point[last()-1]", "x").toInt32();
    // The second polyline has a different start point, but the arc it draws
    // should end at the ~same position as the first polyline.
    sal_Int32 nSecondEnd = getXPath(pXmlDoc, "(//polyline)[2]/point[last()]", "x").toInt32();

    // nFirstEnd was 6023 and nSecondEnd was 6648, now they should be much closer, e.g. nFirstEnd = 6550, nSecondEnd = 6548
    OString aMsg = "nFirstEnd is " + OString::number(nFirstEnd) + ", nSecondEnd is " + OString::number(nSecondEnd);
    // Assert that the difference is less than half point.
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), abs(nFirstEnd - nSecondEnd) < 10);
}

void SwUiWriterTest::testTextCursorInvalidation()
{
    createDoc();
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageStyle.is());
    xPageStyle->setPropertyValue("HeaderIsOn", uno::makeAny(true));
    uno::Reference<text::XText> xHeader(getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT(xHeader.is());
    // create cursor inside the header text
    uno::Reference<text::XTextCursor> xCursor(xHeader->createTextCursor());
    // can't go right in empty header
    CPPUNIT_ASSERT(!xCursor->goRight(1, false));
// this does not actually delete the header:    xPageStyle->setPropertyValue("HeaderIsOn", uno::makeAny(false));
    pWrtShell->ChangeHeaderOrFooter("Default Style", true, false, false);
    // must be disposed after deleting header
    CPPUNIT_ASSERT_THROW(xCursor->goRight(1, false), uno::RuntimeException);
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
        aPaM.Move(fnMoveForward, GoInContent);
        //Inserting a crossRefBookmark
        pMarkAccess->makeMark(aPaM, "Bookmark",
            IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK,
            ::sw::mark::InsertMode::New);
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
    pMarkAccess->makeMark(aPaM, "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
            ::sw::mark::InsertMode::New);
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
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd()));
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark_") != pMarkAccess->getAllMarksEnd());
    //Modification 5
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT(pMarkAccess->findMark("Mark") != pMarkAccess->getAllMarksEnd());
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark_") == pMarkAccess->getAllMarksEnd()));
    //Modification 6
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pWrtShell->IsModified());
    pWrtShell->ResetModified();
    CPPUNIT_ASSERT(bool(pMarkAccess->findMark("Mark") == pMarkAccess->getAllMarksEnd()));
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
    css::lang::Locale alocale;
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

void SwUiWriterTest::testDeleteTableRedlines()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    const SwTable& rTable = pWrtShell->InsertTable(TableOpt, 1, 3);
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    uno::Sequence<beans::PropertyValue> aDescriptor;
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("A1"))), "TableCellInsert", aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("B1"))), "TableCellInsert", aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("C1"))), "TableCellInsert", aDescriptor);
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwExtraRedlineTable& rExtras = rIDRA.GetExtraRedlineTable();
    rExtras.DeleteAllTableRedlines(pDoc, rTable, false, sal_uInt16(USHRT_MAX));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), rExtras.GetSize());
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
        beans::PropertyValue("CharWeight", sal_Int32(0), uno::makeAny(float(css::awt::FontWeight::BOLD)), beans::PropertyState_DIRECT_VALUE)
    };
    xFlatPara3->changeAttributes(sal_Int32(0), sal_Int32(5), aDescriptor);
    //checking Language Portions
    uno::Sequence<::sal_Int32> aLangPortions(xFlatPara4->getLanguagePortions());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aLangPortions.getLength());
    //examining Language of text
    css::lang::Locale alocale = xFlatPara4->getLanguageOfText(sal_Int32(0), sal_Int32(4));
    CPPUNIT_ASSERT_EQUAL(OUString("en"), alocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), alocale.Country);
    //examining Primary Language of text
    css::lang::Locale aprimarylocale = xFlatPara4->getPrimaryLanguageOfText(sal_Int32(0), sal_Int32(20));
    CPPUNIT_ASSERT_EQUAL(OUString("en"), aprimarylocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), aprimarylocale.Country);
}

void SwUiWriterTest::testTdf81995()
{
    uno::Reference<text::XDefaultNumberingProvider> xDefNum(m_xSFactory->createInstance("com.sun.star.text.DefaultNumberingProvider"), uno::UNO_QUERY);
    css::lang::Locale alocale;
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
    xPropSet->setPropertyValue("ParaBackColor", uno::makeAny(sal_Int32(0xFF00FF)));
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess(xSupplier->getStyleFamilies());
    uno::Reference<container::XNameContainer> xNameCont;
    xNameAccess->getByName("ParagraphStyles") >>= xNameCont;
    xNameCont->insertByName("myStyle", uno::makeAny(xStyle));
    CPPUNIT_ASSERT_EQUAL(OUString("myStyle"), xStyle->getName());
    //Setting the properties with proper values
    xPropSet->setPropertyValue("PageDescName", uno::makeAny(OUString("First Page")));
    xPropSet->setPropertyValue("PageNumberOffset", uno::makeAny(sal_Int16(3)));
    //Getting the properties and checking that they have proper values
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(OUString("First Page")), xPropSet->getPropertyValue("PageDescName"));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_Int16(3)), xPropSet->getPropertyValue("PageNumberOffset"));
}

void SwUiWriterTest::testTdf79236()
{
    SwDoc* pDoc = createDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //Getting some paragraph style
    SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName("Text Body");
    const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
    SfxItemSet* pNewSet = rAttrSet.Clone();
    sal_uInt16 initialCount = pNewSet->Count();
    SvxAdjustItem AdjustItem = rAttrSet.GetAdjust();
    SvxAdjust initialAdjust = AdjustItem.GetAdjust();
    //By default the adjust is LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, initialAdjust);
    //Changing the adjust to RIGHT
    AdjustItem.SetAdjust(SvxAdjust::Right);
    //Checking whether the change is made or not
    SvxAdjust modifiedAdjust = AdjustItem.GetAdjust();
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, modifiedAdjust);
    //Modifying the itemset, putting *one* item
    pNewSet->Put(AdjustItem);
    //The count should increment by 1
    sal_uInt16 modifiedCount = pNewSet->Count();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(initialCount + 1), modifiedCount);
    //Setting the updated item set on the style
    pDoc->ChgFormat(*pTextFormat, *pNewSet);
    //Checking the Changes
    SwTextFormatColl* pTextFormat2 = pDoc->FindTextFormatCollByName("Text Body");
    const SwAttrSet& rAttrSet2 = pTextFormat2->GetAttrSet();
    const SvxAdjustItem& rAdjustItem2 = rAttrSet2.GetAdjust();
    SvxAdjust Adjust2 = rAdjustItem2.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust2);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat3 = pDoc->FindTextFormatCollByName("Text Body");
    const SwAttrSet& rAttrSet3 = pTextFormat3->GetAttrSet();
    const SvxAdjustItem& rAdjustItem3 = rAttrSet3.GetAdjust();
    SvxAdjust Adjust3 = rAdjustItem3.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust3);
    //Redo the changes
    rUndoManager.Redo();
    SwTextFormatColl* pTextFormat4 = pDoc->FindTextFormatCollByName("Text Body");
    const SwAttrSet& rAttrSet4 = pTextFormat4->GetAttrSet();
    const SvxAdjustItem& rAdjustItem4 = rAttrSet4.GetAdjust();
    SvxAdjust Adjust4 = rAdjustItem4.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust4);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat5 = pDoc->FindTextFormatCollByName("Text Body");
    const SwAttrSet& rAttrSet5 = pTextFormat5->GetAttrSet();
    const SvxAdjustItem& rAdjustItem5 = rAttrSet5.GetAdjust();
    SvxAdjust Adjust5 = rAdjustItem5.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust5);
}

void SwUiWriterTest::testTextSearch()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCursor, "Hello World This is a test");
    // Use cursor to select part of text
    for (int i = 0; i < 10; i++) {
        pCursor->Move(fnMoveBackward);
    }
    pCursor->SetMark();
    for(int i = 0; i < 4; i++) {
        pCursor->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("This"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    //making another selection of text
    for (int i = 0; i < 7; i++) {
        pCursor->Move(fnMoveBackward);
    }
    pCursor->SetMark();
    for(int i = 0; i < 5; i++) {
        pCursor->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    //Performing Search Operation and also covering the UNO coverage for setProperty
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes(xSearch->createSearchDescriptor(), uno::UNO_QUERY);
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);
    //setting some properties
    uno::Sequence<beans::PropertyValue> aDescriptor =
    {
        beans::PropertyValue("CharWeight", sal_Int32(0), uno::makeAny(float(css::awt::FontWeight::BOLD)), beans::PropertyState_DIRECT_VALUE)
    };
    xProp->setSearchAttributes(aDescriptor);
    //receiving the defined properties and asserting them with expected values, covering UNO
    uno::Sequence<beans::PropertyValue> aPropVal2(xProp->getSearchAttributes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPropVal2.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("CharWeight"), aPropVal2[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(css::awt::FontWeight::BOLD)), aPropVal2[0].Value);
    //specifying the search attributes
    uno::Reference<beans::XPropertySet> xPropSet(xSearchDes, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("SearchWords", uno::makeAny(true));
    xPropSet->setPropertyValue("SearchCaseSensitive", uno::makeAny(true));
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
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(css::awt::FontWeight::BOLD)), aRepProp[0].Value);
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
    SwPageDesc* sPageDesc = source->MakePageDesc("SourceStyle");
    SwPageDesc* tPageDesc = target->MakePageDesc("TargetStyle");
    sPageDesc->ChgFirstShare(false);
    CPPUNIT_ASSERT(!sPageDesc->IsFirstShared());
    SwFrameFormat& rSourceMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    rSourceMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    rSourceMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc(OUString("SourceStyle"), *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& rTargetMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& rTargetLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& rTargetFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& rTargetFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem MasterLRSpace = rTargetMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), MasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), MasterLRSpace.GetRight());
    const SvxLRSpaceItem LeftLRSpace = rTargetLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), LeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), LeftLRSpace.GetRight());
    const SvxLRSpaceItem FirstMasterLRSpace = rTargetFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstMasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstMasterLRSpace.GetRight());
    const SvxLRSpaceItem FirstLeftLRSpace = rTargetFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstLeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstLeftLRSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem MasterULSpace = rTargetMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), MasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), MasterULSpace.GetLower());
    const SvxULSpaceItem LeftULSpace = rTargetLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), LeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), LeftULSpace.GetLower());
    const SvxULSpaceItem FirstMasterULSpace = rTargetFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstMasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstMasterULSpace.GetLower());
    const SvxULSpaceItem FirstLeftULSpace = rTargetFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstLeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstLeftULSpace.GetLower());
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
    SwPageDesc* sPageDesc = source->MakePageDesc("SourceStyle");
    SwPageDesc* tPageDesc = target->MakePageDesc("TargetStyle");
    //Enabling Mirror
    sPageDesc->SetUseOn(UseOnPage::Mirror);
    SwFrameFormat& rSourceMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    rSourceMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    rSourceMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc(OUString("SourceStyle"), *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& rTargetMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& rTargetLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& rTargetFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& rTargetFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem MasterLRSpace = rTargetMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), MasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), MasterLRSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem LeftLRSpace = rTargetLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), LeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), LeftLRSpace.GetRight());
    const SvxLRSpaceItem FirstMasterLRSpace = rTargetFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstMasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstMasterLRSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem FirstLeftLRSpace = rTargetFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstLeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstLeftLRSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem MasterULSpace = rTargetMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), MasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), MasterULSpace.GetLower());
    const SvxULSpaceItem LeftULSpace = rTargetLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), LeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), LeftULSpace.GetLower());
    const SvxULSpaceItem FirstMasterULSpace = rTargetFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstMasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstMasterULSpace.GetLower());
    const SvxULSpaceItem FirstLeftULSpace = rTargetFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstLeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstLeftULSpace.GetLower());
    xSourceDoc->dispose();
}

void SwUiWriterTest::testTdf78742()
{
    //testing with service type and any .ods file
    OUString path = m_directories.getURLFromSrc(DATA_DIRECTORY) + "calc-data-source.ods";
    SfxMedium aMedium(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher("com.sun.star.text.TextDocument");
    std::shared_ptr<const SfxFilter> pFilter;
    sal_uInt32 filter = aMatcher.DetectFilter(aMedium, pFilter);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_IO_ABORT, filter);
    //it should not return any Filter
    CPPUNIT_ASSERT(!pFilter);
    //testing without service type and any .ods file
    SfxMedium aMedium2(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher2;
    std::shared_ptr<const SfxFilter> pFilter2;
    sal_uInt32 filter2 = aMatcher2.DetectFilter(aMedium2, pFilter2);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_CLASS_NONE, filter2);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("calc8"), pFilter2->GetFilterName());
    //testing with service type and any .odt file
    OUString path2 = m_directories.getURLFromSrc(DATA_DIRECTORY) + "fdo69893.odt";
    SfxMedium aMedium3(path2, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher3("com.sun.star.text.TextDocument");
    std::shared_ptr<const SfxFilter> pFilter3;
    sal_uInt32 filter3 = aMatcher3.DetectFilter(aMedium3, pFilter3);
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

void SwUiWriterTest::testTdf72788()
{
    //Create a new empty Writer document
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    //Insert some text - two paragraphs
    rIDCO.InsertString(*pCursor, "this is text");
    //Position of word              9876543210
    //Position of word            0123456789
    //Change Paragraph
    pWrtShell->SplitNode();
    //Insert second paragraph
    rIDCO.InsertString(*pCursor, "more text");
    //Position of word            012345678
    //Make the following selection *bold*
    //this[is text
    //more] text
    //Move cursor back
    for (int i = 0; i < 5; i++) {
        pCursor->Move(fnMoveBackward);
    }
    //Start selection
    pCursor->SetMark();
    for (int i = 0; i < 12; i++) {
        pCursor->Move(fnMoveBackward);
    }
    //Check the text selection
    CPPUNIT_ASSERT_EQUAL(OUString("is textmore"), pCursor->GetText());
    //Apply a *Bold* attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT);
    //Add selected text's attributes to aSet
    pCursor->GetNode().GetTextNode()->GetAttr(aSet, 5, 12);
    SfxPoolItem const * pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    //Check that bold is active on the selection and it's in aSet
    CPPUNIT_ASSERT_EQUAL((*pPoolItem == aWeightItem), true);
    //Make selection to remove formatting in first paragraph
    //[this is text
    //]more text
    pWrtShell->SttDoc();
    //Start selection
    pCursor->SetMark();
    for (int i = 0; i < 13; i++) {
        pCursor->Move(fnMoveForward);
    }
    //Clear all the Direct Formatting ( Ctrl + M )
    SwTextNode* pTextNode = pCursor->GetNode().GetTextNode();
    SwIndex aSt( pTextNode, 0 );
    sal_Int32 nEnd = pTextNode->Len();
    pTextNode->RstTextAttr(aSt, nEnd - aSt.GetIndex());
    //Incase of Regression RstTextAttr() call will result to infinite recursion
    //Check that bold is removed in first paragraph
    pTextNode->GetAttr(aSet, 5, 12);
    SfxPoolItem const * pPoolItem2 = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL((*pPoolItem2 != aWeightItem), true);
}

void SwUiWriterTest::testTdf60967()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    pWrtShell->ChangeHeaderOrFooter("Default Style", true, true, true);
    //Inserting table
    SwInsertTableOptions TableOpt(tabopts::DEFAULT_BORDER, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //getting the cursor's position just after the table insert
    SwPosition aPosAfterTable(*(pCursor->GetPoint()));
    //moving cursor to B2 (bottom right cell)
    pCursor->Move(fnMoveBackward);
    SwPosition aPosInTable(*(pCursor->GetPoint()));
    //deleting paragraph following table with Ctrl+Shift+Del
    sal_Int32 val = pWrtShell->DelToEndOfSentence();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), val);
    //getting the cursor's position just after the paragraph deletion
    SwPosition aPosAfterDel(*(pCursor->GetPoint()));
    //moving cursor forward to check whether there is any node following the table, BTW there should not be any such node
    pCursor->Move(fnMoveForward);
    SwPosition aPosMoveAfterDel(*(pCursor->GetPoint()));
    //checking the positions to verify that the paragraph is actually deleted
    CPPUNIT_ASSERT_EQUAL(aPosAfterDel, aPosInTable);
    CPPUNIT_ASSERT_EQUAL(aPosMoveAfterDel, aPosInTable);
    //Undo the changes
    rUndoManager.Undo();
    {
        //paragraph *text node* should be back
        SwPosition aPosAfterUndo(*(pCursor->GetPoint()));
        //after undo aPosAfterTable increases the node position by one, since this contains the position *text node* so aPosAfterUndo should be less than aPosAfterTable
        CPPUNIT_ASSERT(aPosAfterTable > aPosAfterUndo);
        //moving cursor forward to check whether there is any node following the paragraph, BTW there should not be any such node as paragraph node is the last one in header
        pCursor->Move(fnMoveForward);
        SwPosition aPosMoveAfterUndo(*(pCursor->GetPoint()));
        //checking positions to verify that paragraph node is the last one and we are paragraph node only
        CPPUNIT_ASSERT(aPosAfterTable > aPosMoveAfterUndo);
        CPPUNIT_ASSERT_EQUAL(aPosAfterUndo, aPosMoveAfterUndo);
    }
    //Redo the changes
    rUndoManager.Redo();
    //paragraph *text node* should not be there
    SwPosition aPosAfterRedo(*(pCursor->GetPoint()));
    //position should be exactly same as it was after deletion of *text node*
    CPPUNIT_ASSERT_EQUAL(aPosAfterRedo, aPosMoveAfterDel);
    //moving the cursor forward, but it should not actually move as there is no *text node* after the table due to this same position is expected after move as it was before move
    pCursor->Move(fnMoveForward);
    SwPosition aPosAfterUndoMove(*(pCursor->GetPoint()));
    CPPUNIT_ASSERT_EQUAL(aPosAfterRedo, aPosAfterUndoMove);
}

void SwUiWriterTest::testSearchWithTransliterate()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    {
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        pDoc->getIDocumentContentOperations().InsertString(aPaM,"This is paragraph one");
        pWrtShell->SplitNode();
    }
    {
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        pDoc->getIDocumentContentOperations().InsertString(aPaM,"This is Other PARAGRAPH");
    }
    i18nutil::SearchOptions2 SearchOpt;
    SearchOpt.algorithmType = css::util::SearchAlgorithms_ABSOLUTE;
    SearchOpt.searchFlag = css::util::SearchFlags::ALL_IGNORE_CASE;
    SearchOpt.searchString = "other";
    SearchOpt.replaceString.clear();
    SearchOpt.changedChars = 0;
    SearchOpt.deletedChars = 0;
    SearchOpt.insertedChars = 0;
    SearchOpt.transliterateFlags = TransliterationFlags::IGNORE_DIACRITICS_CTL;
    SearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    SearchOpt.WildcardEscapeCharacter = 0;
    //transliteration option set so that at least one of the search strings is not found
    sal_uLong case1 = pWrtShell->SearchPattern(SearchOpt,true,SwDocPositions::Start,SwDocPositions::End);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(true);
    CPPUNIT_ASSERT_EQUAL(OUString(),pShellCursor->GetText());
    CPPUNIT_ASSERT_EQUAL(0,(int)case1);
    SearchOpt.searchString = "paragraph";
    SearchOpt.transliterateFlags = TransliterationFlags::IGNORE_KASHIDA_CTL;
    //transliteration option set so that all search strings are found
    sal_uLong case2 = pWrtShell->SearchPattern(SearchOpt,true,SwDocPositions::Start,SwDocPositions::End);
    pShellCursor = pWrtShell->getShellCursor(true);
    CPPUNIT_ASSERT_EQUAL(OUString("paragraph"),pShellCursor->GetText());
    CPPUNIT_ASSERT_EQUAL(1,(int)case2);
}

void SwUiWriterTest::testTdf73660()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aData1 = "First" + OUStringLiteral1(CHAR_SOFTHYPHEN) + "Word";
    OUString aData2 = "Seco" + OUStringLiteral1(CHAR_SOFTHYPHEN) + "nd";
    OUString aData3 = OUStringLiteral1(CHAR_SOFTHYPHEN) + "Third";
    OUString aData4 = "Fourth" + OUStringLiteral1(CHAR_SOFTHYPHEN);
    OUString aData5 = "Fifth";
    pWrtShell->Insert("We are inserting some text in the document to check the search feature ");
    pWrtShell->Insert(aData1 + " ");
    pWrtShell->Insert(aData2 + " ");
    pWrtShell->Insert(aData3 + " ");
    pWrtShell->Insert(aData4 + " ");
    pWrtShell->Insert(aData5 + " ");
    pWrtShell->Insert("Now we have enough text let's test search for all the cases");
    //searching for all 5 strings entered with soft-hyphen, search string contains no soft-hyphen
    i18nutil::SearchOptions2 searchOpt;
    searchOpt.algorithmType = css::util::SearchAlgorithms_REGEXP;
    searchOpt.searchFlag = css::util::SearchFlags::NORM_WORD_ONLY;
    //case 1
    searchOpt.searchString = "First";
    CPPUNIT_ASSERT_EQUAL(sal_uLong(1), pWrtShell->SearchPattern(searchOpt,true,SwDocPositions::Start,SwDocPositions::End));
    //case 2
    searchOpt.searchString = "Second";
    CPPUNIT_ASSERT_EQUAL(sal_uLong(1), pWrtShell->SearchPattern(searchOpt,true,SwDocPositions::Start,SwDocPositions::End));
    //case 3
    searchOpt.searchString = "Third";
    CPPUNIT_ASSERT_EQUAL(sal_uLong(1), pWrtShell->SearchPattern(searchOpt,true,SwDocPositions::Start,SwDocPositions::End));
    //case 4
    searchOpt.searchString = "Fourth";
    CPPUNIT_ASSERT_EQUAL(sal_uLong(1), pWrtShell->SearchPattern(searchOpt,true,SwDocPositions::Start,SwDocPositions::End));
    //case 5
    searchOpt.searchString = "Fifth";
    CPPUNIT_ASSERT_EQUAL(sal_uLong(1), pWrtShell->SearchPattern(searchOpt,true,SwDocPositions::Start,SwDocPositions::End));
}

void SwUiWriterTest::testNewDocModifiedState()
{
    //creating a new doc
    SwDoc* pDoc = new SwDoc();
    //getting the state of the document via IDocumentState
    IDocumentState& rState(pDoc->getIDocumentState());
    //the state should not be modified, no modifications yet
    CPPUNIT_ASSERT(!(rState.IsModified()));
}

void SwUiWriterTest::testTdf77342()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting first footnote
    pWrtShell->InsertFootnote("");
    SwFieldType* pField = pWrtShell->GetFieldType(0, SwFieldIds::GetRef);
    SwGetRefFieldType* pRefType = static_cast<SwGetRefFieldType*>(pField);
    //moving cursor to the starting of document
    pWrtShell->SttDoc();
    //inserting reference field 1
    SwGetRefField aField1(pRefType, "", REF_FOOTNOTE, sal_uInt16(0), REF_CONTENT);
    pWrtShell->Insert(aField1);
    //inserting second footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->SttDoc();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aField2(pRefType, "", REF_FOOTNOTE, sal_uInt16(1), REF_CONTENT);
    pWrtShell->Insert(aField2);
    //inserting third footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->SttDoc();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aField3(pRefType, "", REF_FOOTNOTE, sal_uInt16(2), REF_CONTENT);
    pWrtShell->Insert(aField3);
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    SwDoc* pClpDoc = new SwDoc();
    pClpDoc->acquire();
    pClpDoc->SetClipBoard(true);
    pClpDoc->getIDocumentFieldsAccess().LockExpFields();
    //selecting reference field 2 and reference field 3 and footnote 1 and footnote 2
    //selection is such that more than one and not all footnotes and ref fields are selected
    pCursor->Move(fnMoveBackward);
    pCursor->Move(fnMoveBackward);
    //start marking
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //copying the selection to clipboard
    pWrtShell->Copy(pClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking that the footnotes reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->SttDoc();
    SwField* pRef1 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef1->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef1->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pRef2 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef2->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef2->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pRef3 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef3->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef3->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //moving cursor to the end of the document
    pWrtShell->EndDoc();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(pClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->SttDoc();
    //old reference field 1
    SwField* pOldRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef13 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef13->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef13->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //old footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd1 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote1 = pTextNd1->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote1(pFootnote1->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnote1.GetNumber());
    SwTextFootnote* pTFNote1 = static_cast<SwTextFootnote*> (pFootnote1);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pTFNote1->GetSeqRefNo());
    //old footnote 2
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd2 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote2 = pTextNd2->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote2(pFootnote2->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnote2.GetNumber());
    SwTextFootnote* pTFNote2 = static_cast<SwTextFootnote*> (pFootnote2);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pTFNote2->GetSeqRefNo());
    //old footnote 3
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd3 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote3 = pTextNd3->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote3(pFootnote3->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), rFootnote3.GetNumber());
    SwTextFootnote* pTFNote3 = static_cast<SwTextFootnote*> (pFootnote3);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pTFNote3->GetSeqRefNo());
    //new reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pNewRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(3)), aAny);
    //new footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd4 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote4 = pTextNd4->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote4(pFootnote4->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), rFootnote4.GetNumber());
    SwTextFootnote* pTFNote4 = static_cast<SwTextFootnote*> (pFootnote4);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pTFNote4->GetSeqRefNo());
    //moving the cursor to the starting of document
    pWrtShell->SttDoc();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(pClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->SttDoc();
    //new reference field 1
    SwField* pNewRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(4)), aAny);
    //new footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd11 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote11 = pTextNd11->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote11(pFootnote11->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnote11.GetNumber());
    SwTextFootnote* pTFNote11 = static_cast<SwTextFootnote*> (pFootnote11);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pTFNote11->GetSeqRefNo());
    //old reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pOldRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef23 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef23->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef23->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //old footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd12 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote12 = pTextNd12->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote12(pFootnote12->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnote12.GetNumber());
    SwTextFootnote* pTFNote12 = static_cast<SwTextFootnote*> (pFootnote12);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pTFNote12->GetSeqRefNo());
    //old footnote 2
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd13 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote13 = pTextNd13->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote13(pFootnote13->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), rFootnote13.GetNumber());
    SwTextFootnote* pTFNote13 = static_cast<SwTextFootnote*> (pFootnote13);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pTFNote13->GetSeqRefNo());
    //old footnote 3
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd14 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote14 = pTextNd14->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote14(pFootnote14->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), rFootnote14.GetNumber());
    SwTextFootnote* pTFNote14 = static_cast<SwTextFootnote*> (pFootnote14);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pTFNote14->GetSeqRefNo());
    //old reference field 4
    pCursor->Move(fnMoveForward);
    SwField* pOldRef24 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef24->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef24->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //old reference field 5
    pCursor->Move(fnMoveForward);
    SwField* pOldRef25 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef25->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef25->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(3)), aAny);
    //old footnote 4
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd15 = pCursor->GetNode().GetTextNode();
    SwTextAttr* const pFootnote15 = pTextNd15->GetTextAttrForCharAt(pCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote15(pFootnote15->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), rFootnote15.GetNumber());
    SwTextFootnote* pTFNote15 = static_cast<SwTextFootnote*> (pFootnote15);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pTFNote15->GetSeqRefNo());
}

void SwUiWriterTest::testTdf63553()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting sequence field 1
    SwSetExpFieldType* pSeqType = static_cast<SwSetExpFieldType*>(pWrtShell->GetFieldType(SwFieldIds::SetExp, "Illustration"));
    SwSetExpField aSetField1(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField1);
    SwGetRefFieldType* pRefType = static_cast<SwGetRefFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::GetRef));
    //moving cursor to the starting of document
    pWrtShell->SttDoc();
    //inserting reference field 1
    SwGetRefField aGetField1(pRefType, "Illustration", REF_SEQUENCEFLD, sal_uInt16(0), REF_CONTENT);
    pWrtShell->Insert(aGetField1);
    //now we have ref1-seq1
    //moving the cursor
    pCursor->Move(fnMoveForward);
    //inserting sequence field 2
    SwSetExpField aSetField2(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField2);
    //moving the cursor
    pWrtShell->SttDoc();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aGetField2(pRefType, "Illustration", REF_SEQUENCEFLD, sal_uInt16(1), REF_CONTENT);
    pWrtShell->Insert(aGetField2);
    //now we have ref1-ref2-seq1-seq2
    //moving the cursor
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting sequence field 3
    SwSetExpField aSetField3(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField3);
    pWrtShell->SttDoc();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aGetField3(pRefType, "Illustration", REF_SEQUENCEFLD, sal_uInt16(2), REF_CONTENT);
    pWrtShell->Insert(aGetField3);
    //now after insertion we have ref1-ref2-ref3-seq1-seq2-seq3
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    SwDoc* pClpDoc = new SwDoc();
    pClpDoc->acquire();
    pClpDoc->SetClipBoard(true);
    pClpDoc->getIDocumentFieldsAccess().LockExpFields();
    //selecting reference field 2 and 3 and sequence field 1 and 2
    //selection is such that more than one and not all sequence fields and reference fields are selected
    //ref1-[ref2-ref3-seq1-seq2]-seq3
    pWrtShell->SttDoc();
    pCursor->Move(fnMoveForward);
    //start marking
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //copying the selection to clipboard
    pWrtShell->Copy(pClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking whether the sequence and reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->SttDoc();
    SwField* pRef1 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef1->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef1->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pRef2 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef2->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef2->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pRef3 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef3->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef3->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF1 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pSeqF1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF1->GetFieldName());
    //sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF2 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pSeqF2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF2->GetFieldName());
    //sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF3 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pSeqF3->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF3->GetFieldName());
    //moving cursor to the end of the document
    pWrtShell->EndDoc();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(pClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->SttDoc();
    //now we have ref1-ref2-ref3-seq1-seq2-seq3-nref1-nref2-nseq1-nseq2
    //old reference field 1
    SwField* pOldRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef13 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef13->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef13->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //old sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq1 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pSeq1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq1->GetFieldName());
    //old sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq2 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pSeq2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq2->GetFieldName());
    //old sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq3 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pSeq3->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq3->GetFieldName());
    //new reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pNewRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(4)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //new sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq1 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pNewSeq1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq1->GetFieldName());
    //new sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq2 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pNewSeq2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq2->GetFieldName());
    //moving the cursor to the starting of document
    pWrtShell->SttDoc();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(pClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->SttDoc();
    //now we have [nnref1-nnref2-nnseq1-nnseq2]-ref1-[ref2-ref3-seq1-seq2]-seq3-[nref1-nref2-nseq1-nseq2]
    //new reference field 1
    SwField* pNewRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(6)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //new sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq11 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pNewSeq11->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq11->GetFieldName());
    //new sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq12 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pNewSeq12->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq12->GetFieldName());
    //old reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pOldRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef23 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef23->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef23->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //old sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq11 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pOldSeq11->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq11->GetFieldName());
    //old sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq12 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pOldSeq12->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq12->GetFieldName());
    //old sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq13 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pOldSeq13->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq13->GetFieldName());
    //old reference field 4
    pCursor->Move(fnMoveForward);
    SwField* pOldRef24 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef24->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef24->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(4)), aAny);
    //old reference field 5
    pCursor->Move(fnMoveForward);
    SwField* pOldRef25 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef25->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef25->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_uInt16(2)), aAny);
    //old sequence field 4
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq14 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pOldSeq14->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq14->GetFieldName());
    //old sequence field 5
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq15 = static_cast<SwSetExpField*> (SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pOldSeq15->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq15->GetFieldName());
}

void SwUiWriterTest::testTdf74230()
{
    createDoc();
    //exporting the empty document to ODT via TempFile
    uno::Sequence<beans::PropertyValue> aDescriptor;
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());
    //loading an XML DOM of the "styles.xml" of the TempFile
    xmlDocPtr pXmlDoc = parseExportInternal(aTempFile.GetURL(),"styles.xml");
    //pXmlDoc should not be null
    CPPUNIT_ASSERT(pXmlDoc);
    //asserting XPath in loaded XML DOM
    assertXPath(pXmlDoc, "//office:styles/style:default-style[@style:family='graphic']/style:graphic-properties[@svg:stroke-color='#3465a4']");
    assertXPath(pXmlDoc, "//office:styles/style:default-style[@style:family='graphic']/style:graphic-properties[@draw:fill-color='#729fcf']");
    //deleting the TempFile
    aTempFile.EnableKillingFile();
}

void SwUiWriterTest::testTdf74363()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //testing autocorrect of initial capitals on start of first paragraph
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    //Inserting one all-lowercase word into the first paragraph
    pWrtShell->Insert("testing");
    const sal_Unicode cChar = ' ';
    pWrtShell->AutoCorrect(corr, cChar);
    //The word should be capitalized due to autocorrect
    sal_uLong nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("Testing "), static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
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
    //Inserting one row after the existing row
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
    uno::Reference<text::XParagraphCursor> xCursor(xText->createTextCursor(), uno::UNO_QUERY);
    //inserting text into document so that the paragraph is not empty
    xText->setString("Hello World!");
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    //creating bookmark 1
    uno::Reference<text::XTextContent> xHeadingBookmark1(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xHeadingName1(xHeadingBookmark1, uno::UNO_QUERY);
    xHeadingName1->setName("__RefHeading__1");
    //moving cursor to the starting of paragraph
    xCursor->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCursor, xHeadingBookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> xHeadingBookmark2(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xHeadingName2(xHeadingBookmark2, uno::UNO_QUERY);
    xHeadingName2->setName("__RefHeading__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCursor->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCursor, xHeadingBookmark2, true), css::lang::IllegalArgumentException);
    //now testing for __RefNumPara__
    //creating bookmark 1
    uno::Reference<text::XTextContent> xNumBookmark1(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNumName1(xNumBookmark1, uno::UNO_QUERY);
    xNumName1->setName("__RefNumPara__1");
    //moving cursor to the starting of paragraph
    xCursor->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCursor, xNumBookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> xNumBookmark2(xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNumName2(xNumBookmark2, uno::UNO_QUERY);
    xNumName2->setName("__RefNumPara__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCursor->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCursor, xNumBookmark2, true), css::lang::IllegalArgumentException);
}

void SwUiWriterTest::testTdf97601()
{
    // Instructions from the bugreport to trigger an infinite loop.
    createDoc("tdf97601.odt");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEmbeddedObjects = xEmbeddedObjectsSupplier->getEmbeddedObjects();
    uno::Reference<beans::XPropertySet> xChart;
    xEmbeddedObjects->getByName("myChart") >>= xChart;
    uno::Reference<chart2::data::XDataSource> xChartComponent;
    xChart->getPropertyValue("Component") >>= xChartComponent;
    uno::Sequence< uno::Reference<chart2::data::XLabeledDataSequence> > aDataSequences = xChartComponent->getDataSequences();
    uno::Reference<document::XEmbeddedObjectSupplier2> xChartState(xChart, uno::UNO_QUERY);
    xChartState->getExtendedControlOverEmbeddedObject()->changeState(1);
    uno::Reference<util::XModifiable> xDataSequenceModifiable(aDataSequences[2]->getValues(), uno::UNO_QUERY);
    xDataSequenceModifiable->setModified(true);

    // Make sure that the chart is marked as modified.
    uno::Reference<util::XModifiable> xModifiable(xChartComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xModifiable->isModified()));
    calcLayout();
    // This never returned.
    Scheduler::ProcessEventsToIdle();
}

void SwUiWriterTest::testTdf75137()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(true);
    pWrtShell->InsertFootnote("This is first footnote");
    sal_uLong firstIndex = pShellCursor->GetNode().GetIndex();
    pShellCursor->GotoFootnoteAnchor();
    pWrtShell->InsertFootnote("This is second footnote");
    pWrtShell->Up(false);
    sal_uLong secondIndex = pShellCursor->GetNode().GetIndex();
    pWrtShell->Down(false);
    sal_uLong thirdIndex = pShellCursor->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(firstIndex, thirdIndex);
    CPPUNIT_ASSERT(firstIndex != secondIndex);
}

void SwUiWriterTest::testTdf83798()
{
    SwDoc* pDoc = createDoc("tdf83798.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInNode);
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1.A"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2.A"), pCursor->GetText());
    pCursor->DeleteMark();
}

void SwUiWriterTest::testTdf89714()
{
    createDoc();
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xInterface(xFact->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertyState> xPropState(xInterface, uno::UNO_QUERY);
    //enabled Paragraph Orphan and Widows by default starting in LO5.1
    CPPUNIT_ASSERT_EQUAL( uno::makeAny(sal_Int8(2)), xPropState->getPropertyDefault("ParaOrphans") );
    CPPUNIT_ASSERT_EQUAL( uno::makeAny(sal_Int8(2)), xPropState->getPropertyDefault("ParaWidows")  );
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
    uno::Any aCharFontName = xPropState->getPropertyDefault("CharFontName");
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharFontName"), aCharFontName);
    //changing the default value
    xPropSet->setPropertyValue("CharFontName", uno::makeAny(OUString("Symbol")));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(OUString("Symbol")), xPropSet->getPropertyValue("CharFontName"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharFontName");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharFontName"), aCharFontName);
    //testing CharHeight from style::CharacterProperties
    //getting property default
    uno::Any aCharHeight = xPropState->getPropertyDefault("CharHeight");
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharHeight"), aCharHeight);
    //changing the default value
    xPropSet->setPropertyValue("CharHeight", uno::makeAny(float(14)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(14)), xPropSet->getPropertyValue("CharHeight"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharHeight");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharHeight"), aCharHeight);
    //testing CharWeight from style::CharacterProperties
    uno::Any aCharWeight = xPropSet->getPropertyValue("CharWeight");
    //changing the default value
    xPropSet->setPropertyValue("CharWeight", uno::makeAny(float(awt::FontWeight::BOLD)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(float(awt::FontWeight::BOLD)), xPropSet->getPropertyValue("CharWeight"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharWeight");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharWeight"), aCharWeight);
    //testing CharUnderline from style::CharacterProperties
    uno::Any aCharUnderline = xPropSet->getPropertyValue("CharUnderline");
    //changing the default value
    xPropSet->setPropertyValue("CharUnderline", uno::makeAny(sal_Int16(awt::FontUnderline::SINGLE)));
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(sal_Int16(awt::FontUnderline::SINGLE)), xPropSet->getPropertyValue("CharUnderline"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharUnderline");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharUnderline"), aCharUnderline);
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
    css::util::DateTime aDateTime = {sal_uInt32(1234567), sal_uInt16(3), sal_uInt16(3), sal_uInt16(3), sal_uInt16(10), sal_uInt16(11), sal_uInt16(2014), true};
    xUserProps->addProperty("dateTime", sal_Int16(beans::PropertyAttribute::OPTIONAL), uno::makeAny(aDateTime));
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xTextField(xFact->createInstance("com.sun.star.text.textfield.docinfo.Custom"), uno::UNO_QUERY);
    //Setting Name Property
    uno::Reference<beans::XPropertySet> xPropSet(xTextField, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Name", uno::makeAny(OUString("dateTime")));
    //Setting NumberFormat
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XNumberFormatTypes> xNumFormat(xNumberFormatsSupplier->getNumberFormats(), uno::UNO_QUERY);
    css::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    sal_Int16 key = xNumFormat->getStandardFormat(util::NumberFormat::DATETIME, alocale);
    xPropSet->setPropertyValue("NumberFormat", uno::makeAny(sal_Int16(key)));
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
    SwDoc* pDoc = createDoc("tdf90362.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    // Ensure correct initial setting
    comphelper::ConfigurationHelper::writeDirectKey(xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea", css::uno::Any(false), comphelper::EConfigurationModes::Standard);
    // First check if the end of the second paragraph is indeed protected.
    pWrtShell->EndPara();
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->HasReadonlySel());

    // Then enable ignoring of protected areas and make sure that this time the cursor is read-write.
    comphelper::ConfigurationHelper::writeDirectKey(xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea", css::uno::Any(true), comphelper::EConfigurationModes::Standard);
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    // Clean up, otherwise following tests will have that option set
    comphelper::ConfigurationHelper::writeDirectKey(xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea", css::uno::Any(false), comphelper::EConfigurationModes::Standard);
}

void SwUiWriterTest::testUndoDelAsCharTdf107512()
{
    SwDoc * pDoc(createDoc());
    sw::UndoManager & rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell * pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SfxItemSet grfSet(pDoc->GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);
    rIDCO.InsertString(*pShell->GetCursor(), "foo");
    pShell->ClearMark();
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    GraphicObject grf;
    pShell->SttEndDoc(true);
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet, nullptr));
    pShell->SttEndDoc(false);
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    SvxCharHiddenItem hidden(true, RES_CHRATR_HIDDEN);
    pShell->SelectText(1, 4);
    rIDCO.InsertPoolItem(*pShell->GetCursor(), hidden);
    // now we have "\1foo\1" with the "foo" hidden
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringLiteral1(CH_TXTATR_BREAKWORD) + u"foo" + OUStringLiteral1(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    SfxPoolItem const* pItem;
    SfxItemSet query(pDoc->GetAttrPool(), RES_CHRATR_HIDDEN, RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the start
    pShell->SelectText(0, 4);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringLiteral1(CH_TXTATR_BREAKWORD) + u"foo" + OUStringLiteral1(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringLiteral1(CH_TXTATR_BREAKWORD) + u"foo" + OUStringLiteral1(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the end
    pShell->SelectText(1, 5);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringLiteral1(CH_TXTATR_BREAKWORD) + u"foo" + OUStringLiteral1(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringLiteral1(CH_TXTATR_BREAKWORD) + u"foo" + OUStringLiteral1(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
}

void SwUiWriterTest::testUndoCharAttribute()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCursor, "This will be bolded");
    // Position of word                   9876543210
    // Use cursor to select part of text
    pCursor->SetMark();
    for (int i = 0; i < 9; i++) {
        pCursor->Move(fnMoveBackward);
    }
    // Check that correct text was selected
    CPPUNIT_ASSERT_EQUAL(OUString("be bolded"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT);
    // Adds selected text's attributes to aSet
    pCursor->GetNode().GetTextNode()->GetAttr(aSet, 10, 19);
    SfxPoolItem const * pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    // Check that bold is active on the selection; checks if it's in aSet
    CPPUNIT_ASSERT_EQUAL((*pPoolItem == aWeightItem), true);
    // Invoke Undo
    rUndoManager.Undo();
    // Check that bold is no longer active
    aSet.ClearItem(RES_CHRATR_WEIGHT);
    pCursor->GetNode().GetTextNode()->GetAttr(aSet, 10, 19);
    pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL((*pPoolItem == aWeightItem), false);
}

void SwUiWriterTest::testUndoDelAsChar()
{
    SwDoc * pDoc(createDoc());
    sw::UndoManager & rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell * pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SfxItemSet grfSet(pDoc->GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    GraphicObject grf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    pShell->SetMark();
    pShell->Left(1, CRSR_SKIP_CHARS);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(!pShell->GetCursor()->GetNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(!pShell->GetCursor()->GetNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
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
    auto xConnection = xDataSource->getConnection("", "");
    uno::Reference<container::XNameAccess> xTables(
        css::uno::Reference<css::sdbcx::XTablesSupplier>(
            xConnection, uno::UNO_QUERY_THROW)->getTables(),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));
    xConnection->close();

    // Reload: should still have a component and a data source, too.
    reload("writer8", "embedded-data-source.odt");
    CPPUNIT_ASSERT(mxComponent.is());
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1 after saving to a different directory.
    xDataSource.set(xDatabaseContext->getByName("calc-data-source"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    xConnection.set(xDataSource->getConnection("", ""), uno::UNO_QUERY);
    xTables.set(
        css::uno::Reference<css::sdbcx::XTablesSupplier>(
            xConnection, uno::UNO_QUERY_THROW)->getTables(),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));
    xConnection->close();

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
    std::unique_ptr<SwNodeIndex> pIdx(new SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1));
    std::unique_ptr<SwPosition> pPos(new SwPosition(*pIdx));
    sw::UnoCursorPointer pCursor(pDoc->CreateUnoCursor(*pPos));
    CPPUNIT_ASSERT(static_cast<bool>(pCursor));
    pPos.reset(); // we need to kill the SwPosition before disposing
    pIdx.reset(); // we need to kill the SwNodeIndex before disposing
    xDocComponent->dispose();
    CPPUNIT_ASSERT(!static_cast<bool>(pCursor));
}

void SwUiWriterTest::testTextTableCellNames()
{
    sal_Int32 nCol, nRow2;
    SwXTextTable::GetCellPosition( "z1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(51), nCol);
    SwXTextTable::GetCellPosition( "AA1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(52), nCol);
    SwXTextTable::GetCellPosition( "AB1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(53), nCol);
    SwXTextTable::GetCellPosition( "BB1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(105), nCol);
}

void SwUiWriterTest::testShapeAnchorUndo()
{
    SwDoc* pDoc = createDoc("draw-anchor-undo.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    tools::Rectangle aOrigLogicRect(pObject->GetLogicRect());

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.StartUndo(SwUndoId::START, nullptr);

    pWrtShell->SelectObj(Point(), 0, pObject);

    pWrtShell->GetDrawView()->MoveMarkedObj(Size(100, 100));
    pWrtShell->ChgAnchor(RndStdIds::FLY_AT_PARA, true);

    rUndoManager.EndUndo(SwUndoId::END, nullptr);

    CPPUNIT_ASSERT(aOrigLogicRect != pObject->GetLogicRect());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(pObject->GetLogicRect(), aOrigLogicRect);
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
#if HAVE_FEATURE_UI
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
#endif
//IdleTask class to add a low priority Idle task
class IdleTask
{
    public:
    bool GetFlag();
    IdleTask();
    DECL_LINK( FlipFlag, Timer *, void );
    private:
    bool flag;
    Idle maIdle;
};

//constructor of IdleTask Class
IdleTask::IdleTask() : flag( false )
{
    //setting the Priority of Idle task to LOW, LOWEST
    maIdle.SetPriority( TaskPriority::LOWEST );
    //set idle for callback
    maIdle.SetInvokeHandler( LINK( this, IdleTask, FlipFlag) );
    //starting the idle
    maIdle.Start();
}

//GetFlag() of IdleTask Class
bool IdleTask::GetFlag()
{
    //returning the status of current flag
    return this->flag;
}

//Callback function of IdleTask Class
IMPL_LINK(IdleTask, FlipFlag, Timer*, , void)
{
    //setting the flag to make sure that low priority idle task has been dispatched
    this->flag = true;
}

void SwUiWriterTest::testDocModState()
{
    //creating a new writer document via the XDesktop(to have more shells etc.)
    SwDoc* pDoc = createDoc();
    //creating instance of IdleTask Class
    IdleTask idleTask;
    //checking the state of the document via IDocumentState
    IDocumentState& rState(pDoc->getIDocumentState());
    //the state should not be modified
    CPPUNIT_ASSERT(!(rState.IsModified()));
    //checking the state of the document via SfxObjectShell
    SwDocShell* pShell(pDoc->GetDocShell());
    CPPUNIT_ASSERT(!(pShell->IsModified()));
    //looping around yield until low priority idle task is dispatched and flag is flipped
    while(!idleTask.GetFlag())
    {
        //dispatching all the events via VCL main-loop
        Application::Yield();
    }
    //again checking for the state via IDocumentState
    CPPUNIT_ASSERT(!(rState.IsModified()));
    //again checking for the state via SfxObjectShell
    CPPUNIT_ASSERT(!(pShell->IsModified()));
}

void SwUiWriterTest::testTdf94804()
{
    //create new writer document
    SwDoc* pDoc = createDoc();
    //get cursor for making bookmark at a particular location
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCursor();
    IDocumentMarkAccess* pIDMAccess(pDoc->getIDocumentMarkAccess());
    //make first bookmark, CROSSREF_HEADING, with *empty* name
    sw::mark::IMark* pMark1(pIDMAccess->makeMark(*pCrsr, "",
            IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK,
            ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    rtl::OUString bookmark1name = pMark1->GetName();
    //match the bookmark name, it should be like "__RefHeading__**"
    CPPUNIT_ASSERT(bookmark1name.match("__RefHeading__"));
    //make second bookmark, CROSSREF_NUMITEM, with *empty* name
    sw::mark::IMark* pMark2(pIDMAccess->makeMark(*pCrsr, "",
            IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK,
            ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    rtl::OUString bookmark2name = pMark2->GetName();
    //match the bookmark name, it should be like "__RefNumPara__**"
    CPPUNIT_ASSERT(bookmark2name.match("__RefNumPara__"));
}

void SwUiWriterTest::testUnicodeNotationToggle()
{
    SwDoc* pDoc = createDoc("unicodeAltX.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString sOriginalDocString;
    OUString sDocString;
    OUString sExpectedString;
    uno::Sequence<beans::PropertyValue> aPropertyValues;

    pWrtShell->EndPara();
    sOriginalDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("uU+002b"), sOriginalDocString);

    lcl_dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sExpectedString = "u+";
    sDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT( sDocString.equals(sExpectedString) );

    lcl_dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT( sDocString.equals(sOriginalDocString) );
}

void SwUiWriterTest::testTdf34957()
{
    load(DATA_DIRECTORY, "tdf34957.odt");
    // table with "keep with next" always started on a new page if the table was large,
    // regardless of whether it was already kept with the previous paragraph,
    // or whether the following paragraph actually fit on the same page (MAB 3.6 - 5.0)
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[2]/body/tab[1]/row[2]/cell[1]/txt") );
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[4]/body/tab[1]/row[2]/cell[1]/txt") );
}

void SwUiWriterTest::testTdf89954()
{
    SwDoc* pDoc = createDoc("tdf89954.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
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
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT);
    SfxItemSet aSet = aShell.CreateInsertFrameItemSet(aMgr);

    // This was missing along with the gradient and other tables.
    CPPUNIT_ASSERT(aSet.HasItem(SID_COLOR_TABLE));
}

void SwUiWriterTest::testTdf87922()
{
    // Create an SwDrawTextInfo.
    SwDoc* pDoc = createDoc("tdf87922.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwScriptInfo* pScriptInfo = nullptr;
    // Get access to the single paragraph in the document.
    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    const OUString& rText = aNodeIndex.GetNode().GetTextNode()->GetText();
    sal_Int32 nIndex = 0;
    sal_Int32 nLength = rText.getLength();
    SwDrawTextInfo aDrawTextInfo(pWrtShell, *pWrtShell->GetOut(), pScriptInfo, rText, nIndex, nLength);
    // Root -> page -> body -> text.
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    aDrawTextInfo.SetFrame(pTextFrame);

    // If no color background color is found, assume white.
    Color* pColor = sw::GetActiveRetoucheColor();
    *pColor = Color(COL_WHITE);

    // Make sure that automatic color on black background is white, not black.
    vcl::Font aFont;
    aDrawTextInfo.ApplyAutoColor(&aFont);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aFont.GetColor().GetColor());
}

struct PortionItem
{
    PortionItem(OUString const & sItemType, sal_Int32 nLength = 0,
                sal_uInt16 nTextType = 0, OUString const & sText = OUString())
        : msItemType(sItemType)
        , mnLength(nLength)
        , mnTextType(nTextType)
        , msText(sText)
    {}

    OUString msItemType;
    sal_Int32 mnLength;
    sal_uInt16 mnTextType;
    OUString msText;
};

class PortionHandler : public SwPortionHandler
{
  public:

    std::vector<PortionItem> mPortionItems;
    explicit PortionHandler()
        : SwPortionHandler()
    {}

    void clear()
    {
        mPortionItems.clear();
    }

    virtual void Text(sal_Int32 nLength, sal_uInt16 nType,
                      sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/) override
    {
        mPortionItems.push_back(PortionItem("text", nLength, nType));
    }

    virtual void Special(sal_Int32 nLength, const OUString & rText,
                         sal_uInt16 nType, sal_Int32 /*nHeight*/,
                         sal_Int32 /*nWidth*/, const SwFont* /*pFont*/) override
    {
        mPortionItems.push_back(PortionItem("special", nLength, nType, rText));
    }

    virtual void LineBreak(sal_Int32 /*nWidth*/) override
    {
        mPortionItems.push_back(PortionItem("line_break"));
    }

    virtual void Skip(sal_Int32 nLength) override
    {
        mPortionItems.push_back(PortionItem("skip", nLength));
    }

    virtual void Finish() override
    {
        mPortionItems.push_back(PortionItem("finish"));
    }
};

void SwUiWriterTest::testTdf77014()
{
    // The problem described in the bug tdf#77014 is that the input
    // field text ("ThisIsAllOneWord") is broken up on linebreak, but
    // it should be in one piece (like normal text).

    // This test checks that the input field is in one piece and if the
    // input field has more words, it is broken up at the correct place.

    SwDoc* pDoc = createDoc("tdf77014.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());

    PortionHandler aHandler;
    pTextFrame->VisitPortions(aHandler);

    {
        // Input Field - "One Two Three Four Five" = 25 chars
        CPPUNIT_ASSERT_EQUAL(OUString("text"),          aHandler.mPortionItems[0].msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25),             aHandler.mPortionItems[0].mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_INPUTFLD),  aHandler.mPortionItems[0].mnTextType);

        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), aHandler.mPortionItems[1].msItemType);

        CPPUNIT_ASSERT_EQUAL(OUString("finish"), aHandler.mPortionItems[2].msItemType);
    }

    aHandler.clear();

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);

    {
        // Input Field - "ThisIsAllOneWord" = 18 chars
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         aHandler.mPortionItems[0].msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(18),            aHandler.mPortionItems[0].mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_INPUTFLD), aHandler.mPortionItems[0].mnTextType);

        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), aHandler.mPortionItems[1].msItemType);

        CPPUNIT_ASSERT_EQUAL(OUString("finish"), aHandler.mPortionItems[2].msItemType);
    }

    aHandler.clear();

    // skip empty paragraph
    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);

    {
        // Text "The purpose of this report is to summarize the results of the existing bug in the LO suite"
        // = 91 chars
        auto& rPortionItem = aHandler.mPortionItems[0];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(91),       rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_TXT), rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field: "ThisIsAllOneWord" = 18 chars
        // which is 16 chars + 2 hidden chars (start & end input field) = 18 chars
        // If this is correct then the input field is in one piece
        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(18),            rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_INPUTFLD), rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),        rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_TXT), rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[4];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        rPortionItem = aHandler.mPortionItems[5];
        CPPUNIT_ASSERT_EQUAL(OUString("finish"), rPortionItem.msItemType);

    }

    aHandler.clear();

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);
    {
        printf ("Portions:\n");

        for (auto& rPortionItem : aHandler.mPortionItems)
        {
            printf ("-- Type: %s length: %" SAL_PRIdINT32 " text type: %d\n",
                        rPortionItem.msItemType.toUtf8().getStr(),
                        rPortionItem.mnLength,
                        rPortionItem.mnTextType);
        }

        // Text "The purpose of this report is to summarize the results of the existing bug in the LO suite"
        // 91 chars
        auto& rPortionItem = aHandler.mPortionItems[0];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(91),       rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_TXT), rPortionItem.mnTextType);

        // The input field here has more words ("One Two Three Four Five")
        // and it should break after "Two".
        // Input Field: "One Two" = 7 chars + 1 start input field hidden character = 8 chars
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8),             rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_INPUTFLD), rPortionItem.mnTextType);

        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),     rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),         rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_HOLE), rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field:  "Three Four Five" = 16 chars + 1 end input field hidden character = 16 chars
        rPortionItem = aHandler.mPortionItems[4];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16),            rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_INPUTFLD), rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[5];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),        rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(POR_TXT), rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[6];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        rPortionItem = aHandler.mPortionItems[7];
        CPPUNIT_ASSERT_EQUAL(OUString("finish"), rPortionItem.msItemType);
    }
}

void SwUiWriterTest::testTdf92648()
{
    SwDoc* pDoc = createDoc("tdf92648.docx");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // Make sure we have ten draw shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), SwTextBoxHelper::getCount(pPage));
    // and the text boxes haven't got zero height
    sal_Int32 nCount = 0;
    for (const SwFrameFormat* pFormat : *pDoc->GetSpzFrameFormats())
    {
        if (!SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
            continue;
        SwFormatFrameSize aSize(pFormat->GetFrameSize());
        CPPUNIT_ASSERT(aSize.GetHeight() != 0);
        ++nCount;
    }
    // and we have had five of them.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nCount);
}

void SwUiWriterTest::testTdf103978_backgroundTextShape()
{
    SwDoc* pDoc = createDoc("tdf103978_backgroundTextShape.docx");

    // there is only one shape. It has an attached textbox
    bool bShapeIsOpaque = getProperty<bool>(getShape(1), "Opaque");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in the foreground", false, bShapeIsOpaque );
    sal_Int32 nCount = 0;
    for (const SwFrameFormat* pFormat : *pDoc->GetSpzFrameFormats())
    {
        if (!SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
            continue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox syncs the shape's transparency", bShapeIsOpaque, pFormat->GetOpaque().GetValue() );
        ++nCount;
    }
    //ensure that we don't skip the for loop without an error
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of TextBoxes", sal_Int32(1), nCount);
}

void SwUiWriterTest::testTdf96515()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a new paragraph at the end of the document.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(), uno::UNO_QUERY);
    xParagraphAppend->finishParagraph(uno::Sequence<beans::PropertyValue>());
    calcLayout();

    // This was 2, a new page was created for the new paragraph.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void SwUiWriterTest::testTdf96943()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createDoc("tdf96943.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // Insert a new character at the end of the document.
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("d");

    // This was 2, a new page was created for the new layout line.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void SwUiWriterTest::testTdf96536()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a page break and go back to the first page.
    pWrtShell->InsertPageBreak();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    calcLayout();
    sal_Int32 nSingleParaPageHeight = parseDump("/root/page[1]/infos/bounds", "height").toInt32();
    discardDumpedLayout();

    // Insert a 2nd paragraph at the end of the first page, so the page height grows at least twice...
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(), uno::UNO_QUERY);
    const uno::Reference< text::XTextRange > xInsertPos = getRun(getParagraph(1), 1);
    xParagraphAppend->finishParagraphInsert(uno::Sequence<beans::PropertyValue>(), xInsertPos);
    calcLayout();
    CPPUNIT_ASSERT(parseDump("/root/page[1]/infos/bounds", "height").toInt32() >= 2 * nSingleParaPageHeight);
    discardDumpedLayout();

    // ... and then delete the 2nd paragraph, which shrinks the page to the previous size.
    uno::Reference<lang::XComponent> xParagraph(getParagraph(2), uno::UNO_QUERY);
    xParagraph->dispose();
    calcLayout();
    CPPUNIT_ASSERT_EQUAL(nSingleParaPageHeight, parseDump("/root/page[1]/infos/bounds", "height").toInt32());
}

void SwUiWriterTest::testTdf96479()
{
    // We want to verify the empty input text field in the bookmark
    static const OUString emptyInputTextField =
        OUStringLiteral1(CH_TXT_ATR_INPUTFIELDSTART) + OUStringLiteral1(CH_TXT_ATR_INPUTFIELDEND);

    SwDoc* pDoc = createDoc();
    SwXTextDocument *pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // So we can clean up all references for reload
    {
        // Append bookmark
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        IDocumentMarkAccess &rIDMA = *pDoc->getIDocumentMarkAccess();
        sw::mark::IMark *pMark =
            rIDMA.makeMark(aPaM, "original",
                IDocumentMarkAccess::MarkType::BOOKMARK,
                ::sw::mark::InsertMode::New);
        CPPUNIT_ASSERT(!pMark->IsExpanded());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());

        // Get helper objects
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

        // Create cursor from bookmark
        uno::Reference<text::XTextContent> xTextContent(xBookmarksSupplier->getBookmarks()->getByName("original"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursor(xRange->getText()->createTextCursorByRange(xRange), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCursor->isCollapsed());

        // Remove bookmark
        xRange->getText()->removeTextContent(xTextContent);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getBookmarksCount());

        // Insert replacement bookmark
        uno::Reference<text::XTextContent> xBookmarkNew(xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkName(xBookmarkNew, uno::UNO_QUERY);
        xBookmarkName->setName("replacement");
        CPPUNIT_ASSERT(xCursor->isCollapsed());
        // Force bookmark expansion
        xCursor->getText()->insertString(xCursor, ".", true);
        xCursor->getText()->insertTextContent(xCursor, xBookmarkNew, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        auto mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());

        // Create and insert input textfield with some content
        uno::Reference<text::XTextField> xTextField(xFactory->createInstance("com.sun.star.text.TextField.Input"), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursorNew(xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(xBookmarkNew->getAnchor()));
        CPPUNIT_ASSERT(!xCursorNew->isCollapsed());
        xCursorNew->getText()->insertTextContent(xCursorNew, xTextField, true);
        xBookmarkNew = uno::Reference<text::XTextContent>(xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        xCursorNew = uno::Reference<text::XTextCursor>(xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(xBookmarkNew->getAnchor()));
        CPPUNIT_ASSERT(!xCursorNew->isCollapsed());

        // Can't check the actual content of the text node via UNO
        mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());
        SwPaM pam(mark->GetMarkStart(), mark->GetMarkEnd());
        // Check for the actual bug, which didn't include CH_TXT_ATR_INPUTFIELDEND in the bookmark
        CPPUNIT_ASSERT_EQUAL(emptyInputTextField, pam.GetText());
    }

    {
        // Save and load cycle
        // Actually not needed, but the bug symptom of a missing bookmark
        // occurred because a broken bookmark was saved and loading silently
        // dropped the broken bookmark!
        utl::TempFile aTempFile;
        save("writer8", aTempFile);
        loadURL(aTempFile.GetURL(), nullptr);
        pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        pDoc = pTextDoc->GetDocShell()->GetDoc();

        // Lookup "replacement" bookmark
        IDocumentMarkAccess &rIDMA = *pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xBookmarksSupplier->getBookmarks()->hasByName("replacement"));

        uno::Reference<text::XTextContent> xTextContent(xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursor(xRange->getText()->createTextCursorByRange(xRange), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xCursor->isCollapsed());

        // Verify bookmark content via text node / PaM
        auto mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());
        SwPaM pam(mark->GetMarkStart(), mark->GetMarkEnd());
        CPPUNIT_ASSERT_EQUAL(emptyInputTextField, pam.GetText());
    }
}

void SwUiWriterTest::testTdf96961()
{
    // Insert a page break.
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    calcLayout();

    // Assert that the height of the last page is larger than the height of other pages.
    sal_Int32 nOther = parseDump("/root/page[1]/infos/bounds", "height").toInt32();
    sal_Int32 nLast = parseDump("/root/page[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT(nLast > nOther);
}

void SwUiWriterTest::testTdf88453()
{
    createDoc("tdf88453.odt");
    calcLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 0: the table does not fit the first page, but it wasn't split
    // to continue on the second page.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab", 1);
}

void SwUiWriterTest::testTdf88453Table()
{
    createDoc("tdf88453-table.odt");
    calcLayout();
    // This was 2: layout could not split the large outer table in the document
    // into 3 pages.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

namespace
{

int checkShells(SwDocShell* pSource, SwDocShell* pDestination)
{
    return int(SfxClassificationHelper::CheckPaste(pSource->getDocProperties(), pDestination->getDocProperties()));
}

}

void SwUiWriterTest::testClassificationPaste()
{
    SwDocShell* pSourceShell = createDoc()->GetDocShell();
    uno::Reference<lang::XComponent> xSourceComponent = mxComponent;
    mxComponent.clear();

    SwDocShell* pDestinationShell = createDoc()->GetDocShell();

    // Not classified source, not classified destination.
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::None), checkShells(pSourceShell, pDestinationShell));

    // Classified source, not classified destination.
    uno::Sequence<beans::PropertyValue> aInternalOnly = comphelper::InitPropertySequence({{"Name", uno::makeAny(OUString("Internal Only"))}});
    lcl_dispatchCommand(xSourceComponent, ".uno:ClassificationApply", aInternalOnly);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::TargetDocNotClassified), checkShells(pSourceShell, pDestinationShell));

    // Classified source and classified destination -- internal only has a higher level than confidential.
    uno::Sequence<beans::PropertyValue> aConfidential = comphelper::InitPropertySequence({{"Name", uno::makeAny(OUString("Confidential"))}});
    lcl_dispatchCommand(mxComponent, ".uno:ClassificationApply", aConfidential);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::DocClassificationTooLow), checkShells(pSourceShell, pDestinationShell));

    xSourceComponent->dispose();
}

void SwUiWriterTest::testSmallCaps()
{
    // Create a document, add some characters and select them.
    createDoc();
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("text");
    pWrtShell->SelAll();

    // Dispatch the command to make them formatted small capitals.
    lcl_dispatchCommand(mxComponent, ".uno:SmallCaps", {});

    // This was css::style::CaseMap::NONE as the shell didn't handle the command.
    CPPUNIT_ASSERT_EQUAL(css::style::CaseMap::SMALLCAPS, getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharCaseMap"));
}

void SwUiWriterTest::testTdf98987()
{
    createDoc("tdf98987.docx");
    calcLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/SdrObject", "name", "Rectangle 1");
    sal_Int32 nRectangle1 = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "top").toInt32();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject", "name", "Rectangle 2");
    sal_Int32 nRectangle2 = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds", "top").toInt32();
    CPPUNIT_ASSERT(nRectangle1 < nRectangle2);

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/SdrObject", "name", "Rectangle 3");
    sal_Int32 nRectangle3 = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/bounds", "top").toInt32();
    // This failed: the 3rd rectangle had a smaller "top" value than the 2nd one, it even overlapped with the 1st one.
    CPPUNIT_ASSERT(nRectangle2 < nRectangle3);
}

void SwUiWriterTest::testTdf99004()
{
    createDoc("tdf99004.docx");
    calcLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nTextbox1Top = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "top").toInt32();
    sal_Int32 nTextBox1Height = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32();
    sal_Int32 nTextBox1Bottom = nTextbox1Top + nTextBox1Height;

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject", "name", "Rectangle 2");
    sal_Int32 nRectangle2Top = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds", "top").toInt32();
    // This was 3291 and 2531, should be now around 2472 and 2531, i.e. the two rectangles should not overlap anymore.
    CPPUNIT_ASSERT(nTextBox1Bottom < nRectangle2Top);
}

void SwUiWriterTest::testTdf84695()
{
    SwDoc* pDoc = createDoc("tdf84695.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(RES_FLYFRMFMT, static_cast<RES_FMT>(pTextBox->GetFormat()->Which()));

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now Enter + a key should add some text.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, Enter did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

void SwUiWriterTest::testTdf84695NormalChar()
{
    SwDoc* pDoc = createDoc("tdf84695.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(RES_FLYFRMFMT, static_cast<RES_FMT>(pTextBox->GetFormat()->Which()));

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'a' should add a character.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, pressing a normal character did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

void SwUiWriterTest::testTdf84695Tab()
{
    SwDoc* pDoc = createDoc("tdf84695-tab.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SwContact* pShape = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pShape is a draw shape.
    CPPUNIT_ASSERT_EQUAL(RES_DRAWFRMFMT, static_cast<RES_FMT>(pShape->GetFormat()->Which()));

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'tab' should jump to the other shape.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);

    // And finally make sure the selection has changed.
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwContact* pOtherShape = static_cast<SwContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // This failed, 'tab' didn't do anything -> the selected shape was the same.
    CPPUNIT_ASSERT(pOtherShape != pShape);
}

void SwUiWriterTest::testTableStyleUndo()
{
    SwDoc* pDoc = createDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sal_Int32 nStyleCount = pDoc->GetTableStyles().size();
    SwTableAutoFormat* pStyle = pDoc->MakeTableStyle("Test Style");
    SvxBrushItem aBackground(Color(0xFF00FF), RES_BACKGROUND );
    pStyle->GetBoxFormat(0).SetBackground(aBackground);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    // check if attributes are preserved
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    pDoc->DelTableStyle("Test Style");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    // check if attributes are preserved
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);

    // undo delete so we can replace the style
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount +1 );
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    SwTableAutoFormat aNewStyle("Test Style2");
    SvxBrushItem aBackground2(Color(0x00FF00), RES_BACKGROUND);
    aNewStyle.GetBoxFormat(0).SetBackground(aBackground2);

    pDoc->ChgTableStyle("Test Style", aNewStyle);
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
    rUndoManager.Undo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat("Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
}

void SwUiWriterTest::testRedlineCopyPaste()
{
    // regressed in tdf#106746
    SwDoc* pDoc = createDoc();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "abzdezgh");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();

    // Turn on track changes, make changes, turn off track changes
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    lcl_selectCharacters(aPaM, 2, 3);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "c", false);
    lcl_selectCharacters(aPaM, 6, 7);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "f", false);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(false));

    // Create the clipboard document.
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Select the whole content, copy, delete the original and paste the copied content
    pWrtShell->SelAll();
    pWrtShell->Copy(&aClipboard);
    pWrtShell->Delete();
    pWrtShell->Paste(&aClipboard);

    // With the bug this is "abzcdefgh", ie. contains the first deleted piece, too
    CPPUNIT_ASSERT_EQUAL(OUString("abcdefgh"), pTextNode->GetText());
}

void SwUiWriterTest::testRedlineParam()
{
    // Create a document with minimal content.
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and end of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->SttDoc();
    pWrtShell->Insert("aaa");
    pWrtShell->EndDoc();
    pWrtShell->Insert("zzz");

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());

    // Select the first redline.
    pWrtShell->SttDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"NextTrackedChange", uno::makeAny(static_cast<sal_uInt16>(rTable[0]->GetId()))}
    }));
    lcl_dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This failed: the parameter wasn't handled so the next change (zzz) was
    // selected, not the first one (aaa).
    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), pShellCursor->GetText());

    // Select the second redline.
    pWrtShell->SttDoc();
    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"NextTrackedChange", uno::makeAny(static_cast<sal_uInt16>(rTable[1]->GetId()))}
    });
    lcl_dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("zzz"), pShellCursor->GetText());

    // Move the cursor to the start again, and reject the second change.
    pWrtShell->SttDoc();
    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"RejectTrackedChange", uno::makeAny(static_cast<sal_uInt16>(rTable[1]->GetId()))}
    });
    lcl_dispatchCommand(mxComponent, ".uno:RejectTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    pShellCursor = pWrtShell->getShellCursor(false);

    // This was 'middlezzz', the uno command rejected the redline under the
    // cursor, instead of the requested one.
    CPPUNIT_ASSERT_EQUAL(OUString("aaamiddle"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwUiWriterTest::testRedlineViewAuthor()
{
    // Test that setting an author at an SwView level has effect.

    // Create a document with minimal content.
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");
    SwView* pView = pDocShell->GetView();
    const OUString aAuthor("A U. Thor");
    pView->SetRedlineAuthor(aAuthor);
    pDocShell->SetView(pView);

    // Turn on track changes, and add changes to the start of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->SttDoc();
    pWrtShell->Insert("aaa");

    // Now assert that SwView::SetRedlineAuthor() had an effect.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
    SwRangeRedline* pRedline = rTable[0];
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, pRedline->GetAuthorString());

    // Insert a comment and assert that SwView::SetRedlineAuthor() affects this as well.
    lcl_dispatchCommand(mxComponent, ".uno:.uno:InsertAnnotation", {});
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, xField->getPropertyValue("Author").get<OUString>());
}

void SwUiWriterTest::testTdf78727()
{
    SwDoc* pDoc = createDoc("tdf78727.docx");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // This was 1: make sure we don't loose the TextBox anchored inside the
    // table that is moved inside a text frame.
    CPPUNIT_ASSERT(SwTextBoxHelper::getCount(pPage) > 1);
}

void SwUiWriterTest::testRedlineTimestamp()
{
    // Test that a redline timestamp's second is not always 0.

    // Create a document with minimal content.
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and to the end of
    // the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->SttDoc();
    pWrtShell->Insert("aaa");
    osl::Thread::wait(std::chrono::seconds(1));
    pWrtShell->EndDoc();
    pWrtShell->Insert("zzz");

    // Inserting additional characters at the start changed the table size to
    // 3, i.e. the first and the second "aaa" wasn't combined.
    pWrtShell->SttDoc();
    pWrtShell->Insert("aaa");

    // Now assert that at least one of the seconds are not 0.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    if (rTable.size() >= 2 && rTable[0]->GetRedlineData().GetTimeStamp().GetMin() != rTable[1]->GetRedlineData().GetTimeStamp().GetMin())
        // The relatively rare case when waiting for a second also changes the minute.
        return;

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
    sal_uInt16 nSec1 = rTable[0]->GetRedlineData().GetTimeStamp().GetSec();
    sal_uInt16 nSec2 = rTable[1]->GetRedlineData().GetTimeStamp().GetSec();
    // This failed, seconds was always 0.
    CPPUNIT_ASSERT(nSec1 != 0 || nSec2 != 0);
}

void SwUiWriterTest::testCursorWindows()
{
    // Create a new document with one window.
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell1 = pDocShell->GetWrtShell();

    // Create a second view and type something.
    pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_NEWWINDOW, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
    SwWrtShell* pWrtShell2 = pDocShell->GetWrtShell();
    OUString aText("foo");
    pWrtShell2->Insert(aText);

    // Assert that only the cursor of the actual window move, not other cursors.
    SwShellCursor* pShellCursor1 = pWrtShell1->getShellCursor(false);
    SwShellCursor* pShellCursor2 = pWrtShell2->getShellCursor(false);
    // This was 3, not 0 -- cursor of the other window moved.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCursor1->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), pShellCursor2->Start()->nContent.GetIndex());
}

void SwUiWriterTest::testLandscape()
{
    // Set page orientation to landscape.
    SwDoc* pDoc = createDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue("AttributePage.Landscape", true),
    };
    lcl_dispatchCommand(mxComponent, ".uno:AttributePage", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Assert that the document model was modified.
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    size_t nPageDesc = pWrtShell->GetCurPageDesc();
    // This failed, page was still portrait.
    CPPUNIT_ASSERT(pWrtShell->GetPageDesc(nPageDesc).GetLandscape());
}

void SwUiWriterTest::testTdf95699()
{
    // Open the document with single FORMCHECKBOX field, select all and copy to clipboard
    // then check that clipboard contains the FORMCHECKBOX in text body.
    // Previously that failed.
    SwDoc* pDoc = createDoc("tdf95699.odt");
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->Copy(&aClipboard);
    pMarkAccess = aClipboard.getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    ::sw::mark::IFieldmark* pFieldMark = pMarkAccess->getFieldmarkAfter(SwPosition(pDoc->GetNodes().GetEndOfExtras()));
    CPPUNIT_ASSERT_EQUAL(OUString("vnd.oasis.opendocument.field.FORMCHECKBOX"), pFieldMark->GetFieldname());
}

void SwUiWriterTest::testTdf104032()
{
    // Open the document with FORMCHECKBOX field, select it and copy to clipboard
    // Go to end of document and paste it, then undo
    // Previously that asserted in debug build.
    SwDoc* pDoc = createDoc("tdf104032.odt");
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttDoc();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Copy(&aClipboard);
    pWrtShell->EndDoc();
    pWrtShell->Paste(&aClipboard);
    rUndoManager.Undo();
}

void SwUiWriterTest::testTdf104440()
{
    createDoc("tdf104440.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//page[2]/body/txt/anchored");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    // This was 0: both Text Frames in the document were anchored to a
    // paragraph on page 1, while we expect that the second Text Frame is
    // anchored to a paragraph on page 2.
    CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

void SwUiWriterTest::testTdf104425()
{
    createDoc("tdf104425.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // The document contains one top-level 1-cell table with minimum row height set to 70 cm,
    // and the cell contents does not exceed the minimum row height.
    // It should span over 3 pages.
    assertXPath(pXmlDoc, "//page", 3);
    sal_Int32 nHeight1 = getXPath(pXmlDoc, "//page[1]/body/tab/row/infos/bounds", "height").toInt32();
    sal_Int32 nHeight2 = getXPath(pXmlDoc, "//page[2]/body/tab/row/infos/bounds", "height").toInt32();
    sal_Int32 nHeight3 = getXPath(pXmlDoc, "//page[3]/body/tab/row/infos/bounds", "height").toInt32();
    double fSumHeight_mm = (nHeight1 + nHeight2 + nHeight3) * 25.4 / 1440.0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(700.0, fSumHeight_mm, 0.05);
}

// accepting change tracking gets stuck on change
void SwUiWriterTest::testTdf104814()
{
    SwDoc* const pDoc1(createDoc("tdf104814.docx"));

    SwEditShell* const pEditShell(pDoc1->GetEditShell());

    // accept all redlines
    while(pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);
}

void SwUiWriterTest::testTdf66405()
{
    // Imported formula should have zero margins
    createDoc("tdf66405.docx");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEmbeddedObjects = xEmbeddedObjectsSupplier->getEmbeddedObjects();
    uno::Reference<beans::XPropertySet> xFormula;
    xEmbeddedObjects->getByName(xEmbeddedObjects->getElementNames()[0]) >>= xFormula;
    uno::Reference<beans::XPropertySet> xComponent;
    xFormula->getPropertyValue("Component") >>= xComponent;

    // Test embedded object's margins
    sal_Int32 nLeftMargin, nRightMargin, nTopMargin, nBottomMargin;
    xFormula->getPropertyValue("LeftMargin") >>= nLeftMargin;
    xFormula->getPropertyValue("RightMargin") >>= nRightMargin;
    xFormula->getPropertyValue("TopMargin") >>= nTopMargin;
    xFormula->getPropertyValue("BottomMargin") >>= nBottomMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRightMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);

    // Test embedded object component's margins
    xComponent->getPropertyValue("LeftMargin") >>= nLeftMargin;
    xComponent->getPropertyValue("RightMargin") >>= nRightMargin;
    xComponent->getPropertyValue("TopMargin") >>= nTopMargin;
    xComponent->getPropertyValue("BottomMargin") >>= nBottomMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRightMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);
}

void SwUiWriterTest::testTdf35021_tabOverMarginDemo()
{
    createDoc("tdf35021_tabOverMarginDemo.doc");
    calcLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Tabs should go past the margin @ ~3381
    sal_Int32 nMargin = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds", "width").toInt32();
    // left tab was 3381 because it got its own full line
    sal_Int32 nWidth = getXPath(pXmlDoc, "//Text[@nType='POR_TABLEFT']", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~4479", nMargin < nWidth);
    // center tab was 842
    nWidth = getXPath(pXmlDoc, "//Text[@nType='POR_TABCENTER']", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Center Tab width is ~3521", nMargin < nWidth);
    // right tab was probably the same as center tab.
    nWidth = getXPath(pXmlDoc, "//Text[@nType='POR_TABRIGHT']", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Right Tab width is ~2907", sal_Int32(2500) < nWidth);
    // decimal tab was 266
    nWidth = getXPath(pXmlDoc, "//Text[@nType='POR_TABDECIMAL']", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Decimal Tab width is ~4096", nMargin < nWidth);
}

void SwUiWriterTest::testTdf106701_tabOverMarginAutotab()
{
    createDoc("tdf106701_tabOverMarginAutotab.doc");
    calcLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // The right margin is ~3378
    sal_Int32 nRightMargin = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds", "width").toInt32();
    // Automatic tabstops should never be affected by tabOverMargin compatibility
    // The 1st line's width previously was ~9506
    sal_Int32 nWidth = getXPath(pXmlDoc, "//LineBreak[1]", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("1st line's width is less than the right margin", nWidth < nRightMargin);
}

void SwUiWriterTest::testTdf104492()
{
    createDoc("tdf104492.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // The document should split table over 3 pages.
    assertXPath(pXmlDoc, "//page", 3);
}

void SwUiWriterTest::testTdf107025()
{
    // Tdf107025 - characters advance with wrong distance, so that
    // they are cluttered because of negative value or
    // break into multiple lines because of overflow.
    // The test document uses DFKAI-SB shipped with Windows.
    createDoc("tdf107025.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Verify the number of characters in each line.
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[1]", "nLength").toInt32());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(9), getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[2]", "nLength").toInt32());

    // Do the subsequent test only if the first line can be displayed,
    // in case that the required font does not exist.
    sal_Int32 nWidth1 = getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[1]", "nWidth").toInt32();
    if (!nWidth1)
        return;

    CPPUNIT_ASSERT(!parseDump("(//Text[@nType='POR_TXT'])[2]", "nWidth").isEmpty());
    // Width of the second line is expected to be 9 times of the first.
    sal_Int32 nWidth2 = getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[2]", "nWidth").toInt32();

    CPPUNIT_ASSERT_EQUAL( sal_Int32(9), nWidth2 / nWidth1 );
}

void SwUiWriterTest::testTdf107362()
{
    createDoc("tdf107362.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nHeight = getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[1]" , "nHeight").toInt32();
    sal_Int32 nWidth1 = getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[1]" , "nWidth").toInt32();
    sal_Int32 nWidth2 = getXPath(pXmlDoc, "(//Text[@nType='POR_TXT'])[2]" , "nWidth").toInt32();
    sal_Int32 nLineWidth = getXPath(pXmlDoc, "//LineBreak" , "nWidth").toInt32();
    sal_Int32 nKernWidth = nLineWidth - nWidth1 - nWidth2;
    // Test only if fonts are available
    if ( nWidth1 > 500 && nWidth2 > 200 )
    {
        // Kern width should be smaller than 1/3 of the CJK font height.
        CPPUNIT_ASSERT( nKernWidth * 3 < nHeight );
    }
}

void SwUiWriterTest::testTdf105417()
{
    SwDoc* pDoc = createDoc("tdf105417.odt");
    CPPUNIT_ASSERT(pDoc);
    SwView* pView = pDoc->GetDocShell()->GetView();
    CPPUNIT_ASSERT(pView);
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    CPPUNIT_ASSERT(xHyphenator.is());
    // If there are no English hyphenation rules installed, we can't test
    // hyphenation.
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    uno::Reference<linguistic2::XLinguProperties> xLinguProperties(LinguMgr::GetLinguPropertySet());
    // Automatic hyphenation means not opening a dialog, but going ahead
    // non-interactively.
    xLinguProperties->setIsHyphAuto(true);
    SwHyphWrapper aWrap(pView, xHyphenator, /*bStart=*/false, /*bOther=*/true, /*bSelection=*/false);
    // This never returned, it kept trying to hyphenate the last word
    // (greenbacks) again and again.
    aWrap.SpellDocument();
}

void SwUiWriterTest::testTdf105625()
{
    SwDoc* pDoc = createDoc("tdf105625.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    // Ensure correct initial setting
    comphelper::ConfigurationHelper::writeDirectKey(xComponentContext,
        "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea",
        css::uno::Any(false), comphelper::EConfigurationModes::Standard);
    // We should be able to edit at positions adjacent to fields.
    // Check if the start and the end of the 1st paragraph are not protected
    // (they are adjacent to FORMCHECKBOX)
    pWrtShell->SttPara();
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    pWrtShell->EndPara();
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    // 2nd paragraph - FORMTEXT
    pWrtShell->Down(/*bSelect=*/false);
    // Check selection across FORMTEXT field boundary - must be read-only
    pWrtShell->SttPara();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->HasReadonlySel());
    // Test deletion of whole field with single backspace
    // Previously it only removed right boundary of FORMTEXT, or failed removal at all
    const IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    sal_Int32 nMarksBefore = pMarksAccess->getAllMarksCount();
    pWrtShell->EndPara();
    pWrtShell->DelLeft();
    sal_Int32 nMarksAfter = pMarksAccess->getAllMarksCount();
    CPPUNIT_ASSERT_EQUAL(nMarksBefore, nMarksAfter + 1);
}

void SwUiWriterTest::testTdf106736()
{
    createDoc("tdf106736-grid.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth = getXPath(pXmlDoc, "(//Text[@nType='POR_TABLEFT'])[1]", "nWidth").toInt32();
    // In tdf106736, width of tab overflow so that it got
    // width value around 9200, expected value is around 103
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~103", nWidth < 150);
}

void SwUiWriterTest::testMsWordCompTrailingBlanks()
{
    // The option is true in settings.xml
    SwDoc* pDoc = createDoc( "MsWordCompTrailingBlanksTrue.odt" );
    CPPUNIT_ASSERT_EQUAL( true, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS ) );
    calcLayout();
    // Check that trailing spaces spans have no width if option is enabled

    CPPUNIT_ASSERT_EQUAL( OUString(), parseDump( "/root/page/body/txt[2]/Text[4]", "nWidth" ) );
    CPPUNIT_ASSERT_EQUAL( OUString(), parseDump( "/root/page/body/txt[2]/Text[5]", "nWidth" ) );
    CPPUNIT_ASSERT_EQUAL( OUString(), parseDump( "/root/page/body/txt[3]/Text[4]", "nWidth" ) );
    CPPUNIT_ASSERT_EQUAL( OUString(), parseDump( "/root/page/body/txt[3]/Text[5]", "nWidth" ) );

    // The option is false in settings.xml
    pDoc = createDoc( "MsWordCompTrailingBlanksFalse.odt" );
    CPPUNIT_ASSERT_EQUAL( false, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS ) );
    calcLayout();
    // Check that trailing spaces spans have width if option is disabled
    CPPUNIT_ASSERT( !parseDump( "/root/page/body/txt[2]/Text[4]", "nWidth" ).isEmpty() );
    CPPUNIT_ASSERT( !parseDump( "/root/page/body/txt[2]/Text[5]", "nWidth" ).isEmpty() );
    CPPUNIT_ASSERT( !parseDump( "/root/page/body/txt[3]/Text[4]", "nWidth" ).isEmpty() );
    CPPUNIT_ASSERT( !parseDump( "/root/page/body/txt[3]/Text[5]", "nWidth" ).isEmpty() );

    // MsWordCompTrailingBlanks option should be false by default in new documents
    pDoc = createDoc();
    CPPUNIT_ASSERT_EQUAL( false, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS ) );

    // The option should be true if a .docx, .doc or .rtf document is opened
    pDoc = createDoc( "MsWordCompTrailingBlanks.docx" );
    CPPUNIT_ASSERT_EQUAL( true, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS ) );
}

void SwUiWriterTest::testCreateDocxAnnotation()
{
    createDoc();

    // insert an annotation with a text
    const OUString aSomeText("some text");
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Text", uno::makeAny(aSomeText)},
        {"Author", uno::makeAny(OUString("me"))},
    });
    lcl_dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "create-docx-annotation.docx");

    // get the annotation
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);

    // this was empty instead of "some text"
    OUString aResultText = aSomeText
#ifdef WNT
        // FIXME From some unclear reason, on windows we get an additional
        // paragraph in the comment - please adapt this test when that gets
        // fixed.
        + "\n"
#endif
        ;
    CPPUNIT_ASSERT_EQUAL(aResultText, xField->getPropertyValue("Content").get<OUString>());
}

void SwUiWriterTest::testTdf107976()
{
    // Create a document and create two transferables.
    SwDoc* pDoc = createDoc();
    SwWrtShell& rShell = *pDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> pTransferable(new SwTransferable(rShell));
    rtl::Reference<SwTransferable> pTransferable2(new SwTransferable(rShell));
    // Now close the document.
    mxComponent->dispose();
    mxComponent.clear();
    // This failed: the first shell had a pointer to the deleted shell.
    CPPUNIT_ASSERT(!pTransferable->GetShell());
    CPPUNIT_ASSERT(!pTransferable2->GetShell());
}

void SwUiWriterTest::testTdf114536()
{
    // This crashed in SwTextFormatter::MergeCharacterBorder() due to a
    // use after free.
    createDoc("tdf114536.odt");
}

void SwUiWriterTest::testTdf113790()
{
    SwDoc* pDoc = createDoc("tdf113790.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Create the clipboard document.
    SwDoc aClipboard;
    aClipboard.SetClipBoard(true);

    // Go to fourth line - to "ABCD" bulleted list item
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->SelPara(nullptr);
    CPPUNIT_ASSERT_EQUAL(OUString("ABCD"), pWrtShell->GetSelText());
    pWrtShell->Copy(&aClipboard);

    // Go down to next-to-last (empty) line above "Title3"
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->Paste(&aClipboard);

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "tdf113790.docx");
    CPPUNIT_ASSERT(dynamic_cast<SwXTextDocument *>(mxComponent.get()));
}

void SwUiWriterTest::testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc)
{
    load(DATA_DIRECTORY, aDestDoc);

    // set a page cursor into the end of the document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    // insert the same document at current cursor position
    {
        const OUString insertFileid = m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(aInsertDoc);
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
        lcl_dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    }
}

// During insert of the document with list inside into the main document inside the list
// we should merge both lists into one, when they have the same list properties
void SwUiWriterTest::testTdf113877()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT_EQUAL(listId4, listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT_EQUAL(listId6, listId7);
}

// The same test as testTdf113877() but merging of two list should not be performed.
void SwUiWriterTest::testTdf113877NoMerge()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT(listId4 != listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT(listId6 != listId7);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has NO its own paragraph style ("Standard" will be used).
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest::testTdf113877_default_style()
{
    testTdf113877_mergeDocs("tdf113877_blank.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has its own paragraph style derived from "Standard", but this style is the same as "Standard".
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest::testTdf113877_Standard_style()
{
    testTdf113877_mergeDocs("tdf113877_blank_ownStandard.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest::testTdf113877_blank_bold_on()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_on.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest::testTdf113877_blank_bold_off()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_off.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest::testPlaceholderHTMLPaste()
{
    {
        SwDoc* const pDoc = createDoc();
        SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        pWrtShell->Insert("AAA");
        pWrtShell->SplitNode();
        pWrtShell->Insert("BBB");
        pWrtShell->SplitNode();
        pWrtShell->Insert("CCC");

        lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
        lcl_dispatchCommand(mxComponent, ".uno:Copy", {});
    }

    SwDoc* const pDoc = createDoc("placeholder-bold.fodt");

    // select placeholder field
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Paste special as HTML
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::HTML)) } });

    lcl_dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(int(4), getParagraphs());

    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, "Test "), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::BOLD,
        getProperty<float>(getRun(getParagraph(1), 2, "AAA"), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(2), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(3), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(4), 1, " test"), "CharWeight"));
}

void SwUiWriterTest::testPlaceholderHTMLInsert()
{
    createDoc("placeholder-bold-cs.fodt");

    // select placeholder field
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor{ xTextDocument->getText()->createTextCursor() };
    xCursor->gotoStart(false);
    xCursor->goRight(5, false);
    xCursor->goRight(1, true);

    sal_Int8 const html[] = "<meta http-equiv=\"content-type\" content=\"text/html; "
                            "charset=utf-8\"><p>AAA</p><p>BBB</p><p>CCC</p>";
    uno::Sequence<sal_Int8> seq{html, sizeof(html)};
    uno::Reference<io::XInputStream> xStream{ new comphelper::SequenceInputStream{seq} };

    // insert HTML file
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "InputStream", uno::Any(xStream) },
          { "Hidden", uno::Any(true) },
          { "FilterName", uno::Any(OUString("HTML (StarWriter)")) } });

    uno::Reference<document::XDocumentInsertable> xDocInsert{ xCursor, uno::UNO_QUERY };
    xDocInsert->insertDocumentFromURL("private:stream", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(int(4), getParagraphs());

    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, "Test "), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::BOLD,
        getProperty<float>(getRun(getParagraph(1), 2, "AAA"), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(2), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(3), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(4), 1, " test"), "CharWeight"));
}

void SwUiWriterTest::testPlaceholderHTMLPasteStyleOverride()
{
    {
        SwDoc *const pDoc = createDoc();

        SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        pWrtShell->Insert("AAA");
        pWrtShell->SplitNode();
        pWrtShell->Insert("BBB");

        lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
        lcl_dispatchCommand(mxComponent, ".uno:Copy", {});
    }

    SwDoc *const pDoc = createDoc("placeholder-bold-style-override.fodt");

    // select placeholder field
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 15, /*bBasicCall=*/false);

    // Paste special as HTML
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::HTML)) } });

    lcl_dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    // style sets it to bold
    uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xStyles(
        xSFS->getStyleFamilies()->getByName("ParagraphStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStandard(xStyles->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextbody(xStyles->getByName("Text body"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStandard, "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xTextbody, "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xTextbody, "ParaAdjust"));

    CPPUNIT_ASSERT_EQUAL(int(3), getParagraphs());

    // but hard formatting overrides to normal
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, "AAA"), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(2), 1, "BBB"), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(2), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text body"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text body"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
