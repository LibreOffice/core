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


#include "sal/config.h"

#include <algorithm>
#include <vector>

#include "rtl/ustring.hxx"
#include "tools/resid.hxx"
#include "tools/solar.h"
#include "vcl/dialog.hxx"

#include "dp_gui.hrc"
#include "dp_gui_dependencydialog.hxx"
#include "dp_gui_shared.hxx"

class Window;

using dp_gui::DependencyDialog;

DependencyDialog::DependencyDialog(
    Window * parent, std::vector< OUString > const & dependencies):
    ModalDialog(parent, DpGuiResId(RID_DLG_DEPENDENCIES) ),
    m_text(this, DpGuiResId(RID_DLG_DEPENDENCIES_TEXT)),
    m_list(this, DpGuiResId(RID_DLG_DEPENDENCIES_LIST)),
    m_ok(this, DpGuiResId(RID_DLG_DEPENDENCIES_OK)),
    m_listDelta(
        GetOutputSizePixel().Width() - m_list.GetSizePixel().Width(),
        GetOutputSizePixel().Height() - m_list.GetSizePixel().Height())
{
    FreeResource();
    SetMinOutputSizePixel(GetOutputSizePixel());
    m_list.SetReadOnly();
    for (std::vector< OUString >::const_iterator i(dependencies.begin());
         i != dependencies.end(); ++i)
    {
        m_list.InsertEntry(*i);
    }
}

DependencyDialog::~DependencyDialog() {}

void DependencyDialog::Resize() {
    long n = m_ok.GetPosPixel().Y() -
        (m_list.GetPosPixel().Y() + m_list.GetSizePixel().Height());
    m_list.SetSizePixel(
        Size(
            GetOutputSizePixel().Width() - m_listDelta.Width(),
            GetOutputSizePixel().Height() - m_listDelta.Height()));
    m_ok.SetPosPixel(
        Point(
            (m_list.GetPosPixel().X() +
             (m_list.GetSizePixel().Width() - m_ok.GetSizePixel().Width()) / 2),
            m_list.GetPosPixel().Y() + m_list.GetSizePixel().Height() + n));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
