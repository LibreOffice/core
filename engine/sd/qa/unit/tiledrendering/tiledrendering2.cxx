/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_pdfimport.h>
#include <unotools/tempfile.hxx>
#include <sdtiledrenderingtest.hxx>

#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svx/svxids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <vcl/scheduler.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/section.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/kernitem.hxx>
#include <osl/thread.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/cryptosign.hxx>
#include <svl/style.hxx>
#include <svl/undo.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/ipclient.hxx>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <svx/svdotext.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/json_writer.hxx>

#include <vcl/keycodes.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <tools/UnitConversion.hxx>

#include <set>

#include <DrawDocShell.hxx>
#include <SlideSectionManager.hxx>
#include <View.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <SlideLink.hxx>
#include <SlideLinkManager.hxx>
#include <unomodel.hxx>

using namespace css;
using namespace ::cpo;

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSidebarSwitchDeck)
{
    // Given an impress document, with a visible sidebar (ModifyPage deck):
    createDoc("dummy.odp");
    SdTestViewCallback aView;
    sfx2::sidebar::Sidebar::Setup(u"");
    Scheduler::ProcessEventsToIdle();
    aView.m_aStateChanges.clear();

    // When switching to the MasterSlidesPanel deck:
    dispatchCommand(mxComponent, u".uno:MasterSlidesPanel"_ustr, {});

    // Then make sure notifications are sent for both the old and the new decks:
    auto it = aView.m_aStateChanges.find(".uno:ModifyPage");
    // Without the accompanying fix in place, this test would have failed, the notification for the
    // old deck was missing.
    CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
    boost::property_tree::ptree aTree = it->second;
    CPPUNIT_ASSERT(aTree.get_child_optional("state").has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("false"), aTree.get_child("state").get_value<std::string>());
    it = aView.m_aStateChanges.find(".uno:MasterSlidesPanel");
    CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSidebarLayoutsKeptOnViewSwitch)
{
    // Given an Impress document with the Layouts sidebar panel (ModifyPage deck) open in the first
    // of two views:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    int nView1 = KitHelper::getCurrentView();
    SfxViewShell* pViewShell1 = SfxViewShell::Current();
    SdTestViewCallback aView1;
    sfx2::sidebar::Sidebar::Setup(u"");
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_aJSDialogSelectCounts["layoutpanel_icons"] > 0);
    int nView2 = KitHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    SdTestViewCallback aView2;
    Scheduler::ProcessEventsToIdle();
    aView1.m_aJSDialogSelectCounts.clear();

    // When the two views take turns, as they do while two users work at the same time. In the
    // running program each switch marks every state of the first view as dirty, so the first view
    // reads the unchanged vertical text state again. The test does that by hand:
    SfxBindings& rBindings1 = pViewShell1->GetViewFrame().GetBindings();
    for (int i = 0; i < 3; ++i)
    {
        KitHelper::setView(nView1);
        KitHelper::setView(nView2);
        rBindings1.Invalidate(SID_VERTICALTEXT_STATE);
        rBindings1.Update(SID_VERTICALTEXT_STATE);
        Scheduler::ProcessEventsToIdle();
    }

    // Then the panel of the first view keeps its content:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 6
    // i.e. the panel was filled again and sent its selection on each switch, which the client
    // shows as a flicker.
    CPPUNIT_ASSERT_EQUAL(0, aView1.m_aJSDialogSelectCounts["layoutpanel_icons"]);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSidebarLayoutsContextMenuUsesClickedLayout)
{
    // The context menu of the Layouts panel acts on the layout that was right-clicked. The first
    // slide has the Title Slide layout, at position 1 of the panel.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    SdTestViewCallback aView;
    sfx2::sidebar::Sidebar::Setup(u"");
    Scheduler::ProcessEventsToIdle();
    const OUString sSidebarId
        = OUString::number(reinterpret_cast<sal_uInt64>(SfxViewShell::Current())) + "sidebar";

    // Right-click the Title, Content layout at position 2 the way the client does: a select, then
    // a context menu request with the row and the pointer position as URI-encoded JSON.
    StringMap aSelect{ { u"type"_ustr, u"iconview"_ustr },
                       { u"cmd"_ustr, u"select"_ustr },
                       { u"data"_ustr, u"2"_ustr } };
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(sSidebarId, u"layoutpanel_icons"_ustr, aSelect));
    StringMap aContextMenu{
        { u"type"_ustr, u"iconview"_ustr },
        { u"cmd"_ustr, u"contextmenu"_ustr },
        { u"data"_ustr, u"%7B%22row%22%3A2%2C%22clientX%22%3A500%2C%22clientY%22%3A300%7D"_ustr }
    };
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(sSidebarId, u"layoutpanel_icons"_ustr, aContextMenu));
    Scheduler::ProcessEventsToIdle();

    StringMap aApply{ { u"type"_ustr, u"menu"_ustr },
                      { u"cmd"_ustr, u"select"_ustr },
                      { u"data"_ustr, u"apply"_ustr } };
    // A context menu has no window of its own, so it is registered under the window id 0.
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(u"0"_ustr, u"__MENU__"_ustr, aApply));
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 20
    // i.e. the context menu acted on the Blank Slide layout at position 0.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT_EQUAL(AUTOLAYOUT_TITLE_CONTENT, pViewShell->GetActualPage()->GetAutoLayout());
}

#if ENABLE_PDFIMPORT
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testInsertSignatureLineExternal)
{
    // Given a PDF to be signed:
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"ReadOnly"_ustr, true) };
    createTempCopy(u"empty.pdf");
    loadWithParams(maTempFile.GetURL(), aArgs);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    pImpressDocument->initializeForTiledRendering({});
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    sd::View* pView = pViewShell->GetView();
    pViewShell->GetViewShell()->SetKitAuthor(u"myauthor"_ustr);
    SdTestViewCallback aView;

    // When inserting a signature line for electronic (external) signing:
    aArgs = {
        comphelper::makePropertyValue(u"External"_ustr, true),
    };
    // Without the accompanying fix in place, this test would hang here in the certificate chooser
    // dialog.
    dispatchCommand(mxComponent, u".uno:InsertSignatureLine"_ustr, aArgs);
    // Signature line is selected right after inserting:
    CPPUNIT_ASSERT(pViewShell->GetViewShell()->GetSignPDFCertificate().Is());

    // Then make sure the shape is marked as a signature line:
    std::vector<SdrObject*> aMarkedObjects = pView->GetMarkedObjects();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMarkedObjects.size());
    cpo::uno::Any aAny;
    aMarkedObjects[0]->GetGrabBagItem(aAny);
    comphelper::SequenceAsHashMap aMap(aAny);
    CPPUNIT_ASSERT(aMap.contains(u"SignatureCertificate"_ustr));
    // Also verify that this is exposed at a COKit level:
    OString aShapeSelection = "[" + aView.m_ShapeSelection + "]";
    const char* pShapeSelectionStr = aShapeSelection.getStr();
    std::stringstream aStream(pShapeSelectionStr);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    int nCount = 0;
    bool bSignature = false;
    for (const auto& i : aTree)
    {
        ++nCount;
        if (nCount <= 5)
        {
            // x, y, w, h, part
            continue;
        }
        boost::property_tree::ptree aProps = i.second;
        // Without the accompanying fix in place, this test would have failed with:
        // - No such node (isSignature)
        bSignature = aProps.get<bool>("isSignature");
    }
    CPPUNIT_ASSERT(bSignature);

    // Make sure there is no leaked selection after signing is finished:
    OUString aSigUrl = createFileURL(u"signature.pkcs7");
    SvFileStream aSigStream(aSigUrl, StreamMode::READ);
    auto aSigValue
        = OUString::fromUtf8(read_uInt8s_ToOString(aSigStream, aSigStream.remainingSize()));
    aArgs = {
        comphelper::makePropertyValue(u"SignatureTime"_ustr, u"1643201995722"_ustr),
        comphelper::makePropertyValue(u"SignatureValue"_ustr, aSigValue),
    };
    dispatchCommand(mxComponent, u".uno:Signature"_ustr, aArgs);
    // Signature line is not selected after finishing signing:
    CPPUNIT_ASSERT(!pViewShell->GetViewShell()->GetSignPDFCertificate().Is());
}
#endif

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testNotesViewInvalidations)
{
    // Given a document with 2 slides.
    SdXImpressDocument* pXImpressDocument = createDoc("NotesView.odp");
    SdTestViewCallback aView;
    CPPUNIT_ASSERT_EQUAL(2, pXImpressDocument->getParts());

    // Switching to the second slide.
    pXImpressDocument->setPart(1);

    Scheduler::ProcessEventsToIdle();

    aView.invalidatedAll = false;

    // Switching to notes view.
    dispatchCommand(mxComponent, u".uno:NotesMode"_ustr,
                    cpo::uno::Sequence<beans::PropertyValue>());

    CPPUNIT_ASSERT_EQUAL(true, aView.invalidatedAll);
    CPPUNIT_ASSERT_EQUAL(1, aView.partOfInvalidation);
    CPPUNIT_ASSERT_EQUAL(2, aView.editModeOfInvalidation);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testNotesPanelPasteSpecialUnformatted)
{
    // Given a copy of the title text of the first slide, and the notes pane open with the focus in
    // it.
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    SdrObject* pTitle = pViewShell->GetActualPage()->GetObj(0);
    pView->MarkObj(pTitle, pView->GetSdrPageView());
    pView->SdrBeginTextEdit(pTitle);
    OutlinerView* pOutlinerView = pView->GetTextEditOutlinerView();
    CPPUNIT_ASSERT(pOutlinerView);
    pOutlinerView->GetEditView().SetSelection(ESelection(0, 0, 0, 11)); // "First slide"
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    pView->SdrEndTextEdit();
    pView->UnmarkAll();
    dispatchCommand(mxComponent, u".uno:BottomPaneImpress"_ustr,
                    { comphelper::makePropertyValue(u"BottomPaneImpress"_ustr, true) });
    const OUString sNotesPanelId
        = OUString::number(reinterpret_cast<sal_uInt64>(&pViewShell->GetViewShellBase()))
          + "notespanel";
    StringMap aFocus{ { u"type"_ustr, u"customwidget"_ustr },
                      { u"cmd"_ustr, u"focus"_ustr },
                      { u"data"_ustr, u"{}"_ustr } };
    // The pane opens on a later update of the view configuration, which waits on a timer.
    bool bFocused = false;
    for (int i = 0; i < 100 && !bFocused; ++i)
    {
        Scheduler::ProcessEventsToIdle();
        bFocused = jsdialog::ExecuteAction(sNotesPanelId, u"notesedit"_ustr, aFocus);
        if (!bFocused)
            osl::Thread::wait(std::chrono::milliseconds(50));
    }
    CPPUNIT_ASSERT(bFocused);
    Scheduler::ProcessEventsToIdle();

    // When the text is pasted with Paste Special as unformatted text, which the dialog offers in
    // its second row, after the text with its formatting.
    dispatchCommand(mxComponent, u".uno:PasteSpecial"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    OUString sDialogId;
    for (vcl::Window* pWindow = Application::GetFirstTopLevelWindow(); pWindow;
         pWindow = Application::GetNextTopLevelWindow(pWindow))
    {
        vcl::Window* pDialog = pWindow->GetWindow(GetWindowType::Client);
        if (pDialog && pDialog->IsDialog())
            sDialogId = OUString::number(static_cast<sal_uInt64>(pDialog->GetKitWindowId()));
    }
    CPPUNIT_ASSERT(!sDialogId.isEmpty());
    StringMap aSelect{ { u"type"_ustr, u"treeview"_ustr },
                       { u"cmd"_ustr, u"select"_ustr },
                       { u"data"_ustr, u"1"_ustr } };
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(sDialogId, u"list"_ustr, aSelect));
    StringMap aClick{ { u"type"_ustr, u"responsebutton"_ustr },
                      { u"cmd"_ustr, u"click"_ustr },
                      { u"data"_ustr, OUString::number(RET_OK) } };
    // Without the accompanying fix in place, the paste crashed here.
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(sDialogId, u"ok"_ustr, aClick));
    Scheduler::ProcessEventsToIdle();
    StringMap aBlur{ { u"type"_ustr, u"customwidget"_ustr },
                     { u"cmd"_ustr, u"blur"_ustr },
                     { u"data"_ustr, u"{}"_ustr } };
    CPPUNIT_ASSERT(jsdialog::ExecuteAction(sNotesPanelId, u"notesedit"_ustr, aBlur));
    Scheduler::ProcessEventsToIdle();

    // Then the notes of the slide hold the text.
    SdPage* pNotesPage = pXImpressDocument->GetDoc()->GetSdPage(0, PageKind::Notes);
    auto pNotes = dynamic_cast<SdrTextObj*>(pNotesPage->GetPresObj(PresObjKind::Notes));
    CPPUNIT_ASSERT(pNotes);
    const OutlinerParaObject* pParaObject = pNotes->GetOutlinerParaObject();
    CPPUNIT_ASSERT(pParaObject);
    CPPUNIT_ASSERT_EQUAL(u"First slide"_ustr, pParaObject->GetTextObject().GetText(0));
}

#if ENABLE_PDFIMPORT
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testPdfiumLinks)
{
    // Given a pdf file with links:
    SdXImpressDocument* pDoc = createDoc("link_2p.pdf");
    SdTestViewCallback aView;

    auto get_links_array = [&aView]() -> boost::property_tree::ptree {
        auto it = aView.m_aStateChanges.find("PageLinks");
        CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
        boost::property_tree::ptree pTree = it->second;
        auto it2 = pTree.find("state");
        CPPUNIT_ASSERT(it2 != pTree.not_found());
        auto it3 = it2->second.find("links");
        CPPUNIT_ASSERT(it3 != pTree.not_found());
        return it3->second;
    };

    // First page has a link
    pDoc->setPart(0);
    auto links = get_links_array();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), links.size());
    // auto pLinkInfo = links.get_child("");
    auto pLinkInfo = links.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("767.28x292.999@(1133.86,2013)"),
                         pLinkInfo.get_child("rectangle").get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("http://cidac.pt/"),
                         pLinkInfo.get_child("uri").get_value<std::string>());

    // Second doesn't
    pDoc->setPart(1);
    links = get_links_array();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), links.size());
}
#endif

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideBackgroundRemoteNotFetched)
{
    // Slide background fill image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // The assert in createNewSdrFillGraphicAttribute will fire if
    // a remote fetch is attempted.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"slide-background-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    // The slide's deferred remote background fill bitmap is registered as an
    // external link as the page is imported, so it appears in Edit, Links to
    // External Files and can be updated or broken.
    sfx2::LinkManager* pLinkMgr = pImpressDocument->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_MESSAGE("slide background fill bitmap link should be registered",
                           !pLinkMgr->GetLinks().empty());

    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testMasterPageBackgroundRemoteNotFetched)
{
    // A master page background fill image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed. The master
    // background lands on the master's presentation style sheet, not on the
    // page item set, so the link is registered through the SfxStyleSheet path
    // rather than the SdrPage one.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"master-page-background-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    sfx2::LinkManager* pLinkMgr = pImpressDocument->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_MESSAGE("master page background fill bitmap link should be registered",
                           !pLinkMgr->GetLinks().empty());

    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testBulletImageRemoteNotFetched)
{
    // text:list-level-style-image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // Currently the editeng rendering path silently skips unresolved
    // GraphicExternalLink graphics (GraphicType::Default is not handled
    // by create2DDecompositionOfGraphic). If someone adds fetching here,
    // this test should catch it.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"bullet-image-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testShapeFillRemoteNotFetched)
{
    // draw:fill-image with a remote URL on a shape must not fetch
    // the URL during paint when link updates are not allowed.
    // The assert in createNewSdrFillGraphicAttribute will fire if
    // a remote fetch is attempted.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"shape-fill-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testFormImageRemoteNotFetched)
{
    // Form image button with a remote ImageURL must not fetch the
    // URL during import when link updates are not allowed.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"form-image-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testInsertChartAtClientVisibleCenter)
{
    // Given a Draw document, and a client looking at the lower part of the page:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odg");
    const ::tools::Rectangle aVisibleArea(Point(1000, 6000), Size(9000, 4000));
    pXImpressDocument->setClientVisibleArea(aVisibleArea);

    // When inserting a chart:
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    dispatchCommand(mxComponent, u".uno:InsertObjectChart"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Then it lands where the client looks. Without the accompanying fix in place,
    // this test would have failed: the chart was centred on the kit window's own
    // geometry, a fixed spot of the page outside the client's view.
    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    const ::tools::Rectangle aObject = pPage->GetObj(0)->GetLogicRect();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(convertTwipToMm100(aVisibleArea.Center().X())),
                                 double(aObject.Center().X()), 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(convertTwipToMm100(aVisibleArea.Center().Y())),
                                 double(aObject.Center().Y()), 2.0);
}

// The slide-import fixtures live in the shared sd data directory, next to
// the ones the misc-tests suite uses, rather than being copied per suite.
constexpr OUString gSlideImportDataDir = u"/sd/qa/unit/data/"_ustr;

namespace
{
/// The links rDocument reports, parsed.
boost::property_tree::ptree readLinks(SdXImpressDocument& rDocument)
{
    tools::JsonWriter aJsonWriter;
    const bool bWritten = rDocument.getSlideLinks(aJsonWriter);
    const OString aLinks = aJsonWriter.finishAndGetAsOString();
    CPPUNIT_ASSERT(bWritten);
    std::stringstream aStream((std::string(aLinks)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return aTree;
}

/// The links rDocument reports, each as parsed JSON, in the order they are reported.
std::vector<boost::property_tree::ptree> readLinkList(SdXImpressDocument& rDocument)
{
    const boost::property_tree::ptree aTree = readLinks(rDocument);
    std::vector<boost::property_tree::ptree> aLinks;
    for (const auto& rLink : aTree.get_child("links"))
        aLinks.push_back(rLink.second);
    return aLinks;
}

/// The identifier of the slide at nIndex of rDoc.
OUString getSlideId(SdDrawDocument& rDoc, sal_uInt16 nIndex)
{
    SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
    CPPUNIT_ASSERT(pPage);
    return pPage->GetGuid().getOUString();
}

/// The link of the slide at nIndex of rDoc, or nothing when the slide is linked to nothing.
const sd::SlideLinkRecord* findSlideLink(SdDrawDocument& rDoc, sal_uInt16 nIndex)
{
    return rDoc.GetSlideLinkManager().Find(sd::SlideLinkKind::Slide, getSlideId(rDoc, nIndex));
}

/// The link of the section at nSection of rDoc, or nothing when the section is linked to nothing.
const sd::SlideLinkRecord* findSectionLink(SdDrawDocument& rDoc, sal_Int32 nSection)
{
    const sd::SlideSectionManager& rSections = rDoc.GetSectionManager();
    CPPUNIT_ASSERT(nSection >= 0);
    CPPUNIT_ASSERT(nSection < rSections.GetSectionCount());
    return rDoc.GetSlideLinkManager().Find(sd::SlideLinkKind::Section,
                                           rSections.GetSection(nSection).maId);
}

/// The text the slide at nIndex of rDoc shows, taking its shapes in order.
OUString getSlideText(SdDrawDocument& rDoc, sal_uInt16 nIndex)
{
    SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
    CPPUNIT_ASSERT(pPage);
    OUStringBuffer aText;
    for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
    {
        auto* pTextObject = dynamic_cast<SdrTextObj*>(pPage->GetObj(nObject));
        if (pTextObject && !pTextObject->IsEmptyPresObj() && pTextObject->GetOutlinerParaObject())
            aText.append(pTextObject->GetOutlinerParaObject()->GetTextObject().GetText(0));
    }
    return aText.makeStringAndClear();
}

/// The shape at nObject on slide nIndex of rDoc, which holds text.
SdrTextObj& getTextShape(SdDrawDocument& rDoc, sal_uInt16 nIndex, size_t nObject)
{
    SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
    CPPUNIT_ASSERT(pPage);
    auto* pTextObject = dynamic_cast<SdrTextObj*>(pPage->GetObj(nObject));
    CPPUNIT_ASSERT(pTextObject);
    CPPUNIT_ASSERT(pTextObject->GetOutlinerParaObject());
    return *pTextObject;
}

/// The letter sizes the runs of the text of the shape at nObject on slide nIndex carry, in
/// hundredths of a millimetre and in the order the runs stand in.
std::vector<sal_uInt32> getRunLetterSizes(SdDrawDocument& rDoc, sal_uInt16 nIndex, size_t nObject)
{
    SdrTextObj* pTextObject = &getTextShape(rDoc, nIndex, nObject);

    std::vector<editeng::Section> aSections;
    pTextObject->GetOutlinerParaObject()->GetTextObject().GetAllSections(aSections);

    std::vector<sal_uInt32> aSizes;
    for (const editeng::Section& rSection : aSections)
        for (const SfxPoolItem* pItem : rSection.maAttributes)
            if (pItem->Which() == EE_CHAR_FONTHEIGHT)
                aSizes.push_back(static_cast<const SvxFontHeightItem*>(pItem)->GetHeight());
    return aSizes;
}
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkList)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // A document whose slides are all its own reports no links at all.
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    // Two slides of one source and one of another are inserted as links, and one slide is inserted
    // as a plain copy.
    const OUString aFirstUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aFirstUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 #1 100%.odp\"}"_ostr));
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aFirstUrl, "{\"slides\":[0],\"at\":0,\"source\":\"Q3 #1 100%.odp\"}"_ostr));

    const OUString aSecondUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSecondUrl, "{\"slides\":[0],\"link\":true,\"source\":\"Support deck.odp\"}"_ostr));

    // Each linked slide is reported once, in document order, with the source it was linked to
    // and the slide of it. The plain copy belongs to no source and is left out.
    const std::vector<boost::property_tree::ptree> aLinks = readLinkList(*pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aLinks.size());
    CPPUNIT_ASSERT_EQUAL(std::string("slide"), aLinks[0].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(std::string("Q3 #1 100%.odp"), aLinks[0].get<std::string>("source"));
    CPPUNIT_ASSERT_EQUAL(std::string("SourceA"), aLinks[0].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(std::string(), aLinks[0].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("Q3 #1 100%.odp"), aLinks[1].get<std::string>("source"));
    CPPUNIT_ASSERT_EQUAL(std::string("TargetOne"), aLinks[1].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(std::string("Support deck.odp"), aLinks[2].get<std::string>("source"));

    // A page is named by the identifier it keeps for the whole session, not by its position. The
    // plain copy landed before the linked pages, so the first of them is the second slide.
    const std::string aPartId(OUStringToOString(getSlideId(*pDoc, 1), RTL_TEXTENCODING_ASCII_US));
    CPPUNIT_ASSERT_EQUAL(aPartId, aLinks[0].get<std::string>("id"));
    std::vector<std::string> aSlides;
    for (const auto& rSlide : aLinks[0].get_child("slides"))
        aSlides.push_back(rSlide.second.get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSlides.size());
    CPPUNIT_ASSERT_EQUAL(aPartId, aSlides[0]);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLink)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The file the slides are read from is one thing and the source document
    // they belong to is another: here the file stands in for a document the
    // user knows by a name that carries characters a reference has to escape.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");

    // A slide inserted as a link reports the source document, the slide of it that it came
    // from, and the time that source was last modified.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"keepDesign\":false,\"link\":true,"
                    "\"source\":\"Q3 #1 100%.odp\",\"lastModifiedTime\":\"2020-09-13T12:26:40Z\"}"_ostr));
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 #1 100%.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->maName);
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr, pLinked->maLastModifiedTime);

    // A slide inserted from the same file without asking for a link is a plain copy, which is
    // linked to nothing.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl,
        "{\"slides\":[0],\"at\":2,\"keepDesign\":false,\"source\":\"Q3 #1 100%.odp\"}"_ostr));
    CPPUNIT_ASSERT(!findSlideLink(*pDoc, 2));

    // An insert that names no source document has nothing for a link to record, so one that
    // asks for links inserts nothing at all.
    const int nPartsBefore = pXImpressDocument->getParts();
    CPPUNIT_ASSERT(
        !pXImpressDocument->insertPagesFromFile(aSourceUrl, "{\"slides\":[0],\"link\":true}"_ostr));
    CPPUNIT_ASSERT_EQUAL(nPartsBefore, pXImpressDocument->getParts());

    // The source is a persistent link the storage resolves, so one of the shape of an address
    // is recorded as it is.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl,
        "{\"slides\":[0],\"link\":true,\"source\":\"https://storage.example/files/42\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(nPartsBefore + 1, pXImpressDocument->getParts());
    const sd::SlideLinkRecord* pAddressed
        = findSlideLink(*pDoc, static_cast<sal_uInt16>(nPartsBefore));
    CPPUNIT_ASSERT(pAddressed);
    CPPUNIT_ASSERT_EQUAL(u"https://storage.example/files/42"_ustr, pAddressed->maSource);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportScalesLetterSizes)
{
    // A slide that comes from a deck whose pages are wider than this document's is fitted to
    // the page it lands on, and the letters it shows are fitted along with it, so the slide
    // reads the same as it did in the deck it came from.
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The source page is 50.8cm by 28.575cm and keeps no border, so the slide is fitted to the
    // room this document's own page leaves, which is a little over half of that.
    SdPage* pPage = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT(pPage);
    const double fScale
        = double(pPage->GetSize().Height() - pPage->GetUpperBorder() - pPage->GetLowerBorder())
          / 28575.0;

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-wide-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":0,\"keepDesign\":true}"_ostr));

    // The title holds one run of 80pt and the text box below it one run of 40pt, which are
    // 2822 and 1411 hundredths of a millimetre. Without the accompanying fix in place, this
    // test would have failed: the shapes were fitted to the smaller page and the letters kept
    // the size they had, so the text ran past the shape that held it and past the page.
    const std::vector<sal_uInt32> aTitleSizes = getRunLetterSizes(*pDoc, 0, 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTitleSizes.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2822.0 * fScale, static_cast<double>(aTitleSizes[0]), 4.0);

    const std::vector<sal_uInt32> aBodySizes = getRunLetterSizes(*pDoc, 0, 1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aBodySizes.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1411.0 * fScale, static_cast<double>(aBodySizes[0]), 4.0);

    // The room the title paragraph keeps for each of its lines and above itself is a length of
    // its own rather than a share of the letter size, so it is fitted to the page as well. It
    // was 3cm and 1cm in the deck the slide came from.
    const SfxItemSet& rParagraph
        = getTextShape(*pDoc, 0, 0).GetOutlinerParaObject()->GetTextObject().GetParaAttribs(0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        3000.0 * fScale,
        static_cast<double>(rParagraph.Get(EE_PARA_SBL).GetLineHeight()), 4.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1000.0 * fScale, static_cast<double>(rParagraph.Get(EE_PARA_ULSPACE).GetUpper()), 4.0);

    // The shapes are fitted to the page they landed on, and the text within them no longer
    // asks for more room than they hold, so neither one grew past the page.
    for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
        CPPUNIT_ASSERT(pPage->GetObj(nObject)->GetLogicRect().GetHeight()
                       <= pPage->GetSize().Height());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testTextFollowsEachDirectionsOwnScale)
{
    // A page that changes shape as well as size scales by a different amount across than it does
    // down. A length that runs across the text follows the one, and a length that runs down it
    // follows the other, so the text keeps the shape the page gives it.
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-wide-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":0,\"keepDesign\":true}"_ostr));

    // The room between one letter and the next runs across the text; the room a paragraph keeps
    // above itself runs down it. Both are held by the shape here rather than by the text.
    SdrTextObj& rShape = getTextShape(*pDoc, 0, 1);
    rShape.SetMergedItem(SvxKerningItem(400, EE_CHAR_KERNING));
    rShape.SetMergedItem(SvxULSpaceItem(800, 0, EE_PARA_ULSPACE));

    rShape.scaleText(0.5, 0.25);

    // Without the accompanying fix in place, this test would have failed: both lengths took the
    // scale for down the text, so the room between the letters came out at a quarter.
    const SfxItemSet& rShapeSet = rShape.GetMergedItemSet();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200), rShapeSet.Get(EE_CHAR_KERNING).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(200), rShapeSet.Get(EE_PARA_ULSPACE).GetUpper());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkFollowsSourceSlidePosition)
{
    // A slide travels to another presentation in a file written out of its own deck. An insert
    // that names the position the slide holds in the source links the page to that position: the
    // position in the source deck, not the one the slide took in the file it travelled in.
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    SdXImpressDocument* pSourceDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pSourceDocument);
    pSourceDocument->initializeForTiledRendering({});

    // The third slide of the source travels on its own, so it stands first in the file it goes in.
    utl::TempFileNamed aOneSlide(u"", true, u".odp");
    aOneSlide.EnableKillingFile();
    CPPUNIT_ASSERT(pSourceDocument->exportPages({ 2 }, aOneSlide.GetURL()));

    // A refresh reads the whole source deck, so the pages stand there as they do in the source.
    utl::TempFileNamed aWholeDeck(u"", true, u".odp");
    aWholeDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pSourceDocument->exportPages({}, aWholeDeck.GetURL()));

    // The same deck once its third slide has been moved to the front and its content revised. The
    // slides keep the identifiers they hold in the deck above.
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-moved.fodp"));
    SdXImpressDocument* pMovedDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pMovedDocument);
    pMovedDocument->initializeForTiledRendering({});
    utl::TempFileNamed aMovedDeck(u"", true, u".odp");
    aMovedDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pMovedDocument->exportPages({}, aMovedDeck.GetURL()));

    // A deck of three slides of its own, standing where the slides of the source deck stood, and
    // the same again with a name of its own on every slide.
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-replaced.fodp"));
    SdXImpressDocument* pReplacedDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pReplacedDocument);
    pReplacedDocument->initializeForTiledRendering({});
    utl::TempFileNamed aReplacedDeck(u"", true, u".odp");
    aReplacedDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pReplacedDocument->exportPages({}, aReplacedDeck.GetURL()));

    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-named-replaced.fodp"));
    SdXImpressDocument* pNamedDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pNamedDocument);
    pNamedDocument->initializeForTiledRendering({});
    utl::TempFileNamed aNamedDeck(u"", true, u".odp");
    aNamedDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pNamedDocument->exportPages({}, aNamedDeck.GetURL()));

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The third slide of the source travelled on its own, so it stands first in the file the
    // insert reads. The insert names the position it holds in the source, which is the third.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aOneSlide.GetURL(),
        "{\"at\":1,\"link\":true,\"sourcePositions\":[3],\"source\":\"Q3 deck.odp\"}"_ostr));

    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Source three"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pLinked->maName);

    // The page is linked to the position and to no identifier, so nothing but the position says
    // which slide it reads.
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pLinked->maSourceId);

    // A refresh reading the whole source deck reads the slide standing at that position, which is
    // the slide the page came from as long as the source holds it there.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aWholeDeck.GetURL(),
                                                              u"2021-01-01T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Source three"_ustr, getSlideText(*pDoc, 1));
    pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pLinked->maName);

    // The slide the page came from moved to the front of the source, so another slide stands at
    // the position the page follows, and that is the slide it reads. The page keeps the position.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aMovedDeck.GetURL(),
                                                              u"2022-02-02T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Source two"_ustr, getSlideText(*pDoc, 1));
    const sd::SlideLinkRecord* pMoved = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pMoved);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pMoved->maName);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pMoved->maSourceId);

    // The slides of the source are other slides now, and the page reads the one standing at its
    // position.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aReplacedDeck.GetURL(),
                                                              u"2023-03-03T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Other three"_ustr, getSlideText(*pDoc, 1));

    // The slide standing at that position carries a name of its own. The page follows the
    // position rather than a name, so it reads that slide and keeps the position it records.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pXImpressDocument->refreshSlideLinks(
                                                        u"Q3 deck.odp"_ustr, aNamedDeck.GetURL(),
                                                        u"2024-04-04T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Named three"_ustr, getSlideText(*pDoc, 1));
    const sd::SlideLinkRecord* pNamed = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pNamed);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pNamed->maName);
    CPPUNIT_ASSERT_EQUAL(u"Slide 3"_ustr, pNamed->maSourceId);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkTracksSourceSlideByIdentifier)
{
    // An insert that asks for no position name records no name for a slide that carries none of
    // its own, so the page follows the identifier of the slide it came from and nothing else.
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    SdXImpressDocument* pSourceDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pSourceDocument);
    pSourceDocument->initializeForTiledRendering({});

    utl::TempFileNamed aOneSlide(u"", true, u".odp");
    aOneSlide.EnableKillingFile();
    CPPUNIT_ASSERT(pSourceDocument->exportPages({ 2 }, aOneSlide.GetURL()));

    // The same deck with that slide moved to the front and its content revised, and a deck of
    // three slides of its own standing where the slides of the source stood.
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-moved.fodp"));
    SdXImpressDocument* pMovedDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pMovedDocument);
    pMovedDocument->initializeForTiledRendering({});
    utl::TempFileNamed aMovedDeck(u"", true, u".odp");
    aMovedDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pMovedDocument->exportPages({}, aMovedDeck.GetURL()));

    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-replaced.fodp"));
    SdXImpressDocument* pReplacedDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pReplacedDocument);
    pReplacedDocument->initializeForTiledRendering({});
    utl::TempFileNamed aReplacedDeck(u"", true, u".odp");
    aReplacedDeck.EnableKillingFile();
    CPPUNIT_ASSERT(pReplacedDocument->exportPages({}, aReplacedDeck.GetURL()));

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aOneSlide.GetURL(), "{\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(OUString(), pLinked->maName);
    CPPUNIT_ASSERT_EQUAL(u"{33333333-3333-3333-3333-333333333333}"_ustr, pLinked->maSourceId);

    // The slide moved to the front of the source, and the page reads it there and stays nameless.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aMovedDeck.GetURL(),
                                                              u"2022-02-02T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Source three, revised"_ustr, getSlideText(*pDoc, 1));
    const sd::SlideLinkRecord* pMoved = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pMoved);
    CPPUNIT_ASSERT_EQUAL(OUString(), pMoved->maName);
    CPPUNIT_ASSERT_EQUAL(u"{33333333-3333-3333-3333-333333333333}"_ustr, pMoved->maSourceId);

    // The slide the page came from is gone from the source, and the slides standing in its place
    // are other slides. The page keeps the content it holds, and the slide it holds is reported.
    std::vector<OString> aNotUpdated;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aReplacedDeck.GetURL(),
                                                              u"2023-03-03T00:00:00Z"_ustr,
                                                              &aNotUpdated));
    CPPUNIT_ASSERT_EQUAL(u"Source three, revised"_ustr, getSlideText(*pDoc, 1));
    pMoved = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pMoved);
    CPPUNIT_ASSERT_EQUAL(u"2022-02-02T00:00:00Z"_ustr, pMoved->maLastModifiedTime);

    // The slide that was left alone is reported, so the user can be told which one it was.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aNotUpdated.size());
    CPPUNIT_ASSERT_EQUAL(pDoc->GetSdPage(1, PageKind::Standard)->GetGuid().getString(),
                         aNotUpdated[0]);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkModifiedTimeRoundtrip)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"lastModifiedTime\":\"2020-09-13T12:26:40Z\"}"_ostr));
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr, pLinked->maLastModifiedTime);

    // The time the source was last modified survives a save to ODF and the load back.
    saveAndReload(TestFilter::ODP);
    SdXImpressDocument* pReloaded = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pReloaded);
    pLinked = findSlideLink(*pReloaded->GetDoc(), 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr, pLinked->maLastModifiedTime);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkNameRoundtrip)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // The link records the name of its source slide, which holds characters a reference would
    // have to escape.
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    sd::SlideLinkRecord aRenamed = *pLinked;
    aRenamed.maName = u"Q3 #1 100%"_ustr;
    CPPUNIT_ASSERT(pDoc->GetSlideLinkManager().Set({ aRenamed }, OUString()));

    // Both the source document and the slide name come back as they were.
    saveAndReload(TestFilter::ODP);
    SdXImpressDocument* pReloaded = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pReloaded);
    pLinked = findSlideLink(*pReloaded->GetDoc(), 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"Q3 #1 100%"_ustr, pLinked->maName);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefresh)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // One slide of the source deck is inserted as a link and the same slide as a plain copy.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":2,\"source\":\"Q3 deck.odp\"}"_ostr));

    // The linked slide is given a name of its own, so that keeping the name of the slide in this
    // document rather than the name of the slide it came from is what the refresh has to do.
    pDoc->GetSdPage(1, PageKind::Standard)->SetName(u"Q3 numbers"_ustr);
    const int nParts = pXImpressDocument->getParts();
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The source deck changed, and the file staged for the refresh holds the changed slides.
    const OUString aChangedUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr, aChangedUrl,
                                                              u"2021-01-01T00:00:00Z"_ustr));

    // The linked slide shows the new content, and it kept its position, its name and its link.
    CPPUNIT_ASSERT_EQUAL(u"Source title, revised"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nParts, pXImpressDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(u"Q3 numbers"_ustr, pDoc->GetSdPage(1, PageKind::Standard)->GetName());
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->maName);
    // The refreshed link records the source time the refresh was given.
    CPPUNIT_ASSERT_EQUAL(u"2021-01-01T00:00:00Z"_ustr, pLinked->maLastModifiedTime);

    // The plain copy of the same slide belongs to no source and keeps what it holds.
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 2));

    // The whole refresh is one undo step, and undoing it brings the old content back with the link.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nParts, pXImpressDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(u"Q3 numbers"_ustr, pDoc->GetSdPage(1, PageKind::Standard)->GetName());
    pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->maName);

    // A source no page of this document is linked to refreshes nothing at all.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLinks(u"Other deck.odp"_ustr, aChangedUrl,
                                                              OUString()));

    // The slides are read from a local file, so a location that would have to be fetched is
    // refused.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              u"https://example.com/deck.odp"_ustr,
                                                              OUString()));

    // A file that holds no slide the linked page records leaves that slide as it is, and a refresh
    // that changes nothing is no undo step of its own.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"),
            OUString()));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshOnePage)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Two slides of one source are inserted as links, in front of the page the document had.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    const OUString aFirstId = pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString();
    const OUString aSecondId = pDoc->GetSdPage(1, PageKind::Standard)->GetGuid().getOUString();

    // The source deck changed. Reading the second page alone leaves the first one the page it is,
    // with the content and the time it holds, and reads the second one again: a refreshed page is
    // the page read for it, so it holds a new identifier and the time of this read.
    const OUString aChangedUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLink(
                             u"slide"_ustr, aSecondId, u"Q3 deck.odp"_ustr, aChangedUrl,
                             u"2021-01-01T00:00:00Z"_ustr, OUString()));
    CPPUNIT_ASSERT_EQUAL(aFirstId, getSlideId(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));
    const sd::SlideLinkRecord* pFirst = findSlideLink(*pDoc, 0);
    CPPUNIT_ASSERT(pFirst);
    CPPUNIT_ASSERT_EQUAL(OUString(), pFirst->maLastModifiedTime);
    CPPUNIT_ASSERT(aSecondId != getSlideId(*pDoc, 1));
    const sd::SlideLinkRecord* pSecond = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pSecond);
    CPPUNIT_ASSERT_EQUAL(u"2021-01-01T00:00:00Z"_ustr, pSecond->maLastModifiedTime);

    // A page that is not linked to the source, and an identifier that names no page, refresh
    // nothing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLink(u"slide"_ustr, getSlideId(*pDoc, 2),
                                                             u"Q3 deck.odp"_ustr, aChangedUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLink(
                             u"slide"_ustr, u"{00000000-0000-0000-0000-000000000000}"_ustr,
                             u"Q3 deck.odp"_ustr, aChangedUrl, OUString(), OUString()));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshKeepsSections)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Two slides of one source are inserted as links, so a refresh reads them as one run of pages.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // A section starts on each of the two linked slides.
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    rSections.AddSection(0, u"Opening"_ustr);
    rSections.AddSection(1, u"Details"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());

    // Both linked slides are refreshed together, and every section still starts on the slide it
    // did.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(2),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp"),
            OUString()));
    CPPUNIT_ASSERT_EQUAL(u"Source title, revised"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, rSections.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(u"Details"_ustr, rSections.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSection(1).mnStartIndex);

    // The slide that starts the second section is refreshed alone, and the section still starts
    // on it.
    const OUString aSecondId = getSlideId(*pDoc, 1);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(1),
        pXImpressDocument->refreshSlideLink(
            u"slide"_ustr, aSecondId, u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp"),
            OUString(), OUString()));
    CPPUNIT_ASSERT(aSecondId != getSlideId(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionIndexForSlide(2) + 1);
}

// A linked section stays linked when a slide of it is refreshed in place, and when that refresh is
// undone
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshKeepsSectionLink)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // The two linked slides make up the first section.
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    rSections.AddSection(0, u"Q3"_ustr);
    rSections.AddSection(2, u"Own slides"_ustr);
    sd::SlideLinkRecord aLink;
    aLink.meKind = sd::SlideLinkKind::Section;
    aLink.maId = rSections.GetSection(0).maId;
    aLink.maSource = u"Q3 deck.odp"_ustr;
    aLink.maSourceId = u"{6A4F0F6E-2B6C-4B47-9F43-7C3C1D2E5A10}"_ustr;
    aLink.maName = u"Q3"_ustr;
    aLink.maSlides = rSections.GetSectionPageGuids(0);
    CPPUNIT_ASSERT(pDoc->GetSlideLinkManager().Set({ aLink }, OUString()));

    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(1),
        pXImpressDocument->refreshSlideLink(
            u"slide"_ustr, getSlideId(*pDoc, 0), u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp"),
            OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(u"Source title, revised"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));

    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
}

// Slides inserted as a linked section go in as a section of their own, which the link list reports
// with its source section and its slides, and the whole insert is one undo step
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSection)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The slides go in at the end of the deck, after the two slides of the document.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":2,\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"lastModifiedTime\":\"2026-10-20T09:15:00Z\","
                    "\"as\":\"section\",\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\","
                    "\"sourceName\":\"Opening\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(0, PageKind::Standard)->GetName());

    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, rSections.GetSection(0).maName);

    // The section comes first, then each of its slides under it.
    const std::vector<boost::property_tree::ptree> aLinks = readLinkList(*pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aLinks.size());
    const boost::property_tree::ptree& rLinked = aLinks[0];
    const std::string aSectionId(
        OUStringToOString(rSections.GetSection(0).maId, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_EQUAL(std::string("section"), rLinked.get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(aSectionId, rLinked.get<std::string>("id"));
    CPPUNIT_ASSERT_EQUAL(std::string(), rLinked.get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("Q3 deck.odp"), rLinked.get<std::string>("source"));
    CPPUNIT_ASSERT_EQUAL(std::string("{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}"),
                         rLinked.get<std::string>("sourceId"));
    CPPUNIT_ASSERT_EQUAL(std::string("Opening"), rLinked.get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(std::string("2026-10-20T09:15:00Z"),
                         rLinked.get<std::string>("lastModifiedTime"));

    // The section spans the pages it holds here, and each slide of it names the page it is here,
    // the section it is under and the source slide it was read from.
    std::vector<std::string> aSlides;
    for (const auto& rSlide : rLinked.get_child("slides"))
        aSlides.push_back(rSlide.second.get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aSlides.size());
    for (sal_uInt16 i = 0; i < 2; ++i)
    {
        const std::string aPageId(
            OUStringToOString(getSlideId(*pDoc, 2 + i), RTL_TEXTENCODING_ASCII_US));
        CPPUNIT_ASSERT_EQUAL(aPageId, aSlides[i]);
        CPPUNIT_ASSERT_EQUAL(std::string("slide"), aLinks[1 + i].get<std::string>("unit"));
        CPPUNIT_ASSERT_EQUAL(aPageId, aLinks[1 + i].get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(aSectionId, aLinks[1 + i].get<std::string>("parent"));
        const sd::SlideLinkRecord* pSlide = findSlideLink(*pDoc, 2 + i);
        CPPUNIT_ASSERT(pSlide);
        CPPUNIT_ASSERT_EQUAL(OUStringToOString(pSlide->maSourceId, RTL_TEXTENCODING_UTF8),
                             OString(aLinks[1 + i].get<std::string>("sourceId")));
    }

    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSectionCount());
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    // Redo gives the section its link back as well.
    pXImpressDocument->GetDocShell()->GetUndoManager()->Redo();
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
}

// Every view learns that the link list changed when a slide goes into the last section, which is
// linked, although no section starts on another slide afterwards
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionEditNotifiesViews)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdTestViewCallback aView;

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"as\":\"section\",\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\","
                    "\"sourceName\":\"Opening\"}"_ostr));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT(aView.m_nSlideLinksChanged > 0);

    // A new slide at the end of the deck joins the linked section, which is the last one.
    aView.m_nSlideLinksChanged = 0;
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY_THROW);
    xPagesSupplier->getDrawPages()->insertNewByIndex(3);
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(1, aView.m_nSlideLinksChanged);
}

// A section link with an identifier that names no section of the source inserts nothing
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionNeedsSectionIdentifier)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(!pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"as\":\"section\",\"sourceId\":\"Opening\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2),
                         pXImpressDocument->GetDoc()->GetSdPageCount(PageKind::Standard));
}

namespace
{
/// Inserts the first two slides of rSourceUrl before slide nAt, or at the end for -1, as a section
/// linked to the source section "Opening" of "Q3 deck.odp", and returns the identifier of the new
/// section.
OUString insertLinkedSection(SdXImpressDocument& rXImpressDocument, const OUString& rSourceUrl,
                             sal_Int32 nAt = -1)
{
    const sal_Int32 nSlideCount = rXImpressDocument.getParts();
    CPPUNIT_ASSERT(rXImpressDocument.insertPagesFromFile(
        rSourceUrl,
        "{\"slides\":[0,1],\"at\":" + OString::number(nAt)
            + ",\"link\":true,\"source\":\"Q3 deck.odp\","
              "\"lastModifiedTime\":\"2026-10-20T09:15:00Z\","
              "\"as\":\"section\",\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\","
              "\"sourceName\":\"Opening\"}"));
    const sd::SlideSectionManager& rSections = rXImpressDocument.GetDoc()->GetSectionManager();
    const sal_Int32 nSection
        = rSections.GetSectionIndexForSlide(nAt >= 0 && nAt < nSlideCount ? nAt : nSlideCount);
    CPPUNIT_ASSERT(findSectionLink(*rXImpressDocument.GetDoc(), nSection));
    return rSections.GetSection(nSection).maId;
}

/// The name of the section that slide nSlide of rDoc starts, or an empty string when it starts
/// none.
OUString sectionStartingAt(SdDrawDocument& rDoc, sal_Int32 nSlide)
{
    const sd::SlideSectionManager& rSections = rDoc.GetSectionManager();
    for (sal_Int32 i = 0; i < rSections.GetSectionCount(); ++i)
        if (rSections.GetSection(i).mnStartIndex == nSlide)
            return rSections.GetSection(i).maName;
    return OUString();
}
}

// A linked section goes in where the user asked, in a deck with no sections, and the slides after
// it go into a section of their own. Undo and redo take the slides and the sections together.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionGoesInAtPosition)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    insertLinkedSection(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp"), 1);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(0, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(3, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 1));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, rSections.GetSectionIndexForSlide(1)));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 3));

    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSectionCount());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 1));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, rSections.GetSectionIndexForSlide(1)));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 3));
}

// A linked section that goes in before the first slide comes before the section that started
// there, and one that goes in inside a section splits it, so the rest keeps the name of that
// section
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionSplitsSection)
{
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    {
        loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
        SdXImpressDocument* pXImpressDocument
            = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        pXImpressDocument->initializeForTiledRendering({});
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
        rSections.AddSection(0, u"First"_ustr);
        rSections.AddSection(1, u"Second"_ustr);

        insertLinkedSection(*pXImpressDocument, aSourceUrl, 0);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionCount());
        CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 0));
        CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
        CPPUNIT_ASSERT_EQUAL(u"First"_ustr, sectionStartingAt(*pDoc, 2));
        CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(2, PageKind::Standard)->GetName());
        CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, sectionStartingAt(*pDoc, 3));
        CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(3, PageKind::Standard)->GetName());
    }
    {
        loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
        SdXImpressDocument* pXImpressDocument
            = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        pXImpressDocument->initializeForTiledRendering({});
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
        rSections.AddSection(0, u"First"_ustr);

        insertLinkedSection(*pXImpressDocument, aSourceUrl, 1);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionCount());
        CPPUNIT_ASSERT_EQUAL(u"First"_ustr, sectionStartingAt(*pDoc, 0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSectionSlideCount(0));
        CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 1));
        CPPUNIT_ASSERT(findSectionLink(*pDoc, 1));
        CPPUNIT_ASSERT_EQUAL(u"First"_ustr, sectionStartingAt(*pDoc, 3));
        CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(3, PageKind::Standard)->GetName());
    }
}

// A section link reads every slide by the identifier of its source slide, so an insert that links
// the pages to positions inserts nothing
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionNeedsLinkedSlides)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(!pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"sourcePositions\":[1,2],\"as\":\"section\","
                    "\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
}

// A linked section read from a file reaches the views as unlinked when a slide is inserted into it
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkSectionLoadedEditNotifiesViews)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    insertLinkedSection(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp"));

    saveAndReload(TestFilter::ODP);
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    SdTestViewCallback aView;

    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY_THROW);
    xPagesSupplier->getDrawPages()->insertNewByIndex(3);
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT(aView.m_nSlideLinksChanged > 0);
}

// Slides that make up a source section and are not linked go in as a plain section with the name of
// the source section, and the slides and the section are one undo step
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportSectionWithoutLink)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp"),
        "{\"slides\":[0,1],\"at\":1,\"as\":\"section\","
        "\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\","
        "\"sourceName\":\"Opening\"}"_ostr));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionSlideCount(0));
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 3));
    for (sal_uInt16 nSlide = 1; nSlide < 3; ++nSlide)
        CPPUNIT_ASSERT(!findSlideLink(*pDoc, nSlide));
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSectionCount());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 3));
}

namespace
{
/// Writes the slides rPages of the presentation xComponent out to a file of its own at rFileUrl,
/// the way a source document writes out the slides another document reads.
void exportSlides(const uno::Reference<lang::XComponent>& xComponent,
                  const std::vector<sal_Int32>& rPages, const OUString& rFileUrl)
{
    SdXImpressDocument* pDocument = dynamic_cast<SdXImpressDocument*>(xComponent.get());
    CPPUNIT_ASSERT(pDocument);
    pDocument->initializeForTiledRendering({});
    CPPUNIT_ASSERT(pDocument->exportPages(rPages, rFileUrl));
}
}

// Slides inserted with the design of their source can be undone and redone, and each step brings
// the slides and their design back
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportKeepDesignUndoRedo)
{
    utl::TempFileNamed aSourceDeck(u"", true, u".odp");
    aSourceDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSourceDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const sal_uInt16 nMasters = pDoc->GetMasterSdPageCount(PageKind::Standard);

    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(aSourceDeck.GetURL(),
                                                          "{\"keepDesign\":true}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    const sal_uInt16 nMastersInserted = pDoc->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT(nMastersInserted > nMasters);

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(nMasters, pDoc->GetMasterSdPageCount(PageKind::Standard));

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(nMastersInserted, pDoc->GetMasterSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 2));

    // A second round goes the same way.
    pUndoManager->Undo();
    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
}


// An update of a linked section gives it the slides its source section holds now, in source order,
// takes the new name of the source section, and is one undo step
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkUpdateSection)
{
    // The source section holds the first two slides of the source deck, and later the second
    // slide revised, a new slide, and the first slide moved to the end.
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());
    utl::TempFileNamed aUpdatedDeck(u"", true, u".odp");
    aUpdatedDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-section-update-source.fodp"));
    exportSlides(mxComponent, {}, aUpdatedDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL());

    // The second slide is edited here, and the update replaces it all the same.
    pDoc->GetSdPage(3, PageKind::Standard)->SetName(u"Edited here"_ustr);

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr,
                             aUpdatedDeck.GetURL(), u"2026-10-22T10:00:00Z"_ustr,
                             u"Opening, revised"_ustr));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(0, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(1, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(u"Source two, revised"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Source four"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 4));

    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(aSectionId, rSections.GetSection(0).maId);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, rSections.GetSection(0).maName);
    const sd::SlideLinkRecord* pLink = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}"_ustr, pLink->maSourceId);
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, pLink->maName);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-22T10:00:00Z"_ustr, pLink->maLastModifiedTime);
    CPPUNIT_ASSERT(bool(rSections.GetSectionPageGuids(0) == pLink->maSlides));

    // Each slide of the section is linked to its source slide on its own as well.
    const std::vector<OUString> aSourceGuids = { u"{22222222-2222-2222-2222-222222222222}"_ustr,
                                                 u"{44444444-4444-4444-4444-444444444444}"_ustr,
                                                 u"{11111111-1111-1111-1111-111111111111}"_ustr };
    for (sal_uInt16 i = 0; i < 3; ++i)
    {
        const sd::SlideLinkRecord* pSlide = findSlideLink(*pDoc, 2 + i);
        CPPUNIT_ASSERT(pSlide);
        CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pSlide->maSource);
        CPPUNIT_ASSERT_EQUAL(aSourceGuids[i], pSlide->maSourceId);
        CPPUNIT_ASSERT_EQUAL(u"2026-10-22T10:00:00Z"_ustr, pSlide->maLastModifiedTime);
        CPPUNIT_ASSERT_EQUAL(aSectionId, pDoc->GetSlideLinkManager().GetParent(*pSlide));
    }

    // One undo brings the old slides, the old name and the old link back.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Source two"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, rSections.GetSection(0).maName);
    pLink = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-20T09:15:00Z"_ustr, pLink->maLastModifiedTime);
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 2));

    // Redo brings the update back with the source design its slides came with.
    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source four"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, rSections.GetSection(0).maName);
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 3));
}

// A linked section in the middle of the deck is updated in place: the section after it keeps its
// slides and starts after the fresh ones, and updating twice from one file leaves the masters of
// the source once in the document
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkUpdateSectionInMiddle)
{
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());
    utl::TempFileNamed aUpdatedDeck(u"", true, u".odp");
    aUpdatedDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-section-update-source.fodp"));
    exportSlides(mxComponent, {}, aUpdatedDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const OUString aSectionId
        = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL(), /*nAt=*/1);
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr,
                             aUpdatedDeck.GetURL(), OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(0, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(u"Source two, revised"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(4, PageKind::Standard)->GetName());
    const sal_Int32 nSection = rSections.GetSectionIndexForId(aSectionId);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSection(nSection).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionSlideCount(nSection));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, nSection));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 4));

    const sal_uInt16 nMasters = pDoc->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr,
                             aUpdatedDeck.GetURL(), OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(nMasters, pDoc->GetMasterSdPageCount(PageKind::Standard));

    pDoc->GetDocSh()->GetUndoManager()->Undo();
    pDoc->GetDocSh()->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, rSections.GetSectionIndexForId(aSectionId)));
}

// A section renamed in this document keeps its own name when its source section is renamed
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkUpdateSectionKeepsLocalName)
{
    // The source section holds the first two slides of the source deck, and later the second
    // slide revised, a new slide, and the first slide moved to the end.
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());
    utl::TempFileNamed aUpdatedDeck(u"", true, u".odp");
    aUpdatedDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-section-update-source.fodp"));
    exportSlides(mxComponent, {}, aUpdatedDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL());

    sd::SlideSectionManager& rSections = pXImpressDocument->GetDoc()->GetSectionManager();
    rSections.RenameSection(0, u"Our opening"_ustr);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr,
                             aUpdatedDeck.GetURL(), OUString(), u"Opening, revised"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Our opening"_ustr, rSections.GetSection(0).maName);
    const sd::SlideLinkRecord* pLink = findSectionLink(*pXImpressDocument->GetDoc(), 0);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, pLink->maName);
}

// Only a section that is linked to the named source is updated, and a refused update changes
// nothing
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkUpdateSectionNeedsLinkedSection)
{
    // The source section holds the first two slides of the source deck, and later the second
    // slide revised, a new slide, and the first slide moved to the end.
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());
    utl::TempFileNamed aUpdatedDeck(u"", true, u".odp");
    aUpdatedDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-section-update-source.fodp"));
    exportSlides(mxComponent, {}, aUpdatedDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL());
    const OUString aFileUrl = aUpdatedDeck.GetURL();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLink(u"section"_ustr, aSectionId,
                                                             u"Other deck.odp"_ustr, aFileUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, u"{00000000-0000-0000-0000-000000000000}"_ustr,
                             u"Q3 deck.odp"_ustr, aFileUrl, OUString(), OUString()));

    // A slide inserted into the section unlinks it, so there is no source section to follow.
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY_THROW);
    xPagesSupplier->getDrawPages()->insertNewByIndex(3);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLink(u"section"_ustr, aSectionId,
                                                             u"Q3 deck.odp"_ustr, aFileUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 2));
}

// A section unlinked from its source section keeps its slides, and each of them stays linked to its
// source slide. Undo links the section again, and redo unlinks it again.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreakSection)
{
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL());
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"section"_ustr, aSectionId, /*bDeep=*/false));
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionSlideCount(0));
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    // The slides keep their own links, and are listed under no section any more.
    for (const boost::property_tree::ptree& rLink : readLinkList(*pXImpressDocument))
    {
        CPPUNIT_ASSERT_EQUAL(std::string("slide"), rLink.get<std::string>("unit"));
        CPPUNIT_ASSERT_EQUAL(std::string(), rLink.get<std::string>("parent"));
    }
    for (sal_uInt16 nSlide = 2; nSlide < 4; ++nSlide)
        CPPUNIT_ASSERT(findSlideLink(*pDoc, nSlide));

    // A section that is no longer linked has no link to take off.
    CPPUNIT_ASSERT(
        !pXImpressDocument->breakSlideLink(u"section"_ustr, aSectionId, /*bDeep=*/false));
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());

    pUndoManager->Undo();
    const sd::SlideLinkRecord* pLink = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}"_ustr, pLink->maSourceId);

    pUndoManager->Redo();
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 2));
}

// A section unlinked together with its slides keeps its slides as slides of this document alone.
// One undo gives the section and each slide their links back, and redo takes them off again.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreakSectionAndSlides)
{
    utl::TempFileNamed aSectionDeck(u"", true, u".odp");
    aSectionDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, { 0, 1 }, aSectionDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aSectionDeck.GetURL());
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // An identifier that names no section changes nothing.
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(
        u"section"_ustr, u"{00000000-0000-0000-0000-000000000000}"_ustr, /*bDeep=*/true));
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());

    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"section"_ustr, aSectionId, /*bDeep=*/true));
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionSlideCount(0));
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Source two"_ustr, getSlideText(*pDoc, 3));
    for (sal_uInt16 nSlide = 2; nSlide < 4; ++nSlide)
        CPPUNIT_ASSERT(!findSlideLink(*pDoc, nSlide));
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    pUndoManager->Undo();
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    for (sal_uInt16 nSlide = 2; nSlide < 4; ++nSlide)
        CPPUNIT_ASSERT(findSlideLink(*pDoc, nSlide));

    pUndoManager->Redo();
    CPPUNIT_ASSERT(!findSectionLink(*pDoc, 0));
    for (sal_uInt16 nSlide = 2; nSlide < 4; ++nSlide)
        CPPUNIT_ASSERT(!findSlideLink(*pDoc, nSlide));
}

// A section link finds its section in a file that lists sections by the identifier of the source
// section, and takes the slides of that section alone; a file that lists other sections but not
// that one refreshes nothing
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshSectionByIdentifier)
{
    // The source deck holds the section "Opening" with one slide and the section "Details" with
    // another, and the whole deck is written out, sections and all.
    utl::TempFileNamed aWholeDeck(u"", true, u".odp");
    aWholeDeck.EnableKillingFile();
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp"));
    exportSlides(mxComponent, {}, aWholeDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Both slides of the deck go in as the section "Opening", which the source holds one slide in.
    const OUString aSectionId = insertLinkedSection(*pXImpressDocument, aWholeDeck.GetURL());
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionSlideCount(0));

    // The refresh takes the one slide the file lists under the section, out of the two it holds.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr, aWholeDeck.GetURL(),
                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSectionSlideCount(0));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));

    // A section the file lists nothing for keeps the slide it holds, which is reported: the link
    // follows another section now, which the deck does not hold.
    const sd::SlideLinkRecord* pLink = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLink);
    sd::SlideLinkRecord aOther = *pLink;
    aOther.maSourceId = u"{00000000-0000-4000-8000-000000000000}"_ustr;
    CPPUNIT_ASSERT(pDoc->GetSlideLinkManager().Set({ aOther }, OUString()));
    std::vector<OString> aNotUpdated;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pXImpressDocument->refreshSlideLink(
                             u"section"_ustr, aSectionId, u"Q3 deck.odp"_ustr, aWholeDeck.GetURL(),
                             OUString(), OUString(), &aNotUpdated));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aNotUpdated.size());
    CPPUNIT_ASSERT_EQUAL(OUStringToOString(getSlideId(*pDoc, 2), RTL_TEXTENCODING_ASCII_US),
                         aNotUpdated[0]);
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
}

// A refresh of a source reads every link to it that is under no other link from one file: a
// linked slide on its own, and a linked section as a whole, in one undo step
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshSource)
{
    utl::TempFileNamed aSourceDeck(u"", true, u".odp");
    aSourceDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-unnamed-source.fodp"));
    exportSlides(mxComponent, {}, aSourceDeck.GetURL());
    utl::TempFileNamed aUpdatedDeck(u"", true, u".odp");
    aUpdatedDeck.EnableKillingFile();
    loadFromURL(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-section-update-source.fodp"));
    exportSlides(mxComponent, {}, aUpdatedDeck.GetURL());

    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The first slide of the source goes in on its own at the front, and the other two as a
    // section at the end.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceDeck.GetURL(),
        "{\"slides\":[0],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceDeck.GetURL(),
        "{\"slides\":[1,2],\"link\":true,\"source\":\"Q3 deck.odp\","
        "\"as\":\"section\",\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\","
        "\"sourceName\":\"Opening\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(u"Source two"_ustr, getSlideText(*pDoc, 3));

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The slide reads the slide it follows, and the section the three slides the file lists under
    // its source section, which count once.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr,
                                                              aUpdatedDeck.GetURL(),
                                                              u"2026-10-22T10:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(u"Source two, revised"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Source four"_ustr, getSlideText(*pDoc, 4));
    CPPUNIT_ASSERT_EQUAL(u"Source one"_ustr, getSlideText(*pDoc, 5));
    const sd::SlideLinkRecord* pSlide = findSlideLink(*pDoc, 0);
    CPPUNIT_ASSERT(pSlide);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-22T10:00:00Z"_ustr, pSlide->maLastModifiedTime);
    const sd::SlideLinkRecord* pSection = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pSection);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-22T10:00:00Z"_ustr, pSection->maLastModifiedTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pSection->maSlides.size());

    // The section takes the name the file lists for its source section, since the refresh of
    // a source names none itself.
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, pSection->maName);
    CPPUNIT_ASSERT_EQUAL(u"Opening, revised"_ustr, pDoc->GetSectionManager().GetSection(0).maName);

    // The whole refresh is one undo step.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Source two"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, pDoc->GetSectionManager().GetSection(0).maName);
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 0));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkSourceNotFetched)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // A linked slide holds no link with the sfx2 link manager: its link is a record of the
    // document, which only a refresh with a staged file reads.
    pDoc->GetSdPage(1, PageKind::Standard)->ConnectLink();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkManager);
    CPPUNIT_ASSERT(pLinkManager->GetLinks().empty());

    // Updating all links reaches nothing, so the slide keeps the content it holds, with its link.
    pLinkManager->UpdateAllLinks(/*bAskUpdate=*/false, u""_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    const sd::SlideLinkRecord* pLinked = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLinked->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->maName);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreak)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Two slides of one source are inserted as links, so that taking the source off one of them
    // leaves the other linked.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The first of the two keeps the content it holds and belongs to no source any more, and the
    // second is still linked to the source both came from.
    const OUString aFirstId = getSlideId(*pDoc, 0);
    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"slide"_ustr, aFirstId, /*bDeep=*/false));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT(!findSlideLink(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());

    const std::vector<boost::property_tree::ptree> aLinks = readLinkList(*pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aLinks.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TargetOne"), aLinks[0].get<std::string>("name"));

    // A page that belongs to no source, an identifier that names no page at all, and a kind of no
    // link leave the document as it is.
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(u"slide"_ustr, aFirstId, /*bDeep=*/false));
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(
        u"slide"_ustr, u"{00000000-0000-0000-0000-000000000000}"_ustr, /*bDeep=*/false));
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(u"deck"_ustr, getSlideId(*pDoc, 1),
                                                      /*bDeep=*/false));

    // A refresh of the source reads the slide of the page that is still linked and leaves the other
    // one alone.
    const OUString aChangedUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr, aChangedUrl,
                                                              u"2021-01-01T00:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));

    // Undoing the refresh and then the break gives the page the source and the slide it recorded
    // back.
    pUndoManager->Undo();
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
    const sd::SlideLinkRecord* pRestored = findSlideLink(*pDoc, 0);
    CPPUNIT_ASSERT(pRestored);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pRestored->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pRestored->maName);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreakOnEdit)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // One slide of the source deck is inserted as a link, and the whole insertion leaves it linked.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         readLinks(*pXImpressDocument).get_child("links").size());

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    sd::ViewShell* pViewShell = pDoc->GetDocSh()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    sd::View* pView = pViewShell->GetView();
    SdPage* pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT(pLinked->GetObjCount() > 0);
    SdrObject* pObject = pLinked->GetObj(0);

    // A change with no undo step in hand is the engine's own work on the page, the shape a link
    // update or an embedded object gives it, so the page keeps its link.
    pObject->Move(Size(100, 100));
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());

    // A shape of the linked slide is moved by somebody editing it.
    pXImpressDocument->setPart(1);
    const Point aStart = pObject->GetSnapRect().TopLeft();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pView->MoveMarkedObj(Size(500, 500));

    // The slide became this document's own, and a refresh of the source it came from has no page
    // left to read.
    CPPUNIT_ASSERT(!findSlideLink(*pDoc, 1));
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(-1),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp"),
            OUString()));

    // The edit and the source it took off are one undo step, so one undo puts the shape back where
    // it was and the slide back with its source.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(aStart, pObject->GetSnapRect().TopLeft());
    const sd::SlideLinkRecord* pRestored = findSlideLink(*pDoc, 1);
    CPPUNIT_ASSERT(pRestored);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pRestored->maSource);
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pRestored->maName);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         readLinks(*pXImpressDocument).get_child("links").size());

    // A page linked to a file by its path belongs to the older linked-page feature, which no
    // refresh of a source document reaches, so an edit of it leaves what it records alone.
    SdPage* pPathLinked = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT(pPathLinked);
    CPPUNIT_ASSERT(pPathLinked->GetObjCount() > 0);
    pPathLinked->SetFileName(u"file:///decks/q3.odp"_ustr);
    pPathLinked->SetBookmarkName(u"Slide 1"_ustr);
    pXImpressDocument->setPart(0);
    pView->MarkObj(pPathLinked->GetObj(0), pView->GetSdrPageView());
    pView->MoveMarkedObj(Size(500, 500));
    CPPUNIT_ASSERT_EQUAL(u"file:///decks/q3.odp"_ustr, pPathLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"Slide 1"_ustr, pPathLinked->GetBookmarkName());
}

// A written presentation carries the sections of the pages it holds, under the identifiers and
// names they have in the writing document, trimmed to the written pages
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testExportPagesWritesSections)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    const sd::SlideSectionManager& rSections = pXImpressDocument->GetDoc()->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    const OUString aOpeningId = rSections.GetSection(0).maId;
    const OUString aDetailsId = rSections.GetSection(1).maId;

    // The pages are written in the other order, so the sections come in that order as well.
    utl::TempFileNamed aWritten(u"", true, u".odp");
    aWritten.EnableKillingFile();
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({ 1, 0 }, aWritten.GetURL()));

    uno::Reference<lang::XComponent> xWritten(loadFromDesktop(aWritten.GetURL()));
    SdXImpressDocument* pWrittenDocument = dynamic_cast<SdXImpressDocument*>(xWritten.get());
    CPPUNIT_ASSERT(pWrittenDocument);
    const sd::SlideSectionManager& rWritten = pWrittenDocument->GetDoc()->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rWritten.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Details"_ustr, rWritten.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(aDetailsId, rWritten.GetSection(0).maId);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rWritten.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, rWritten.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(aOpeningId, rWritten.GetSection(1).maId);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rWritten.GetSection(1).mnStartIndex);
    xWritten->dispose();

    // A write of one page carries the section of that page alone.
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({ 1 }, aWritten.GetURL()));
    xWritten = loadFromDesktop(aWritten.GetURL());
    pWrittenDocument = dynamic_cast<SdXImpressDocument*>(xWritten.get());
    CPPUNIT_ASSERT(pWrittenDocument);
    const sd::SlideSectionManager& rOneWritten = pWrittenDocument->GetDoc()->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rOneWritten.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(aDetailsId, rOneWritten.GetSection(0).maId);
    xWritten->dispose();
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testExportPages)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const sal_uInt16 nPages = pDoc->GetSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT(nPages > 1);

    // Two of the pages are written out as a presentation of their own.
    utl::TempFileNamed aWritten(u"", true, u".odp");
    aWritten.EnableKillingFile();
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({ 1, 0 }, aWritten.GetURL()));

    // The document that was written holds those pages, in the order they were asked for, and
    // the document they came from is as it was.
    CPPUNIT_ASSERT_EQUAL(nPages, pDoc->GetSdPageCount(PageKind::Standard));

    uno::Reference<lang::XComponent> xWritten(loadFromDesktop(aWritten.GetURL()));
    SdXImpressDocument* pWrittenDocument = dynamic_cast<SdXImpressDocument*>(xWritten.get());
    CPPUNIT_ASSERT(pWrittenDocument);
    SdDrawDocument* pWritten = pWrittenDocument->GetDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), pWritten->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(getSlideText(*pDoc, 1), getSlideText(*pWritten, 0));
    CPPUNIT_ASSERT_EQUAL(getSlideText(*pDoc, 0), getSlideText(*pWritten, 1));

    // A slide is a standard page and the notes page that belongs to it, so the written
    // document holds one notes page per slide and each slide finds its own.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), pWritten->GetSdPageCount(PageKind::Notes));
    for (sal_uInt16 nPage = 0; nPage < 2; ++nPage)
    {
        const SdPage* pSlide = pWritten->GetSdPage(nPage, PageKind::Standard);
        const SdPage* pNotes = pWritten->GetSdPage(nPage, PageKind::Notes);
        CPPUNIT_ASSERT(pSlide);
        CPPUNIT_ASSERT(pNotes);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(pSlide->GetPageNum() + 1),
                             pNotes->GetPageNum());
    }

    xWritten->dispose();

    // A page can be found by the identifier it keeps
    const OUString aGuid = pDoc->GetSdPage(1, PageKind::Standard)->GetGuid().getOUString();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pXImpressDocument->getSlideIndexOfGuid(aGuid));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                         pXImpressDocument->getSlideIndexOfGuid(
                             u"{99999999-9999-9999-9999-999999999999}"_ustr));

    // A page the document does not hold is written by nobody.
    utl::TempFileNamed aRefused(u"", true, u".odp");
    aRefused.EnableKillingFile();
    CPPUNIT_ASSERT(!pXImpressDocument->exportPages({ 0, nPages }, aRefused.GetURL()));

    // An empty list writes every page.
    utl::TempFileNamed aWhole(u"", true, u".odp");
    aWhole.EnableKillingFile();
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({}, aWhole.GetURL()));
    uno::Reference<lang::XComponent> xWhole(loadFromDesktop(aWhole.GetURL()));
    SdXImpressDocument* pWholeDocument = dynamic_cast<SdXImpressDocument*>(xWhole.get());
    CPPUNIT_ASSERT(pWholeDocument);
    CPPUNIT_ASSERT_EQUAL(nPages, pWholeDocument->GetDoc()->GetSdPageCount(PageKind::Standard));
    xWhole->dispose();
}

// Leaving the master view puts the view back on the slide it was on, not on the first slide.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideKeptWhenLeavingMasterView)
{
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    Scheduler::ProcessEventsToIdle();

    pXImpressDocument->setPart(1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getPart());

    pXImpressDocument->setEditMode(1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getEditMode());

    pXImpressDocument->setEditMode(0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getEditMode());

    // Without the fix the view came back on the first slide.
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getPart());
}

// A view that goes through the master view comes back on its own slide, even when another view
// selected a different slide in the meantime.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideKeptWhenLeavingMasterViewTwoViews)
{
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    const int nView1 = KitHelper::getCurrentView();
    pXImpressDocument->setPart(1);
    Scheduler::ProcessEventsToIdle();

    KitHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    const int nView2 = KitHelper::getCurrentView();
    pXImpressDocument->setPart(0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getPart());

    KitHelper::setView(nView1);
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getPart());

    pXImpressDocument->setEditMode(1);
    Scheduler::ProcessEventsToIdle();
    pXImpressDocument->setEditMode(0);
    Scheduler::ProcessEventsToIdle();

    // Without the fix the first view followed the second view's selection to the first slide.
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getEditMode());
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getPart());

    KitHelper::setView(nView2);
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getPart());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testAnimatedGifSelectableOutsidePaintedTile)
{
    // An animated GIF is selected by a click, whichever area of the slide was painted last.
    SdXImpressDocument* pXImpressDocument = createDoc("animated-gif.fodp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrObject* pGif = pViewShell->GetActualPage()->GetObj(0);
    CPPUNIT_ASSERT(static_cast<SdrGrafObj*>(pGif)->IsAnimated());

    // The tile is the top left corner of the slide, away from the GIF.
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(256, 256));
    pXImpressDocument->paintTile(*pDevice, 256, 256, 0, 0, 2268, 2268);

    const Point aCenter = pGif->GetSnapRect().Center();
    const int nX = o3tl::toTwips(aCenter.getX(), o3tl::Length::mm100);
    const int nY = o3tl::toTwips(aCenter.getY(), o3tl::Length::mm100);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, nX, nY, 1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, nX, nY, 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, the click selected nothing.
    const SdrMarkList& rMarkList = pViewShell->GetView()->GetMarkedObjectList();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rMarkList.GetMarkCount());
    CPPUNIT_ASSERT_EQUAL(pGif, rMarkList.GetMark(0)->GetMarkedSdrObj());
}

namespace
{
/// The parts that the view saw invalidated whole in the given edit mode, in ascending order and
/// each followed by a space. The part nLeftOutPart is not listed.
OString wholePartInvalidationsInMode(const SdTestViewCallback& rView, int nWantedMode,
                                     int nLeftOutPart = -1)
{
    std::set<int> aParts;
    for (const auto& [nPart, nMode] : rView.m_aWholePartInvalidations)
    {
        if (nMode == nWantedMode && nPart != nLeftOutPart)
            aParts.insert(nPart);
    }
    OStringBuffer aBuffer;
    for (const int nPart : aParts)
        aBuffer.append(OString::number(nPart) + " ");
    return aBuffer.makeStringAndClear();
}
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testMasterBackgroundInvalidatesItsSlides)
{
    // Given a presentation of 8 slides, whose third slide uses a master of its own while the others
    // share the first master:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    SdDrawDocument* pDoc = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pDoc->GetSdPageCount(PageKind::Standard));
    uno::Reference<drawing::XMasterPagesSupplier> xMastersSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xOtherMaster
        = xMastersSupplier->getMasterPages()->insertNewByIndex(1);
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XMasterPageTarget> xThirdSlide(
        xPagesSupplier->getDrawPages()->getByIndex(2), uno::UNO_QUERY_THROW);
    xThirdSlide->setMasterPage(xOtherMaster);
    SdPage* pMasterPage = pDoc->GetMasterSdPage(0, PageKind::Standard);

    // When the view is in master view and the background of the first master changes:
    SdTestViewCallback aView;
    dispatchCommand(mxComponent, u".uno:SlideMasterPage"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getEditMode());
    aView.m_aWholePartInvalidations.clear();
    SfxStyleSheet* pStyleSheet = pMasterPage->getSdrPageProperties().GetStyleSheet();
    CPPUNIT_ASSERT(pStyleSheet);
    pStyleSheet->GetItemSet().Put(XFillStyleItem(drawing::FillStyle_SOLID));
    pStyleSheet->GetItemSet().Put(XFillColorItem(OUString(), COL_LIGHTRED));
    pStyleSheet->Broadcast(SfxHint(SfxHintId::DataChanged));

    // Then every slide that uses the master, and the notes page of each, is invalidated whole,
    // although the view shows none of them, and the slide with the other master is left alone.
    // Without the accompanying fix in place, this test would have failed: only the master itself
    // was invalidated.
    CPPUNIT_ASSERT_EQUAL("0 1 3 4 5 6 7 "_ostr, wholePartInvalidationsInMode(aView, 0));
    CPPUNIT_ASSERT_EQUAL("0 1 3 4 5 6 7 "_ostr, wholePartInvalidationsInMode(aView, 2));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLayoutKeepsOtherSlidesOfItsMaster)
{
    // Given a presentation of 8 slides that share one master, shown in normal view:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    SdDrawDocument* pDoc = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getEditMode());
    SdTestViewCallback aView;
    aView.m_aWholePartInvalidations.clear();

    // When the second slide gets another layout, which leaves the background of the master as it
    // was:
    pDoc->GetSdPage(1, PageKind::Standard)->SetAutoLayout(AUTOLAYOUT_TITLE_CONTENT, true);

    // Then no other slide of the master and no notes page is invalidated whole. Without the
    // accompanying fix in place, this test would have failed: the master announced a change of its
    // background, and every slide of the master was invalidated.
    CPPUNIT_ASSERT_EQUAL(""_ostr, wholePartInvalidationsInMode(aView, 0, 1));
    CPPUNIT_ASSERT_EQUAL(""_ostr, wholePartInvalidationsInMode(aView, 2));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlidePropertiesInMasterViewKeepsActualSlide)
{
    // Given a presentation of 8 slides and 9 masters, shown in master view on the last master:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    SdDrawDocument* pDoc = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pDoc->GetSdPageCount(PageKind::Standard));
    uno::Reference<drawing::XMasterPagesSupplier> xMastersSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xMasters = xMastersSupplier->getMasterPages();
    while (xMasters->getCount() < 9)
        xMasters->insertNewByIndex(xMasters->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(9), pDoc->GetMasterSdPageCount(PageKind::Standard));
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pFirstSlide = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(pFirstSlide, pViewShell->GetActualPage());
    dispatchCommand(mxComponent, u".uno:SlideMasterPage"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, pXImpressDocument->getEditMode());
    pXImpressDocument->setPart(8);

    // When the Slide Properties sidebar is toggled:
    dispatchCommand(mxComponent, u".uno:ModifyPage"_ustr, {});

    // Then the view still sits on the slide it came from. Without the accompanying fix in place,
    // this test would have failed: the position of the master in the master list was read as a
    // slide position, and with no ninth slide the view was left without a page, so the next
    // status bar update crashed.
    CPPUNIT_ASSERT_EQUAL(pFirstSlide, pViewShell->GetActualPage());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testA11yNumberedParagraphPrefix)
{
    // Given a text box whose first paragraph is "1. plain BOLD end", with screen reader support on:
    loadFromURL(m_directories.getURLFromSrc(u"/sd/qa/unit/a11y/data/bulleted-runs.fodp"));
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewShell()->SetKitAccessibilityState(true);

    // When editing starts at the beginning of that paragraph:
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pViewShell->GetActualPage()->GetObj(0), pView->GetSdrPageView());
    pXImpressDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RETURN);
    pXImpressDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    pXImpressDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_MOD1 | KEY_HOME);
    pXImpressDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_MOD1 | KEY_HOME);
    Scheduler::ProcessEventsToIdle();

    // Then the paragraph text has no list prefix, as the label is not part of it:
    CPPUNIT_ASSERT_EQUAL(std::string("plain BOLD end"),
                         aView.m_aA11yFocusedParagraph.get<std::string>("content"));
    // Without the fix, it was 6, the length of the first formatting run.
    CPPUNIT_ASSERT_EQUAL(0, aView.m_aA11yFocusedParagraph.get<int>("listPrefixLength", 0));
}

// A view that shows the slide of another view's text edit keeps the cursor of its window. The
// outliner view that follows that text edit in this view goes away with the text edit, so a text
// edit of this view must not find the cursor of that outliner view on the window and restore it at
// its end.
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testShowPageKeepsWindowCursor)
{
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    const int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    const int nView2 = KitHelper::getCurrentView();
    pXImpressDocument->setPart(1);
    Scheduler::ProcessEventsToIdle();

    // The first view edits the title of the first slide.
    KitHelper::setView(nView1);
    sd::ViewShell* pViewShell1 = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView1 = pViewShell1->GetView();
    pView1->SdrBeginTextEdit(pViewShell1->GetActualPage()->GetObj(0));
    CPPUNIT_ASSERT(pView1->GetTextEditObject());

    // The second view goes to the first slide.
    KitHelper::setView(nView2);
    sd::ViewShell* pViewShell2 = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell1 != pViewShell2);
    vcl::Window* pWindow2 = pViewShell2->GetActiveWindow();
    vcl::Cursor* pCursor2 = pWindow2->GetCursor();
    pXImpressDocument->setPart(0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getPart());

    // Without the fix in place, the window of the second view showed the cursor of the outliner
    // view that the first view's text edit deletes when it ends.
    CPPUNIT_ASSERT_EQUAL(pCursor2, pWindow2->GetCursor());
}

/// Puts a link to rURL at the cursor of the text being edited, with the cursor just after it.
static void insertLinkAtCursor(SdrView* pView, const OUString& rURL)
{
    OutlinerView* pOutlinerView = pView->GetTextEditOutlinerView();
    CPPUNIT_ASSERT(pOutlinerView);
    pOutlinerView->InsertField(
        SvxFieldItem(SvxURLField(rURL, u"link"_ustr, SvxURLFormat::Repr), EE_FEATURE_FIELD));
    CPPUNIT_ASSERT(pOutlinerView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testOpenHyperlinkOnlyOrdinaryProtocols)
{
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    SdrObject* pTitle = pViewShell->GetActualPage()->GetObj(0);
    pView->MarkObj(pTitle, pView->GetSdrPageView());
    pView->SdrBeginTextEdit(pTitle);
    CPPUNIT_ASSERT(pView->GetTextEditObject());

    // A link to a dispatch command is not passed to the client.
    insertLinkAtCursor(pView, u".uno:About"_ustr);
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OPEN_HYPERLINK, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT(aView.m_aHyperlinkClicked.isEmpty());

    // A web link is.
    insertLinkAtCursor(pView, u"https://www.example.com/"_ustr);
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OPEN_HYPERLINK, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL("https://www.example.com/"_ostr, aView.m_aHyperlinkClicked);
}

namespace
{
/// The spreadsheet object of embedded-spreadsheet.odp that holds the revenue table.
SdrOle2Obj* revenueSheetOf(sd::ViewShell& rViewShell)
{
    SdrOle2Obj* pSheet = nullptr;
    for (const rtl::Reference<SdrObject>& pObject : *rViewShell.getCurrentPage())
    {
        SdrOle2Obj* pOle = dynamic_cast<SdrOle2Obj*>(pObject.get());
        // the larger of the two sheets holds the table
        if (pOle && (!pSheet || pOle->GetLogicRect().GetWidth() > pSheet->GetLogicRect().GetWidth()))
            pSheet = pOle;
    }
    CPPUNIT_ASSERT(pSheet);
    return pSheet;
}

/// The spreadsheet object of embedded-spreadsheet.odp that holds the revenue table, active in
/// place in the view after this.
SdrOle2Obj* activateRevenueSheet(sd::ViewShell& rViewShell)
{
    SdrOle2Obj* pSheet = revenueSheetOf(rViewShell);
    rViewShell.ActivateObject(pSheet, embed::EmbedVerbs::MS_OLEVERB_PRIMARY);
    Scheduler::ProcessEventsToIdle();
    return pSheet;
}

/// The rectangle of the last payload of the type that the view was sent, empty for none.
tools::Rectangle lastRectangleOf(const SdTestViewCallback& rView, COKitCallbackType eType)
{
    for (auto it = rView.m_aCallbacks.rbegin(); it != rView.m_aCallbacks.rend(); ++it)
    {
        if (it->first != eType || it->second == "EMPTY")
            continue;
        cpo::uno::Sequence<OUString> aFields
            = comphelper::string::convertCommaSeparated(OUString::fromUtf8(it->second));
        CPPUNIT_ASSERT(aFields.getLength() >= 4);
        return tools::Rectangle(Point(aFields[0].toInt32(), aFields[1].toInt32()),
                                Size(aFields[2].toInt32(), aFields[3].toInt32()));
    }
    return {};
}

uno::Reference<table::XCellRange> firstSheetOf(const SdrOle2Obj& rSheet)
{
    uno::Reference<sheet::XSpreadsheetDocument> xDocument(rSheet.getXModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocument.is());
    uno::Reference<container::XIndexAccess> xSheets(xDocument->getSheets(), uno::UNO_QUERY);
    return uno::Reference<table::XCellRange>(xSheets->getByIndex(0), uno::UNO_QUERY);
}

void typeText(SdXImpressDocument& rDocument, std::string_view aText)
{
    for (char c : aText)
    {
        rDocument.postKeyEvent(COKitKeyEventType::DOWN, c, 0);
        rDocument.postKeyEvent(COKitKeyEventType::UP, c, 0);
    }
    Scheduler::ProcessEventsToIdle();
}
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testEmbeddedSpreadsheetEditInPlace)
{
    // Given a slide with a spreadsheet that shows A1:J21 of its sheet, active in place:
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    comphelper::ScopeGuard aResetFlags([] { comphelper::COKit::resetCompatFlag(); });
    SdXImpressDocument* pXImpressDocument = createDoc("embedded-spreadsheet.odp");
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrOle2Obj* pSheet = activateRevenueSheet(*pViewShell);
    SfxInPlaceClient* pIPClient = pViewShell->GetViewShell()->GetIPClient();
    CPPUNIT_ASSERT(pIPClient);
    comphelper::ScopeGuard aLeave([pIPClient] {
        pIPClient->DeactivateObject();
        Scheduler::ProcessEventsToIdle();
    });
    // The cell cursor shows on A1, at the corner of the object.
    const Point aCorner = o3tl::convert(pIPClient->GetObjArea().TopLeft(), o3tl::Length::mm100,
                                        o3tl::Length::twip);
    tools::Rectangle aCursor = lastRectangleOf(aView, COKitCallbackType::CELL_CURSOR);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aCorner.X()), double(aCursor.Left()), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aCorner.Y()), double(aCursor.Top()), 1.0);

    // When B2 is clicked, in the middle of its 1280 x 256 twips, and 777 is typed into it:
    aView.m_aCallbacks.clear();
    const Point aB2(aCorner.X() + 1280 + 640, aCorner.Y() + 256 + 128);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // Then the cell cursor goes to B2, in the coordinates of the slide. Without the fix, the
    // click went to the slide and the spreadsheet reported nothing.
    aCursor = lastRectangleOf(aView, COKitCallbackType::CELL_CURSOR);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aCorner.X() + 1280), double(aCursor.Left()), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aCorner.Y() + 256), double(aCursor.Top()), 1.0);

    typeText(*pXImpressDocument, "777");
    // The caret of the cell shows in the coordinates of the slide too, inside B2.
    bool bCaretInB2 = false;
    for (const std::pair<COKitCallbackType, OString>& rCallback : aView.m_aCallbacks)
    {
        if (rCallback.first != COKitCallbackType::INVALIDATE_VISIBLE_CURSOR)
            continue;
        boost::property_tree::ptree aTree;
        std::stringstream aStream((std::string(rCallback.second)));
        boost::property_tree::read_json(aStream, aTree);
        cpo::uno::Sequence<OUString> aFields = comphelper::string::convertCommaSeparated(
            OUString::fromUtf8(aTree.get<std::string>("rectangle")));
        const Point aCaret(aFields[0].toInt32(), aFields[1].toInt32());
        // the edit area of the cell reaches a little over its edges
        tools::Rectangle aB2Area(aCursor.TopLeft(), Size(1280, 256));
        aB2Area.expand(15);
        bCaretInB2 = aB2Area.Contains(aCaret);
        // the caret is given in the document alone, not relative to a point of the sheet
        CPPUNIT_ASSERT(!aTree.get_optional<std::string>("refpoint"));
    }
    CPPUNIT_ASSERT(bCaretInB2);

    pXImpressDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RETURN);
    pXImpressDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // And Enter puts 777 into B2 and goes on to B3.
    CPPUNIT_ASSERT_EQUAL(u"777"_ustr, firstSheetOf(*pSheet)->getCellByPosition(1, 1)->getFormula());
    aCursor = lastRectangleOf(aView, COKitCallbackType::CELL_CURSOR);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aCorner.Y() + 512), double(aCursor.Top()), 1.0);

    // The cursor of the view of the spreadsheet is no cursor of another view of the slide.
    for (const std::pair<COKitCallbackType, OString>& rCallback : aView.m_aCallbacks)
        CPPUNIT_ASSERT(rCallback.first != COKitCallbackType::CELL_VIEW_CURSOR);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testEmbeddedSpreadsheetVectorContentFollowsTyping)
{
    // Given a view that draws the slide from its objects, with the revenue spreadsheet active in
    // place and B2 selected:
    SdXImpressDocument* pXImpressDocument = createDoc("embedded-spreadsheet.odp");
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewShell()->setDrawsFromObjects(true);
    const OString aPartId = pViewShell->getCurrentPage()->GetGuid().getString();
    {
        // the client holds the slide
        tools::JsonWriter aWriter;
        const OString aCommand = ".uno:VectorPrimitives?partid=" + aPartId + "&mode=0";
        pXImpressDocument->getCommandValues(aWriter, std::string_view(aCommand));
        aWriter.finishAndGetAsOString();
    }
    SdrOle2Obj* pSheet = activateRevenueSheet(*pViewShell);
    SfxInPlaceClient* pIPClient = pViewShell->GetViewShell()->GetIPClient();
    CPPUNIT_ASSERT(pIPClient);
    comphelper::ScopeGuard aLeave([pIPClient] {
        pIPClient->DeactivateObject();
        Scheduler::ProcessEventsToIdle();
    });
    const Point aCorner = o3tl::convert(pIPClient->GetObjArea().TopLeft(), o3tl::Length::mm100,
                                        o3tl::Length::twip);
    const Point aB2(aCorner.X() + 1280 + 640, aCorner.Y() + 256 + 128);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    {
        tools::JsonWriter aWriter;
        pXImpressDocument->pushVectorPrimitivesDelta(aWriter, std::string_view(aPartId), 0);
        aWriter.finishAndGetAsOString();
    }

    // When 777 is typed into the cell, which is not put into the cell yet:
    typeText(*pXImpressDocument, "777");
    CPPUNIT_ASSERT_EQUAL(u"245000"_ustr,
                         firstSheetOf(*pSheet)->getCellByPosition(1, 1)->getFormula());

    // Then the slide sends the spreadsheet again, with the text being typed and the cells around
    // it. Without the fix, the slide kept the picture of the spreadsheet from before it was
    // active.
    tools::JsonWriter aWriter;
    pXImpressDocument->pushVectorPrimitivesDelta(aWriter, std::string_view(aPartId), 0);
    const OString aDelta = aWriter.finishAndGetAsOString();
    const OString aId = OString::number(sal_Int64(pSheet->GetUniqueID()));
    CPPUNIT_ASSERT(aDelta.indexOf(rtl::Concat2View("\"id\": " + aId)) >= 0);
    CPPUNIT_ASSERT(aDelta.indexOf("\"text\": \"777\"") >= 0);
    CPPUNIT_ASSERT(aDelta.indexOf("\"text\": \"Revenue\"") >= 0);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testEmbeddedSpreadsheetSecondView)
{
    // Given a slide shown in two views, with the spreadsheet active in place in the first one:
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    comphelper::ScopeGuard aResetFlags([] { comphelper::COKit::resetCompatFlag(); });
    SdXImpressDocument* pXImpressDocument = createDoc("embedded-spreadsheet.odp");
    sd::ViewShell* pViewShell1 = pXImpressDocument->GetDocShell()->GetViewShell();
    const int nView1 = KitHelper::getCurrentView();
    SdTestViewCallback aView1;
    const int nView2 = KitHelper::createView();
    SfxViewShell* pViewShell2 = SfxViewShell::Current();
    SdTestViewCallback aView2;
    KitHelper::setView(nView1);
    activateRevenueSheet(*pViewShell1);
    SfxInPlaceClient* pIPClient = pViewShell1->GetViewShell()->GetIPClient();
    CPPUNIT_ASSERT(pIPClient);
    comphelper::ScopeGuard aLeave([pIPClient, nView1] {
        KitHelper::setView(nView1);
        pIPClient->DeactivateObject();
        Scheduler::ProcessEventsToIdle();
    });
    const tools::Rectangle aObject = o3tl::convert(pIPClient->GetObjArea(), o3tl::Length::mm100,
                                                   o3tl::Length::twip);
    const Point aB2(aObject.Left() + 1280 + 640, aObject.Top() + 256 + 128);

    // When the first view types into B2, its view current as for any event of its client:
    KitHelper::setView(nView1);
    aView2.m_aCallbacks.clear();
    aView2.m_aInvalidations.clear();
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aObject.Left() + 1280),
                                 double(lastRectangleOf(aView1, COKitCallbackType::CELL_CURSOR).Left()),
                                 1.0);
    typeText(*pXImpressDocument, "7");

    // Then the second view is told to paint the area of the spreadsheet again, as its tiles show
    // the spreadsheet as it is being edited. Without the fix, only the first view was told.
    bool bInvalidated = false;
    for (const tools::Rectangle& rInvalidation : aView2.m_aInvalidations)
        bInvalidated = bInvalidated || rInvalidation.Overlaps(aObject);
    CPPUNIT_ASSERT(bInvalidated);
    // And the cell cursor of the first view is not one of the second view.
    for (const std::pair<COKitCallbackType, OString>& rCallback : aView2.m_aCallbacks)
        CPPUNIT_ASSERT(rCallback.first != COKitCallbackType::CELL_CURSOR);

    // When the second view double-clicks the spreadsheet that the first view edits:
    KitHelper::setView(nView2);
    aView2.m_aCallbacks.clear();
    for (int nCount : { 1, 2 })
    {
        pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aB2.X(), aB2.Y(),
                                          nCount, MOUSE_LEFT, 0);
        pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, aB2.X(), aB2.Y(),
                                          nCount, MOUSE_LEFT, 0);
        Scheduler::ProcessEventsToIdle();
    }

    // Then the second view does not select the spreadsheet, which stays with the first view, as a
    // chart that another view edits does.
    for (const std::pair<COKitCallbackType, OString>& rCallback : aView2.m_aCallbacks)
        CPPUNIT_ASSERT(rCallback.first != COKitCallbackType::GRAPHIC_SELECTION
                       || rCallback.second == "EMPTY");
    CPPUNIT_ASSERT(!pViewShell2->GetIPClient());
    CPPUNIT_ASSERT(pIPClient->IsObjectInPlaceActive());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testEmbeddedSpreadsheetTakesViewSettings)
{
    // Given a view of a slide that the client gave a timezone and blocked commands:
    SdXImpressDocument* pXImpressDocument = createDoc("embedded-spreadsheet.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SfxViewShell* pSfxViewShell = pViewShell->GetViewShell();
    const int nView = KitHelper::getCurrentView();
    SdTestViewCallback aView;
    KitHelper::setViewTimezone(nView, true, u"Europe/Budapest"_ustr);
    KitHelper::setBlockedCommandList(nView, ".uno:Bold");

    // When the spreadsheet is active in place in that view:
    activateRevenueSheet(*pViewShell);
    SfxInPlaceClient* pIPClient = pSfxViewShell->GetIPClient();
    CPPUNIT_ASSERT(pIPClient);
    comphelper::ScopeGuard aLeave([pIPClient] {
        pIPClient->DeactivateObject();
        Scheduler::ProcessEventsToIdle();
    });
    SfxViewShell* pCalcViewShell = KitCalcHelper(pSfxViewShell).GetCalcViewShell();
    CPPUNIT_ASSERT(pCalcViewShell);

    // Then the view of the spreadsheet has the same settings. Without the fix, it had only the
    // language and the locale.
    CPPUNIT_ASSERT_EQUAL(u"Europe/Budapest"_ustr, pCalcViewShell->GetKitTimezone().second);
    CPPUNIT_ASSERT(pCalcViewShell->isBlockedCommand(u".uno:Bold"_ustr));
    CPPUNIT_ASSERT(!pCalcViewShell->IsKitReadOnlyView());

    // And a setting the client changes while the spreadsheet is active reaches it too.
    KitHelper::setViewReadOnly(nView, true);
    CPPUNIT_ASSERT(pCalcViewShell->IsKitReadOnlyView());
    KitHelper::setAllowChangeComments(nView, true);
    CPPUNIT_ASSERT(pCalcViewShell->IsAllowChangeComments());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testEmbeddedSpreadsheetStretched)
{
    // Given a spreadsheet object that is drawn twice as wide as the area of the sheet it shows,
    // active in place:
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    comphelper::ScopeGuard aResetFlags([] { comphelper::COKit::resetCompatFlag(); });
    SdXImpressDocument* pXImpressDocument = createDoc("embedded-spreadsheet.odp");
    SdTestViewCallback aView;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrOle2Obj* pSheet = revenueSheetOf(*pViewShell);
    tools::Rectangle aWide(pSheet->GetLogicRect());
    aWide.setWidth(aWide.GetWidth() * 2);
    pSheet->SetLogicRect(aWide);
    pViewShell->ActivateObject(pSheet, embed::EmbedVerbs::MS_OLEVERB_PRIMARY);
    Scheduler::ProcessEventsToIdle();
    SfxInPlaceClient* pIPClient = pViewShell->GetViewShell()->GetIPClient();
    CPPUNIT_ASSERT(pIPClient);
    comphelper::ScopeGuard aLeave([pIPClient] {
        pIPClient->DeactivateObject();
        Scheduler::ProcessEventsToIdle();
    });
    const double fScaleX = pIPClient->GetScaleWidth();
    CPPUNIT_ASSERT(std::abs(fScaleX - 1.0) > 0.1);

    // When B2 is clicked in its middle, where the object draws it:
    const Point aCorner = o3tl::convert(pIPClient->GetObjArea().TopLeft(), o3tl::Length::mm100,
                                        o3tl::Length::twip);
    const Point aB2(aCorner.X() + std::round((1280 + 640) * fScaleX), aCorner.Y() + 256 + 128);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, aB2.X(), aB2.Y(), 1,
                                      MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // Then the cell cursor is on B2, drawn as wide as the object draws the cell. The cursor of a
    // cell is a twip narrower than its column.
    const tools::Rectangle aCursor = lastRectangleOf(aView, COKitCallbackType::CELL_CURSOR);
    CPPUNIT_ASSERT(aCursor.Contains(aB2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1279 * fScaleX, double(aCursor.GetWidth()), 2.0);
}

namespace
{
/// The link of the one presentation linked into rDoc, or nothing when none holds.
const sd::SlideLinkRecord* findPresentationLink(SdDrawDocument& rDoc)
{
    for (const sd::SlideLinkRecord& rRecord : rDoc.GetSlideLinkManager().GetLinks())
    {
        if (rRecord.meKind == sd::SlideLinkKind::Presentation)
            return rDoc.GetSlideLinkManager().Find(sd::SlideLinkKind::Presentation, rRecord.maId);
    }
    return nullptr;
}

/// Inserts every slide of the file at rSourceUrl before slide nAt, or at the end for -1, as the
/// whole presentation "Q3 deck.odp" linked to it, and returns the identifier of the link.
OUString insertLinkedPresentation(SdXImpressDocument& rXImpressDocument, const OUString& rSourceUrl,
                                  sal_Int32 nAt = -1)
{
    CPPUNIT_ASSERT(rXImpressDocument.insertPagesFromFile(
        rSourceUrl, "{\"at\":" + OString::number(nAt)
                        + ",\"link\":true,\"source\":\"Q3 deck.odp\","
                          "\"lastModifiedTime\":\"2026-10-20T09:15:00Z\","
                          "\"as\":\"presentation\",\"sourceName\":\"Q3 deck.odp\"}"));
    const sd::SlideLinkRecord* pLink = findPresentationLink(*rXImpressDocument.GetDoc());
    CPPUNIT_ASSERT(pLink);
    return pLink->maId;
}

/// Makes slide nSlide of the document xComponent the current one and runs the command rCommand
/// on it.
void dispatchOnSlide(const uno::Reference<lang::XComponent>& xComponent, sal_Int32 nSlide,
                     const OUString& rCommand)
{
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(xComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->setPart(nSlide);
    Scheduler::ProcessEventsToIdle();
    unotest::MacrosTest::dispatchCommand(xComponent, rCommand, {});
}
}

// A presentation inserted whole goes in as the sections of the file, with the slides outside any
// section in a section named after the file, under one link over them all, one per section the
// file identifies and one per slide, and the whole insert is one undo step
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLinkPresentation)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The four slides of the file go in between the two slides of the document.
    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"), 1);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"TargetOne"_ustr, pDoc->GetSdPage(0, PageKind::Standard)->GetName());
    CPPUNIT_ASSERT_EQUAL(u"Intro text"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(u"TargetTwo"_ustr, pDoc->GetSdPage(5, PageKind::Standard)->GetName());

    // The slide of the file in no section sits in a section named after the file, then come the
    // sections of the file, and the slide after the presentation keeps a section apart.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Closing"_ustr, sectionStartingAt(*pDoc, 4));
    CPPUNIT_ASSERT_EQUAL(SdResId(STR_DEFAULT_SLIDE_SECTION_NAME), sectionStartingAt(*pDoc, 5));

    // The section named after the file is linked to the slides of the file outside any section,
    // which it names by no identifier.
    const sd::SlideLinkRecord* pLoose = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLoose);
    CPPUNIT_ASSERT_EQUAL(OUString(), pLoose->maSourceId);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLoose->maName);
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 1));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 2));

    const sd::SlideLinkRecord* pLink
        = pDoc->GetSlideLinkManager().Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLink->maSource);
    CPPUNIT_ASSERT_EQUAL(OUString(), pLink->maSourceId);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLink->maName);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-20T09:15:00Z"_ustr, pLink->maLastModifiedTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pLink->maSlides.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pLink->maSections.size());
    CPPUNIT_ASSERT_EQUAL(rSections.GetSection(0).maId, pLink->maSections[0]);
    CPPUNIT_ASSERT_EQUAL(rSections.GetSection(2).maId, pLink->maSections[2]);

    // The presentation comes first, then its sections and slides in document order, each under
    // the innermost link that spans it.
    const std::vector<boost::property_tree::ptree> aLinks = readLinkList(*pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), aLinks.size());
    const std::string aPresentation(OUStringToOString(aPresentationId, RTL_TEXTENCODING_ASCII_US));
    CPPUNIT_ASSERT_EQUAL(std::string("presentation"), aLinks[0].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(aPresentation, aLinks[0].get<std::string>("id"));
    CPPUNIT_ASSERT_EQUAL(std::string(), aLinks[0].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string(), aLinks[0].get<std::string>("sourceId"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aLinks[0].get_child("slides").size());
    CPPUNIT_ASSERT_EQUAL(std::string("section"), aLinks[1].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(std::string("Q3 deck.odp"), aLinks[1].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(std::string(), aLinks[1].get<std::string>("sourceId"));
    CPPUNIT_ASSERT_EQUAL(aPresentation, aLinks[1].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("slide"), aLinks[2].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(aLinks[1].get<std::string>("id"), aLinks[2].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("section"), aLinks[3].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(std::string("Opening"), aLinks[3].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(aPresentation, aLinks[3].get<std::string>("parent"));
    const std::string aOpening = aLinks[3].get<std::string>("id");
    CPPUNIT_ASSERT_EQUAL(std::string("slide"), aLinks[4].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(aOpening, aLinks[4].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(aOpening, aLinks[5].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("section"), aLinks[6].get<std::string>("unit"));
    CPPUNIT_ASSERT_EQUAL(std::string("Closing"), aLinks[6].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(aPresentation, aLinks[6].get<std::string>("parent"));
    CPPUNIT_ASSERT_EQUAL(aLinks[6].get<std::string>("id"), aLinks[7].get<std::string>("parent"));

    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSectionCount());
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rSections.GetSectionCount());
    CPPUNIT_ASSERT(
        pDoc->GetSlideLinkManager().Find(sd::SlideLinkKind::Presentation, aPresentationId));
}

// A presentation inserted whole without a link goes in as the sections of the file and nothing
// is linked
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportPresentationWithoutLink)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"),
        "{\"as\":\"presentation\",\"sourceName\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    const sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Closing"_ustr, sectionStartingAt(*pDoc, 5));
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    // A whole presentation is every page of the file, so an insert that names pages, positions
    // or a unit of the source inserts nothing.
    CPPUNIT_ASSERT(!pXImpressDocument->insertPagesFromFile(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"),
        "{\"slides\":[0],\"as\":\"presentation\",\"sourceName\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT(!pXImpressDocument->insertPagesFromFile(
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"),
        "{\"as\":\"presentation\",\"sourceId\":\"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
}

// A presentation link ends when a section is added, taken away or moved inside its run, and undo
// gives it back each time, while a renamed section leaves it, and the views hear of each change
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkPresentationEndsOnSectionChange)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();

    // The presentation spans slides 2 to 5: the file's loose slide, then Opening on 3 and 4, then
    // Closing on 5.
    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"));
    const sd::SlideLinkManager& rLinks = pDoc->GetSlideLinkManager();
    const auto bHolds = [&rLinks, &aPresentationId]
    { return rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId) != nullptr; };
    SdTestViewCallback aView;

    // A section added inside Opening splits it.
    dispatchOnSlide(mxComponent, 4, u".uno:AddSlideSection"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rSections.GetSectionCount());
    CPPUNIT_ASSERT(!bHolds());
    CPPUNIT_ASSERT(aView.m_nSlideLinksChanged > 0);
    aView.m_nSlideLinksChanged = 0;
    pUndoManager->Undo();
    CPPUNIT_ASSERT(bHolds());
    CPPUNIT_ASSERT(aView.m_nSlideLinksChanged > 0);

    // A section renamed is the same section.
    rSections.RenameSection(1, u"Mine"_ustr);
    CPPUNIT_ASSERT(bHolds());
    rSections.RenameSection(1, u"Opening"_ustr);

    // Closing taken away merges its slide into Opening.
    dispatchOnSlide(mxComponent, 5, u".uno:RemoveSlideSection"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT(!bHolds());
    pUndoManager->Undo();
    CPPUNIT_ASSERT(bHolds());

    // Closing moved before Opening changes the order of the sections.
    aView.m_nSlideLinksChanged = 0;
    dispatchOnSlide(mxComponent, 5, u".uno:MoveSlideSectionUp"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Closing"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT(!bHolds());
    CPPUNIT_ASSERT(aView.m_nSlideLinksChanged > 0);
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT(bHolds());
}

// A presentation refreshed from its source takes every page and section of the file: a section
// the file still lists is read again under the identifier it has here and follows a rename in
// the source, one the file no longer lists goes, a new one comes in, and the slides outside any
// section stay in the section named after the file. Undo gives everything back, and a section
// renamed here keeps its name
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshPresentation)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    sd::SlideLinkManager& rLinks = pDoc->GetSlideLinkManager();

    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"));
    const OUString aLooseId = rSections.GetSection(0).maId;
    const OUString aOpeningId = rSections.GetSection(1).maId;
    const OUString aClosingId = rSections.GetSection(2).maId;
    const OUString aChangedUrl = m_directories.getURLFromSrc(
        gSlideImportDataDir, u"slide-import-presentation-changed.odp");

    // The changed file holds the loose slide and Opening with one slide, both revised, Opening
    // renamed Kickoff, no Closing, and a new section Wrap-up with two slides.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         pXImpressDocument->refreshSlideLink(
                             u"presentation"_ustr, aPresentationId, u"Q3 deck.odp"_ustr,
                             aChangedUrl, u"2026-11-04T10:00:00Z"_ustr, u"Q3 deck v2.odp"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Intro text, revised"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Opening one, revised"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Wrap one"_ustr, getSlideText(*pDoc, 4));
    CPPUNIT_ASSERT_EQUAL(u"Wrap two"_ustr, getSlideText(*pDoc, 5));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck v2.odp"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(aLooseId, rSections.GetSection(0).maId);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck v2.odp"_ustr, findSectionLink(*pDoc, 0)->maName);
    CPPUNIT_ASSERT_EQUAL(u"Kickoff"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(aOpeningId, rSections.GetSection(1).maId);
    CPPUNIT_ASSERT_EQUAL(u"Wrap-up"_ustr, sectionStartingAt(*pDoc, 4));
    CPPUNIT_ASSERT(rSections.GetSection(2).maId != aClosingId);

    const sd::SlideLinkRecord* pLink
        = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck v2.odp"_ustr, pLink->maName);
    CPPUNIT_ASSERT_EQUAL(u"2026-11-04T10:00:00Z"_ustr, pLink->maLastModifiedTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pLink->maSlides.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pLink->maSections.size());
    CPPUNIT_ASSERT_EQUAL(rSections.GetSection(2).maId, pLink->maSections[2]);

    const sd::SlideLinkRecord* pKickoff = findSectionLink(*pDoc, 1);
    CPPUNIT_ASSERT(pKickoff);
    CPPUNIT_ASSERT_EQUAL(u"{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}"_ustr, pKickoff->maSourceId);
    CPPUNIT_ASSERT_EQUAL(u"Kickoff"_ustr, pKickoff->maName);
    const sd::SlideLinkRecord* pWrapUp = findSectionLink(*pDoc, 2);
    CPPUNIT_ASSERT(pWrapUp);
    CPPUNIT_ASSERT_EQUAL(u"{7A21D2A4-6A4F-4C82-8E7B-A6B39F1E5C0D}"_ustr, pWrapUp->maSourceId);
    CPPUNIT_ASSERT(!rLinks.Find(sd::SlideLinkKind::Section, aClosingId));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), readLinkList(*pXImpressDocument).size());

    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Intro text"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Closing one"_ustr, getSlideText(*pDoc, 5));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Closing"_ustr, sectionStartingAt(*pDoc, 5));
    CPPUNIT_ASSERT_EQUAL(aClosingId, rSections.GetSection(2).maId);
    pLink = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLink->maName);
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), readLinkList(*pXImpressDocument).size());

    // A refresh of the whole source reads the presentation once, with the links under it, and
    // names it as before. A section renamed here keeps its name.
    rSections.RenameSection(1, u"Mine"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr, aChangedUrl,
                                                              u"2026-11-04T10:00:00Z"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Mine"_ustr, sectionStartingAt(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(aOpeningId, rSections.GetSection(1).maId);
    pLink = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLink->maName);
    CPPUNIT_ASSERT_EQUAL(u"Kickoff"_ustr, findSectionLink(*pDoc, 1)->maName);
}

// A section or a slide refreshed inside a linked presentation leaves the presentation linked,
// spanning the fresh slides
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshInsidePresentation)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    sd::SlideLinkManager& rLinks = pDoc->GetSlideLinkManager();

    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"));
    const OUString aOpeningId = rSections.GetSection(1).maId;
    const OUString aChangedUrl = m_directories.getURLFromSrc(
        gSlideImportDataDir, u"slide-import-presentation-changed.odp");

    // Opening alone takes the one slide the file lists for it now.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLink(u"section"_ustr, aOpeningId,
                                                             u"Q3 deck.odp"_ustr, aChangedUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Opening one, revised"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT_EQUAL(u"Kickoff"_ustr, sectionStartingAt(*pDoc, 3));
    const sd::SlideLinkRecord* pLink
        = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pLink->maSlides.size());
    CPPUNIT_ASSERT_EQUAL(getSlideId(*pDoc, 3), pLink->maSlides[1]);

    // The section of the slides outside any section, alone, takes those slides of the file.
    const OUString aLooseId = rSections.GetSection(0).maId;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLink(u"section"_ustr, aLooseId,
                                                             u"Q3 deck.odp"_ustr, aChangedUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT_EQUAL(u"Intro text, revised"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(aLooseId, rSections.GetSection(0).maId);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 2));
    pLink = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(getSlideId(*pDoc, 2), pLink->maSlides[0]);

    // One slide alone is read again in place.
    const OUString aOpenOneId = getSlideId(*pDoc, 3);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLink(u"slide"_ustr, aOpenOneId,
                                                             u"Q3 deck.odp"_ustr, aChangedUrl,
                                                             OUString(), OUString()));
    CPPUNIT_ASSERT(aOpenOneId != getSlideId(*pDoc, 3));
    pLink = rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(getSlideId(*pDoc, 3), pLink->maSlides[1]);

    for (int nStep = 0; nStep < 3; ++nStep)
        pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(u"Intro text"_ustr, getSlideText(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(u"Opening one"_ustr, getSlideText(*pDoc, 3));
    CPPUNIT_ASSERT(rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId));
}

// Unlinking a presentation leaves its sections and slides linked on their own, unlinking it with
// its contents takes every link inside off, and undo gives each back
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreakPresentation)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    sd::SlideLinkManager& rLinks = pDoc->GetSlideLinkManager();
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();

    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"));

    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"presentation"_ustr, aPresentationId,
                                                     /*bDeep=*/false));
    CPPUNIT_ASSERT(!rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 1));
    CPPUNIT_ASSERT(findSlideLink(*pDoc, 2));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), readLinkList(*pXImpressDocument).size());
    CPPUNIT_ASSERT(
        !pXImpressDocument->breakSlideLink(u"presentation"_ustr, aPresentationId, false));

    pUndoManager->Undo();
    CPPUNIT_ASSERT(rLinks.Find(sd::SlideLinkKind::Presentation, aPresentationId));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), readLinkList(*pXImpressDocument).size());

    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"presentation"_ustr, aPresentationId,
                                                     /*bDeep=*/true));
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), readLinkList(*pXImpressDocument).size());

    // A section unlinked on its own is replaced on the next refresh of the presentation like
    // any other section, under a new identifier, the section of the slides outside any section
    // included.
    const OUString aLooseId = rSections.GetSection(0).maId;
    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(u"section"_ustr, aLooseId, false));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         pXImpressDocument->refreshSlideLink(
                             u"presentation"_ustr, aPresentationId, u"Q3 deck.odp"_ustr,
                             m_directories.getURLFromSrc(gSlideImportDataDir,
                                                         u"slide-import-presentation-changed.odp"),
                             OUString(), OUString()));
    CPPUNIT_ASSERT(rSections.GetSection(0).maId != aLooseId);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, sectionStartingAt(*pDoc, 2));
    CPPUNIT_ASSERT(findSectionLink(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), readLinkList(*pXImpressDocument).size());
}

// A presentation link is saved with the slides it spans and read back over the sections that
// hold them, under the identifier it had
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkPresentationRoundtripODP)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    const OUString aPresentationId = insertLinkedPresentation(
        *pXImpressDocument,
        m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-presentation.odp"));
    const std::vector<OUString> aSections
        = pXImpressDocument->GetDoc()
              ->GetSlideLinkManager()
              .Find(sd::SlideLinkKind::Presentation, aPresentationId)
              ->maSections;

    saveAndReload(TestFilter::ODP);
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const sd::SlideLinkRecord* pLink
        = pDoc->GetSlideLinkManager().Find(sd::SlideLinkKind::Presentation, aPresentationId);
    CPPUNIT_ASSERT(pLink);
    CPPUNIT_ASSERT_EQUAL(u"Q3 deck.odp"_ustr, pLink->maName);
    CPPUNIT_ASSERT_EQUAL(u"2026-10-20T09:15:00Z"_ustr, pLink->maLastModifiedTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pLink->maSlides.size());
    CPPUNIT_ASSERT(bool(aSections == pLink->maSections));
    const sd::SlideLinkRecord* pLoose = findSectionLink(*pDoc, 0);
    CPPUNIT_ASSERT(pLoose);
    CPPUNIT_ASSERT_EQUAL(OUString(), pLoose->maSourceId);
    const std::vector<boost::property_tree::ptree> aLinks = readLinkList(*pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), aLinks.size());
    CPPUNIT_ASSERT_EQUAL(std::string("presentation"), aLinks[0].get<std::string>("unit"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
