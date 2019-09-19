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

#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <modcfg.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <cmdid.h>
#include <convert.hxx>
#include <tablemgr.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <shellres.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <swabstdlg.hxx>
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
    else if (m_xOtherBtn->get_active() && !m_xOtherEd->get_text().isEmpty())
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

    SwInsertTableFlags nInsMode = SwInsertTableFlags::NONE;
    if (m_xHeaderCB->get_active())
        nInsMode |= SwInsertTableFlags::Headline;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= SwInsertTableFlags::SplitLayout;

    if (pTAutoFormat)
        prTAFormat = new SwTableAutoFormat(*pTAutoFormat);

    rInsTableOpts.mnInsMode = nInsMode;
}

SwConvertTableDlg::SwConvertTableDlg(SwView& rView, bool bToTable)
    : SfxDialogController(rView.GetFrameWeld(),
            "modules/swriter/ui/converttexttable.ui", "ConvertTextTableDialog")
    , pShell(&rView.GetWrtShell())
    , pTAutoFormat(nullptr)
    , m_xTabBtn(m_xBuilder->weld_radio_button("tabs"))
    , m_xSemiBtn(m_xBuilder->weld_radio_button("semicolons"))
    , m_xParaBtn(m_xBuilder->weld_radio_button("paragraph"))
    , m_xOtherBtn(m_xBuilder->weld_radio_button("other"))
    , m_xOtherEd(m_xBuilder->weld_entry("othered"))
    , m_xKeepColumn(m_xBuilder->weld_check_button("keepcolumn"))
    , m_xOptions(m_xBuilder->weld_container("options"))
    , m_xHeaderCB(m_xBuilder->weld_check_button("headingcb"))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button("repeatheading"))
    , m_xRepeatRows(m_xBuilder->weld_container("repeatrows"))
    , m_xRepeatHeaderNF(m_xBuilder->weld_spin_button("repeatheadersb"))
    , m_xDontSplitCB(m_xBuilder->weld_check_button("dontsplitcb"))
    , m_xInsertBtn(m_xBuilder->weld_button("ok"))
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlbinstable"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "previewinstable", m_aWndPreview))
{
    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    if (nSaveButtonState > -1)
    {
        switch (nSaveButtonState)
        {
            case 0:
                m_xTabBtn->set_active(true);
                m_xKeepColumn->set_active(bIsKeepColumn);
                break;
            case 1:
                m_xSemiBtn->set_active(true);
                break;
            case 2:
                m_xParaBtn->set_active(true);
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
        m_xDialog->set_title(SwResId(STR_CONVERT_TEXT_TABLE));
        m_xKeepColumn->show();
        m_xKeepColumn->set_sensitive(m_xTabBtn->get_active());
    }
    else
    {
        //hide insert options
        m_xOptions->hide();
    }
    m_xKeepColumn->save_state();

    Link<weld::Button&,void> aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
    m_xTabBtn->connect_clicked(aLk);
    m_xSemiBtn->connect_clicked(aLk);
    m_xParaBtn->connect_clicked(aLk);
    m_xOtherBtn->connect_clicked(aLk);
    m_xOtherEd->set_sensitive(m_xOtherBtn->get_active());

    m_xInsertBtn->connect_clicked(LINK(this, SwConvertTableDlg, OKHdl));

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));

    m_xHeaderCB->connect_clicked(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    m_xRepeatHeaderCB->connect_clicked(LINK(this, SwConvertTableDlg, ReapeatHeaderCheckBoxHdl));
    ReapeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);

    InitAutoTableFormat();
}

void SwConvertTableDlg::InitAutoTableFormat()
{
    m_aWndPreview.DetectRTL(pShell);

    m_xLbFormat->connect_changed( LINK( this, SwConvertTableDlg, SelFormatHdl ) );

    pTableTable = new SwTableAutoFormatTable;
    pTableTable->Load();

    // Add "- none -" style autoformat table.
    m_xLbFormat->append_text(SwViewShell::GetShellRes()->aStrNone); // Insert to listbox

    // Add other styles of autoformat tables.
    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTable->size());
            i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*pTableTable)[ i ];
        m_xLbFormat->append_text(rFormat.GetName());
        if (pTAutoFormat && rFormat.GetName() == pTAutoFormat->GetName())
            lbIndex = i;
    }

    // Change this min variable if you add autotable manually.
    minTableIndexInLb = 1;
    maxTableIndexInLb = minTableIndexInLb + static_cast<sal_uInt8>(pTableTable->size());
    lbIndex = 0;
    m_xLbFormat->select( lbIndex );
    tbIndex = lbIndexToTableIndex(lbIndex);

    SelFormatHdl( *m_xLbFormat );
}

sal_uInt8 SwConvertTableDlg::lbIndexToTableIndex( const sal_uInt8 listboxIndex )
{
    if( minTableIndexInLb != maxTableIndexInLb &&
            minTableIndexInLb <= listboxIndex &&
            listboxIndex < maxTableIndexInLb )
    {
        return listboxIndex - minTableIndexInLb;
    }

    return 255;
}

static void lcl_SetProperties( SwTableAutoFormat* pTableAutoFormat, bool bVal )
{
    pTableAutoFormat->SetFont( bVal );
    pTableAutoFormat->SetJustify( bVal );
    pTableAutoFormat->SetFrame( bVal );
    pTableAutoFormat->SetBackground( bVal );
    pTableAutoFormat->SetValueFormat( bVal );
    pTableAutoFormat->SetWidthHeight( bVal );
}

IMPL_LINK_NOARG(SwConvertTableDlg, SelFormatHdl, weld::TreeView&, void)
{
    // Get index of selected item from the listbox
    lbIndex = static_cast<sal_uInt8>(m_xLbFormat->get_selected_index());
    tbIndex = lbIndexToTableIndex( lbIndex );

    // To understand this index maping, look InitAutoTableFormat function to
    // see how listbox item is implemented.
    if( tbIndex < 255 )
        m_aWndPreview.NotifyChange( (*pTableTable)[tbIndex] );
    else
    {
        SwTableAutoFormat aTmp( SwViewShell::GetShellRes()->aStrNone );
        lcl_SetProperties( &aTmp, false );

        m_aWndPreview.NotifyChange( aTmp );
    }
}

IMPL_LINK_NOARG(SwConvertTableDlg, OKHdl, weld::Button&, void)
{
    if( tbIndex < 255 )
        pShell->SetTableStyle((*pTableTable)[tbIndex]);

    if( tbIndex < 255 )
    {
        if( pTAutoFormat )
            *pTAutoFormat = (*pTableTable)[ tbIndex ];
        else
            pTAutoFormat = new SwTableAutoFormat( (*pTableTable)[ tbIndex ] );
    }
    else
    {
        delete pTAutoFormat;
        pTAutoFormat = new SwTableAutoFormat( SwViewShell::GetShellRes()->aStrNone );
        lcl_SetProperties( pTAutoFormat, false );
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwConvertTableDlg, BtnHdl, weld::Button&, rButton, void)
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

IMPL_LINK_NOARG(SwConvertTableDlg, CheckBoxHdl, weld::Button&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    ReapeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwConvertTableDlg, ReapeatHeaderCheckBoxHdl, weld::Button&, void)
{
    bool bEnable = m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active();
    m_xRepeatRows->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
