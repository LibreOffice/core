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

#include <dialmgr.hxx>
#include <strings.hrc>
#include <insrc.hxx>

bool SvxInsRowColDlg::isInsertBefore() const
{
    return !m_xAfterBtn->get_active();
}

sal_uInt16 SvxInsRowColDlg::getInsertCount() const
{
    return m_xCountEdit->get_value();
}

SvxInsRowColDlg::SvxInsRowColDlg(weld::Window* pParent, bool bColumn, const OUString& rHelpId)
    : GenericDialogController(pParent, u"cui/ui/insertrowcolumn.ui"_ustr, u"InsertRowColumnDialog"_ustr)
    , m_xCountEdit(m_xBuilder->weld_spin_button(u"insert_number"_ustr))
    , m_xBeforeBtn(m_xBuilder->weld_radio_button(u"insert_before"_ustr))
    , m_xAfterBtn(m_xBuilder->weld_radio_button(u"insert_after"_ustr))
{
    m_xDialog->set_title(bColumn ? CuiResId(RID_CUISTR_COL) : CuiResId(RID_CUISTR_ROW));

    // tdf#119293
    if (bColumn) {
        m_xBeforeBtn->set_label(CuiResId(RID_CUISTR_INSERTCOL_BEFORE));
        m_xAfterBtn->set_label(CuiResId(RID_CUISTR_INSERTCOL_AFTER));
    } else {
        m_xBeforeBtn->set_label(CuiResId(RID_CUISTR_INSERTROW_BEFORE));
        m_xAfterBtn->set_label(CuiResId(RID_CUISTR_INSERTROW_AFTER));
    }

    m_xDialog->set_help_id(rHelpId);
}

short SvxAbstractInsRowColDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool SvxAbstractInsRowColDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

bool SvxAbstractInsRowColDlg_Impl::isInsertBefore() const
{
    return m_xDlg->isInsertBefore();
}

sal_uInt16 SvxAbstractInsRowColDlg_Impl::getInsertCount() const
{
    return m_xDlg->getInsertCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
