/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ViewTabBar.hxx>

#include <ViewShellBase.hxx>
#include <framework/FrameworkHelper.hxx>
#include <framework/Pane.hxx>
#include <DrawController.hxx>

#include <Client.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/viewfrm.hxx>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace sd {

namespace {
bool IsEqual (const TabBarButton& rButton1, const TabBarButton& rButton2)
{
    return ((rButton1.ResourceId.is()
                && rButton2.ResourceId.is()
                && rButton1.ResourceId->compareTo(rButton2.ResourceId) == 0)
        || rButton1.ButtonLabel == rButton2.ButtonLabel);
}

} // end of anonymous namespace

ViewTabBar::ViewTabBar (
    const Reference<XResourceId>& rxViewTabBarId,
    const Reference<frame::XController>& rxController)
    : ViewTabBarInterfaceBase(maMutex),
      mpTabControl(VclPtr<TabBarControl>::Create(GetAnchorWindow(rxViewTabBarId,rxController), this)),
      mxController(rxController),
      maTabBarButtons(),
      mxViewTabBarId(rxViewTabBarId),
      mpViewShellBase(nullptr),
      mnNoteBookWidthPadding(0)
{
    // Tunnel through the controller and use the ViewShellBase to obtain the
    // view frame.
    try
    {
        Reference<lang::XUnoTunnel> xTunnel (mxController, UNO_QUERY_THROW);
        DrawController* pController = reinterpret_cast<DrawController*>(
            xTunnel->getSomething(DrawController::getUnoTunnelId()));
        mpViewShellBase = pController->GetViewShellBase();
    }
    catch (const RuntimeException&)
    {
    }

    // Register as listener at XConfigurationController.
    Reference<XControllerManager> xControllerManager (mxController, UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                    FrameworkHelper::msResourceActivationEvent,
                Any());
        }
    }

    mpTabControl->Show();

    if (mpViewShellBase != nullptr
        && rxViewTabBarId->isBoundToURL(
            FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
    {
        mpViewShellBase->SetViewTabBar(this);
    }
}

ViewTabBar::~ViewTabBar()
{
}

void ViewTabBar::disposing()
{
    if (mpViewShellBase != nullptr
        && mxViewTabBarId->isBoundToURL(
            FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
    {
        mpViewShellBase->SetViewTabBar(nullptr);
    }

    if (mxConfigurationController.is())
    {
        // Unregister listener from XConfigurationController.
        try
        {
            mxConfigurationController->removeConfigurationChangeListener(this);
        }
        catch (const lang::DisposedException&)
        {
            // Receiving a disposed exception is the normal case.  Is there
            // a way to avoid it?
        }
        mxConfigurationController = nullptr;
    }

    {
        const SolarMutexGuard aSolarGuard;
        mpTabControl.disposeAndClear();
    }

    mxController = nullptr;
}

vcl::Window* ViewTabBar::GetAnchorWindow(
    const Reference<XResourceId>& rxViewTabBarId,
    const Reference<frame::XController>& rxController)
{
    vcl::Window* pWindow = nullptr;
    ViewShellBase* pBase = nullptr;

    // Tunnel through the controller and use the ViewShellBase to obtain the
    // view frame.
    try
    {
        Reference<lang::XUnoTunnel> xTunnel (rxController, UNO_QUERY_THROW);
        DrawController* pController = reinterpret_cast<DrawController*>(
            xTunnel->getSomething(DrawController::getUnoTunnelId()));
        pBase = pController->GetViewShellBase();
    }
    catch (const RuntimeException&)
    {
    }

    // The ViewTabBar supports at the moment only the center pane.
    if (rxViewTabBarId.is()
        && rxViewTabBarId->isBoundToURL(
            FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
    {
        if (pBase != nullptr && pBase->GetViewFrame() != nullptr)
            pWindow = &pBase->GetViewFrame()->GetWindow();
    }

    // The rest is (at the moment) just for the emergency case.
    if (pWindow == nullptr)
    {
        Reference<XPane> xPane;
        try
        {
            Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY_THROW);
            Reference<XConfigurationController> xCC (
                xControllerManager->getConfigurationController());
            if (xCC.is())
                xPane.set(xCC->getResource(rxViewTabBarId->getAnchor()), UNO_QUERY);
        }
        catch (const RuntimeException&)
        {
        }

        // Tunnel through the XWindow to the VCL side.
        try
        {
            Reference<lang::XUnoTunnel> xTunnel (xPane, UNO_QUERY_THROW);
            framework::Pane* pPane = reinterpret_cast<framework::Pane*>(
                xTunnel->getSomething(framework::Pane::getUnoTunnelId()));
            if (pPane != nullptr)
                pWindow = pPane->GetWindow()->GetParent();
        }
        catch (const RuntimeException&)
        {
        }
    }

    return pWindow;
}

//----- XConfigurationChangeListener ------------------------------------------

void SAL_CALL  ViewTabBar::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (rEvent.Type == FrameworkHelper::msResourceActivationEvent
        && rEvent.ResourceId->getResourceURL().match(FrameworkHelper::msViewURLPrefix)
        && rEvent.ResourceId->isBoundTo(mxViewTabBarId->getAnchor(), AnchorBindingMode_DIRECT))
    {
        UpdateActiveButton();
    }
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL ViewTabBar::disposing(
    const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxConfigurationController)
    {
        mxConfigurationController = nullptr;
        mxController = nullptr;
    }
}

//----- XTabBar ---------------------------------------------------------------

void SAL_CALL ViewTabBar::addTabBarButtonAfter (
    const TabBarButton& rButton,
    const TabBarButton& rAnchor)
{
    const SolarMutexGuard aSolarGuard;
    AddTabBarButton(rButton, rAnchor);
}

void SAL_CALL ViewTabBar::appendTabBarButton (const TabBarButton& rButton)
{
    const SolarMutexGuard aSolarGuard;
    AddTabBarButton(rButton);
}

void SAL_CALL ViewTabBar::removeTabBarButton (const TabBarButton& rButton)
{
    const SolarMutexGuard aSolarGuard;
    RemoveTabBarButton(rButton);
}

sal_Bool SAL_CALL ViewTabBar::hasTabBarButton (const TabBarButton& rButton)
{
    const SolarMutexGuard aSolarGuard;
    return HasTabBarButton(rButton);
}

Sequence<TabBarButton> SAL_CALL ViewTabBar::getTabBarButtons()
{
    const SolarMutexGuard aSolarGuard;
    return GetTabBarButtons();
}

//----- XResource -------------------------------------------------------------

Reference<XResourceId> SAL_CALL ViewTabBar::getResourceId()
{
    return mxViewTabBarId;
}

sal_Bool SAL_CALL ViewTabBar::isAnchorOnly()
{
    return false;
}

//----- XUnoTunnel ------------------------------------------------------------

namespace
{
    class theViewTabBarUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theViewTabBarUnoTunnelId > {};
}

const Sequence<sal_Int8>& ViewTabBar::getUnoTunnelId()
{
    return theViewTabBarUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL ViewTabBar::getSomething (const Sequence<sal_Int8>& rId)
{
    sal_Int64 nResult = 0;

    if (isUnoTunnelId<ViewTabBar>(rId))
    {
        nResult = reinterpret_cast<sal_Int64>(this);
    }

    return nResult;
}

bool ViewTabBar::ActivatePage(size_t nIndex)
{
    try
    {
        Reference<XControllerManager> xControllerManager (mxController,UNO_QUERY_THROW);
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if ( ! xConfigurationController.is())
            throw RuntimeException();
        Reference<XView> xView;
        try
        {
            xView.set(xConfigurationController->getResource(
                          ResourceId::create(
                              ::comphelper::getProcessComponentContext(),
                              FrameworkHelper::msCenterPaneURL)),
                      UNO_QUERY);
        }
        catch (const DeploymentException&)
        {
        }

        Client* pIPClient = nullptr;
        if (mpViewShellBase != nullptr)
            pIPClient = dynamic_cast<Client*>(mpViewShellBase->GetIPClient());
        if (pIPClient==nullptr || ! pIPClient->IsObjectInPlaceActive())
        {
            if (nIndex < maTabBarButtons.size())
            {
                xConfigurationController->requestResourceActivation(
                    maTabBarButtons[nIndex].ResourceId,
                    ResourceActivationMode_REPLACE);
            }

            return true;
        }
    }
    catch (const RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd.view");
    }

    return false;
}

int ViewTabBar::GetHeight() const
{
    int nHeight (0);

    if (!maTabBarButtons.empty())
    {
        if (mpTabControl->IsReallyVisible())
        {
            weld::Notebook& rNotebook = mpTabControl->GetNotebook();
            int nAllocatedWidth = mpTabControl->GetAllocatedWidth();
            int nPageWidth = nAllocatedWidth - mnNoteBookWidthPadding;

            // set each page width-request to the size it takes to fit the notebook allocation
            for (int nIndex = 1, nPageCount = rNotebook.get_n_pages(); nIndex <= nPageCount; ++nIndex)
            {
                OString sIdent(OString::number(nIndex));
                weld::Container* pContainer = rNotebook.get_page(sIdent);
                pContainer->set_size_request(nPageWidth, -1);
            }

            // get the height-for-width for this allocation
            nHeight = mpTabControl->get_preferred_size().Height();
        }

        if (nHeight <= 0)
        {
            // Using a default when the real height can not be determined.
            // To get correct height this method should be called when the
            // control is visible.
            nHeight = 21;
        }
    }

    return nHeight;
}

void ViewTabBar::AddTabBarButton (
    const css::drawing::framework::TabBarButton& rButton,
    const css::drawing::framework::TabBarButton& rAnchor)
{
    TabBarButtonList::size_type nIndex;

    if ( ! rAnchor.ResourceId.is()
        || (rAnchor.ResourceId->getResourceURL().isEmpty()
            && rAnchor.ButtonLabel.isEmpty()))
    {
        nIndex = 0;
    }
    else
    {
        for (nIndex=0; nIndex<maTabBarButtons.size(); ++nIndex)
        {
            if (IsEqual(maTabBarButtons[nIndex], rAnchor))
            {
                ++nIndex;
                break;
            }
        }
    }

    AddTabBarButton(rButton,nIndex);
}

void ViewTabBar::AddTabBarButton (
    const css::drawing::framework::TabBarButton& rButton)
{
    AddTabBarButton(rButton, maTabBarButtons.size());
}

void ViewTabBar::AddTabBarButton (
    const css::drawing::framework::TabBarButton& rButton,
    sal_Int32 nPosition)
{
    if (nPosition >= 0 &&
        nPosition <= mpTabControl->GetNotebook().get_n_pages())
    {
        // Insert the button into our local array.
        maTabBarButtons.insert(maTabBarButtons.begin() + nPosition, rButton);
        UpdateTabBarButtons();
        UpdateActiveButton();
    }
}

void ViewTabBar::RemoveTabBarButton (
    const css::drawing::framework::TabBarButton& rButton)
{
    for (TabBarButtonList::size_type nIndex=0; nIndex<maTabBarButtons.size(); ++nIndex)
    {
        if (IsEqual(maTabBarButtons[nIndex], rButton))
        {
            maTabBarButtons.erase(maTabBarButtons.begin()+nIndex);
            UpdateTabBarButtons();
            UpdateActiveButton();
            break;
        }
    }
}

bool ViewTabBar::HasTabBarButton (
    const css::drawing::framework::TabBarButton& rButton)
{
    bool bResult (false);

    for (const css::drawing::framework::TabBarButton & r : maTabBarButtons)
    {
        if (IsEqual(r, rButton))
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

css::uno::Sequence<css::drawing::framework::TabBarButton>
    ViewTabBar::GetTabBarButtons()
{
    sal_uInt32 nCount (maTabBarButtons.size());
    css::uno::Sequence<css::drawing::framework::TabBarButton>
          aList (nCount);

    for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        aList[nIndex] = maTabBarButtons[nIndex];

    return aList;
}

void ViewTabBar::UpdateActiveButton()
{
    Reference<XView> xView;
    if (mpViewShellBase != nullptr)
        xView = FrameworkHelper::Instance(*mpViewShellBase)->GetView(
            mxViewTabBarId->getAnchor());
    if (!xView.is())
        return;

    Reference<XResourceId> xViewId (xView->getResourceId());
    for (size_t nIndex=0; nIndex<maTabBarButtons.size(); ++nIndex)
    {
        if (maTabBarButtons[nIndex].ResourceId->compareTo(xViewId) == 0)
        {
            mpTabControl->GetNotebook().set_current_page(nIndex);
            break;
        }
    }
}

void ViewTabBar::UpdateTabBarButtons()
{
    int nMaxPageWidthReq(0);

    weld::Notebook& rNotebook = mpTabControl->GetNotebook();
    int nPageCount(rNotebook.get_n_pages());
    int nIndex = 1;
    for (const auto& rTab : maTabBarButtons)
    {
        OString sIdent(OString::number(nIndex));
        // Create a new tab when there are not enough.
        if (nPageCount < nIndex)
            rNotebook.append_page(sIdent, rTab.ButtonLabel);
        else
        {
            // Update the tab.
            rNotebook.set_tab_label_text(sIdent, rTab.ButtonLabel);
        }

        // Set a fairly arbitrary initial width request for the pages so we can
        // measure what extra width the notebook itself uses
        weld::Container* pContainer = rNotebook.get_page(sIdent);
        int nTextWidth = pContainer->get_pixel_size(rTab.ButtonLabel).Width();
        pContainer->set_size_request(nTextWidth, -1);
        nMaxPageWidthReq = std::max(nMaxPageWidthReq, nTextWidth);

        ++nIndex;
    }

    // Delete tabs that are no longer used.
    for (; nIndex<=nPageCount; ++nIndex)
        rNotebook.remove_page(OString::number(nIndex));

    int nWidthReq = rNotebook.get_preferred_size().Width();
    // The excess width over the page request that the notebook uses we will
    // use this later to help measure the best height-for-width given the
    // eventual allocated width of the notebook
    mnNoteBookWidthPadding = nWidthReq - nMaxPageWidthReq;
}

//===== TabBarControl =========================================================

TabBarControl::TabBarControl (
    vcl::Window* pParentWindow,
    const ::rtl::Reference<ViewTabBar>& rpViewTabBar)
    : InterimItemWindow(pParentWindow, "modules/simpress/ui/tabviewbar.ui", "TabViewBar")
    , mxTabControl(m_xBuilder->weld_notebook("tabcontrol"))
    , mpViewTabBar(rpViewTabBar)
    , mnAllocatedWidth(0)
{
    // Because the actual window background is transparent--to avoid
    // flickering due to multiple background paintings by this and by child
    // windows--we have to paint the background for this control explicitly:
    // the actual control is not painted over its whole bounding box.
    SetPaintTransparent(false);
    SetBackground(Application::GetSettings().GetStyleSettings().GetDialogColor());

    InitControlBase(mxTabControl.get());

    mxTabControl->connect_enter_page(LINK(this, TabBarControl, ActivatePageHdl));
    mxTabControl->connect_size_allocate(LINK(this, TabBarControl, NotebookSizeAllocHdl));
}

void TabBarControl::dispose()
{
    mxTabControl.reset();
    InterimItemWindow::dispose();
}

TabBarControl::~TabBarControl()
{
    disposeOnce();
}

IMPL_LINK(TabBarControl, NotebookSizeAllocHdl, const Size&, rSize, void)
{
    mnAllocatedWidth = rSize.Width();
}

IMPL_LINK(TabBarControl, ActivatePageHdl, const OString&, rPage, void)
{
    if (!mpViewTabBar->ActivatePage(mxTabControl->get_page_index(rPage)))
    {
        // When we run into this else branch then we have an active OLE
        // object.  We ignore the request to switch views.  Additionally
        // we put the active tab back to the one for the current view.
        mpViewTabBar->UpdateActiveButton();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
