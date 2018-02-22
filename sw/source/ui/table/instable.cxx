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
    sal_uInt16 nInsMode = 0;
    rName = m_pNameEdit->GetText();
    rRow = static_cast<sal_uInt16>(m_pRowNF->GetValue());
    rCol = static_cast<sal_uInt16>(m_pColNF->GetValue());

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
    , pTAutoFormat(nullptr)
    , nEnteredValRepeatHeaderNF(-1)
{
    get(m_pNameEdit, "nameedit");
    m_pNameEdit->SetTextFilter(&m_aTextFilter);
    get(m_pColNF, "colspin");
    get(m_pRowNF, "rowspin");
    get(m_pHeaderCB, "headercb");
    get(m_pRepeatHeaderCB, "repeatcb");
    get(m_pDontSplitCB, "dontsplitcb");
    get(m_pInsertBtn, "ok");
    get(m_pRepeatGroup, "repeatgroup");
    get(m_pRepeatHeaderNF, "repeatheaderspin");
    get(m_pLbFormat, "formatlbinstable");
    get(m_pWndPreview, "previewinstable");

    m_pNameEdit->SetText(pShell->GetUniqueTableName());
    m_pNameEdit->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyName));
    m_pColNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_pRowNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_pRowNF->SetMax(ROW_COL_PROD/m_pColNF->GetValue());
    m_pColNF->SetMax(ROW_COL_PROD/m_pRowNF->GetValue());

    m_pInsertBtn->SetClickHdl(LINK(this, SwInsTableDlg, OKHdl));

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    m_pHeaderCB->Check( 0 != (nInsTableFlags & tabopts::HEADLINE) );
    m_pRepeatHeaderCB->Check(aInsOpts.mnRowsToRepeat > 0);
    if(bHTMLMode)
        m_pDontSplitCB->Hide();
    else
        m_pDontSplitCB->Check( 0 == (nInsTableFlags & tabopts::SPLIT_LAYOUT) );

    m_pRepeatHeaderNF->SetModifyHdl( LINK( this, SwInsTableDlg, ModifyRepeatHeaderNF_Hdl ) );
    m_pHeaderCB->SetClickHdl( LINK( this, SwInsTableDlg, CheckBoxHdl ) );
    m_pRepeatHeaderCB->SetClickHdl( LINK( this, SwInsTableDlg, ReapeatHeaderCheckBoxHdl ) );
    ReapeatHeaderCheckBoxHdl(nullptr);
    CheckBoxHdl(nullptr);

    sal_Int64 nMax = m_pRowNF->GetValue();
    if( nMax <= 1 )
        nMax = 1;
    else
        --nMax;
    m_pRepeatHeaderNF->SetMax( nMax );

    InitAutoTableFormat();
}

void SwInsTableDlg::InitAutoTableFormat()
{
    m_pWndPreview->DetectRTL(pShell);

    m_pLbFormat->SetSelectHdl( LINK( this, SwInsTableDlg, SelFormatHdl ) );

    pTableTable = new SwTableAutoFormatTable;
    pTableTable->Load();

    // Add "- none -" style autoformat table.
    m_pLbFormat->InsertEntry( SwViewShell::GetShellRes()->aStrNone ); // Insert to listbox

    // Add other styles of autoformat tables.
    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTable->size());
            i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*pTableTable)[ i ];
        m_pLbFormat->InsertEntry(rFormat.GetName());
        if (pTAutoFormat && rFormat.GetName() == pTAutoFormat->GetName())
            lbIndex = i;
    }

    // Change this min variable if you add autotable manually.
    minTableIndexInLb = 1;
    maxTableIndexInLb = minTableIndexInLb + static_cast<sal_uInt8>(pTableTable->size());
    lbIndex = 1;
    m_pLbFormat->SelectEntryPos( lbIndex );
    tbIndex = lbIndexToTableIndex(lbIndex);

    SelFormatHdl( *m_pLbFormat );
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

SwInsTableDlg::~SwInsTableDlg()
{
    disposeOnce();
}

void SwInsTableDlg::dispose()
{
    delete pTAutoFormat;
    m_pLbFormat.clear();
    m_pNameEdit.clear();
    m_pColNF.clear();
    m_pRowNF.clear();
    m_pHeaderCB.clear();
    m_pRepeatHeaderCB.clear();
    m_pRepeatHeaderNF.clear();
    m_pRepeatGroup.clear();
    m_pDontSplitCB.clear();
    m_pInsertBtn.clear();
    m_pWndPreview.clear();
    pTableTable->Save();
    delete pTableTable;
    SfxModalDialog::dispose();
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


IMPL_LINK_NOARG(SwInsTableDlg, SelFormatHdl, ListBox&, void)
{
    // Get index of selected item from the listbox
    lbIndex = static_cast<sal_uInt8>(m_pLbFormat->GetSelectedEntryPos());
    tbIndex = lbIndexToTableIndex( lbIndex );

    // To understand this index maping, look InitAutoTableFormat function to
    // see how listbox item is implemented.
    if( tbIndex < 255 )
        m_pWndPreview->NotifyChange( (*pTableTable)[tbIndex] );
    else
    {
        SwTableAutoFormat aTmp( SwViewShell::GetShellRes()->aStrNone );
        lcl_SetProperties( &aTmp, false );

        m_pWndPreview->NotifyChange( aTmp );
    }
}

IMPL_LINK_NOARG(SwInsTableDlg, OKHdl, Button*, void)
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

    EndDialog(RET_OK);
}

IMPL_LINK( SwInsTableDlg, ModifyName, Edit&, rEdit, void )
{
    OUString sTableName = rEdit.GetText();
    if (sTableName.indexOf(' ') != -1)
    {
        sTableName = sTableName.replaceAll(" ", "");
        rEdit.SetText(sTableName);
    }

    m_pInsertBtn->Enable(pShell->GetTableStyle( sTableName ) == nullptr);
}

IMPL_LINK( SwInsTableDlg, ModifyRowCol, Edit&, rEdit, void )
{
    if(&rEdit == m_pColNF)
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
            m_pRepeatHeaderNF->SetValue( std::min( nEnteredValRepeatHeaderNF, nMax ) );
    }
}

IMPL_LINK_NOARG(SwInsTableDlg, CheckBoxHdl, Button*, void)
{
    m_pRepeatHeaderCB->Enable(m_pHeaderCB->IsChecked());
    ReapeatHeaderCheckBoxHdl(nullptr);
}

IMPL_LINK_NOARG(SwInsTableDlg, ReapeatHeaderCheckBoxHdl, Button*, void)
{
    m_pRepeatGroup->Enable(m_pHeaderCB->IsChecked() && m_pRepeatHeaderCB->IsChecked());
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl, Edit&, void)
{
    nEnteredValRepeatHeaderNF = m_pRepeatHeaderNF->GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
