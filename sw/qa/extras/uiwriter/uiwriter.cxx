/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <vcl/errcode.hxx>
#include <tools/gen.hxx>
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
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <unotbl.hxx>
#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagedesc.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <vcl/TypeSerializer.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <svx/svxids.hrc>
#include <unotools/localfilehelper.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <i18nutil/transliteration.hxx>
#include <i18nutil/searchopt.hxx>
#include <reffld.hxx>
#include <dbfld.hxx>
#include <txatbase.hxx>
#include <txtftn.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <unofldmid.h>
#include <UndoManager.hxx>
#include <textsh.hxx>
#include <frmatr.hxx>
#include <frmmgr.hxx>
#include <tblafmt.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <paratr.hxx>
#include <drawfont.hxx>
#include <txtfrm.hxx>
#include <txttypes.hxx>
#include <SwPortionHandler.hxx>
#include <hyp.hxx>
#include <swdtflvr.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/classificationhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/docfilt.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/configurationhelper.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/scheduler.hxx>
#include <config_features.h>
#include <sfx2/watermarkitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <test/htmltesttools.hxx>
#include <fmthdft.hxx>
#include <iodetect.hxx>
#include <wrthtml.hxx>
#include <dbmgr.hxx>
#include <frameformats.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <comphelper/processfactory.hxx>
#include <rootfrm.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";

int CountFilesInDirectory(const OUString &rURL)
{
    int nRet = 0;

    osl::Directory aDir(rURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aDir.open());

    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        if (aFileStatus.getFileType() != osl::FileStatus::Directory)
            ++nRet;
    }

    return nRet;
}
}

class SwUiWriterTest : public SwModelTestBase, public HtmlTestTools
{

public:
    void testReplaceForward();
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    void testReplaceBackward();
    void testRedlineFrame(char const*const file);
    void testRedlineFrameAtCharStartOutside0();
    void testRedlineFrameAtCharStartOutside();
    void testRedlineFrameAtCharStartInside();
    void testRedlineFrameAtParaStartOutside();
    void testRedlineFrameAtParaEndInside();
    void testRedlineFrameAtParaOneParagraph();
    void testRedlineFrameAtPara2ndParagraph();
    void testThreadedException();
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
    void testWatermarkPosition();
    void testTdf67238();
    void testFdo75110();
    void testFdo75898();
    void testFdo74981();
    void testTdf98512();
    void testShapeTextboxSelect();
    void testShapeTextboxDelete();
    void testAnchorChangeSelection();
    void testCp1000071();
    void testShapeTextboxVertadjust();
    void testShapeTextboxAutosize();
    void testFdo82191();
    void testCommentedWord();
    void testTextFieldGetAnchorGetTextInFooter();
    void testChineseConversionBlank();
    void testChineseConversionNonChineseText();
    void testChineseConversionTraditionalToSimplified();
    void testChineseConversionSimplifiedToTraditional();
    void testFdo85554();
    void testAutoCorr();
    void testTdf83260();
    void testTdf130274();
    void testMergeDoc();
    void testCreatePortions();
    void testBookmarkUndo();
    void testFdo85876();
    void testCaretPositionMovingUp();
    void testTdf93441();
    void testTdf81226();
    void testTdf137532();
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
    void testForcepoint3();
    void testForcepoint80();
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
    void testTdf131990();
    void testTdf90808();
    void testTdf97601();
    void testTdf75137();
    void testTdf83798();
    void testTdf89714();
    void testTdf130287();
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
    void testTdf127635();
    void testDde();
    void testDocModState();
    void testTdf94804();
    void testTdf34957();
    void testTdf89954();
    void testTdf89720();
    void testTdf88986();
    void testTdf78150();
    void testTdf138873();
    void testTdf87922();
    void testTdf77014();
    void testTdf92648();
    void testTdf103978_backgroundTextShape();
    void testTdf117225();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST(testRedlineFrameAtCharStartOutside0);
    CPPUNIT_TEST(testRedlineFrameAtCharStartOutside);
    CPPUNIT_TEST(testRedlineFrameAtCharStartInside);
    CPPUNIT_TEST(testRedlineFrameAtParaStartOutside);
    CPPUNIT_TEST(testRedlineFrameAtParaEndInside);
    CPPUNIT_TEST(testRedlineFrameAtParaOneParagraph);
    CPPUNIT_TEST(testRedlineFrameAtPara2ndParagraph);
    CPPUNIT_TEST(testThreadedException);
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
    CPPUNIT_TEST(testWatermarkPosition);
    CPPUNIT_TEST(testTdf67238);
    CPPUNIT_TEST(testFdo75110);
    CPPUNIT_TEST(testFdo75898);
    CPPUNIT_TEST(testFdo74981);
    CPPUNIT_TEST(testTdf98512);
    CPPUNIT_TEST(testShapeTextboxSelect);
    CPPUNIT_TEST(testShapeTextboxDelete);
    CPPUNIT_TEST(testAnchorChangeSelection);
    CPPUNIT_TEST(testCp1000071);
    CPPUNIT_TEST(testShapeTextboxVertadjust);
    CPPUNIT_TEST(testShapeTextboxAutosize);
    CPPUNIT_TEST(testFdo82191);
    CPPUNIT_TEST(testCommentedWord);
    CPPUNIT_TEST(testTextFieldGetAnchorGetTextInFooter);
    CPPUNIT_TEST(testChineseConversionBlank);
    CPPUNIT_TEST(testChineseConversionNonChineseText);
    CPPUNIT_TEST(testChineseConversionTraditionalToSimplified);
    CPPUNIT_TEST(testChineseConversionSimplifiedToTraditional);
    CPPUNIT_TEST(testFdo85554);
    CPPUNIT_TEST(testAutoCorr);
    CPPUNIT_TEST(testTdf83260);
    CPPUNIT_TEST(testTdf130274);
    CPPUNIT_TEST(testMergeDoc);
    CPPUNIT_TEST(testCreatePortions);
    CPPUNIT_TEST(testBookmarkUndo);
    CPPUNIT_TEST(testFdo85876);
    CPPUNIT_TEST(testCaretPositionMovingUp);
    CPPUNIT_TEST(testTdf93441);
    CPPUNIT_TEST(testTdf81226);
    CPPUNIT_TEST(testTdf137532);
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
    CPPUNIT_TEST(testForcepoint3);
    CPPUNIT_TEST(testForcepoint80);
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
    CPPUNIT_TEST(testTdf131990);
    CPPUNIT_TEST(testTdf90808);
    CPPUNIT_TEST(testTdf97601);
    CPPUNIT_TEST(testTdf75137);
    CPPUNIT_TEST(testTdf83798);
    CPPUNIT_TEST(testTdf89714);
    CPPUNIT_TEST(testTdf130287);
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
    CPPUNIT_TEST(testTdf127635);
    CPPUNIT_TEST(testDde);
    CPPUNIT_TEST(testDocModState);
    CPPUNIT_TEST(testTdf94804);
    CPPUNIT_TEST(testTdf34957);
    CPPUNIT_TEST(testTdf89954);
    CPPUNIT_TEST(testTdf89720);
    CPPUNIT_TEST(testTdf88986);
    CPPUNIT_TEST(testTdf78150);
    CPPUNIT_TEST(testTdf138873);
    CPPUNIT_TEST(testTdf87922);
    CPPUNIT_TEST(testTdf77014);
    CPPUNIT_TEST(testTdf92648);
    CPPUNIT_TEST(testTdf103978_backgroundTextShape);
    CPPUNIT_TEST(testTdf117225);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<SwTextBlocks> readDOCXAutotext(
        std::u16string_view sFileName, bool bEmpty = false);
};

std::unique_ptr<SwTextBlocks> SwUiWriterTest::readDOCXAutotext(std::u16string_view sFileName, bool bEmpty)
{
    utl::TempFile tmp;
    tmp.EnableKillingFile();
    OUString rURL = tmp.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::FileBase::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + sFileName, rURL));

    SfxMedium aSrcMed(rURL, StreamMode::STD_READ);
    SwDoc* pDoc = createSwDoc();

    SwReader aReader(aSrcMed, rURL, pDoc);
    Reader* pDOCXReader = SwReaderWriter::GetDOCXReader();
    auto pGlossary = std::make_unique<SwTextBlocks>(rURL);

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

constexpr OUStringLiteral ORIGINAL_REPLACE_CONTENT(u"toto titi tutu");
constexpr OUStringLiteral EXPECTED_REPLACE_CONTENT(u"toto toto tutu");

void SwUiWriterTest::testReplaceForward()
{
    SwDoc* pDoc = createSwDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, ORIGINAL_REPLACE_CONTENT);

    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

    CPPUNIT_ASSERT_EQUAL(OUString(EXPECTED_REPLACE_CONTENT), pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(OUString(ORIGINAL_REPLACE_CONTENT), pTextNode->GetText());
}

void SwUiWriterTest::testRedlineFrame(char const*const file)
{
    SwDoc * pDoc(createSwDoc(DATA_DIRECTORY, file));
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // there is exactly one frame
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    RedlineFlags nMode = pWrtShell->GetRedlineFlags();
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    // hide delete redlines
    pWrtShell->SetRedlineFlags(nMode & ~RedlineFlags::ShowDelete);

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    pWrtShell->SetRedlineFlags(nMode); // show again

    // there is still exactly one frame
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

void SwUiWriterTest::testRedlineFrameAtCharStartOutside0()
{
    testRedlineFrame("redlineFrame.fodt");
}

void SwUiWriterTest::testRedlineFrameAtCharStartOutside()
{
    testRedlineFrame("redlineFrame_at_char_start_outside.fodt");
}

void SwUiWriterTest::testRedlineFrameAtCharStartInside()
{
    testRedlineFrame("redlineFrame_at_char_start_inside.fodt");
}

void SwUiWriterTest::testRedlineFrameAtParaStartOutside()
{
    testRedlineFrame("redline_fly_duplication_at_para_start_outside.fodt");
}

void SwUiWriterTest::testRedlineFrameAtParaEndInside()
{
    testRedlineFrame("redline_fly_duplication_at_para_end_inside.fodt");
}

void SwUiWriterTest::testRedlineFrameAtParaOneParagraph()
{
    // test ALLFLYS flag: oddly enough it didn't fail as fodt but failed as odt...
    testRedlineFrame("redline_fly_at_para_one_paragraph.odt");
}

void SwUiWriterTest::testRedlineFrameAtPara2ndParagraph()
{
    // lost via the buggy increment in Copy
    testRedlineFrame("redline_fly_duplication_at_para_2nd_paragraph.fodt");
}

void SwUiWriterTest::testThreadedException()
{
    SvFileStream aFileStream(m_directories.getURLFromSrc(DATA_DIRECTORY) + "threadedException.fodt", StreamMode::READ);

    //threaded reading only kicks in if there is sufficient buffer to make it worthwhile, so read
    //from a SvFileStream to ensure that
    bool bRes = TestImportFODT(aFileStream);

    CPPUNIT_ASSERT(!bRes);
}

void SwUiWriterTest::testBookmarkCopy()
{
    SwDoc * pDoc(createSwDoc());

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

    rIDCO.CopyRange(aPaM, target, SwCopyFlags::CheckPosInFly);

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

    rIDCO.CopyRange(aCopyPaM, *aPaM.GetPoint(), SwCopyFlags::CheckPosInFly);

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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //insert a 3X3 table in the newly created document
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(TableOpt, 3, 3);
    //checking for the rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    //selecting the table
    pWrtShell->StartOfSection();
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
    pWrtShell->StartOfSection();
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "fdo75110.odt");
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "fdo75898.odt");
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
    SwDoc* pDoc = createSwDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "toto titi tutu");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

    CPPUNIT_ASSERT_EQUAL(OUString(EXPECTED_REPLACE_CONTENT), pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(OUString(ORIGINAL_REPLACE_CONTENT), pTextNode->GetText());
}

void SwUiWriterTest::testFdo69893()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "fdo69893.odt");
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
    createSwDoc(DATA_DIRECTORY, "fdo70807.odt");

    uno::Reference<container::XIndexAccess> xStylesIter(getStyles("PageStyles"), uno::UNO_QUERY);

    for (sal_Int32 i = 0; i < xStylesIter->getCount(); ++i)
    {
        uno::Reference<style::XStyle> xStyle(xStylesIter->getByIndex(i), uno::UNO_QUERY);

        bool expectedUsedStyle = false;
        bool expectedUserDefined = false;

        OUString styleName(xStyle->getName());

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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("foobar");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);

    // Insert the RTF at the cursor position.
    OString aData = "{\\rtf1 Hello world!\\par}";
    SvMemoryStream aStream(const_cast<char*>(aData.getStr()), aData.getLength(), StreamMode::READ);
    SwReader aReader(aStream, OUString(), OUString(), *pWrtShell->GetCursor());
    Reader* pRTFReader = SwReaderWriter::GetRtfReader();
    CPPUNIT_ASSERT(pRTFReader != nullptr);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aReader.Read(*pRTFReader));

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("fooHello world!"), pDoc->GetNodes()[nIndex - 1]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), pDoc->GetNodes()[nIndex]->GetTextNode()->GetText());
}

void SwUiWriterTest::testExportRTF()
{
    // Insert "aaabbbccc" and select "bbb".
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("aaabbbccc");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 3, /*bBasicCall=*/false);

    // Create the clipboard document.
    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);
    pWrtShell->Copy(*xClpDoc);

    // And finally export it as RTF.
    WriterRef xWrt;
    SwReaderWriter::GetWriter("RTF", OUString(), xWrt);
    SvMemoryStream aStream;
    SwWriter aWrt(aStream, *xClpDoc);
    aWrt.Write(xWrt);

    OString aData(static_cast<const char*>(aStream.GetData()), aStream.GetSize());

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
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext(u"autotext-empty.dotx", true);
    CPPUNIT_ASSERT(pGlossary != nullptr);
}

void SwUiWriterTest::testDOCXAutoTextMultiple()
{
    // file contains three AutoText entries
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext(u"autotext-multiple.dotx");

    // check entries count
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pGlossary->GetCount());

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
    CPPUNIT_ASSERT(rNode.IsTextNode());
    SwTextNode& rTextNode = *rNode.GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Another "), rTextNode.GetText());

    // Make sure that autotext does not set a custom page style, leading to an unexpected page break
    // on insertion.
    // Without the accompanying fix in place, this test would have failed: the text node had an
    // attribute set containing a page style item.
    CPPUNIT_ASSERT(!rTextNode.HasSwAttrSet() || !rTextNode.GetSwAttrSet().HasItem(RES_PAGEDESC));

    // last line
    SwNodeIndex aLast(*aDocEnd.GetNode().EndOfSectionNode(), -1);
    SwNode& rLastNode = aLast.GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("complex"), rLastNode.GetTextNode()->GetText());
}

void SwUiWriterTest::testDOTMAutoText()
{
    // this is dotm file difference is that in the dotm
    // there are no empty paragraphs at the end of each entry
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext(u"autotext-dotm.dotm");

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
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext(u"autotext-gallery.dotx");

    SwDoc* pDoc = pGlossary->GetDoc();
    CPPUNIT_ASSERT(pDoc != nullptr);

    // check entries count
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pGlossary->GetCount());

    // check entry name (if not contains gallery type)
    CPPUNIT_ASSERT_EQUAL(OUString("Multiple"), pGlossary->GetLongName(0));
}

void SwUiWriterTest::testWatermarkDOCX()
{
    SwDoc* const pDoc = createSwDoc(DATA_DIRECTORY, "watermark.docx");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    const SfxPoolItem* pItem;
    SfxItemState eState = pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher()->QueryState(SID_WATERMARK, pItem);

    CPPUNIT_ASSERT(eState >= SfxItemState::DEFAULT);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(SID_WATERMARK), pItem->Which());

    const SfxWatermarkItem* pWatermark = static_cast<const SfxWatermarkItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomWatermark"), pWatermark->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("DejaVu Sans Light"), pWatermark->GetFont());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(45), pWatermark->GetAngle());
    CPPUNIT_ASSERT_EQUAL(Color(0x548dd4), pWatermark->GetColor());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), pWatermark->GetTransparency());
}

void SwUiWriterTest::testWatermarkPosition()
{
    // tdf#108494 Watermark inserted in the document with page break was outside the first page
    const int aPagesInDocument = 2;
    const int aAdditionalPagesCount[] = {    0,     0,    1,     1,    5,     5,   20,    20 };
    const bool aChangeHeader[]        = { true, false, true, false, true, false, true, false };

    for (tools::ULong i = 0; i < sizeof(aAdditionalPagesCount) / sizeof(int); ++i)
    {
        int aPages = aPagesInDocument + aAdditionalPagesCount[i];

        // Empty document with one Page Break
        SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "watermark-position.odt");
        SwEditShell* pEditShell = pDoc->GetEditShell();
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        uno::Reference<frame::XModel> xModel = pDoc->GetDocShell()->GetBaseModel();
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName("Default Page Style"), uno::UNO_QUERY);

        // 1. Add additional page breaks
        for (int j = 0; j < aAdditionalPagesCount[i]; ++j)
            pWrtShell->InsertPageBreak();

        // 2. Change header state (On, Off, On)
        if (aChangeHeader[i])
        {
            SwPageDesc aDesc(pDoc->GetPageDesc(0));
            SwFrameFormat& rMaster = aDesc.GetMaster();
            rMaster.SetFormatAttr(SwFormatHeader(true));
            pDoc->ChgPageDesc(0, aDesc);

            aDesc = pDoc->GetPageDesc(0);
            SwFrameFormat& rMaster2 = aDesc.GetMaster();
            rMaster2.SetFormatAttr(SwFormatHeader(false));
            pDoc->ChgPageDesc(0, aDesc);

            aDesc = pDoc->GetPageDesc(0);
            SwFrameFormat& rMaster3 = aDesc.GetMaster();
            rMaster3.SetFormatAttr(SwFormatHeader(true));
            pDoc->ChgPageDesc(0, aDesc);
        }

        // 3. Insert Watermark
        SfxWatermarkItem aWatermark;
        aWatermark.SetText("Watermark");
        aWatermark.SetFont("DejaVu Sans");

        pEditShell->SetWatermark(aWatermark);

        uno::Reference<css::drawing::XShape> xShape = getShape(1);
        CPPUNIT_ASSERT(xShape.is());

        SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);

        // Get Watermark object
        SdrObject* pObject = pPage->GetObj(0);
        pObject->RecalcBoundRect();
        const tools::Rectangle& rRect = pObject->GetSnapRect();
        Size rSize = pPage->GetSize();

        // Page break, calculate height of a page
        const int nPageHeight = rSize.getHeight() / aPages;

        std::stringstream aMessage;
        aMessage << "Case: " << i << ", nPageHeight = " << nPageHeight << ", rRect.Bottom = " << rRect.Bottom();

        // Check if Watermark is inside a page
        CPPUNIT_ASSERT_MESSAGE(aMessage.str(), nPageHeight >= rRect.Bottom());

        // Check if Watermark is centered
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xShape, "HoriOrient"));
        CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xShape, "VertOrient"));
    }
}

void SwUiWriterTest::testFdo74981()
{
    // create a document with an input field
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
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
        OUStringChar(CH_TXT_ATR_INPUTFIELDSTART) + "foo" + OUStringChar(CH_TXT_ATR_INPUTFIELDEND));
    OUString const expected2(
        OUStringChar(CH_TXT_ATR_INPUTFIELDSTART) + "baz" + OUStringChar(CH_TXT_ATR_INPUTFIELDEND)
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-textbox.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwDrawContact* pShape = static_cast<SwDrawContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // And finally make sure the shape got selected, not just the textbox itself.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_DRAWFRMFMT), pShape->GetFormat()->Which());
}

void SwUiWriterTest::testShapeTextboxDelete()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-textbox.odt");
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

void SwUiWriterTest::testAnchorChangeSelection()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "test_anchor_as_character.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    CPPUNIT_ASSERT_EQUAL(pObject, rMarkList.GetMark(0)->GetMarkedSdrObj());

    pWrtShell->ChgAnchor(RndStdIds::FLY_AS_CHAR);

    // tdf#125039 shape must still be selected, extensions depend on that
    CPPUNIT_ASSERT_EQUAL(pObject, rMarkList.GetMark(0)->GetMarkedSdrObj());
}

void SwUiWriterTest::testCp1000071()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "cp1000071.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL( SwRedlineTable::size_type( 2 ), rTable.size());
    SwNodeOffset redlineStart0NodeIndex = rTable[ 0 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart0Index = rTable[ 0 ]->Start()->nContent.GetIndex();
    SwNodeOffset redlineEnd0NodeIndex = rTable[ 0 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd0Index = rTable[ 0 ]->End()->nContent.GetIndex();
    SwNodeOffset redlineStart1NodeIndex = rTable[ 1 ]->Start()->nNode.GetIndex();
    sal_Int32 redlineStart1Index = rTable[ 1 ]->Start()->nContent.GetIndex();
    SwNodeOffset redlineEnd1NodeIndex = rTable[ 1 ]->End()->nNode.GetIndex();
    sal_Int32 redlineEnd1Index = rTable[ 1 ]->End()->nContent.GetIndex();

    // Change the document layout to be 2 columns, and then undo.
    pWrtShell->SelAll();
    SwSectionData section(SectionType::Content, pWrtShell->GetUniqueSectionName());
    SfxItemSet set( pDoc->GetDocShell()->GetPool(), svl::Items<RES_COL, RES_COL> );
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-textbox-vertadjust.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwFrameFormat* pFormat = static_cast<SwContact*>(pObject->GetUserCall())->GetFormat();
    // This was SDRTEXTVERTADJUST_TOP.
    CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_CENTER, pFormat->GetTextVertAdjust().GetValue());
}

void SwUiWriterTest::testShapeTextboxAutosize()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-textbox-autosize.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // 0-1 is the first UI-visible shape+textbox.
    SdrObject* pFirst = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(OUString("1st"), pFirst->GetName());

    // 2-3 is the second UI-visible shape+textbox.
    SdrObject* pSecond = pPage->GetObj(2);
    CPPUNIT_ASSERT_EQUAL(OUString("2nd"), pSecond->GetName());

    // Shape -> textbox synchronization was missing, the second shape had the
    // same height as the first, even though the first contained 1 paragraph
    // and the other 2 ones.
    CPPUNIT_ASSERT(pFirst->GetSnapRect().getHeight() < pSecond->GetSnapRect().getHeight());
}

void SwUiWriterTest::testFdo82191()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "fdo82191.odt");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // Make sure we have a single draw shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), SwTextBoxHelper::getCount(pPage));

    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrObject* pObject = pPage->GetObj(0);
    // Select it, then copy and paste.
    pWrtShell->SelectObj(Point(), 0, pObject);
    pWrtShell->Copy(aClipboard);
    pWrtShell->Paste(aClipboard);

    // This was one: the textbox of the shape wasn't copied.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), SwTextBoxHelper::getCount(*pDoc));
}

void SwUiWriterTest::testCommentedWord()
{
    // This word is commented. <- string in document
    // 123456789 <- character positions
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "commented-word.odt");
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

void SwUiWriterTest::testTextFieldGetAnchorGetTextInFooter() {
    createSwDoc(DATA_DIRECTORY, "textfield-getanchor-gettext-in-footer.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);

    OUString value = xField->getAnchor()->getText()->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("userfield_in_footer"), value );
}

// Chinese conversion tests

const sal_Unicode CHINESE_TRADITIONAL_CONTENT(0x9F8D);
const sal_Unicode CHINESE_SIMPLIFIED_CONTENT(0x9F99);
constexpr OUStringLiteral NON_CHINESE_CONTENT(u"Hippopotamus");

// Tests that a blank document is still blank after conversion
void SwUiWriterTest::testChineseConversionBlank()
{

    // Given
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
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
    createSwDoc(DATA_DIRECTORY, "fdo85554.odt");

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

    // This was 1, we lost a shape on export.
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

void SwUiWriterTest::testAutoCorr()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    const sal_Unicode cIns = ' ';

    //Normal AutoCorrect
    pWrtShell->Insert("tset");
    pWrtShell->AutoCorrect(corr, cIns);
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
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

void SwUiWriterTest::testTdf83260()
{
    SwDoc* const pDoc(createSwDoc(DATA_DIRECTORY, "tdf83260-1.odt"));
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());

    // enabled but not shown
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());
#if 0
    CPPUNIT_ASSERT(IDocumentRedlineAccess::IsHideChanges(
            pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
#endif
    CPPUNIT_ASSERT(IDocumentRedlineAccess::IsRedlineOn(
            pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(!pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());

    // the document contains redlines that are combined with CompressRedlines()
    // if that happens during AutoCorrect then indexes in Undo are off -> crash
    pWrtShell->Insert("tset");
    pWrtShell->AutoCorrect(corr, u' ');
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    auto const nActions(rUndoManager.GetUndoActionCount());
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Undo();
    }
    // check that every text node has a layout frame
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const*const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Redo();
    }
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const*const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Undo();
    }
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const*const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
}

void SwUiWriterTest::testTdf130274()
{
    SwDoc *const pDoc(createSwDoc());
    SwWrtShell *const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());

    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());
    CPPUNIT_ASSERT(!IDocumentRedlineAccess::IsRedlineOn(
            pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // "tset" may be replaced by the AutoCorrect in the test profile
    pWrtShell->Insert("tset");
    // select from left to right
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    pWrtShell->SetRedlineFlags(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    // this would crash in AutoCorrect
    pWrtShell->AutoCorrect(corr, '.');

    CPPUNIT_ASSERT(!pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());
}

void SwUiWriterTest::testMergeDoc()
{
    SwDoc* const pDoc1(createSwDoc(DATA_DIRECTORY, "merge-change1.odt"));

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
    createSwDoc(DATA_DIRECTORY, "uno-cycle.odt");
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
    SwDoc* pDoc = createSwDoc();
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

    pMarkAccess->renameMark(*ppBkmk, "Mark_");
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
    SwDoc* const pDoc = createSwDoc();
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

void SwUiWriterTest::testCaretPositionMovingUp()
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("after");
    pWrtShell->InsertLineBreak();
    pWrtShell->Up(false);
    pWrtShell->Insert("before");

    CPPUNIT_ASSERT_EQUAL(OUString(u"beforeAfter" + OUStringChar(CH_TXTATR_NEWLINE)), getParagraph(1)->getString());
}

void SwUiWriterTest::testTdf93441()
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("Hello");
    pWrtShell->InsertLineBreak();
    pWrtShell->Insert("Hello World");
    pWrtShell->Up(false);
    pWrtShell->Insert(" World");

    // Without the fix in place, this test would have failed with
    // - Expected: Hello World\nHello World
    // - Actual  :  WorldHello\nHello World
    CPPUNIT_ASSERT_EQUAL(OUString(u"Hello World" + OUStringChar(CH_TXTATR_NEWLINE) + u"Hello World"), getParagraph(1)->getString());
}

void SwUiWriterTest::testTdf81226()
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("before");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Down(false);
    pWrtShell->Insert("after");

    // Without the fix in place, this test would have failed with
    // - Expected: beforeafter
    // - Actual  : beafterfore
    CPPUNIT_ASSERT_EQUAL(OUString("beforeafter"), getParagraph(1)->getString());
}

void SwUiWriterTest::testTdf137532()
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("test");

    //Select the word and change it to bold
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    lcl_setWeight(pWrtShell, WEIGHT_BOLD);

    // Select first character and replace it
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Insert("x");

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));

    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("xest"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    xCursor.set(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xCursor->getString());

    // Without the fix in place, this test would have failed in
    // - Expected: 150
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    xCursor.set(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
}

void SwUiWriterTest::testFdo87448()
{
    createSwDoc(DATA_DIRECTORY, "fdo87448.odt");

    // Save the first shape to a metafile.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(comphelper::getProcessComponentContext());
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "OutputStream", uno::makeAny(xOutputStream) },
            { "FilterName", uno::makeAny(OUString("SVM")) }
        }));
    xGraphicExporter->filter(aDescriptor);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Read it back and dump it as an XML file.
    Graphic aGraphic;
    TypeSerializer aSerializer(aStream);
    aSerializer.readGraphic(aGraphic);
    const GDIMetaFile& rMetaFile = aGraphic.GetGDIMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, rMetaFile);

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
    createSwDoc();
    SwDoc* pDoc = createSwDoc();
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
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, false, false);
    // must be disposed after deleting header
    // cursor ends up in body
    // UPDATE: this behaviour has been corrected as a side effect of the fix to tdf#46561:
    //CPPUNIT_ASSERT_THROW(xCursor->goRight(1, false), uno::RuntimeException);
}

void SwUiWriterTest::testTdf68183()
{
    // First disable RSID and check if indeed no such attribute is inserted.
    SwDoc* pDoc = createSwDoc();
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
    createSwDoc(DATA_DIRECTORY, "cp1000115.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
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
    SwDoc* pDoc = createSwDoc();
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
    createSwDoc(DATA_DIRECTORY, "tdf90003.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // This was 1: an unexpected fly portion was created, resulting in too
    // large x position for the empty paragraph marker.
    assertXPath(pXmlDoc, "//Special[@nType='PortionType::Fly']", 0);
}

void SwUiWriterTest::testTdf51741()
{
    SwDoc* pDoc = createSwDoc();
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
    pMarkAccess->renameMark(*ppBkmk, "Mark_");
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
    const uno::Sequence<beans::PropertyValues> aPropVal(xDefNum->getDefaultContinuousNumberingLevels(alocale));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aPropVal.getLength());
    for(const auto& rPropValues : aPropVal)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rPropValues.getLength());
        for(const auto& rPropVal : rPropValues)
        {
            uno::Any aAny = rPropVal.Value;
            if(rPropVal.Name == "Prefix" || rPropVal.Name == "Suffix" || rPropVal.Name == "Transliteration")
                CPPUNIT_ASSERT_EQUAL(OUString("string"), aAny.getValueTypeName());
            else if(rPropVal.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(OUString("short"), aAny.getValueTypeName());
            else if(rPropVal.Name == "NatNum")
                CPPUNIT_ASSERT_EQUAL(OUString("short"), aAny.getValueTypeName());
                //It is expected to be long but right now its short !error!
            else
                CPPUNIT_FAIL("Property Name not matched");
        }
    }
}

void SwUiWriterTest::testDeleteTableRedlines()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(TableOpt, 1, 3);
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    uno::Sequence<beans::PropertyValue> aDescriptor;
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("A1"))), u"TableCellInsert", aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("B1"))), u"TableCellInsert", aDescriptor);
    SwUnoCursorHelper::makeTableCellRedline((*const_cast<SwTableBox*>(rTable.GetTableBox("C1"))), u"TableCellInsert", aDescriptor);
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwExtraRedlineTable& rExtras = rIDRA.GetExtraRedlineTable();
    rExtras.DeleteAllTableRedlines(*pDoc, rTable, false, RedlineType::Any);
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(0), rExtras.GetSize());
}

void SwUiWriterTest::testXFlatParagraph()
{
    SwDoc* pDoc = createSwDoc();
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
    //checking "checked" status, modifying it and asserting the changes
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
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
         { "CharWeight", uno::Any(float(css::awt::FontWeight::BOLD)) }
    }));
    xFlatPara3->changeAttributes(sal_Int32(0), sal_Int32(5), aDescriptor);
    //checking Language Portions
    uno::Sequence<::sal_Int32> aLangPortions(xFlatPara4->getLanguagePortions());
    CPPUNIT_ASSERT(!aLangPortions.hasElements());
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
    const uno::Sequence<uno::Reference<container::XIndexAccess>> aIndexAccess(xDefNum->getDefaultOutlineNumberings(alocale));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aIndexAccess.getLength());
    for(const auto& rIndexAccess : aIndexAccess)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rIndexAccess->getCount());
        for(int j=0;j<rIndexAccess->getCount();j++)
        {
            uno::Sequence<beans::PropertyValue> aProps;
            rIndexAccess->getByIndex(j) >>= aProps;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aProps.getLength());
            for(const beans::PropertyValue& rProp : std::as_const(aProps))
            {
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

void SwUiWriterTest::testForcepoint3()
{
    createSwDoc(DATA_DIRECTORY, "flowframe_null_ptr_deref.sample");
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
    }));
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    // printing asserted in SwFrame::GetNextSctLeaf()
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    aTempFile.EnableKillingFile();
}

void SwUiWriterTest::testForcepoint80()
{
    try
    {
        createSwDoc(DATA_DIRECTORY, "forcepoint80-1.rtf");
        uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
        }));
        utl::TempFile aTempFile;
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        // printing asserted in SwCellFrame::FindStartEndOfRowSpanCell
        xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
        aTempFile.EnableKillingFile();
    }
    catch(...)
    {
    }
}

void SwUiWriterTest::testExportToPicture()
{
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aFilterData( comphelper::InitPropertySequence({
        { "PixelWidth", uno::Any(sal_Int32(610)) },
        { "PixelHeight", uno::Any(sal_Int32(610)) }
    }));
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_png_Export")) },
        { "FilterData", uno::Any(aFilterData) }
    }));
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
    createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //Getting some paragraph style
    SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName(u"Text Body");
    const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
    std::unique_ptr<SfxItemSet> pNewSet = rAttrSet.Clone();
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
    SwTextFormatColl* pTextFormat2 = pDoc->FindTextFormatCollByName(u"Text Body");
    const SwAttrSet& rAttrSet2 = pTextFormat2->GetAttrSet();
    const SvxAdjustItem& rAdjustItem2 = rAttrSet2.GetAdjust();
    SvxAdjust Adjust2 = rAdjustItem2.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust2);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat3 = pDoc->FindTextFormatCollByName(u"Text Body");
    const SwAttrSet& rAttrSet3 = pTextFormat3->GetAttrSet();
    const SvxAdjustItem& rAdjustItem3 = rAttrSet3.GetAdjust();
    SvxAdjust Adjust3 = rAdjustItem3.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust3);
    //Redo the changes
    rUndoManager.Redo();
    SwTextFormatColl* pTextFormat4 = pDoc->FindTextFormatCollByName(u"Text Body");
    const SwAttrSet& rAttrSet4 = pTextFormat4->GetAttrSet();
    const SvxAdjustItem& rAdjustItem4 = rAttrSet4.GetAdjust();
    SvxAdjust Adjust4 = rAdjustItem4.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust4);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat5 = pDoc->FindTextFormatCollByName(u"Text Body");
    const SwAttrSet& rAttrSet5 = pTextFormat5->GetAttrSet();
    const SvxAdjustItem& rAdjustItem5 = rAttrSet5.GetAdjust();
    SvxAdjust Adjust5 = rAdjustItem5.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust5);
}

void SwUiWriterTest::testTextSearch()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createSwDoc();
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
    uno::Reference<util::XSearchDescriptor> xSearchDes = xSearch->createSearchDescriptor();
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);
    //setting some properties
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
        { "CharWeight", uno::Any(float(css::awt::FontWeight::BOLD)) }
    }));
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
    uno::Reference<util::XReplaceDescriptor> xReplaceDes = xReplace->createReplaceDescriptor();
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
    // regex tests
    xSearchDes->setPropertyValue("SearchRegularExpression", uno::makeAny(true));
    // regex: test correct matching combined with attributes like BOLD
    xSearchDes->setSearchString(".*"); // should match all bold words in the text
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndex->getCount());
    uno::Reference<text::XTextRange> xFound(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFound->getString());
    xFound.set(xIndex->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("This"), xFound->getString());
    xFound.set(xIndex->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xFound->getString());
    // regex: test anchor combined with attributes like BOLD
    xSearchDes->setSearchString("^.*|.*$"); // should match first and last words (they are bold)
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndex->getCount());
    xFound.set(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFound->getString());
    xFound.set(xIndex->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xFound->getString());
    // regex: test look-ahead/look-behind assertions outside of the bold text
    xSearchDes->setSearchString("(?<= ).*(?= )"); // should match second bold word
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndex->getCount());
    xFound.set(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("This"), xFound->getString());
    xReplaceDes->setPropertyValue("SearchRegularExpression", uno::makeAny(true));
    // regex: test correct match of paragraph start
    xReplaceDes->setSearchString("^."); // should only match first character of the paragraph
    xReplaceDes->setReplaceString("C");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: test correct match of word start
    xReplaceDes->setSearchString("\\b\\w"); // should match all words' first characters
    xReplaceDes->setReplaceString("x&");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), ReplaceCount);
    // regex: test negative look-behind assertion
    xReplaceDes->setSearchString("(?<!xCelly xW)o"); // only "o" in "xCello", not in "xWorld"
    xReplaceDes->setReplaceString("y");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: test positive look-behind assertion
    xReplaceDes->setSearchString("(?<=xCelly xWorld xTh)i"); // only "i" in "xThis", not in "xis"
    xReplaceDes->setReplaceString("z");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: use capturing group to test reference
    xReplaceDes->setSearchString("\\b(\\w\\w\\w\\w)\\w");
    xReplaceDes->setReplaceString("$1q"); // only fifth characters in words should change
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), ReplaceCount);
    // check of the end result
    CPPUNIT_ASSERT_EQUAL(OUString("xCelqy xWorqd xThzq xis xa xtasq"),
                         pCursor->GetNode().GetTextNode()->GetText());
    // regex: use positive look-ahead assertion
    xReplaceDes->setSearchString("Wor(?=qd xThzq xis xa xtasq)");
    xReplaceDes->setReplaceString("&p"); // testing & reference
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: use negative look-ahead assertion
    xReplaceDes->setSearchString("x(?!Worpqd xThzq xis xa xtasq)");
    xReplaceDes->setReplaceString("m");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ReplaceCount); // one of the 6 "x" must not be replaced
    // check of the end result
    CPPUNIT_ASSERT_EQUAL(OUString("mCelqy xWorpqd mThzq mis ma mtasq"),
                         pCursor->GetNode().GetTextNode()->GetText());
}

void SwUiWriterTest::testTdf69282()
{
    SwDoc* source = createSwDoc();
    uno::Reference<lang::XComponent> xSourceDoc = mxComponent;
    mxComponent.clear();
    SwDoc* target = createSwDoc();
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
    source->ChgPageDesc("SourceStyle", *sPageDesc);
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
    SwDoc* source = createSwDoc();
    uno::Reference<lang::XComponent> xSourceDoc = mxComponent;
    mxComponent.clear();
    SwDoc* target = createSwDoc();
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
    source->ChgPageDesc("SourceStyle", *sPageDesc);
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
    ErrCode filter = aMatcher.DetectFilter(aMedium, pFilter);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_IO_ABORT, filter);
    //it should not return any Filter
    CPPUNIT_ASSERT(!pFilter);
    //testing without service type and any .ods file
    SfxMedium aMedium2(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher2;
    std::shared_ptr<const SfxFilter> pFilter2;
    ErrCode filter2 = aMatcher2.DetectFilter(aMedium2, pFilter2);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, filter2);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("calc8"), pFilter2->GetFilterName());
    //testing with service type and any .odt file
    OUString path2 = m_directories.getURLFromSrc(DATA_DIRECTORY) + "fdo69893.odt";
    SfxMedium aMedium3(path2, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher3("com.sun.star.text.TextDocument");
    std::shared_ptr<const SfxFilter> pFilter3;
    ErrCode filter3 = aMatcher3.DetectFilter(aMedium3, pFilter3);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, filter3);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("writer8"), pFilter3->GetFilterName());
}

void SwUiWriterTest::testUnoParagraph()
{
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
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
    CPPUNIT_ASSERT_EQUAL(OUString(u"is text" + OUStringChar(CH_TXTATR_NEWLINE) + u"more"), pCursor->GetText());
    //Apply a *Bold* attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    SfxItemSet aSet( pDoc->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
    //Add selected text's attributes to aSet
    pCursor->GetNode().GetTextNode()->GetParaAttr(aSet, 5, 12);
    SfxPoolItem const * pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    //Check that bold is active on the selection and it's in aSet
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem == aWeightItem));
    //Make selection to remove formatting in first paragraph
    //[this is text
    //]more text
    pWrtShell->StartOfSection();
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
    //In case of Regression RstTextAttr() call will result to infinite recursion
    //Check that bold is removed in first paragraph
    aSet.ClearItem();
    pTextNode->GetParaAttr(aSet, 5, 12);
    SfxPoolItem const * pPoolItem2 = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem2 != aWeightItem));
}

void SwUiWriterTest::testTdf60967()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, true, true);
    //Inserting table
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //getting the cursor's position just after the table insert
    SwPosition aPosAfterTable(*(pCursor->GetPoint()));
    //moving cursor to B2 (bottom right cell)
    pCursor->Move(fnMoveBackward);
    SwPosition aPosInTable(*(pCursor->GetPoint()));
    //deleting paragraph following table with Ctrl+Shift+Del
    bool val = pWrtShell->DelToEndOfSentence();
    CPPUNIT_ASSERT_EQUAL(true, val);
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
    SwDoc* pDoc = createSwDoc();
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
    CPPUNIT_ASSERT_EQUAL(0,static_cast<int>(case1));
    SearchOpt.searchString = "paragraph";
    SearchOpt.transliterateFlags = TransliterationFlags::IGNORE_KASHIDA_CTL;
    //transliteration option set so that all search strings are found
    sal_uLong case2 = pWrtShell->SearchPattern(SearchOpt,true,SwDocPositions::Start,SwDocPositions::End);
    pShellCursor = pWrtShell->getShellCursor(true);
    CPPUNIT_ASSERT_EQUAL(OUString("paragraph"),pShellCursor->GetText());
    CPPUNIT_ASSERT_EQUAL(1,static_cast<int>(case2));
}

void SwUiWriterTest::testTdf73660()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aData1 = "First" + OUStringChar(CHAR_SOFTHYPHEN) + "Word";
    OUString aData2 = "Seco" + OUStringChar(CHAR_SOFTHYPHEN) + "nd";
    OUString aData3 = OUStringChar(CHAR_SOFTHYPHEN) + "Third";
    OUString aData4 = "Fourth" + OUStringChar(CHAR_SOFTHYPHEN);
    pWrtShell->Insert("We are inserting some text in the document to check the search feature ");
    pWrtShell->Insert(aData1 + " ");
    pWrtShell->Insert(aData2 + " ");
    pWrtShell->Insert(aData3 + " ");
    pWrtShell->Insert(aData4 + " ");
    pWrtShell->Insert("Fifth ");
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting first footnote
    pWrtShell->InsertFootnote("");
    SwFieldType* pField = pWrtShell->GetFieldType(0, SwFieldIds::GetRef);
    SwGetRefFieldType* pRefType = static_cast<SwGetRefFieldType*>(pField);
    //moving cursor to the starting of document
    pWrtShell->StartOfSection();
    //inserting reference field 1
    SwGetRefField aField1(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(0), REF_CONTENT);
    pWrtShell->Insert(aField1);
    //inserting second footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aField2(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(1), REF_CONTENT);
    pWrtShell->Insert(aField2);
    //inserting third footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aField3(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(2), REF_CONTENT);
    pWrtShell->Insert(aField3);
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);
    xClpDoc->getIDocumentFieldsAccess().LockExpFields();
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
    pWrtShell->Copy(*xClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking that the footnotes reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->StartOfSection();
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
    pWrtShell->EndOfSection();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
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
    pWrtShell->StartOfSection();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting sequence field 1
    SwSetExpFieldType* pSeqType = static_cast<SwSetExpFieldType*>(pWrtShell->GetFieldType(SwFieldIds::SetExp, "Illustration"));
    SwSetExpField aSetField1(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField1);
    SwGetRefFieldType* pRefType = static_cast<SwGetRefFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::GetRef));
    //moving cursor to the starting of document
    pWrtShell->StartOfSection();
    //inserting reference field 1
    SwGetRefField aGetField1(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(0), REF_CONTENT);
    pWrtShell->Insert(aGetField1);
    //now we have ref1-seq1
    //moving the cursor
    pCursor->Move(fnMoveForward);
    //inserting sequence field 2
    SwSetExpField aSetField2(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField2);
    //moving the cursor
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aGetField2(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(1), REF_CONTENT);
    pWrtShell->Insert(aGetField2);
    //now we have ref1-ref2-seq1-seq2
    //moving the cursor
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting sequence field 3
    SwSetExpField aSetField3(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->Insert(aSetField3);
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aGetField3(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(2), REF_CONTENT);
    pWrtShell->Insert(aGetField3);
    //now after insertion we have ref1-ref2-ref3-seq1-seq2-seq3
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    rtl::Reference<SwDoc> xClpDoc( new SwDoc() );
    xClpDoc->SetClipBoard(true);
    xClpDoc->getIDocumentFieldsAccess().LockExpFields();
    //selecting reference field 2 and 3 and sequence field 1 and 2
    //selection is such that more than one and not all sequence fields and reference fields are selected
    //ref1-[ref2-ref3-seq1-seq2]-seq3
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //start marking
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //copying the selection to clipboard
    pWrtShell->Copy(*xClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking whether the sequence and reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->StartOfSection();
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
    pWrtShell->EndOfSection();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
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
    pWrtShell->StartOfSection();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
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
    createSwDoc();
    //exporting the empty document to ODT via TempFile
    uno::Sequence<beans::PropertyValue> aDescriptor;
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());
    //loading an XML DOM of the "styles.xml" of the TempFile
    xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(),"styles.xml");
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //testing autocorrect of initial capitals on start of first paragraph
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    //Inserting one all-lowercase word into the first paragraph
    pWrtShell->Insert("testing");
    const sal_Unicode cChar = ' ';
    pWrtShell->AutoCorrect(corr, cChar);
    //The word should be capitalized due to autocorrect
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("Testing "), static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest::testTdf80663()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 2x2 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the first row
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 1x1 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 1, 1);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one row before the existing row
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
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

void SwUiWriterTest::testTdf131990()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT(!pWrtShell->Up( false, 1, true ));
    CPPUNIT_ASSERT(!pWrtShell->Down( false, 1, true ));
}

void SwUiWriterTest::testTdf90808()
{
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
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
    createSwDoc(DATA_DIRECTORY, "tdf97601.odt");
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(true);
    pWrtShell->InsertFootnote("This is first footnote");
    SwNodeOffset firstIndex = pShellCursor->GetNode().GetIndex();
    pShellCursor->GotoFootnoteAnchor();
    pWrtShell->InsertFootnote("This is second footnote");
    pWrtShell->Up(false);
    SwNodeOffset secondIndex = pShellCursor->GetNode().GetIndex();
    pWrtShell->Down(false);
    SwNodeOffset thirdIndex = pShellCursor->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(firstIndex, thirdIndex);
    CPPUNIT_ASSERT(firstIndex != secondIndex);
}

void SwUiWriterTest::testTdf83798()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf83798.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->Move(fnMoveBackward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents"), pCursor->GetText());
    pCursor->Move(fnMoveForward, GoInContent);
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
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xInterface(xFact->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertyState> xPropState(xInterface, uno::UNO_QUERY);
    //enabled Paragraph Orphan and Widows by default starting in LO5.1
    CPPUNIT_ASSERT_EQUAL( uno::makeAny(sal_Int8(2)), xPropState->getPropertyDefault("ParaOrphans") );
    CPPUNIT_ASSERT_EQUAL( uno::makeAny(sal_Int8(2)), xPropState->getPropertyDefault("ParaWidows")  );
}

void SwUiWriterTest::testTdf130287()
{
    //create a new writer document
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //insert a 1-cell table in the newly created document
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 1, 1);
    //checking for the row and column
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCellText);
    // they were 2 (orphan/widow control enabled unnecessarily in Table Contents paragraph style)
    CPPUNIT_ASSERT_EQUAL( sal_Int8(0), getProperty<sal_Int8>(xParagraph, "ParaOrphans"));
    CPPUNIT_ASSERT_EQUAL( sal_Int8(0), getProperty<sal_Int8>(xParagraph, "ParaWidows"));
}

void SwUiWriterTest::testPropertyDefaults()
{
    createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 3, 3); //Inserting Table
    //Checking Rows and Columns of Inserted Table
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    pWrtShell->StartOfSection();
    pWrtShell->SelTableRow(); //Selecting First Row
    pWrtShell->ClearMark();
    //Modifying the color of Table Box
    pWrtShell->SetBoxBackground(SvxBrushItem(Color(sal_Int32(0xFF00FF)), sal_Int16(RES_BACKGROUND)));
    //Checking cells for background color only A1 should be modified
    uno::Reference<table::XCell> xCell;
    xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(Color(0xFF00FF), Color(ColorTransparency, getProperty<sal_Int32>(xCell, "BackColor")));
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
    createSwDoc();
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Reference<beans::XPropertyContainer> xUserProps = xProps->getUserDefinedProperties();
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
    xPropSet->setPropertyValue("NumberFormat", uno::makeAny(key));
    //Inserting Text Content
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
    xText->insertTextContent(xTextRange, xTextField, true);
    //Retrieving the contents for verification
    CPPUNIT_ASSERT_EQUAL(OUString("11/10/14 03:03 AM"), xTextField->getPresentation(false));
}

void SwUiWriterTest::testTdf90362()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf90362.fodt");
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
    SwDoc * pDoc(createSwDoc());
    sw::UndoManager & rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell * pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>);
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END-1>);
    rIDCO.InsertString(*pShell->GetCursor(), "foo");
    pShell->ClearMark();
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    GraphicObject grf;
    pShell->SttEndDoc(true);
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet));
    pShell->SttEndDoc(false);
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    SvxCharHiddenItem hidden(true, RES_CHRATR_HIDDEN);
    pShell->SelectText(1, 4);
    rIDCO.InsertPoolItem(*pShell->GetCursor(), hidden);
    // now we have "\1foo\1" with the "foo" hidden
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    SfxPoolItem const* pItem;
    SfxItemSet query(pDoc->GetAttrPool(), svl::Items<RES_CHRATR_HIDDEN, RES_CHRATR_HIDDEN>);
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the start
    pShell->SelectText(0, 4);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the end
    pShell->SelectText(1, 5);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)), pShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
}

void SwUiWriterTest::testUndoCharAttribute()
{
    // Create a new empty Writer document
    SwDoc* pDoc = createSwDoc();
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
    SfxItemSet aSet( pDoc->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
    // Adds selected text's attributes to aSet
    pCursor->GetNode().GetTextNode()->GetParaAttr(aSet, 10, 19);
    SfxPoolItem const * pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    // Check that bold is active on the selection; checks if it's in aSet
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem == aWeightItem));
    // Invoke Undo
    rUndoManager.Undo();
    // Check that bold is no longer active
    aSet.ClearItem(RES_CHRATR_WEIGHT);
    pCursor->GetNode().GetTextNode()->GetParaAttr(aSet, 10, 19);
    pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(false, (*pPoolItem == aWeightItem));
}

void SwUiWriterTest::testUndoDelAsChar()
{
    SwDoc * pDoc(createSwDoc());
    sw::UndoManager & rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell * pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>);
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END-1>);
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    GraphicObject grf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), grf, &frameSet, &grfSet));
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf86639.rtf");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextFormatColl* pColl = pDoc->FindTextFormatCollByName(u"Heading");
    pWrtShell->SetTextFormatColl(pColl);
    OUString aExpected = pColl->GetAttrSet().GetFont().GetFamilyName();
    // This was Calibri, should be Liberation Sans.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

void SwUiWriterTest::testTdf90883TableBoxGetCoordinates()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf90883.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(true);
    SwSelBoxes aBoxes;
    ::GetTableSel( *pWrtShell, aBoxes );
    CPPUNIT_ASSERT_EQUAL( 2, static_cast<int>(aBoxes.size()) );
    Point pos ( aBoxes[0]->GetCoordinates() );
    CPPUNIT_ASSERT_EQUAL( 1, static_cast<int>(pos.X()) );
    CPPUNIT_ASSERT_EQUAL( 1, static_cast<int>(pos.Y()) );
    pos = aBoxes[1]->GetCoordinates();
    CPPUNIT_ASSERT_EQUAL( 1, static_cast<int>(pos.X()) );
    CPPUNIT_ASSERT_EQUAL( 2, static_cast<int>(pos.Y()) );
}

void SwUiWriterTest::testEmbeddedDataSource()
{
    // Initially no data source.
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(xComponentContext);
    CPPUNIT_ASSERT(!xDatabaseContext->hasByName("calc-data-source"));

    // Load: should have a component and a data source, too.
    // Path with "#" must not cause issues
    createSwDoc(OUStringConcatenation(DATA_DIRECTORY + OUString::Concat(u"hash%23path/")), "embedded-data-source.odt");
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1.
    uno::Reference<sdbc::XDataSource> xDataSource(xDatabaseContext->getByName("calc-data-source"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    auto xConnection = xDataSource->getConnection("", "");
    uno::Reference<container::XNameAccess> xTables =
        css::uno::Reference<css::sdbcx::XTablesSupplier>(
            xConnection, uno::UNO_QUERY_THROW)->getTables();
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
    xConnection = xDataSource->getConnection("", "");
    xTables =
        css::uno::Reference<css::sdbcx::XTablesSupplier>(
            xConnection, uno::UNO_QUERY_THROW)->getTables();
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "draw-anchor-undo.odt");
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

void SwUiWriterTest::testTdf127635()
{
    SwDoc* pDoc = createSwDoc();

    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, ' ', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'b', 0);
    Scheduler::ProcessEventsToIdle();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);

    //Select 'a'
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, ' ', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'd', 0);
    Scheduler::ProcessEventsToIdle();

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // accept all redlines
    while(pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // Without the fix in place, this test would have failed with
    // - Expected: C d b
    // - Actual  : Cd  b
    CPPUNIT_ASSERT_EQUAL(OUString("C d b"), getParagraph(1)->getString());
}

void SwUiWriterTest::testDde()
{
#if HAVE_FEATURE_UI
    // Type asdf and copy it.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("asdf");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    uno::Sequence<beans::PropertyValue> aPropertyValues;
    dispatchCommand(mxComponent, ".uno:Copy", aPropertyValues);

    // Go before the selection and paste as a DDE link.
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::LINK))}
    });
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    // Make sure that the document starts with a field now, and its expanded string value contains asdf.
    const uno::Reference< text::XTextRange > xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    CPPUNIT_ASSERT(xField->getString().endsWith("asdf"));
#endif
}

namespace {

//IdleTask class to add a low priority Idle task
class IdleTask
{
    public:
    bool GetFlag() const;
    IdleTask();
    DECL_LINK( FlipFlag, Timer *, void );
    private:
    bool flag;
    Idle maIdle { "sw uiwriter IdleTask" };
};

}

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
bool IdleTask::GetFlag() const
{
    //returning the status of current flag
    return flag;
}

//Callback function of IdleTask Class
IMPL_LINK(IdleTask, FlipFlag, Timer*, , void)
{
    //setting the flag to make sure that low priority idle task has been dispatched
    flag = true;
}

void SwUiWriterTest::testDocModState()
{
    //creating a new writer document via the XDesktop(to have more shells etc.)
    SwDoc* pDoc = createSwDoc();
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
    SwDoc* pDoc = createSwDoc();
    //get cursor for making bookmark at a particular location
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCursor();
    IDocumentMarkAccess* pIDMAccess(pDoc->getIDocumentMarkAccess());
    //make first bookmark, CROSSREF_HEADING, with *empty* name
    sw::mark::IMark* pMark1(pIDMAccess->makeMark(*pCrsr, "",
            IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK,
            ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    OUString bookmark1name = pMark1->GetName();
    //match the bookmark name, it should be like "__RefHeading__**"
    CPPUNIT_ASSERT(bookmark1name.match("__RefHeading__"));
    //make second bookmark, CROSSREF_NUMITEM, with *empty* name
    sw::mark::IMark* pMark2(pIDMAccess->makeMark(*pCrsr, "",
            IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK,
            ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    OUString bookmark2name = pMark2->GetName();
    //match the bookmark name, it should be like "__RefNumPara__**"
    CPPUNIT_ASSERT(bookmark2name.match("__RefNumPara__"));
}

void SwUiWriterTest::testUnicodeNotationToggle()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "unicodeAltX.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString sOriginalDocString;
    OUString sDocString;
    OUString sExpectedString;
    uno::Sequence<beans::PropertyValue> aPropertyValues;

    pWrtShell->EndPara();
    sOriginalDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("uU+002b"), sOriginalDocString);

    dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sExpectedString = "u+";
    sDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL( sDocString, sExpectedString );

    dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sDocString = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL( sDocString, sOriginalDocString );
}

void SwUiWriterTest::testTdf34957()
{
    createSwDoc(DATA_DIRECTORY, "tdf34957.odt");
    // table with "keep with next" always started on a new page if the table was large,
    // regardless of whether it was already kept with the previous paragraph,
    // or whether the following paragraph actually fit on the same page (MAB 3.6 - 5.0)
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[2]/body/tab[1]/row[2]/cell[1]/txt") );
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[4]/body/tab[1]/row[2]/cell[1]/txt") );
}

void SwUiWriterTest::testTdf89954()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf89954.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 't', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'e', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 's', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 't', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '.', 0);
    Scheduler::ProcessEventsToIdle();

    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    // Placeholder character for the comment anchor was ^A (CH_TXTATR_BREAKWORD), not <fff9> (CH_TXTATR_INWORD).
    // As a result, autocorrect did not turn the 't' input into 'T'.
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"Tes\uFFF9t. Test."), aNodeIndex.GetNode().GetTextNode()->GetText());
}

void SwUiWriterTest::testTdf89720()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf89720.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (std::unique_ptr<SwSidebarItem> const & pItem : *pPostItMgr)
    {
        if (pItem->mpPostIt->IsFollow())
            // This was non-0: reply comments had a text range overlay,
            // resulting in unexpected dark color.
            CPPUNIT_ASSERT(!pItem->mpPostIt->TextRange());
    }
}

void SwUiWriterTest::testTdf88986()
{
    // Create a text shell.
    SwDoc* pDoc = createSwDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwTextShell aShell(*pView);

    // Create the item set that is normally passed to the insert frame dialog.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    SfxItemSet aSet = aShell.CreateInsertFrameItemSet(aMgr);

    // This was missing along with the gradient and other tables.
    CPPUNIT_ASSERT(aSet.HasItem(SID_COLOR_TABLE));
}

void SwUiWriterTest::testTdf78150()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("foobar");

    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:DelToStartOfWord", {});

    // Without the fix, test fails with:
    // equality assertion failed
    //  - Expected:
    //  - Actual  : f
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

void SwUiWriterTest::testTdf138873()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("A B C");

    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());

    // Select B
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    pWrtShell->Insert("DDD");

    CPPUNIT_ASSERT_EQUAL(OUString("A DDD C"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());

    // Select B and C
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: A B C
    // - Actual  : A  CB CB
    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());
}

void SwUiWriterTest::testTdf87922()
{
    // Create an SwDrawTextInfo.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf87922.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwScriptInfo* pScriptInfo = nullptr;
    // Get access to the single paragraph in the document.
    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    const OUString& rText = aNodeIndex.GetNode().GetTextNode()->GetText();
    sal_Int32 nLength = rText.getLength();
    SwDrawTextInfo aDrawTextInfo(pWrtShell, *pWrtShell->GetOut(), pScriptInfo, rText, TextFrameIndex(0), TextFrameIndex(nLength));
    // Root -> page -> body -> text.
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    aDrawTextInfo.SetFrame(pTextFrame);

    // If no color background color is found, assume white.
    Color* pColor = sw::GetActiveRetoucheColor();
    *pColor = COL_WHITE;

    // Make sure that automatic color on black background is white, not black.
    vcl::Font aFont;
    aDrawTextInfo.ApplyAutoColor(&aFont);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aFont.GetColor());
}

#if HAVE_MORE_FONTS
namespace {

struct PortionItem
{
    PortionItem(OUString const & sItemType, sal_Int32 nLength,
                PortionType nTextType)
        : msItemType(sItemType)
        , mnLength(nLength)
        , mnTextType(nTextType)
    {}

    OUString msItemType;
    sal_Int32 mnLength;
    PortionType mnTextType;
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

    virtual void Text(TextFrameIndex nLength, PortionType nType,
                      sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/) override
    {
        mPortionItems.emplace_back("text", sal_Int32(nLength), nType);
    }

    virtual void Special(TextFrameIndex nLength, const OUString & /*rText*/,
                         PortionType nType, sal_Int32 /*nHeight*/,
                         sal_Int32 /*nWidth*/, const SwFont* /*pFont*/) override
    {
        mPortionItems.emplace_back("special", sal_Int32(nLength), nType);
    }

    virtual void LineBreak(sal_Int32 /*nWidth*/) override
    {
        mPortionItems.emplace_back("line_break", 0, PortionType::NONE);
    }

    virtual void Skip(TextFrameIndex nLength) override
    {
        mPortionItems.emplace_back("skip", sal_Int32(nLength), PortionType::NONE);
    }

    virtual void Finish() override
    {
        mPortionItems.emplace_back("finish", 0, PortionType::NONE);
    }
};

}
#endif

void SwUiWriterTest::testTdf77014()
{
#if HAVE_MORE_FONTS
    // The problem described in the bug tdf#77014 is that the input
    // field text ("ThisIsAllOneWord") is broken up on linebreak, but
    // it should be in one piece (like normal text).

    // This test checks that the input field is in one piece and if the
    // input field has more words, it is broken up at the correct place.

    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf77014.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());

    PortionHandler aHandler;
    pTextFrame->VisitPortions(aHandler);

    {
        // Input Field - "One Two Three Four Five" = 25 chars
        CPPUNIT_ASSERT_EQUAL(OUString("text"),          aHandler.mPortionItems[0].msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25),             aHandler.mPortionItems[0].mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField,   aHandler.mPortionItems[0].mnTextType);

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
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField,  aHandler.mPortionItems[0].mnTextType);

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
        CPPUNIT_ASSERT_EQUAL(PortionType::Text,   rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field: "ThisIsAllOneWord" = 18 chars
        // which is 16 chars + 2 hidden chars (start & end input field) = 18 chars
        // If this is correct then the input field is in one piece
        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(18),            rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField,  rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),        rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text,   rPortionItem.mnTextType);

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

        for (const auto& rPortionItem : aHandler.mPortionItems)
        {
            printf ("-- Type: %s length: %" SAL_PRIdINT32 " text type: %d\n",
                        rPortionItem.msItemType.toUtf8().getStr(),
                        rPortionItem.mnLength,
                        sal_uInt16(rPortionItem.mnTextType));
        }

        // Text "The purpose of this report is to summarize the results of the existing bug in the LO suite"
        // 91 chars
        auto& rPortionItem = aHandler.mPortionItems[0];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(91),       rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text,   rPortionItem.mnTextType);

        // The input field here has more words ("One Two Three Four Five")
        // and it should break after "Two".
        // Input Field: "One Two" = 7 chars + 1 start input field hidden character = 8 chars
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8),             rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField,  rPortionItem.mnTextType);

        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),     rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),         rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Hole,    rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field:  "Three Four Five" = 16 chars + 1 end input field hidden character = 16 chars
        rPortionItem = aHandler.mPortionItems[4];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),         rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16),            rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField,  rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[5];
        CPPUNIT_ASSERT_EQUAL(OUString("text"),    rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),        rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text,   rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[6];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        rPortionItem = aHandler.mPortionItems[7];
        CPPUNIT_ASSERT_EQUAL(OUString("finish"), rPortionItem.msItemType);
    }
#endif
}

void SwUiWriterTest::testTdf92648()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf92648.docx");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // Make sure we have ten draw shapes.
    // Yes, we have if the left/right pages have different header/footer,
    // but if not we have only nine of them:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), SwTextBoxHelper::getCount(pPage));
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf103978_backgroundTextShape.docx");

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

void SwUiWriterTest::testTdf117225()
{
    // Test that saving a document with an embedded object does not leak
    // tempfiles in the directory of the target file.
    OUString aTargetDirectory = m_directories.getURLFromWorkdir(u"/CppunitTest/sw_uiwriter.test.user/");
    OUString aTargetFile = aTargetDirectory + "tdf117225.odt";
    OUString aSourceFile = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf117225.odt";
    osl::File::copy(aSourceFile, aTargetFile);
    mxComponent = loadFromDesktop(aTargetFile);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    int nExpected = CountFilesInDirectory(aTargetDirectory);
    xStorable->store();
    int nActual = CountFilesInDirectory(aTargetDirectory);
    // nActual was nExpected + 1, i.e. we leaked a tempfile.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);

    OUString aTargetFileSaveAs = aTargetDirectory + "tdf117225-save-as.odt";
    xStorable->storeAsURL(aTargetFileSaveAs, {});
    ++nExpected;
    nActual = CountFilesInDirectory(aTargetDirectory);
    // nActual was nExpected + 1, i.e. we leaked a tempfile.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}


CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
