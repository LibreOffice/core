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
#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>
#include <deque>

/** Object inspector tree handler
 *
 * Handles the object inspector part of DevTools - mainly interaction
 * between UI objects that consist of the object inspector.
 *
 */
class ObjectInspectorTreeHandler
{
private:
    std::unique_ptr<weld::TreeView>& mpInterfacesTreeView;
    std::unique_ptr<weld::TreeView>& mpServicesTreeView;
    std::unique_ptr<weld::TreeView>& mpPropertiesTreeView;
    std::unique_ptr<weld::TreeView>& mpMethodsTreeView;
    std::unique_ptr<weld::Label>& mpClassNameLabel;
    std::unique_ptr<weld::Toolbar>& mpObjectInspectorToolbar;
    std::unique_ptr<weld::Notebook>& mpObjectInspectorNotebook;

    // object stack to remember previously inspected objects so it is
    // possible to return back to them
    std::deque<css::uno::Any> maInspectionStack;

    // just the current context
    css::uno::Reference<css::uno::XComponentContext> mxContext;

    static void clearObjectInspectorChildren(std::unique_ptr<weld::TreeView>& pTreeView,
                                             weld::TreeIter const& rParent);
    static void handleExpanding(std::unique_ptr<weld::TreeView>& pTreeView,
                                weld::TreeIter const& rParent);
    static void clearAll(std::unique_ptr<weld::TreeView>& pTreeView);

    void appendInterfaces(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendServices(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendProperties(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendMethods(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void inspectObject(css::uno::Reference<css::uno::XInterface> const& xInterface);

    // Object stack handling
    void clearStack();
    void addToStack(css::uno::Any const& rAny);
    css::uno::Any popFromStack();

    void updateBackButtonState();

public:
    ObjectInspectorTreeHandler(std::unique_ptr<weld::TreeView>& pInterfacesTreeView,
                               std::unique_ptr<weld::TreeView>& pServicesTreeView,
                               std::unique_ptr<weld::TreeView>& pPropertiesTreeView,
                               std::unique_ptr<weld::TreeView>& pMethodsTreeView,
                               std::unique_ptr<weld::Label>& pClassNameLabel,
                               std::unique_ptr<weld::Toolbar>& pObjectInspectorToolbar,
                               std::unique_ptr<weld::Notebook>& pObjectInspectorNotebook);

    // callbacks when a node in the tree view is expanded
    DECL_LINK(ExpandingHandlerInterfaces, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerServices, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerProperties, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerMethods, const weld::TreeIter&, bool);

    // callback when the tree view selection changed to a different node
    DECL_LINK(SelectionChanged, weld::TreeView&, void);

    // callback when a pop-up is triggered on a tree view node
    DECL_LINK(PopupMenuHandler, const CommandEvent&, bool);

    // callback when a button is clicked on a toolbar
    DECL_LINK(ToolbarButtonClicked, const OString&, void);

    // callback when a page is entered or left on the notebook bar for
    // different categories
    DECL_LINK(NotebookEnterPage, const OString&, void);
    DECL_LINK(NotebookLeavePage, const OString&, bool);

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void dispose();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
