/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>

#include "wrtsh.hxx"
#include "view.hxx"
#include "itabenum.hxx"
#include "instable.hxx"
#include "tblafmt.hxx"
#include "modcfg.hxx"
#include "swmodule.hxx"
#include <svx/htmlmode.hxx>
#include <viewopt.hxx>

#include "table.hrc"
#include "instable.hrc"

#include "swabstdlg.hxx"

namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( String& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTblOpts, String& rAutoName,
                                SwTableAutoFmt *& prTAFmt )
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
        rInsTblOpts.mnRowsToRepeat = sal_uInt16( m_pRepeatHeaderNF->GetValue() );
    else
        rInsTblOpts.mnRowsToRepeat = 0;
    if (!m_pDontSplitCB->IsChecked())
        nInsMode |= tabopts::SPLIT_LAYOUT;
    if( pTAutoFmt )
    {
        prTAFmt = new SwTableAutoFmt( *pTAutoFmt );
        rAutoName = prTAFmt->GetName();
    }

    rInsTblOpts.mnInsMode = nInsMode;
}

// CTOR / DTOR -----------------------------------------------------------


SwInsTableDlg::SwInsTableDlg( SwView& rView )
    : SfxModalDialog(rView.GetWindow(), "InsertTableDialog", "modules/swriter/ui/inserttable.ui")
    , pShell(&rView.GetWrtShell())
    , pTAutoFmt(0)
    , nEnteredValRepeatHeaderNF(-1)
{
    get(m_pNameEdit, "nameedit");
    get(m_pColNF, "colspin");
    get(m_pRowNF, "rowspin");
    get(m_pHeaderCB, "headercb");
    get(m_pRepeatHeaderCB, "repeatcb");
    get(m_pDontSplitCB, "dontsplitcb");
    get(m_pBorderCB, "bordercb");
    get(m_pAutoFmtBtn, "autoformat");
    get(m_pOkBtn, "ok");
    get(m_pRepeatGroup, "repeatgroup");
    get(m_pRepeatHeaderNF, "repeatheaderspin");

    m_pNameEdit->SetText(pShell->GetUniqueTblName());
    m_pNameEdit->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyName));
    m_pColNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_pRowNF->SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_pRowNF->SetMax(ROW_COL_PROD/m_pColNF->GetValue());
    m_pColNF->SetMax(ROW_COL_PROD/m_pRowNF->GetValue());
    m_pAutoFmtBtn->SetClickHdl(LINK(this, SwInsTableDlg, AutoFmtHdl));

    sal_Bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTblFlags(bHTMLMode);
    sal_uInt16 nInsTblFlags = aInsOpts.mnInsMode;

    m_pHeaderCB->Check( 0 != (nInsTblFlags & tabopts::HEADLINE) );
    m_pRepeatHeaderCB->Check(aInsOpts.mnRowsToRepeat > 0);
    if(bHTMLMode)
    {
        m_pDontSplitCB->Hide();
        m_pBorderCB->SetPosPixel(m_pDontSplitCB->GetPosPixel());
    }
    else
    {
        m_pDontSplitCB->Check( 0 == (nInsTblFlags & tabopts::SPLIT_LAYOUT) );
    }
    m_pBorderCB->Check( 0 != (nInsTblFlags & tabopts::DEFAULT_BORDER) );

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

SwInsTableDlg::~SwInsTableDlg()
{
    delete pTAutoFmt;
}

IMPL_LINK_INLINE_START( SwInsTableDlg, ModifyName, Edit *, pEdit )
{
    String sTblName = pEdit->GetText();
    if(sTblName.Search(' ') != STRING_NOTFOUND)
    {
        sTblName = comphelper::string::remove(sTblName, ' ');
        pEdit->SetText(sTblName);
    }

    m_pOkBtn->Enable(pShell->GetTblStyle( sTblName ) == 0);
    return 0;
}
IMPL_LINK_INLINE_END( SwInsTableDlg, ModifyName, Edit *, pEdit )

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

IMPL_LINK( SwInsTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwAutoFormatDlg* pDlg = pFact->CreateSwAutoFormatDlg(pButton,pShell, sal_False, pTAutoFmt);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFmtOfIndex( pTAutoFmt );
    delete pDlg;
    return 0;
}

IMPL_LINK_NOARG(SwInsTableDlg, CheckBoxHdl)
{
    m_pRepeatHeaderCB->Enable(m_pHeaderCB->IsChecked());
    ReapeatHeaderCheckBoxHdl();

    return 0;
}

IMPL_LINK_NOARG(SwInsTableDlg, ReapeatHeaderCheckBoxHdl)
{
    m_pRepeatGroup->Enable(m_pHeaderCB->IsChecked() && m_pRepeatHeaderCB->IsChecked());

    return 0;
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl)
{
    nEnteredValRepeatHeaderNF = m_pRepeatHeaderNF->GetValue();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
