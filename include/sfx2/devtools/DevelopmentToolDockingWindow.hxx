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

#include <sfx2/devtools/DocumentModelTreeHandler.hxx>
#include <sfx2/devtools/ObjectInspectorTreeHandler.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <unordered_map>

class SFX2_DLLPUBLIC DevelopmentToolDockingWindow final : public SfxDockingWindow
{
private:
    std::unique_ptr<weld::Label> mpClassNameLabel;
    std::unique_ptr<weld::TreeView> mpInterfacesTreeView;
    std::unique_ptr<weld::TreeView> mpServicesTreeView;
    std::unique_ptr<weld::TreeView> mpPropertiesTreeView;
    std::unique_ptr<weld::TreeView> mpMethodsTreeView;
    std::unique_ptr<weld::TreeView> mpDocumentModelTreeView;
    std::unique_ptr<weld::ToggleButton> mpSelectionToggle;
    std::unique_ptr<weld::Toolbar> mpObjectInspectorToolbar;

    css::uno::Reference<css::uno::XInterface> mxRoot;
    css::uno::Reference<css::uno::XInterface> mxCurrentSelection;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;
    css::uno::Reference<css::view::XSelectionSupplier> mxSelectionSupplier;

    DocumentModelTreeHandler maDocumentModelTreeHandler;
    ObjectInspectorTreeHandler maObjectInspectorTreeHandler;

    DECL_LINK(DocumentModelTreeViewSelectionHandler, weld::TreeView&, void);
    DECL_LINK(SelectionToggled, weld::ToggleButton&, void);

    void inspectDocument();
    void updateSelection();
    void inspectSelectionOrRoot();

public:
    DevelopmentToolDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 vcl::Window* pParent);

    virtual ~DevelopmentToolDockingWindow() override;

    virtual void dispose() override;

    virtual void ToggleFloatingMode() override;

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void selectionChanged(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void changeToCurrentSelection();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
