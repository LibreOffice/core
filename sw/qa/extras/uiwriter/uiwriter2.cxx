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
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf47471_paraStyleBackground)
{
    createSwDoc("tdf47471_paraStyleBackground.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(u"00Background"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xe0c2cd), getProperty<Color>(getParagraph(2), u"FillColor"_ustr));

    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, u".uno:ResetAttributes"_ustr, {});

    // the background color should revert to the color for 00Background style
    CPPUNIT_ASSERT_EQUAL(Color(0xdedce6), getProperty<Color>(getParagraph(2), u"FillColor"_ustr));
    // the paragraph style should not be reset
    CPPUNIT_ASSERT_EQUAL(u"00Background"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"00Background"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);

    CPPUNIT_ASSERT_EQUAL(Color(0xdedce6), getProperty<Color>(getParagraph(2), u"FillColor"_ustr));
    // on round-trip, the paragraph style name was lost
    CPPUNIT_ASSERT_EQUAL(u"00Background"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"00Background"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
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
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", u"GHI GHI GHI GHI");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "type", u"PortionType::Number");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"2.");
    // The numbering height was 960 in DOC format.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Grow"_ustr, {});
    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"260");

    // save it to DOCX
    saveAndReload(u"Office Open XML Text"_ustr);
    pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();
    // this was 220
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"260");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101534)
{
    // Copy the first paragraph of the document.
    createSwDoc("tdf101534.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(),
                    svl::Items<RES_MARGIN_FIRSTLINE, RES_MARGIN_TEXTLEFT>);
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(!aSet.HasItem(RES_MARGIN_FIRSTLINE));
    CPPUNIT_ASSERT(aSet.HasItem(RES_MARGIN_TEXTLEFT));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSet.GetItem(RES_MARGIN_TEXTLEFT)->ResolveTextLeft({}));

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    aSet.ClearItem();
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(!aSet.HasItem(RES_MARGIN_FIRSTLINE));
    CPPUNIT_ASSERT(aSet.HasItem(RES_MARGIN_TEXTLEFT));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSet.GetItem(RES_MARGIN_TEXTLEFT)->ResolveTextLeft({}));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testExtendedSelectAllHang)
{
    createSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->InsertFootnote(u""_ustr);
    pWrtShell->StartOfSection();
    SwView* pView = getSwDocShell()->GetView();
    SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Default_Spanish (Bolivia)"_ustr);
    // this looped
    pView->GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON,
                                                       { &aLangString });
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineMoveInsertInDelete)
{
    createSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u" foo"_ustr);
    pWrtShell->SttEndDoc(true);
    pWrtShell->InsertFootnote(u""_ustr);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    // insert redline
    pWrtShell->Insert(u"bar"_ustr);
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
    CPPUNIT_ASSERT_EQUAL(u" "_ustr,
                         pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetText());
    pWrtShell->SetRedlineFlags(mode); // show again
    CPPUNIT_ASSERT_EQUAL(u"\u0001 foo"_ustr,
                         pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineInHiddenSection)
{
    createSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->SplitNode();
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"baz"_ustr);

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
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    // these are required so that IsBlank() is true
    aViewOptions.SetBlank(true);
    aViewOptions.SetViewMetaChars(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    calcLayout();
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
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u"A"_ustr);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

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

    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion", 0);
    // not sure why there's an empty text portion here, but it's not a problem
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"");

    pWrtShell->Undo();

    // the problem was that here the "A" showed up again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"");

    pWrtShell->Undo();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"");

    pWrtShell->Undo();

    // now the "A" is no longer deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[@portion]", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "length", u"1");

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"A");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136704)
{
    createSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    corr.GetSwFlags().bReplaceStyles = true;
    SvxSwAutoFormatFlags flags(*SwEditShell::GetAutoFormatFlags());
    comphelper::ScopeGuard const g([=]() { SwEditShell::SetAutoFormatFlags(&flags); });
    flags.bReplaceStyles = true;
    SwEditShell::SetAutoFormatFlags(&flags);

    pWrtShell->Insert(u"test"_ustr);
    const sal_Unicode cIns = ':';
    pWrtShell->AutoCorrect(corr, cIns);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here

    CPPUNIT_ASSERT_EQUAL(u"Heading 3"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(u"foo" SAL_NEWLINE_STRING "bar"_ustr,
                         xTextContent->getAnchor()->getString());

    // select all with table at start -> 3 times
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());

    // this would crash in 2 different ways
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    // Without the fix in place, section's content would have been gone after undo
    CPPUNIT_ASSERT_EQUAL(u"foo" SAL_NEWLINE_STRING "bar"_ustr,
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"foo" SAL_NEWLINE_STRING "bar"_ustr,
                         xTextContent->getAnchor()->getString());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf134436)
{
    createSwDoc("tdf134436.fodt");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // select all 3 times, table at the start
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    // the stupid SwXTextView::getString doesn't work "for symmetry" so use CursorShell
    CPPUNIT_ASSERT_EQUAL(u"a\nb\n"_ustr, pWrtShell->GetCursor()->GetText());

    // first, the section doesn't get deleted
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"a\nb\n"_ustr, pWrtShell->GetCursor()->GetText());

    // second, the section does get deleted because point is at the end
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"a\nb\n"_ustr, pWrtShell->GetCursor()->GetText());

    // the problem was that the section was not deleted on Redo
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"a\nb\n"_ustr, pWrtShell->GetCursor()->GetText());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pWrtShell->GetCursor()->GetText());
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
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING ""_ustr,
                         xCursor->getString());

    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xCursor->getString());

    // this would crash
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING ""_ustr,
                         xCursor->getString());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xCursor->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING ""_ustr,
                         xCursor->getString());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136452)
{
    createSwDoc("tdf136452.fodt");
    SwDoc* pDoc = getSwDoc();

    SwNodeOffset const nNodes(pDoc->GetNodes().Count());

    SwWrtShell* const pWrtShell(getSwDocShell()->GetWrtShell());

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
    SwWrtShell* const pWrtShell(getSwDocShell()->GetWrtShell());

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

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 0);

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137245)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
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
    pWrtShell->Insert(u"---"_ustr);
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
    SwWrtShell* const pWrtShell(getSwDocShell()->GetWrtShell());
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
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sw::UnoCursorPointer pCursor(
        pDoc->CreateUnoCursor(SwPosition(pDoc->GetNodes().GetEndOfContent(), SwNodeOffset(-1))));

    pDoc->getIDocumentContentOperations().InsertString(*pCursor, u"foo"_ustr);

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
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, u"blahblah"_ustr, false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"blahblah"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"blahblah"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetPointContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetPointContentNode(), 3);

    // replace with less text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, u"x"_ustr, false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetPointContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetPointContentNode(), 3);

    // regex replace with paragraph breaks
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, u"xyz\\n\\nquux\\n"_ustr, true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"xyz"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"xyz\n\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"xyz"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"xyz\n\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    // regex replace with paragraph join
    pWrtShell->StartOfSection(false);
    pWrtShell->Down(true);
    pDoc->getIDocumentContentOperations().ReplaceRange(*pWrtShell->GetCursor(), u"bar"_ustr, true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"bar\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"xyz"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"xyz\n\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"bar\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(u"xyz"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"xyz\n\nquux\n"_ustr, pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140007)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"baz"_ustr);
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(u" "_ustr, true);
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(u" "_ustr, true);
    CPPUNIT_ASSERT_EQUAL(u"foo bar baz"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(u"foo bar baz"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr,
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139982)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // turn on redlining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    pWrtShell->Insert(u"helloo"_ustr);

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

    pWrtShell->Replace(u"hello"_ustr, true);

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
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u"foobar"_ustr);

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
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
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

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // store original text of the document for checking Undo
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    OUString sOrigText(xTextDocument->getText()->getString());

    // first paragraph is "Lorem ipsum" with deleted "m ips"
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());

    // move down first paragraph with change tracking
    dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});

    // deletion isn't rejected
    CPPUNIT_ASSERT_EQUAL(u"Loremm"_ustr, getParagraph(3)->getString());

    // Undo and repeat it with the second paragraph
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sOrigText, xTextDocument->getText()->getString());

    // second paragraph is "dolor sit" with deleted "lor "
    CPPUNIT_ASSERT_EQUAL(u"dolor sit"_ustr, getParagraph(2)->getString());

    // move down second paragraph with change tracking
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);

    dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});

    // This was "dolor sit" (rejecting tracked deletion)
    CPPUNIT_ASSERT_EQUAL(u"dolsit"_ustr, getParagraph(4)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sOrigText, xTextDocument->getText()->getString());
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveUp"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveUp"_ustr, {});

    // accept all changes
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    // This was 2 (bad deletion of the first paragraph)
    // TODO fix unnecessary insertion of a new list item at the end of the document
    CPPUNIT_ASSERT(getParagraphs() >= 3);

    // This was "Loremdolsit\namet.\n" (bad deletion of "m\n" at the end of item 1)
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Loremm" SAL_NEWLINE_STRING "dolsit" SAL_NEWLINE_STRING
                         "amet." SAL_NEWLINE_STRING ""_ustr,
                         xTextDocument->getText()->getString());
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
        dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 3 (bad conversion of the empty item to a tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    // check move up

    for (int i = 0; i < 3; ++i)
        dispatchCommand(mxComponent, u".uno:MoveUp"_ustr, {});

    // This was 3 (bad conversion of the empty item to a tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    //turn on red-lining and hide changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove first paragraph with paragraph break
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // remaining paragraph keeps its original style
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819_keep_numbering_with_Undo)
{
    createSwDoc("tdf54819b.odt");
    SwDoc* pDoc = getSwDoc();

    // heading
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    OUString sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    sNumName = getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr);
    // solved problem: lost numbering
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // accept deletion, remaining (now second) paragraph: still bulleted list item
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading, manual test is correct
    // TODO: it works well, but the test fails...
    // SwWrtShell* const pWrtShell2 = getSwDocShell()->GetWrtShell();
    // CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
    //                     getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
    //                     getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
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
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    OUString sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr);
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

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
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // accept deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // Joined paragraph 2 and paragraph 4: Fusce...nunc.
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith("Fusce"));
    CPPUNIT_ASSERT(getParagraph(2)->getString().endsWith("nunc."));
    // Remaining (now second) paragraph: it is still heading
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571_keep_numbering_with_Reject)
{
    // as the previous test, but with partial paragraph deletion:
    // all deleted paragraphs get the formatting of the first (the partially deleted) one
    createSwDoc("tdf54819b.odt");
    SwDoc* pDoc = getSwDoc();

    // heading
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    OUString sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr);
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

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
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // reject deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    // heading

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Outline"_ustr,
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    sNumName = getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140077)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

    pWrtShell->Insert(u"a"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"b"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    // enable
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Delete();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"bar"_ustr);
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"bar"_ustr);
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
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());

    pWrtShell->Insert(u"Abc"_ustr);

    // hide and enable
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

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
        u"Ab c"_ustr,
        pWrtShell->getShellCursor(false)->GetPoint()->GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147310)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
        assertXPath(pXmlDoc, "/root/page/body/tab", 0);
    }
    pWrtShell->Undo();
    // this did not create frames for the table
    pWrtShell->Undo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab", 2);
    }
    pWrtShell->Redo();
    pWrtShell->Redo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/tab", 0);
    }
    pWrtShell->Undo();
    pWrtShell->Undo();
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab", 2);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf80194_autoSubscript)
{
    createSwDoc();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Automatic mode enabled", false,
        getProperty<bool>(getRun(getParagraph(1), 1), u"CharAutoEscapement"_ustr));
    dispatchCommand(mxComponent, u".uno:SubScript"_ustr, {});
    // Writer has always had automatic mode enabled when creating subscript/superscripts.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Automatic mode enabled", true,
        getProperty<bool>(getRun(getParagraph(1), 1), u"CharAutoEscapement"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf64242_optimizeTable)
{
    createSwDoc("tdf64242_optimizeTable.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    double origWidth = getProperty<double>(xTextTable, u"Width"_ustr);
    sal_Int32 nToleranceW = origWidth * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width", double(17013), origWidth, nToleranceW);

    pWrtShell->SelTable(); //select the whole table

    dispatchCommand(mxComponent, u".uno:SetOptimalColumnWidth"_ustr, {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width: optimize", origWidth,
                                         getProperty<double>(xTextTable, u"Width"_ustr),
                                         nToleranceW);

    dispatchCommand(mxComponent, u".uno:SetMinimalColumnWidth"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("Table Width: minimized",
                           (origWidth - nToleranceW)
                               > getProperty<double>(xTextTable, u"Width"_ustr));

    double origRowHeight = getProperty<double>(xTableRows->getByIndex(2), u"Height"_ustr);
    sal_Int32 nToleranceH = origRowHeight * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Row Height", double(3441), origRowHeight, nToleranceH);

    dispatchCommand(mxComponent, u".uno:SetOptimalRowHeight"_ustr, {});
    double optimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), u"Height"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Row Height: optimized",
                           (origRowHeight - nToleranceH) > optimalRowHeight);

    dispatchCommand(mxComponent, u".uno:SetMinimalRowHeight"_ustr, {});
    double minimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), u"Height"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Row Height: minimized",
                           (optimalRowHeight - nToleranceH) > minimalRowHeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row set to auto-height", double(0), minimalRowHeight);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf45525)
{
    createSwDoc("tdf45525.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    CPPUNIT_ASSERT_EQUAL(3889.0, getProperty<double>(xTableRows->getByIndex(0), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(1), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(2), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(3), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(4), u"Height"_ustr));

    //Select three cells in the first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, u".uno:SetOptimalRowHeight"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: 1914
    // - Actual  : 3889
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(0), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(1), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(1914.0, getProperty<double>(xTableRows->getByIndex(2), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(3), u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(0.0, getProperty<double>(xTableRows->getByIndex(4), u"Height"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126784_distributeSelectedColumns)
{
    createSwDoc("tdf126784_distributeSelectedColumns.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    auto aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(
        xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr);
    sal_Int16 nOrigCol2Pos = aSeq[0].Position;
    sal_Int16 nOrigCol3Pos = aSeq[1].Position;

    //Select column 1 and 2
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, u".uno:DistributeColumns"_ustr, {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  u"TableColumnSeparators"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Second column should shrink", nOrigCol2Pos < aSeq[0].Position);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last column shouldn't change", nOrigCol3Pos, aSeq[1].Position);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144317)
{
    createSwDoc("tdf144317.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    auto aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(
        xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr);
    sal_Int16 nOrigCol1Pos = aSeq[0].Position;

    // Move the cursor inside the table
    pWrtShell->Down(/*bSelect=*/false);

    //Select some cells in the first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, u".uno:SetMinimalColumnWidth"_ustr, {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  u"TableColumnSeparators"_ustr);
    CPPUNIT_ASSERT_MESSAGE("First column should shrink", aSeq[0].Position < nOrigCol1Pos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf108687_tabstop)
{
    createSwDoc("tdf108687_tabstop.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwNodeOffset nStartIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), nStartIndex);

    // Now pressing 'tab' should jump to the radio buttons.
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    //sal_Int32 nEndIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(11), nEndIndex);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // join paragraphs by removing the end of the first one with paragraph break
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // second paragraph changes its style in "Show changes" mode
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf144058)
{
    createSwDoc("tdf144058.fodt");
    SwDoc* pDoc = getSwDoc();

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));

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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // accept all: tables are deleted
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SwBackSpace"_ustr, {});
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119019)
{
    // check handling of overlapping redlines
    createSwDoc("tdf119019.docx");

    CPPUNIT_ASSERT_EQUAL(u"Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus."_ustr,
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 1)->getString());
    // second paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), u"RedlineType"_ustr));

    // delete last word of the second paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // check tracked text deletion
    CPPUNIT_ASSERT_EQUAL(u"tellus."_ustr, getRun(getParagraph(2), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), u"RedlineType"_ustr));

    // make sure that the tracked paragraph formatting is removed
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), u"RedlineType"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119824)
{
    // check handling of overlapping redlines with Redo
    createSwDoc("tdf119019.docx");
    SwDoc* pDoc = getSwDoc();

    CPPUNIT_ASSERT_EQUAL(u"Pellentesque habitant morbi tristique senectus "
                         "et netus et malesuada fames ac turpis egestas. "
                         "Proin pharetra nonummy pede. Mauris et orci."_ustr,
                         getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 1)->getString());
    // third paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), u"RedlineType"_ustr));

    // and a tracked text deletion at the beginning of the paragraph
    CPPUNIT_ASSERT_EQUAL(u"Pellentesque habitant morbi tristique senectus "_ustr,
                         getRun(getParagraph(3), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 2), u"RedlineType"_ustr));

    // delete last word of the third paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // check tracking of the new text deletion
    CPPUNIT_ASSERT_EQUAL(u"orci."_ustr, getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), u"RedlineType"_ustr));

    // make sure that the tracked paragraph formatting is removed (tracked deletion is in the second run)
    CPPUNIT_ASSERT_EQUAL(u"Pellentesque habitant morbi tristique senectus "_ustr,
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), u"RedlineType"_ustr));

    // tdf#119824 check redo
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Redo();

    // check again the first tracked text deletion (we lost this before the redo fix)
    CPPUNIT_ASSERT_EQUAL(u"Pellentesque habitant morbi tristique senectus "_ustr,
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), u"RedlineType"_ustr));

    // check redo of the new tracked text deletion
    CPPUNIT_ASSERT_EQUAL(u"orci."_ustr, getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), u"RedlineType"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105413)
{
    createSwDoc("tdf105413.fodt");
    SwDoc* pDoc = getSwDoc();

    // all paragraphs have got Standard paragraph style
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                             getProperty<OUString>(getParagraph(i), u"ParaStyleName"_ustr));
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817)
{
    createSwDoc("num-parent-style.docx");

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    // This was "1 Heading" instead of "2 Heading"
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    // This was "2.1 Heading"
    CPPUNIT_ASSERT_EQUAL(u"3.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // set Heading 1 style of paragraph 3 to Heading 2

    pWrtShell->Down(/*bSelect=*/false);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(3), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    uno::Sequence<beans::PropertyValue> aPropertyValues2 = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 2"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues2);

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(3), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817_round_trip)
{
    createSwDoc("tdf76817.fodt");

    // save it to DOCX
    saveAndReload(u"Office Open XML Text"_ustr);

    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    // This was "1 Heading" instead of "2 Heading"
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    // This was "2.1 Heading"
    CPPUNIT_ASSERT_EQUAL(u"3.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // set Heading 1 style of paragraph 3 to Heading 2

    pWrtShell->Down(/*bSelect=*/false);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(3), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    uno::Sequence<beans::PropertyValue> aPropertyValues2 = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 2"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues2);

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(3), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf76817_custom_outline)
{
    createSwDoc("tdf76817.docx");

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // set Heading 2 style of paragraph 2 to Heading 1

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    // This wasn't numbered
    CPPUNIT_ASSERT_EQUAL(u"3.1"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf123102)
{
    createSwDoc("tdf123102.odt");
    // insert a new row after a vertically merged cell
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "3." - caused by the hidden numbered paragraph of the new merged cell
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row[6]/cell[1]/txt/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"2.");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonSmallTable)
{
    // The floating table in the test document is too small, so we don't provide an unfloat button
    createSwDoc("small_floating_table.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
        u"unfloatable_floating_table.odt"_ustr, // Typical use case of multipage floating table
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        createSwDoc(sTestFileName.getStr());
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
        u"unfloatable_floating_table.odt"_ustr,
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString::Concat("Failure in the test file: ") + sTestFileName;

        // Test what happens when pushing the unfloat button
        createSwDoc(sTestFileName.getStr());
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    aSettings.SetLanguageTag(LanguageTag(u"ar"_ustr));
    Application::SetSettings(aSettings);
    comphelper::ScopeGuard g([this] { Application::SetSettings(this->m_aSavedSettings); });

    createSwDoc(); // new, empty doc - everything defaults to RTL with Arabic locale

    // Save it and load it back.
    saveAndReload(u"Office Open XML Text"_ustr);

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY_THROW);
    // Test the text Direction value for the -none- based paragraph styles
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RTL Writing Mode", sal_Int32(1),
                                 getProperty<sal_Int32>(xPageStyle, u"WritingMode"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122893)
{
    createSwDoc("tdf105413.fodt");
    SwDoc* pDoc = getSwDoc();

    // all paragraphs are left-aligned with preset single line spacing
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), u"ParaAdjust"_ustr));
        dispatchCommand(mxComponent, u".uno:SpacePara1"_ustr, {});
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    dispatchCommand(mxComponent, u".uno:CenterPara"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SpacePara2"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(
                                           getParagraph(3), u"ParaAdjust"_ustr)); // center-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200),
                         getProperty<style::LineSpacing>(getParagraph(3), u"ParaLineSpacing"_ustr)
                             .Height); // double line spacing
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), u"ParaAdjust"_ustr)); // left-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(getParagraph(2), u"ParaLineSpacing"_ustr)
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
                             getProperty<sal_Int32>(getParagraph(i), u"ParaTopMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), u"ParaBottomMargin"_ustr));
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    dispatchCommand(mxComponent, u".uno:ParaspaceIncrease"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(3), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(3), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122942)
{
    createSwDoc("tdf122942.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

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

    saveAndReload(u"writer8"_ustr);
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
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // this would crash due to insert redline ending on table node
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137526)
{
    createSwDoc("tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word
    dispatchCommand(mxComponent, u".uno:WordRightSel"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137684)
{
    createSwDoc("tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word letter by letter
    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // this would crash due to bad redline range
    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption "));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137503)
{
    createSwDoc("tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete the first two paragraphs
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("The"));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption "));

    // this would crash due to bad redline range
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("The"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138605)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // insert a word, delete it with change tracking and try to undo it
    pWrtShell->Insert(u"word"_ustr);
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    // this crashed due to bad access to the empty redline table
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // more Undo
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("word"));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith(""));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf138135)
{
    createSwDoc("tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // select and delete a word letter by letter by using backspace
    dispatchCommand(mxComponent, u".uno:GoToNextWord"_ustr, {});

    for (int i = 0; i <= 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("support"));

    // TODO group redlines for managing tracked changes/showing in margin
    for (int i = 0; i <= 10; ++i)
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Encryption"));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf52391)
{
    createSwDoc("tdf52391.fodt");

    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // this was "Portion1", because the tracked background color of Portion1 was
    // accepted for "Reject All". Now rejection clears formatting of the text
    // in format-only changes, concatenating the text portions in the first paragraph.
    CPPUNIT_ASSERT_EQUAL(u"Portion1Portion2"_ustr, xRun->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137771)
{
    createSwDoc("tdf132160.odt");

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // delete a word at the end of the paragraph.
    dispatchCommand(mxComponent, u".uno:GotoEndOfPara"_ustr, {});
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    }

    CPPUNIT_ASSERT(getParagraph(1)->getString().endsWith("to be "));

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This would be 5 without the new vertical redline mark
    assertXPath(pXmlDoc, "/metafile/push/push/push/line", 6);

    // This was the content of the next <text> (missing deletion on margin)
    // or only the first character of the deleted character sequence
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[9]/text", u" saved.");

    // this would crash due to bad redline range
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }
    CPPUNIT_ASSERT(getParagraph(1)->getString().endsWith("to be saved."));

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
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
    SwDocShell* pShell = getSwDocShell();
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
    SwDocShell* pShell = getSwDocShell();
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
    SwDocShell* pShell = getSwDocShell();
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
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
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
    dispatchCommand(mxComponent, u".uno:GotoEndOfPara"_ustr, {});
    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    }

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum sit amet."_ustr, xTextDocument->getText()->getString());

    for (int i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }

    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"dolor sit amet."_ustr, getParagraph(2)->getString());

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was the 3, containing the text "$2" instead of nothing
    assertXPath(pXmlDoc, "/metafile/push/push/push/textarray", 2);

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testJoinParaChangesInMargin)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
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
    dispatchCommand(mxComponent, u".uno:GotoEndOfPara"_ustr, {});
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsudolor sit amet."_ustr, getParagraph(1)->getString());

    // Undo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // this would crash due to bad redline range
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140757)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
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
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"orem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"olor sit amet."_ustr, getParagraph(2)->getString());

    // accept all changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/true);

    CPPUNIT_ASSERT_EQUAL(u"orem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"olor sit amet."_ustr, getParagraph(2)->getString());

    // This crashed
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Check result of Undo
    rIDRA.AcceptAllRedline(/*bAccept=*/false);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"dolor sit amet."_ustr, getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
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
    save(u"writer_pdf_Export"_ustr);

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
