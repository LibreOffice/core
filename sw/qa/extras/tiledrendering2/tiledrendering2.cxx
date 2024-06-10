/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <string>

#include <com/sun/star/util/URLTransformer.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <sfx2/msgpool.hxx>

#include <wrtsh.hxx>
#include <view.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <swmodule.hxx>
#include <swdll.hxx>

namespace
{
/// Writer tests with comphelper::LibreOfficeKit::isActive() enabled, part 2.
class SwTiledRenderingTest : public SwModelTestBase
{
public:
    SwTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    SwXTextDocument* createDoc();
};

SwTiledRenderingTest::SwTiledRenderingTest()
    : SwModelTestBase(u"/sw/qa/extras/tiledrendering2/data/"_ustr)
{
}

void SwTiledRenderingTest::setUp()
{
    SwModelTestBase::setUp();
    SwGlobals::ensure();
    SW_MOD()->ClearRedlineAuthors();
    comphelper::LibreOfficeKit::setActive(true);
}

void SwTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }
    comphelper::LibreOfficeKit::setActive(false);
    test::BootstrapFixture::tearDown();
}

SwXTextDocument* SwTiledRenderingTest::createDoc()
{
    createSwDoc();
    auto pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDocument);
    pTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pTextDocument;
}

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    std::vector<OString> m_aStateChanges;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback()
        : m_callbackWrapper(&callback, this)
    {
        mpViewShell = SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId(mnView);
    }

    ~ViewCallback()
    {
        SfxLokHelper::setView(mnView);
        mpViewShell->setLibreOfficeKitViewCallback(nullptr);
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        switch (nType)
        {
            case LOK_CALLBACK_STATE_CHANGED:
            {
                m_aStateChanges.push_back(pPayload);
                break;
            }
        }
    }
};

CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testStatusBarPageNumber)
{
    // Given a document with 2 pages, first view on page 1, second view on page 2:
    SwXTextDocument* pXTextDocument = createDoc();
    int nView1 = SfxLokHelper::getView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
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
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
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
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aView2.m_aStateChanges.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: .uno:StatePageNumber=Page 2 of 2
    // - Actual  : .uno:StatePageNumber=Page 1 of 2
    // i.e. view 2 got the page number of view 1.
    CPPUNIT_ASSERT_EQUAL(".uno:StatePageNumber=Page 2 of 2"_ostr, aView2.m_aStateChanges[0]);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
