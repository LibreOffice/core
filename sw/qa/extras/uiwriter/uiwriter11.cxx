/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <officecfg/Office/Writer.hxx>
#include <test/commontesttools.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/propertysequence.hxx>
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
#include <sortedobjs.hxx>
#include <rootfrm.hxx>
#include <anchoredobject.hxx>
#include <flyfrm.hxx>

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

SwPostItMgr* getPostItMgr(SwDocShell* pDocShell)
{
    CPPUNIT_ASSERT(pDocShell);
    SwView* pView = pDocShell->GetView();
    CPPUNIT_ASSERT(pView);
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    CPPUNIT_ASSERT(pPostItMgr);
    return pPostItMgr;
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791)
{
    // Given a document with tracked changes enabled, having some threads of comments:

    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_2)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_3)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_4)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.fodt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_5)
{
    // Now test the same with an equivalent DOCX: comment deletion must delete immediately

    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_6)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_7)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf108791_8)
{
    createSwDoc("tdf108791_comments_with_tracked_changes.docx");

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
    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"ExportNotes"_ustr, false),
                               comphelper::makePropertyValue(u"ExportNotesInMargin"_ustr, true) };
    uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
                               comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL()) };
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
            u"A comment. One two three four"_ustr,
            u"five six seven eight nine ten"_ustr,
            u"eleven twelve thirteen fourteen"_ustr,
            u"fifteen sixteen seventeen"_ustr,
            u"eighteen nineteen twenty."_ustr,
            // Second annotation
            u"Another comment. Twenty-one"_ustr,
            u"twenty-two twenty-three"_ustr,
            u"twenty-four twenty-five twenty-"_ustr,
            u"six twenty-seven twenty-eight"_ustr,
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf165206DirSwitchPreservesAlignment)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    dispatchCommand(mxComponent, u".uno:ParaLeftToRight"_ustr, {});
    dispatchCommand(mxComponent, u".uno:LeftPara"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(short(0),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT_EQUAL(short(0),
                         getProperty<short>(getRun(getParagraph(1), 1), u"ParaAdjust"_ustr));

    // Explicitly set the direction to RTL.
    // Previously, this would have also set ParaAdjust to force right.
    dispatchCommand(mxComponent, u".uno:ParaRightToLeft"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT_EQUAL(short(0),
                         getProperty<short>(getRun(getParagraph(1), 1), u"ParaAdjust"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf169651)
{
    // Given a document with a fly frame that has anchored FLY_AT_FLY, a shape object and a
    // fly frame that itself has a shape object anchored FLY_AT_FLY:
    createSwDoc("tdf169651.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwSortedObjs* pAnchoredObjs
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchoredObjs);
    SwAnchoredObject* pAnchoredObj = (*pAnchoredObjs)[0];
    SwFlyFrame* pFlyFrame = pAnchoredObj->DynCastFlyFrame();
    CPPUNIT_ASSERT(pFlyFrame);

    pWrtShell->SelectFlyFrame(*pFlyFrame);

    // Without the patch this test would crash during the following
    pWrtShell->UnfloatFlyFrame();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf151857ParaStylePreservesWritingMode)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Initially, paragraphs have environment writing mode and automatic writing mode enabled
    CPPUNIT_ASSERT_EQUAL(short(4),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));

    dispatchCommand(mxComponent, u".uno:ParaRightToLeft"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Verify that changing the paragraph style preserves the writing mode DF
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(1), 1), u"WritingMode"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(1), 1), u"WritingModeAutomatic"_ustr));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
