/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sstream>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/AxisOrientation.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/configuration.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <flyfrm.hxx>
#include <fmtanchr.hxx>
#include <UndoManager.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <itabenum.hxx>
#include <fmtfsize.hxx>
#include <xmloff/odffields.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/acorrcfg.hxx>
#include <swacorr.hxx>
#include <txtfrm.hxx>
#include <redline.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <AnnotationWin.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <fmtcntnt.hxx>
#include <frameformats.hxx>
#include <shellio.hxx>
#include <editeng/fontitem.hxx>
#include <tools/json_writer.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <comphelper/processfactory.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";
constexpr OUStringLiteral FLOATING_TABLE_DATA_DIRECTORY
    = u"/sw/qa/extras/uiwriter/data/floating_table/";
} // namespace

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename).indexOf("LocaleArabic") != -1)
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ar"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf47471_paraStyleBackground)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf47471_paraStyleBackground.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14729933), getProperty<sal_Int32>(getParagraph(2), "FillColor"));

    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:ResetAttributes", {});

    // the background color should revert to the color for 00Background style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // the paragraph style should not be reset
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // Save it and load it back.
    reload("writer8", "tdf47471_paraStyleBackgroundRT.odt");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // on round-trip, the paragraph style name was lost
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdfChangeNumberingListAutoFormat)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf117923.docx");

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", "GHI GHI GHI GHI");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nType",
                "PortionType::Number");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "rText", "2.");
    // The numbering height was 960 in DOC format.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Grow", {});
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "260");

    // save it to DOCX
    reload("Office Open XML Text", "tdf117923.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // this was 220
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "260");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101534)
{
    // Copy the first paragraph of the document.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf101534.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>);
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    TransferableDataHelper aHelper(pTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    aSet.ClearItem();
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineMoveInsertInDelete)
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert(" foo");
    pWrtShell->SttEndDoc(true);
    pWrtShell->InsertFootnote("");
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    // insert redline
    pWrtShell->Insert("bar");
    // first delete redline, logically containing the insert redline
    // (note: Word apparently allows similar things...)
    pWrtShell->SttEndDoc(true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete(); // the footnote
    // second delete redline, following the first one
    pWrtShell->EndOfSection(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    pWrtShell->Delete(); // "foo"

    // hiding used to copy the 2nd delete redline "foo", but not delete it
    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide
    CPPUNIT_ASSERT_EQUAL(
        OUString(" "),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->SetRedlineFlags(mode); // show again
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"\u0001 foo"),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineInHiddenSection)
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SplitNode();
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("baz");

    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);

    // delete paragraph "bar"
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->StartOfSection();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndOfSection(true);

    SwSectionData section(SectionType::Content, pWrtShell->GetUniqueSectionName());
    section.SetHidden(true);
    SwSection const* pSection = pWrtShell->InsertSection(section, nullptr);

    SwSectionNode const* pNode = pSection->GetFormat()->GetSectionNode();

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 2]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode); // show again

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    // there was a frame created here
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineSplitContentNode)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "try2.fodt");
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    // these are required so that IsBlank() is true
    aViewOptions.SetBlank(true);
    aViewOptions.SetViewMetaChars(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    pWrtShell->CalcLayout();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 18, /*bBasicCall=*/false);
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
    // crashed
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Undo();
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136704)
{
    SwDoc* const pDoc(createSwDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    corr.GetSwFlags().bReplaceStyles = true;
    SvxSwAutoFormatFlags flags(*SwEditShell::GetAutoFormatFlags());
    comphelper::ScopeGuard const g([=]() { SwEditShell::SetAutoFormatFlags(&flags); });
    flags.bReplaceStyles = true;
    SwEditShell::SetAutoFormatFlags(&flags);

    pWrtShell->Insert("test");
    const sal_Unicode cIns = ':';
    pWrtShell->AutoCorrect(corr, cIns);

    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134250)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134250.fodt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextContent> xTextContent(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("foo" SAL_NEWLINE_STRING "bar"),
                         xTextContent->getAnchor()->getString());

    // select all with table at start -> 3 times
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // .uno:Copy without touching shared clipboard
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    // .uno:Paste without touching shared clipboard
    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    // this would crash in 2 different ways
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    // Without the fix in place, section's content would have been gone after undo
    CPPUNIT_ASSERT_EQUAL(OUString("foo" SAL_NEWLINE_STRING "bar"),
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("foo" SAL_NEWLINE_STRING "bar"),
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134436)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134436.fodt");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // select all 3 times, table at the start
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    // the stupid SwXTextView::getString doesn't work "for symmetry" so use CursorShell
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    // first, the section doesn't get deleted
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    // second, the section does get deleted because point is at the end
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    // the problem was that the section was not deleted on Redo
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134252)
{
    createSwDoc(DATA_DIRECTORY, "tdf134252.fodt");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCursor(xTextViewCursorSupplier->getViewCursor());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // select all with section
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING),
                         xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());

    // this would crash
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING),
                         xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING),
                         xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136453)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf136453.fodt");
    SwWrtShell* const pWrtShell(pDoc->GetDocShell()->GetWrtShell());

    SwNodeOffset const nNodes(pDoc->GetNodes().Count());

    pWrtShell->SttEndDoc(false);
    pWrtShell->SetMark();
    pWrtShell->Up(true, 1);
    pWrtShell->SttPara(true);
    pWrtShell->Delete();

    // one paragraph deleted, section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 3, pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    // check that every node has 1 frame
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt", 1);

    pWrtShell->Redo();

    // one paragraph deleted, section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 3, pDoc->GetNodes().Count());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 0);

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137245)
{
    SwDoc* const pDoc(createSwDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    corr.GetSwFlags().bSetBorder = true;
    // sigh, it's a global... err i mean Singleton design pattern *eyeroll*
    SvxSwAutoFormatFlags flags(*SwEditShell::GetAutoFormatFlags());
    comphelper::ScopeGuard const g([=]() { SwEditShell::SetAutoFormatFlags(&flags); });
    flags.bSetBorder = true;
    SwEditShell::SetAutoFormatFlags(&flags);

    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    // move cursor back to body
    pWrtShell->SttEndDoc(false);
    // keep first paragraph empty so that its flys may be deleted too
    //pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);

    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }

    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    // move cursor back to body
    pWrtShell->SttEndDoc(false);
    pWrtShell->Insert("---");
    pWrtShell->SplitNode(true);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    // check that the AutoFormat did something
    pWrtShell->SttEndDoc(true);
    SfxItemSet set{ pDoc->GetAttrPool(), svl::Items<RES_BOX, RES_BOX> };
    pWrtShell->GetCurParAttr(set);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, set.GetItemState(RES_BOX, false));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf132236)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132236.odt");

    // select everything and delete
    SwWrtShell* const pWrtShell(pDoc->GetDocShell()->GetWrtShell());
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Delete();
    sw::UndoManager& rUndoManager(pDoc->GetUndoManager());
    rUndoManager.Undo();

    // check that the text frames exist inside their sections
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf131912)
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sw::UnoCursorPointer pCursor(
        pDoc->CreateUnoCursor(SwPosition(SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1))));

    pDoc->getIDocumentContentOperations().InsertString(*pCursor, "foo");

    {
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        pWrtShell->StartOfSection(false);
        pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    pCursor->SetMark();
    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // replace with more text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "blahblah", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // replace with less text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "x", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // regex replace with paragraph breaks
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "xyz\\n\\nquux\\n", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    // regex replace with paragraph join
    pWrtShell->StartOfSection(false);
    pWrtShell->Down(true);
    pDoc->getIDocumentContentOperations().ReplaceRange(*pWrtShell->GetCursor(), "bar", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf39721)
{
// FIXME: disabled on Windows because of a not reproducible problem (not related to the patch)
#if !defined(_WIN32)
    // check move down with redlining
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // store original text of the document for checking Undo
    OUString sOrigText(pTextDoc->getText()->getString());

    // first paragraph is "Lorem ipsum" with deleted "m ips"
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());

    // move down first paragraph with change tracking
    dispatchCommand(mxComponent, ".uno:MoveDown", {});

    // deletion isn't rejected
    CPPUNIT_ASSERT_EQUAL(OUString("Loremm"), getParagraph(3)->getString());

    // Undo and repeat it with the second paragraph
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sOrigText, pTextDoc->getText()->getString());

    // second paragraph is "dolor sit" with deleted "lor "
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit"), getParagraph(2)->getString());

    // move down second paragraph with change tracking
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);

    dispatchCommand(mxComponent, ".uno:MoveDown", {});

    // This was "dolor sit" (rejecting tracked deletion)
    CPPUNIT_ASSERT_EQUAL(OUString("dolsit"), getParagraph(4)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sOrigText, pTextDoc->getText()->getString());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf145066_bad_paragraph_deletion)
{
    // check move down with redlining: jumping over a deleted paragraph
    // resulted bad deletion of the not deleted adjacent paragraph in Show Changes mode
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Three paragraphs (list items)
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

    // move down once and move up two times second paragraph with change tracking
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:MoveDown", {});
    dispatchCommand(mxComponent, ".uno:MoveUp", {});
    dispatchCommand(mxComponent, ".uno:MoveUp", {});

    // accept all changes
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});

    // This was 2 (bad deletion of the first paragraph)
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and hide changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove first paragraph with paragraph break
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // remaining paragraph keeps its original style
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819_keep_numbering_with_Undo)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819b.odt");

    // heading
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove heading with paragraph break
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    sNumName = getProperty<OUString>(getParagraph(2), "NumberingStyleName");
    // solved problem: lost numbering
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // accept deletion, remaining (now second) paragraph: still bulleted list item
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(2), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading, manual test is correct
    // TODO: it works well, but the test fails...
    // SwWrtShell* const pWrtShell2 = pDoc->GetDocShell()->GetWrtShell();
    // CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
    //                     getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
    //                     getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571_keep_numbering_with_Undo)
{
    // as the previous test, but with partial paragraph deletion:
    // all deleted paragraphs get the formatting of the first (the partially deleted) one
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819b.odt");

    // heading
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove only end part of the heading and the next numbered paragraph with paragraph break
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // accept deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // Joined paragraph 2 and paragraph 4: Fusce...nunc.
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith("Fusce"));
    CPPUNIT_ASSERT(getParagraph(2)->getString().endsWith("nunc."));
    // Remaining (now second) paragraph: it is still heading
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571_keep_numbering_with_Reject)
{
    // as the previous test, but with partial paragraph deletion:
    // all deleted paragraphs get the formatting of the first (the partially deleted) one
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819b.odt");

    // heading
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove only end part of the heading and the next numbered paragraph with paragraph break
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // reject deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109376_redline)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    flySet.Put(anchor);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
    rIDRA.AppendRedline(new SwRangeRedline(RedlineType::Delete, pam), true);
    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109376)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    flySet.Put(anchor);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    pDoc->getIDocumentContentOperations().DeleteRange(pam);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf64242_optimizeTable)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf64242_optimizeTable.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    double origWidth = getProperty<double>(xTextTable, "Width");
    sal_Int32 nToleranceW = origWidth * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width", double(17013), origWidth, nToleranceW);

    pWrtShell->SelTable(); //select the whole table

    dispatchCommand(mxComponent, ".uno:SetOptimalColumnWidth", {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width: optimize", origWidth,
                                         getProperty<double>(xTextTable, "Width"), nToleranceW);

    dispatchCommand(mxComponent, ".uno:SetMinimalColumnWidth", {});
    CPPUNIT_ASSERT_MESSAGE("Table Width: minimized",
                           (origWidth - nToleranceW) > getProperty<double>(xTextTable, "Width"));

    double origRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    sal_Int32 nToleranceH = origRowHeight * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Row Height", double(3441), origRowHeight, nToleranceH);

    dispatchCommand(mxComponent, ".uno:SetOptimalRowHeight", {});
    double optimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: optimized",
                           (origRowHeight - nToleranceH) > optimalRowHeight);

    dispatchCommand(mxComponent, ".uno:SetMinimalRowHeight", {});
    double minimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: minimized",
                           (optimalRowHeight - nToleranceH) > minimalRowHeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row set to auto-height", double(0), minimalRowHeight);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf45525)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf45525.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    CPPUNIT_ASSERT_EQUAL(3889.0, getProperty<double>(xTableRows->getByIndex(0), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(1), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(2), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(3), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(4), "Height"));

    //Select three cells in the first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, ".uno:SetOptimalRowHeight", {});

    // Without the fix in place, this test would have failed with
    // - Expected: 1914
    // - Actual  : 3889
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(0), "Height"));
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(1), "Height"));
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(2), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(3), "Height"));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(4), "Height"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126784_distributeSelectedColumns)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126784_distributeSelectedColumns.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    auto aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                       "TableColumnSeparators");
    sal_Int16 nOrigCol2Pos = aSeq[0].Position;
    sal_Int16 nOrigCol3Pos = aSeq[1].Position;

    //Select column 1 and 2
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:DistributeColumns", {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  "TableColumnSeparators");
    CPPUNIT_ASSERT_MESSAGE("Second column should shrink", nOrigCol2Pos < aSeq[0].Position);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last column shouldn't change", nOrigCol3Pos, aSeq[1].Position);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144317)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144317.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    auto aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                       "TableColumnSeparators");
    sal_Int16 nOrigCol1Pos = aSeq[0].Position;

    // Move the cursor inside the table
    pWrtShell->Down(/*bSelect=*/false);

    //Select some cells in the first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, ".uno:SetMinimalColumnWidth", {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  "TableColumnSeparators");
    CPPUNIT_ASSERT_MESSAGE("First column should shrink", aSeq[0].Position < nOrigCol1Pos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf108687_tabstop)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf108687_tabstop.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nStartIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), nStartIndex);

    // Now pressing 'tab' should jump to the radio buttons.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    //sal_Int32 nEndIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(11), nEndIndex);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // join paragraphs by removing the end of the first one with paragraph break
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // second paragraph changes its style in "Show changes" mode
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144058)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144058.fodt");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // join first and last but one paragraphs by removing the end of the first paragraph
    // with paragraph break, and by removing two tables of the selected range completely
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // accept all: tables are deleted
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 2 (remaining empty tables)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119019)
{
    // check handling of overlapping redlines
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf119019.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus."),
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 1)->getString());
    // second paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), "RedlineType"));

    // delete last word of the second paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("tellus."), getRun(getParagraph(2), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119824)
{
    // check handling of overlapping redlines with Redo
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf119019.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "
                                  "et netus et malesuada fames ac turpis egestas. "
                                  "Proin pharetra nonummy pede. Mauris et orci."),
                         getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    // third paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // and a tracked text deletion at the beginning of the paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 2), "RedlineType"));

    // delete last word of the third paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracking of the new text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed (tracked deletion is in the second run)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // tdf#119824 check redo
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Redo();

    // check again the first tracked text deletion (we lost this before the redo fix)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // check redo of the new tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105413)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105413.fodt");

    // all paragraphs have got Standard paragraph style
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                             getProperty<OUString>(getParagraph(i), "ParaStyleName"));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Set Heading 1 paragraph style in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same style in the first paragraph automatically
    // to keep the changed paragraph style at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // first paragraph gets the same heading style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "num-parent-style.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("1.1"),
                         getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    // This was "1 Heading" instead of "2 Heading"
    CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    // This was "2.1 Heading"
    CPPUNIT_ASSERT_EQUAL(OUString("3.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // set Heading 1 style of paragraph 3 to Heading 2

    pWrtShell->Down(/*bSelect=*/false);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(3), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(getParagraph(3), "ListLabelString"));

    uno::Sequence<beans::PropertyValue> aPropertyValues2 = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 2")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues2);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(3), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(3), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.2"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817_round_trip)
{
    createSwDoc(DATA_DIRECTORY, "tdf76817.fodt");

    // save it to DOCX
    reload("Office Open XML Text", "tdf76817.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("1.1"),
                         getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    // This was "1 Heading" instead of "2 Heading"
    CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    // This was "2.1 Heading"
    CPPUNIT_ASSERT_EQUAL(OUString("3.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // set Heading 1 style of paragraph 3 to Heading 2

    pWrtShell->Down(/*bSelect=*/false);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(3), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(getParagraph(3), "ListLabelString"));

    uno::Sequence<beans::PropertyValue> aPropertyValues2 = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 2")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues2);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(3), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(3), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2.2"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817_custom_outline)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf76817.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(OUString("1.1"),
                         getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), "OutlineLevel"));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), "OutlineLevel"));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(OUString("3.1"),
                         getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf123102)
{
    createSwDoc(DATA_DIRECTORY, "tdf123102.odt");
    // insert a new row after a vertically merged cell
    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "3." - caused by the hidden numbered paragraph of the new merged cell
    assertXPath(pXmlDoc, "/root/page/body/tab/row[6]/cell[1]/txt/Special", "rText", "2.");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonSmallTable)
{
    // The floating table in the test document is too small, so we don't provide an unfloat button
    SwDoc* pDoc = createSwDoc(FLOATING_TABLE_DATA_DIRECTORY, "small_floating_table.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = pAnchoredObj->DynCastFlyFrame();
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButton)
{
    // Different use cases where unfloat button should be visible
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt", // Typical use case of multipage floating table
        "unfloatable_floating_table.docx", // Need to test the DOCX import whether we detect the floating table correctly
        "unfloatable_floating_table.doc", // Also the DOC import
        "unfloatable_small_floating_table.docx" // Atypical use case, when the table is small, but because of it's position is it broken to two pages
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        SwDoc* pDoc = createSwDoc(FLOATING_TABLE_DATA_DIRECTORY, sTestFileName.getStr());
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        const SwSortedObjs* pAnchored;
        if (sTestFileName == "unfloatable_small_floating_table.docx")
            pAnchored = pWrtShell->GetLayout()
                            ->GetLower()
                            ->GetLower()
                            ->GetLower()
                            ->GetNext()
                            ->GetDrawObjs();
        else
            pAnchored = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                     pAnchored->size());
        SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

        // The unfloat button is not visible until it gets selected
        SwFlyFrame* pFlyFrame = pAnchoredObj->DynCastFlyFrame();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pFlyFrame->IsShowUnfloatButton(pWrtShell));

        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonReadOnlyMode)
{
    // In read only mode we don't show the unfloat button even if we have a multipage floating table
    SwDoc* pDoc = createSwDoc(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->SetReadonlyOption(true);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = pAnchoredObj->DynCastFlyFrame();
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloating)
{
    // Test unfloating with tables imported from different file formats
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt",
        "unfloatable_floating_table.docx",
        "unfloatable_floating_table.doc",
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        // Test what happens when pushing the unfloat button
        SwDoc* pDoc = createSwDoc(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.docx");
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        SwFlyFrame* pFlyFrame;

        // Before unfloating we have only one page with a fly frame
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                         pWrtShell->GetLayout()->GetLower()->GetType());
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                                   !pWrtShell->GetLayout()->GetLower()->GetNext());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                sFailureMessage.getStr(), SwFrameType::Txt,
                pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
            const SwSortedObjs* pAnchored
                = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                         pAnchored->size());
            SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];
            pFlyFrame = pAnchoredObj->DynCastFlyFrame();
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        }

        // Select the floating table
        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));

        // Push the unfloat button
        pFlyFrame->ActiveUnfloatButton(pWrtShell);
        Scheduler::ProcessEventsToIdle();

        // After unfloating we have two pages with one table frame on each page
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               pWrtShell->GetLayout()->GetLower()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                     pWrtShell->GetLayout()->GetLower()->GetNext()->GetType());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pWrtShell->GetLayout()->GetLower()->GetNext()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetNext()->GetLower()->GetLower()->GetType());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRTLparaStyle_LocaleArabic)
{
    // New documents, created in RTL locales, were not round-tripping the paragraph style as RTL.
    // Set the locale to "ar" for this test - see preTest() at the top of this file.
    std::unique_ptr<Resetter> const pChanges(preTest("LocaleArabic"));

    createSwDoc(); // new, empty doc - everything defaults to RTL with Arabic locale

    // Save it and load it back.
    reload("Office Open XML Text", "tdf116404_paraStyleFrameDir.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("ParagraphStyles")->getByName("Default Paragraph Style"), uno::UNO_QUERY_THROW);
    // Test the text Direction value for the -none- based paragraph styles
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RTL Writing Mode", sal_Int32(1),
                                 getProperty<sal_Int32>(xPageStyle, "WritingMode"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122893)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105413.fodt");

    // all paragraphs are left-aligned with preset single line spacing
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(i), "ParaAdjust"));
        dispatchCommand(mxComponent, ".uno:SpacePara1", {});
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Set center-aligned paragraph with preset double line spacing in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    dispatchCommand(mxComponent, ".uno:CenterPara", {});
    dispatchCommand(mxComponent, ".uno:SpacePara2", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200),
                         getProperty<style::LineSpacing>(getParagraph(3), "ParaLineSpacing")
                             .Height); // double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(getParagraph(2), "ParaLineSpacing")
                             .Height); // single line spacing
    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(200), getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing").Height);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122901)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105413.fodt");

    // all paragraphs with zero borders
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaTopMargin"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaBottomMargin"));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Increase paragraph borders in the 3th paragraph, similar to the default icon of the UI
    // "Increase Paragraph Spacing". Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    dispatchCommand(mxComponent, ".uno:ParaspaceIncrease", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));

    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122942)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf122942.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Do the moral equivalent of mouse button down, move and up.
    // Start creating a custom shape that overlaps with the rounded rectangle
    // already present in the document.
    Point aStartPos(8000, 3000);
    pWrtShell->BeginCreate(OBJ_CUSTOMSHAPE, aStartPos);

    // Set its size.
    Point aMovePos(10000, 5000);
    pWrtShell->MoveCreate(aMovePos);

    // Finish creation.
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Make sure that the shape is inserted.
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());

    reload("writer8", "tdf122942.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pDoc = pWrtShell->GetDoc();
    const SwFrameFormats& rFormats2 = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats2.size());

    // Make sure the top of the inserted shape does not move outside the existing shape, even after
    // reload.
    SdrObject* pObject1 = rFormats2[0]->FindSdrObject();
    CPPUNIT_ASSERT(pObject1);
    const tools::Rectangle& rOutRect1 = pObject1->GetLastBoundRect();
    SdrObject* pObject2 = rFormats2[1]->FindSdrObject();
    CPPUNIT_ASSERT(pObject2);
    const tools::Rectangle& rOutRect2 = pObject2->GetLastBoundRect();
    CPPUNIT_ASSERT(rOutRect2.Top() > rOutRect1.Top());
    CPPUNIT_ASSERT(rOutRect2.Top() < rOutRect1.Bottom());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf132160)
{
    createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // this would crash due to delete redline starting with ToX
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // this would crash due to insert redline ending on table node
    dispatchCommand(mxComponent, ".uno:Undo", {});

    dispatchCommand(mxComponent, ".uno:Redo", {});

    dispatchCommand(mxComponent, ".uno:Undo", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137526)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word
    dispatchCommand(mxComponent, ".uno:WordRightSel", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137684)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word letter by letter
    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Delete", {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // this would crash due to bad redline range
    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption "));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137503)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete the first two paragraphs
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("The"));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption "));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("The"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138605)
{
    SwDoc* const pDoc(createSwDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // insert a word, delete it with change tracking and try to undo it
    pWrtShell->Insert("word");
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    // this crashed due to bad access to the empty redline table
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // more Undo
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("word"));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith(""));
    dispatchCommand(mxComponent, ".uno:Undo", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138135)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word letter by letter by using backspace
    dispatchCommand(mxComponent, ".uno:GoToNextWord", {});

    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // TODO group redlines for managing tracked changes/showing in margin
    for (int i = 0; i <= 10; ++i)
        dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf52391)
{
    createSwDoc(DATA_DIRECTORY, "tdf52391.fodt");

    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // this was "Portion1", because the tracked background color of Portion1 was
    // accepted for "Reject All". Now rejection clears formatting of the text
    // in format-only changes, concatenating the text portions in the first paragraph.
    CPPUNIT_ASSERT_EQUAL(OUString("Portion1Portion2"), xRun->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137771)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // delete a word at the end of the paragraph.
    dispatchCommand(mxComponent, ".uno:GotoEndOfPara", {});
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    }

    CPPUNIT_ASSERT(getParagraph(1)->getString().endsWith("to be "));

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This would be 5 without the new vertical redline mark
    assertXPath(pXmlDoc, "/metafile/push/push/push/line", 6);

    // This was the content of the next <text> (missing deletion on margin)
    // or only the first character of the deleted character sequence
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[9]/text", " saved.");

    // this would crash due to bad redline range
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().endsWith("to be saved."));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf142130)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf142130.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 6 (bad crossing out of the first, not deleted image)
    // (4 lines = 2 lines for crossing out of the second image + 2 lines =
    // vertical "changed line" indicator before the two paragraph lines)
    assertXPath(pXmlDoc, "/metafile/push/push/push/line", 4);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/linecolor", 5);
    // tdf#142128 This was 3 (NON_PRINTING_CHARACTER_COLOR = #268bd2)
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/linecolor[@color='#268bd2']", 0);

    // reject deletion of the second image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);
    // no crossing out and vertical redlines
    assertXPath(pXmlDoc2, "/metafile/push/push/push/line", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf142196)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf142196.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 1 (missing crossing out of the deleted image)
    // (2 lines = crossing out of the deleted image + 1 line for the
    // vertical "changed line" indicator before the paragraph line)
    assertXPath(pXmlDoc, "//line", 3);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor", 1);
    // tdf#142128 This was NON_PRINTING_CHARACTER_COLOR (#268bd2)
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor[@color='#268bd2']", 0);

    // reject deletion of the image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);

    // no crossing out and vertical "changed line" indicator
    assertXPath(pXmlDoc2, "//line", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf142700)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf142700.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // (2 lines = crossing out of the deleted image + 1 line for the
    // vertical "changed line" indicator before the paragraph line)
    assertXPath(pXmlDoc, "//line", 3);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor", 1);
    // tdf#142128 This was NON_PRINTING_CHARACTER_COLOR (#268bd2)
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor[@color='#268bd2']", 0);

    // reject deletion of the image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);

    // no crossing out and vertical "changed line" indicator
    // This was 2 (not removed strikethrough)
    assertXPath(pXmlDoc2, "//line", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139120)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // delete paragraph break
    dispatchCommand(mxComponent, ".uno:GotoEndOfPara", {});
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Delete", {});
    }

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum sit amet."), pTextDoc->getText()->getString());

    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});
    }

    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit amet."), getParagraph(2)->getString());

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was the 3, containing the text "$2" instead of nothing
    assertXPath(pXmlDoc, "/metafile/push/push/push/textarray", 2);

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testJoinParaChangesInMargin)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // delete a character and the paragraph break at the end of the paragraph
    dispatchCommand(mxComponent, ".uno:GotoEndOfPara", {});
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsudolor sit amet."), getParagraph(1)->getString());

    // Undo
    dispatchCommand(mxComponent, ".uno:Undo", {});
    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140757)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // delete a character in the first paragraph, and another character in the second one
    dispatchCommand(mxComponent, ".uno:Delete", {});
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});

    CPPUNIT_ASSERT_EQUAL(OUString("orem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("olor sit amet."), getParagraph(2)->getString());

    // accept all changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/true);

    CPPUNIT_ASSERT_EQUAL(OUString("orem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("olor sit amet."), getParagraph(2)->getString());

    // This crashed
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // Check result of Undo
    rIDRA.AcceptAllRedline(/*bAccept=*/false);
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit amet."), getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139127)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf139127.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // delete the last two characters with a page break at the end of the document
    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(OUString("First page"), pTextDoc->getText()->getString());

    // Undo
    dispatchCommand(mxComponent, ".uno:Undo", {});
    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(OUString("First page"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138479)
{
    SwDoc* const pDoc = createSwDoc();

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("Lorem");
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), getParagraph(1)->getString());

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // delete "r" in "Lorem"
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Loem"), getParagraph(1)->getString());

    // delete "oe" in "Loem"
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lm"), getParagraph(1)->getString());

    // test embedded Undo in ChangesInMargin mode
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Loem"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), getParagraph(1)->getString());

    // this would crash due to bad redline range
    for (int i = 0; i < 5; ++i)
        dispatchCommand(mxComponent, ".uno:Undo", {});

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138666)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit"), getParagraph(2)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(OUString("Loremm"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolsit"), getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit"), getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140982)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115815.odt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum dolor sit amet..."), getParagraph(1)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem  amet..."), getParagraph(1)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum dolor sit amet..."), getParagraph(1)->getString());

    // Save it and load it back.
    reload("writer8", "tdf115815.odt");

    // Test comment range feature on tracked deletion.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    bool bAnnotationStart = false;
    bool bBeforeAnnotation = true;
    OUString sTextBeforeAnnotation;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType = getProperty<OUString>(xPropertySet, "TextPortionType");
        // there is no AnnotationEnd with preceding AnnotationStart,
        // i.e. annotation with lost range
        CPPUNIT_ASSERT(aType != "AnnotationEnd" || !bAnnotationStart);

        bAnnotationStart = (aType == "Annotation");

        // collect paragraph text before the first annotation
        if (bBeforeAnnotation)
        {
            if (bAnnotationStart)
                bBeforeAnnotation = false;
            else if (aType == "Text")
            {
                uno::Reference<text::XTextRange> xRun(xPropertySet, uno::UNO_QUERY);
                sTextBeforeAnnotation += xRun->getString();
            }
        }
    }

    // This was "Lorem ipsum" (collapsed annotation range)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), sTextBeforeAnnotation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126206)
{
    createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    // normal text (it was bold)
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 4)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 3)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf50447)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // remove bold formatting with change tracking
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Bold", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf143918)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // remove bold formatting with change tracking and after that, apply underline, too
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Bold", {});
    dispatchCommand(mxComponent, ".uno:Underline", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        // This was NORMAL (only underlining was removed)
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf143938)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // select first paragraph, add underline without change tracking
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:Underline", {});

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // apply italic with change tracking
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Italic", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        // (first empty run is associated to the redline)
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // no italic, but still underline direct formatting
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        // This wasn't underlined (lost direct formatting)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf143939)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // positionate the text cursor inside the first word
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // remove bold formatting with change tracking without selection
    dispatchCommand(mxComponent, ".uno:Bold", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf50447_keep_hints)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf50447.fodt");

    // first paragraph (_Lorem_ /ipsum/)

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("dolor"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" sit "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("amet"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // modify character formatting of the all the text

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SuperScript", {});

    // multiple format redlines for the multiple hints

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(7), pEditShell->GetRedlineCount());

    // reject tracked changes

    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // all hints and text portions between them got back the original formatting

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("dolor"), xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" sit "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("amet"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144272)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf50447.fodt");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph style of the first paragraph: Heading 1, second paragraph: Standard

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // modify with track changes: Standard and Heading 2

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Standard")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);
    pWrtShell->Down(/*bSelect=*/false);
    aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 2")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // This was Standard (missing reject)
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    // This was Heading 2 (missing reject)
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101873)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert some content.
    pWrtShell->Insert("something");

    // Search for something which does not exist, twice.
    uno::Sequence<beans::PropertyValue> aFirst(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("fig")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);

    uno::Sequence<beans::PropertyValue> aSecond(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("something")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSecond);

    // Without the accompanying fix in place, this test would have failed with "Expected: something;
    // Actual:", i.e. searching for "something" failed, even if it was inserted above.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("something"), pShellCursor->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTextFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a text form field
    dispatchCommand(mxComponent, ".uno:TextFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMTEXT), pFieldmark->GetFieldname());

    // The text form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT(xPara->getString().isEmpty());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testCheckboxFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a checkbox form field
    dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
    // The checkbox is not checked by default
    ::sw::mark::ICheckboxFieldmark* pCheckBox
        = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(pFieldmark);
    CPPUNIT_ASSERT(pCheckBox);
    CPPUNIT_ASSERT(!pCheckBox->IsChecked());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDropDownFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a drop-down form field
    dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
    // Check drop down field's parameters. By default these params are not set
    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries == pParameters->end()));
    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    CPPUNIT_ASSERT(bool(pResult == pParameters->end()));

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testMixedFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert fields
    dispatchCommand(mxComponent, ".uno:TextFormField", {});
    dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf124261)
{
#if !defined(_WIN32)
    // Make sure that pressing a key in a btlr cell frame causes an immediate, correct repaint.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf124261.docx");
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPageFrame = pLayout->GetLower();
    CPPUNIT_ASSERT(pPageFrame->IsPageFrame());

    SwFrame* pBodyFrame = pPageFrame->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsBodyFrame());

    SwFrame* pTabFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pTabFrame->IsTabFrame());

    SwFrame* pRowFrame = pTabFrame->GetLower();
    CPPUNIT_ASSERT(pRowFrame->IsRowFrame());

    SwFrame* pCellFrame = pRowFrame->GetLower();
    CPPUNIT_ASSERT(pCellFrame->IsCellFrame());

    SwFrame* pFrame = pCellFrame->GetLower();
    CPPUNIT_ASSERT(pFrame->IsTextFrame());

    // Make sure that the text frame's area and the paint rectangle match.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1721;
    // Actual: 1547', i.e. an area other than the text frame was invalidated for a single-line
    // paragraph.
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pFrame);
    SwRect aRect = pTextFrame->GetPaintSwRect();
    CPPUNIT_ASSERT_EQUAL(pTextFrame->getFrameArea().Top(), aRect.Top());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDocxAttributeTableExport)
{
    createSwDoc(DATA_DIRECTORY, "floating-table-position.docx");

    // get the table frame, set new values and dismiss the references
    {
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

        // change the properties
        // 8133 -> 8000
        xShape->setPropertyValue("VertOrientPosition", uno::makeAny(static_cast<sal_Int32>(8000)));
        // 5964 -> 5000
        xShape->setPropertyValue("HoriOrientPosition", uno::makeAny(static_cast<sal_Int32>(5000)));
        // 0 (frame) -> 8 (page print area)
        xShape->setPropertyValue("VertOrientRelation", uno::makeAny(static_cast<sal_Int16>(8)));
        // 8 (page print area) -> 0 (frame)
        xShape->setPropertyValue("HoriOrientRelation", uno::makeAny(static_cast<sal_Int16>(0)));
    }
    // save it to docx
    reload("Office Open XML Text", "floating-table-position.docx");

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

    // test the new values
    sal_Int32 nValue = getProperty<sal_Int32>(xShape, "VertOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(7999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(8001));
    nValue = getProperty<sal_Int32>(xShape, "HoriOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(4999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(5001));

    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf118699_redline_numbering)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118699.docx");

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    uno::Reference<beans::XPropertySet> xProps(getParagraph(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    CPPUNIT_ASSERT_MESSAGE(
        "first paragraph after the second deletion: missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(5), "NumberingRules")
            .is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125881_redline_list_level)
{
    createSwDoc(DATA_DIRECTORY, "tdf125881.docx");

    uno::Reference<beans::XPropertySet> xProps(getParagraph(8), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("deleted paragraph: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    // deleted paragraph gets the numbering of the next paragraph
    uno::Reference<beans::XPropertySet> xProps2(getParagraph(9), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: missing numbering",
                           xProps2->getPropertyValue("NumberingRules").hasValue());

    // check numbering level at deletion (1 instead of 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(getParagraph(9), "NumberingLevel"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125916_redline_restart_numbering)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125916.docx");

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // check unnecessary numbering
    uno::Reference<beans::XPropertySet> xProps(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125310)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125310.fodt");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph join
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // copied paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // without copying the page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125310b)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125310b.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // remove second paragraph with the page break
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // losing the page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120336)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf120336.docx");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // keep page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf106843)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf106843.docx");

    // try to turn off red-lining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // but the protection doesn't allow it
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testImageComment)
{
    // Load a document with an as-char image in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image-comment.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // Test document has "before<image>after", remove the content before the image.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as FN_POSTIT was disabled
    // in the frame shell.
    // Then this test would have failed, as in case the as-char anchored image was at the start of
    // the paragraph, the comment of the image covered the character after the image, not the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

#if !defined(MACOSX)
    // Calc the left edge of the as-char frame.
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTextFrame = pBody->GetLower();
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    const SwSortedObjs& rAnchored = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_GREATER(static_cast<size_t>(0), rAnchored.size());
    SwAnchoredObject* pObject = rAnchored[0];
    tools::Long nFrameLeft = pObject->GetObjRect().Left();
    tools::Long nFrameTop = pObject->GetObjRect().Top();

    // Make sure that the anchor points to the bottom left corner of the image.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 1418
    // - Actual: 2442
    // The anchor pointed to the bottom right corner, so as-char and at-char was inconsistent.
    Scheduler::ProcessEventsToIdle();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (const auto& pItem : *pPostItMgr)
    {
        const SwRect& rAnchor = pItem->mpPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_EQUAL(nFrameLeft, rAnchor.Left());
    }

    // Test the comment anchor we expose via the LOK API.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1418, 1418, 0, 0
    // - Actual  : 1418, 1418, 1024, 1024
    // I.e. the anchor position had a non-empty size, which meant different rendering via tiled
    // rendering and on the desktop.
    tools::JsonWriter aJsonWriter;
    pTextDoc->getPostIts(aJsonWriter);
    char* pChar = aJsonWriter.extractData();
    std::stringstream aStream(pChar);
    free(pChar);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;
        OString aAnchorPos(rComment.get<std::string>("anchorPos").c_str());
        OString aExpected
            = OString::number(nFrameLeft) + ", " + OString::number(nFrameTop) + ", 0, 0";
        CPPUNIT_ASSERT_EQUAL(aExpected, aAnchorPos);
    }

#endif

    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testImageCommentAtChar)
{
    // Load a document with an at-char image in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image-comment-at-char.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as the comment was
    // anchored at the end of the paragraph, it was not around the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());

    // Undo the deletion and move the image down, so the anchor changes.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aNewAnchor = pWrtShell->GetFlyRect().TopLeft();
    aNewAnchor.Move(0, 600);
    pWrtShell->SetFlyPos(aNewAnchor);

    // Get the image anchor doc model position.
    SwFlyFrame* pFly = pWrtShell->GetCurrFlyFrame(false);
    CPPUNIT_ASSERT(pFly);
    SwFrameFormat& rFlyFormat = pFly->GetFrameFormat();
    const SwPosition* pImageAnchor = rFlyFormat.GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pImageAnchor);

    // Get the annotation mark doc model start.
    auto it = pDoc->getIDocumentMarkAccess()->getAnnotationMarksBegin();
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAnnotationMarksEnd());
    const sw::mark::IMark* pMark = *it;
    const SwPosition& rAnnotationMarkStart = pMark->GetMarkPos();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: SwPosition (node 14, offset 15)
    // - Actual  : SwPosition (node 12, offset 3)
    // This means moving the image anchor did not move the comment anchor / annotation mark, so the
    // image and its comment got out of sync.
    CPPUNIT_ASSERT_EQUAL(*pImageAnchor, rAnnotationMarkStart);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTrackImageDeletion)
{
    // load a document with an image anchored to paragraph in it
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // select the image
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // now delete the image with track changes
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of deletion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // tdf#142701 this was AS_CHARACTER (convert AT_PARA to AT_CHAR to keep the layout)
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTrackImageInsertion)
{
    SwDoc* pDoc = createSwDoc();

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Insert an image with change tracking
    OUString aImageURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ole2.png";
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", aImageURL),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of insertion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120338)
{
    createSwDoc(DATA_DIRECTORY, "tdf120338.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // right

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));

    // reject tracked paragraph adjustments
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // left

    // tdf#126245 revert numbering changes
    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    // tdf#126243 revert paragraph styles
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120338_multiple_paragraph_join)
{
    createSwDoc(DATA_DIRECTORY, "redline-para-join.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // reject tracked paragraph styles
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testShapePageMove)
{
    // Load a document with 2 pages, shape on the first page.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-page-move.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Make sure that the 2nd page is below the 1st one.
    pView->SetViewLayout(/*nColumns=*/1, /*bBookMode=*/false);
    calcLayout();

    // Select the shape.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    // Move the shape down to the 2nd page.
    SfxInt32Item aXItem(SID_ATTR_TRANSFORM_POS_X, 4000);
    SfxInt32Item aYItem(SID_ATTR_TRANSFORM_POS_Y, 12000);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM, SfxCallMode::SYNCHRON,
                                                        { &aXItem, &aYItem });

    // Check if the shape anchor was moved to the 2nd page as well.
    SwFrameFormats* pShapeFormats = pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!pShapeFormats->empty());
    auto it = pShapeFormats->begin();
    SwFrameFormat* pShapeFormat = *it;
    const SwPosition* pAnchor = pShapeFormat->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pAnchor);

    // Find out the node index of the 1st para on the 2nd page.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pFirstPage = pLayout->Lower();
    SwFrame* pSecondPage = pFirstPage->GetNext();
    CPPUNIT_ASSERT(pSecondPage->IsLayoutFrame());
    SwFrame* pBodyFrame = static_cast<SwLayoutFrame*>(pSecondPage)->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsLayoutFrame());
    SwFrame* pTextFrame = static_cast<SwLayoutFrame*>(pBodyFrame)->GetLower();
    CPPUNIT_ASSERT(pTextFrame->IsTextFrame());
    SwNodeOffset nNodeIndex = static_cast<SwTextFrame*>(pTextFrame)->GetTextNodeFirst()->GetIndex();

    // Without the accompanying fix in place, this test would have failed with "Expected: 13;
    // Actual: 12", i.e. the shape was anchored to the last paragraph of the 1st page, not to a
    // paragraph on the 2nd page.
    CPPUNIT_ASSERT_EQUAL(nNodeIndex, pAnchor->nNode.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The date form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldContentOperations)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Check the default content added by insertion
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());

    // Set content to empty string
    pFieldmark->ReplaceContent("");
    CPPUNIT_ASSERT_EQUAL(OUString(""), pFieldmark->GetContent());

    // Replace empty string with a valid content
    pFieldmark->ReplaceContent("2019-10-23");
    CPPUNIT_ASSERT_EQUAL(OUString("2019-10-23"), pFieldmark->GetContent());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldCurrentDateHandling)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The default content is not a valid date
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check empty string
    pFieldmark->ReplaceContent("");
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check valid date
    // Set date format first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= OUString("YYYY/MM/DD");
    (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= OUString("en-US");

    // Set date value and check whether the content is formatted correctly
    pFieldmark->SetCurrentDate(48000.0);
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);
    CPPUNIT_ASSERT_EQUAL(OUString("2031/06/01"), pFieldmark->GetContent());
    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);
}

#if !defined(_WIN32)
CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldCurrentDateInvalidation)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Set a date first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    pFieldmark->SetCurrentDate(48000.0);
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);

    // Do the layouting to trigger invalidation
    // Since we have the current date consistent with the field content
    // This invalidation won't change anything
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    // We have the current date parameter set
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);

    // Now change the content of the field
    pFieldmark->ReplaceContent("[select date]");
    // Do the layouting to trigger invalidation
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    sCurrentDate.clear();
    pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString(""), sCurrentDate);
}
#endif

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testOleSaveWhileEdit)
{
    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "ole-save-while-edit.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);

    // Select the frame and switch to the frame shell.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->StopShellTimer();

    // Start editing the OLE object.
    pWrtShell->LaunchOLEObj();

    // Save the document without existing the OLE edit.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // Without the accompanying fix in place, this test would have failed: the OLE object lost its
    // replacement on save if the edit was active while saving.
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105330)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105330.odt");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxUInt16Item aRows(SID_ATTR_TABLE_ROW, 1);
    SfxUInt16Item aColumns(SID_ATTR_TABLE_COLUMN, 1);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_TABLE, SfxCallMode::SYNCHRON,
                                                        { &aRows, &aColumns });

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    // Without the accompanying fix in place, height was only 1 twips (practically invisible).
    // Require at least 12pt height (font size under the cursor), in twips.
    CPPUNIT_ASSERT_GREATEREQUAL(
        static_cast<tools::Long>(240),
        pWrtShell->GetVisibleCursor()->GetTextCursor().GetSize().getHeight());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf118311)
{
    createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // Jump to the first cell, selecting its content
    uno::Sequence<beans::PropertyValue> aSearch(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("a")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSearch);

    //  .uno:Cut doesn't remove the table, only the selected content of the first cell
    dispatchCommand(mxComponent, ".uno:Cut", {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // .uno:SelectAll selects the whole table, and UNO command Cut cuts it
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineTableRowDeletion)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // This was deleted without change tracking
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);

    // Undo, and repeat the previous test, but only with deletion of the text content of the cells
    // (HasTextChangesOnly property will be removed by Undo)

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // table exists again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // Table row still exists
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) still exists
    // (HasTextChangesOnly property wasn't set for table row deletion)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Undo, and delete the row without change tracking

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // table exists again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // disable change tracking
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table row without change tracking
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // the table (row) was deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineTableRowDeletionWithExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    reload("writer8", "tdf60382_tracked_table_deletion.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineTableRowDeletionWithDOCXExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it to a DOCX and load it back.
    // Exporting change tracking of the row wasn't supported.
    // Also Manage Changes for the import.
    reload("Office Open XML Text", "tdf79069_tracked_table_deletion.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineDOCXTableInsertion)
{
    // load a 3-row table inserted with change tracking by text to table conversion
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-converttotable.docx");

    // check table count (1)
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // reject the text insertions of the table cells (also reject deletion of the tabulated
    // text source of the table, which was used by the tracked text to table conversion)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(10), pEditShell->GetRedlineCount());
    while (pEditShell->GetRedlineCount())
        pEditShell->RejectRedline(0);

    // rejecting all text insertions must undo the table insertion
    // This was 1 (remaining empty table after rejecting all table text insertions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineDOCXTableMoveToFrame)
{
    // load a table with tracked drag & drop: Table1 is the moveFrom,
    // Table2 is the moveTo - and framed - table
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-DnD-move.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableNames = xTextTablesSupplier->getTextTables();
    // check table count (2)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());

    // accept tracked table moving, remaining table is Table2
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName("Table2"));
    CPPUNIT_ASSERT(!xTableNames->hasByName("Table1"));

    // Undo and reject tracked table moving, remaining table is Table1
    dispatchCommand(mxComponent, ".uno:Undo", {});
    rIDRA.AcceptAllRedline(false);
    // This was 2 (not deleted Table2 – framed)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName("Table1"));
    CPPUNIT_ASSERT(!xTableNames->hasByName("Table2"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf143215)
{
    // load a table with tracked insertion of an empty row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-rowadd.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // check table count
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // reject insertion of the empty table row
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());

    // Undo and accept insertion of the table row
    dispatchCommand(mxComponent, ".uno:Undo", {});
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // delete it with change tracking, and accept the deletion
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());
    rIDRA.AcceptAllRedline(true);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144748)
{
    // load a table with an empty row, and an empty line before the table
    // (to allow the easy selection of the full text with the table)
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144748.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // there is a table in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // delete full text with the table and check Undo

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    // this crashed LibreOffice
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // redo and check redline usage

    dispatchCommand(mxComponent, ".uno:Redo", {});
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 2 (bad extra redline for the empty row of the deleted table)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    // accept deletion of the text, including the table with the empty row

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // no table left in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf128335)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128335.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Select all shapes.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(xModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xSelectionSupplier->select(pXTextDocument->getDrawPages()->getByIndex(0));

    // Cut them.
    // Without the accompanying fix in place, this test would have crashed as the textboxes were
    // deleted in an incorrect order.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineTableRowDeletionWithReject)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    reload("writer8", "tdf60382_tracked_table_deletion.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // reject the deletion of the content of the first cell
    // HasTextChangesOnly property of the table row will be true
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);

    // Select and delete the content of the first cell
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // This was table row deletion instead of remaining the empty row
    // (HasTextChangesOnly was false)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // restore HasTextChangesOnly = false
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineTableRowInsertionWithReject)
{
    // load a 1-row table, and insert a row with enabled change tracking
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table and its single row
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // insert rows before and after with enabled change tracking
    // (HasTextChangesOnly property of the row will be false, and
    // add dummy characters CH_TXT_TRACKED_DUMMY_CHAR)
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 0 (not tracked row insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // reject redlines
    pEditShell->RejectRedline(0);
    pEditShell->RejectRedline(0);

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // This was 3 (not rejected row insertion)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf128603)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128603.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Undo all of this.
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // Make sure the content indexes still match.
    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), rSpzFrameFormats.size());
    const SwNodeIndex* pIndex4 = rSpzFrameFormats[4]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex4);
    const SwNodeIndex* pIndex5 = rSpzFrameFormats[5]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex5);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11
    // - Actual  : 14
    // i.e. the shape content index and the frame content index did not match after undo, even if
    // their "other text box format" pointers pointed to each other.
    CPPUNIT_ASSERT_EQUAL(pIndex4->GetIndex(), pIndex5->GetIndex());
}

// only care that it doesn't assert/crash
CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testOfz18563)
{
    OUString sURL(m_directories.getURLFromSrc(u"/sw/qa/extras/uiwriter/data/ofz18563.docx"));
    SvFileStream aFileStream(sURL, StreamMode::READ);
    TestImportDOCX(aFileStream);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf143904)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf143904.odt");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    pWrtShell->Down(false);
    pWrtShell->Insert("foo");

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(false, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"), sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf90069)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf90069.docx");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    pWrtShell->Down(false);
    pWrtShell->Insert("foo");

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(true, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(OUString("Lohit Devanagari"), sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109266)
{
    // transliteration with redlining
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "lorem.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNode = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);

    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetRedlineText());

    //turn on red-lining and show changes
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    // This was "Lorem Ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsumIpsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToUpper", {});

    // This was "LOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...LOREM IPSUM..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("LOREM IPSUM..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToLower", {});

    // This was "lorem ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...lorem ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("lorem ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToToggleCase", {});

    // This was "lOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...lOREM IPSUM..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("lOREM IPSUM..."), pTextNode->GetRedlineText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf129655)
{
    createSwDoc(DATA_DIRECTORY, "tdf129655-vtextbox.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/fly/txt[@WritingMode='Vertical']", 1);
}

static uno::Reference<text::XTextRange> getAssociatedTextRange(uno::Any object)
{
    // possible cases:
    // 1. a container of other objects - e.g. selection of 0 to n text portions, or 1 to n drawing objects
    try
    {
        uno::Reference<container::XIndexAccess> xIndexAccess(object, uno::UNO_QUERY_THROW);
        if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
        {
            for (int i = 0; i < xIndexAccess->getCount(); ++i)
            {
                uno::Reference<text::XTextRange> xRange
                    = getAssociatedTextRange(xIndexAccess->getByIndex(i));
                if (xRange.is())
                    return xRange;
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    // 2. another TextContent, having an anchor we can use
    try
    {
        uno::Reference<text::XTextContent> xTextContent(object, uno::UNO_QUERY_THROW);
        if (xTextContent.is())
        {
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            if (xRange.is())
                return xRange;
        }
    }
    catch (const uno::Exception&)
    {
    }

    // an object which supports XTextRange directly
    try
    {
        uno::Reference<text::XTextRange> xRange(object, uno::UNO_QUERY_THROW);
        if (xRange.is())
            return xRange;
    }
    catch (const uno::Exception&)
    {
    }

    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf123218)
{
    struct ReverseXAxisOrientationDoughnutChart
        : public comphelper::ConfigurationProperty<ReverseXAxisOrientationDoughnutChart, bool>
    {
        static OUString path()
        {
            return "/org.openoffice.Office.Compatibility/View/ReverseXAxisOrientationDoughnutChart";
        }
        ~ReverseXAxisOrientationDoughnutChart() = delete;
    };

    struct ClockwisePieChartDirection
        : public comphelper::ConfigurationProperty<ClockwisePieChartDirection, bool>
    {
        static OUString path()
        {
            return "/org.openoffice.Office.Compatibility/View/ClockwisePieChartDirection";
        }
        ~ClockwisePieChartDirection() = delete;
    };
    auto batch = comphelper::ConfigurationChanges::create();

    ReverseXAxisOrientationDoughnutChart::set(false, batch);
    ClockwisePieChartDirection::set(true, batch);
    batch->commit();

    createSwDoc();

    // create an OLE shape in the document
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xMSF);
    uno::Reference<beans::XPropertySet> xShapeProps(
        xMSF->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY);
    xShapeProps->setPropertyValue("CLSID",
                                  uno::makeAny(OUString("12dcae26-281f-416f-a234-c3086127382e")));
    uno::Reference<drawing::XShape> xShape(xShapeProps, uno::UNO_QUERY_THROW);
    xShape->setSize(awt::Size(16000, 9000));
    uno::Reference<text::XTextContent> chartTextContent(xShapeProps, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    xTextRange->getText()->insertTextContent(xTextRange, chartTextContent, false);

    // insert a doughnut chart
    uno::Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel);
    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc);
    uno::Reference<lang::XMultiServiceFactory> xChartMSF(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartMSF);
    uno::Reference<chart::XDiagram> xDiagram(
        xChartMSF->createInstance("com.sun.star.chart.DonutDiagram"), uno::UNO_QUERY);
    xChartDoc->setDiagram(xDiagram);

    // test primary X axis Orientation value
    uno::Reference<chart2::XChartDocument> xChartDoc2(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc2);
    uno::Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(
        xChartDoc2->getFirstDiagram(), uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> xCooSysSequence
        = xCooSysContainer->getCoordinateSystems();
    uno::Reference<chart2::XCoordinateSystem> xCoord = xCooSysSequence[0];
    CPPUNIT_ASSERT(xCoord.is());
    uno::Reference<chart2::XAxis> xAxis = xCoord->getAxisByDimension(0, 0);
    CPPUNIT_ASSERT(xAxis.is());
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_MATHEMATICAL, aScaleData.Orientation);

    // tdf#108059 test primary Y axis Orientation value
    uno::Reference<chart2::XAxis> xYAxis = xCoord->getAxisByDimension(1, 0);
    CPPUNIT_ASSERT(xYAxis.is());
    aScaleData = xYAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_REVERSE, aScaleData.Orientation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126735)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check next selected tracked change
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(OUString(" ipsu"), xTextRange->getString());

    // check next selected tracked change
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(OUString("or "), xTextRange->getString());

    // check next selected tracked change at the end of the document:
    // select the first tracked change of the document
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the end of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(OUString(" ipsu"), xTextRange->getString());

    // check the previous tracked change at the start of the document:
    // select the last tracked change of the document
    dispatchCommand(mxComponent, ".uno:PreviousTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the start of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(OUString("or "), xTextRange->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
