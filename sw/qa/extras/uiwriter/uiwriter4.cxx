
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <drawdoc.hxx>
#include <redline.hxx>
#include <dcontact.hxx>
#include <view.hxx>
#include <IDocumentSettingAccess.hxx>

#include <editeng/brushitem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include <i18nutil/transliteration.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <UndoManager.hxx>
#include <tblafmt.hxx>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <osl/thread.hxx>
#include <hyp.hxx>
#include <swdtflvr.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/classificationhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/scheduler.hxx>
#include <config_fonts.h>
#include <unotxdoc.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <officecfg/Office/Writer.hxx>

namespace
{
void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetPointContentNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetPointContentNode(), end);
}
} //namespace

class SwUiWriterTest4 : public SwModelTestBase
{
public:
    SwUiWriterTest4()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }

    void mergeDocs(const char* aDestDoc, const char* aInsertDoc);
};

void SwUiWriterTest4::mergeDocs(const char* aDestDoc, const char* aInsertDoc)
{
    createSwDoc(aDestDoc);

    // set a page cursor into the end of the document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    // insert the same document at current cursor position
    {
        const OUString insertFileid = createFileURL(OUString::createFromAscii(aInsertDoc));
        uno::Sequence<beans::PropertyValue> aPropertyValues(
            comphelper::InitPropertySequence({ { "Name", uno::Any(insertFileid) } }));
        dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf96515)
{
    // Enable hide whitespace mode.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a new paragraph at the end of the document.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(),
                                                            uno::UNO_QUERY);
    xParagraphAppend->finishParagraph(uno::Sequence<beans::PropertyValue>());
    calcLayout();

    // This was 2, a new page was created for the new paragraph.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

static OUString lcl_translitTest(SwDoc& rDoc, const SwPaM& rPaM, TransliterationFlags const nType)
{
    utl::TransliterationWrapper aTrans(::comphelper::getProcessComponentContext(), nType);
    rDoc.getIDocumentContentOperations().TransliterateText(rPaM, aTrans);
    //SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    return rPaM.GetMarkNode().GetTextNode()->GetText();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf146449)
{
    createSwDoc("tdf146449.odt");

    auto pDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDoc);
    auto pShell = pDoc->GetDocShell()->GetFEShell();
    CPPUNIT_ASSERT(pShell);

    auto xTextBox = getShapeByName(u"Frame1");
    auto pObject = SdrObject::getSdrObjectFromXShape(xTextBox);

    CPPUNIT_ASSERT(pShell->SelectObj(Point(), 0, pObject));

    dispatchCommand(mxComponent, ".uno:Cut", {});

    dispatchCommand(mxComponent, ".uno:Undo", {});

    uno::Reference<beans::XPropertySet> xShapeProps(xTextBox, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProps(xShapeProps->getPropertyValue("TextBoxContent"),
                                                    uno::UNO_QUERY);

    const auto& nShapeZOrder = pObject->GetOrdNum();
    const auto& nFrameZOrder = xFrameProps->getPropertyValue("ZOrder");

    CPPUNIT_ASSERT_MESSAGE("Wrong Zorder!", nShapeZOrder < nFrameZOrder.get<sal_uInt32>());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf49033)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Insert the test text at the end of the document.
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("Mary Jones met joe Smith. Time Passed.");
    pWrtShell->StartOfSection();
    SwShellCursor* pCursor = pWrtShell->getShellCursor(false);

    using TF = TransliterationFlags;

    /* -- Test behavior when there is no selection -- */

    /* Move cursor between the 't' and the ' ' after 'met', nothing should change */
    for (int i = 0; i < 14; i++)
        pCursor->Move(fnMoveForward);

    CPPUNIT_ASSERT_EQUAL(false, pCursor->HasMark());
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->IsSelection());
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Move cursor between the 'h' and the '.' after 'Smith', nothing should change */
    for (int i = 0; i < 10; i++)
        pCursor->Move(fnMoveForward);

    CPPUNIT_ASSERT_EQUAL(false, pCursor->HasMark());
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->IsSelection());
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Move cursor between the 'm' and the 'e' in 'met' */
    for (int i = 0; i < 12; i++)
        pCursor->Move(fnMoveBackward);

    CPPUNIT_ASSERT_EQUAL(false, pCursor->HasMark());
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->IsSelection());
    CPPUNIT_ASSERT_EQUAL(OUString("Mary jones met joe smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));

    /* Undo the sentence case change to reset for the following tests */
    pDoc->GetIDocumentUndoRedo().Undo();

    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones Met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones MET joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* -- Test behavior when there is a selection that does not cross a word boundary -- */
    pCursor->Move(fnMoveBackward);
    pWrtShell->SelWrd();
    CPPUNIT_ASSERT_EQUAL(true, pCursor->HasMark());
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->IsSelection());

    OUString currentSelectedText;
    pWrtShell->GetSelectedText(currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("met"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones Met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones Met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones MET joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* -- Test behavior when there is a selection that does not begin at a word boundary: "et" -- */
    for (int i = 0; i < 2; i++)
        pCursor->Move(fnMoveBackward);
    pCursor->SetMark();
    for (int i = 0; i < 2; i++)
        pCursor->Move(fnMoveForward);
    pWrtShell->GetSelectedText(currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("et"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones mEt joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    pDoc->GetIDocumentUndoRedo().Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("et"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones mEt joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    pDoc->GetIDocumentUndoRedo().Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("et"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones mET joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* -- Test behavior when there is a selection that crosses a word boundary -- */
    for (int i = 0; i < 7; i++)
        pCursor->Move(fnMoveBackward);
    pCursor->SetMark();
    for (int i = 0; i < 14; i++)
        pCursor->Move(fnMoveForward);

    pWrtShell->GetSelectedText(currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("nes met joe Sm"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary JoNes met joe smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary JoNes Met Joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary JoNES MET JOE SMith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Reset the 's' to upper-case for the next test */
    for (int i = 0; i < 2; i++)
        pCursor->Move(fnMoveBackward);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, OUString('S'), false);

    /* -- Test behavior when there is a selection that crosses a sentence boundary -- */
    for (int i = 0; i < 4; i++)
        pCursor->Move(fnMoveBackward);
    pCursor->SetMark();
    for (int i = 0; i < 22; i++)
        pCursor->Move(fnMoveForward);
    pWrtShell->GetSelectedText(currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("joe Smith. Time Passed"), currentSelectedText);

    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met Joe smith. Time passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));

    /* Undo the sentence case change to reset for the following tests */
    pDoc->GetIDocumentUndoRedo().Undo();

    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met Joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met JOE SMITH. TIME PASSED."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe smith. time passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Undo the previous changes to reset for the following tests */
    pDoc->GetIDocumentUndoRedo().Undo();
    pDoc->GetIDocumentUndoRedo().Undo();
    pDoc->GetIDocumentUndoRedo().Undo();

    /* -- Test behavior when there is a selection that does not reach end of sentence -- */
    for (int i = 0; i < 37; i++)
        pCursor->Move(fnMoveBackward);
    pCursor->SetMark();
    for (int i = 0; i < 10; i++)
        pCursor->Move(fnMoveForward);
    pWrtShell->GetSelectedText(currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones"), currentSelectedText);
    CPPUNIT_ASSERT_EQUAL(OUString("Mary jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("Mary Jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("MARY JONES met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("mary jones met joe Smith. Time Passed."),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf147196)
{
    using TF = TransliterationFlags;
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Insert the test text at the end of the document.
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert(
        "2.2 Publication of information - CAA\nSection 4.2 of a CA\'s Certificate Policy and/or "
        "Certification Practice Statement SHALL state the CA\'s policy or practice on processing "
        "CAA Records for Fully Qualified Domain Names; that policy shall be consistent with these "
        "Requirements. \n\nIt shall clearly specify the set of Issuer Domain Names that the CA "
        "recognises in CAA \"issue\" or \"issuewild\" records as permitting it to issue. The CA "
        "SHALL log all actions taken, if any, consistent with its processing practice.");

    pWrtShell->StartOfSection();
    SwShellCursor* pCursor = pWrtShell->getShellCursor(false);
    pCursor->SetMark();
    for (int i = 0; i < 510; i++)
    {
        pCursor->Move(fnMoveForward);
    }
    CPPUNIT_ASSERT_EQUAL(
        OUString("2.2 Publication Of Information - Caa\nSection 4.2 Of A Ca\'s Certificate Policy "
                 "And/Or Certification Practice Statement Shall State The Ca\'s Policy Or Practice "
                 "On Processing Caa Records For Fully Qualified Domain Names; That Policy Shall Be "
                 "Consistent With These Requirements. \n\nIt Shall Clearly Specify The Set Of "
                 "Issuer Domain Names That The Ca Recognises In Caa \"Issue\" Or \"Issuewild\" "
                 "Records As Permitting It To Issue. The Ca Shall Log All Actions Taken, If Any, "
                 "Consistent With Its Processing Practice."),
        lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf148148)
{
    using TF = TransliterationFlags;
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("   text");

    /* Test what happens when node contains text but selection does not contain any text */
    pWrtShell->StartOfSection();
    SwShellCursor* pCursor = pWrtShell->getShellCursor(false);
    pCursor->SetMark();
    for (int i = 0; i < 3; i++)
    {
        pCursor->Move(fnMoveForward);
    }
    CPPUNIT_ASSERT_EQUAL(OUString("   text"), lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   text"), lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   text"),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   text"),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Test what happens when node contains text but selection does not contain any text */
    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("text   ");

    pWrtShell->StartOfSection();
    pCursor = pWrtShell->getShellCursor(false);
    for (int i = 0; i < 4; i++)
    {
        pCursor->Move(fnMoveForward);
    }
    pCursor->SetMark();
    for (int i = 0; i < 2; i++)
    {
        pCursor->Move(fnMoveForward);
    }

    CPPUNIT_ASSERT_EQUAL(OUString("text   "), lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("text   "), lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("text   "),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("text   "),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Test what happens when node contains only non-word text but selection does not contain any text */
    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("-1   ");

    pWrtShell->StartOfSection();
    pCursor = pWrtShell->getShellCursor(false);
    for (int i = 0; i < 2; i++)
    {
        pCursor->Move(fnMoveForward);
    }
    pCursor->SetMark();
    for (int i = 0; i < 2; i++)
    {
        pCursor->Move(fnMoveForward);
    }

    CPPUNIT_ASSERT_EQUAL(OUString("-1   "), lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("-1   "), lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("-1   "),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("-1   "),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("   -1");

    pWrtShell->StartOfSection();
    pCursor = pWrtShell->getShellCursor(false);
    pCursor->SetMark();
    for (int i = 0; i < 2; i++)
    {
        pCursor->Move(fnMoveForward);
    }

    CPPUNIT_ASSERT_EQUAL(OUString("   -1"), lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"), lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));

    /* Test what happens when node and selection contains only non-word text */
    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("   -1");

    pWrtShell->StartOfSection();
    pCursor = pWrtShell->getShellCursor(false);
    pCursor->SetMark();
    for (int i = 0; i < 5; i++)
    {
        pCursor->Move(fnMoveForward);
    }

    CPPUNIT_ASSERT_EQUAL(OUString("   -1"), lcl_translitTest(*pDoc, *pCursor, TF::SENTENCE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"), lcl_translitTest(*pDoc, *pCursor, TF::TITLE_CASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"),
                         lcl_translitTest(*pDoc, *pCursor, TF::LOWERCASE_UPPERCASE));
    CPPUNIT_ASSERT_EQUAL(OUString("   -1"),
                         lcl_translitTest(*pDoc, *pCursor, TF::UPPERCASE_LOWERCASE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf96943)
{
    // Enable hide whitespace mode.
    createSwDoc("tdf96943.odt");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf96536)
{
    // Enable hide whitespace mode.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a page break and go back to the first page.
    pWrtShell->InsertPageBreak();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    calcLayout();
    sal_Int32 nSingleParaPageHeight
        = parseDump("/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    discardDumpedLayout();

    // Insert a 2nd paragraph at the end of the first page, so the page height grows at least twice...
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(),
                                                            uno::UNO_QUERY);
    const uno::Reference<text::XTextRange> xInsertPos = getRun(getParagraph(1), 1);
    xParagraphAppend->finishParagraphInsert(uno::Sequence<beans::PropertyValue>(), xInsertPos);
    calcLayout();
    CPPUNIT_ASSERT(parseDump("/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32()
                   >= 2 * nSingleParaPageHeight);
    discardDumpedLayout();

    // ... and then delete the 2nd paragraph, which shrinks the page to the previous size.
    uno::Reference<lang::XComponent> xParagraph(getParagraph(2), uno::UNO_QUERY);
    xParagraph->dispose();
    calcLayout();
    CPPUNIT_ASSERT_EQUAL(nSingleParaPageHeight,
                         parseDump("/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf96479)
{
    // We want to verify the empty input text field in the bookmark
    static const OUString emptyInputTextField
        = OUStringChar(CH_TXT_ATR_INPUTFIELDSTART) + OUStringChar(CH_TXT_ATR_INPUTFIELDEND);

    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // So we can clean up all references for reload
    {
        // Append bookmark
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        sw::mark::IMark* pMark = rIDMA.makeMark(
            aPaM, "original", IDocumentMarkAccess::MarkType::BOOKMARK, ::sw::mark::InsertMode::New);
        CPPUNIT_ASSERT(!pMark->IsExpanded());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());

        // Get helper objects
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

        // Create cursor from bookmark
        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("original"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        uno::Reference<text::XTextCursor> xCursor
            = xRange->getText()->createTextCursorByRange(xRange);
        CPPUNIT_ASSERT(xCursor->isCollapsed());

        // Remove bookmark
        xRange->getText()->removeTextContent(xTextContent);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getBookmarksCount());

        // Insert replacement bookmark
        uno::Reference<text::XTextContent> xBookmarkNew(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
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
        uno::Reference<text::XTextField> xTextField(
            xFactory->createInstance("com.sun.star.text.TextField.Input"), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursorNew(
            xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(
                xBookmarkNew->getAnchor()));
        CPPUNIT_ASSERT(!xCursorNew->isCollapsed());
        xCursorNew->getText()->insertTextContent(xCursorNew, xTextField, true);
        xBookmarkNew = uno::Reference<text::XTextContent>(
            xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        xCursorNew = xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(
            xBookmarkNew->getAnchor());
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
        saveAndReload("writer8");
        pDoc = getSwDoc();

        // Lookup "replacement" bookmark
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xBookmarksSupplier->getBookmarks()->hasByName("replacement"));

        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        uno::Reference<text::XTextCursor> xCursor
            = xRange->getText()->createTextCursorByRange(xRange);
        CPPUNIT_ASSERT(!xCursor->isCollapsed());

        // Verify bookmark content via text node / PaM
        auto mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());
        SwPaM pam(mark->GetMarkStart(), mark->GetMarkEnd());
        CPPUNIT_ASSERT_EQUAL(emptyInputTextField, pam.GetText());
    }
}

// If you resave original document the bookmark will be changed from
//
//  <text:p text:style-name="Standard">
//      <text:bookmark-start text:name="test"/>
//      <text:bookmark-end text:name="test"/>
//      def
//  </text:p>
//
// to
//
//  <text:p text:style-name="Standard">
//      <text:bookmark text:name="test"/>
//      def
//  </text:p>
//
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testBookmarkCollapsed)
{
    // load document
    createSwDoc("collapsed_bookmark.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    // save original document
    save("writer8");

    // load only content.xml from the resaved document
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    constexpr OString aPath("/office:document-content/office:body/office:text/text:p"_ostr);

    const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark");
    CPPUNIT_ASSERT_EQUAL(0, pos1); // found, and it is first

    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark-start")); // not found
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark-end")); // not found
}

// 1. Open a new writer document
// 2. Enter the text "abcdef"
// 3. Select "abc"
// 4. Insert a bookmark on "abc" using Insert->Bookmark. Name the bookmark "test".
// 5. Open the navigator (F5)
//    Select the bookmark "test" using the navigator.
// 6. Hit Del, thus deleting "abc" (The bookmark "test" is still there).
// 7. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark text:name="test"/>
//          def
//      </text:p>
//
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRemoveBookmarkText)
{
    // create document
    {
        // create a text document with "abcdef"
        createSwDoc();
        SwDoc* pDoc = getSwDoc();

        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);
            pDoc->getIDocumentContentOperations().InsertString(aPaM, "abcdef");
        }

        // mark "abc" with "testBookmark" bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            sw::mark::IMark* pMark
                = rIDMA.makeMark(aPaM, "testBookmark", IDocumentMarkAccess::MarkType::BOOKMARK,
                                 ::sw::mark::InsertMode::New);

            // verify
            CPPUNIT_ASSERT(pMark->IsExpanded());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // remove text marked with bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            pDoc->getIDocumentContentOperations().DeleteRange(aPaM);

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }
    }

    // save document
    save("writer8");

    // load only content.xml from the resaved document
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // Bookmark without text becomes collapsed
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/text:bookmark"_ostr, 1);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/text:bookmark-start"_ostr, 0);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/text:bookmark-end"_ostr, 0);
}

// 1. Open a new writer document
// 2. Enter the text "abcdef"
// 3. Select "abc"
// 4. Insert a bookmark on "abc" using Insert->Bookmark. Name the bookmark "test".
// 5. Open the navigator (F5)
//    Select the bookmark "test" using the navigator.
// 6. Hit Del, thus deleting "abc" (The bookmark "test" is still there).
// 7. Call our macro
//
//      Sub Main
//          bookmark = ThisComponent.getBookmarks().getByName("test")
//          bookmark.getAnchor().setString("abc")
//      End Sub
//
//    The text "abc" gets inserted inside the bookmark "test", and the document now contains the string "abcdef".
// 7. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark-start text:name="test"/>
//          abc
//          <text:bookmark-end text:name="test"/>
//          def
//      </text:p>
//
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRemoveBookmarkTextAndAddNew)
{
    // create document
    {
        // create a text document with "abcdef"
        createSwDoc();
        SwDoc* pDoc = getSwDoc();
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);
            pDoc->getIDocumentContentOperations().InsertString(aPaM, "abcdef");
        }

        // mark "abc" with "testBookmark" bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            sw::mark::IMark* pMark
                = rIDMA.makeMark(aPaM, "testBookmark", IDocumentMarkAccess::MarkType::BOOKMARK,
                                 ::sw::mark::InsertMode::New);

            // verify
            CPPUNIT_ASSERT(pMark->IsExpanded());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // remove text marked with bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            pDoc->getIDocumentContentOperations().DeleteRange(aPaM);

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // write "abc" to area marked with "testBookmark" bookmark
        {
            // Get helper objects
            uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent,
                                                                        uno::UNO_QUERY);

            // Create cursor from bookmark
            uno::Reference<text::XTextContent> xTextContent(
                xBookmarksSupplier->getBookmarks()->getByName("testBookmark"), uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            CPPUNIT_ASSERT_EQUAL(OUString(""), xRange->getString());

            // write "abc"
            xRange->setString("abc");

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }
    }

    // save document
    save("writer8");

    // load only content.xml from the resaved document
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    constexpr OString aPath("/office:document-content/office:body/office:text/text:p"_ostr);

    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark")); // not found
    const int pos2 = getXPathPosition(pXmlDoc, aPath, "bookmark-start");
    const int pos3 = getXPathPosition(pXmlDoc, aPath, "text");
    const int pos4 = getXPathPosition(pXmlDoc, aPath, "bookmark-end");

    CPPUNIT_ASSERT_EQUAL(0, pos2);
    CPPUNIT_ASSERT_EQUAL(1, pos3);
    CPPUNIT_ASSERT_EQUAL(2, pos4);
}

// 1. Load document:
//  <text:p text:style-name="Standard">
//      <text:bookmark-start text:name="test"/>
//      <text:bookmark-end text:name="test"/>
//      def
//  </text:p>
//
// 2. Call our macro
//
//      Sub Main
//          bookmark = ThisComponent.getBookmarks().getByName("test")
//          bookmark.getAnchor().setString("abc")
//      End Sub
//
//    The text "abc" gets inserted inside the bookmark "test", and the document now contains the string "abcdef".
// 3. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark text:name="test"/>
//          abcdef
//      </text:p>
//
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRemoveBookmarkTextAndAddNewAfterReload)
{
    // load document
    createSwDoc("collapsed_bookmark.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    // write "abc" to area marked with "testBookmark" bookmark
    {
        // Get helper objects
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);

        // Create cursor from bookmark
        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("test"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString(""), xRange->getString());

        // write "abc"
        xRange->setString("abc");

        // verify: bookmark is still exist
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
    }

    // save original document
    save("writer8");

    // load only content.xml from the resaved document
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    constexpr OString aPath("/office:document-content/office:body/office:text/text:p"_ostr);

    const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark");
    const int pos2 = getXPathPosition(pXmlDoc, aPath, "text");

    CPPUNIT_ASSERT_EQUAL(0, pos1);
    CPPUNIT_ASSERT_EQUAL(1, pos2);

    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark-start")); // not found
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark-end")); // not found
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf96961)
{
    // Insert a page break.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    calcLayout();

    // Assert that the height of the last page is larger than the height of other pages.
    sal_Int32 nOther = parseDump("/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    sal_Int32 nLast = parseDump("/root/page[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT(nLast > nOther);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf88453)
{
    createSwDoc("tdf88453.odt");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 0: the table does not fit the first page, but it wasn't split
    // to continue on the second page.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf88453Table)
{
    createSwDoc("tdf88453-table.odt");
    calcLayout();
    // This was 2: layout could not split the large outer table in the document
    // into 3 pages.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

namespace
{
int checkShells(const SwDocShell* pSource, const SwDocShell* pDestination)
{
    return int(SfxClassificationHelper::CheckPaste(pSource->getDocProperties(),
                                                   pDestination->getDocProperties()));
}
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testClassificationPaste)
{
    createSwDoc();
    SwDocShell* pSourceShell = getSwDoc()->GetDocShell();
    uno::Reference<lang::XComponent> xSourceComponent = mxComponent;
    mxComponent.clear();

    createSwDoc();
    SwDocShell* pDestinationShell = getSwDoc()->GetDocShell();

    // Not classified source, not classified destination.
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::None),
                         checkShells(pSourceShell, pDestinationShell));

    // Classified source, not classified destination.
    uno::Sequence<beans::PropertyValue> aInternalOnly
        = comphelper::InitPropertySequence({ { "Name", uno::Any(OUString("Internal Only")) } });
    dispatchCommand(xSourceComponent, ".uno:ClassificationApply", aInternalOnly);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::TargetDocNotClassified),
                         checkShells(pSourceShell, pDestinationShell));

    // Classified source and classified destination -- internal only has a higher level than confidential.
    uno::Sequence<beans::PropertyValue> aConfidential
        = comphelper::InitPropertySequence({ { "Name", uno::Any(OUString("Confidential")) } });
    dispatchCommand(mxComponent, ".uno:ClassificationApply", aConfidential);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::DocClassificationTooLow),
                         checkShells(pSourceShell, pDestinationShell));

    xSourceComponent->dispose();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testSmallCaps)
{
    // Create a document, add some characters and select them.
    createSwDoc();
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("text");
    pWrtShell->SelAll();

    // Dispatch the command to make them formatted small capitals.
    dispatchCommand(mxComponent, ".uno:SmallCaps", {});

    // This was css::style::CaseMap::NONE as the shell didn't handle the command.
    CPPUNIT_ASSERT_EQUAL(css::style::CaseMap::SMALLCAPS,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharCaseMap"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf98987)
{
    createSwDoc("tdf98987.docx");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/SdrObject"_ostr,
                "name"_ostr, "Rectangle 1");
    sal_Int32 nRectangle1
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject"_ostr,
                "name"_ostr, "Rectangle 2");
    sal_Int32 nRectangle2
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(nRectangle1, nRectangle2);

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/SdrObject"_ostr,
                "name"_ostr, "Rectangle 3");
    sal_Int32 nRectangle3
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    // This failed: the 3rd rectangle had a smaller "top" value than the 2nd one, it even overlapped with the 1st one.
    CPPUNIT_ASSERT_GREATER(nRectangle2, nRectangle3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf99004)
{
    createSwDoc("tdf99004.docx");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nTextbox1Top
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    sal_Int32 nTextBox1Height
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "height"_ostr)
              .toInt32();
    sal_Int32 nTextBox1Bottom = nTextbox1Top + nTextBox1Height;

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject"_ostr,
                "name"_ostr, "Rectangle 2");
    sal_Int32 nRectangle2Top
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    // This was 3291 and 2531, should be now around 2472 and 2531, i.e. the two rectangles should not overlap anymore.
    CPPUNIT_ASSERT(nTextBox1Bottom < nRectangle2Top);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf84695)
{
    createSwDoc("tdf84695.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now Enter + a key should add some text.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    emulateTyping(*pXTextDocument, u"a");

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, Enter did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf84695NormalChar)
{
    createSwDoc("tdf84695.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'a' should add a character.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    emulateTyping(*pXTextDocument, u"a");

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, pressing a normal character did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf84695Tab)
{
    createSwDoc("tdf84695-tab.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SwContact* pShape = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pShape is a draw shape.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_DRAWFRMFMT), pShape->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'tab' should jump to the other shape.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // And finally make sure the selection has changed.
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwContact* pOtherShape
        = static_cast<SwContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // This failed, 'tab' didn't do anything -> the selected shape was the same.
    CPPUNIT_ASSERT(pOtherShape != pShape);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTableStyleUndo)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sal_Int32 nStyleCount = pDoc->GetTableStyles().size();
    SwTableAutoFormat* pStyle = pDoc->MakeTableStyle("Test Style");
    SvxBrushItem aBackground(COL_LIGHTMAGENTA, RES_BACKGROUND);
    pStyle->GetBoxFormat(0).SetBackground(aBackground);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    // check if attributes are preserved
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    pDoc->DelTableStyle("Test Style");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    // check if attributes are preserved
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);

    // undo delete so we can replace the style
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    SwTableAutoFormat aNewStyle("Test Style2");
    SvxBrushItem aBackground2(COL_LIGHTGREEN, RES_BACKGROUND);
    aNewStyle.GetBoxFormat(0).SetBackground(aBackground2);

    pDoc->ChgTableStyle("Test Style", aNewStyle);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
    rUndoManager.Undo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRedlineCopyPaste)
{
    // regressed in tdf#106746
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "abzdezgh");
    SwTextNode* pTextNode = aPaM.GetPointNode().GetTextNode();

    // Turn on track changes, make changes, turn off track changes
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(true));
    lcl_selectCharacters(aPaM, 2, 3);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "c", false);
    lcl_selectCharacters(aPaM, 6, 7);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "f", false);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(false));

    // Create the clipboard document.
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Select the whole content, copy, delete the original and paste the copied content
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pWrtShell->Delete();
    pWrtShell->Paste(aClipboard);

    // With the bug this is "abzcdefgh", ie. contains the first deleted piece, too
    CPPUNIT_ASSERT_EQUAL(OUString("abcdefgh"), pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf135260)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("test");

    // Turn on track changes
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(true));

    for (int i = 0; i < 4; i++)
    {
        pWrtShell->DelLeft();
    }

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : tes
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRedlineParam)
{
    // Create a document with minimal content.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and end of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());

    // Select the first redline.
    pWrtShell->StartOfSection();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        { { "NextTrackedChange", uno::Any(o3tl::narrowing<sal_uInt16>(rTable[0]->GetId())) } }));
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This failed: the parameter wasn't handled so the next change (zzz) was
    // selected, not the first one (aaa).
    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), pShellCursor->GetText());

    // Select the second redline.
    pWrtShell->StartOfSection();
    aPropertyValues = comphelper::InitPropertySequence(
        { { "NextTrackedChange", uno::Any(o3tl::narrowing<sal_uInt16>(rTable[1]->GetId())) } });
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("zzz"), pShellCursor->GetText());

    // Move the cursor to the start again, and reject the second change.
    pWrtShell->StartOfSection();
    aPropertyValues = comphelper::InitPropertySequence(
        { { "RejectTrackedChange", uno::Any(o3tl::narrowing<sal_uInt16>(rTable[1]->GetId())) } });
    dispatchCommand(mxComponent, ".uno:RejectTrackedChange", aPropertyValues);
    pShellCursor = pWrtShell->getShellCursor(false);

    // This was 'middlezzz', the uno command rejected the redline under the
    // cursor, instead of the requested one.
    CPPUNIT_ASSERT_EQUAL(OUString("aaamiddle"),
                         pShellCursor->GetPoint()->GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRedlineViewAuthor)
{
    // Test that setting an author at an SwView level has effect.

    // Create a document with minimal content.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");
    SwView* pView = pDocShell->GetView();
    constexpr OUString aAuthor(u"A U. Thor"_ustr);
    pView->SetRedlineAuthor(aAuthor);
    pDocShell->SetView(pView);

    // Turn on track changes, and add changes to the start of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");

    // Now assert that SwView::SetRedlineAuthor() had an effect.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
    SwRangeRedline* pRedline = rTable[0];
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, pRedline->GetAuthorString());

    // Insert a comment and assert that SwView::SetRedlineAuthor() affects this as well.
    dispatchCommand(mxComponent, ".uno:.uno:InsertAnnotation", {});
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, xField->getPropertyValue("Author").get<OUString>());

    //Reset the redline author after using it, otherwise, it might interfere with other unittests
    pView->SetRedlineAuthor("Unknown Author");
    pDocShell->SetView(pView);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf91292)
{
    createSwDoc("tdf91292_paraBackground.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Solid background color", drawing::FillStyle_SOLID,
                                 getProperty<drawing::FillStyle>(xPropertySet, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Background Color", Color(0x5C2D91),
                                 getProperty<Color>(xPropertySet, "FillColor"));

    // remove background color
    xPropertySet->setPropertyValue("FillStyle", uno::Any(drawing::FillStyle_NONE));

    // Save it and load it back.
    saveAndReload("Office Open XML Text");

    xPropertySet.set(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No background color", drawing::FillStyle_NONE,
                                 getProperty<drawing::FillStyle>(xPropertySet, "FillStyle"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf78727)
{
    createSwDoc("tdf78727.docx");
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // This was 1: make sure we don't loose the TextBox anchored inside the
    // table that is moved inside a text frame.
    CPPUNIT_ASSERT(SwTextBoxHelper::getCount(pPage) > 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRedlineTimestamp)
{
    // Test that a redline timestamp's second is not always 0.

    // Create a document with minimal content.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and to the end of
    // the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::Any(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");
    osl::Thread::wait(std::chrono::seconds(1));
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    // Inserting additional characters at the start changed the table size to
    // 3, i.e. the first and the second "aaa" wasn't combined.
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");

    // Now assert that at least one of the seconds are not 0.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    if (rTable.size() >= 2
        && rTable[0]->GetRedlineData().GetTimeStamp().GetMin()
               != rTable[1]->GetRedlineData().GetTimeStamp().GetMin())
        // The relatively rare case when waiting for a second also changes the minute.
        return;

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
    sal_uInt16 nSec1 = rTable[0]->GetRedlineData().GetTimeStamp().GetSec();
    sal_uInt16 nSec2 = rTable[1]->GetRedlineData().GetTimeStamp().GetSec();
    // This failed, seconds was always 0.
    CPPUNIT_ASSERT(nSec1 != 0 || nSec2 != 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testCursorWindows)
{
    // Create a new document with one window.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell1 = pDocShell->GetWrtShell();

    // Create a second view and type something.
    pDocShell->GetViewShell()->GetViewFrame().GetDispatcher()->Execute(
        SID_NEWWINDOW, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
    SwWrtShell* pWrtShell2 = pDocShell->GetWrtShell();
    OUString aText("foo");
    pWrtShell2->Insert(aText);

    // Assert that only the cursor of the actual window move, not other cursors.
    SwShellCursor* pShellCursor1 = pWrtShell1->getShellCursor(false);
    SwShellCursor* pShellCursor2 = pWrtShell2->getShellCursor(false);
    // This was 3, not 0 -- cursor of the other window moved.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCursor1->Start()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), pShellCursor2->Start()->GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testLandscape)
{
    // Set page orientation to landscape.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence({ { "AttributePage.Landscape", uno::Any(true) } }));
    dispatchCommand(mxComponent, ".uno:AttributePage", aPropertyValues);

    // Assert that the document model was modified.
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    size_t nPageDesc = pWrtShell->GetCurPageDesc();
    // This failed, page was still portrait.
    CPPUNIT_ASSERT(pWrtShell->GetPageDesc(nPageDesc).GetLandscape());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf95699)
{
    // Open the document with single FORMCHECKBOX field, select all and copy to clipboard
    // then check that clipboard contains the FORMCHECKBOX in text body.
    // Previously that failed.
    createSwDoc("tdf95699.odt");
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pMarkAccess = aClipboard.getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    ::sw::mark::IFieldmark* pFieldMark
        = pMarkAccess->getFieldmarkAfter(SwPosition(aClipboard.GetNodes().GetEndOfExtras()), false);
    CPPUNIT_ASSERT_EQUAL(OUString("vnd.oasis.opendocument.field.FORMCHECKBOX"),
                         pFieldMark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf151548_tabNavigation2)
{
    // given a form-protected doc with 2 unchecked legacy fieldmark checkboxes, 1 modern
    // checkbox, and a couple of other content controls that are not supposed to
    // have their contents selected upon entry into the control (i.e. no placeholder text).
    createSwDoc("tdf151548_tabNavigation2.docx");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getFieldmarksCount());

    // verify that the checkboxes start off in the unchecked state
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        sw::mark::ICheckboxFieldmark* pCheckBox
            = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(*it);
        CPPUNIT_ASSERT(!pCheckBox->IsChecked());
    }

    // Toggle on the legacy checkbox
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 32, KEY_SPACE);
    // Tab to the next control - the modern checkbox
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    // Tab to the next control - the second legacy checkbox, and toggle it on.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 32, KEY_SPACE);
    // Tab to the next control - a plain text control without placeholder text
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    // Tab to the next control - a combobox with custom text
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        sw::mark::ICheckboxFieldmark* pCheckBox
            = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(*it);
        // verify that the legacy checkbox became checked by the first loop.
        CPPUNIT_ASSERT(pCheckBox->IsChecked());

        // This is where it was failing. Tab got stuck moving into the plain text/combobox,
        // so it could never loop around. At this point we are at the end of the loop,
        // so the next tab should take us back to the beginning with the first legacy checkbox.

        // Tab to the legacy checkbox, and toggle it off.
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 32, KEY_SPACE);
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT(!pCheckBox->IsChecked());

        // Tab to the next content control
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf151548_tabNavigation)
{
    // given a form-protected doc with 4 unchecked legacy fieldmark checkboxes (and several modern
    // content controls which all have a tabstop of -1 to disable tabstop navigation to them)
    // we want to test that tab navigation completes and loops around to continue at the beginning.
    createSwDoc("tdf151548_tabNavigation.docm");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pMarkAccess->getFieldmarksCount());

    // Tab and toggle 4 times, verifying beforehand that the state was unchecked
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        sw::mark::ICheckboxFieldmark* pCheckBox
            = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(*it);
        CPPUNIT_ASSERT(!pCheckBox->IsChecked());

        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 32, KEY_SPACE); // toggle checkbox on
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB); // move to next control
        Scheduler::ProcessEventsToIdle();
    }

    // Tab 4 more times, verifying beforehand that the checkbox had been toggle on, then toggles off
    // meaning that looping is working, and no other controls are reacting to the tab key.
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        sw::mark::ICheckboxFieldmark* pCheckBox
            = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(*it);

        CPPUNIT_ASSERT(pCheckBox->IsChecked());
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 32, KEY_SPACE); // toggle checkbox off
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT(!pCheckBox->IsChecked());
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB); // move to next control
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf104032)
{
    // Open the document with FORMCHECKBOX field, select it and copy to clipboard
    // Go to end of document and paste it, then undo
    // Previously that asserted in debug build.
    createSwDoc("tdf104032.odt");
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->StartOfSection();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Copy(aClipboard);
    pWrtShell->EndOfSection();
    pWrtShell->Paste(aClipboard);
    rUndoManager.Undo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf104440)
{
    createSwDoc("tdf104440.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 0: both Text Frames in the document were anchored to a
    // paragraph on page 1, while we expect that the second Text Frame is
    // anchored to a paragraph on page 2.
    assertXPath(pXmlDoc, "//page[2]/body/txt/anchored"_ostr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf104425)
{
    createSwDoc("tdf104425.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The document contains one top-level 1-cell table with minimum row height set to 70 cm,
    // and the cell contents does not exceed the minimum row height.
    // It should span over 3 pages.
    assertXPath(pXmlDoc, "//page"_ostr, 3);
    sal_Int32 nHeight1
        = getXPath(pXmlDoc, "//page[1]/body/tab/row/infos/bounds"_ostr, "height"_ostr).toInt32();
    sal_Int32 nHeight2
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/infos/bounds"_ostr, "height"_ostr).toInt32();
    sal_Int32 nHeight3
        = getXPath(pXmlDoc, "//page[3]/body/tab/row/infos/bounds"_ostr, "height"_ostr).toInt32();
    double fSumHeight_mm = o3tl::convert<double>(nHeight1 + nHeight2 + nHeight3, o3tl::Length::twip,
                                                 o3tl::Length::mm);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(700.0, fSumHeight_mm, 0.05);
}

// accepting change tracking gets stuck on change
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf104814)
{
    createSwDoc("tdf104814.docx");
    SwDoc* pDoc1 = getSwDoc();

    SwEditShell* const pEditShell(pDoc1->GetEditShell());

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);
}

// crash at redo of accepting table change tracking imported from DOCX
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTableRedlineRedoCrash)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    // accept all redlines, Undo and accept all redlines again

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/true);

    rUndoManager.Undo();

    // without the fix, it crashes
    rIDRA.AcceptAllRedline(true);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTableRemoveHasTextChangesOnly)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // disable Record Changes
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // 4 rows in Show Changes mode
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // Accepting tracked deletions results 3 rows
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/true);
    Scheduler::ProcessEventsToIdle();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 3);

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // Accepting again: 3 rows (Undo of HasTextChangesOnly is correct)
    rIDRA.AcceptAllRedline(/*bAccept=*/true);
    Scheduler::ProcessEventsToIdle();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 3);

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // Move the cursor after the redline, and insert some text without change tracking
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Insert("X");

    // Accepting again: 4 rows (extra text keeps the deleted row)
    rIDRA.AcceptAllRedline(/*bAccept=*/true);
    Scheduler::ProcessEventsToIdle();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // delete the extra text with change tracking:
    // this resulted tracked row deletion again, because of missing
    // removing of HasTextChangeOnly SwTabLine property at accepting deletions previously

    // disable Record Changes
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    dispatchCommand(mxComponent, ".uno:SwBackSpace", {});
    rIDRA.AcceptAllRedline(/*bAccept=*/true);
    Scheduler::ProcessEventsToIdle();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 3
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTableRemoveHasTextChangesOnly2)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // disable Record Changes
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // check redline count
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());

    // 4 rows in Show Changes mode
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // Move the cursor to the tracked insertion, after the first redline to activate the
    // acception of the whole table row insertion with a single "Accept Change"
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:AcceptTrackedChange", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // Accepting tracked insertion results still 4 rows, but less redlines
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(11), pEditShell->GetRedlineCount());

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());

    // To check Undo of HasTextChangesOnly reject the same row results 3 rows
    dispatchCommand(mxComponent, ".uno:Escape", {});
    dispatchCommand(mxComponent, ".uno:RejectTrackedChange", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 4 (lost HasTextChangesOnly)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 3);

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf147182_AcceptAllChangesInTableSelection)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // check redline count
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());

    // 4 rows in Show Changes mode
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);

    // Select the first table to get a table selection
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:AcceptTrackedChange", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // Accepting tracked changes in the selected table results 3 rows
    // This was 4 (only text changes of the first selected cell were accepted)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 3);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(8), pEditShell->GetRedlineCount());

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());

    // To check Undo of HasTextChangesOnly reject the same row results 3 rows
    dispatchCommand(mxComponent, ".uno:Escape", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:RejectTrackedChange", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 4 (only text changes of the first selected cell were rejected)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 3);

    // Undo: 4 rows again
    pDoc->GetIDocumentUndoRedo().Undo();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row"_ostr, 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf66405)
{
    // Imported formula should have zero margins
    createSwDoc("tdf66405.docx");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent,
                                                                                uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEmbeddedObjects
        = xEmbeddedObjectsSupplier->getEmbeddedObjects();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf35021_tabOverMarginDemo)
{
#if HAVE_MORE_FONTS
    createSwDoc("tdf35021_tabOverMarginDemo.doc");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Tabs should go past the margin @ ~3381
    sal_Int32 nMargin
        = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    // left tab was 3381 because it got its own full line
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "//SwFixPortion[@type='PortionType::TabLeft']"_ostr, "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~4479", nMargin < nWidth);
    // center tab was 842
    nWidth = getXPath(pXmlDoc, "//SwFixPortion[@type='PortionType::TabCenter']"_ostr, "width"_ostr)
                 .toInt32();
    CPPUNIT_ASSERT_MESSAGE("Center Tab width is ~3521", nMargin < nWidth);
    // right tab was probably the same as center tab.
    nWidth = getXPath(pXmlDoc, "//SwFixPortion[@type='PortionType::TabRight']"_ostr, "width"_ostr)
                 .toInt32();
    CPPUNIT_ASSERT_MESSAGE("Right Tab width is ~2907", sal_Int32(2500) < nWidth);
    // decimal tab was 266
    nWidth = getXPath(pXmlDoc, "//SwFixPortion[@type='PortionType::TabDecimal']"_ostr, "width"_ostr)
                 .toInt32();
    CPPUNIT_ASSERT_MESSAGE("Decimal Tab width is ~4096", nMargin < nWidth);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf106701_tabOverMarginAutotab)
{
    createSwDoc("tdf106701_tabOverMarginAutotab.doc");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The right margin is ~3378
    sal_Int32 nRightMargin
        = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    // Automatic tabstops should never be affected by tabOverMargin compatibility
    // The 1st line's width previously was ~9506
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("1st line's width is less than the right margin", nWidth < nRightMargin);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf104492)
{
    createSwDoc("tdf104492.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The document should split table over 3 pages.
    assertXPath(pXmlDoc, "//page"_ostr, 3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf107025)
{
    // Tdf107025 - characters advance with wrong distance, so that
    // they are cluttered because of negative value or
    // break into multiple lines because of overflow.
    // The test document uses DFKAI-SB shipped with Windows.
    createSwDoc("tdf107025.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Verify the number of characters in each line.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getXPath(pXmlDoc, "(//SwLinePortion)[1]"_ostr, "length"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9),
                         getXPath(pXmlDoc, "(//SwLinePortion)[2]"_ostr, "length"_ostr).toInt32());

    // Do the subsequent test only if the first line can be displayed,
    // in case that the required font does not exist.
    sal_Int32 nWidth1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]"_ostr, "width"_ostr).toInt32();
    if (!nWidth1)
        return;

    CPPUNIT_ASSERT(!parseDump("(//SwLinePortion)[2]"_ostr, "width"_ostr).isEmpty());
    sal_Int32 nWidth2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]"_ostr, "width"_ostr).toInt32();
    sal_Int32 nRatio = nWidth2 / nWidth1;

    CPPUNIT_ASSERT(nRatio >= 9); // Occupy at least 9 cells.
    CPPUNIT_ASSERT(nRatio < 18); // Occupy at most 18 cells.
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf107362)
{
    createSwDoc("tdf107362.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nHeight
        = getXPath(pXmlDoc,
                   "(//SwParaPortion/SwLineLayout/child::*[@type='PortionType::Text'])[1]"_ostr,
                   "height"_ostr)
              .toInt32();
    sal_Int32 nWidth1
        = getXPath(pXmlDoc,
                   "(//SwParaPortion/SwLineLayout/child::*[@type='PortionType::Text'])[1]"_ostr,
                   "width"_ostr)
              .toInt32();
    sal_Int32 nWidth2
        = getXPath(pXmlDoc,
                   "(//SwParaPortion/SwLineLayout/child::*[@type='PortionType::Text'])[2]"_ostr,
                   "width"_ostr)
              .toInt32();
    sal_Int32 nLineWidth
        = getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout"_ostr, "width"_ostr).toInt32();
    sal_Int32 nKernWidth = nLineWidth - nWidth1 - nWidth2;
    // Test only if fonts are available
    if (nWidth1 > 500 && nWidth2 > 200)
    {
        // Kern width should be smaller than 1/3 of the CJK font height.
        CPPUNIT_ASSERT(nKernWidth * 3 < nHeight);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf105417)
{
    createSwDoc("tdf105417.odt");
    SwDoc* pDoc = getSwDoc();
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
    SwHyphWrapper aWrap(pView, xHyphenator, /*bStart=*/false, /*bOther=*/true,
                        /*bSelection=*/false);
    // This never returned, it kept trying to hyphenate the last word
    // (greenbacks) again and again.
    aWrap.SpellDocument();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf105625)
{
    createSwDoc("tdf105625.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Ensure correct initial setting
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(false, batch);
    batch->commit();
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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf125151_protected)
{
    // Similar to testTdf105625 except this is in a protected section,
    // so read-only is already true when fieldmarks are considered.
    createSwDoc("tdf125151_protected.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Ensure correct initial setting
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(false, batch);
    batch->commit();
    pWrtShell->Down(/*bSelect=*/false);
    // The cursor moved inside of the FieldMark textbox.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 1", false, pWrtShell->HasReadonlySel());
    // Move left to the start/definition of the textbox
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 2", true, pWrtShell->HasReadonlySel());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf125151_protectedB)
{
    // Similar to testTdf105625 except this is protected with the Protect_Form compat setting
    createSwDoc("tdf125151_protectedB.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Ensure correct initial setting
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(false, batch);
    batch->commit();
    // The cursor starts inside of the FieldMark textbox.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 1", false, pWrtShell->HasReadonlySel());
    // Move left to the start/definition of the textbox
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 2", true, pWrtShell->HasReadonlySel());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf106736)
{
    createSwDoc("tdf106736-grid.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc,
                   "(//SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft'])[1]"_ostr,
                   "width"_ostr)
              .toInt32();
    // In tdf106736, width of tab overflow so that it got
    // width value around 9200, expected value is around 103
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~103", nWidth < 150);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testMsWordCompTrailingBlanks)
{
    // The option is true in settings.xml
    createSwDoc("MsWordCompTrailingBlanksTrue.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(true, pDoc->getIDocumentSettingAccess().get(
                                   DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Check that trailing spaces spans are put into Hole portion if option is enabled

    assertXPath(pXmlDoc, "/root/page/body/txt"_ostr, 3);

    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*"_ostr, 4);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"         "_ustr); // All the trailing blanks

    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*"_ostr, 4);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"         "_ustr); // All the trailing blanks

    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*"_ostr, 4);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"         "_ustr); // All the trailing blanks

    // The option is false in settings.xml
    createSwDoc("MsWordCompTrailingBlanksFalse.odt");
    pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(false, pDoc->getIDocumentSettingAccess().get(
                                    DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
    calcLayout();
    pXmlDoc = parseLayoutDump();
    // Check that trailing spaces spans are put into Text portions if option is disabled

    assertXPath(pXmlDoc, "/root/page/body/txt"_ostr, 3);

    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*"_ostr, 5);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T   "_ustr); // first colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"   "_ustr); // second colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "portion"_ostr, u"   "_ustr); // third colored trailing blank span here

    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*"_ostr, 5);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T   "_ustr); // first colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"   "_ustr); // second colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "portion"_ostr, u"   "_ustr); // third colored trailing blank span here

    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*"_ostr, 5);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"TEST "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "portion"_ostr, u"   "_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, u"   T   "_ustr); // first colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "portion"_ostr, u"   "_ustr); // second colored trailing blank span here
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "type"_ostr, u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "portion"_ostr, u"   "_ustr); // third colored trailing blank span here

    // MsWordCompTrailingBlanks option should be false by default in new documents
    createSwDoc();
    pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(false, pDoc->getIDocumentSettingAccess().get(
                                    DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));

    // The option should be true if a .docx, .doc or .rtf document is opened
    createSwDoc("MsWordCompTrailingBlanks.docx");
    pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(true, pDoc->getIDocumentSettingAccess().get(
                                   DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testCreateDocxAnnotation)
{
    createSwDoc();

    // insert an annotation with a text
    constexpr OUString aSomeText(u"some text"_ustr);
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Text", uno::Any(aSomeText) },
        { "Author", uno::Any(OUString("me")) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    // Save it as DOCX & load it again
    saveAndReload("Office Open XML Text");

    // get the annotation
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);

    // this was empty instead of "some text"
    CPPUNIT_ASSERT_EQUAL(aSomeText, xField->getPropertyValue("Content").get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf107976)
{
    // Create a document and create two transferables.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf58604)
{
#ifdef _WIN32
    // Allow linebreak character follows hanging punctuation immediately instead of
    // breaking at the start of the next line.
    createSwDoc("tdf58604.odt");
    CPPUNIT_ASSERT_EQUAL(
        OUString("PortionType::Break"),
        parseDump("(/root/page/body/txt/SwParaPortion/SwLineLayout[1]/child::*)[last()]", "type"));
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf112025)
{
    mergeDocs("fdo112025.odt", "fdo112025-insert.docx");

    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf72942)
{
    mergeDocs("fdo72942.docx", "fdo72942-insert.docx");

    // check styles of paragraphs added from [fdo72942.docx]
    const uno::Reference<text::XTextRange> xRun1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Default English (Liberation serif) text with "),
                         xRun1->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"),
                         getProperty<OUString>(xRun1, "CharFontName"));

    const uno::Reference<text::XTextRange> xRun2 = getRun(getParagraph(2), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Header 1 English text (Liberation sans) with "),
                         xRun2->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Sans"), getProperty<OUString>(xRun2, "CharFontName"));

    // check styles of paragraphs added from [fdo72942-insert.docx]
    const uno::Reference<text::XTextRange> xRun3 = getRun(getParagraph(4), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Default German text (Calibri) with "), xRun3->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"),
                         getProperty<OUString>(xRun3, "CharFontName"));

    const uno::Reference<text::XTextRange> xRun4 = getRun(getParagraph(5), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Header 1 German text (Calibri Light) with "),
                         xRun4->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Sans"), getProperty<OUString>(xRun4, "CharFontName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf114306)
{
    createSwDoc("fdo114306.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // There are 2 long paragraphs in cell A1.
    // A part of paragraph 2 should flow over to the second page but
    // *not* the whole paragraph. There should be 2 paragraphs on
    // page 1 and 1 paragraph on page 2.
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf114306_2)
{
    // tdf#114306 fix unexpected page break in row-spanned table
    // load regression document without writer crash
    createSwDoc("fdo114306_2.odt");

    // correct number of pages
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

// During insert of the document with list inside into the main document inside the list
// we should merge both lists into one, when they have the same list properties
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877)
{
    mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list.odt");

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
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877NoMerge)
{
    mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list_abcd.odt");

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
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877_default_style)
{
    mergeDocs(nullptr, "tdf113877_insert_numbered_list_abcd.odt");

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
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877_Standard_style)
{
    mergeDocs("tdf113877_blank_ownStandard.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877_blank_bold_on)
{
    mergeDocs("tdf113877_blank_bold_on.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf107975)
{
    // This test also covers tdf#117185 tdf#110442

    createSwDoc("tdf107975.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // without the fix, it crashes
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //try again with anchor at start of doc which is another special case
    xShape.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xShapeContent(xShape, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xStart = pTextDoc->getText()->getStart();
    xShapeContent->attach(xStart);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

// Fails quite a lot on multiple Jenkins slaves, but entirely reliably,
// with:
// sw/qa/extras/uiwriter/uiwriter4.cxx(2407) : error : Assertion
// - Expected: 2
// - Actual  : 1
// i.e. the xIndexAccess->getCount() line.
#if 0
    // without the fix, it crashes
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf113877_blank_bold_off)
{
    mergeDocs("tdf113877_blank_bold_off.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

// just care that this does crash/assert
CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testRhbz1810732) { mergeDocs(nullptr, "rhbz1810732.docx"); }

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf142157)
{
    mergeDocs(nullptr, "tdf142157.odt");

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest4, testTdf143320)
{
    createSwDoc("tdf143320.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("x"));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Create a new document
    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("x"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("x"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
