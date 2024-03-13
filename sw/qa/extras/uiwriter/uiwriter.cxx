/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <shellio.hxx>
#include <expfld.hxx>
#include <drawdoc.hxx>
#include <redline.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <view.hxx>
#include <hhcwrp.hxx>
#include <swacorr.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <editeng/acorrcfg.hxx>
#include <unotools/streamwrap.hxx>
#include <unocrsrhelper.hxx>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <vcl/TypeSerializer.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <UndoManager.hxx>
#include <frmatr.hxx>

#include <com/sun/star/text/TextMarkupType.hpp>
#include <osl/file.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scheduler.hxx>
#include <sfx2/watermarkitem.hxx>
#include <sfx2/docfile.hxx>
#include <fmthdft.hxx>
#include <iodetect.hxx>
#include <comphelper/processfactory.hxx>
#include <unotxdoc.hxx>
#include <swdtflvr.hxx>
#include <sortedobjs.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>

namespace
{
void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetPointContentNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetPointContentNode(), end);
}
} //namespace

class SwUiWriterTest : public SwModelTestBase
{
public:
    SwUiWriterTest() :
        SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {}

    std::unique_ptr<SwTextBlocks> readDOCXAutotext(
        std::u16string_view sFileName, bool bEmpty = false);
    void testRedlineFrame(char const*const file);
};

std::unique_ptr<SwTextBlocks> SwUiWriterTest::readDOCXAutotext(std::u16string_view sFileName, bool bEmpty)
{
    createTempCopy(sFileName);

    SfxMedium aSrcMed(maTempFile.GetURL(), StreamMode::STD_READ);
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    SwReader aReader(aSrcMed, maTempFile.GetURL(), pDoc);
    Reader* pDOCXReader = SwReaderWriter::GetDOCXReader();
    auto pGlossary = std::make_unique<SwTextBlocks>(maTempFile.GetURL());

    CPPUNIT_ASSERT(pDOCXReader != nullptr);
    CPPUNIT_ASSERT_EQUAL(!bEmpty, aReader.ReadGlossaries(*pDOCXReader, *pGlossary, false));

    return pGlossary;
}

void SwUiWriterTest::testRedlineFrame(char const*const file)
{
    createSwDoc(file);
    SwDoc* pDoc = getSwDoc();
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

//Replacement tests

constexpr OUString ORIGINAL_REPLACE_CONTENT(u"toto titi tutu"_ustr);
constexpr OUString EXPECTED_REPLACE_CONTENT(u"toto toto tutu"_ustr);

// Chinese conversion tests

const sal_Unicode CHINESE_TRADITIONAL_CONTENT(0x9F8D);
const sal_Unicode CHINESE_SIMPLIFIED_CONTENT(0x9F99);
constexpr OUString NON_CHINESE_CONTENT(u"Hippopotamus"_ustr);

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testReplaceForward)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, ORIGINAL_REPLACE_CONTENT);

    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTextNode->GetText());
}


CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtCharStartOutside0)
{
    testRedlineFrame("redlineFrame.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtCharStartOutside)
{
    testRedlineFrame("redlineFrame_at_char_start_outside.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtCharStartInside)
{
    testRedlineFrame("redlineFrame_at_char_start_inside.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtParaStartOutside)
{
    testRedlineFrame("redline_fly_duplication_at_para_start_outside.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtParaEndInside)
{
    testRedlineFrame("redline_fly_duplication_at_para_end_inside.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtParaOneParagraph)
{
    // test ALLFLYS flag: oddly enough it didn't fail as fodt but failed as odt...
    testRedlineFrame("redline_fly_at_para_one_paragraph.odt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testRedlineFrameAtPara2ndParagraph)
{
    // lost via the buggy increment in Copy
    testRedlineFrame("redline_fly_duplication_at_para_2nd_paragraph.fodt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testThreadedException)
{
    SvFileStream aFileStream(createFileURL(u"threadedException.fodt"), StreamMode::READ);

    //threaded reading only kicks in if there is sufficient buffer to make it worthwhile, so read
    //from a SvFileStream to ensure that
    bool bRes = TestImportFODT(aFileStream);

    CPPUNIT_ASSERT(!bRes);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf149595)
{
    createSwDoc("demo91.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // all 4 shapes are on the 2nd paragraph
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() == nullptr);
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(4), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());

    {
        pWrtShell->Down(false);
        pWrtShell->EndPara(/*bSelect=*/true);
        dispatchCommand(mxComponent, ".uno:Cut", {});

        // one shape is anchored in the middle, others at the start/end/at-para
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() == nullptr);
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());

        pWrtShell->Up(false);
        dispatchCommand(mxComponent, ".uno:Paste", {});

        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs()->size());
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());

        pWrtShell->Undo();
        pWrtShell->Undo();

        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() == nullptr);
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());
    }

    // now try the same with redlining enabled - should be the same result
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    {
        pWrtShell->Down(false);
        pWrtShell->SttPara(/*bSelect=*/false);
        pWrtShell->EndPara(/*bSelect=*/true);
        dispatchCommand(mxComponent, ".uno:Cut", {});

        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() == nullptr);
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        // problem was that this deleted all at-char flys, even at the start/end
        CPPUNIT_ASSERT_EQUAL(size_t(3), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());

        pWrtShell->Up(false);
        dispatchCommand(mxComponent, ".uno:Paste", {});

        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs()->size());
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());

        pWrtShell->Undo();
        pWrtShell->Undo();

        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs() == nullptr);
        CPPUNIT_ASSERT(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs() != nullptr);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetNext()->GetDrawObjs()->size());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf149548)
{
    createSwDoc("forum-mso-en-13192-min.docx");
    SwDoc* pDoc = getSwDoc();

    for (SwRangeRedline const*const pRedline : pDoc->getIDocumentRedlineAccess().GetRedlineTable())
    {
        if (pRedline->GetType() == RedlineType::Delete)
        {
            int nLevel(0);
            for (SwNodeIndex index = pRedline->Start()->nNode; index <= pRedline->End()->nNode; ++index)
            {
                switch (index.GetNode().GetNodeType())
                {
                    case SwNodeType::Start:
                    case SwNodeType::Table:
                    case SwNodeType::Section:
                        ++nLevel;
                        break;
                    case SwNodeType::End:
                        CPPUNIT_ASSERT_MESSAGE("bad overlapping redline", nLevel != 0);
                        --nLevel;
                        break;
                    default:
                        break;
                }
            }
            CPPUNIT_ASSERT_EQUAL_MESSAGE("bad overlapping redline", int(0), nLevel);
        }
    }

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // this was a use-after-free on nodes deleted by Copy
    dispatchCommand(mxComponent, ".uno:Paste", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testPasteTableAtFlyAnchor)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>);
    flySet.Put(anchor);
    SwFlyFrameFormat const* pFly = dynamic_cast<SwFlyFrameFormat const*>(
            pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true));
    CPPUNIT_ASSERT(pFly != nullptr);
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);
    pWrtShell->SelFlyGrabCursor();
    pWrtShell->GetDrawView()->UnmarkAll();
    CPPUNIT_ASSERT(pWrtShell->GetCurrFlyFrame() != nullptr);

    // insert table in fly
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(tableOpt, 2, 2);

    // select table
    pWrtShell->SelAll();

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // move cursor back to body
    pWrtShell->ClearMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(!pWrtShell->GetCurrFlyFrame());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInTable());
    CPPUNIT_ASSERT(!pFly->GetAnchor().GetContentAnchor()->GetNode().FindTableNode());

    pWrtShell->Undo();

    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(!pWrtShell->IsCursorInTable());
    CPPUNIT_ASSERT(!pFly->GetAnchor().GetContentAnchor()->GetNode().FindTableNode());

    // the problem was that Redo moved the fly anchor into the first table cell
    pWrtShell->Redo();

    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInTable());
    CPPUNIT_ASSERT(!pFly->GetAnchor().GetContentAnchor()->GetNode().FindTableNode());

    pWrtShell->Undo();

    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(!pWrtShell->IsCursorInTable());
    CPPUNIT_ASSERT(!pFly->GetAnchor().GetContentAnchor()->GetNode().FindTableNode());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCopyPastePageBreak)
{
    {
        createSwDoc("pagebreak-source.fodt");
        SwDoc* pDoc = getSwDoc();

        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());

        pWrtShell->SelAll();
        dispatchCommand(mxComponent, ".uno:Copy", {});

        mxComponent->dispose();
        mxComponent.clear();
    }

    createSwDoc("pagebreak-target.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("WithMargin"), getProperty<OUString>(getParagraph(1), "PageDescName"));
    CPPUNIT_ASSERT_EQUAL(OUString("TargetSection"), pWrtShell->GetCurrSection()->GetSectionName());
    // page style WithMargin is used
    CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("WithMargin"), getProperty<OUString>(getParagraph(1), "PageDescName"));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetSections().size());
    CPPUNIT_ASSERT_EQUAL(OUString("SourceSection"), pWrtShell->GetCurrSection()->GetSectionName());
    // the problem was that there was a page break now
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // page style WithMargin is used
    CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());

    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("WithMargin"), getProperty<OUString>(getParagraph(1), "PageDescName"));
    CPPUNIT_ASSERT_EQUAL(OUString("TargetSection"), pWrtShell->GetCurrSection()->GetSectionName());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // page style WithMargin is used
    CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());

    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("WithMargin"), getProperty<OUString>(getParagraph(1), "PageDescName"));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetSections().size());
    CPPUNIT_ASSERT_EQUAL(OUString("SourceSection"), pWrtShell->GetCurrSection()->GetSectionName());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // page style WithMargin is used
    CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());

    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("WithMargin"), getProperty<OUString>(getParagraph(1), "PageDescName"));
    CPPUNIT_ASSERT_EQUAL(OUString("TargetSection"), pWrtShell->GetCurrSection()->GetSectionName());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // page style WithMargin is used
    CPPUNIT_ASSERT_EQUAL(tools::Long(5669), pWrtShell->GetLayout()->GetLower()->getFramePrintArea().Top());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testBookmarkCopy)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFormulaNumberWithGroupSeparator)
{
    createSwDoc("tdf125154.odt");
    dispatchCommand(mxComponent, ".uno:UpdateAll", {});
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(true);
    SwField const* pField;

    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), pField->GetFormula());
    CPPUNIT_ASSERT_EQUAL(OUString("1.000"), pField->ExpandField(true, nullptr));
    pWrtShell->GoNextCell();
    CPPUNIT_ASSERT_EQUAL(OUString("10000"), pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->GoNextCell();
    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("test"), pField->GetFormula());
    CPPUNIT_ASSERT_EQUAL(OUString("1.000"), pField->ExpandField(true, nullptr));
    pWrtShell->GoNextCell();
    // the problem was that this was 0
    CPPUNIT_ASSERT_EQUAL(OUString("10000"), pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->Down(false);
    pWrtShell->SttPara(false);
    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("1000*10%"), pField->GetFormula());
    CPPUNIT_ASSERT_EQUAL(OUString("100"), pField->ExpandField(true, nullptr));
    pWrtShell->Down(false);
    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("5.000*10%"), pField->GetFormula());
    // the problem was that this was 0
    CPPUNIT_ASSERT_EQUAL(OUString("500"), pField->ExpandField(true, nullptr));
    pWrtShell->Down(false);
    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("5.000*10%"), pField->GetFormula());
    // the problem was that this was
    CPPUNIT_ASSERT_EQUAL(OUString("500"), pField->ExpandField(true, nullptr));
    pWrtShell->Down(false);
    pField = pWrtShell->GetCurField();
    CPPUNIT_ASSERT_EQUAL(OUString("5000*10%"), pField->GetFormula());
    CPPUNIT_ASSERT_EQUAL(OUString("500"), pField->ExpandField(true, nullptr));
    pWrtShell->Down(false);
    CPPUNIT_ASSERT_EQUAL(u"-100,00 €"_ustr, pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->GoNextCell();
    // tdf#42518 the problem was that this was 1.900,00 €
    CPPUNIT_ASSERT_EQUAL(OUString("** Expression is faulty **"), pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testInsertFileInInputFieldException)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTextDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xBody(xTextDoc->getText());
    uno::Reference<lang::XMultiServiceFactory> const xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> const xCursor(xBody->createTextCursor());
    uno::Reference<document::XDocumentInsertable> const xInsertable(xCursor, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> const xContent(
        xFactory->createInstance("com.sun.star.text.textfield.Input"), uno::UNO_QUERY);
    xBody->insertTextContent(xCursor, xContent, false);
    xCursor->goLeft(1, false);
    // try to insert some random file
    // inserting even asserts in debug builds - document model goes invalid with input field split across 2 nodes
    CPPUNIT_ASSERT_THROW(xInsertable->insertDocumentFromURL(createFileURL(u"fdo75110.odt"), {}), uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf67238)
{
    //create a new writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf155685)
{
    createSwDoc("table-at-end-of-cell.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GoNextCell();
    pWrtShell->GoNextCell();
    pWrtShell->GoNextCell();
    pWrtShell->SelAll();
    pWrtShell->Delete();
    // this crashed
    pWrtShell->Undo();
    pWrtShell->Undo();
    pWrtShell->Redo();
    // this crashed
    pWrtShell->Redo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf147220)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert(u"él"_ustr);

    // hide and enable
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->GoStartSentence();
    pWrtShell->SetMark();
    pWrtShell->GoEndSentence();

    // this did not remove the original text from the layout
    pWrtShell->Replace(u"Él"_ustr, false);

    // currently the deleted text is before the replacement text, not sure if
    // that is really required
    CPPUNIT_ASSERT_EQUAL(u"élÉl"_ustr,
        pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"Él"_ustr,
        static_cast<SwTextFrame const*>(pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->getLayoutFrame(nullptr))->GetText());

    SwRedlineTable const& rRedlines(pDoc->getIDocumentRedlineAccess().GetRedlineTable());
    CPPUNIT_ASSERT_EQUAL(SwRedlineTable::size_type(2), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlines[0]->GetType());
    CPPUNIT_ASSERT_EQUAL(u"él"_ustr, rRedlines[0]->GetText());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[1]->GetType());
    CPPUNIT_ASSERT_EQUAL(u"Él"_ustr, rRedlines[1]->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf135978)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("foobar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bazquux");

    CPPUNIT_ASSERT(pWrtShell->IsEndOfDoc());

    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>);
    flySet.Put(anchor);
    SwFlyFrameFormat const* pFly = dynamic_cast<SwFlyFrameFormat const*>(
            pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true));
    CPPUNIT_ASSERT(pFly != nullptr);
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);
    // move cursor back to body
    pWrtShell->SttEndDoc(/*bStt=*/false);

    // hide and enable
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // now split
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);

    // the problem was that undo removed the fly frame from the layout
    pWrtShell->Undo();
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);

    pWrtShell->Redo();
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);

    pWrtShell->Undo();
    CPPUNIT_ASSERT(pFly->GetFrame() != nullptr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo75110)
{
    createSwDoc("fdo75110.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll();
    // The problem was that SwEditShell::DeleteSel() what this Delete() invokes took the wrong selection...
    pWrtShell->Delete();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    // ... so this Undo() call resulted in a crash.
    rUndoManager.Undo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo75898)
{
    createSwDoc("fdo75898.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->InsertRow(1, true);
    pWrtShell->InsertRow(1, true);

    // Now check if the table has 3 lines.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    SwTableNode* pTableNode = pShellCursor->Start()->GetNode().FindTableNode();
    // This was 1, when doing the same using the UI, Writer even crashed.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pTableNode->GetTable().GetTabLines().size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testReplaceBackward)
{
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "toto titi tutu");
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "toto", false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTextNode->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo69893)
{
    createSwDoc("fdo69893.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll(); // A1 is empty -> selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.

    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
    // Selection did not include the para after table, this was "B1".
    CPPUNIT_ASSERT_EQUAL(OUString("Para after table."), rEnd.GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo70807)
{
    createSwDoc("fdo70807.odt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testImportRTF)
{
    // Insert "foobar" and position the cursor between "foo" and "bar".
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("foobar");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);

    // Insert the RTF at the cursor position.
    OString aData = "{\\rtf1 Hello world!\\par}"_ostr;
    SvMemoryStream aStream(const_cast<char*>(aData.getStr()), aData.getLength(), StreamMode::READ);
    SwReader aReader(aStream, OUString(), OUString(), *pWrtShell->GetCursor());
    Reader* pRTFReader = SwReaderWriter::GetRtfReader();
    CPPUNIT_ASSERT(pRTFReader != nullptr);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aReader.Read(*pRTFReader).GetCode());

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString("fooHello world!"), pDoc->GetNodes()[nIndex - 1]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), pDoc->GetNodes()[nIndex]->GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testExportRTF)
{
    // Insert "aaabbbccc" and select "bbb".
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("aaabbbccc");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);

    // Create the clipboard document.
    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);
    pWrtShell->Copy(*xClpDoc);

    // And finally export it as RTF.
    WriterRef xWrt;
    SwReaderWriter::GetWriter(u"RTF", OUString(), xWrt);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDOCXAutoTextEmpty)
{
    // file contains normal content but no AutoText
    std::unique_ptr<SwTextBlocks> pGlossary = readDOCXAutotext(u"autotext-empty.dotx", true);
    CPPUNIT_ASSERT(pGlossary != nullptr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDOCXAutoTextMultiple)
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDOTMAutoText)
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDOCXAutoTextGallery)
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testWatermarkDOCX)
{
    createSwDoc("watermark.docx");
    SwDoc* const pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SfxPoolItemHolder aResult;
    SfxItemState eState = pDocShell->GetViewShell()->GetViewFrame().GetDispatcher()->QueryState(SID_WATERMARK, aResult);
    const SfxWatermarkItem* pWatermark(static_cast<const SfxWatermarkItem*>(aResult.getItem()));

    CPPUNIT_ASSERT(eState >= SfxItemState::DEFAULT);
    CPPUNIT_ASSERT(pWatermark);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(SID_WATERMARK), pWatermark->Which());

    CPPUNIT_ASSERT_EQUAL(OUString("CustomWatermark"), pWatermark->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("DejaVu Sans Light"), pWatermark->GetFont());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(45), pWatermark->GetAngle());
    CPPUNIT_ASSERT_EQUAL(Color(0x548dd4), pWatermark->GetColor());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), pWatermark->GetTransparency());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testWatermarkPosition)
{
    // tdf#108494 Watermark inserted in the document with page break was outside the first page
    const int aPagesInDocument = 2;
    const int aAdditionalPagesCount[] = {    0,     0,    1,     1,    5,     5,   20,    20 };
    const bool aChangeHeader[]        = { true, false, true, false, true, false, true, false };

    for (tools::ULong i = 0; i < sizeof(aAdditionalPagesCount) / sizeof(int); ++i)
    {
        int aPages = aPagesInDocument + aAdditionalPagesCount[i];

        // Empty document with one Page Break
        createSwDoc("watermark-position.odt");
        SwDoc* pDoc = getSwDoc();
        SwEditShell* pEditShell = pDoc->GetEditShell();
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        uno::Reference<frame::XModel> xModel = pDoc->GetDocShell()->GetBaseModel();
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo74981)
{
    // create a document with an input field
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInputField aField(static_cast<SwInputFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::Input)), "foo", "bar", 0, 0);
    pWrtShell->InsertField2(aField);

    {
        // expect hints
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
        CPPUNIT_ASSERT(pTextNode->HasHints());
    }

    // go to the begin of the paragraph and split this node
    pWrtShell->Left(SwCursorSkipMode::Chars, false, 100, false);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf98512)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInputFieldType *const pType(static_cast<SwInputFieldType*>(
                pWrtShell->GetFieldType(0, SwFieldIds::Input)));
    SwInputField aField1(pType, "foo", "bar", INP_TXT, 0);
    pWrtShell->InsertField2(aField1);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwInputField aField2(pType, "baz", "quux", INP_TXT, 0);
    pWrtShell->InsertField2(aField2);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testShapeTextboxSelect)
{
    createSwDoc("shape-textbox.odt");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testShapeTextboxDelete)
{
    createSwDoc("shape-textbox.odt");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testAnchorChangeSelection)
{
    createSwDoc("test_anchor_as_character.odt");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCp1000071)
{
    createSwDoc("cp1000071.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL( SwRedlineTable::size_type( 2 ), rTable.size());
    SwNodeOffset redlineStart0NodeIndex = rTable[ 0 ]->Start()->GetNodeIndex();
    sal_Int32 redlineStart0Index = rTable[ 0 ]->Start()->GetContentIndex();
    SwNodeOffset redlineEnd0NodeIndex = rTable[ 0 ]->End()->GetNodeIndex();
    sal_Int32 redlineEnd0Index = rTable[ 0 ]->End()->GetContentIndex();
    SwNodeOffset redlineStart1NodeIndex = rTable[ 1 ]->Start()->GetNodeIndex();
    sal_Int32 redlineStart1Index = rTable[ 1 ]->Start()->GetContentIndex();
    SwNodeOffset redlineEnd1NodeIndex = rTable[ 1 ]->End()->GetNodeIndex();
    sal_Int32 redlineEnd1Index = rTable[ 1 ]->End()->GetContentIndex();

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
    CPPUNIT_ASSERT_EQUAL( redlineStart0NodeIndex, rTable[ 0 ]->Start()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart0Index, rTable[ 0 ]->Start()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0NodeIndex, rTable[ 0 ]->End()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd0Index, rTable[ 0 ]->End()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1NodeIndex, rTable[ 1 ]->Start()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL( redlineStart1Index, rTable[ 1 ]->Start()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1NodeIndex, rTable[ 1 ]->End()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL( redlineEnd1Index, rTable[ 1 ]->End()->GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testShapeTextboxVertadjust)
{
    createSwDoc("shape-textbox-vertadjust.odt");
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwFrameFormat* pFormat = static_cast<SwContact*>(pObject->GetUserCall())->GetFormat();
    // This was SDRTEXTVERTADJUST_TOP.
    CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_CENTER, pFormat->GetTextVertAdjust().GetValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testShapeTextboxAutosize)
{
    createSwDoc("shape-textbox-autosize.odt");
    SwDoc* pDoc = getSwDoc();
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
    CPPUNIT_ASSERT(pFirst->GetSnapRect().getOpenHeight() < pSecond->GetSnapRect().getOpenHeight());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo82191)
{
    createSwDoc("fdo82191.odt");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCommentedWord)
{
    // This word is commented. <- string in document
    // 123456789 <- character positions
    createSwDoc("commented-word.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Select the word.
    pWrtShell->SelWrd();

    // Make sure that not only the word, but its comment anchor is also selected.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This was 9, only "word", not "word<anchor character>" was selected.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pShellCursor->End()->GetContentIndex());

    // Test that getAnchor() points to "word", not to an empty string.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("word"), xField->getAnchor()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTextFieldGetAnchorGetTextInFooter)
{
    createSwDoc("textfield-getanchor-gettext-in-footer.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);

    OUString value = xField->getAnchor()->getText()->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("userfield_in_footer"), value );
}

// Tests that a blank document is still blank after conversion
CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testChineseConversionBlank)
{

    // Given
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(), pTextNode->GetText());

}

// Tests that non Chinese text is unchanged after conversion
CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testChineseConversionNonChineseText)
{

    // Given
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(NON_CHINESE_CONTENT, pTextNode->GetText());

}

// Tests conversion of traditional Chinese characters to simplified Chinese
CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testChineseConversionTraditionalToSimplified)
{

    // Given
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(CHINESE_SIMPLIFIED_CONTENT), pTextNode->GetText());

}

// Tests conversion of simplified Chinese characters to traditional Chinese
CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testChineseConversionSimplifiedToTraditional)
{

    // Given
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString(CHINESE_TRADITIONAL_CONTENT), pTextNode->GetText());

}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo85554)
{
    // Load the document, it contains one shape with a textbox.
    createSwDoc("fdo85554.odt");

    // Add a second shape to the document.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    xDrawPage->add(xShape);

    // Save it and load it back.
    saveAndReload("writer8");

    // This was 1, we lost a shape on export.
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testMergeDoc)
{
    createSwDoc("merge-change1.odt");
    SwDoc* pDoc1 = getSwDoc();

    auto xDoc2Component(loadFromDesktop(
            createFileURL(u"merge-change2.odt"),
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCreatePortions)
{
    createSwDoc("uno-cycle.odt");
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testBookmarkUndo)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

    pMarkAccess->deleteMark(pMarkAccess->findMark("Mark_"), false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf148389_Left)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Insert("foo bar baz");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();

    auto pMark = pMarkAccess->makeMark(*pWrtShell->GetCursor(), "Mark",
        IDocumentMarkAccess::MarkType::BOOKMARK, ::sw::mark::InsertMode::New);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelLeft();
    // historically it wasn't deleted if empty, not sure if it should be
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    // the problem was that the end position was not restored
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    // this undo is no longer grouped, to prevent Redo deleting bookmark
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pMark->GetOtherMarkPos().GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf148389_Right)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Insert("foo bar baz");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();

    auto pMark = pMarkAccess->makeMark(*pWrtShell->GetCursor(), "Mark",
        IDocumentMarkAccess::MarkType::BOOKMARK, ::sw::mark::InsertMode::New);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->DelRight();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelRight();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelRight();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->DelRight();
    // historically it wasn't deleted if empty, not sure if it should be
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    // the problem was that the start position was not restored
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    // this undo is no longer grouped, to prevent Redo deleting bookmark
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    // this undo is no longer grouped, to prevent Redo deleting bookmark
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMark->GetOtherMarkPos().GetContentIndex());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    // Undo re-creates the mark...
    pMark = *pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMark->GetMarkPos().GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pMark->GetOtherMarkPos().GetContentIndex());
}

static void lcl_setWeight(SwWrtShell* pWrtShell, FontWeight aWeight)
{
    SvxWeightItem aWeightItem(aWeight, EE_CHAR_WEIGHT);
    SvxScriptSetItem aScriptSetItem(SID_ATTR_CHAR_WEIGHT, pWrtShell->GetAttrPool());
    aScriptSetItem.PutItemForScriptType(SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX, aWeightItem);
    pWrtShell->SetAttrSet(aScriptSetItem.GetItemSet());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo85876)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCaretPositionMovingUp)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("after");
    pWrtShell->InsertLineBreak();
    pWrtShell->Up(false);
    pWrtShell->Insert("before");

    CPPUNIT_ASSERT_EQUAL(OUString(u"beforeAfter" + OUStringChar(CH_TXTATR_NEWLINE)), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf93441)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf81226)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("before");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Down(false);
    pWrtShell->Insert("after");

    // Without the fix in place, this test would have failed with
    // - Expected: beforeafter
    // - Actual  : beafterfore
    CPPUNIT_ASSERT_EQUAL(OUString("beforeafter"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf137532)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("test");

    //Select the word and change it to bold
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    lcl_setWeight(pWrtShell, WEIGHT_BOLD);

    // Select first character and replace it
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Insert("x");

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));

    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("xest"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    xCursor.set(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xCursor->getString());

    // Without the fix in place, this test would have failed in
    // - Expected: 150
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    xCursor.set(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT(xCursor.is());
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testFdo87448)
{
    createSwDoc("fdo87448.odt");

    // Save the first shape to a metafile.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(comphelper::getProcessComponentContext());
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "OutputStream", uno::Any(xOutputStream) },
            { "FilterName", uno::Any(OUString("SVM")) }
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
    sal_Int32 nFirstEnd = getXPath(pXmlDoc, "(//polyline)[1]/point[last()-1]"_ostr, "x"_ostr).toInt32();
    // The second polyline has a different start point, but the arc it draws
    // should end at the ~same position as the first polyline.
    sal_Int32 nSecondEnd = getXPath(pXmlDoc, "(//polyline)[2]/point[last()]"_ostr, "x"_ostr).toInt32();

    // nFirstEnd was 6023 and nSecondEnd was 6648, now they should be much closer, e.g. nFirstEnd = 6550, nSecondEnd = 6548
    OString aMsg = "nFirstEnd is " + OString::number(nFirstEnd) + ", nSecondEnd is " + OString::number(nSecondEnd);
    // Assert that the difference is less than half point.
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), abs(nFirstEnd - nSecondEnd) < 10);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTextCursorInvalidation)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageStyle.is());
    xPageStyle->setPropertyValue("HeaderIsOn", uno::Any(true));
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf68183)
{
    // First disable RSID and check if indeed no such attribute is inserted.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SW_MOD()->GetModuleConfig()->SetStoreRsid(false);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert2("X");

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(false, pTextNode->GetSwAttrSet().HasItem(RES_PARATR_RSID));

    // Then enable storing of RSID and make sure that the attribute is inserted.
    SW_MOD()->GetModuleConfig()->SetStoreRsid(true);

    pWrtShell->DelToStartOfLine();
    pWrtShell->Insert2("X");

    CPPUNIT_ASSERT_EQUAL(true, pTextNode->GetSwAttrSet().HasItem(RES_PARATR_RSID));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testCp1000115)
{
    createSwDoc("cp1000115.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1: the long paragraph in the B1 cell did flow over to the
    // second page, so there was only one paragraph in the second cell of the
    // second page.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[2]/txt"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf63214)
{
    //This is a crash test
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf90003)
{
    createSwDoc("tdf90003.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // This was 1: an unexpected fly portion was created, resulting in too
    // large x position for the empty paragraph marker.
    assertXPath(pXmlDoc, "//SwFixPortion[@type='PortionType::Fly']"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf51741)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    pMarkAccess->deleteMark(pMarkAccess->findMark("Mark_"), false);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDefaultsOfOutlineNumbering)
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testDeleteTableRedlines)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testXFlatParagraph)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest, testTdf81995)
{
    uno::Reference<text::XDefaultNumberingProvider> xDefNum(m_xSFactory->createInstance("com.sun.star.text.DefaultNumberingProvider"), uno::UNO_QUERY);
    css::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    const uno::Sequence<uno::Reference<container::XIndexAccess>> aIndexAccess(xDefNum->getDefaultOutlineNumberings(alocale));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aIndexAccess.getLength());
    for(const auto& rIndexAccess : aIndexAccess)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), rIndexAccess->getCount());
        for(int j=0;j<rIndexAccess->getCount();j++)
        {
            uno::Sequence<beans::PropertyValue> aProps;
            rIndexAccess->getByIndex(j) >>= aProps;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aProps.getLength());
            for (const beans::PropertyValue& rProp : aProps)
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
