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

#include <SelectLayerDlg.hxx>
#include <vcl/weld.hxx>

SdSelectLayerDlg::SdSelectLayerDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/sdraw/ui/selectlayerdialog.ui"_ustr,
                              u"SelectLayerDialog"_ustr)
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xListLB(m_xBuilder->weld_tree_view(u"treeview"_ustr))
{
    m_xListLB->set_size_request(m_xListLB->get_approximate_digit_width() * 32,
                                m_xListLB->get_height_rows(8));
    m_xListLB->connect_row_activated(LINK(this, SdSelectLayerDlg, DoubleClickHdl));
    m_xListLB->connect_changed(LINK(this, SdSelectLayerDlg, SelectHdl));
}

IMPL_LINK_NOARG(SdSelectLayerDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(SdSelectLayerDlg, SelectHdl, weld::TreeView&, void)
{
    m_xOk->set_sensitive(m_xListLB->get_selected_index() != -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
