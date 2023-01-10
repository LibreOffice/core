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
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include "SelectionChangeHandler.hxx"

using namespace css;

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "sfx/ui/developmenttool.ui")
    , mpObjectInspectorWidgets(new ObjectInspectorWidgets(m_xBuilder))
    , mpDocumentModelTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
    , mpDomToolbar(m_xBuilder->weld_toolbar("dom_toolbar"))
    , maDocumentModelTreeHandler(
          mpDocumentModelTreeView,
          pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel())
    , maObjectInspectorTreeHandler(mpObjectInspectorWidgets)
{
    mpDocumentModelTreeView->connect_changed(
        LINK(this, DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler));
    mpDomToolbar->connect_clicked(
        LINK(this, DevelopmentToolDockingWindow, DomToolbarButtonClicked));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    maDocumentModelTreeHandler.inspectDocument();
    mxSelectionListener.set(new SelectionChangeHandler(xController, this));
    mxSelectionSupplier.set(xController, css::uno::UNO_QUERY);

    maObjectInspectorTreeHandler.introspect(mxRoot);

    SetText(SfxResId(STR_OBJECT_INSPECTOR));
}

IMPL_LINK(DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler, weld::TreeView&,
          rView, void)
{
    if (mpDomToolbar->get_item_active("dom_current_selection_toggle"))
        return;

    OUString sID = rView.get_selected_id();
    auto xObject = DocumentModelTreeHandler::getObjectByID(sID);
    if (xObject.is())
        maObjectInspectorTreeHandler.introspect(xObject);
}

IMPL_LINK(DevelopmentToolDockingWindow, DomToolbarButtonClicked, const OString&, rSelectionId, void)
{
    if (rSelectionId == "dom_refresh_button")
    {
        maDocumentModelTreeHandler.inspectDocument();
    }
    else if (rSelectionId == "dom_current_selection_toggle")
    {
        updateSelection();
    }
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose()
{
    // Stop and remove the listener
    auto* pSelectionChangeHandler
        = dynamic_cast<SelectionChangeHandler*>(mxSelectionListener.get());
    if (pSelectionChangeHandler)
        pSelectionChangeHandler->stopListening();

    mxSelectionListener = uno::Reference<view::XSelectionChangeListener>();

    // dispose DOM and object inspector handlers
    maDocumentModelTreeHandler.dispose();
    maObjectInspectorTreeHandler.dispose();

    // dispose welded objects
    mpObjectInspectorWidgets.reset();
    mpDomToolbar.reset();
    mpDocumentModelTreeView.reset();

    SfxDockingWindow::dispose();
}

void DevelopmentToolDockingWindow::updateSelection()
{
    bool bActive = mpDomToolbar->get_item_active("dom_current_selection_toggle");
    if (bActive)
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
                mpDomToolbar->set_item_active("dom_current_selection_toggle", true);
                return;
            }
        }
    }
    mpDomToolbar->set_item_active("dom_current_selection_toggle", false);
    maObjectInspectorTreeHandler.introspect(mxRoot);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
