/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/configuration.hxx>

#include <AnnotationWin.hxx>
#include <cmdid.h>
#include <docufld.hxx>
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <svx/svxids.hrc>

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

    CPPUNIT_ASSERT_EQUAL(OUString("1.1."),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply a non-numbered style to paragraph 1
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr) });
    // the numbering should not be affected by changing a paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("1.1."),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply a numbered style
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"aList"_ustr) });
    // the numbering should be changed to that specified by the paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("i.I.a)"),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply the non-numbered style while holding down the Ctrl-key
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr),
                      comphelper::makePropertyValue(u"KeyModifier"_ustr, uno::Any(KEY_MOD1)) });
    // the numbering should be removed when the Ctrl-key is held down
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "ListLabelString"));
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
    CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(getParagraph(6), "ListLabelString"));

    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(6), "ListLabelString"));
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf162120AutoRTLAfterDirSwitch)
{
    // Tests that Writer no longer tries to update the writing direction after
    // an explicit override.

    createSwDoc("tdf162120-auto-rtl.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // The initial direction should be RTL
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));

    // Explicitly set the direction to RTL
    // This should clear the auto writing mode flag
    dispatchCommand(mxComponent, u".uno:ParaRightToLeft"_ustr, {});
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));

    // Insert a strong LTR character at the start of the paragraph.
    // Since the flag is gone, the writing mode should not automatically switch to LTR.
    pWrtShell->Insert(u"a"_ustr);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf169035ParaStartDefault)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_EQUAL(short(5),
                         getProperty<short>(getRun(getParagraph(1), 1), u"ParaAdjust"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf162120AutoRTLDefault)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
