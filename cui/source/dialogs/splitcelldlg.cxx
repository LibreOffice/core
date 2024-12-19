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
    : GenericDialogController(pParent, u"cui/ui/splitcellsdialog.ui"_ustr, u"SplitCellsDialog"_ustr)
    , m_xCountEdit(m_xBuilder->weld_spin_button(u"countnf"_ustr))
    , m_xGrid(m_xBuilder->weld_grid("directiongrid"))
    , m_xHorzBox(!bIsTableVertical ? m_xBuilder->weld_radio_button(u"hori"_ustr) : m_xBuilder->weld_radio_button(u"vert"_ustr))
    , m_xVertBox(!bIsTableVertical ? m_xBuilder->weld_radio_button(u"vert"_ustr) : m_xBuilder->weld_radio_button(u"hori"_ustr))
    , m_xPropCB(m_xBuilder->weld_check_button(u"prop"_ustr))
    , mnMaxVertical(nMaxVertical)
    , mnMaxHorizontal(nMaxHorizontal)
{
    m_xHorzBox->connect_toggled(LINK(this, SvxSplitTableDlg, ToggleHdl));
    m_xVertBox->connect_toggled(LINK(this, SvxSplitTableDlg, ToggleHdl));

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
        int nHorzTopAttach = m_xGrid->get_child_top_attach(*m_xHorzBox);
        int nVertTopAttach = m_xGrid->get_child_top_attach(*m_xVertBox);
        m_xGrid->set_child_top_attach(*m_xHorzBox, nVertTopAttach);
        m_xGrid->set_child_top_attach(*m_xVertBox, nHorzTopAttach);
        m_xHorzBox->set_active(m_xVertBox->get_active());
    }
}

IMPL_LINK(SvxSplitTableDlg, ToggleHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    const bool bIsVert = m_xVertBox->get_active();
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

void SvxSplitTableDlg::SetSplitVerticalByDefault()
{
    if( mnMaxVertical >= 2 )
        m_xVertBox->set_active(true); // tdf#60242
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
