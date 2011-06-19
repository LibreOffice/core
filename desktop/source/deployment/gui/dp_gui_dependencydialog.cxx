/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <algorithm>
#include <vector>

#include "rtl/ustring.hxx"
#include "tools/gen.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "vcl/dialog.hxx"

#include "dp_gui.hrc"
#include "dp_gui_dependencydialog.hxx"
#include "dp_gui_shared.hxx"

class Window;

using dp_gui::DependencyDialog;

DependencyDialog::DependencyDialog(
    Window * parent, std::vector< rtl::OUString > const & dependencies):
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
    for (std::vector< rtl::OUString >::const_iterator i(dependencies.begin());
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
