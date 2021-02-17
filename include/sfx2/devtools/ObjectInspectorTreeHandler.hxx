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

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

class ObjectInspectorTreeHandler
{
private:
    std::unique_ptr<weld::TreeView>& mpInterfacesTreeView;
    std::unique_ptr<weld::TreeView>& mpServicesTreeView;
    std::unique_ptr<weld::TreeView>& mpPropertiesTreeView;
    std::unique_ptr<weld::TreeView>& mpMethodsTreeView;

    std::unique_ptr<weld::Label>& mpClassNameLabel;

    static void clearObjectInspectorChildren(std::unique_ptr<weld::TreeView>& pTreeView,
                                             weld::TreeIter const& rParent);
    static void handleExpanding(std::unique_ptr<weld::TreeView>& pTreeView,
                                weld::TreeIter const& rParent);
    static void clearAll(std::unique_ptr<weld::TreeView>& pTreeView);

    void appendInterfaces(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendServices(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendProperties(css::uno::Reference<css::uno::XInterface> const& xInterface);
    void appendMethods(css::uno::Reference<css::uno::XInterface> const& xInterface);

public:
    ObjectInspectorTreeHandler(std::unique_ptr<weld::TreeView>& pInterfacesTreeView,
                               std::unique_ptr<weld::TreeView>& pServicesTreeView,
                               std::unique_ptr<weld::TreeView>& pPropertiesTreeView,
                               std::unique_ptr<weld::TreeView>& pMethodsTreeView,
                               std::unique_ptr<weld::Label>& pClassNameLabel);

    DECL_LINK(ExpandingHandlerInterfaces, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerServices, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerProperties, const weld::TreeIter&, bool);
    DECL_LINK(ExpandingHandlerMethods, const weld::TreeIter&, bool);

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void dispose();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
