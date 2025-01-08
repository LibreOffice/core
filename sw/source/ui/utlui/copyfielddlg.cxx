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

#include <copyfielddlg.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;

CopyFieldDlg::CopyFieldDlg(weld::Widget* pParent, const rtl::OUString& rFieldValue)
    : GenericDialogController(pParent, u"modules/swriter/ui/copyfielddialog.ui"_ustr,
                              u"CopyFieldDialog"_ustr)
    , m_xFieldValueED(new ConditionEdit<weld::TextView>(m_xBuilder->weld_text_view(u"value"_ustr)))
    , m_xCopy(m_xBuilder->weld_button(u"copy"_ustr))
    , m_xClose(m_xBuilder->weld_button(u"cancel"_ustr))
{
    m_xClose->connect_clicked(LINK(this, CopyFieldDlg, CloseHdl));
    m_xCopy->connect_clicked(LINK(this, CopyFieldDlg, CopyHdl));

    m_xFieldValueED->set_text(rFieldValue);
    sal_Int32 nRows{ comphelper::string::getTokenCount(rFieldValue, '\n') };

    m_xFieldValueED->get_widget().set_size_request(
        m_xFieldValueED->get_widget().get_preferred_size().Width() * 2,
        m_xFieldValueED->get_widget().get_height_rows(std::min<sal_Int32>(nRows + 1, 6)));
}

IMPL_LINK_NOARG(CopyFieldDlg, CloseHdl, weld::Button&, void) { m_xDialog->response(RET_OK); }

IMPL_LINK_NOARG(CopyFieldDlg, CopyHdl, weld::Button&, void)
{
    int nStartPos;
    int nEndPos;
    m_xFieldValueED->get_widget().get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos == nEndPos)
        m_xFieldValueED->get_widget().select_region(0, -1);

    m_xFieldValueED->get_widget().copy_clipboard();
    if (nStartPos == nEndPos)
        m_xFieldValueED->get_widget().select_region(0, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
