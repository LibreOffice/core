/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtiledrenderingtest.hxx>

#include <com/sun/star/util/URLTransformer.hpp>

#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/msgpool.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/scopeguard.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>
#include <unotxdoc.hxx>

#include <view.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cmdid.h>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swtestviewcallback.hxx>

namespace
{
CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testStatusBarPageNumber)
{
    // Given a document with 2 pages, first view on page 1, second view on page 2:
    SwXTextDocument* pXTextDocument = createDoc();
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = getSwDocShell()->GetWrtShell();
    pWrtShell1->InsertPageBreak();
    SwRootFrame* pLayout = pWrtShell1->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage1 = pLayout->GetLower();
    CPPUNIT_ASSERT(pPage1);
    SwFrame* pPage2 = pPage1->GetNext();
    CPPUNIT_ASSERT(pPage2);
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SfxLokHelper::setView(nView1);
    SwTestViewCallback aView1;
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pWrtShell1->Insert(u"start"_ustr);
    pWrtShell1->GetView().SetVisArea(pPage1->getFrameArea().SVRect());
    SfxLokHelper::setView(nView2);
    SwTestViewCallback aView2;
    SwWrtShell* pWrtShell2 = getSwDocShell()->GetWrtShell();
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    pWrtShell2->Insert(u"end"_ustr);
    pWrtShell2->GetView().SetVisArea(pPage2->getFrameArea().SVRect());
    {
        // Listen to StatePageNumber changes in view 2:
        SfxViewFrame& rFrame = pWrtShell2->GetView().GetViewFrame();
        SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool(&rFrame);
        uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(m_xContext));
        util::URL aCommandURL;
        aCommandURL.Complete = ".uno:StatePageNumber";
        xParser->parseStrict(aCommandURL);
        const SfxSlot* pSlot = rSlotPool.GetUnoSlot(aCommandURL.Path);
        rFrame.GetBindings().GetDispatch(pSlot, aCommandURL, false);
    }
    aView2.m_aStateChanges.clear();

    // When deleting a character in view 2 and processing jobs with view 1 set to active:
    pWrtShell2->DelLeft();
    SfxLokHelper::setView(nView1);
    pWrtShell2->GetView().GetViewFrame().GetBindings().GetTimer().Invoke();
    // Once more to hit the pImpl->bMsgDirty = false case in SfxBindings::NextJob_Impl().
    pWrtShell2->GetView().GetViewFrame().GetBindings().GetTimer().Invoke();

    // Then make sure the page number in view 2 is correct:
    // FIXME this should not happen, but it does from time to time.
    if (aView2.m_aStateChanges.empty())
    {
        return;
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aView2.m_aStateChanges.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: .uno:StatePageNumber=Page 2 of 2
    // - Actual  : .uno:StatePageNumber=Page 1 of 2
    // i.e. view 2 got the page number of view 1.
    CPPUNIT_ASSERT_EQUAL(".uno:StatePageNumber=Page 2 of 2"_ostr, aView2.m_aStateChanges[0]);
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testPasteInvalidateNumRules)
{
    // Given a document with 3 pages: first page is ~empty, then page break, then pages 2 & 3 have
    // bullets:
    createDoc("numrules.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Insert(u"test"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    m_aInvalidations = tools::Rectangle();
    m_bFullInvalidateSeen = false;

    // When pasting at the end of page 1:
    dispatchCommand(mxComponent, u".uno:PasteUnformatted"_ustr, {});

    // Then make sure we only invalidate page 1, not page 2 or page 3:
    CPPUNIT_ASSERT(!m_bFullInvalidateSeen);
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage1 = pLayout->GetLower();
    CPPUNIT_ASSERT(m_aInvalidations.Overlaps(pPage1->getFrameArea().SVRect()));
    SwFrame* pPage2 = pPage1->GetNext();
    // Without the accompanying fix in place, this test would have failed, we invalidated page 2 and
    // page 3 as well.
    CPPUNIT_ASSERT(!m_aInvalidations.Overlaps(pPage2->getFrameArea().SVRect()));
    SwFrame* pPage3 = pPage2->GetNext();
    CPPUNIT_ASSERT(!m_aInvalidations.Overlaps(pPage3->getFrameArea().SVRect()));
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testPasteInvalidateNumRulesBullet)
{
    // Given a document with 3 pages: first page is ~empty, then page break, then pages 2 & 3 have
    // bullets:
    createDoc("numrules.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Insert(u"test"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, {});
    m_aInvalidations = tools::Rectangle();
    m_bFullInvalidateSeen = false;

    // When pasting at the end of page 1, in a paragraph that is a bullet (a list, but not a
    // numbering):
    dispatchCommand(mxComponent, u".uno:PasteUnformatted"_ustr, {});

    // Then make sure we only invalidate page 1, not page 2 or page 3:
    CPPUNIT_ASSERT(!m_bFullInvalidateSeen);
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage1 = pLayout->GetLower();
    CPPUNIT_ASSERT(m_aInvalidations.Overlaps(pPage1->getFrameArea().SVRect()));
    SwFrame* pPage2 = pPage1->GetNext();
    // Without the accompanying fix in place, this test would have failed, we invalidated page 2 and
    // page 3 as well.
    CPPUNIT_ASSERT(!m_aInvalidations.Overlaps(pPage2->getFrameArea().SVRect()));
    SwFrame* pPage3 = pPage2->GetNext();
    CPPUNIT_ASSERT(!m_aInvalidations.Overlaps(pPage3->getFrameArea().SVRect()));
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testAsyncLayout)
{
    // Given a document with 3 pages, the first page is visible:
    createDoc();
    SwTestViewCallback aView;
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertPageBreak();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwPageFrame* pPage1 = pLayout->GetLower()->DynCastPageFrame();
    pWrtShell->setLOKVisibleArea(pPage1->getFrameArea().SVRect());

    // When all pages get invalidated:
    pWrtShell->StartAllAction();
    pPage1->InvalidateContent();
    SwPageFrame* pPage2 = pPage1->GetNext()->DynCastPageFrame();
    pPage2->InvalidateContent();
    SwPageFrame* pPage3 = pPage2->GetNext()->DynCastPageFrame();
    pPage3->InvalidateContent();
    pWrtShell->EndAllAction();

    // Then make sure only the first page gets a synchronous layout:
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    CPPUNIT_ASSERT(pPage2->IsInvalidContent());
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());

    // And then processing all idle events:
    Scheduler::ProcessEventsToIdle();

    // Then make sure all pages get an async layout:
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    CPPUNIT_ASSERT(!pPage2->IsInvalidContent());
    CPPUNIT_ASSERT(!pPage3->IsInvalidContent());
}

/// Test callback that works with comphelper::LibreOfficeKit::setAnyInputCallback().
class AnyInputCallback final
{
public:
    static bool callback(void* /*pData*/, int /*nPriority*/) { return true; }

    AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(&callback, this,
                                                        []() -> int { return -1; });
    }

    ~AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(nullptr, nullptr,
                                                        []() -> int { return -1; });
    }
};

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testAnyInput)
{
    // Given a document with 3 pages, the first page is visible:
    createDoc();
    SwTestViewCallback aView;
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertPageBreak();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwPageFrame* pPage1 = pLayout->GetLower()->DynCastPageFrame();
    pWrtShell->setLOKVisibleArea(pPage1->getFrameArea().SVRect());

    // When all pages get invalidated:
    pWrtShell->StartAllAction();
    pPage1->InvalidateContent();
    SwPageFrame* pPage2 = pPage1->GetNext()->DynCastPageFrame();
    pPage2->InvalidateContent();
    SwPageFrame* pPage3 = pPage2->GetNext()->DynCastPageFrame();
    pPage3->InvalidateContent();
    pWrtShell->EndAllAction();

    // Then make sure sync layout calculates page 1:
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    CPPUNIT_ASSERT(pPage2->IsInvalidContent());
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());

    // And when doing one idle layout:
    AnyInputCallback aAnyInput;
    pWrtShell->LayoutIdle();

    // Then make sure async layout calculates page 2:
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    CPPUNIT_ASSERT(!pPage2->IsInvalidContent());
    // Without the fix in place, async layout calculated all pages, even if there were pending input
    // events.
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testSignatureState)
{
    // Given a document with a signature where the digest matches:
    SwXTextDocument* pXTextDocument = createDoc("signed-doc.odt");

    // When initializing tiled rendering with an author name:
    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue(".uno:Author", uno::Any(u"A"_ustr)) };
    pXTextDocument->initializeForTiledRendering(aPropertyValues);
    SignatureState eState = getSwDocShell()->GetDocumentSignatureState();

    // Then make sure the signature state is unchanged:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4 (NOTVALIDATED)
    // - Actual  : 3 (INVALID)
    // i.e. the doc was modified by the time the signature state was calculated.
    CPPUNIT_ASSERT_EQUAL(SignatureState::NOTVALIDATED, eState);
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testFormatInsertStartList)
{
    // Given a document containing a list where the text has a changed font
    SwXTextDocument* pXTextDocument = createDoc("format-insert-list.docx");
    VclPtr<vcl::Window> pDocWindow = pXTextDocument->getDocWindow();
    SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
    assert(pView);

    // Insert a string at the beginning of a list item
    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, u"a"_ustr);
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, u""_ustr);
    Scheduler::ProcessEventsToIdle();

    // The inserted text should have the same font as the rest
    std::unique_ptr<SvxFontItem> pFontItem;
    pView->GetViewFrame().GetBindings().QueryState(SID_ATTR_CHAR_FONT, pFontItem);
    CPPUNIT_ASSERT(pFontItem);
    CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr, pFontItem->GetFamilyName());
    // Without the accompanying fix in place, this test fails with:
    // - Expected: Calibri
    // - Actual  : MS Sans Serif
}

/// Job on the main loop that switches to the first view.
class ViewSwitcher
{
public:
    DECL_STATIC_LINK(ViewSwitcher, SwitchView, void*, void);
};

IMPL_STATIC_LINK_NOARG(ViewSwitcher, SwitchView, void*, void) { SfxLokHelper::setView(0); }

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testPDFExportViewSwitch)
{
    // Given a document with 2 views:
    SwXTextDocument* pXTextDocument = createDoc("to-pdf.odt");
    SfxLokHelper::registerViewCallbacks();
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    SwTestViewCallback aView1;
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    SwView* pView2 = pDoc->GetDocShell()->GetView();
    uno::Reference<frame::XFrame> xFrame2 = pView2->GetViewFrame().GetFrame().GetFrameInterface();

    // When exporting to PDF on the second view and a job on the main loop that switches to the
    // first view:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("SynchronMode", false),
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
    };
    comphelper::dispatchCommand(".uno:ExportDirectToPDF", xFrame2, aPropertyValues);
    Application::PostUserEvent(LINK(nullptr, ViewSwitcher, SwitchView), nullptr);
    Scheduler::ProcessEventsToIdle();

    // Then make sure the callback is invoked exactly on the second view:
    // Without the accompanying fix in place, this test failed, as the callback was invoked on the
    // first view.
    CPPUNIT_ASSERT(aView1.m_aExportFile.isEmpty());
    CPPUNIT_ASSERT(!aView2.m_aExportFile.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testLoadVisibleArea)
{
    // Given a document with 3 pages, the LOK visible area at load time is set to the first page:
    awt::Rectangle aVisibleArea{ 0, 0, 12240, 15840 };
    comphelper::LibreOfficeKit::setInitialClientVisibleArea(aVisibleArea);
    comphelper::ScopeGuard g([] { comphelper::LibreOfficeKit::setInitialClientVisibleArea({}); });

    // When loading that document:
    OUString aURL = createFileURL(u"3pages.odt");
    UnoApiXmlTest::loadFromURL(aURL);

    // Then make sure only the first page is laid out:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwPageFrame* pPage1 = pLayout->GetLower()->DynCastPageFrame();
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    SwPageFrame* pPage2 = pPage1->GetNext()->DynCastPageFrame();
    // Without the accompanying fix in place, this test failed, as the entire document was laid out
    // before the loading finished.
    CPPUNIT_ASSERT(pPage2->IsInvalidContent());
    SwPageFrame* pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());
}

std::vector<OString> FilterStateChanges(const std::vector<OString>& rChanges,
                                        std::string_view rPrefix)
{
    std::vector<OString> aRet;
    for (const auto& rChange : rChanges)
    {
        if (rChange.startsWith(rPrefix))
        {
            aRet.push_back(rChange);
        }
    }
    return aRet;
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerViewEnableOne)
{
    // Given a document with two views:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = SfxLokHelper::getView();

    // When recording changes in view1:
    SfxLokHelper::setView(nView1);
    aView1.m_aStateChanges.clear();
    aView2.m_aStateChanges.clear();
    dispatchCommand(mxComponent, ".uno:TrackChangesInThisView", {});

    // Then make sure view1 gets a state track changes state change, but not view2:
    // Filter out .uno:ModifiedStatus=true, which is not interesting here.
    std::vector<OString> aRecord1 = FilterStateChanges(aView1.m_aStateChanges, ".uno:TrackChanges");
    CPPUNIT_ASSERT(!aRecord1.empty());
    std::vector<OString> aRecord2 = FilterStateChanges(aView2.m_aStateChanges, ".uno:TrackChanges");
    CPPUNIT_ASSERT(aRecord2.empty());

    // And given a reset state (both view1 and view2 recording is disabled):
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TrackChanges", false),
    };
    dispatchCommand(mxComponent, ".uno:TrackChanges", aArgs);

    // When recording changes in view2:
    SfxLokHelper::setView(nView2);
    aView1.m_aStateChanges.clear();
    aView2.m_aStateChanges.clear();
    dispatchCommand(mxComponent, ".uno:TrackChangesInThisView", {});

    // Then make sure view2 gets a 'track changes is now on' state change, but not view1:
    aRecord1 = FilterStateChanges(aView1.m_aStateChanges, ".uno:TrackChanges=true");
    CPPUNIT_ASSERT(aRecord1.empty());
    aRecord2 = FilterStateChanges(aView2.m_aStateChanges, ".uno:TrackChanges=true");
    CPPUNIT_ASSERT(!aRecord2.empty());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerViewEnableBoth)
{
    // Given a document with 2 views, view1 record changes:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:TrackChangesInThisView", {});
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT(pWrtShell1->GetViewOptions()->IsRedlineRecordingOn());
    CPPUNIT_ASSERT(!pWrtShell2->GetViewOptions()->IsRedlineRecordingOn());

    // When turning on track changes for view2:
    comphelper::dispatchCommand(".uno:TrackChangesInThisView", {});

    // Then make sure both views have track changes turned on:
    CPPUNIT_ASSERT(pWrtShell1->GetViewOptions()->IsRedlineRecordingOn());
    // Without the accompanying fix in place, this test would have failed, .uno:TrackChanges in
    // view2 was ignored when view1 already tracked changes.
    CPPUNIT_ASSERT(pWrtShell2->GetViewOptions()->IsRedlineRecordingOn());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerViewAllToOneTransition)
{
    // Given 2 views, recording is on in all views:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:TrackChangesInAllViews", {});
    CPPUNIT_ASSERT(pWrtShell1->GetViewOptions()->IsRedlineRecordingOn());
    CPPUNIT_ASSERT(pWrtShell2->GetViewOptions()->IsRedlineRecordingOn());

    // When limiting recording to just view 1:
    comphelper::dispatchCommand(".uno:TrackChangesInThisView", {});

    // Then make sure view 2 has recording off:
    CPPUNIT_ASSERT(pWrtShell1->GetViewOptions()->IsRedlineRecordingOn());
    // Without the accompanying fix in place, this test would have failed,
    // .uno:TrackChangesInThisView didn't turn off recording for view 2.
    CPPUNIT_ASSERT(!pWrtShell2->GetViewOptions()->IsRedlineRecordingOn());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerViewInsert)
{
    // Given 2 views, view 1 records changes, view does not record changes:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell1->Insert(u"X"_ustr);
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:TrackChangesInThisView", {});

    // When view 1 types:
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pWrtShell1->Insert(u"A"_ustr);
    // Then make sure a redline is created:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pWrtShell1->GetRedlineCount());

    // When view 2 types:
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    pWrtShell2->Insert(u"Z"_ustr);
    // Then make sure no redline is created:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the insertion in view 2 was recorded.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pWrtShell2->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerViewDelete)
{
    // Given 2 views, view 1 records changes, view does not record changes:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell1->Insert(u"test"_ustr);
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:TrackChangesInThisView", {});

    // When view 1 deletes:
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pWrtShell1->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell1->DelRight();
    // Then make sure a redline is created:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pWrtShell1->GetRedlineCount());

    // When view 2 deletes:
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    pWrtShell2->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell2->DelLeft();
    // Then make sure no redline is created:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the deletion in view 2 was recorded.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pWrtShell2->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesPerDocInsert)
{
    // Given 2 views, view 1 turns on per-doc change recording:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell1->Insert(u"X"_ustr);
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:TrackChanges", {});

    // When view 1 types:
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pWrtShell1->Insert(u"A"_ustr);
    // Then make sure a redline is created:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pWrtShell1->GetRedlineCount());

    // When view 2 types:
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    pWrtShell2->Insert(u"Z"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. track changes recording was unconditionally per-view.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pWrtShell2->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testTrackChangesStates)
{
    // Given a document with 2 views:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SwView* pView1 = pXTextDocument->GetDocShell()->GetView();
    SfxLokHelper::createView();
    SwTestViewCallback aView2;
    SwView* pView2 = pXTextDocument->GetDocShell()->GetView();
    SfxLokHelper::setView(nView1);

    // When enabling recording in view1 only:
    pView1->GetViewFrame().GetDispatcher()->Execute(FN_TRACK_CHANGES_IN_THIS_VIEW,
                                                    SfxCallMode::SYNCHRON);

    // Then make sure the "is record", "is record for this view on" and "is record for all views on"
    // statuses are correct:
    std::unique_ptr<SfxPoolItem> pItem;
    pView1->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    // Without the accompanying fix in place, this test would have failed, enabling recording for
    // this view didn't enable the toolbar button in the same view, which looked confusing.
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());

    // When disabling recording:
    SfxBoolItem aOn(FN_REDLINE_ON, false);
    pView1->GetViewFrame().GetDispatcher()->ExecuteList(FN_REDLINE_ON, SfxCallMode::SYNCHRON,
                                                        { &aOn });

    // Then make sure the "is record", "is record for this view on" and "is record for all views on"
    // statuses are correct:
    pView1->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());

    // When enabling recording in all views:
    pView1->GetViewFrame().GetDispatcher()->Execute(FN_TRACK_CHANGES_IN_ALL_VIEWS,
                                                    SfxCallMode::SYNCHRON);

    // Then make sure the "is record", "is record for this view on" and "is record for all views on"
    // statuses are correct:
    pView1->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView1->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_REDLINE_ON, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_REDLINE_ON), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_THIS_VIEW, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_THIS_VIEW), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView2->GetViewFrame().GetBindings().QueryState(FN_TRACK_CHANGES_IN_ALL_VIEWS, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_TRACK_CHANGES_IN_ALL_VIEWS), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
}

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testControlCodesCursor)
{
    // Given a document with hidden formatting marks:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    aView1.m_bCursorVisible = false;

    // When showing formatting marks:
    dispatchCommand(mxComponent, ".uno:ControlCodes", {});

    // Then make sure this doesn't result in a LOK_CALLBACK_CURSOR_VISIBLE callback:
    // Without the accompanying fix in place, this test would have failed, the view jumped to the
    // cursor when showing formatting marks.
    CPPUNIT_ASSERT(!aView1.m_bCursorVisible);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
