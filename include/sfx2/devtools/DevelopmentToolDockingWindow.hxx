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

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <unordered_map>

class SFX2_DLLPUBLIC DevelopmentToolDockingWindow final : public SfxDockingWindow
{
private:
    std::unique_ptr<weld::Label> mpClassNameLabel;
    std::unique_ptr<weld::TreeView> mpClassListBox;
    std::unique_ptr<weld::TreeView> mpDocumentModelTreeView;
    std::unique_ptr<weld::ToggleButton> mpSelectionToggle;

    css::uno::Reference<css::uno::XInterface> mxRoot;
    css::uno::Reference<css::uno::XInterface> mxCurrentSelection;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    DocumentModelTreeHandler maDocumentModelTreeHandler;

    DECL_LINK(DocumentModelTreeViewSelectionHandler, weld::TreeView&, void);
    DECL_LINK(SelectionToggled, weld::ToggleButton&, void);

    DECL_LINK(ObjectInspectorExpandingHandler, const weld::TreeIter&, bool);

    void inspectDocument();
    void updateSelection();
    void inspectSelectionOrRoot(css::uno::Reference<css::frame::XController> const& xController);

    void clearObjectInspectorChildren(weld::TreeIter const& rParent);

public:
    DevelopmentToolDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 vcl::Window* pParent);

    virtual ~DevelopmentToolDockingWindow() override;

    virtual void dispose() override;

    virtual void ToggleFloatingMode() override;

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void selectionChanged(css::uno::Reference<css::uno::XInterface> const& xInterface);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
