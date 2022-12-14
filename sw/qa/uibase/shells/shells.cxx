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
#include <vcl/GraphicObject.hxx>
#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <xmloff/odffields.hxx>
#include <comphelper/string.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <docsh.hxx>
#include <bookmark.hxx>
#include <ndtxt.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/uibase/shells/data/";

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testTdf130179)
{
    SwDoc* pDoc = createSwDoc();
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursorShell* pShell(pDoc->GetEditShell());
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet aGrfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
    aFrameSet.Put(aAnchor);
    GraphicObject aGrf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), aGrf, &aFrameSet, &aGrfSet));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame()->GetBindings().QueryState(FN_POSTIT, pItem);
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
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aStartPos(1000, 1000);
    pWrtShell->BeginCreate(OBJ_RECT, aStartPos);
    Point aMovePos(2000, 2000);
    pWrtShell->MoveCreate(aMovePos);
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Start shape text edit.
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Select the shape.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    pView->StopShellTimer();
    // Start the actual text edit.
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    pView->EnterShapeDrawTextMode(pObject);
    pView->AttrChangedNotify(nullptr);

    // Change paragraph adjustment to center.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_CENTER,
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
    load(DATA_DIRECTORY, "ole-save-preview-update.odt");

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

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyUrlContextMenu)
{
    // Given a document with a bibliography field:
    SwDoc* pDoc = createSwDoc();
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
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxDispatcher* pDispatcher = pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher();
    css::uno::Any aState;
    SfxItemState eState = pDispatcher->QueryState(SID_OPEN_HYPERLINK, aState);

    // Then the "open hyperlink" menu item should be visible:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32 (SfxItemState::DEFAULT)
    // - Actual  : 1 (SfxItemState::DISABLED)
    // i.e. the menu item was not visible for biblio entry fields with an URL.
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, eState);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyLocalCopyContextMenu)
{
    // Given a document with a bibliography field's local copy:
    SwDoc* pDoc = createSwDoc();
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
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxDispatcher* pDispatcher = pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher();
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
    SwDoc* pDoc = createSwDoc();
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
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);

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
    SwDoc* pDoc = createSwDoc();

    // When inserting an ODF_UNHANDLED fieldmark:
    OUString aExpectedCommand("ADDIN ZOTERO_BIBL foo bar");
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
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
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_UNHANDLED), pFieldmark->GetFieldname());

    auto it = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
    CPPUNIT_ASSERT(it != pFieldmark->GetParameters()->end());
    OUString aActualCommand;
    it->second >>= aActualCommand;
    CPPUNIT_ASSERT_EQUAL(aExpectedCommand, aActualCommand);

    SwPaM aPam(pFieldmark->GetMarkStart(), pFieldmark->GetMarkEnd());
    // Ignore the leading field start + sep.
    aPam.GetMark()->nContent = aPam.GetMark()->nContent.GetIndex() + 2;
    // Ignore the trailing field end.
    aPam.GetPoint()->nContent = aPam.GetPoint()->nContent.GetIndex() - 1;
    CPPUNIT_ASSERT(aPam.HasMark());
    OUString aActualResult = aPam.GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("aaa\nbbb"), aActualResult);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateFieldmarks)
{
    // Given a document with 2 fieldmarks:
    SwDoc* pDoc = createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 1"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("old result 1"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
            comphelper::makePropertyValue("FieldCommand",
                                          uno::Any(OUString("ADDIN ZOTERO_ITEM old command 2"))),
            comphelper::makePropertyValue("FieldResult", uno::Any(OUString("old result 2"))),
        };
        dispatchCommand(mxComponent, ".uno:TextFormField", aArgs);
    }

    // When updating those fieldmarks:
    uno::Sequence<css::beans::PropertyValue> aField1{
        comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM new command 1"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("new result 1"))),
    };
    uno::Sequence<css::beans::PropertyValue> aField2{
        comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
        comphelper::makePropertyValue("FieldCommand",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM new command 2"))),
        comphelper::makePropertyValue("FieldResult", uno::Any(OUString("new result 2"))),
    };
    uno::Sequence<uno::Sequence<css::beans::PropertyValue>> aFields = { aField1, aField2 };
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FieldType", uno::Any(OUString(ODF_UNHANDLED))),
        comphelper::makePropertyValue("FieldCommandPrefix",
                                      uno::Any(OUString("ADDIN ZOTERO_ITEM"))),
        comphelper::makePropertyValue("Fields", uno::Any(aFields)),
    };
    dispatchCommand(mxComponent, ".uno:TextFormFields", aArgs);

    // Then make sure that the document text contains the new field results:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->Start()->nNode.GetNode().GetTextNode()->GetText();
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
    SwDoc* pDoc = createSwDoc();

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

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateBookmarks)
{
    // Given a document with 2 bookmarks, first covering "B" and second covering "D":
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("ABCDE");
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), "ZOTERO_BREF_GiQ7DAWQYWLy");
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
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
                    "value": "ZOTERO_BREF_GiQ7DAWQYWLy"
                },
                "BookmarkText": {
                    "type": "string",
                    "value": "new result 1"
                }
            },
            {
                "Bookmark": {
                    "type": "string",
                    "value": "ZOTERO_BREF_PRxDGUb4SWXF"
                },
                "BookmarkText": {
                    "type": "string",
                    "value": "new result 2"
                }
            }
        ]
    }
}
)json");
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:UpdateBookmarks", aArgs);

    // Then make sure that the only paragraph is updated correctly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Anew result 1Cnew result 2E
    // - Actual  : ABCDE
    // i.e. the content was not updated.
    CPPUNIT_ASSERT_EQUAL(OUString("Anew result 1Cnew result 2E"), aActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
