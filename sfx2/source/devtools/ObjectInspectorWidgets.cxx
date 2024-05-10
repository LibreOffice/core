/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/devtools/ObjectInspectorWidgets.hxx>

ObjectInspectorWidgets::ObjectInspectorWidgets(const std::unique_ptr<weld::Builder>& rxBuilder)
    : mpClassNameLabel(rxBuilder->weld_label(u"class_name_value_id"_ustr))
    , mpInterfacesTreeView(rxBuilder->weld_tree_view(u"interfaces_treeview_id"_ustr))
    , mpServicesTreeView(rxBuilder->weld_tree_view(u"services_treeview_id"_ustr))
    , mpPropertiesTreeView(rxBuilder->weld_tree_view(u"properties_treeview_id"_ustr))
    , mpMethodsTreeView(rxBuilder->weld_tree_view(u"methods_treeview_id"_ustr))
    , mpToolbar(rxBuilder->weld_toolbar(u"object_inspector_toolbar"_ustr))
    , mpNotebook(rxBuilder->weld_notebook(u"object_inspector_notebookbar"_ustr))
    , mpTextView(rxBuilder->weld_text_view(u"object_inspector_text_view"_ustr))
    , mpPaned(rxBuilder->weld_paned(u"object_inspector_paned"_ustr))
{
}

ObjectInspectorWidgets::~ObjectInspectorWidgets()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
