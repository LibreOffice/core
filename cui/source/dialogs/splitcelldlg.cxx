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

#include <splitcelldlg.hxx>

SvxSplitTableDlg::SvxSplitTableDlg(weld::Window *pParent, bool bIsTableVertical, tools::Long nMaxVertical, tools::Long nMaxHorizontal)
    : GenericDialogController(pParent, "cui/ui/splitcellsdialog.ui", "SplitCellsDialog")
    , m_xCountEdit(m_xBuilder->weld_spin_button("countnf"))
    , m_xHorzBox(!bIsTableVertical ? m_xBuilder->weld_radio_button("hori") : m_xBuilder->weld_radio_button("vert"))
    , m_xVertBox(!bIsTableVertical ? m_xBuilder->weld_radio_button("vert") : m_xBuilder->weld_radio_button("hori"))
    , m_xPropCB(m_xBuilder->weld_check_button("prop"))
    , mnMaxVertical(nMaxVertical)
    , mnMaxHorizontal(nMaxHorizontal)
{
    m_xHorzBox->connect_clicked(LINK(this, SvxSplitTableDlg, ClickHdl));
    m_xPropCB->connect_clicked(LINK(this, SvxSplitTableDlg, ClickHdl));
    m_xVertBox->connect_clicked(LINK(this, SvxSplitTableDlg, ClickHdl));

    if (mnMaxVertical < 2)
    {
        if (!bIsTableVertical)
            m_xVertBox->set_sensitive(false);
        else
            m_xHorzBox->set_sensitive(false);
    }

    //exchange the meaning of horizontal and vertical for vertical text
    if (bIsTableVertical)
    {
        int nHorzTopAttach = m_xHorzBox->get_grid_top_attach();
        int nVertTopAttach = m_xVertBox->get_grid_top_attach();
        m_xHorzBox->set_grid_top_attach(nVertTopAttach);
        m_xVertBox->set_grid_top_attach(nHorzTopAttach);
        m_xHorzBox->set_active(m_xVertBox->get_active());
    }
}

IMPL_LINK(SvxSplitTableDlg, ClickHdl, weld::Button&, rButton, void)
{
    const bool bIsVert = &rButton == m_xVertBox.get();
    tools::Long nMax = bIsVert ? mnMaxVertical : mnMaxHorizontal;
    m_xPropCB->set_sensitive(!bIsVert);
    m_xCountEdit->set_max(nMax);
}

bool SvxSplitTableDlg::IsHorizontal() const
{
    return m_xHorzBox->get_active();
}

bool SvxSplitTableDlg::IsProportional() const
{
    return m_xPropCB->get_active() && m_xHorzBox->get_active();
}

tools::Long SvxSplitTableDlg::GetCount() const
{
    return m_xCountEdit->get_value();
}

short SvxSplitTableDlg::Execute()
{
    return run();
}

void SvxSplitTableDlg::SetSplitVerticalByDefault()
{
    if( mnMaxVertical >= 2 )
        m_xVertBox->set_active(true); // tdf#60242
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
