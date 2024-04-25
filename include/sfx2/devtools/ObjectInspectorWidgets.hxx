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

struct SFX2_DLLPUBLIC ObjectInspectorWidgets
{
    ObjectInspectorWidgets(const std::unique_ptr<weld::Builder>& rxBuilder);

    ~ObjectInspectorWidgets();

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
