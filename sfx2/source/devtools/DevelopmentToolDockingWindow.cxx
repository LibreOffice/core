/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <sfx2/devtools/DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>

#include "SelectionChangeHandler.hxx"

using namespace css;

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "sfx/ui/developmenttool.ui")
    , mpObjectInspectorWidgets(new ObjectInspectorWidgets(m_xBuilder))
    , mpDocumentModelTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
    , mpSelectionToggle(m_xBuilder->weld_toggle_button("dom_selection_toggle"))
    , mpDomToolbar(m_xBuilder->weld_toolbar("dom_toolbar"))
    , maDocumentModelTreeHandler(
          mpDocumentModelTreeView,
          pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel())
    , maObjectInspectorTreeHandler(mpObjectInspectorWidgets)
{
    mpDocumentModelTreeView->connect_changed(
        LINK(this, DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler));
    mpSelectionToggle->connect_toggled(LINK(this, DevelopmentToolDockingWindow, SelectionToggled));
    mpDomToolbar->connect_clicked(
        LINK(this, DevelopmentToolDockingWindow, DomToolbarButtonClicked));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    maDocumentModelTreeHandler.inspectDocument();
    mxSelectionListener.set(new SelectionChangeHandler(xController, this));
    mxSelectionSupplier.set(xController, css::uno::UNO_QUERY);

    maObjectInspectorTreeHandler.introspect(mxRoot);
}

IMPL_LINK(DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler, weld::TreeView&,
          rView, void)
{
    if (mpSelectionToggle->get_state() == TRISTATE_TRUE)
        return;

    OUString sID = rView.get_selected_id();
    auto xObject = DocumentModelTreeHandler::getObjectByID(sID);
    if (xObject.is())
        maObjectInspectorTreeHandler.introspect(xObject);
}

IMPL_LINK_NOARG(DevelopmentToolDockingWindow, SelectionToggled, weld::ToggleButton&, void)
{
    updateSelection();
}

IMPL_LINK(DevelopmentToolDockingWindow, DomToolbarButtonClicked, const OString&, rSelectionId, void)
{
    if (rSelectionId == "dom_refresh_button")
    {
        maDocumentModelTreeHandler.inspectDocument();
    }
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose()
{
    auto* pSelectionChangeHandler
        = dynamic_cast<SelectionChangeHandler*>(mxSelectionListener.get());
    if (pSelectionChangeHandler)
        pSelectionChangeHandler->stopListening();

    mxSelectionListener = uno::Reference<view::XSelectionChangeListener>();

    maDocumentModelTreeHandler.dispose();
    maObjectInspectorTreeHandler.dispose();

    // dispose welded objects
    mpObjectInspectorWidgets.reset();
    mpSelectionToggle.reset();
    mpDomToolbar.reset();
    mpDocumentModelTreeView.reset();

    SfxDockingWindow::dispose();
}

void DevelopmentToolDockingWindow::updateSelection()
{
    TriState eTriState = mpSelectionToggle->get_state();
    if (eTriState == TRISTATE_TRUE)
    {
        maObjectInspectorTreeHandler.introspect(mxCurrentSelection);
        maDocumentModelTreeHandler.selectObject(mxCurrentSelection);
    }
    else
    {
        mpDocumentModelTreeView->set_sensitive(true);
    }
}

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

void DevelopmentToolDockingWindow::selectionChanged(
    uno::Reference<uno::XInterface> const& xInterface)
{
    mxCurrentSelection = xInterface;
    updateSelection();
}

void DevelopmentToolDockingWindow::changeToCurrentSelection()
{
    if (mxSelectionSupplier.is())
    {
        css::uno::Any aAny = mxSelectionSupplier->getSelection();
        if (aAny.hasValue())
        {
            auto xInterface = aAny.get<css::uno::Reference<css::uno::XInterface>>();
            if (xInterface.is())
            {
                maObjectInspectorTreeHandler.introspect(xInterface);
                mpSelectionToggle->set_state(TRISTATE_TRUE);
                return;
            }
        }
    }
    mpSelectionToggle->set_state(TRISTATE_FALSE);
    maObjectInspectorTreeHandler.introspect(mxRoot);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
