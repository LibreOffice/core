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

#include <algorithm>
#include <vector>

#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <vcl/dialog.hxx>

#include "strings.hrc"
#include "dp_gui_dependencydialog.hxx"
#include "dp_shared.hxx"

namespace vcl { class Window; }

using dp_gui::DependencyDialog;

DependencyDialog::DependencyDialog(
    vcl::Window * parent, std::vector< OUString > const & dependencies):
    ModalDialog(parent, "Dependencies", "desktop/ui/dependenciesdialog.ui")
{
    get(m_list, "depListTreeview");
    set_height_request(200);
    SetMinOutputSizePixel(GetOutputSizePixel());
    m_list->SetReadOnly();
    for (std::vector< OUString >::const_iterator i(dependencies.begin());
         i != dependencies.end(); ++i)
    {
        m_list->InsertEntry(*i);
    }
}

DependencyDialog::~DependencyDialog()
{
    disposeOnce();
}

void DependencyDialog::dispose()
{
    m_list.clear();
    ModalDialog::dispose();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
