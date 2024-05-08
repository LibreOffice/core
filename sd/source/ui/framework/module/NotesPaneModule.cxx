/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "NotesPaneModule.hxx"

#include <DrawController.hxx>
#include <DrawViewShell.hxx>
#include <EventMultiplexer.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>

#include <framework/ConfigurationController.hxx>
#include <framework/FrameworkHelper.hxx>
#include <framework/ViewShellWrapper.hxx>

#include <officecfg/Office/Impress.hxx>

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/frame/XController.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace
{
const sal_Int32 ResourceActivationRequestEvent = 0;
const sal_Int32 ResourceDeactivationRequestEvent = 1;
}

namespace sd::framework
{
NotesPaneModule::NotesPaneModule(const rtl::Reference<::sd::DrawController>& rxController)
    : mxBottomImpressPaneId(FrameworkHelper::CreateResourceId(
          FrameworkHelper::msNotesPanelViewURL, FrameworkHelper::msBottomImpressPaneURL))
    , mxMainViewAnchorId(FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL))
{
    if (!rxController.is())
        return;

    mpViewShellBase = rxController->GetViewShellBase();

    mxConfigurationController = rxController->getConfigurationController();
    if (!mxConfigurationController.is())
        return;

    mxConfigurationController->addConfigurationChangeListener(
        this, FrameworkHelper::msResourceActivationRequestEvent,
        Any(ResourceActivationRequestEvent));
    mxConfigurationController->addConfigurationChangeListener(
        this, FrameworkHelper::msResourceDeactivationRequestEvent,
        Any(ResourceDeactivationRequestEvent));

    if (officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::ImpressView::get().value_or(
            false))
        AddActiveMainView(FrameworkHelper::msImpressViewURL);
    if (officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::OutlineView::get().value_or(
            false))
        AddActiveMainView(FrameworkHelper::msOutlineViewURL);
    if (officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::NotesView::get().value_or(
            false))
        AddActiveMainView(FrameworkHelper::msNotesViewURL);
}

NotesPaneModule::~NotesPaneModule()
{
    if (mpViewShellBase && mbListeningEventMultiplexer)
        mpViewShellBase->GetEventMultiplexer()->RemoveEventListener(
            LINK(this, NotesPaneModule, EventMultiplexerListener));
}

void NotesPaneModule::AddActiveMainView(const OUString& rsMainViewURL)
{
    maActiveMainViewContainer.insert(rsMainViewURL);
}

bool NotesPaneModule::IsResourceActive(const OUString& rsMainViewURL)
{
    return maActiveMainViewContainer.contains(rsMainViewURL);
}

void NotesPaneModule::SaveResourceState()
{
    auto xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::ImpressView::set(
        IsResourceActive(FrameworkHelper::msImpressViewURL), xChanges);
    officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::OutlineView::set(
        IsResourceActive(FrameworkHelper::msOutlineViewURL), xChanges);
    officecfg::Office::Impress::MultiPaneGUI::NotesPane::Visible::NotesView::set(
        IsResourceActive(FrameworkHelper::msNotesViewURL), xChanges);
    xChanges->commit();
}

void NotesPaneModule::disposing(std::unique_lock<std::mutex>&)
{
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
}

IMPL_LINK(NotesPaneModule, EventMultiplexerListener, sd::tools::EventMultiplexerEvent&, rEvent,
          void)
{
    if (!mxConfigurationController.is())
        return;

    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::EditModeNormal:
            mbInMasterEditMode = false;
            if (IsResourceActive(msCurrentMainViewURL))
            {
                mxConfigurationController->requestResourceActivation(
                    mxBottomImpressPaneId->getAnchor(), ResourceActivationMode_ADD);
                mxConfigurationController->requestResourceActivation(
                    mxBottomImpressPaneId, ResourceActivationMode_REPLACE);
            }
            else
            {
                mxConfigurationController->requestResourceDeactivation(mxBottomImpressPaneId);
            }
            break;
        case EventMultiplexerEventId::EditModeMaster:
            mbInMasterEditMode = true;
            mxConfigurationController->requestResourceDeactivation(mxBottomImpressPaneId);
            break;
        default:
            break;
    }
}

void SAL_CALL NotesPaneModule::notifyConfigurationChange(const ConfigurationChangeEvent& rEvent)
{
    if (!mxConfigurationController.is())
        return;

    // the late init is hacked here since there's EventMultiplexer isn't available when the
    // NotesPaneModule is constructed
    if (!mbListeningEventMultiplexer)
    {
        mpViewShellBase->GetEventMultiplexer()->AddEventListener(
            LINK(this, NotesPaneModule, EventMultiplexerListener));
        mbListeningEventMultiplexer = true;
    }

    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case ResourceActivationRequestEvent:
            if (rEvent.ResourceId->isBoundToURL(FrameworkHelper::msCenterPaneURL,
                                                AnchorBindingMode_DIRECT))
            {
                if (rEvent.ResourceId->getResourceTypePrefix() == FrameworkHelper::msViewURLPrefix)
                {
                    onMainViewSwitch(rEvent.ResourceId->getResourceURL(), true);
                }
            }
            else if (rEvent.ResourceId->compareTo(mxBottomImpressPaneId) == 0)
            {
                onResourceRequest(true, rEvent.Configuration);
            }
            break;

        case ResourceDeactivationRequestEvent:
            if (rEvent.ResourceId->compareTo(mxMainViewAnchorId) == 0)
            {
                onMainViewSwitch(OUString(), false);
            }
            else if (rEvent.ResourceId->compareTo(mxBottomImpressPaneId) == 0)
            {
                onResourceRequest(false, rEvent.Configuration);
            }
            break;
        default:
            break;
    }
}

void SAL_CALL NotesPaneModule::disposing(const lang::EventObject& rEvent)
{
    if (mxConfigurationController.is() && rEvent.Source == mxConfigurationController)
    {
        SaveResourceState();
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = nullptr;
        dispose();
    }
}

void NotesPaneModule::onMainViewSwitch(const OUString& rsViewURL, const bool bIsActivated)
{
    if (bIsActivated)
        msCurrentMainViewURL = rsViewURL;
    else
        msCurrentMainViewURL.clear();

    if (!mxConfigurationController.is())
        return;

    sd::framework::ConfigurationController::Lock aLock(mxConfigurationController);

    if (IsResourceActive(msCurrentMainViewURL) && !mbInMasterEditMode)
    {
        mxConfigurationController->requestResourceActivation(mxBottomImpressPaneId->getAnchor(),
                                                             ResourceActivationMode_ADD);
        mxConfigurationController->requestResourceActivation(mxBottomImpressPaneId,
                                                             ResourceActivationMode_REPLACE);
    }
    else
    {
        mxConfigurationController->requestResourceDeactivation(mxBottomImpressPaneId);
    }
}

bool NotesPaneModule::IsMasterView(const Reference<XView>& xView)
{
    if (mpViewShellBase != nullptr)
    {
        auto pViewShellWrapper = dynamic_cast<ViewShellWrapper*>(xView.get());
        if (pViewShellWrapper)
        {
            std::shared_ptr<ViewShell> pViewShell = pViewShellWrapper->GetViewShell();
            auto pDrawViewShell = std::dynamic_pointer_cast<DrawViewShell>(pViewShell);

            if (pDrawViewShell && pDrawViewShell->GetEditMode() == EditMode::MasterPage)
                return true;
        }
    }
    return false;
}

void NotesPaneModule::onResourceRequest(
    bool bActivation,
    const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration)
{
    Sequence<Reference<XResourceId>> aCenterViews = rxConfiguration->getResources(
        FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
        FrameworkHelper::msViewURLPrefix, AnchorBindingMode_DIRECT);

    if (aCenterViews.getLength() != 1)
        return;

    // do not record the state of bottom pane when in master edit modes
    if (!IsMasterView({ mxConfigurationController->getResource(aCenterViews[0]), UNO_QUERY }))
    {
        if (bActivation)
        {
            maActiveMainViewContainer.insert(aCenterViews[0]->getResourceURL());
        }
        else
        {
            maActiveMainViewContainer.erase(aCenterViews[0]->getResourceURL());
        }
    }
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
