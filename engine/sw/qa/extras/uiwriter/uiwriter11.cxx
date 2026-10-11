/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>

#include <officecfg/Office/Writer.hxx>
#include <test/commontesttools.hxx>
#include <vcl/event.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/scheduler.hxx>
#include <svl/srchitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>
#include <o3tl/unit_conversion.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/configuration.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <rtl/strbuf.hxx>
#include <utility>

#include <editeng/brushitem.hxx>
#include <hintids.hxx>
#include <AnnotationWin.hxx>
#include <cmdid.h>
#include <docufld.hxx>
#include <drawdoc.hxx>
#include <dflyobj.hxx>
#include <edtwin.hxx>
#include <flyfrm.hxx>
#include <fmtfsize.hxx>
#include <frameformats.hxx>
#include <i18nutil/paper.hxx>
#include <itabenum.hxx>
#include <names.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <swdtflvr.hxx>
#include <PostItMgr.hxx>
#include <translatehelper.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <redline.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <svx/svxids.hrc>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
class SwUiWriterTest11 : public SwModelTestBase
{
public:
    SwUiWriterTest11()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf167760_numberedPara)
{
    createSwDoc("tdf167760_numberedPara.odt");

    CPPUNIT_ASSERT_EQUAL(u"1.1."_ustr,
                         getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));

    // apply a non-numbered style to paragraph 1
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr) });
    // the numbering should not be affected by changing a paragraph style
    CPPUNIT_ASSERT_EQUAL(u"1.1."_ustr,
                         getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));

    // apply a numbered style
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"aList"_ustr) });
    // the numbering should be changed to that specified by the paragraph style
    CPPUNIT_ASSERT_EQUAL(u"i.I.a)"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));

    // apply the non-numbered style while holding down the Ctrl-key
    dispatchCommand(
        mxComponent, u".uno:StyleApply"_ustr,
        { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
          comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr),
          comphelper::makePropertyValue(u"KeyModifier"_ustr, cpo::uno::Any(KEY_MOD1)) });
    // the numbering should be removed when the Ctrl-key is held down
    CPPUNIT_ASSERT_EQUAL(OUString(),
                         getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf113213_addToList)
{
    // given a document with two separate numbering lists
    createSwDoc("tdf113213_addToList.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Select the second list
    pWrtShell->EndOfSection();
    pWrtShell->Up(/*bSelect=*/true);
    // "Add to list": join the second list to the previous one
    dispatchCommand(mxComponent, u".uno:ContinueNumbering"_ustr, {});
    // The "restart numbering" property should be helpfully removed when joining to another list
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));

    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf149061_moveNumParas)
{
    // given a document with a list containing some subPoints
    createSwDoc("tdf149061_moveNumParas.odt");

    // Move 'First Point' below 'Second Point' and its subpoint
    dispatchCommand(mxComponent, u".uno:MoveDownSubItems"_ustr, {});
    // Without the fix, this action was prevented by the section.
    getParagraph(3, u"First Point."_ustr);

    // Position the cursor on 'A Point'
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {}); // subpoint
    dispatchCommand(mxComponent, u".uno:GoToPrevPara"_ustr, {}); // B Point
    dispatchCommand(mxComponent, u".uno:GoToPrevPara"_ustr, {}); // A Point

    // Move 'A Point' below 'B Point' and its subpoint
    dispatchCommand(mxComponent, u".uno:MoveDownSubItems"_ustr, {});
    // Without the fix, this separated B from its subpoint - leaving the subpoint under A.
    getParagraph(8, u"A Point"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791)
{
    auto getPostItMgr = [](SwDocShell* pDocShell) {
        CPPUNIT_ASSERT(pDocShell);
        SwView* pView = pDocShell->GetView();
        CPPUNIT_ASSERT(pView);
        SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
        CPPUNIT_ASSERT(pPostItMgr);
        return pPostItMgr;
    };

    // Given a document with tracked changes enabled, having some threads of comments:

    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

    {
        // Test "Delete Comment": the selected comment must be marked as deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Comment thread 1 reply 2"_ustr, pPostItField->GetText());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteComment"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(i == 2 ? SwPostItHelper::SwLayoutStatus::DELETED
                                        : SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

    {
        // Test "Delete Comment Thread": the comment, and all its thread, must be marked as deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Comment thread 1 reply 2"_ustr, pPostItField->GetText());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteCommentThread"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(i <= 2 ? SwPostItHelper::SwLayoutStatus::DELETED
                                        : SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

    {
        // Test "Delete Comments by Author3": the author's comments must be marked as deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Author3"_ustr, pPostItField->GetPar1());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteAuthor"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt);
            pPostItField = aPostItFields[i]->mpPostIt->GetPostItField();
            CPPUNIT_ASSERT(pPostItField);
            CPPUNIT_ASSERT_EQUAL(pPostItField->GetPar1() == "Author3"
                                     ? SwPostItHelper::SwLayoutStatus::DELETED
                                     : SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

    {
        // Test "Delete All Comments": all comments must be marked as deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select any comment
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteAllNotes"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::DELETED,
                                 aPostItFields[i]->mLayoutStatus);
        }
    }

    // Now test the same with an equivalent DOCX: comment deletion must delete immediately

    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

    {
        // Test "Delete Comment": the selected comment must be deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Comment thread 1 reply 2"_ustr, pPostItField->GetText());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteComment"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(5), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt);
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt->GetPostItField());
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt->GetPostItField()->GetText()
                           != "Comment thread 1 reply 2");
        }
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

    {
        // Test "Delete Comment Thread": the comment, and all its thread, must be deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Comment thread 1 reply 2"_ustr, pPostItField->GetText());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteCommentThread"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(3), aPostItFields.size());
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

    {
        // Test "Delete Comments by Author3": the author's comments must be deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select "Comment thread 1 reply 2"
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);
        const SwPostItField* pPostItField = aPostItFields[2]->mpPostIt->GetPostItField();
        CPPUNIT_ASSERT(pPostItField);
        CPPUNIT_ASSERT_EQUAL(u"Author3"_ustr, pPostItField->GetPar1());

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteAuthor"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(size_t(4), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt);
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt->GetPostItField());
            CPPUNIT_ASSERT(aPostItFields[i]->mpPostIt->GetPostItField()->GetPar1() != "Author3");
        }
    }

    // load it anew
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

    {
        // Test "Delete All Comments": all comments must be deleted

        SwPostItMgr* pPostItMgr = getPostItMgr(getSwDocShell());
        auto& aPostItFields = pPostItMgr->GetPostItFields();

        CPPUNIT_ASSERT_EQUAL(size_t(6), aPostItFields.size());
        for (size_t i = 0; i < aPostItFields.size(); ++i)
        {
            CPPUNIT_ASSERT(aPostItFields[i]);
            CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::VISIBLE,
                                 aPostItFields[i]->mLayoutStatus);
        }

        // Select any comment
        CPPUNIT_ASSERT(aPostItFields[2]->mpPostIt);

        pPostItMgr->SetActiveSidebarWin(aPostItFields[2]->mpPostIt);
        dispatchCommand(mxComponent, u".uno:DeleteAllNotes"_ustr, {});

        CPPUNIT_ASSERT(aPostItFields.empty());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testRedlineTooltipListsCommentAnchorChanges)
{
    // Given "Cec", three adjacent comments, each anchored in its own tracked insertion by Author A,
    // Author B and Author C, then "ipour" inserted by Author D:
    createSwDoc("redline-comment-anchors.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    auto pTextFrame
        = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    const SwTextNode* pTextNode = pTextFrame->GetTextNodeForFirstText();

    // Returns the authors of the changes in the tooltip when hovering the character at nIndex.
    auto getAuthors = [&](sal_Int32 nIndex) {
        SwRect aChar;
        pTextFrame->GetCharRect(aChar, SwPosition(*pTextNode, nIndex));
        SwRect aNextChar;
        pTextFrame->GetCharRect(aNextChar, SwPosition(*pTextNode, nIndex + 1));
        Point aPoint((aChar.Left() + aNextChar.Left()) / 2, aChar.Center().Y());
        SwContentAtPos aContentAtPos(IsAttrAtPos::Redline);
        CPPUNIT_ASSERT(pWrtShell->GetContentAtPos(aPoint, aContentAtPos));
        OUString aAuthors = aContentAtPos.aFnd.pRedl->GetAuthorString();
        for (const SwRangeRedline* pRedline : aContentAtPos.aCommentAnchorRedlines)
            aAuthors += ", " + pRedline->GetAuthorString();
        return aAuthors;
    };

    // Then hovering the text on either side of the comments lists the changes on the anchors too:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Author A, Author B, Author C
    // - Actual  : Author A
    // i.e. the changes that hold only a comment anchor never appeared in a tooltip.
    CPPUNIT_ASSERT_EQUAL(u"Author A, Author B, Author C"_ustr, getAuthors(2));
    CPPUNIT_ASSERT_EQUAL(u"Author D, Author A, Author B, Author C"_ustr, getAuthors(6));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf162120AutoRTL)
{
    createSwDoc("tdf162120-auto-rtl.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // The initial direction should be RTL
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));

    // Insert a strong LTR character at the start of the paragraph.
    // The writing mode should automatically switch to LTR.
    pWrtShell->Insert(u"a"_ustr);
    CPPUNIT_ASSERT_EQUAL(short(0),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));

    // Delete the leading LTR character.
    // The writing mode should switch back to RTL.
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testFontworkSelection)
{
    // Load a document with a Fontwork shape and select the shape
    createSwDoc("tdf131124_Fontwork_Selection.odt");
    dispatchCommand(mxComponent, u".uno:JumpToNextFrame"_ustr, {});

    // Make sure the CurrentSelection of the document is not empty.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    // Without fix, this test would have failed here
    CPPUNIT_ASSERT(xSelections.is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testExtrudedShapeSelection)
{
    // Load a document with an extruded custom shape and select the shape
    createSwDoc("tdf131124_Extruded_Shape_Selection.odt");
    dispatchCommand(mxComponent, u".uno:JumpToNextFrame"_ustr, {});

    // Make sure the CurrentSelection of the document is not empty.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    // Without fix, this test would have failed here
    CPPUNIT_ASSERT(xSelections.is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf163194)
{
    // Test (1) that exporting comments in margin to PDF produces expected layout of the comments,
    // that have limited and reasonable width and position, inside the page bounds (and close to
    // its right side), independent on the manually set comment sidebar width; and (2) that the
    // export doesn't change the annotation sizes in the document.

    // Annotation size depends on DisplayWidthFactor. Set it to a fixed value for testing.
    ScopedConfigValue<officecfg::Office::Writer::Notes::DisplayWidthFactor> aCfg(4.0);

    createSwDoc("tdf163194-two-comments.fodt");

    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);
    SwView* pView = pDocShell->GetView();
    CPPUNIT_ASSERT(pView);
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    CPPUNIT_ASSERT(pPostItMgr);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPostItMgr->GetPostItFields().size());
    std::vector<tools::Long> aOriginalHeights;
    for (const auto& pAnnotationItem : pPostItMgr->GetPostItFields())
    {
        CPPUNIT_ASSERT(pAnnotationItem);
        CPPUNIT_ASSERT(pAnnotationItem->mpPostIt);
        const Size aSize = pAnnotationItem->mpPostIt->GetSizePixel();
        // Pixel width depends on DisplayWidthFactor, seems to not depend on DPI
        CPPUNIT_ASSERT_EQUAL(tools::Long(400), aSize.Width());
        // Heights depend on width and current DPI scaling; just remember them for later comparison
        aOriginalHeights.push_back(aSize.Height());
    }

    // Export to PDF with comments in margin
    cpo::uno::Sequence aFilterData{ comphelper::makePropertyValue(u"ExportNotes"_ustr, false),
                                    comphelper::makePropertyValue(u"ExportNotesInMargin"_ustr,
                                                                  true) };
    cpo::uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
                                    comphelper::makePropertyValue(u"URL"_ustr,
                                                                  maTempFile.GetURL()) };
    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

    if (auto pPdfDocument = parsePDFExport()) // This part will be skipped without PDFium
    {
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
        auto pPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPage);
        // 595 pt corresponds to 210 mm (width of A4)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(595, pPage->getWidth(), 1.0);
        auto pTextPage = pPage->getTextPage();
        CPPUNIT_ASSERT(pTextPage);

        // There should be two filled path objects corresponding to the two annotations
        int nPathCount = 0;

        // Check that there are all the expected lines of the annotations, i.e. that the layout
        // of the comments is stable and independent of the DPI.
        const OUString aLines[] = {
            // First annotation
            u"A comment. One two three four "_ustr,
            u"five six seven eight nine ten "_ustr,
            u"eleven twelve thirteen fourteen "_ustr,
            u"fifteen sixteen seventeen "_ustr,
            u"eighteen nineteen twenty."_ustr,
            // Second annotation
            u"Another comment. Twenty-one "_ustr,
            u"twenty-two twenty-three "_ustr,
            u"twenty-four twenty-five twenty-"_ustr,
            u"six twenty-seven twenty-eight "_ustr,
            u"twenty-nine thirty."_ustr,
        };
        std::set<OUString> aFoundLines;

        for (int i = 0; i < pPage->getObjectCount(); ++i)
        {
            auto pObject = pPage->getObject(i);

            if (pObject->getType() == vcl::pdf::PDFPageObjectType::Path
                && pObject->getPathSegmentCount() == 5)
            {
                // This is the filled rectangle of an annotation. I hope that path segment count
                // is stable and unique enough to identify it.
                CPPUNIT_ASSERT(pObject->getFillColor() != COL_TRANSPARENT);
                basegfx::B2DRectangle bounds = pObject->getBounds();
                // The object must start at position 448, and have width 101 (so its right
                // edge is ~at position 549, within the page's width of 595).
                // Without the fix, this failed like "Expected: 101; Actual: 225", i.e. its
                // right edge protruded beyond the right page limit (and generally depended
                // on the UI width of the comment sidebar).
                CPPUNIT_ASSERT_DOUBLES_EQUAL(101, bounds.getWidth(), 1.0);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(448, bounds.getMinX(), 1.0);
                ++nPathCount;
            }
            else if (pObject->getType() == vcl::pdf::PDFPageObjectType::Text)
            {
                // This is text; check if it matches expected annotation lines
                OUString aText = pObject->getText(pTextPage);
                if (std::ranges::find(aLines, aText) != std::end(aLines))
                {
                    // Check that it fits into the expected horizontal range of the comment
                    basegfx::B2DRectangle bounds = pObject->getBounds();
                    CPPUNIT_ASSERT_LESS(101.0, bounds.getWidth());
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(450, bounds.getMinX(), 1.0);

                    CPPUNIT_ASSERT(!aFoundLines.contains(aText)); // each line only once
                    aFoundLines.insert(aText);
                }
            }
        }
        CPPUNIT_ASSERT_EQUAL(2, nPathCount); // all annotation rectangles
        CPPUNIT_ASSERT_EQUAL(std::size(aLines), aFoundLines.size()); // all annotation lines
    }

    // Test that export didn't change the annotation sizes
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPostItMgr->GetPostItFields().size());
    for (size_t i = 0; i < 2; ++i)
    {
        const auto& pAnnotationItem = pPostItMgr->GetPostItFields()[i];
        CPPUNIT_ASSERT(pAnnotationItem);
        CPPUNIT_ASSERT(pAnnotationItem->mpPostIt);
        const Size aSize = pAnnotationItem->mpPostIt->GetSizePixel();
        CPPUNIT_ASSERT_EQUAL(tools::Long(400), aSize.Width());
        CPPUNIT_ASSERT_EQUAL(aOriginalHeights[i], aSize.Height());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testPageSizeKeepsOrientation)
{
    createSwDoc();

    dispatchCommand(
        mxComponent, u".uno:Orientation"_ustr,
        comphelper::InitPropertySequence({ { u"isLandscape"_ustr, cpo::uno::Any(true) } }));

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    const size_t nPageDesc = pWrtShell->GetCurPageDesc();

    CPPUNIT_ASSERT(pWrtShell->GetPageDesc(nPageDesc).GetLandscape());
    const Size aLandscapeA4
        = pWrtShell->GetPageDesc(nPageDesc).GetMaster().GetFrameSize().GetSize();
    CPPUNIT_ASSERT_GREATER(aLandscapeA4.Height(), aLandscapeA4.Width());

    dispatchCommand(mxComponent, u".uno:AttributePageSize"_ustr,
                    comphelper::InitPropertySequence(
                        { { u"PaperFormat"_ustr, cpo::uno::Any(sal_uInt16(PAPER_A3)) } }));

    const SwPageDesc& rPageDesc = pWrtShell->GetPageDesc(nPageDesc);
    CPPUNIT_ASSERT(rPageDesc.GetLandscape());
    const Size aNewSize = rPageDesc.GetMaster().GetFrameSize().GetSize();
    // Without the fix in place, this test would have failed with
    // - Expected greater than: 23811
    // - Actual  : 16838
    // i.e. the paper size change flipped the page to portrait.
    CPPUNIT_ASSERT_GREATER(aNewSize.Height(), aNewSize.Width());
    CPPUNIT_ASSERT_GREATER(aLandscapeA4.Width(), aNewSize.Width());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testResolveCommentThreadPartiallyResolved)
{
    // Given a document with a comment thread (root + replies):
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

    SwPostItMgr* pPostItMgr = getSwDocShell()->GetView()->GetPostItMgr();
    auto& aPostItFields = pPostItMgr->GetPostItFields();
    const SwPostItField* pRoot = aPostItFields[0]->mpPostIt->GetPostItField();
    const SwPostItField* pReply = aPostItFields[1]->mpPostIt->GetPostItField();

    // Put the thread into the partially-resolved state: root resolved, reply not.
    OUString sRootId = OUString::number(pRoot->GetPostItId());
    dispatchCommand(mxComponent, u".uno:ResolveComment"_ustr,
                    { comphelper::makePropertyValue(u"Id"_ustr, sRootId) });
    CPPUNIT_ASSERT(pRoot->GetResolved());
    CPPUNIT_ASSERT(!pReply->GetResolved());

    // Resolve the thread the way online does, by id.
    dispatchCommand(mxComponent, u".uno:ResolveCommentThread"_ustr,
                    { comphelper::makePropertyValue(u"Id"_ustr, sRootId) });

    // Without the fix in place, this test would have failed with:
    // - Expression: pRoot->GetResolved()
    // i.e. the whole thread was unresolved.
    CPPUNIT_ASSERT(pRoot->GetResolved());
    CPPUNIT_ASSERT(pReply->GetResolved());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf135857_findWithoutUnderline)
{
    // given a document contains a few underlined words and a few explicitly un-underlined words

    createSwDoc("tdf135857_findWithoutUnderline.odt");

    // Get the count of a FindAll of all runs that are not underlined.
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes = xSearch->createSearchDescriptor();

    // specifying the search attributes
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "CharUnderline", uno::Any(css::awt::FontUnderline::NONE) } }));
    xProp->setSearchAttributes(aDescriptor);

    // In the UI, this results in 5 matches. UNO seems to produce different results
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xSearch->findAll(xSearchDes)->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf36582_findReplaceRedline)
{
    // given a two-paragraph document, where the first paragraph contains a change tracking deletion

    createSwDoc("tdf36582_findReplaceRedline.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();
    IDocumentRedlineAccess& rIDRA(getSwDoc()->getIDocumentRedlineAccess());

    // Start by seeing the redlines, but NOT tracking changes.
    CPPUNIT_ASSERT(!rIDRA.IsRedlineOn());

    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(1)->getString()); // showing deletion
    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(2)->getString()); // all normal text

    // initialize environment to emulate a Find and Replace situation
    SfxItemSet aSet(
        SfxItemSet::makeFixedSfxItemSet<SID_SEARCH_ITEM, SID_SEARCH_ITEM>(rView.GetPool()));
    rView.StateSearch(aSet); // initializes SwView::GetSearchItem
    SvxSearchItem& rInit = *SwView::GetSearchItem();
    rInit.SetReplaceString(
        u"Deleted"_ustr); // emulated environment needs to match replaceString below
    rInit.SetCommand(SvxSearchCmd::REPLACE); // needed only to emulate find/replace environment

    i18nutil::SearchOptions2 aSearchOpt;
    aSearchOpt.searchString
        = u"Expunged Text"_ustr; // deletion is 'Expunged ': search for more than that
    aSearchOpt.replaceString = u"Deleted"_ustr;
    aSearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    const FindRanges flagReplaceAll = FindRanges::InBody | FindRanges::InSelAll;

    // Find and replace all
    pWrtShell->SearchPattern(aSearchOpt, /*SearchInNotes=*/false, SwDocPositions::Start,
                             SwDocPositions::End, flagReplaceAll, /*Replace=*/true);

    // don't replace when it found a mixture of deleted and non-deleted text
    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(1)->getString()); // found mixture
    CPPUNIT_ASSERT_EQUAL(u"Deleted"_ustr, getParagraph(2)->getString());

    pWrtShell->Undo();
    // reset search to just find (part of) the deletion string
    aSearchOpt.searchString = u"Expunged"_ustr;

    pWrtShell->SearchPattern(aSearchOpt, /*SearchInNotes=*/false, SwDocPositions::Start,
                             SwDocPositions::End, flagReplaceAll, /*Replace=*/true);

    // Special case: the deletion should remain deleted - just 'fix' the deleted text
    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(2)->getString()); // sanity check

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(2)->getString());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(2)->getString());

    pWrtShell->Undo(); // back to initial state

    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(SwRedlineTable::size_type(1), rIDRA.GetRedlineTable().size());

    // now turn change tracking on and do it again
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(getSwDoc()->getIDocumentRedlineAccess().IsRedlineOn());

    pWrtShell->SearchPattern(aSearchOpt, /*SearchInNotes=*/false, SwDocPositions::Start,
                             SwDocPositions::End, flagReplaceAll, /*Replace=*/true);

    // The deletion should remain deleted - just 'fix' the deleted text
    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"ExpungedDeleted Text"_ustr, getParagraph(2)->getString());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Expunged Text"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(SwRedlineTable::size_type(1), rIDRA.GetRedlineTable().size());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(u"Deleted Text"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"ExpungedDeleted Text"_ustr, getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testRedlineAutoCorrectInsertOverlap)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    emulateTyping(u"tset");

    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(getSwDoc()->getIDocumentRedlineAccess().IsRedlineOn());

    // The trailing space is a tracked insertion adjacent to the autocorrect
    // word "tset". The replacement "tset" -> "test" must still fire.
    emulateTyping(u" ");

    // NOTE: whether the autocorrect replacement itself should be recorded
    // as a tracked change is an open question.
    CPPUNIT_ASSERT_EQUAL(u"tsettest "_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testCommentDeletionStaysDeletedAfterHighlight)
{
    // A comment deleted with change tracking must keep showing as deleted even
    // after a character highlight is applied on top of the tracked deletion.
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    pWrtShell->Insert(u"word"_ustr);

    // anchor a comment to the whole word
    pWrtShell->SelAll();
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr,
                    { comphelper::makePropertyValue(u"Text"_ustr, u"a comment"_ustr),
                      comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr) });
    Scheduler::ProcessEventsToIdle();

    SwView* pView = getSwDocShell()->GetView();
    CPPUNIT_ASSERT(pView);
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    CPPUNIT_ASSERT(pPostItMgr);
    auto& aPostItFields = pPostItMgr->GetPostItFields();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPostItFields.size());

    // record changes, so deleting the comment marks it as deleted instead of
    // removing it outright
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

    CPPUNIT_ASSERT(aPostItFields[0]->mpPostIt);
    pPostItMgr->SetActiveSidebarWin(aPostItFields[0]->mpPostIt);
    dispatchCommand(mxComponent, u".uno:DeleteComment"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(1), aPostItFields.size());
    CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::DELETED, aPostItFields[0]->mLayoutStatus);

    // apply a character highlight over the word the comment was anchored to,
    // which stacks a format redline on top of the delete redline
    pWrtShell->SelAll();
    SvxBrushItem aHighlight(COL_YELLOW, RES_CHRATR_HIGHLIGHT);
    SfxItemSetFixed<RES_CHRATR_HIGHLIGHT, RES_CHRATR_HIGHLIGHT> aSet(pWrtShell->GetAttrPool());
    aSet.Put(aHighlight);
    pWrtShell->SetAttrSet(aSet);
    Scheduler::ProcessEventsToIdle();

    // Without the fix the format redline on top of the delete redline hid the
    // deletion, so the comment turned back to VISIBLE.
    CPPUNIT_ASSERT_EQUAL(SwPostItHelper::SwLayoutStatus::DELETED, aPostItFields[0]->mLayoutStatus);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testHeaderFlyClickInBody)
{
    createSwDoc("header-bg.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);

    // Get the object to calculate where to click
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);
    const tools::Rectangle& rFlyRect = pObject->GetLogicRect();

    SwPageFrame* pPageFrame = static_cast<SwPageFrame*>(pWrtShell->GetLayout()->GetLower());
    const SwLayoutFrame* pBody = pPageFrame->FindBodyCont();
    const SwFrame* pLastPara = pBody->GetLastLower();

    // Image anchored in header, wrapped through extends well over text in body
    // Pick a point halfway between end of text and bottom of image
    Point aPoint(rFlyRect.Center().X(),
                 (pLastPara->getFrameArea().Bottom() + rFlyRect.Bottom()) / 2);
    CPPUNIT_ASSERT(rFlyRect.Contains(aPoint));
    CPPUNIT_ASSERT(pBody->getFrameArea().Contains(aPoint));
    CPPUNIT_ASSERT_GREATER(pLastPara->getFrameArea().Bottom(), aPoint.Y());

    vcl::Window& rEditWin = getSwDocShell()->GetView()->GetEditWin();

    auto lcl_mouseClick = [&rEditWin](const Point& rDocPos)
    {
        const Point aPixPos = rEditWin.LogicToPixel(rDocPos);
        MouseEvent aEvent(aPixPos, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
        rEditWin.MouseButtonDown(aEvent);
        rEditWin.MouseButtonUp(aEvent);
        Scheduler::ProcessEventsToIdle();
    };

    // Without the fix the click would pick the wrap-through bg image anchored in header, even when editing body
    lcl_mouseClick(aPoint);
    CPPUNIT_ASSERT_MESSAGE(
        "image anchored in header unexpectedly selected by click when not editing header",
        pWrtShell->GetSelectedFlyFrame() == nullptr);

    // Ensure expected behavior when clicking while editing header: image should become selected
    pWrtShell->ToggleHeaderFooterEdit();
    lcl_mouseClick(aPoint);
    CPPUNIT_ASSERT_MESSAGE(
        "image anchored in header should be selected by click when editing header",
        pWrtShell->GetSelectedFlyFrame() != nullptr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf36181_findReplaceParaStyle)
{
    // given a document with various paragraph styles

    createSwDoc("tdf36181_findReplaceParaStyle.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();

    // initialize find/replace static environment
    SfxItemSet aSet(rView.GetPool(), svl::Items<SID_SEARCH_ITEM, SID_SEARCH_ITEM>);
    rView.StateSearch(aSet); // initializes SwView::GetSearchItem
    SvxSearchItem& rInit = *SwView::GetSearchItem();
    rInit.SetSearchString(u"Body Text"_ustr);
    rInit.SetReplaceString(u"Caption"_ustr);
    rInit.SetPattern(true); // paragraph styles replacement
    rInit.SetSelection(true); // only search inside of the current selection
    rInit.SetBackward(false); // search forward - important to specify to avoid other tests...
    rInit.SetCommand(SvxSearchCmd::REPLACE);

    // pre-select something - that is what the bug is about...
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Execute 'Replace' one instance
    SfxItemSet aFn(rView.GetPool(), svl::Items<FN_REPEAT_SEARCH, FN_REPEAT_SEARCH>);
    SfxRequest aRequest(FN_REPEAT_SEARCH, SfxCallMode::SYNCHRON, aFn);
    rView.ExecSearch(aRequest);

    // Without the fix, the Title style was also replaced by the Caption style
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Caption"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    // Without the fix, all of the selection applied the Caption style
    CPPUNIT_ASSERT_EQUAL(u"Text body indent"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
}

/// Fake translator for SwTranslateHelper::TranslateRanges: strips the HTML
/// tags of the exported fragment and returns the content prefixed with "de:".
OString FakeTranslate(const OString& rHtml)
{
    OStringBuffer aText;
    bool bInTag = false;
    for (sal_Int32 i = 0; i < rHtml.getLength(); ++i)
    {
        const char c = rHtml[i];
        if (c == '<')
            bInTag = true;
        else if (c == '>')
            bInTag = false;
        else if (!bInTag)
            aText.append(c);
    }
    return "<span>de:"_ostr + aText.makeStringAndClear() + "</span>"_ostr;
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTranslateTextSelection)
{
    createSwDoc();
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();
    pWrtSh->Insert(u"Hello World"_ustr);

    // Select the whole paragraph and "translate" it.
    pWrtSh->SelAll();
    sal_Int32 nCalls = 0;
    auto aTranslate = [&nCalls](const OString& rHtml, OString& /*rError*/) -> OString
    {
        ++nCalls;
        return FakeTranslate(rHtml);
    };
    bool bCancel = false;
    CPPUNIT_ASSERT(SwTranslateHelper::TranslateRanges(*pWrtSh, aTranslate, bCancel));

    // The original text is replaced by the translation, not kept.
    CPPUNIT_ASSERT_EQUAL(u"de:Hello World"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nCalls);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTranslateTableSelection)
{
    createSwDoc();
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();

    // Insert a 3-row x 2-column table.
    SwInsertTableOptions aTableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtSh->InsertTable(aTableOpt, 3, 2);

    // Fill the cells via the model (independent of layout): the text cursor
    // after InsertTable would need layout to step between the cells.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTable.is());
    for (const char* pCellName : { "A1", "B1", "A2", "B2", "A3", "B3" })
    {
        uno::Reference<text::XText> xCellText(
            xTable->getCellByName(OUString::createFromAscii(pCellName)), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCellText.is());
        xCellText->setString(OUString::createFromAscii(pCellName));
    }

    // Move the cursor back to the start of the table: after InsertTable it is
    // outside the table, where SelTable() would fail.
    CPPUNIT_ASSERT(pWrtSh->MoveTable(GotoPrevTable, fnTableStart));

    // Select the whole table, i.e. a table box selection of all 6 cells.
    pWrtSh->SelTable();
    CPPUNIT_ASSERT(pWrtSh->IsTableMode());

    sal_Int32 nCalls = 0;
    auto aTranslate = [&nCalls](const OString& rHtml, OString& /*rError*/) -> OString
    {
        ++nCalls;
        return FakeTranslate(rHtml);
    };
    bool bCancel = false;
    CPPUNIT_ASSERT(SwTranslateHelper::TranslateRanges(*pWrtSh, aTranslate, bCancel));

    // Every cell was translated separately: without handling the box
    // selection, only the first cell was exported and its translation was
    // pasted into every selected cell, appended after the original text.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), nCalls);
    const std::pair<const char*, const char*> aCases[] = {
        { "A1", "de:A1" }, { "B1", "de:B1" }, { "A2", "de:A2" },
        { "B2", "de:B2" }, { "A3", "de:A3" }, { "B3", "de:B3" },
    };
    for (const auto& rCase : aCases)
    {
        uno::Reference<text::XText> xCellText(
            xTable->getCellByName(OUString::createFromAscii(rCase.first)), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCellText.is());
        // The cell contains its own translation and the original text is gone.
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(rCase.second), xCellText->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf129449_findReplaceParaStyle2)
{
    // given a document that starts (and ends) with an empty paragraph with Title style

    createSwDoc("tdf129449_findReplaceParaStyle2.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();

    // initialize find/replace static environment
    SfxItemSet aSet(rView.GetPool(), svl::Items<SID_SEARCH_ITEM, SID_SEARCH_ITEM>);
    rView.StateSearch(aSet); // initializes SwView::GetSearchItem
    SvxSearchItem& rInit = *SwView::GetSearchItem();
    rInit.SetSearchString(u"Title"_ustr);
    rInit.SetReplaceString(u"Caption"_ustr);
    rInit.SetPattern(true); // paragraph styles replacement
    rInit.SetBackward(true); // find the previous result
    rInit.SetCommand(SvxSearchCmd::FIND);

    // Go to the end of document, so we can test finding backwards
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});

    // Execute 'Find' - should find the first paragraph
    SfxItemSet aFn(rView.GetPool(), svl::Items<FN_REPEAT_SEARCH, FN_REPEAT_SEARCH>);
    SfxRequest aRequest(FN_REPEAT_SEARCH, SfxCallMode::SYNCHRON, aFn);
    rView.ExecSearch(aRequest);

    // Sanity check - no change requested yet.
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));

    // Find doesn't tell us much. Change it to replace (it replaces what was already found)
    // and moves to the next item
    rInit.SetCommand(SvxSearchCmd::REPLACE);
    rInit.SetBackward(false); // after replacing, test using search forward for the last paragraph
    rView.ExecSearch(aRequest);

    // Test the replacement of the first paragraph. Without the fix, the Title style was not found.
    CPPUNIT_ASSERT_EQUAL(u"Caption"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));

    // Last Paragraph found - now do the actual replace.
    rView.ExecSearch(aRequest);
    // Test the replacement of the last paragraph. Without the fix, the Title style was not found.
    CPPUNIT_ASSERT_EQUAL(u"Caption"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf129449_findReplaceParaStyle3)
{
    // given a document that starts (and ends) with an empty paragraph with Title style

    createSwDoc("tdf129449_findReplaceParaStyle2.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();

    // initialize find/replace static environment
    SfxItemSet aSet(rView.GetPool(), svl::Items<SID_SEARCH_ITEM, SID_SEARCH_ITEM>);
    rView.StateSearch(aSet); // initializes SwView::GetSearchItem
    SvxSearchItem& rInit = *SwView::GetSearchItem();
    rInit.SetSearchString(u"Title"_ustr);
    rInit.SetPattern(true); // paragraph styles replacement
    rInit.SetBackward(false); // find the next result
    rInit.SetCommand(SvxSearchCmd::FIND);

    // Go to the end of document (wrap around to find the first paragraph in the document)
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pWrtShell->GetCursor()->GetPointNode().GetIndex());

    // Execute search with 'Find' - should find the first paragraph's style
    SfxItemSet aFn(rView.GetPool(), svl::Items<FN_REPEAT_SEARCH, FN_REPEAT_SEARCH>);
    SfxRequest aRequest(FN_REPEAT_SEARCH, SfxCallMode::SYNCHRON, aFn);
    rView.ExecSearch(aRequest);

    // Without the fix, the Cursor had not moved.
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), pWrtShell->GetCursor()->GetPointNode().GetIndex());

    // Go to the start of document (wrap around to find the last paragraph in the document)
    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), pWrtShell->GetCursor()->GetPointNode().GetIndex());

    rInit.SetBackward(true); // find the previous result
    rView.ExecSearch(aRequest);

    // Without the fix, the Cursor had not moved.
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pWrtShell->GetCursor()->GetPointNode().GetIndex());

    // Get the count of a FindAll for the paragraphs with a Title style
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes = xSearch->createSearchDescriptor();

    // specifying the search attributes
    uno::Reference<beans::XPropertySet> xPropSet(xSearchDes, uno::UNO_QUERY_THROW);
    xSearchDes->setPropertyValue(u"SearchStyles"_ustr, uno::Any(true));
    xSearchDes->setSearchString(u"Title"_ustr);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSearch->findAll(xSearchDes)->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf124442)
{
    // Create a new empty Writer document
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SwPaM* pCursor = pShell->GetCursor();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.InsertString(*pCursor, u"Test search not found bug"_ustr);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();

    SfxItemSet aSet(rView.GetPool(), svl::Items<SID_SEARCH_ITEM, SID_SEARCH_ITEM>);
    rView.StateSearch(aSet); // initializes SwView::GetSearchItem
    SvxSearchItem& rSearchItem = *SwView::GetSearchItem();
    rSearchItem.SetCommand(SvxSearchCmd::FIND);

    SfxItemSet aFn(rView.GetPool(), svl::Items<FN_REPEAT_SEARCH, FN_REPEAT_SEARCH>);
    SfxRequest aRequest(FN_REPEAT_SEARCH, SfxCallMode::SYNCHRON, aFn);
    OUString sText;
    OUString sSearchString;

    // Test when a forward search finds a unique match at the end of the document followed by a
    // backward search.
    sSearchString = u"bug"_ustr;
    rSearchItem.SetSearchString(sSearchString);

    rSearchItem.SetBackward(false);
    rView.ExecSearch(aRequest);
    rSearchItem.SetBackward(true);
    rView.ExecSearch(aRequest);

    pWrtShell->GetSelectedText(sText);

    // Without the patch in place assert failed with:
    // equality assertion failed
    // - Expected: bug
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(sSearchString, sText);

    // Test when a backward search finds a unique match at the start of the document followed by a
    // forward search.
    sSearchString = u"Test"_ustr;
    rSearchItem.SetSearchString(sSearchString);

    rSearchItem.SetBackward(true);
    rView.ExecSearch(aRequest);
    rSearchItem.SetBackward(false);
    rView.ExecSearch(aRequest);

    pWrtShell->GetSelectedText(sText);

    // Without the patch in place assert failed with:
    // equality assertion failed
    // - Expected: Test
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(sSearchString, sText);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testDeleteSeveralSelectedImagesTracked)
{
    // With change tracking on, deleting two selected images records a deletion for each and
    // keeps both images in the document until the change is accepted.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pPage->GetObj(0), pView->GetSdrPageView());
    pView->MarkObj(pPage->GetObj(1), pView->GetSdrPageView());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pWrtShell->GetSelectedFlyFrames().size());

    pWrtShell->DelRight();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDoc->GetSpzFrameFormats()->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testDeleteSeveralSelectedImages)
{
    // Deleting a selection that holds two images removes both, and one undo brings both back.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDoc->GetSpzFrameFormats()->size());

    // Mark both images on the draw view, as a multi-selection would.
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pPage->GetObj(0), pView->GetSdrPageView());
    pView->MarkObj(pPage->GetObj(1), pView->GetSdrPageView());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pWrtShell->GetSelectedFlyFrames().size());

    pWrtShell->DelSelectedObj();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pView->GetMarkedObjectList().GetMarkCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetSpzFrameFormats()->size());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDoc->GetSpzFrameFormats()->size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTransformSeveralSelectedImages)
{
    // A position sent for a selection of two images moves both by the same distance, and a
    // width sent for it scales both.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pPage->GetObj(0), pView->GetSdrPageView());
    pView->MarkObj(pPage->GetObj(1), pView->GetSdrPageView());
    // The frame shell handles the transform command. The view switches to it on a timer
    // after a selection change, so do that switch here.
    pWrtShell->GetView().SelectShell();

    std::vector<SwFlyFrame*> aFlyFrames = pWrtShell->GetSelectedFlyFrames();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aFlyFrames.size());
    const SwRect aOldFirst(aFlyFrames[0]->getFrameArea());
    const SwRect aOldSecond(aFlyFrames[1]->getFrameArea());
    const tools::Rectangle aOldUnion(pView->GetMarkedObjRect());

    // The requested position is the new top left corner of the whole selection.
    const tools::Long nDeltaX = 500;
    const tools::Long nDeltaY = 300;
    dispatchCommand(mxComponent, u".uno:TransformDialog"_ustr,
                    { comphelper::makePropertyValue(u"TransformPosX"_ustr,
                                                    sal_Int32(aOldUnion.Left() + nDeltaX)),
                      comphelper::makePropertyValue(u"TransformPosY"_ustr,
                                                    sal_Int32(aOldUnion.Top() + nDeltaY)) });

    CPPUNIT_ASSERT_EQUAL(aOldFirst.Left() + nDeltaX, aFlyFrames[0]->getFrameArea().Left());
    CPPUNIT_ASSERT_EQUAL(aOldFirst.Top() + nDeltaY, aFlyFrames[0]->getFrameArea().Top());
    CPPUNIT_ASSERT_EQUAL(aOldSecond.Left() + nDeltaX, aFlyFrames[1]->getFrameArea().Left());
    CPPUNIT_ASSERT_EQUAL(aOldSecond.Top() + nDeltaY, aFlyFrames[1]->getFrameArea().Top());

    // Doubling the width of the whole selection doubles the width of each image.
    const tools::Rectangle aMovedUnion(pView->GetMarkedObjRect());
    dispatchCommand(mxComponent, u".uno:TransformDialog"_ustr,
                    { comphelper::makePropertyValue(u"TransformWidth"_ustr,
                                                    sal_uInt32(aMovedUnion.GetWidth() * 2)) });

    // The scaled width may be one twip off after the fraction is applied.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aOldFirst.Width() * 2.0, aFlyFrames[0]->getFrameArea().Width(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aOldSecond.Width() * 2.0, aFlyFrames[1]->getFrameArea().Width(), 1.0);
    CPPUNIT_ASSERT_EQUAL(aOldFirst.Height(), aFlyFrames[0]->getFrameArea().Height());

    // One undo step takes the resize back, a second one the move.
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(aOldFirst.Width(), aFlyFrames[0]->getFrameArea().Width());
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(aOldFirst.Left(), aFlyFrames[0]->getFrameArea().Left());
    CPPUNIT_ASSERT_EQUAL(aOldSecond.Left(), aFlyFrames[1]->getFrameArea().Left());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testCopySeveralSelectedImages)
{
    // Copying a selection of two images and pasting it adds two more images to the document.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pPage->GetObj(0), pView->GetSdrPageView());
    pView->MarkObj(pPage->GetObj(1), pView->GetSdrPageView());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pWrtShell->GetSelectedFlyFrames().size());

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    // Paste into the text behind the anchor paragraph, with no image selected.
    pWrtShell->UnSelectFrame();
    pWrtShell->LeaveSelFrameMode();
    pWrtShell->EndOfSection();
    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pDoc->GetSpzFrameFormats()->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testSelectSeveralImages)
{
    // A second image added to a selected image keeps both selected as one graphic selection,
    // adding a selected image again takes it out, and a plain selection replaces the set.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pFirst = pPage->GetObj(0);
    SdrObject* pSecond = pPage->GetObj(1);

    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pFirst));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pWrtShell->GetSelectedFlyFrames().size());
    CPPUNIT_ASSERT(!pWrtShell->IsMultipleFlyFramesSelected());

    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pSecond));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pWrtShell->GetSelectedFlyFrames().size());
    CPPUNIT_ASSERT(pWrtShell->IsFrameSelected());
    CPPUNIT_ASSERT(pWrtShell->IsMultipleFlyFramesSelected());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SelectionType::Graphic),
                         static_cast<int>(pWrtShell->GetSelectionType()));
    // The first image stands for the selection.
    CPPUNIT_ASSERT_EQUAL(DynCastSwVirtFlyDrawObj(pFirst)->GetFlyFrame(),
                         pWrtShell->GetSelectedFlyFrame());

    // Adding the first image again removes it, and the second one stays selected alone.
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pFirst));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pWrtShell->GetSelectedFlyFrames().size());
    CPPUNIT_ASSERT_EQUAL(DynCastSwVirtFlyDrawObj(pSecond)->GetFlyFrame(),
                         pWrtShell->GetSelectedFlyFrame());

    // Removing the last image leaves nothing selected and the cursor in the text.
    CPPUNIT_ASSERT(!pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pSecond));
    CPPUNIT_ASSERT(!pWrtShell->IsFrameSelected());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SelectionType::Text),
                         static_cast<int>(pWrtShell->GetSelectionType()));

    // A selection without the add flag replaces a set of images.
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pFirst));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pSecond));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pSecond));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pWrtShell->GetSelectedFlyFrames().size());
    CPPUNIT_ASSERT_EQUAL(DynCastSwVirtFlyDrawObj(pSecond)->GetFlyFrame(),
                         pWrtShell->GetSelectedFlyFrame());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testSelectImagesAgainAfterUndoOfDeletion)
{
    // After two selected images are deleted with the Delete key and the deletion is undone,
    // each image can be selected again, by its object and by a click at its centre.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrView* pView = pWrtShell->GetDrawView();

    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pPage->GetObj(0)));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pPage->GetObj(1)));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pWrtShell->GetSelectedFlyFrames().size());

    pWrtShell->DelRight();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    pWrtShell->Do(SwWrtShell::UNDO);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPage->GetObjCount());

    for (size_t nObject = 0; nObject < 2; ++nObject)
    {
        SdrObject* pObject = pPage->GetObj(nObject);
        CPPUNIT_ASSERT(pObject->IsInserted());
        CPPUNIT_ASSERT(pView->GetSdrPageView()->IsObjMarkable(pObject));

        CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pObject));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pWrtShell->GetSelectedFlyFrames().size());
        CPPUNIT_ASSERT_EQUAL(DynCastSwVirtFlyDrawObj(pObject)->GetFlyFrame(),
                             pWrtShell->GetSelectedFlyFrame());

        // A click at the centre of the image selects it as well.
        pWrtShell->UnSelectFrame();
        CPPUNIT_ASSERT(!pWrtShell->IsFrameSelected());
        CPPUNIT_ASSERT(pWrtShell->SelectObj(pObject->GetSnapRect().Center()));
        CPPUNIT_ASSERT_EQUAL(DynCastSwVirtFlyDrawObj(pObject)->GetFlyFrame(),
                             pWrtShell->GetSelectedFlyFrame());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testSelectionPictureOfSeveralImages)
{
    // The SVG picture of a selection that holds two images shows both images at the places
    // they have in the layout, and each image reports that place as its layout position.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pPage->GetObj(0)));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pPage->GetObj(1)));
    std::vector<SwFlyFrame*> aFlyFrames = pWrtShell->GetSelectedFlyFrames();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aFlyFrames.size());

    for (size_t nImage = 0; nImage < 2; ++nImage)
    {
        const SwRect& rArea = aFlyFrames[nImage]->getFrameArea();
        const awt::Point aPosition
            = getProperty<awt::Point>(getShape(nImage + 1), u"LayoutPosition"_ustr);
        CPPUNIT_ASSERT_EQUAL(o3tl::convert(rArea.Left(), o3tl::Length::twip, o3tl::Length::mm100),
                             static_cast<tools::Long>(aPosition.X));
        CPPUNIT_ASSERT_EQUAL(o3tl::convert(rArea.Top(), o3tl::Length::twip, o3tl::Length::mm100),
                             static_cast<tools::Long>(aPosition.Y));
    }

    // Export the selection the way the selection picture is made.
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutput = new utl::OOutputStreamWrapper(aStream);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    xStorable->storeToURL(u"private:stream"_ustr,
                          { comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_svg_Export"_ustr),
                            comphelper::makePropertyValue(u"SelectionOnly"_ustr, true),
                            comphelper::makePropertyValue(u"OutputStream"_ustr, xOutput) });
    aStream.Seek(0);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // The picture holds one graphic per image, the second to the right of the first.
    assertXPath(pXmlDoc, "//*[@class='Graphic']", 2);
    const sal_Int32 nFirstX = getXPath(pXmlDoc, "(//*[@class='BoundingBox'])[1]", "x").toInt32();
    const sal_Int32 nSecondX = getXPath(pXmlDoc, "(//*[@class='BoundingBox'])[2]", "x").toInt32();
    CPPUNIT_ASSERT(nSecondX > nFirstX);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTransformStaysEnabledForSeveralImages)
{
    // With two images selected, moving and resizing them stays available while the commands
    // that act on one image are disabled.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pPage->GetObj(0)));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pPage->GetObj(1)));
    CPPUNIT_ASSERT(pWrtShell->IsMultipleFlyFramesSelected());
    // The view switches to the frame shell on a timer after a selection change, so do that
    // switch here.
    SwView& rView = pWrtShell->GetView();
    rView.SelectShell();

    std::unique_ptr<SfxPoolItem> pItem;
    CPPUNIT_ASSERT(rView.GetViewFrame().GetBindings().QueryState(SID_ATTR_TRANSFORM, pItem)
                   != SfxItemState::DISABLED);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED,
                         rView.GetViewFrame().GetBindings().QueryState(FN_FORMAT_FRAME_DLG, pItem));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testAccessibleSelectionOfSeveralImages)
{
    // Every accessible context counts only the selected images that are its own children, and
    // hands out exactly those by index.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pPage->GetObj(0)));
    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pPage->GetObj(1)));
    CPPUNIT_ASSERT(pWrtShell->IsMultipleFlyFramesSelected());

    uno::Reference<accessibility::XAccessible> xDocumentAccessible
        = pWrtShell->GetView().GetEditWin().GetAccessible();
    uno::Reference<accessibility::XAccessibleContext> xDocumentContext
        = xDocumentAccessible->getAccessibleContext();
    uno::Reference<accessibility::XAccessibleSelection> xDocumentSelection(xDocumentContext,
                                                                           uno::UNO_QUERY_THROW);
    // Both images hang below the document, so both are its selected children.
    CPPUNIT_ASSERT_EQUAL(sal_Int64(2), xDocumentSelection->getSelectedAccessibleChildCount());
    for (sal_Int64 nIndex = 0; nIndex < 2; ++nIndex)
    {
        uno::Reference<accessibility::XAccessible> xChild
            = xDocumentSelection->getSelectedAccessibleChild(nIndex);
        CPPUNIT_ASSERT(xChild.is());
        CPPUNIT_ASSERT_EQUAL(accessibility::AccessibleRole::GRAPHIC,
                             xChild->getAccessibleContext()->getAccessibleRole());
    }

    // The paragraph holds neither image, so it has no selected child. Before, it reported two
    // and threw for the first index.
    uno::Reference<accessibility::XAccessibleSelection> xParagraphSelection;
    for (sal_Int64 nIndex = 0; nIndex < xDocumentContext->getAccessibleChildCount(); ++nIndex)
    {
        uno::Reference<accessibility::XAccessibleContext> xChildContext
            = xDocumentContext->getAccessibleChild(nIndex)->getAccessibleContext();
        if (xChildContext->getAccessibleRole() == accessibility::AccessibleRole::PARAGRAPH)
        {
            xParagraphSelection.set(xChildContext, uno::UNO_QUERY);
            break;
        }
    }
    CPPUNIT_ASSERT(xParagraphSelection.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), xParagraphSelection->getSelectedAccessibleChildCount());
}

// Collects the children that an accessible context reports as newly selected.
class SelectionEventListener : public cppu::WeakImplHelper<accessibility::XAccessibleEventListener>
{
public:
    std::vector<uno::Reference<accessibility::XAccessible>> m_aSelectedChildren;

    void notifyEvent(const accessibility::AccessibleEventObject& rEvent) override
    {
        if (rEvent.EventId != accessibility::AccessibleEventId::SELECTION_CHANGED)
            return;
        uno::Reference<accessibility::XAccessible> xChild;
        rEvent.NewValue >>= xChild;
        m_aSelectedChildren.push_back(xChild);
    }

    void disposing(const lang::EventObject&) override {}
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testAccessibleEventForImageJoiningSelection)
{
    // An image that joins the selection is announced to the document's accessible as a newly
    // selected child, the same as the first selected image.
    createSwDoc("two-images.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);

    uno::Reference<accessibility::XAccessibleContext> xDocumentContext
        = pWrtShell->GetView().GetEditWin().GetAccessible()->getAccessibleContext();
    uno::Reference<accessibility::XAccessible> xLeftImage;
    uno::Reference<accessibility::XAccessible> xRightImage;
    for (sal_Int64 nIndex = 0; nIndex < xDocumentContext->getAccessibleChildCount(); ++nIndex)
    {
        uno::Reference<accessibility::XAccessible> xChild = xDocumentContext->getAccessibleChild(nIndex);
        if (xChild->getAccessibleContext()->getAccessibleRole() != accessibility::AccessibleRole::GRAPHIC)
            continue;
        uno::Reference<accessibility::XAccessibleComponent> xComponent(xChild->getAccessibleContext(),
                                                                       uno::UNO_QUERY_THROW);
        if (!xLeftImage.is() || xComponent->getBounds().X < uno::Reference<accessibility::XAccessibleComponent>(
                xLeftImage->getAccessibleContext(), uno::UNO_QUERY_THROW)->getBounds().X)
        {
            xRightImage = xLeftImage;
            xLeftImage = xChild;
        }
        else
            xRightImage = xChild;
    }
    CPPUNIT_ASSERT(xLeftImage.is());
    CPPUNIT_ASSERT(xRightImage.is());

    rtl::Reference<SelectionEventListener> xListener(new SelectionEventListener);
    uno::Reference<accessibility::XAccessibleEventBroadcaster> xBroadcaster(xDocumentContext,
                                                                             uno::UNO_QUERY_THROW);
    xBroadcaster->addAccessibleEventListener(xListener);

    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), 0, pPage->GetObj(0)));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), xListener->m_aSelectedChildren.size());
    CPPUNIT_ASSERT_EQUAL(xLeftImage.get(), xListener->m_aSelectedChildren[0].get());

    CPPUNIT_ASSERT(pWrtShell->SelectObj(Point(), SW_ADD_SELECT, pPage->GetObj(1)));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), xListener->m_aSelectedChildren.size());
    CPPUNIT_ASSERT_EQUAL(xRightImage.get(), xListener->m_aSelectedChildren[1].get());

    xBroadcaster->removeAccessibleEventListener(xListener);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
