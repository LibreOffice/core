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

#include "table.hrc"

#include "swabstdlg.hxx"
#include <swuiexp.hxx>
#include <memory>

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableOpts, OUString& rAutoName,
                                SwTableAutoFormat *& prTAFormat )
{
    sal_uInt16 nInsMode = 0;
    rName = m_pNameEdit->GetText();
    rRow = (sal_uInt16)m_pRowNF->GetValue();
    rCol = (sal_uInt16)m_pColNF->GetValue();

    if (m_pBorderCB->IsChecked())
        nInsMode |= tabopts::DEFAULT_BORDER;
    if (m_pHeaderCB->IsChecked())
        nInsMode |= tabopts::HEADLINE;
    if (m_pRepeatHeaderCB->IsEnabled() && m_pRepeatHeaderCB->IsChecked())
        rInsTableOpts.mnRowsToRepeat = sal_uInt16( m_pRepeatHeaderNF->GetValue() );
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_pDontSplitCB->IsChecked())
        nInsMode |= tabopts::SPLIT_LAYOUT;
    if( pTAutoFormat )
    {
        prTAFormat = new SwTableAutoFormat( *pTAutoFormat );
        rAutoName = prTAFormat->GetName();
    }

    rInsTableOpts.mnInsMode = nInsMode;
}

SwInsTableDlg::SwInsTableDlg( SwView& rView )
    : SfxModalDialog(rView.GetWindow(), "InsertTableDialog", "modules/swriter/ui/inserttable.ui")
    , m_aTextFilter(" .<>")
    , pShell(&rView.GetWrtShell())
    , pTAutoFormat(0)
    , nEnteredValRepeatHeaderNF(-1)
{
    get(m_pNameEdit, "nameedit");
    m_pNameEdit->SetTextFilter(&m_aTextFilter);
    get(m_pColNF, "colspin");
    get(m_pRowNF, "rowspin");
    get(m_pHeaderCB, "headercb");
    get(m_pRepeatHeaderCB, "repeatcb");
    get(m_pDontSplitCB, "dontsplitcb");
    get(m_pBorderCB, "bordercb");
    get(m_pAutoFormatBtn, "autoformat");
    get(m_pInsertBtn, "ok");
    get(m_pRepeatGroup, "repeatgroup");
    get(m_pRepeatHeaderNF, "repeatheaderspin");

    m_pNameEdit->SetText(pShell->GetUniqueTableName());
    m_pNameEdit->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyName));
    m_pColNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_pRowNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_pRowNF->SetMax(ROW_COL_PROD/m_pColNF->GetValue());
    m_pColNF->SetMax(ROW_COL_PROD/m_pRowNF->GetValue());
    m_pAutoFormatBtn->SetClickHdl(LINK(this, SwInsTableDlg, AutoFormatHdl));

    m_pInsertBtn->SetClickHdl(LINK(this, SwInsTableDlg, OKHdl));

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    m_pHeaderCB->Check( 0 != (nInsTableFlags & tabopts::HEADLINE) );
    m_pRepeatHeaderCB->Check(aInsOpts.mnRowsToRepeat > 0);
    if(bHTMLMode)
    {
        m_pDontSplitCB->Hide();
        m_pBorderCB->SetPosPixel(m_pDontSplitCB->GetPosPixel());
    }
    else
    {
        m_pDontSplitCB->Check( 0 == (nInsTableFlags & tabopts::SPLIT_LAYOUT) );
    }
    m_pBorderCB->Check( 0 != (nInsTableFlags & tabopts::DEFAULT_BORDER) );

    m_pRepeatHeaderNF->SetModifyHdl( LINK( this, SwInsTableDlg, ModifyRepeatHeaderNF_Hdl ) );
    m_pHeaderCB->SetClickHdl(LINK(this, SwInsTableDlg, CheckBoxHdl));
    m_pRepeatHeaderCB->SetClickHdl(LINK(this, SwInsTableDlg, ReapeatHeaderCheckBoxHdl));
    ReapeatHeaderCheckBoxHdl();
    CheckBoxHdl();

    sal_Int64 nMax = m_pRowNF->GetValue();
    if( nMax <= 1 )
        nMax = 1;
    else
        --nMax;
    m_pRepeatHeaderNF->SetMax( nMax );
}

IMPL_LINK_NOARG_TYPED(SwInsTableDlg, OKHdl, Button*, void)
{
    EndDialog(RET_OK);
}

SwInsTableDlg::~SwInsTableDlg()
{
    disposeOnce();
}

void SwInsTableDlg::dispose()
{
    delete pTAutoFormat;
    m_pNameEdit.clear();
    m_pColNF.clear();
    m_pRowNF.clear();
    m_pHeaderCB.clear();
    m_pRepeatHeaderCB.clear();
    m_pRepeatHeaderNF.clear();
    m_pRepeatGroup.clear();
    m_pDontSplitCB.clear();
    m_pBorderCB.clear();
    m_pInsertBtn.clear();
    m_pAutoFormatBtn.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK( SwInsTableDlg, ModifyName, Edit *, pEdit )
{
    OUString sTableName = pEdit->GetText();
    if (sTableName.indexOf(' ') != -1)
    {
        sTableName = sTableName.replaceAll(" ", "");
        pEdit->SetText(sTableName);
    }

    m_pInsertBtn->Enable(pShell->GetTableStyle( sTableName ) == 0);
    return 0;
}

IMPL_LINK( SwInsTableDlg, ModifyRowCol, NumericField *, pField )
{
    if(pField == m_pColNF)
    {
        sal_Int64 nCol = m_pColNF->GetValue();
        if(!nCol)
            nCol = 1;
        m_pRowNF->SetMax(ROW_COL_PROD/nCol);
    }
    else
    {
        sal_Int64 nRow = m_pRowNF->GetValue();
        if(!nRow)
            nRow = 1;
        m_pColNF->SetMax(ROW_COL_PROD/nRow);

        // adjust depending NF for repeated rows
        sal_Int64 nMax = ( nRow == 1 )? 1 : nRow - 1 ;
        sal_Int64 nActVal = m_pRepeatHeaderNF->GetValue();

        m_pRepeatHeaderNF->SetMax( nMax );

        if( nActVal > nMax )
            m_pRepeatHeaderNF->SetValue( nMax );
        else if( nActVal < nEnteredValRepeatHeaderNF )
            m_pRepeatHeaderNF->SetValue( ( nEnteredValRepeatHeaderNF < nMax )? nEnteredValRepeatHeaderNF : nMax );
    }
    return 0;
}

IMPL_LINK_TYPED( SwInsTableDlg, AutoFormatHdl, Button*, pButton, void )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    std::unique_ptr<AbstractSwAutoFormatDlg> pDlg(pFact->CreateSwAutoFormatDlg(pButton,pShell, false, pTAutoFormat));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFormatOfIndex( pTAutoFormat );
}

IMPL_LINK_NOARG_TYPED(SwInsTableDlg, CheckBoxHdl, Button*, void)
{
    m_pRepeatHeaderCB->Enable(m_pHeaderCB->IsChecked());
    ReapeatHeaderCheckBoxHdl();
}

IMPL_LINK_NOARG_TYPED(SwInsTableDlg, ReapeatHeaderCheckBoxHdl, Button*, void)
{
    m_pRepeatGroup->Enable(m_pHeaderCB->IsChecked() && m_pRepeatHeaderCB->IsChecked());
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl)
{
    nEnteredValRepeatHeaderNF = m_pRepeatHeaderNF->GetValue();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
