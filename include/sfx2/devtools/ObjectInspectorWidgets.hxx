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

struct ObjectInspectorWidgets
{
    ObjectInspectorWidgets(std::unique_ptr<weld::Builder>& rxBuilder)
        : mpClassNameLabel(rxBuilder->weld_label("class_name_value_id"))
        , mpInterfacesTreeView(rxBuilder->weld_tree_view("interfaces_treeview_id"))
        , mpServicesTreeView(rxBuilder->weld_tree_view("services_treeview_id"))
        , mpPropertiesTreeView(rxBuilder->weld_tree_view("properties_treeview_id"))
        , mpMethodsTreeView(rxBuilder->weld_tree_view("methods_treeview_id"))
        , mpToolbar(rxBuilder->weld_toolbar("object_inspector_toolbar"))
        , mpNotebook(rxBuilder->weld_notebook("object_inspector_notebookbar"))
        , mpTextView(rxBuilder->weld_text_view("object_inspector_text_view"))
        , mpPaned(rxBuilder->weld_paned("object_inspector_paned"))
    {
    }

    ~ObjectInspectorWidgets()
    {
        // dispose welded objects
        mpClassNameLabel.reset();
        mpInterfacesTreeView.reset();
        mpServicesTreeView.reset();
        mpPropertiesTreeView.reset();
        mpMethodsTreeView.reset();
        mpToolbar.reset();
        mpNotebook.reset();
        mpTextView.reset();
        mpPaned.reset();
    }

    std::unique_ptr<weld::Label> mpClassNameLabel;
    std::unique_ptr<weld::TreeView> mpInterfacesTreeView;
    std::unique_ptr<weld::TreeView> mpServicesTreeView;
    std::unique_ptr<weld::TreeView> mpPropertiesTreeView;
    std::unique_ptr<weld::TreeView> mpMethodsTreeView;
    std::unique_ptr<weld::Toolbar> mpToolbar;
    std::unique_ptr<weld::Notebook> mpNotebook;
    std::unique_ptr<weld::TextView> mpTextView;
    std::unique_ptr<weld::Paned> mpPaned;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
