/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;

#define MENUBAR_STR "private:resource/menubar/menubar"

bool SfxNotebookBar::m_bLock = false;
Reference<css::frame::XLayoutManager> SfxNotebookBar::m_xLayoutManager;

void SfxNotebookBar::CloseMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    if (rFrame.GetSystemWindow()->GetNotebookBar())
        rFrame.GetSystemWindow()->CloseNotebookBar();
    m_xLayoutManager.clear();
}

void SfxNotebookBar::CloseMethod(SystemWindow* pSysWindow)
{
    if (pSysWindow && pSysWindow->GetNotebookBar())
        pSysWindow->CloseNotebookBar();
    m_xLayoutManager.clear();
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings)
{
    SvtViewOptions aViewOpt(E_WINDOW, "notebookbar");
    aViewOpt.SetVisible(!aViewOpt.IsVisible());

    // trigger the StateMethod
    rBindings.Invalidate(SID_NOTEBOOKBAR);
    rBindings.Update();
}

void SfxNotebookBar::StateMethod(SfxBindings& rBindings, const OUString& rUIFile)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    StateMethod(rFrame.GetSystemWindow(), rFrame.GetFrameInterface(), rUIFile);
}

void SfxNotebookBar::StateMethod(SystemWindow* pSysWindow,
                                 const Reference<css::frame::XFrame> & xFrame,
                                 const OUString& rUIFile)
{
    assert(pSysWindow);

    if (!m_xLayoutManager.is())
    {
        Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);

        if (xPropSet.is())
        {
            Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= m_xLayoutManager;
        }
    }

    SvtViewOptions aViewOpt(E_WINDOW, "notebookbar");

    if (aViewOpt.IsVisible())
    {
        RemoveListeners(pSysWindow);

        // setup if necessary
        pSysWindow->SetNotebookBar(rUIFile, xFrame);

        pSysWindow->GetNotebookBar()->Show();
        pSysWindow->GetNotebookBar()->SetIconClickHdl(LINK(nullptr, SfxNotebookBar, ToggleMenubar));

        SfxViewFrame* pView = SfxViewFrame::Current();

        if(pView)
        {
            Reference<XContextChangeEventMultiplexer> xMultiplexer
                        = ContextChangeEventMultiplexer::get(
                                ::comphelper::getProcessComponentContext());

            if(xFrame.is() && xMultiplexer.is())
            {
                xMultiplexer->addContextChangeEventListener(
                                    pSysWindow->GetNotebookBar()->getContextChangeEventListener(),
                                    xFrame->getController());
            }
        }
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
        pNotebookBar->Hide();
}

void SfxNotebookBar::RemoveListeners(SystemWindow* pSysWindow)
{
    Reference<XContextChangeEventMultiplexer> xMultiplexer
                        = ContextChangeEventMultiplexer::get(
                                ::comphelper::getProcessComponentContext());

    if (pSysWindow->GetNotebookBar() && xMultiplexer.is())
    {
        xMultiplexer->removeAllContextChangeEventListeners(
                           pSysWindow->GetNotebookBar()->getContextChangeEventListener());
    }
}

IMPL_STATIC_LINK_NOARG_TYPED(SfxNotebookBar, ToggleMenubar, LinkParamNone*, void)
{
    if (m_xLayoutManager.is())
    {
        if (m_xLayoutManager->getElement(MENUBAR_STR).is())
            ShowMenubar(false);
        else
            ShowMenubar(true);
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (!m_bLock && m_xLayoutManager.is())
    {
        m_bLock = true;
        m_xLayoutManager->lock();

        if (m_xLayoutManager->getElement(MENUBAR_STR).is() && !bShow)
            m_xLayoutManager->destroyElement(MENUBAR_STR);
        else if(!m_xLayoutManager->getElement(MENUBAR_STR).is() && bShow)
            m_xLayoutManager->createElement(MENUBAR_STR);

        m_xLayoutManager->unlock();
        m_bLock = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
