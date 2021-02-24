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

#include <vcl/floatwin.hxx>

#include "SelectionChangeHandler.hxx"

using namespace css;

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "sfx/ui/developmenttool.ui")
    , mpClassNameLabel(m_xBuilder->weld_label("class_name_value_id"))
    , mpInterfacesTreeView(m_xBuilder->weld_tree_view("interfaces_treeview_id"))
    , mpServicesTreeView(m_xBuilder->weld_tree_view("services_treeview_id"))
    , mpPropertiesTreeView(m_xBuilder->weld_tree_view("properties_treeview_id"))
    , mpMethodsTreeView(m_xBuilder->weld_tree_view("methods_treeview_id"))
    , mpDocumentModelTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
    , mpSelectionToggle(m_xBuilder->weld_toggle_button("selection_toggle"))
    , maDocumentModelTreeHandler(
          mpDocumentModelTreeView,
          pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel())
    , maObjectInspectorTreeHandler(mpInterfacesTreeView, mpServicesTreeView, mpPropertiesTreeView,
                                   mpMethodsTreeView, mpClassNameLabel)
{
    mpDocumentModelTreeView->connect_changed(
        LINK(this, DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler));
    mpSelectionToggle->connect_toggled(LINK(this, DevelopmentToolDockingWindow, SelectionToggled));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    maDocumentModelTreeHandler.inspectDocument();
    mxSelectionListener.set(new SelectionChangeHandler(xController, this));
    mxSelectionSupplier.set(xController, css::uno::UNO_QUERY);

    maObjectInspectorTreeHandler.introspect(mxRoot);
}

void DevelopmentToolDockingWindow::inspectSelectionOrRoot()
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
    mpClassNameLabel.reset();
    mpInterfacesTreeView.reset();
    mpServicesTreeView.reset();
    mpPropertiesTreeView.reset();
    mpMethodsTreeView.reset();
    mpSelectionToggle.reset();
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

void DevelopmentToolDockingWindow::changeToCurrentSelection() { inspectSelectionOrRoot(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
