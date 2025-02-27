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
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>

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
#include <unotxdoc.hxx>
#include <tools/json_writer.hxx>

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
public:
    SwUibaseShellsTest()
        : SwModelTestBase(u"/sw/qa/uibase/shells/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testTdf130179)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet aGrfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
    aFrameSet.Put(aAnchor);
    Graphic aGrf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pShell->GetCursor(), OUString(), OUString(), &aGrf,
                                       &aFrameSet, &aGrfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    SwView* pView = getSwDocShell()->GetView();
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    Point aStartPos(1000, 1000);
    pWrtShell->BeginCreate(SdrObjKind::Rectangle, aStartPos);
    Point aMovePos(2000, 2000);
    pWrtShell->MoveCreate(aMovePos);
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Start shape text edit.
    SwView* pView = getSwDocShell()->GetView();
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
    dispatchCommand(mxComponent, u".uno:UpdateAll"_ustr, {});

    // Save the document and see if we get the previews.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // Without the accompanying fix in place, this test would have failed, because the object
    // replacements were not generated, even after UpdateAll.
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"ObjectReplacements/Object 1"_ustr));
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"ObjectReplacements/Object 2"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testOlePreviewUpdate)
{
    // Given a document with an embedded Writer object:
    createSwDoc("ole-preview-update.odt");

    // When updating "all" (including OLE previews):
    dispatchCommand(mxComponent, u".uno:UpdateAll"_ustr, {});

    // Then make sure the preview is no longer a 0-sized stream:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(
        xNameAccess->getByName(u"ObjectReplacements/Object 1"_ustr), uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    // Without the accompanying fix in place, this test would have failed, the stream was still
    // empty.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_uInt64>(0), pStream->remainingSize());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyUrlContextMenu)
{
    // Given a document with a bibliography field:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=1"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = getSwDocShell();
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
    dispatchCommand(mxComponent, u".uno:CopyHyperlinkLocation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    // Assert generic hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(u"http://reset.url/1"_ustr, getParagraph(2)->getString());

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoLeft"_ustr, {});
    // Copy bibliography mark hyperlink
    dispatchCommand(mxComponent, u".uno:CopyHyperlinkLocation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    // Assert bibliography mark hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(u"https://test.url/1"_ustr, getParagraph(4)->getString());

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoLeft"_ustr, {});
    // Copy generic hyperlink
    dispatchCommand(mxComponent, u".uno:CopyHyperlinkLocation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    // Assert generic hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(u"http://reset.url/2"_ustr, getParagraph(6)->getString());

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoLeft"_ustr, {});
    // Copy bibliography table hyperlink
    dispatchCommand(mxComponent, u".uno:CopyHyperlinkLocation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    // Assert bibliography table entry hyperlink was correctly copied and pasted
    CPPUNIT_ASSERT_EQUAL(u"https://test.url/1"_ustr, getParagraph(9)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testBibliographyLocalCopyContextMenu)
{
    // Given a document with a bibliography field's local copy:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=1"_ustr),
        comphelper::makePropertyValue(u"LocalURL"_ustr, u"file:///home/me/test.pdf"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When selecting the field and opening the context menu:
    SwDocShell* pDocShell = getSwDocShell();
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
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // When trying to insert a page break:
    dispatchCommand(mxComponent, u".uno:InsertPagebreak"_ustr, {});

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
    OUString aExpectedCommand(u"ADDIN ZOTERO_BIBL foo bar"_ustr);
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr, uno::Any(aExpectedCommand)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

    // Then make sure that it's type/name is correct:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    pCursor->SttEndDoc(/*bSttDoc=*/true);
    sw::mark::Fieldmark* pFieldmark
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
    CPPUNIT_ASSERT_EQUAL(u"aaa\nbbb"_ustr, aActualResult);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateFieldmarks)
{
    // Given a document with 2 fieldmarks:
    createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM old command 1"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"old result 1"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM old command 2"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"old result 2"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }

    // When updating those fieldmarks:
    uno::Sequence<css::beans::PropertyValue> aField1{
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_ITEM new command 1"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"new result 1"_ustr)),
    };
    uno::Sequence<css::beans::PropertyValue> aField2{
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_ITEM new command 2"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"new result 2"_ustr)),
    };
    uno::Sequence<uno::Sequence<css::beans::PropertyValue>> aFields = { aField1, aField2 };
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommandPrefix"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_ITEM"_ustr)),
        comphelper::makePropertyValue(u"Fields"_ustr, uno::Any(aFields)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormFields"_ustr, aArgs);

    // Then make sure that the document text contains the new field results:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    CPPUNIT_ASSERT_EQUAL(u"new result 1new result 2"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertBookmark)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a bookmark with text:
    OUString aExpectedBookmarkName(u"ZOTERO_BREF_GiQ7DAWQYWLy"_ustr);
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Bookmark"_ustr, uno::Any(aExpectedBookmarkName)),
        comphelper::makePropertyValue(u"BookmarkText"_ustr, uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertBookmark"_ustr, aArgs);

    // Then make sure that we create a bookmark that covers that text:
    IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), rIDMA.getBookmarksCount());
    for (auto it = rIDMA.getBookmarksBegin(); it != rIDMA.getBookmarksEnd(); ++it)
    {
        sw::mark::MarkBase* pMark = *it;
        CPPUNIT_ASSERT_EQUAL(aExpectedBookmarkName, pMark->GetName());
        SwPaM aPam(pMark->GetMarkStart(), pMark->GetMarkEnd());
        OUString aActualResult = aPam.GetText();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: aaa\nbbb
        // - Actual  :
        // i.e. no text was inserted, the bookmark was collapsed.
        CPPUNIT_ASSERT_EQUAL(u"aaa\nbbb"_ustr, aActualResult);
    }
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testGotoMark)
{
    // Given a document with 2 paragraphs, a bookmark on the second one:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SplitNode();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"mybookmark"_ustr);
    SwNodeOffset nExpected = pWrtShell->GetCursor()->GetPointNode().GetIndex();

    // When jumping to that mark from the doc start:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"GotoMark"_ustr, uno::Any(u"mybookmark"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:GotoMark"_ustr, aArgs);

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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"ABCDE"_ustr);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"ZOTERO_BREF_GiQ7DAWQYWLy"_ustr);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"ZOTERO_BREF_PRxDGUb4SWXF"_ustr);

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
)json");
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateBookmarks"_ustr, aArgs);

    // Then make sure that the only paragraph is updated correctly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Anew result 1Cnew result 2E
    // - Actual  : ABCDE
    // i.e. the content was not updated.
    CPPUNIT_ASSERT_EQUAL(u"Anew result 1Cnew result 2E"_ustr, aActual);

    // Without the accompanying fix in place, this test would have failed, ZOTERO_BREF_GiQ7DAWQYWLy
    // was not renamed to ZOTERO_BREF_new1.
    auto it = pDoc->getIDocumentMarkAccess()->findMark(u"ZOTERO_BREF_new1"_ustr);
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testInsertFieldmarkReadonly)
{
    // Given a document with a fieldmark, the cursor inside the fieldmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr, uno::Any(u"my command"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"my result"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    pCursor->SttEndDoc(/*bSttDoc=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // When trying to insert an inner fieldmark:
    // Without the accompanying fix in place, this test would have crashed.
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

    // Then make sure the read-only content refuses to accept that inner fieldmark, so we still have
    // just one:
    IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), rIDMA.getFieldmarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDocumentStructureTransformChart)
{
    createSwDoc("docStructureChartExampleOriginal.odt");
    OString aJson = R"json(
{
    "Transforms": [
        {"Charts.ByEmbedIndex.0": [
            {"modifyrow.1": [ 19, 15 ]},
            {"datayx.3.1": 37},
            {"deleterow.0": ""},
            {"deleterow.1": ""},
            {"insertrow.0": [ 15, 17 ]},
            {"setrowdesc.0": "Paul"},
            {"insertrow.2": [ 19, 22 ]},
            {"setrowdesc.2": "Barbara"},
            {"insertrow.4": [ 29, 27 ]},
            {"setrowdesc.4": "Elizabeth"},
            {"insertrow.5": [ 14, 26 ]},
            {"setrowdesc.5": "William"},
            {"insertcolumn.1": [ 1,2,3,4,5,6 ]},
            {"insertcolumn.0": [ 2,1,2,1,2,1 ]},
            {"insertcolumn.4": [ 7,7,7,7,7,7 ]},
            {"setcolumndesc.4": "c4"},
            {"setcolumndesc.0": "c0"},
            {"setcolumndesc.2": "c2"}
        ]},
        {"Charts.BySubTitle.Subtitle2": {
            "deletecolumn.3": ""
        }},
        {"Charts.ByEmbedName.Object3": [
            {"resize": [ 12, 3 ]},
            {"setrowdesc": [
                "United Kingdom",
                "United States of America",
                "Canada",
                "Brazil",
                "Germany",
                "India",
                "Japan",
                "Sudan",
                "Norway",
                "Italy",
                "France",
                "Egypt"
            ]},
            {"modifycolumn.0": [ 12, 9, 8, 5, 5, 4, 3, 3, 2, 2, 1, 1]},
            {"resize": [ 12, 2 ]}
        ]},
        {"Charts.ByTitle.Fixed issues": {
            "data": [ [ 3,1,2 ],
                      [ 2,0,1 ],
                      [ 3,2,0 ],
                      [ 2,1,1 ],
                      [ 4,2,2 ],
                      [ 2,2,1 ],
                      [ 3,2,0,0 ],
                      [ 3,1,2,1 ],
                      [ 3,3,1,0,1 ],
                      [ 2,1,0,1,2 ],
                      [ 4,2,1,2,2,3 ],
                      [ 2,1,0,0,1,4 ],
                      [ 3,2,2,1,3,2 ],
                      [ 4,2,1,1,2,3 ],
                      [ 3,3,2,0,3,5 ],
                      [ 3,3,1,2,2,3 ],
                      [ 5,1,1,1,1,4 ],
                      [ 2,2,2,1,2,3 ] ],
            "setrowdesc": ["2023.01",".02",".03",".04",".05",".06",".07",".08",".09",".10",".11",".12","2023.01",".02",".03",".04",".05",".06"],
            "setcolumndesc": ["Jennifer", "Charles", "Thomas", "Maria", "Lisa", "Daniel"]
        }}
    ]
}
)json"_ostr;

    //transform
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"DataJson"_ustr,
                                      uno::Any(OStringToOUString(aJson, RTL_TEXTENCODING_UTF8))),
    };
    dispatchCommand(mxComponent, u".uno:TransformDocumentStructure"_ustr, aArgs);

    // Check transformed values of the 3 chart
    for (int nShape = 0; nShape < 3; nShape++)
    {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xEOS(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xEOS.is());
        uno::Reference<container::XIndexAccess> xEmbeddeds(xEOS->getEmbeddedObjects(),
                                                           uno::UNO_QUERY);
        CPPUNIT_ASSERT(xEmbeddeds.is());

        uno::Reference<beans::XPropertySet> xShapeProps(xEmbeddeds->getByIndex(nShape),
                                                        uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapeProps.is());

        uno::Reference<frame::XModel> xDocModel;
        xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
        CPPUNIT_ASSERT(xDocModel.is());

        uno::Reference<chart2::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDoc.is());

        uno::Reference<chart::XChartDataArray> xDataArray(xChartDoc->getDataProvider(),
                                                          uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDataArray.is());

        switch (nShape)
        {
            case 0:
            {
                std::vector<std::vector<double>> aValues = {
                    { 2, 15, 1, 7 }, { 1, 19, 2, 7 }, { 2, 19, 3, 7 },
                    { 1, 25, 4, 7 }, { 2, 29, 5, 7 }, { 1, 14, 6, 7 },
                };
                // RowDescriptions
                std::vector<OUString> aRowDescsValues
                    = { u"Paul"_ustr,  u"Mary"_ustr,      u"Barbara"_ustr,
                        u"David"_ustr, u"Elizabeth"_ustr, u"William"_ustr };
                // ColumnDescriptions
                std::vector<OUString> aColDescsValues
                    = { u"c0"_ustr, u"2022"_ustr, u"c2"_ustr, u"c4"_ustr };

                uno::Sequence<uno::Sequence<double>> aData = xDataArray->getData();
                for (size_t nY = 0; nY < aValues.size(); nY++)
                {
                    for (size_t nX = 0; nX < aValues[nY].size(); nX++)
                    {
                        CPPUNIT_ASSERT_EQUAL(aData[nY][nX], aValues[nY][nX]);
                    }
                }

                uno::Sequence<OUString> aColDescs = xDataArray->getColumnDescriptions();
                for (size_t i = 0; i < aColDescsValues.size(); i++)
                {
                    CPPUNIT_ASSERT_EQUAL(aColDescs[i], aColDescsValues[i]);
                }
                uno::Sequence<OUString> aRowDescs = xDataArray->getRowDescriptions();
                for (size_t i = 0; i < aRowDescsValues.size(); i++)
                {
                    CPPUNIT_ASSERT_EQUAL(aRowDescs[i], aRowDescsValues[i]);
                }
            }
            break;
            case 1:
            {
                uno::Sequence<uno::Sequence<double>> aData = xDataArray->getData();
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(18), aData.getLength());
                uno::Sequence<double>* pRows = aData.getArray();
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), pRows[0].getLength());
            }
            break;
            case 2:
            {
                uno::Sequence<uno::Sequence<double>> aData = xDataArray->getData();
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12), aData.getLength());
                uno::Sequence<double>* pRows = aData.getArray();
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), pRows[0].getLength());
            }
            break;
            default:
                break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDocumentStructureExtractChart)
{
    createSwDoc("docStructureChartExampleOriginal.odt");

    //extract
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:ExtractDocumentStructure?filter=charts");
    getSwTextDoc()->getCommandValues(aJsonWriter, aCommand);

    OString aExpectedStr
        = "{ \"DocStructure\": { \"Charts.ByEmbedIndex.0\": { \"name\": \"Object1\", \"title\": "
          "\"Paid leave days\", \"subtitle\": \"Subtitle2\", \"RowDescriptions\": [ \"James\", "
          "\"Mary\", \"Patricia\", \"David\"], \"ColumnDescriptions\": [ \"2022\", \"2023\"], "
          "\"DataValues\": [ [ \"22\", \"24\"], [ \"18\", \"16\"], [ \"32\", \"32\"], "
          "[ \"25\", \"23\"]]}, "
          "\"Charts.ByEmbedIndex.1\": { \"name\": \"Object2\", \"title\": \"Fixed issues\", "
          "\"subtitle\": \"Subtitle1\", \"RowDescriptions\": [ \"\"], \"ColumnDescriptions\": [ \" "
          "\"], \"DataValues\": [ [ \"NaN\"]]}, \"Charts.ByEmbedIndex.2\": { \"name\": "
          "\"Object3\", \"title\": \"Employees from countries\", \"subtitle\": \"Subtitle3\", "
          "\"RowDescriptions\": [ \"\"], \"ColumnDescriptions\": [ \"Column 1\"], \"DataValues\": "
          "[ [ \"NaN\"]]}}}"_ostr;

    CPPUNIT_ASSERT_EQUAL(aExpectedStr, aJsonWriter.finishAndGetAsOString());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDocumentStructureDocProperties)
{
    createSwDoc("docStructureChartExampleOriginal.odt");
    OString aJson = R"json(
{
    "Transforms": [
        { "DocumentProperties": {
            "Author":"Author TxT",
            "Generator":"Generator TxT",
            "CreationDate":"2024-01-21T14:45:00",
            "Title":"Title TxT",
            "Subject":"Subject TxT",
            "Description":"Description TxT",
            "Keywords": [ ],
            "Language":"en-GB",
            "ModifiedBy":"ModifiedBy TxT",
            "ModificationDate":"2024-05-23T10:05:50.159530766",
            "PrintedBy":"PrintedBy TxT",
            "PrintDate":"0000-00-00T00:00:00",
            "TemplateName":"TemplateName TxT",
            "TemplateURL":"TemplateURL TxT",
            "TemplateDate":"0000-00-00T00:00:00",
            "AutoloadURL":"",
            "AutoloadSecs": 0,
            "DefaultTarget":"DefaultTarget TxT",
            "DocumentStatistics": {
                "PageCount": 300,
                "TableCount": 60,
                "ImageCount": 10,
                "ObjectCount": 0,
                "ParagraphCount": 2880,
                "WordCount": 78680,
                "CharacterCount": 485920,
                "NonWhitespaceCharacterCount": 411520
            },
            "EditingCycles":12,
            "EditingDuration":12345,
            "Contributor":["Contributor1 TxT","Contributor2 TXT"],
            "Coverage":"Coverage TxT",
            "Identifier":"Identifier TxT",
            "Publisher":["Publisher TxT","Publisher2 TXT"],
            "Relation":["Relation TxT","Relation2 TXT"],
            "Rights":"Rights TxT",
            "Source":"Source TxT",
            "Type":"Type TxT",
            "UserDefinedProperties": [
                {"Add.NewPropName Str": {
                    "type": "string",
                    "value": "this is a string"
                }},
                {"Add.NewPropName Str": {
                    "type": "boolean",
                    "value": false
                }},
                {"Add.NewPropName Bool": {
                    "type": "boolean",
                    "value": true
                }},
                {"Add.NewPropName Numb": {
                    "type": "long",
                    "value": 1245
                }},
                {"Add.NewPropName float": {
                    "type": "float",
                    "value": 12.45
                }},
                {"Add.NewPropName Double": {
                    "type": "double",
                    "value": 124.578
                }},
                {"Delete": "NewPropName Double"}
            ]
        } }
    ]
}
)json"_ostr;

    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"DataJson"_ustr,
                                      uno::Any(OStringToOUString(aJson, RTL_TEXTENCODING_UTF8))),
    };
    dispatchCommand(mxComponent, u".uno:TransformDocumentStructure"_ustr, aArgs);

    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:ExtractDocumentStructure?filter=docprops");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    OString aExpectedStr
        = "{ \"DocStructure\": { \"DocumentProperties\": { \"Author\": \"Author TxT\", "
          "\"Generator\": \"Generator TxT\", \"CreationDate\": \"2024-01-21T14:45:00\", "
          "\"Title\": \"Title TxT\", \"Subject\": \"Subject TxT\", \"Description\": "
          "\"Description TxT\", \"Keywords\": [ ], \"Language\": \"en-GB\", "
          "\"ModifiedBy\": \"ModifiedBy TxT\", \"ModificationDate\": "
          "\"2024-05-23T10:05:50.159530766\", \"PrintedBy\": \"PrintedBy TxT\", \"PrintDate\": "
          "\"0000-00-00T00:00:00\", \"TemplateName\": \"TemplateName TxT\", \"TemplateURL\": "
          "\"TemplateURL TxT\", \"TemplateDate\": \"0000-00-00T00:00:00\", \"AutoloadURL\": \"\", "
          "\"AutoloadSecs\": 0, \"DefaultTarget\": \"DefaultTarget TxT\", \"DocumentStatistics\": "
          "{ \"PageCount\": 300, \"TableCount\": 60, \"ImageCount\": 10, \"ObjectCount\": 0, "
          "\"ParagraphCount\": 2880, \"WordCount\": 78680, \"CharacterCount\": 485920, "
          "\"NonWhitespaceCharacterCount\": 411520}, \"EditingCycles\": 12, \"EditingDuration\": "
          "12345, \"Contributor\": [ \"Contributor1 TxT\", \"Contributor2 TXT\"], \"Coverage\": "
          "\"Coverage TxT\", \"Identifier\": \"Identifier TxT\", \"Publisher\": [ "
          "\"Publisher TxT\", \"Publisher2 TXT\"], \"Relation\": [ \"Relation TxT\", "
          "\"Relation2 TXT\"], \"Rights\": \"Rights TxT\", \"Source\": \"Source TxT\", \"Type\": "
          "\"Type TxT\", \"UserDefinedProperties\": { \"NewPropName Bool\": { \"type\": "
          "\"boolean\", \"value\": true}, \"NewPropName Numb\": { \"type\": \"long\", "
          "\"value\": 1245}, \"NewPropName Str\": { \"type\": \"boolean\", \"value\": false}, "
          "\"NewPropName float\": { \"type\": \"float\", \"value\": 12.45}}}}}"_ostr;

    CPPUNIT_ASSERT_EQUAL(aExpectedStr, aJsonWriter.finishAndGetAsOString());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateRefmarks)
{
    // Given a document with two refmarks, one is not interesting the other is a citation:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr, uno::Any(u"some other old refmark"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"some other old content"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SplitNode();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr,
                                      uno::Any(u"ZOTERO_ITEM CSL_CITATION {} old refmark"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"old content"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

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
)json");
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateFields"_ustr, aArgs);

    // Then make sure that the document text features the new content:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new content
    // - Actual  : old content
    // i.e. the doc content was not updated.
    CPPUNIT_ASSERT_EQUAL(u"new content"_ustr, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateFieldmark)
{
    // Given a document with a fieldmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_ITEM old command 1"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"old result 1"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

    // When updating that fieldmark to have new field command & result:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
)json");
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateTextFormField"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"new result 1"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateSections)
{
    // Given a document with a section:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"RegionName"_ustr,
                                      uno::Any(u"ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDold"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"old content"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertSection"_ustr, aArgs);

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
)json");
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateSections"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"new content"_ustr, aActualResult);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteFieldmarks)
{
    // Given a document with 2 fieldmarks:
    createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM old command 1"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result 1"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM old command 2"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result 2"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }

    // When deleting those fieldmarks:
    uno::Sequence<css::beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommandPrefix"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM"_ustr)) };
    dispatchCommand(mxComponent, u".uno:DeleteTextFormFields"_ustr, aArgs);

    // Then make sure that the document doesn't contain fields anymore:
    SwDoc* pDoc = getSwDoc();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the fieldmarks were not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAllMarksCount());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->Start()->GetNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(u"result 1result 2"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateBookmark)
{
    // Given a document with a bookmarks, covering "BC":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"ABCD"_ustr);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"ZOTERO_BREF_old"_ustr);

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
)json");
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateBookmark"_ustr, aArgs);

    // Then make sure that the only paragraph is updated correctly:
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActual = pCursor->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Anew resultD
    // - Actual  : ABCD
    // i.e. it was not possible to update just the bookmark under cursor.
    CPPUNIT_ASSERT_EQUAL(u"Anew resultD"_ustr, aActual);
    auto it = pDoc->getIDocumentMarkAccess()->findMark(u"ZOTERO_BREF_new"_ustr);
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateRefmark)
{
    // Given a document with a refmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr,
                                      uno::Any(u"ZOTERO_ITEM CSL_CITATION {} old refmark"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"old content"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

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
)json");
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:UpdateField"_ustr, aArgs);

    // Then make sure that the document text features the new content:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new content
    // - Actual  : old content
    // i.e. the content was not updated.
    CPPUNIT_ASSERT_EQUAL(u"new content"_ustr, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteBookmarks)
{
    // Given a document with 2 bookmarks, first covering "B" and second covering "D":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"ABCDE"_ustr);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"ZOTERO_BREF_GiQ7DAWQYWLy"_ustr);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->SetBookmark(vcl::KeyCode(), u"other"_ustr);

    // When deleting 1 matching bookmark:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "BookmarkNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BREF_"
    }
}
)json");
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:DeleteBookmarks"_ustr, aArgs);

    // Then make sure that only the other bookmark is kept:
    auto it = pDoc->getIDocumentMarkAccess()->findMark(u"ZOTERO_BREF_GiQ7DAWQYWLy"_ustr);
    // Without the accompanying fix in place, this test would have failed, the matching bookmark was
    // not removed.
    CPPUNIT_ASSERT(bool(it == pDoc->getIDocumentMarkAccess()->getAllMarksEnd()));
    it = pDoc->getIDocumentMarkAccess()->findMark(u"other"_ustr);
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAllMarksEnd());
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testDeleteFields)
{
    // Given a document with a refmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr,
                                      uno::Any(u"ZOTERO_ITEM CSL_CITATION {} RNDpyJknp173F"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"aaa<b>bbb</b>ccc"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

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
)json");
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:DeleteFields"_ustr, aArgs);

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
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_BIBL foo bar"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result"_ustr)),
        comphelper::makePropertyValue(u"Wrapper"_ustr, uno::Any(u"Footnote"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

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
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_BIBL foo bar"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result"_ustr)),
        comphelper::makePropertyValue(u"Wrapper"_ustr, uno::Any(u"Endnote"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"result"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testUpdateSelectedField)
{
    // Given an empty doc:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SwPaM* pCursor = pShell->GetCursor();

    // Insert a time field and select it:
    dispatchCommand(mxComponent, u".uno:InsertTimeFieldVar"_ustr, {});

    pCursor->SetMark();
    pCursor->Move(fnMoveBackward);

    OUString aTimeFieldBefore, aTimeFieldAfter;
    pWrtShell->GetSelectedText(aTimeFieldBefore);

    // Wait for one second:
    osl::Thread::wait(std::chrono::seconds(1));

    // Update the field at cursor:
    dispatchCommand(mxComponent, u".uno:UpdateSelectedField"_ustr, {});
    pWrtShell->GetSelectedText(aTimeFieldAfter);

    // Check that the selected field has changed:
    CPPUNIT_ASSERT(aTimeFieldAfter != aTimeFieldBefore);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
