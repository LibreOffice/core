/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tiledrenderingmodeltestbase.cxx"

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

#include <view.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>

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
    ViewCallback aView1;
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pWrtShell1->Insert(u"start"_ustr);
    pWrtShell1->GetView().SetVisArea(pPage1->getFrameArea().SVRect());
    SfxLokHelper::setView(nView2);
    ViewCallback aView2;
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
    ViewCallback aView;
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

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testAnyInput)
{
    // Given a document with 3 pages, the first page is visible:
    createDoc();
    ViewCallback aView;
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
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    ViewCallback aView1;
    SfxLokHelper::createView();
    ViewCallback aView2;
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
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
