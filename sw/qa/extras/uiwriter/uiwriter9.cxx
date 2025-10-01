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
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtinfmt.hxx>
#include <rootfrm.hxx>
#include <dbmgr.hxx>
#include <mmconfigitem.hxx>
#include <svx/svxids.hrc>

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
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenSectionShape)
{
    createSwDoc("section-shape.fodt");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    CPPUNIT_ASSERT(xSections->getByName(u"Section1"_ustr)
                       .queryThrow<css::beans::XPropertySet>()
                       ->getPropertyValue(u"IsVisible"_ustr)
                       .get<bool>());
    CPPUNIT_ASSERT(!xSections->getByName(u"Section2"_ustr)
                        .queryThrow<css::beans::XPropertySet>()
                        ->getPropertyValue(u"IsVisible"_ustr)
                        .get<bool>());

    // shape is anchored in hidden section
    auto xShape = getShape(1).queryThrow<text::XTextContent>();
    CPPUNIT_ASSERT_EQUAL(u"Section2"_ustr, getProperty<uno::Reference<container::XNamed>>(
                                               xShape->getAnchor(), u"TextSection"_ustr)
                                               ->getName());

    // prevent the warning dialog which is automatically cancelled
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Content::Display::ShowWarningHiddenSection::set(false, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard _([&] {
        officecfg::Office::Writer::Content::Display::ShowWarningHiddenSection::set(true, pBatch);
        pBatch->commit();
    });

    // backspace should delete the hidden section
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);
    pWrtShell->DelLeft();

    CPPUNIT_ASSERT_THROW(xSections->getByName(u"Section2"_ustr), container::NoSuchElementException);

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    // this would assert/crash because the shape was anchored to SwEndNode
    save(mpFilter);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testSetStringDeletesShape)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    IDocumentUndoRedo& rUndoManager = pDoc->GetIDocumentUndoRedo();

    sw::UnoCursorPointer pCursor(
        pDoc->CreateUnoCursor(SwPosition(pDoc->GetNodes().GetEndOfContent(), SwNodeOffset(-1))));

    pDoc->getIDocumentContentOperations().InsertString(*pCursor, u"foo"_ustr);
    pDoc->getIDocumentContentOperations().SplitNode(*pCursor->GetPoint(), false);
    pDoc->getIDocumentContentOperations().SplitNode(*pCursor->GetPoint(), false);
    pDoc->getIDocumentContentOperations().InsertString(*pCursor, u"end"_ustr);

    {
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        pWrtShell->StartOfSection(false);
        pWrtShell->Down(/*bSelect=*/false, 1, /*bBasicCall=*/false);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        flySet.Put(anchor);
        SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    auto xCursor = mxComponent.queryThrow<text::XTextDocument>()->getText()->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->goRight(3, false);
    xCursor->gotoEnd(true);
    xCursor->setString("bar"); // replace multi-paragraph selection

    // the problem was that the fly on the middle node was not deleted
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenSectionPDFExport)
{
    // this uses biblio.odb
    createSwDoc("database-display-hidden-section.fodt");

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XTextSection> xSection(xSections->getByIndex(0), uno::UNO_QUERY);

    // apparently without a record it evaluates to hidden?
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsVisible"));
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    // somehow a timer that constructs toolbars isn't run in unit tests -
    // it would query state of .uno:MailMergeEmailDocuments which as a side
    // effect (!) calls this:
    getSwDocShell()->GetView()->EnsureMailMergeConfigItem();

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "English ");
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "English ");
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    uno::Sequence<css::beans::PropertyValue> args{
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL())
    };
    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, args);

    // fetch again in case it was deleted and undo
    xSection.set(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();

    int nObjectsHidden;
    if (pPDFium)
    {
        SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
        SvMemoryStream aMemory;
        aMemory.WriteStream(aPDFFile);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
        CPPUNIT_ASSERT(pPdfDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);
        nObjectsHidden = pPdfPage->getObjectCount();
    }

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "Deutsch ");
    CPPUNIT_ASSERT(getProperty<bool>(xSection, "IsCurrentlyVisible"));

    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, args);

    // fetch again in case it was deleted and undo
    xSection.set(xSections->getByIndex(0), uno::UNO_QUERY);
    // the problem was that PDF export hid the section
    CPPUNIT_ASSERT(getProperty<bool>(xSection, "IsCurrentlyVisible"));

    if (pPDFium)
    {
        SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
        SvMemoryStream aMemory;
        aMemory.WriteStream(aPDFFile);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
        CPPUNIT_ASSERT(pPdfDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);
        // there must be more objects on the page now (currently goes 2->3)
        CPPUNIT_ASSERT_LESS(pPdfPage->getObjectCount(), nObjectsHidden);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenSectionPDFExport2)
{
    // this uses biblio.odb
    createSwDoc("database-display-hidden-section2.fodt");

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XTextSection> xSection(xSections->getByIndex(0), uno::UNO_QUERY);

    // apparently without a record it evaluates to hidden?
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsVisible"));
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    // somehow a timer that constructs toolbars isn't run in unit tests -
    // it would query state of .uno:MailMergeEmailDocuments which as a side
    // effect (!) calls this:
    auto pMMConfig = getSwDocShell()->GetView()->EnsureMailMergeConfigItem();

    // ... but the file has wrong DB name so we first have to fix it
    // in the same way as SwChangeDBDlg
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GetDBManager()->RegisterConnection("Bibliography");
    SwDBData data;
    data.sDataSource = "Bibliography";
    data.sCommand = "biblio";
    pWrtShell->ChgDBData(data);
    // this would be called via a SwXTextView listener
    pMMConfig->updateCurrentDBDataFromDocument();
    pWrtShell->ChangeDBFields({ u"Bibliography2ÿbiblioÿ0"_ustr }, u"Bibliographyÿbiblioÿ0"_ustr);

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "English ");
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "English ");
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    uno::Sequence<css::beans::PropertyValue> args{
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL())
    };
    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, args);

    // fetch again in case it was deleted and undo
    xSection.set(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!getProperty<bool>(xSection, "IsCurrentlyVisible"));

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();

    int nObjectsHidden;
    if (pPDFium)
    {
        SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
        SvMemoryStream aMemory;
        aMemory.WriteStream(aPDFFile);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
        CPPUNIT_ASSERT(pPdfDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);
        nObjectsHidden = pPdfPage->getObjectCount();
    }

    dispatchCommand(mxComponent, u".uno:MailMergeNextEntry"_ustr, {});
    getParagraph(1, "Deutsch ");
    CPPUNIT_ASSERT(getProperty<bool>(xSection, "IsCurrentlyVisible"));

    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, args);

    // fetch again in case it was deleted and undo
    xSection.set(xSections->getByIndex(0), uno::UNO_QUERY);
    // the problem was that PDF export hid the section
    CPPUNIT_ASSERT(getProperty<bool>(xSection, "IsCurrentlyVisible"));

    if (pPDFium)
    {
        SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
        SvMemoryStream aMemory;
        aMemory.WriteStream(aPDFFile);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
        CPPUNIT_ASSERT(pPdfDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);
        // there must be more objects on the page now (currently goes 2->3)
        CPPUNIT_ASSERT_LESS(pPdfPage->getObjectCount(), nObjectsHidden);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenTextFieldPDFExport)
{
    createSwDoc("set-variable-hidden-text.fodt");

    getParagraph(1, "2");
    getParagraph(2, "Expected");

    uno::Sequence<css::beans::PropertyValue> args{
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL())
    };
    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, args);

    getParagraph(1, "2");
    // the problem was that this changed to Wrong
    getParagraph(2, "Expected");

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (pPDFium)
    {
        SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
        SvMemoryStream aMemory;
        aMemory.WriteStream(aPDFFile);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
        CPPUNIT_ASSERT(pPdfDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);
        std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
        CPPUNIT_ASSERT(pPdfTextPage);
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
        OUString sText = pPageObject->getText(pPdfTextPage);
        // the problem was that this changed to Wrong
        CPPUNIT_ASSERT_EQUAL(u"Expected"_ustr, sText);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testPDFExportCrash)
{
    createSwDoc("section-table-section.fodt");

    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf165351)
{
    createSwDoc("tdf165351.fodt");

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Move the cursor into the fly frame
    pWrtShell->GotoFly(u"Frame1"_ustr, FLYCNTTYPE_FRM, false);
    pWrtShell->EndOfSection();
    pWrtShell->GoNextCell(/*bAppendLine=*/true);
    pWrtShell->Undo();
    // getting this item crashed
    SfxItemSet temp{ pDoc->GetAttrPool(), svl::Items<SID_RULER_LR_MIN_MAX, SID_RULER_LR_MIN_MAX> };
    pWrtShell->GetView().StateTabWin(temp);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf160898)
{
    // Given a document with a 1-cell table in another 1-cell table:
    createSwDoc("table-in-table.fodt");
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pXTextDocument->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Move to the normally hidden paragraph inside the outer table cell, following the inner table
    pWrtShell->Down(false, 2);
    // Without the fix, this would crash:
    pWrtShell->SelAll();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf161172)
{
    // Given a paragraph manually made a member of a list:
    createSwDoc("tdf161172.fodt");
    auto para = getParagraph(1);

    // Check initial state: the first paragraph has "No_list" para style, "Num_1" numbering style,
    // numbering level 0, and "Num1_lvl1_1" numbering label.
    CPPUNIT_ASSERT_EQUAL(u"No_list"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl1_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));

    // Assign "Num_1_lvl2" paragraph style to the first paragraph. The style is associated with
    // "Num_1" numbering style, level 1.
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Num_1_lvl2"_ustr) });

    // Check that the respective properties got correctly applied
    CPPUNIT_ASSERT_EQUAL(u"Num_1_lvl2"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl2_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));

    // Undo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Check that the numbering properties got correctly restored
    CPPUNIT_ASSERT_EQUAL(u"No_list"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    // Without the fix, this would fail with
    // - Expected: Num1_lvl1_1
    // - Actual  : Num1_lvl2_1
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl1_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    // Without the fix, this would fail with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf144752)
{
    // Undoing/redoing a replacement must select the new text
    createSwDoc();
    SwXTextDocument* pDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    emulateTyping(*pDoc, u"Some Text");
    CPPUNIT_ASSERT(!pWrtShell->HasSelection());
    // Select "Text", and replace with "Word"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect*/ true, 4, /*bBasicCall*/ false);
    pWrtShell->Replace(u"Word"_ustr, false);
    pWrtShell->EndOfSection();
    CPPUNIT_ASSERT(!pWrtShell->HasSelection());

    // Undo and check, that the "Text" is selected
    dispatchCommand(mxComponent, ".uno:Undo", {});
    // Without the fix, this would fail
    CPPUNIT_ASSERT(pWrtShell->HasSelection());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, pWrtShell->GetSelText());

    // Redo and check, that the "Word" is selected
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT(pWrtShell->HasSelection());
    CPPUNIT_ASSERT_EQUAL(u"Word"_ustr, pWrtShell->GetSelText());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
