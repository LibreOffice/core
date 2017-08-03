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

#include <vcl/msgbox.hxx>

#include "wrtsh.hxx"
#include "view.hxx"
#include "itabenum.hxx"
#include "instable.hxx"
#include "tblafmt.hxx"
#include "modcfg.hxx"
#include "swmodule.hxx"
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>

#include "swabstdlg.hxx"
#include <swuiexp.hxx>
#include <memory>

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableOpts, OUString& rAutoName,
                                SwTableAutoFormat *& prTAFormat )
{
    sal_uInt16 nInsMode = 0;
    rName = m_xNameEdit->get_text();
    rRow = m_xRowNF->get_value_as_int();
    rCol = m_xColNF->get_value_as_int();

    if (m_xBorderCB->get_active())
        nInsMode |= tabopts::DEFAULT_BORDER;
    if (m_xHeaderCB->get_active())
        nInsMode |= tabopts::HEADLINE;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value_as_int();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= tabopts::SPLIT_LAYOUT;
    if (m_xTAutoFormat)
    {
        prTAFormat = new SwTableAutoFormat(*m_xTAutoFormat);
        rAutoName = prTAFormat->GetName();
    }

    rInsTableOpts.mnInsMode = nInsMode;
}

SwInsTableDlg::SwInsTableDlg( SwView& rView )
    : m_xBuilder(Application::CreateBuilder("modules/swriter/ui/inserttable.ui"))
    , m_aTextFilter(" .<>")
    , pShell(&rView.GetWrtShell())
    , nEnteredValRepeatHeaderNF(-1)
{
    m_xDialog.reset(m_xBuilder->weld_dialog("InsertTableDialog"));
    m_xDialog->set_transient_for(rView.GetWindow()->GetFrameWeld());
    m_xNameEdit.reset(m_xBuilder->weld_entry("nameedit"));
    m_xNameEdit->connect_insert_text(LINK(this, SwInsTableDlg, TextFilterHdl));
    m_xColNF.reset(m_xBuilder->weld_spin_button("colspin"));
    m_xRowNF.reset(m_xBuilder->weld_spin_button("rowspin"));
    m_xHeaderCB.reset(m_xBuilder->weld_check_button("headercb"));
    m_xRepeatHeaderCB.reset(m_xBuilder->weld_check_button("repeatcb"));
    m_xDontSplitCB.reset(m_xBuilder->weld_check_button("dontsplitcb"));
    m_xBorderCB.reset(m_xBuilder->weld_check_button("bordercb"));
    m_xAutoFormatBtn.reset(m_xBuilder->weld_button("autoformat"));
    m_xInsertBtn.reset(m_xBuilder->weld_button("ok"));
    m_xRepeatGroup.reset(m_xBuilder->weld_widget("repeatgroup"));
    m_xRepeatHeaderNF.reset(m_xBuilder->weld_spin_button("repeatheaderspin"));

    m_xNameEdit->set_text(pShell->GetUniqueTableName());
    m_xNameEdit->connect_changed(LINK(this, SwInsTableDlg, ModifyName));
    m_xColNF->connect_value_changed(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_xRowNF->connect_value_changed(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_xRowNF->set_range(1, ROW_COL_PROD/m_xColNF->get_value_as_int());
    m_xColNF->set_range(1, ROW_COL_PROD/m_xRowNF->get_value_as_int());
    m_xAutoFormatBtn->connect_clicked(LINK(this, SwInsTableDlg, AutoFormatHdl));

    m_xInsertBtn->connect_clicked(LINK(this, SwInsTableDlg, OKHdl));

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(0 != (nInsTableFlags & tabopts::HEADLINE));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    if (bHTMLMode)
    {
        m_xDontSplitCB->hide();
    }
    else
    {
        m_xDontSplitCB->set_active(0 == (nInsTableFlags & tabopts::SPLIT_LAYOUT));
    }
    m_xBorderCB->set_active(0 != (nInsTableFlags & tabopts::DEFAULT_BORDER));

    m_xRepeatHeaderNF->connect_value_changed(LINK( this, SwInsTableDlg, ModifyRepeatHeaderNF_Hdl));
    m_xHeaderCB->connect_toggled(LINK(this, SwInsTableDlg, CheckBoxHdl));
    m_xRepeatHeaderCB->connect_toggled(LINK(this, SwInsTableDlg, RepeatHeaderCheckBoxHdl));
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);

    int nMax = m_xRowNF->get_value_as_int();
    if( nMax <= 1 )
        nMax = 1;
    else
        --nMax;
    m_xRepeatHeaderNF->set_range(1, nMax);
}

IMPL_LINK(SwInsTableDlg, TextFilterHdl, OUString&, rText, bool)
{
    rText = m_aTextFilter.filter(rText);
    return true;
}

IMPL_LINK_NOARG(SwInsTableDlg, OKHdl, Weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwInsTableDlg, ModifyName, Weld::Entry&, rEdit, void)
{
    OUString sTableName = rEdit.get_text();
    if (sTableName.indexOf(' ') != -1)
    {
        sTableName = sTableName.replaceAll(" ", "");
        rEdit.set_text(sTableName);
    }

    m_xInsertBtn->set_sensitive(pShell->GetTableStyle(sTableName) == nullptr);
}

IMPL_LINK(SwInsTableDlg, ModifyRowCol, Weld::SpinButton&, rEdit, void)
{
    if (&rEdit == m_xColNF.get())
    {
        int nCol = m_xColNF->get_value_as_int();
        if(!nCol)
            nCol = 1;
        m_xRowNF->set_range(1, ROW_COL_PROD/nCol);
    }
    else
    {
        int nRow = m_xRowNF->get_value_as_int();
        if(!nRow)
            nRow = 1;
        m_xColNF->set_range(1, ROW_COL_PROD/nRow);

        // adjust depending NF for repeated rows
        int nMax = ( nRow == 1 )? 1 : nRow - 1 ;
        int nActVal = m_xRepeatHeaderNF->get_value_as_int();

        m_xRepeatHeaderNF->set_range(1, nMax);

        if( nActVal > nMax )
            m_xRepeatHeaderNF->set_value(nMax);
        else if( nActVal < nEnteredValRepeatHeaderNF )
            m_xRepeatHeaderNF->set_value((nEnteredValRepeatHeaderNF < nMax) ? nEnteredValRepeatHeaderNF : nMax);
    }
}
IMPL_LINK(SwInsTableDlg, AutoFormatHdl, Weld::Button&, /*rButton*/, void)
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    ScopedVclPtr<AbstractSwAutoFormatDlg> pDlg(pFact->CreateSwAutoFormatDlg(m_xDialog.get(), pShell, false, m_xTAutoFormat.get()));
    if (RET_OK == pDlg->Execute())
        m_xTAutoFormat.reset(pDlg->FillAutoFormatOfIndex());
}

IMPL_LINK_NOARG(SwInsTableDlg, CheckBoxHdl, Weld::ToggleButton&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwInsTableDlg, RepeatHeaderCheckBoxHdl, Weld::ToggleButton&, void)
{
    m_xRepeatGroup->set_sensitive(m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active());
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl, Weld::SpinButton&, void)
{
    nEnteredValRepeatHeaderNF = m_xRepeatHeaderNF->get_value_as_int();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
