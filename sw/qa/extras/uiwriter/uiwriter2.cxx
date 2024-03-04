/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <view.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <fmtanchr.hxx>
#include <UndoManager.hxx>
#include <sortedobjs.hxx>
#include <itabenum.hxx>
#include <fmtfsize.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/lrspitem.hxx>
#include <swacorr.hxx>
#include <redline.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    SwUiWriterTest2()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf47471_paraStyleBackground)
{
    createSwDoc("tdf47471_paraStyleBackground.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(Color(0xe0c2cd), getProperty<Color>(getParagraph(2), "FillColor"));

    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:ResetAttributes", {});

    // the background color should revert to the color for 00Background style
    CPPUNIT_ASSERT_EQUAL(Color(0xdedce6), getProperty<Color>(getParagraph(2), "FillColor"));
    // the paragraph style should not be reset
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // Save it and load it back.
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(Color(0xdedce6), getProperty<Color>(getParagraph(2), "FillColor"));
    // on round-trip, the paragraph style name was lost
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdfChangeNumberingListAutoFormat)
{
    createSwDoc("tdf117923.docx");
    SwDoc* pDoc = getSwDoc();

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]"_ostr, "GHI GHI GHI GHI");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "type"_ostr, "PortionType::Number");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "expand"_ostr, "2.");
    // The numbering height was 960 in DOC format.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Grow", {});
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "260");

    // save it to DOCX
    saveAndReload("Office Open XML Text");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // this was 220
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "260");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101534)
{
    // Copy the first paragraph of the document.
    createSwDoc("tdf101534.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(),
                    svl::Items<RES_MARGIN_FIRSTLINE, RES_MARGIN_TEXTLEFT>);
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(!aSet.HasItem(RES_MARGIN_FIRSTLINE));
    CPPUNIT_ASSERT(aSet.HasItem(RES_MARGIN_TEXTLEFT));
    CPPUNIT_ASSERT_EQUAL(::tools::Long(0), aSet.GetItem(RES_MARGIN_TEXTLEFT)->GetTextLeft());

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:Paste", {});
    aSet.ClearItem();
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(!aSet.HasItem(RES_MARGIN_FIRSTLINE));
    CPPUNIT_ASSERT(aSet.HasItem(RES_MARGIN_TEXTLEFT));
    CPPUNIT_ASSERT_EQUAL(::tools::Long(0), aSet.GetItem(RES_MARGIN_TEXTLEFT)->GetTextLeft());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testExtendedSelectAllHang)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxStringItem aLangString(SID_LANGUAGE_STATUS, "Default_Spanish (Bolivia)");
    // this looped
    pView->GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON,
                                                       { &aLangString });
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineMoveInsertInDelete)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete(); // the footnote
    // second delete redline, following the first one
    pWrtShell->EndOfSection(false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    pWrtShell->Delete(); // "foo"

    // hiding used to copy the 2nd delete redline "foo", but not delete it
    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide
    CPPUNIT_ASSERT_EQUAL(OUString(" "),
                         pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetText());
    pWrtShell->SetRedlineFlags(mode); // show again
    CPPUNIT_ASSERT_EQUAL(u"\u0001 foo"_ustr,
                         pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineInHiddenSection)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
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
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->StartOfSection();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    createSwDoc("try2.fodt");
    SwDoc* pDoc = getSwDoc();
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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 18, /*bBasicCall=*/false);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137318)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("A");

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->DelLine();
    pWrtShell->StartOfSection(false);
    pWrtShell->SplitNode(true);
    pWrtShell->SplitNode(true);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion"_ostr, 0);
    // not sure why there's an empty text portion here, but it's not a problem
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "portion"_ostr, "");

    pWrtShell->Undo();

    // the problem was that here the "A" showed up again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "portion"_ostr, "");

    pWrtShell->Undo();

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "portion"_ostr, "");

    pWrtShell->Undo();

    // now the "A" is no longer deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[@portion]"_ostr,
                1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "length"_ostr, "1");

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "portion"_ostr, "A");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136704)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf134250.fodt");

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

    dispatchCommand(mxComponent, ".uno:Copy", {});

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());

    // this would crash in 2 different ways
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    // Without the fix in place, section's content would have been gone after undo
    CPPUNIT_ASSERT_EQUAL(OUString("foo" SAL_NEWLINE_STRING "bar"),
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("foo" SAL_NEWLINE_STRING "bar"),
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134436)
{
    createSwDoc("tdf134436.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

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

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    // second, the section does get deleted because point is at the end
    dispatchCommand(mxComponent, ".uno:Delete", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    // the problem was that the section was not deleted on Redo
    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb\n"), pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134252)
{
    createSwDoc("tdf134252.fodt");

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

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());

    // this would crash
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING),
                         xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING),
                         xCursor->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136452)
{
    createSwDoc("tdf136452.fodt");
    SwDoc* pDoc = getSwDoc();

    SwNodeOffset const nNodes(pDoc->GetNodes().Count());

    SwWrtShell* const pWrtShell(pDoc->GetDocShell()->GetWrtShell());

    // first deletion spanning 2 sections
    pWrtShell->SttEndDoc(false);
    pWrtShell->SetMark();
    pWrtShell->Up(true, 2);
    pWrtShell->Delete();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    // second deletion spanning 2 sections
    pWrtShell->SetMark();
    pWrtShell->Up(true, 3);
    pWrtShell->Delete();

    // 3 paragraphs deleted, 2nd section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 9, pDoc->GetNodes().Count());

    pWrtShell->Undo();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    // this crashed
    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    pWrtShell->Redo();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    pWrtShell->Redo();

    // 3 paragraphs deleted, 2nd section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 9, pDoc->GetNodes().Count());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136453)
{
    createSwDoc("tdf136453.fodt");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt"_ostr, 1);

    pWrtShell->Redo();

    // one paragraph deleted, section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 3, pDoc->GetNodes().Count());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section"_ostr, 0);

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137245)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

    const auto& rFormats = *pDoc->GetSpzFrameFormats();
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
    createSwDoc("tdf132236.odt");
    SwDoc* pDoc = getSwDoc();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf131912)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sw::UnoCursorPointer pCursor(
        pDoc->CreateUnoCursor(SwPosition(pDoc->GetNodes().GetEndOfContent(), SwNodeOffset(-1))));

    pDoc->getIDocumentContentOperations().InsertString(*pCursor, "foo");

    {
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        pWrtShell->StartOfSection(false);
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    pCursor->SetMark();
    pCursor->GetMark()->nContent.Assign(pCursor->GetPointContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetPointContentNode(), 3);

    // replace with more text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "blahblah", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetPointContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetPointContentNode(), 3);

    // replace with less text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "x", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetPointContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetPointContentNode(), 3);

    // regex replace with paragraph breaks
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "xyz\\n\\nquux\\n", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetPointNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    // regex replace with paragraph join
    pWrtShell->StartOfSection(false);
    pWrtShell->Down(true);
    pDoc->getIDocumentContentOperations().ReplaceRange(*pWrtShell->GetCursor(), "bar", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetPointNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140007)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("baz");
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(" ", true);
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(" ", true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar baz"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(OUString("foo bar baz"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139982)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // turn on redlining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    pWrtShell->Insert("helloo");

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>);
        flySet.Put(anchor);
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(/*bSelect=*/true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Replace("hello", true);

    // the problem was that a redline delete with the same author as redline
    // insert has its text deleted immediately, including anchored flys.
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf135976)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("foobar");

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>);
    flySet.Put(anchor);
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    // turn on redlining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->UnSelectFrame();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtShell->DelLeft();
    pWrtShell->DelLeft();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    // the problem was that the fly was deleted from the layout
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    // check that the anchor was moved outside the redline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    // check that the anchor was restored
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Redo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetAnchorContentOffset());

    // now again in the other direction:

    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);

    pWrtShell->DelRight();
    pWrtShell->DelRight();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    // the problem was that the fly was deleted from the layout
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Redo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFly->GetAnchor().GetAnchorContentOffset());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetAnchorContentOffset());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf39721)
{
// FIXME: disabled on Windows because of a not reproducible problem (not related to the patch)
#if !defined(_WIN32)
    // check move down with redlining
    createSwDoc("tdf39721.fodt");
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf39721.fodt");
    SwDoc* pDoc = getSwDoc();

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
    // TODO fix unnecessary insertion of a new list item at the end of the document
    CPPUNIT_ASSERT(getParagraphs() >= 3);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // This was "Loremdolsit\namet.\n" (bad deletion of "m\n" at the end of item 1)
    CPPUNIT_ASSERT_EQUAL(OUString("Loremm" SAL_NEWLINE_STRING "dolsit" SAL_NEWLINE_STRING
                                  "amet." SAL_NEWLINE_STRING),
                         pTextDoc->getText()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf145311_move_over_empty_paragraphs)
{
    // check move up/down with redlining: jumping over an empty paragraph
    // resulted bad insertion of the empty paragraph in Show Changes mode
    createSwDoc("tdf145311.fodt");
    SwDoc* pDoc = getSwDoc();

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // 8 paragraphs (list items)
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());

    // move down the first item over the empty paragraph
    for (int i = 0; i < 4; ++i)
        dispatchCommand(mxComponent, ".uno:MoveDown", {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 3 (bad conversion of the empty item to a tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    // check move up

    for (int i = 0; i < 3; ++i)
        dispatchCommand(mxComponent, ".uno:MoveUp", {});

    // This was 3 (bad conversion of the empty item to a tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

    // remaining paragraph keeps its original style
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819_keep_numbering_with_Undo)
{
    createSwDoc("tdf54819b.odt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

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
    createSwDoc("tdf54819b.odt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

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
    createSwDoc("tdf54819b.odt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140077)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    pWrtShell->Insert("a");
    pWrtShell->SplitNode();
    pWrtShell->Insert("b");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    // enable
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Delete();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // crashed in layout
    pWrtShell->SplitNode();

    pWrtShell->Undo();
    pWrtShell->Redo();
    pWrtShell->Undo();
    pWrtShell->Redo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109376_redline)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    pam.GetPoint()->Assign(*rTable.GetTableNode());
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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    pam.GetPoint()->Assign(*rTable.GetTableNode());
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147414)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());

    pWrtShell->Insert("Abc");

    // hide and enable
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // backspace
    pWrtShell->DelLeft();
    pWrtShell->AutoCorrect(corr, u' ');

    // problem was this was 1 i.e. before the deleted "b" while " " was inserted after
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         pWrtShell->getShellCursor(false)->GetPoint()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(
        OUString("Ab c"),
        pWrtShell->getShellCursor(false)->GetPoint()->GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147310)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // somehow bug happens only with 2 tables
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(tableOpt, 1, 1);
    pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->SttEndDoc(/*bStart=*/true);

    pWrtShell->DeleteRow(false);
    pWrtShell->DeleteRow(false);

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/tab"_ostr, 0);
        discardDumpedLayout();
    }
    pWrtShell->Undo();
    // this did not create frames for the table
    pWrtShell->Undo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab"_ostr, 2);
        discardDumpedLayout();
    }
    pWrtShell->Redo();
    pWrtShell->Redo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/tab"_ostr, 0);
        discardDumpedLayout();
    }
    pWrtShell->Undo();
    pWrtShell->Undo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab"_ostr, 2);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf80194_autoSubscript)
{
    createSwDoc();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Automatic mode enabled", false,
        getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoEscapement"));
    dispatchCommand(mxComponent, ".uno:SubScript", {});
    // Writer has always had automatic mode enabled when creating subscript/superscripts.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Automatic mode enabled", true,
        getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoEscapement"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf64242_optimizeTable)
{
    createSwDoc("tdf64242_optimizeTable.odt");
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf45525.odt");
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf126784_distributeSelectedColumns.odt");
    SwDoc* pDoc = getSwDoc();
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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:DistributeColumns", {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  "TableColumnSeparators");
    CPPUNIT_ASSERT_MESSAGE("Second column should shrink", nOrigCol2Pos < aSeq[0].Position);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last column shouldn't change", nOrigCol3Pos, aSeq[1].Position);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144317)
{
    createSwDoc("tdf144317.odt");
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf108687_tabstop.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nStartIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
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
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

    // second paragraph changes its style in "Show changes" mode
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144058)
{
    createSwDoc("tdf144058.fodt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:Cut", {});

    // accept all: tables are deleted
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 2 (remaining empty tables)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147507)
{
    createSwDoc("tdf147507.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // select all, backspace and reject all crashed
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SwBackSpace", {});
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119019)
{
    // check handling of overlapping redlines
    createSwDoc("tdf119019.docx");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

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
    createSwDoc("tdf119019.docx");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Cut", {});

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
    createSwDoc("tdf105413.fodt");
    SwDoc* pDoc = getSwDoc();

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
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Heading 1")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817)
{
    createSwDoc("num-parent-style.docx");
    SwDoc* pDoc = getSwDoc();

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
        { "Style", uno::Any(OUString("Heading 1")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
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
        { "Style", uno::Any(OUString("Heading 2")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
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
    createSwDoc("tdf76817.fodt");

    // save it to DOCX
    saveAndReload("Office Open XML Text");

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
        { "Style", uno::Any(OUString("Heading 1")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
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
        { "Style", uno::Any(OUString("Heading 2")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
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
    createSwDoc("tdf76817.docx");
    SwDoc* pDoc = getSwDoc();

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
        { "Style", uno::Any(OUString("Heading 1")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
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
    createSwDoc("tdf123102.odt");
    // insert a new row after a vertically merged cell
    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "3." - caused by the hidden numbered paragraph of the new merged cell
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row[6]/cell[1]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "expand"_ostr, "2.");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonSmallTable)
{
    // The floating table in the test document is too small, so we don't provide an unfloat button
    createSwDoc("small_floating_table.odt");
    SwDoc* pDoc = getSwDoc();
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
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        createSwDoc(sTestFileName.getStr());
        SwDoc* pDoc = getSwDoc();
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        const SwSortedObjs* pAnchored;
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
    createSwDoc("unfloatable_floating_table.odt");
    SwDoc* pDoc = getSwDoc();
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
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        // Test what happens when pushing the unfloat button
        createSwDoc(sTestFileName.getStr());
        SwDoc* pDoc = getSwDoc();
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
    // Set the locale to "ar" for this test.
    m_aSavedSettings = Application::GetSettings();
    AllSettings aSettings(m_aSavedSettings);
    aSettings.SetLanguageTag(LanguageTag("ar"));
    Application::SetSettings(aSettings);
    comphelper::ScopeGuard g([this] { Application::SetSettings(this->m_aSavedSettings); });

    createSwDoc(); // new, empty doc - everything defaults to RTL with Arabic locale

    // Save it and load it back.
    saveAndReload("Office Open XML Text");

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("ParagraphStyles")->getByName("Default Paragraph Style"), uno::UNO_QUERY_THROW);
    // Test the text Direction value for the -none- based paragraph styles
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RTL Writing Mode", sal_Int32(1),
                                 getProperty<sal_Int32>(xPageStyle, "WritingMode"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122893)
{
    createSwDoc("tdf105413.fodt");
    SwDoc* pDoc = getSwDoc();

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
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122901)
{
    createSwDoc("tdf105413.fodt");
    SwDoc* pDoc = getSwDoc();

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
    // "Increase Paragraph Spacing".
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
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122942)
{
    createSwDoc("tdf122942.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Do the moral equivalent of mouse button down, move and up.
    // Start creating a custom shape that overlaps with the rounded rectangle
    // already present in the document.
    Point aStartPos(8000, 3000);
    pWrtShell->BeginCreate(SdrObjKind::CustomShape, aStartPos);

    // Set its size.
    Point aMovePos(10000, 5000);
    pWrtShell->MoveCreate(aMovePos);

    // Finish creation.
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Make sure that the shape is inserted.
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());

    saveAndReload("writer8");
    pDoc = getSwDoc();
    const auto& rFormats2 = *pDoc->GetSpzFrameFormats();
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
    createSwDoc("tdf132160.odt");

    // this would crash due to delete redline starting with ToX
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // this would crash due to insert redline ending on table node
    dispatchCommand(mxComponent, ".uno:Undo", {});

    dispatchCommand(mxComponent, ".uno:Redo", {});

    dispatchCommand(mxComponent, ".uno:Undo", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137526)
{
    createSwDoc("tdf132160.odt");
    SwDoc* pDoc = getSwDoc();

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
    createSwDoc("tdf132160.odt");
    SwDoc* pDoc = getSwDoc();

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
    createSwDoc("tdf132160.odt");
    SwDoc* pDoc = getSwDoc();

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete the first two paragraphs
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc("tdf132160.odt");
    SwDoc* pDoc = getSwDoc();

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
    createSwDoc("tdf52391.fodt");

    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // this was "Portion1", because the tracked background color of Portion1 was
    // accepted for "Reject All". Now rejection clears formatting of the text
    // in format-only changes, concatenating the text portions in the first paragraph.
    CPPUNIT_ASSERT_EQUAL(OUString("Portion1Portion2"), xRun->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137771)
{
    createSwDoc("tdf132160.odt");
    SwDoc* pDoc = getSwDoc();

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
    assertXPath(pXmlDoc, "/metafile/push/push/push/line"_ostr, 6);

    // This was the content of the next <text> (missing deletion on margin)
    // or only the first character of the deleted character sequence
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[9]/text"_ostr, " saved.");

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
    createSwDoc("tdf142130.fodt");
    SwDoc* pDoc = getSwDoc();

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
    assertXPath(pXmlDoc, "/metafile/push/push/push/line"_ostr, 4);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/linecolor"_ostr, 5);
    // tdf#142128 This was 3 (NON_PRINTING_CHARACTER_COLOR = #268bd2)
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/linecolor[@color='#268bd2']"_ostr, 0);

    // reject deletion of the second image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);
    // no crossing out and vertical redlines
    assertXPath(pXmlDoc2, "/metafile/push/push/push/line"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf142196)
{
    createSwDoc("tdf142196.fodt");
    SwDoc* pDoc = getSwDoc();

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
    assertXPath(pXmlDoc, "//line"_ostr, 3);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor"_ostr,
                1);
    // tdf#142128 This was NON_PRINTING_CHARACTER_COLOR (#268bd2)
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor[@color='#268bd2']"_ostr,
        0);

    // reject deletion of the image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);

    // no crossing out and vertical "changed line" indicator
    assertXPath(pXmlDoc2, "//line"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf142700)
{
    createSwDoc("tdf142700.fodt");
    SwDoc* pDoc = getSwDoc();

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
    assertXPath(pXmlDoc, "//line"_ostr, 3);

    // check line color
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor"_ostr,
                1);
    // tdf#142128 This was NON_PRINTING_CHARACTER_COLOR (#268bd2)
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/linecolor[@color='#268bd2']"_ostr,
        0);

    // reject deletion of the image
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    xMetaFile = pShell->GetPreviewMetaFile();
    xmlDocUniquePtr pXmlDoc2 = dumpAndParse(dumper, *xMetaFile);

    // no crossing out and vertical "changed line" indicator
    // This was 2 (not removed strikethrough)
    assertXPath(pXmlDoc2, "//line"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139120)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "/metafile/push/push/push/textarray"_ostr, 2);

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testJoinParaChangesInMargin)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

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
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
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
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testConditionalHiddenSectionIssue)
{
    // tdf#54703
    // When exporting the bug document as PDF, the conditional hidden
    // sections became visible in the PDF and in the document.

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    createSwDoc("HiddenSection.odt");
    SwDoc* pDoc = getSwDoc();

    // Check section conditional hidden status - all should be hidden (IsCondHidden == true)
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwSectionNode const* const pNode = pDoc->GetNodes()[i]->GetSectionNode())
        {
            CPPUNIT_ASSERT_EQUAL(true, pNode->GetSection().IsCondHidden());
        }
    }

    // PDF export
    save("writer_pdf_Export");

    auto pPdfDocument = parsePDFExport();
    auto pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);

    // No PDF object should be present in the page - sections remained hidden
    CPPUNIT_ASSERT_EQUAL(0, pPdfPage->getObjectCount());

    // Check section conditional hidden status - all should remained hidden (IsCondHidden == true)
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwSectionNode const* const pNode = pDoc->GetNodes()[i]->GetSectionNode())
        {
            CPPUNIT_ASSERT_EQUAL(true, pNode->GetSection().IsCondHidden());
        }
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
