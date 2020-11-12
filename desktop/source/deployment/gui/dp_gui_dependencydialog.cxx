/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

#include "dp_gui_dependencydialog.hxx"

using dp_gui::DependencyDialog;

DependencyDialog::DependencyDialog(weld::Window* parent, std::vector<OUString> const& dependencies)
    : GenericDialogController(parent, "desktop/ui/dependenciesdialog.ui", "Dependencies")
    , m_xList(m_xBuilder->weld_tree_view("depListTreeview"))
{
    m_xList->set_size_request(-1, m_xList->get_height_rows(10));
    for (auto const& dependency : dependencies)
    {
        m_xList->append_text(dependency);
    }
}

DependencyDialog::~DependencyDialog() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
