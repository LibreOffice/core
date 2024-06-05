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

#include <contentcontrollistitemdlg.hxx>

#include <formatcontentcontrol.hxx>

SwContentControlListItemDlg::SwContentControlListItemDlg(weld::Widget* pParent,
                                                         SwContentControlListItem& rItem)
    : GenericDialogController(pParent, u"modules/swriter/ui/contentcontrollistitemdlg.ui"_ustr,
                              u"ContentControlListItemDialog"_ustr)
    , m_rItem(rItem)
    , m_xDisplayNameED(m_xBuilder->weld_entry(u"displayname"_ustr))
    , m_xValueED(m_xBuilder->weld_entry(u"value"_ustr))
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xOk->connect_clicked(LINK(this, SwContentControlListItemDlg, OkHdl));
    m_xDisplayNameED->set_text(rItem.m_aDisplayText);
    m_xValueED->set_text(rItem.m_aValue);
}

IMPL_LINK_NOARG(SwContentControlListItemDlg, OkHdl, weld::Button&, void)
{
    m_rItem.m_aDisplayText = m_xDisplayNameED->get_text();
    m_rItem.m_aValue = m_xValueED->get_text();

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
