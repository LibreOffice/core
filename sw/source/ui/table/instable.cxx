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

#include <instable.hxx>
#include <shellres.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <swabstdlg.hxx>

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableOpts, OUString& rAutoName,
                                SwTableAutoFormat *& prTAFormat )
{
    SwInsertTableFlags nInsMode = SwInsertTableFlags::NONE;
    rName = m_xNameEdit->get_text();
    rRow = m_xRowNF->get_value();
    rCol = m_xColNF->get_value();

    if (m_xHeaderCB->get_active())
        nInsMode |= SwInsertTableFlags::Headline;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= SwInsertTableFlags::SplitLayout;
    if( pTAutoFormat )
    {
        prTAFormat = new SwTableAutoFormat( *pTAutoFormat );
        rAutoName = prTAFormat->GetName();
    }

    rInsTableOpts.mnInsMode = nInsMode;
}

IMPL_LINK(SwInsTableDlg, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwInsTableDlg::SwInsTableDlg(SwView& rView)
    : weld::GenericDialogController(rView.GetFrameWeld(), "modules/swriter/ui/inserttable.ui", "InsertTableDialog")
    , m_aTextFilter(" .<>")
    , pShell(&rView.GetWrtShell())
    , pTAutoFormat(nullptr)
    , nEnteredValRepeatHeaderNF(-1)
    , m_xNameEdit(m_xBuilder->weld_entry("nameedit"))
    , m_xColNF(m_xBuilder->weld_spin_button("colspin"))
    , m_xRowNF(m_xBuilder->weld_spin_button("rowspin"))
    , m_xHeaderCB(m_xBuilder->weld_check_button("headercb"))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button("repeatcb"))
    , m_xRepeatHeaderNF(m_xBuilder->weld_spin_button("repeatheaderspin"))
    , m_xRepeatGroup(m_xBuilder->weld_widget("repeatgroup"))
    , m_xDontSplitCB(m_xBuilder->weld_check_button("dontsplitcb"))
    , m_xInsertBtn(m_xBuilder->weld_button("ok"))
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlbinstable"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "previewinstable", m_aWndPreview))
{
    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    m_xNameEdit->connect_insert_text(LINK(this, SwInsTableDlg, TextFilterHdl));
    m_xNameEdit->set_text(pShell->GetUniqueTableName());
    m_xNameEdit->connect_changed(LINK(this, SwInsTableDlg, ModifyName));
    m_xColNF->connect_value_changed(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_xRowNF->connect_value_changed(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_xRowNF->set_max(ROW_COL_PROD/m_xColNF->get_value());
    m_xColNF->set_max(ROW_COL_PROD/m_xRowNF->get_value());

    m_xInsertBtn->connect_clicked(LINK(this, SwInsTableDlg, OKHdl));

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    if (bHTMLMode)
        m_xDontSplitCB->hide();
    else
        m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));

    m_xRepeatHeaderNF->connect_value_changed( LINK( this, SwInsTableDlg, ModifyRepeatHeaderNF_Hdl ) );
    m_xHeaderCB->connect_toggled( LINK( this, SwInsTableDlg, CheckBoxHdl ) );
    m_xRepeatHeaderCB->connect_toggled( LINK( this, SwInsTableDlg, RepeatHeaderCheckBoxHdl ) );
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);

    sal_Int64 nMax = m_xRowNF->get_value();
    if( nMax <= 1 )
        nMax = 1;
    else
        --nMax;
    m_xRepeatHeaderNF->set_max( nMax );

    InitAutoTableFormat();
}

void SwInsTableDlg::InitAutoTableFormat()
{
    m_aWndPreview.DetectRTL(pShell);

    m_xLbFormat->connect_changed(LINK(this, SwInsTableDlg, SelFormatHdl));

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
    lbIndex = 1;
    m_xLbFormat->select( lbIndex );
    tbIndex = lbIndexToTableIndex(lbIndex);

    SelFormatHdl( *m_xLbFormat );
}

sal_uInt8 SwInsTableDlg::lbIndexToTableIndex( const sal_uInt8 listboxIndex )
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

IMPL_LINK_NOARG(SwInsTableDlg, SelFormatHdl, weld::TreeView&, void)
{
    // Get index of selected item from the listbox
    lbIndex = static_cast<sal_uInt8>(m_xLbFormat->get_selected_index());
    tbIndex = lbIndexToTableIndex( lbIndex );

    // To understand this index mapping, look InitAutoTableFormat function to
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

IMPL_LINK_NOARG(SwInsTableDlg, OKHdl, weld::Button&, void)
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

IMPL_LINK( SwInsTableDlg, ModifyName, weld::Entry&, rEdit, void )
{
    OUString sTableName = rEdit.get_text();
    m_xInsertBtn->set_sensitive(pShell->GetTableStyle(sTableName) == nullptr);
}

IMPL_LINK( SwInsTableDlg, ModifyRowCol, weld::SpinButton&, rEdit, void )
{
    if(&rEdit == m_xColNF.get())
    {
        sal_Int64 nCol = m_xColNF->get_value();
        if(!nCol)
            nCol = 1;
        m_xRowNF->set_max(ROW_COL_PROD/nCol);
    }
    else
    {
        sal_Int64 nRow = m_xRowNF->get_value();
        if(!nRow)
            nRow = 1;
        m_xColNF->set_max(ROW_COL_PROD/nRow);

        // adjust depending NF for repeated rows
        sal_Int64 nMax = ( nRow == 1 )? 1 : nRow - 1 ;
        sal_Int64 nActVal = m_xRepeatHeaderNF->get_value();

        m_xRepeatHeaderNF->set_max( nMax );

        if( nActVal > nMax )
            m_xRepeatHeaderNF->set_value( nMax );
        else if( nActVal < nEnteredValRepeatHeaderNF )
            m_xRepeatHeaderNF->set_value(std::min(nEnteredValRepeatHeaderNF, nMax));
    }
}

IMPL_LINK_NOARG(SwInsTableDlg, CheckBoxHdl, weld::ToggleButton&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwInsTableDlg, RepeatHeaderCheckBoxHdl, weld::ToggleButton&, void)
{
    m_xRepeatGroup->set_sensitive(m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active());
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl, weld::SpinButton&, void)
{
    nEnteredValRepeatHeaderNF = m_xRepeatHeaderNF->get_value();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
