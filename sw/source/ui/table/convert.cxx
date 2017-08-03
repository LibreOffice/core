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
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <modcfg.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include "swmodule.hxx"
#include "cmdid.h"
#include "convert.hxx"
#include "tablemgr.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "tblafmt.hxx"

#include "app.hrc"
#include "strings.hrc"
#include "swabstdlg.hxx"
#include <swuiexp.hxx>
#include <memory>

//keep the state of the buttons on runtime
static int nSaveButtonState = -1; // 0: tab, 1: semicolon, 2: paragraph, 3: other, -1: not yet used
static bool bIsKeepColumn = true;
static sal_Unicode uOther = ',';

void SwConvertTableDlg::GetValues(  sal_Unicode& rDelim,
                                    SwInsertTableOptions& rInsTableOpts,
                                    SwTableAutoFormat const*& prTAFormat )
{
    if (m_xTabBtn->get_active())
    {
        //0x0b mustn't be set when re-converting table into text
        bIsKeepColumn = !m_xKeepColumn->get_visible() || m_xKeepColumn->get_active();
        rDelim = bIsKeepColumn ? 0x09 : 0x0b;
        nSaveButtonState = 0;
    }
    else if (m_xSemiBtn->get_active())
    {
        rDelim = ';';
        nSaveButtonState = 1;
    }
    else if (m_xOtherBtn->get_active() && !m_xOtherEd->get_text().isEmpty() )
    {
        uOther = m_xOtherEd->get_text()[0];
        rDelim = uOther;
        nSaveButtonState = 3;
    }
    else
    {
        nSaveButtonState = 2;
        rDelim = cParaDelim;
        if (m_xOtherBtn->get_active())
        {
            nSaveButtonState = 3;
            uOther = 0;
        }
    }

    sal_uInt16 nInsMode = 0;
    if (m_xBorderCB->get_active())
        nInsMode |= tabopts::DEFAULT_BORDER;
    if (m_xHeaderCB->get_active())
        nInsMode |= tabopts::HEADLINE;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= tabopts::SPLIT_LAYOUT;

    if (mxTAutoFormat)
        prTAFormat = new SwTableAutoFormat(*mxTAutoFormat);

    rInsTableOpts.mnInsMode = nInsMode;
}

SwConvertTableDlg::SwConvertTableDlg(SwView& rView, bool bToTable)
    : m_xBuilder(Application::CreateBuilder(rView.GetViewFrame()->GetWindow().GetFrameWeld(),
                 "modules/swriter/ui/converttexttable.ui"))
    , sConvertTextTable(SwResId(STR_CONVERT_TEXT_TABLE))
    , pShell(&rView.GetWrtShell())
{
    m_xDialog.reset(m_xBuilder->weld_dialog("ConvertTextTableDialog"));
    m_xTabBtn.reset(m_xBuilder->weld_radio_button("tabs"));
    m_xSemiBtn.reset(m_xBuilder->weld_radio_button("semicolons"));
    m_xParaBtn.reset(m_xBuilder->weld_radio_button("paragraph"));
    m_xOtherBtn.reset(m_xBuilder->weld_radio_button("other"));
    m_xOtherEd.reset(m_xBuilder->weld_entry("othered"));
    m_xKeepColumn.reset(m_xBuilder->weld_check_button("keepcolumn"));
    m_xOptions.reset(m_xBuilder->weld_widget("options"));
    m_xHeaderCB.reset(m_xBuilder->weld_check_button("headingcb"));
    m_xRepeatHeaderCB.reset(m_xBuilder->weld_check_button("repeatheading"));
    m_xRepeatRows.reset(m_xBuilder->weld_widget("repeatrows"));
    m_xRepeatHeaderNF.reset(m_xBuilder->weld_spin_button("repeatheadersb"));
    m_xDontSplitCB.reset(m_xBuilder->weld_check_button("dontsplitcb"));
    m_xBorderCB.reset(m_xBuilder->weld_check_button("bordercb"));
    m_xAutoFormatBtn.reset(m_xBuilder->weld_button("autofmt"));

    if(nSaveButtonState > -1)
    {
        switch (nSaveButtonState)
        {
            case 0:
                m_xTabBtn->set_active(true);
                m_xKeepColumn->set_active(bIsKeepColumn);
                break;
            case 1: m_xSemiBtn->set_active(true);
                break;
            case 2: m_xParaBtn->set_active(true);
                break;
            case 3:
                m_xOtherBtn->set_active(true);
                if (uOther)
                    m_xOtherEd->set_text(OUString(uOther));
                break;
        }

    }
    if( bToTable )
    {
        m_xDialog->set_title(sConvertTextTable);
        m_xAutoFormatBtn->connect_clicked(LINK(this, SwConvertTableDlg, AutoFormatHdl));
        m_xAutoFormatBtn->show();
        m_xKeepColumn->show();
        m_xKeepColumn->set_sensitive(m_xTabBtn->get_active());
    }
    else
    {
        //hide insert options
        m_xOptions->hide();
    }
    m_xKeepColumn->save_state();

    Link<Weld::ToggleButton&,void> aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
    m_xTabBtn->connect_toggled(aLk);
    m_xSemiBtn->connect_toggled(aLk);
    m_xParaBtn->connect_toggled(aLk);
    m_xOtherBtn->connect_toggled(aLk);
    m_xOtherEd->set_sensitive(m_xOtherBtn->get_active());

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(0 != (nInsTableFlags & tabopts::HEADLINE));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    m_xDontSplitCB->set_active(0 == (nInsTableFlags & tabopts::SPLIT_LAYOUT));
    m_xBorderCB->set_active(0!= (nInsTableFlags & tabopts::DEFAULT_BORDER));

    m_xHeaderCB->connect_toggled(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    m_xRepeatHeaderCB->connect_toggled(LINK(this, SwConvertTableDlg, RepeatHeaderCheckBoxHdl));
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);
}

IMPL_LINK_NOARG(SwConvertTableDlg, AutoFormatHdl, Weld::Button&, void)
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    ScopedVclPtr<AbstractSwAutoFormatDlg> pDlg(pFact->CreateSwAutoFormatDlg(m_xDialog.get(), pShell, false, mxTAutoFormat.get()));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if (RET_OK == pDlg->Execute())
        mxTAutoFormat.reset(pDlg->FillAutoFormatOfIndex());
}

IMPL_LINK(SwConvertTableDlg, BtnHdl, Weld::ToggleButton&, rButton, void)
{
    if (&rButton == m_xTabBtn.get())
        m_xKeepColumn->set_state(m_xKeepColumn->get_saved_state());
    else
    {
        if (m_xKeepColumn->get_sensitive())
            m_xKeepColumn->save_state();
        m_xKeepColumn->set_active(true);
    }
    m_xKeepColumn->set_sensitive(m_xTabBtn->get_active());
    m_xOtherEd->set_sensitive(m_xOtherBtn->get_active());
}

IMPL_LINK_NOARG(SwConvertTableDlg, CheckBoxHdl, Weld::ToggleButton&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwConvertTableDlg, RepeatHeaderCheckBoxHdl, Weld::ToggleButton&, void)
{
    bool bEnable = m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active();
    m_xRepeatRows->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
