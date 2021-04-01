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

#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <sfx2/devtools/ObjectInspectorWidgets.hxx>

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
    std::unique_ptr<ObjectInspectorWidgets>& mpObjectInspectorWidgets;

    // object stack to remember previously inspected objects so it is
    // possible to return back to them
    std::deque<css::uno::Any> maInspectionStack;

    // just the current context
    css::uno::Reference<css::uno::XComponentContext> mxContext;

    // should the paned size be reset to default on resize
    bool mbPanedResetSize;

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
    ObjectInspectorTreeHandler(std::unique_ptr<ObjectInspectorWidgets>& pObjectInspectorWidgets);

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

    DECL_LINK(PanedSizeChange, const Size&, void);

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void dispose();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
