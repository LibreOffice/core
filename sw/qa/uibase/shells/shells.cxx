/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/odffields.hxx>
#include <comphelper/string.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <osl/thread.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
public:
    SwUibaseShellsTest()
        : SwModelTestBase("/sw/qa/uibase/shells/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testTdf130179)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursorShell* pShell(pDoc->GetEditShell());
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet aGrfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
    aFrameSet.Put(aAnchor);
    Graphic aGrf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pShell->GetCursor(), OUString(), OUString(), &aGrf,
                                       &aFrameSet, &aGrfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    SwView* pView = pDoc->GetDocShell()->GetView();
    selectShape(1);

    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame().GetBindings().QueryState(FN_POSTIT, pItem);
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: !pItem
    // i.e. comment insertion was enabled for an at-para anchored image.
    CPPUNIT_ASSERT(!pItem);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testShapeTextAlignment)
{
// FIXME find out why this fails on macOS/Windows
#if !defined(MACOSX) && !defined(_WIN32)
    // Create a document with a rectangle in it.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aStartPos(1000, 1000);
    pWrtShell->BeginCreate(SdrObjKind::Rectangle, aStartPos);
    Point aMovePos(2000, 2000);
    pWrtShell->MoveCreate(aMovePos);
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Start shape text edit.
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Select the shape.
    selectShape(1);
    // Start the actual text edit.
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    pView->EnterShapeDrawTextMode(pObject);
    pView->AttrChangedNotify(nullptr);

    // Change paragraph adjustment to center.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_CENTER,
                                                   SfxCallMode::SYNCHRON);

    // End shape text edit.
    pWrtShell->EndTextEdit();

    const OutlinerParaObject* pOutliner = pObject->GetOutlinerParaObject();
    // Without the accompanying fix in place, this test would have failed, because the shape had no
    // text or text formatting. In other words the paragraph adjustment command was ignored.
    CPPUNIT_ASSERT(pOutliner);
    const SfxItemSet& rParaAttribs = pOutliner->GetTextObject().GetParaAttribs(0);
    SvxAdjust eAdjust = rParaAttribs.GetItem(EE_PARA_JUST)->GetAdjust();
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Center, eAdjust);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testOleSavePreviewUpdate)
{
    // Load a document with 2 charts in it. The second is down enough that you have to scroll to
    // trigger its rendering. Previews are missing for both.
    createSwDoc("ole-save-preview-update.odt");

    // Explicitly update OLE previews, etc.
    dispatchCommand(mxComponent, ".uno:UpdateAll", {});

    // Save the document and see if we get the previews.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // Without the accompanying fix in place, this test would have failed, because the object
    // replacements were not generated, even after UpdateAll.
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 2"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testOlePreviewUpdate)
{
    // Given a document with an embedded Writer object:
    createSwDoc("ole-preview-update.odt");

    // When updating "all" (including OLE previews):
    dispatchCommand(mxComponent, ".uno:UpdateAll", {});

    // Then make sure the preview is no longer a 0-sized stream:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(
        xNameAccess->getByName("ObjectReplacements/Object 1"), uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    // Without the accompanying fix in place, this test would have failed, the stream was still
    // empty.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_uInt64>(0), pStream->remainingSize());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyUrlContextMenu)
{
    // Given a document with a bibliography field:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=1")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxDispatcher* pDispatcher = pDocShell->GetViewShell()->GetViewFrame().GetDispatcher();
    css::uno::Any aState;
    SfxItemState eStateOpen = pDispatcher->QueryState(SID_OPEN_HYPERLINK, aState);
    SfxItemState eStateCopy = pDispatcher->QueryState(SID_COPY_HYPERLINK_LOCATION, aState);

    // Then the "open hyperlink" and "copy hyperlink location" menu items should be visible:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32 (SfxItemState::DEFAULT)
    // - Actual  : 1 (SfxItemState::DISABLED)
    // i.e. the menu item was not visible for biblio entry fields with an URL.
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, eStateOpen);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, eStateCopy);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testProtectedFieldsCopyHyperlinkLocation)
{
    // Given a test document document that contains:
    //      - generic url
    //      - empty line
    //      - bibliography mark
    //      - empty line
    //      - generic url
    //      - empty line
    //      - bibliography table heading
    //      - bibliography entry containing only url
    //      - empty line
    createSwDoc("protectedLinkCopy.fodt");

    // Copy generic hyperlink
    dispatchCommand(mxComponent, ".uno:CopyHyperlinkLocation", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // Assert generic hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(OUString("http://reset.url/1"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoLeft", {});
    // Copy bibliography mark hyperlink
    dispatchCommand(mxComponent, ".uno:CopyHyperlinkLocation", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // Assert bibliography mark hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(OUString("https://test.url/1"), getParagraph(4)->getString());

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoLeft", {});
    // Copy generic hyperlink
    dispatchCommand(mxComponent, ".uno:CopyHyperlinkLocation", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // Assert generic hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(OUString("http://reset.url/2"), getParagraph(6)->getString());

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoLeft", {});
    // Copy bibliography table hyperlink
    dispatchCommand(mxComponent, ".uno:CopyHyperlinkLocation", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // Assert bibliography table entry hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(OUString("https://test.url/1"), getParagraph(9)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyLocalCopyContextMenu)
{
    // Given a document with a bibliography field's local copy:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=1")),
        comphelper::makePropertyValue("LocalURL", OUString("file:///home/me/test.pdf")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxDispatcher* pDispatcher = pDocShell->GetViewShell()->GetViewFrame().GetDispatcher();
    css::uno::Any aState;
    SfxItemState eState = pDispatcher->QueryState(FN_OPEN_LOCAL_URL, aState);

    // Then the "open local copy" menu item should be visible:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32 (SfxItemState::DEFAULT)
    // - Actual  : 1 (SfxItemState::DISABLED)
    // i.e. the context menu was disabled all the time, even for biblio fields.
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, eState);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testContentControlPageBreak)
{
    // Given a document with a content control and a cursor inside the content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // When trying to insert a page break:
    dispatchCommand(mxComponent, ".uno:InsertPagebreak", {});

    // Then make sure that the document still has a single page:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. inline content control had its start and end in different text nodes, which is not
    // allowed.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertTextFormField)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting an ODF_UNHANDLED fieldmark:
    OUString aExpectedCommand("ADDIN ZOTERO_BIBL foo bar");
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand", uno::Any(aExpectedCommand)),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("<p>aaa</p><p>bbb</p>"))),
    };
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);

    // Then make sure that it's type/name is correct:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    pCursor->SttEndDoc(/*bSttDoc=*/true);
    sw::mark::IFieldmark* pFieldmark
        = pDoc->getIDocumentMarkAccess()->getFieldmarkAt(*pCursor->GetPoint());
    CPPUNIT_ASSERT(pFieldmark);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: vnd.oasis.opendocument.field.UNHANDLED
    // - Actual  : vnd.oasis.opendocument.field.FORMTEXT
    // i.e. the custom type parameter was ignored.
    CPPUNIT_ASSERT_EQUAL(ODF_UNHANDLED, pFieldmark->GetFieldname());

    auto it = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
    CPPUNIT_ASSERT(it != pFieldmark->GetParameters()->end());
    OUString aActualCommand;
    it->second >>= aActualCommand;
    CPPUNIT_ASSERT_EQUAL(aExpectedCommand, aActualCommand);

    SwPaM aPam(pFieldmark->GetMarkStart(), pFieldmark->GetMarkEnd());
    // Ignore the leading field start + sep.
    aPam.GetMark()->SetContent(aPam.GetMark()->GetContentIndex() + 2);
    // Ignore the trailing field end.
    aPam.GetPoint()->SetContent(aPam.GetPoint()->GetContentIndex() - 1);
    CPPUNIT_ASSERT(aPam.HasMark());
    OUString aActualResult = aPam.GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("aaa\nbbb"), aActualResult);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateFieldmarks)
{
    // Given a document with 2 fieldmarks:
    createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 1"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("old result 1"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 2"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("old result 2"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }

    // When updating those fieldmarks:
    uno::Sequence<css::beans::PropertyValue> aField1{
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM new command 1"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("new result 1"))),
    };
    uno::Sequence<css::beans::PropertyValue> aField2{
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM new command 2"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("new result 2"))),
    };
    uno::Sequence<uno::Sequence<css::beans::PropertyValue>> aFields = { aField1, aField2 };
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommandPrefix",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM"))),
        comphelper::makePropertyValue("Fields", uno::Any(aFields)),
    };
    dispatchCommand(mxComponent, ".uno:TextFormFields", aArgs);

    // Then make sure that the document text contains the new field results:
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->Start()->GetNode().GetTextNode()->GetText();
    static sal_Unicode const aForbidden[]
        = { CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDSEP, CH_TXT_ATR_FIELDEND, 0 };
    aActual = comphelper::string::removeAny(aActual, aForbidden);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new result 1new result 2
    // - Actual  : old result 1old result 2
    // i.e. the fieldmarks were not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("new result 1new result 2"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertBookmark)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a bookmark with text:
    OUString aExpectedBookmarkName("ZOTERO_BREF_GiQ7DAWQYWLy");
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("Bookmark", uno::Any(aExpectedBookmarkName)),
        comphelper::makePropertyValue("BookmarkText", uno::Any(OUString("<p>aaa</p><p>bbb</p>"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertBookmark", aArgs);

    // Then make sure that we create a bookmark that covers that text:
    IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), rIDMA.getBookmarksCount());
    for (auto it = rIDMA.getBookmarksBegin(); it != rIDMA.getBookmarksEnd(); ++it)
    {
        sw::mark::IMark* pMark = *it;
        CPPUNIT_ASSERT_EQUAL(aExpectedBookmarkName, pMark->GetName());
        SwPaM aPam(pMark->GetMarkStart(), pMark->GetMarkEnd());
        OUString aActualResult = aPam.GetText();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: aaa\nbbb
        // - Actual  :
        // i.e. no text was inserted, the bookmark was collapsed.
        CPPUNIT_ASSERT_EQUAL(OUString("aaa\nbbb"), aActualResult);
    }
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testGotoMark)
{
    // Given a document with 2 paragraphs, a bookmark on the second one:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SplitNode();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "mybookmark");
    SwNodeOffset nExpected = pWrtShell->GetCursor()->GetPointNode().GetIndex();

    // When jumping to that mark from the doc start:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("GotoMark", uno::Any(OUString("mybookmark"))),
    };
    dispatchCommand(mxComponent, ".uno:GotoMark", aArgs);

    // Then make sure that the final cursor position is at the bookmark:
    SwNodeOffset nActual = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10 (bookmark)
    // - Actual  : 9 (doc start)
    // i.e. the actual jump didn't happen.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateBookmarks)
{
    // Given a document with 2 bookmarks, first covering "B" and second covering "D":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("ABCDE");
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "ZOTERO_BREF_GiQ7DAWQYWLy");
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "ZOTERO_BREF_PRxDGUb4SWXF");

    // When updating the content of bookmarks:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "BookmarkNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BREF_"
    },
    "Bookmarks": {
        "type": "[][]com.sun.star.beans.PropertyValue",
        "value": [
            {
                "Bookmark": {
                    "type": "string",
                    "value": "ZOTERO_BREF_new1"
                },
                "BookmarkText": {
                    "type": "string",
                    "value": "new result 1"
                }
            },
            {
                "Bookmark": {
                    "type": "string",
                    "value": "ZOTERO_BREF_new2"
                },
                "BookmarkText": {
                    "type": "string",
                    "value": "new result 2"
                }
            }
        ]
    }
}
)json"_ostr);
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateBookmarks", aArgs);

    // Then make sure that the only paragraph is updated correctly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Anew result 1Cnew result 2E
    // - Actual  : ABCDE
    // i.e. the content was not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("Anew result 1Cnew result 2E"), aActual);

    // Without the accompanying fix in place, this test would have failed, ZOTERO_BREF_GiQ7DAWQYWLy
    // was not renamed to ZOTERO_BREF_new1.
    auto it = pDoc->getIDocumentMarkAccess()->findMark("ZOTERO_BREF_new1");
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertFieldmarkReadonly)
{
    // Given a document with a fieldmark, the cursor inside the fieldmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand", uno::Any(OUString("my command"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("my result"))),
    };
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    pCursor->SttEndDoc(/*bSttDoc=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // When trying to insert an inner fieldmark:
    // Without the accompanying fix in place, this test would have crashed.
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);

    // Then make sure the read-only content refuses to accept that inner fieldmark, so we still have
    // just one:
    size_t nActual = 0;
    IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
    for (auto it = rIDMA.getFieldmarksBegin(); it != rIDMA.getFieldmarksEnd(); ++it)
    {
        ++nActual;
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), nActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateRefmarks)
{
    // Given a document with two refmarks, one is not interesting the other is a citation:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue("Name", uno::Any(OUString("some other old refmark"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("some other old content"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SplitNode();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue(
            "Name", uno::Any(OUString("ZOTERO_ITEM CSL_CITATION {} old refmark"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("old content"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // When updating that refmark:
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "TypeName": {
        "type": "string",
        "value": "SetRef"
    },
    "NamePrefix": {
        "type": "string",
        "value": "ZOTERO_ITEM CSL_CITATION"
    },
    "Fields": {
        "type": "[][]com.sun.star.beans.PropertyValue",
        "value": [
            {
                "Name": {
                    "type": "string",
                    "value": "ZOTERO_ITEM CSL_CITATION {} new refmark"
                },
                "Content": {
                    "type": "string",
                    "value": "new content"
                }
            }
        ]
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateFields", aArgs);

    // Then make sure that the document text features the new content:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new content
    // - Actual  : old content
    // i.e. the doc content was not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("new content"), pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateFieldmark)
{
    // Given a document with a fieldmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM old command 1"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("old result 1"))),
    };
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);

    // When updating that fieldmark to have new field command & result:
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "FieldType": {
        "type": "string",
        "value": "vnd.oasis.opendocument.field.UNHANDLED"
    },
    "FieldCommandPrefix": {
        "type": "string",
        "value": "ADDIN ZOTERO_ITEM"
    },
    "Field": {
        "type": "[]com.sun.star.beans.PropertyValue",
        "value": {
            "FieldType": {
                "type": "string",
                "value": "vnd.oasis.opendocument.field.UNHANDLED"
            },
            "FieldCommand": {
                "type": "string",
                "value": "ADDIN ZOTERO_ITEM new command 1"
            },
            "FieldResult": {
                "type": "string",
                "value": "new result 1"
            }
        }
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateTextFormField", aArgs);

    // Then make sure that the document text is updated accordingly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->Start()->GetNode().GetTextNode()->GetText();
    static sal_Unicode const aForbidden[]
        = { CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDSEP, CH_TXT_ATR_FIELDEND, 0 };
    aActual = comphelper::string::removeAny(aActual, aForbidden);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new result 1
    // - Actual  : old result 1
    // i.e. the document text was not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("new result 1"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateSections)
{
    // Given a document with a section:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("RegionName",
                                      uno::Any(OUString("ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDold"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("old content"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertSection", aArgs);

    // When updating that section:
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "SectionNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BIBL"
    },
    "Sections": {
        "type": "[][]com.sun.star.beans.PropertyValue",
        "value": [
            {
                "RegionName": {
                    "type": "string",
                    "value": "ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDnew"
                },
                "Content": {
                    "type": "string",
                    "value": "new content"
                }
            }
        ]
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateSections", aArgs);

    // Then make sure that the section is updated:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->EndOfSection(/*bSelect=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActualResult = pCursor->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new content
    // - Actual  : old content
    // i.e. the content wasn't updated.
    CPPUNIT_ASSERT_EQUAL(OUString("new content"), aActualResult);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteFieldmarks)
{
    // Given a document with 2 fieldmarks:
    createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 1"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("result 1"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 2"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("result 2"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }

    // When deleting those fieldmarks:
    uno::Sequence<css::beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue("FieldCommandPrefix",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM"))) };
    dispatchCommand(mxComponent, ".uno:DeleteTextFormFields", aArgs);

    // Then make sure that the document doesn't contain fields anymore:
    SwDoc* pDoc = getSwDoc();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the fieldmarks were not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAllMarksCount());
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->Start()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("result 1result 2"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateBookmark)
{
    // Given a document with a bookmarks, covering "BC":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("ABCD");
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "ZOTERO_BREF_old");

    // When updating the content of the bookmark under the cursor:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "BookmarkNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BREF_"
    },
    "Bookmark": {
        "type": "[]com.sun.star.beans.PropertyValue",
        "value": {
            "Bookmark": {
                "type": "string",
                "value": "ZOTERO_BREF_new"
            },
            "BookmarkText": {
                "type": "string",
                "value": "new result"
            }
        }
    }
}
)json"_ostr);
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateBookmark", aArgs);

    // Then make sure that the only paragraph is updated correctly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Anew resultD
    // - Actual  : ABCD
    // i.e. it was not possible to update just the bookmark under cursor.
    CPPUNIT_ASSERT_EQUAL(OUString("Anew resultD"), aActual);
    auto it = pDoc->getIDocumentMarkAccess()->findMark("ZOTERO_BREF_new");
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateRefmark)
{
    // Given a document with a refmark:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue(
            "Name", uno::Any(OUString("ZOTERO_ITEM CSL_CITATION {} old refmark"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("old content"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // When updating that refmark:
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "TypeName": {
        "type": "string",
        "value": "SetRef"
    },
    "NamePrefix": {
        "type": "string",
        "value": "ZOTERO_ITEM CSL_CITATION"
    },
    "Field": {
        "type": "[]com.sun.star.beans.PropertyValue",
        "value": {
            "Name": {
                "type": "string",
                "value": "ZOTERO_ITEM CSL_CITATION {} new refmark"
            },
            "Content": {
                "type": "string",
                "value": "new content"
            }
        }
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateField", aArgs);

    // Then make sure that the document text features the new content:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new content
    // - Actual  : old content
    // i.e. the content was not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("new content"), pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteBookmarks)
{
    // Given a document with 2 bookmarks, first covering "B" and second covering "D":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("ABCDE");
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "ZOTERO_BREF_GiQ7DAWQYWLy");
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "other");

    // When deleting 1 matching bookmark:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "BookmarkNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BREF_"
    }
}
)json"_ostr);
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:DeleteBookmarks", aArgs);

    // Then make sure that only the other bookmark is kept:
    auto it = pDoc->getIDocumentMarkAccess()->findMark("ZOTERO_BREF_GiQ7DAWQYWLy");
    // Without the accompanying fix in place, this test would have failed, the matching bookmark was
    // not removed.
    CPPUNIT_ASSERT(bool(it == pDoc->getIDocumentMarkAccess()->getAllMarksEnd()));
    it = pDoc->getIDocumentMarkAccess()->findMark("other");
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteFields)
{
    // Given a document with a refmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue(
            "Name", uno::Any(OUString("ZOTERO_ITEM CSL_CITATION {} RNDpyJknp173F"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("aaa<b>bbb</b>ccc"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // When deleting the refmarks:
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "TypeName": {
        "type": "string",
        "value": "SetRef"
    },
    "NamePrefix": {
        "type": "string",
        "value": "ZOTERO_ITEM CSL_CITATION"
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:DeleteFields", aArgs);

    // Then make sure that no refmark is kept:
    SwDoc* pDoc = getSwDoc();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the refmark was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), pDoc->GetRefMarks());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertTextFormFieldFootnote)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting an ODF_UNHANDLED fieldmark inside a footnote:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_BIBL foo bar"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("result"))),
        comphelper::makePropertyValue("Wrapper", uno::Any(OUString("Footnote"))),
    };
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);

    // Then make sure that the footnote is created:
    SwFootnoteIdxs& rFootnotes = pDoc->GetFootnoteIdxs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no footnote was created.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFootnotes.size());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertTextFormFieldEndnote)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting an ODF_UNHANDLED fieldmark inside an endnote:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_BIBL foo bar"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("result"))),
        comphelper::makePropertyValue("Wrapper", uno::Any(OUString("Endnote"))),
    };
    dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);

    // Then make sure that the endnote is created:
    SwFootnoteIdxs& rFootnotes = pDoc->GetFootnoteIdxs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no endnote was inserted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFootnotes.size());
    SwTextFootnote* pEndnote = rFootnotes[0];
    const SwFormatFootnote& rFormatEndnote = pEndnote->GetFootnote();
    CPPUNIT_ASSERT(rFormatEndnote.IsEndNote());
    // Also check that the endnote body contains the fieldmark:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->GotoFootnoteText();
    pWrtShell->EndOfSection(/*bSelect=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetText();
    static sal_Unicode const aForbidden[]
        = { CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDSEP, CH_TXT_ATR_FIELDEND, 0 };
    aActual = comphelper::string::removeAny(aActual, aForbidden);
    // Then this was empty: the fieldmark was inserted before the note anchor, not in the note body.
    CPPUNIT_ASSERT_EQUAL(OUString("result"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateSelectedField)
{
    // Given an empty doc:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();

    // Insert a time field and select it:
    dispatchCommand(mxComponent, ".uno:InsertTimeFieldVar", {});

    pCursor->SetMark();
    pCursor->Move(fnMoveBackward);

    OUString aTimeFieldBefore, aTimeFieldAfter;
    pWrtShell->GetSelectedText(aTimeFieldBefore);

    // Wait for one second:
    osl::Thread::wait(std::chrono::seconds(1));

    // Update the field at cursor:
    dispatchCommand(mxComponent, ".uno:UpdateSelectedField", {});
    pWrtShell->GetSelectedText(aTimeFieldAfter);

    // Check that the selected field has changed:
    CPPUNIT_ASSERT(aTimeFieldAfter != aTimeFieldBefore);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
