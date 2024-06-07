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

#include "ToolBarModule.hxx"
#include "ViewShell.hxx"
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <DrawController.hxx>
#include <EventMultiplexer.hxx>
#include <comphelper/servicehelper.hxx>
#include <framework/FrameworkHelper.hxx>
#include <vcl/EnumContext.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace {
    const sal_Int32 gnConfigurationUpdateStartEvent(0);
    const sal_Int32 gnConfigurationUpdateEndEvent(1);
    const sal_Int32 gnResourceActivationRequestEvent(2);
    const sal_Int32 gnResourceDeactivationRequestEvent(3);
}

namespace sd::framework {

//===== ToolBarModule =========================================================

ToolBarModule::ToolBarModule (
    const rtl::Reference<sd::DrawController>& rxController)
    : mpBase(nullptr),
      mbMainViewSwitchUpdatePending(false)
{
    // Tunnel through the controller to obtain a ViewShellBase.
    if (rxController != nullptr)
        mpBase = rxController->GetViewShellBase();

    if (!rxController.is())
        return;

    mxConfigurationController = rxController->getConfigurationController();
    if (!mxConfigurationController.is())
        return;

    mxConfigurationController->addConfigurationChangeListener(
        this,
        FrameworkHelper::msConfigurationUpdateStartEvent,
        Any(gnConfigurationUpdateStartEvent));
    mxConfigurationController->addConfigurationChangeListener(
        this,
        FrameworkHelper::msConfigurationUpdateEndEvent,
        Any(gnConfigurationUpdateEndEvent));
    mxConfigurationController->addConfigurationChangeListener(
        this,
        FrameworkHelper::msResourceActivationRequestEvent,
        Any(gnResourceActivationRequestEvent));
    mxConfigurationController->addConfigurationChangeListener(
        this,
        FrameworkHelper::msResourceDeactivationRequestEvent,
        Any(gnResourceDeactivationRequestEvent));
}

ToolBarModule::~ToolBarModule()
{
    if (mpBase && mbListeningEventMultiplexer)
        mpBase->GetEventMultiplexer()->RemoveEventListener(
            LINK(this, ToolBarModule, EventMultiplexerListener));
}

void ToolBarModule::disposing(std::unique_lock<std::mutex>&)
{
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
}

void SAL_CALL ToolBarModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (!mxConfigurationController.is())
        return;

    // since EventMultiplexer isn't available when the ToolBarModule is
    // initialized, subscribing the event listener hacked here.
    if (!mbListeningEventMultiplexer)
    {
        mpBase->GetEventMultiplexer()->AddEventListener(
            LINK(this, ToolBarModule, EventMultiplexerListener));
        mbListeningEventMultiplexer = true;
    }


    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case gnConfigurationUpdateStartEvent:
            HandleUpdateStart();
            break;

        case gnConfigurationUpdateEndEvent:
            HandleUpdateEnd();
            break;

        case gnResourceActivationRequestEvent:
        case gnResourceDeactivationRequestEvent:
            // Remember the request for the activation or deactivation
            // of the center pane view.  When that happens then on end
            // of the next configuration update the set of visible tool
            // bars will be updated.
            if ( ! mbMainViewSwitchUpdatePending)
                if (rEvent.ResourceId->getResourceURL().match(
                    FrameworkHelper::msViewURLPrefix)
                    && rEvent.ResourceId->isBoundToURL(
                        FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                {
                    mbMainViewSwitchUpdatePending = true;
                }
            break;
    }
}

void ToolBarModule::HandlePaneViewShellFocused(const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId)
{
    std::shared_ptr<FrameworkHelper> pFrameworkHelper (FrameworkHelper::Instance(*mpBase));
    std::shared_ptr<ViewShell> pViewShell = pFrameworkHelper->GetViewShell(pFrameworkHelper->GetView(rxResourceId));

    switch(pViewShell->GetShellType())
    {
        // TODO: refactor this bit into something that doesn't hardcode on the viewshell types.
        // i remember having some trivial ideas on this -> check notes.
        case sd::ViewShell::ST_IMPRESS:
        case sd::ViewShell::ST_DRAW:
        case sd::ViewShell::ST_OUTLINE:
            // mainviewshells.
            mpBase->GetViewShellManager()->RemoveOverridingMainShell();
            break;
        case ViewShell::ST_NOTESPANEL:
            // shells that override mainviewshell functionality.
            mpBase->GetViewShellManager()->SetOverridingMainShell(pViewShell);
            UpdateToolbars(pViewShell.get());
            break;
        default:
            break;
    }
    mpToolBarManagerLock.reset();
}

void ToolBarModule::HandleUpdateStart()
{
    // Lock the ToolBarManager and tell it to lock the ViewShellManager as
    // well.  This way the ToolBarManager can optimize the releasing of
    // locks and arranging of updates of both tool bars and the view shell
    // stack.
    if (mpBase != nullptr)
    {
        std::shared_ptr<ToolBarManager> pToolBarManager (mpBase->GetToolBarManager());
        mpToolBarManagerLock.reset(new ToolBarManager::UpdateLock(pToolBarManager));
        pToolBarManager->LockViewShellManager();
    }
}

void ToolBarModule::HandleUpdateEnd()
{
    if (mbMainViewSwitchUpdatePending)
    {
        mbMainViewSwitchUpdatePending = false;
        // Update the set of visible tool bars and deactivate those that are
        // no longer visible.  This is done before the old view shell is
        // destroyed in order to avoid unnecessary updates of those tool
        // bars.
        std::shared_ptr<ToolBarManager> pToolBarManager (mpBase->GetToolBarManager());
        std::shared_ptr<FrameworkHelper> pFrameworkHelper (
            FrameworkHelper::Instance(*mpBase));
        auto pViewShell
            = pFrameworkHelper->GetViewShell(FrameworkHelper::msCenterPaneURL);

        UpdateToolbars(pViewShell.get());
    }
    // Releasing the update lock of the ToolBarManager  will let the
    // ToolBarManager with the help of the ViewShellManager take care of
    // updating tool bars and view shell with the minimal amount of
    // shell stack modifications and tool bar updates.
    mpToolBarManagerLock.reset();
}

void ToolBarModule::UpdateToolbars(ViewShell* pViewShell)
{
    // Update the set of visible tool bars and deactivate those that are
    // no longer visible.  This is done before the old view shell is
    // destroyed in order to avoid unnecessary updates of those tool
    // bars.
    std::shared_ptr<ToolBarManager> pToolBarManager (mpBase->GetToolBarManager());

    if (pViewShell)
    {
        pToolBarManager->MainViewShellChanged(*pViewShell);
        pToolBarManager->SelectionHasChanged(
            *pViewShell,
            *pViewShell->GetView());
        pToolBarManager->PreUpdate();
    }
    else
    {
        pToolBarManager->MainViewShellChanged();
        pToolBarManager->PreUpdate();
    }
}

void SAL_CALL ToolBarModule::disposing (const lang::EventObject& rEvent)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = nullptr;
        dispose();
    }
}

IMPL_LINK(ToolBarModule, EventMultiplexerListener, sd::tools::EventMultiplexerEvent&, rEvent,
          void)
{
    if (!mpBase)
        return;

    switch(rEvent.meEventId)
    {
        case EventMultiplexerEventId::FocusShifted:
            {
                uno::Reference<drawing::framework::XResourceId> xResourceId{ rEvent.mxUserData,
                                                                             UNO_QUERY };
                if (xResourceId.is())
                    HandlePaneViewShellFocused(xResourceId);
                break;
            }
        default:
            break;
    }
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
