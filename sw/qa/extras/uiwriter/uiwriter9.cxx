/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/configuration.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>
#include <editeng/acorrcfg.hxx>
#include <swacorr.hxx>
#include <sfx2/linkmgr.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <fmtinfmt.hxx>
#include <rootfrm.hxx>

namespace
{
class SwUiWriterTest9 : public SwModelTestBase
{
public:
    SwUiWriterTest9()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158785)
{
    // given a document with a hyperlink surrounded by N-dashes (–www.dordt.edu–)
    createSwDoc("tdf158785_hyperlink.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the end of the hyperlink
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last point that will be part of the hyperlink (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are right by the hyperlink
    aLogicL.AdjustX(-1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::InetAttr);
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::InetAttr, aContentAtPos.eContentAtPos);

    // The test: the position of the N-dash should not indicate hyperlink properties
    // cursor pos would NOT be considered part of the hyperlink, but increase for good measure...
    aLogicR.AdjustX(1);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    /*
     * tdf#111969: the beginning of the hyperlink should allow the right-click menu to remove it
     */
    // move cursor (with no selection) to the start of the hyperlink - after the N-dash
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    aLogicL = pWrtShell->GetCharRect().Center();
    aLogicR = aLogicL;

    // sanity check - we really are right in front of the hyperlink
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
    aLogicR.AdjustX(1);
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::InetAttr, aContentAtPos.eContentAtPos);

    // Remove the hyperlink
    dispatchCommand(mxComponent, ".uno:RemoveHyperlink", {});

    // The test: was the hyperlink actually removed?
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf111969)
{
    // given a document with a field surrounded by N-dashes (–date–)
    createSwDoc("tdf111969_field.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the end of the field
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last point that will be part of the field (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are at the right edge of the field
    aLogicR.AdjustX(1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::Field);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(pWrtShell->GetCurField(true));

    /*
     * An edge case at the start of a field - don't start the field menu on the first N-dash
     */
    // go to the start of the field
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get first point that will be part of the field (current position 1pt wide).
    aLogicL = pWrtShell->GetCharRect().Center();
    aLogicR = aLogicL;

    // sanity check - we really are at the left edge of the field
    aLogicR.AdjustX(1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse (at the end-edge of the N-dash)
    // which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(!pWrtShell->GetCurField(true));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf111969B)
{
    // given a document with a field surrounded by two N-dashes (––date––)
    createSwDoc("tdf111969_fieldB.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the start of the field
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    // get first point that will be part of the field (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are at the left edge of the field
    aLogicR.AdjustX(1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::Field);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse (at the end-edge of the second N-dash)
    // which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(!pWrtShell->GetCurField(true));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159049)
{
    // The document contains a shape which has a text with a line break. When copying the text to
    // clipboard the line break was missing in the RTF flavor of the clipboard.
    createSwDoc("tdf159049_LineBreakRTFClipboard.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    selectShape(1);

    // Bring shape into text edit mode
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    // Copy text
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Deactivate text edit mode ...
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();
    // ... and deselect shape.
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } }));
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aArgs);
    // Without fix Actual was "Abreakhere", the line break \n was missing.
    CPPUNIT_ASSERT_EQUAL(u"Abreak\nhere"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf135083)
{
    createSwDoc("tdf135083-simple-text-plus-list.fodt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { u"SelectedFormat"_ustr,
            uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } }));
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aArgs);

    auto xLastPara = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(u"dolor"_ustr, xLastPara->getString());
    // Without the fix in place, the last paragraph would loose its settings. ListId would be empty.
    CPPUNIT_ASSERT(!getProperty<OUString>(xLastPara, u"ListId"_ustr).isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenSectionsAroundPageBreak)
{
    createSwDoc("hiddenSectionsAroundPageBreak.fodt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    auto xModel(mxComponent.queryThrow<frame::XModel>());
    auto xTextViewCursorSupplier(
        xModel->getCurrentController().queryThrow<text::XTextViewCursorSupplier>());
    auto xCursor(xTextViewCursorSupplier->getViewCursor().queryThrow<text::XPageCursor>());

    // Make sure that the page style is set correctly
    xCursor->jumpToFirstPage();
    CPPUNIT_ASSERT_EQUAL(u"Landscape"_ustr, getProperty<OUString>(xCursor, "PageStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159565)
{
    // Given a document with a hidden section in the beginning, additionally containing a frame
    createSwDoc("FrameInHiddenSection.fodt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Check that the selection covers the whole visible text
    auto xModel(mxComponent.queryThrow<css::frame::XModel>());
    auto xSelSupplier(xModel->getCurrentController().queryThrow<css::view::XSelectionSupplier>());
    auto xSelections(xSelSupplier->getSelection().queryThrow<css::container::XIndexAccess>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelections->getCount());
    auto xSelection(xSelections->getByIndex(0).queryThrow<css::text::XTextRange>());

    // Without the fix, this would fail - there was no selection
    CPPUNIT_ASSERT_EQUAL(u"" SAL_NEWLINE_STRING SAL_NEWLINE_STRING "ipsum"_ustr,
                         xSelection->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159816)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Add 5 empty paragraphs
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();

    // Add a bookmark at the very end
    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    rIDMA.makeMark(*pWrtShell->GetCursor(), "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
                   sw::mark::InsertMode::New);

    // Get coordinates of the end point in the document
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pLastPara = pBody->GetLower()->GetNext()->GetNext()->GetNext()->GetNext()->GetNext();
    Point ptTo = pLastPara->getFrameArea().BottomRight();

    pWrtShell->SelAll();

    // Drag-n-drop to its own end
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // Without the fix, this would crash: either in CopyFlyInFlyImpl (tdf#159813):
    // Assertion failed: !pCopiedPaM || pCopiedPaM->End()->GetNode() == rRg.aEnd.GetNode()
    // or in BigPtrArray::operator[] (tdf#159816):
    // Assertion failed: idx < m_nSize
    xTransfer->PrivateDrop(*pWrtShell, ptTo, /*bMove=*/true, /*bXSelection=*/true);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf151710)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Check that the particular setting is turned on by default
    const SwViewOption* pVwOpt = pTextDoc->GetDocShell()->GetWrtShell()->GetViewOptions();
    CPPUNIT_ASSERT(pVwOpt);
    CPPUNIT_ASSERT(pVwOpt->IsEncloseWithCharactersOn());

    // Localized quotation marks
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    CPPUNIT_ASSERT(pACorr);
    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    OUString sStartSingleQuote{ pACorr->GetQuote('\'', true, eLang) };
    OUString sEndSingleQuote{ pACorr->GetQuote('\'', false, eLang) };
    OUString sStartDoubleQuote{ pACorr->GetQuote('\"', true, eLang) };
    OUString sEndDoubleQuote{ pACorr->GetQuote('\"', false, eLang) };

    // Insert some text to work with
    uno::Sequence<beans::PropertyValue> aArgsInsert(
        comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("abcd")) } }));
    dispatchCommand(mxComponent, ".uno:InsertText", aArgsInsert);
    CPPUNIT_ASSERT_EQUAL(OUString("abcd"), pTextDoc->getText()->getString());

    // Successfully enclose the text; afterwards the selection should exist with the new
    // enclosed text
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '(', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("(abcd)"), pTextDoc->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '[', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("[(abcd)]"), pTextDoc->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '{', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("{[(abcd)]}"), pTextDoc->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\'', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString(sStartSingleQuote + "{[(abcd)]}" + sEndSingleQuote),
                         pTextDoc->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\"', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString(sStartDoubleQuote + sStartSingleQuote + "{[(abcd)]}"
                                  + sEndSingleQuote + sEndDoubleQuote),
                         pTextDoc->getText()->getString());

    // Disable the setting and check that enclosing doesn't happen anymore
    const_cast<SwViewOption*>(pVwOpt)->SetEncloseWithCharactersOn(false);
    CPPUNIT_ASSERT(!pVwOpt->IsEncloseWithCharactersOn());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '(', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("("), pTextDoc->getText()->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '[', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("["), pTextDoc->getText()->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '{', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("{"), pTextDoc->getText()->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\'', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sStartSingleQuote, pTextDoc->getText()->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\"', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sStartDoubleQuote, pTextDoc->getText()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158375_dde_disable)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(true, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> _pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(false, _pBatch);
        _pBatch->commit();
    });

    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // force the AppName to enable DDE, it is not there for test runs
    Application::SetAppName("soffice");

    // temp copy for the file that will be used as a reference for DDE link
    // this file includes a section named "Section1" with text inside
    createTempCopy(u"tdf158375_dde_reference.fodt");

    comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer
        = getSwDocShell()->getEmbeddedObjectContainer();
    rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);

    // create a section with DDE link
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSectionProps(
        xFactory->createInstance("com.sun.star.text.TextSection"), uno::UNO_QUERY);

    uno::Sequence<OUString> aNames{ "DDECommandFile", "DDECommandType", "DDECommandElement",
                                    "IsAutomaticUpdate", "IsProtected" };
    uno::Sequence<uno::Any> aValues{ uno::Any(OUString{ "soffice" }), uno::Any(maTempFile.GetURL()),
                                     uno::Any(OUString{ "Section1" }), uno::Any(true),
                                     uno::Any(true) };
    uno::Reference<beans::XMultiPropertySet> rMultiPropSet(xTextSectionProps, uno::UNO_QUERY);
    rMultiPropSet->setPropertyValues(aNames, aValues);

    // insert the TextSection with DDE link
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XParagraphCursor> xCursor(xText->createTextCursor(), uno::UNO_QUERY);
    xText->insertTextContent(
        xCursor, uno::Reference<text::XTextContent>(xTextSectionProps, uno::UNO_QUERY), false);

    CPPUNIT_ASSERT_EQUAL(
        size_t(1), pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());

    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false, nullptr);

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XTextSection> xSection(xSections->getByIndex(0), uno::UNO_QUERY);

    // make sure there's no text in the section after UpdateAllLinks, since
    // DisableActiveContent disables DDE links.
    CPPUNIT_ASSERT_EQUAL(OUString(""), xSection->getAnchor()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158375_ole_object_disable)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(true, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> _pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(false, _pBatch);
        _pBatch->commit();
    });

    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    createSwDoc("ole-save-while-edit.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    selectShape(1);

    // attempt to edit the OLE object.
    pWrtShell->LaunchOLEObj();

    // it shouldn't switch because the current configuration
    // (DisableActiveContent) prohibits OLE objects changing to states other
    // then LOADED
    auto xShape = getShape(1);
    uno::Reference<document::XEmbeddedObjectSupplier2> xEmbedSupplier(xShape, uno::UNO_QUERY);
    auto xEmbeddedObj = xEmbedSupplier->getExtendedControlOverEmbeddedObject();
    CPPUNIT_ASSERT_EQUAL(embed::EmbedStates::LOADED, xEmbeddedObj->getCurrentState());

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
