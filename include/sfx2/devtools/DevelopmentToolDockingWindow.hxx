/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/dllapi.h>
#include <sfx2/dockwin.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include <sfx2/devtools/ObjectInspectorWidgets.hxx>
#include <sfx2/devtools/DocumentModelTreeHandler.hxx>
#include <sfx2/devtools/ObjectInspectorTreeHandler.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <unordered_map>

/** Development tool main docking window
 *
 * Contains two sides. Left side contains the simplified DOM tree and
 * the right side the object inspector tree.
 */
class SFX2_DLLPUBLIC DevelopmentToolDockingWindow final : public SfxDockingWindow
{
private:
    std::unique_ptr<ObjectInspectorWidgets> mpObjectInspectorWidgets;
    std::unique_ptr<weld::TreeView> mpDocumentModelTreeView;
    std::unique_ptr<weld::Toolbar> mpDomToolbar;

    // Reference to the root object for the current document
    css::uno::Reference<css::uno::XInterface> mxRoot;
    // Stores the current selected object in the document
    css::uno::Reference<css::uno::XInterface> mxCurrentSelection;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;
    css::uno::Reference<css::view::XSelectionSupplier> mxSelectionSupplier;

    // Handler for the DOM tree
    DocumentModelTreeHandler maDocumentModelTreeHandler;
    // Handler for the object inspector tree
    ObjectInspectorTreeHandler maObjectInspectorTreeHandler;

    DECL_LINK(DocumentModelTreeViewSelectionHandler, weld::TreeView&, void);
    DECL_LINK(DomToolbarButtonClicked, const OString&, void);

    void updateSelection();

public:
    DevelopmentToolDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 vcl::Window* pParent);

    virtual ~DevelopmentToolDockingWindow() override;

    void dispose() override;

    void ToggleFloatingMode() override;

    // Signals that the selected object in the document changes
    void selectionChanged(css::uno::Reference<css::uno::XInterface> const& xInterface);

    // Signals to change to the current selected object in the object inspector
    void changeToCurrentSelection();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
