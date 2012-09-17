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

#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <modcfg.hxx>
#include <svx/htmlmode.hxx>
#include <viewopt.hxx>
#include "swmodule.hxx"
#include "cmdid.h"
#include "convert.hxx"
#include "tablemgr.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "tblafmt.hxx"

#include "app.hrc"
#include "table.hrc"
#include "swabstdlg.hxx"

namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

//keep the state of the buttons on runtime
static int nSaveButtonState = -1; // 0: tab, 1: semicolon, 2: paragraph, 3: other, -1: not yet used
static sal_Bool bIsKeepColumn = sal_True;
static sal_Unicode uOther = ',';

void SwConvertTableDlg::GetValues(  sal_Unicode& rDelim,
                                    SwInsertTableOptions& rInsTblOpts,
                                    SwTableAutoFmt const*& prTAFmt )
{
    if( mpTabBtn->IsChecked() )
    {
        //0x0b mustn't be set when re-converting table into text
        bIsKeepColumn = !mpKeepColumn->IsVisible() || mpKeepColumn->IsChecked();
        rDelim = bIsKeepColumn ? 0x09 : 0x0b;
        nSaveButtonState = 0;
    }
    else if( mpSemiBtn->IsChecked() )
    {
        rDelim = ';';
        nSaveButtonState = 1;
    }
    else if( mpOtherBtn->IsChecked() && mpOtherEd->GetText().Len() )
    {
        uOther = mpOtherEd->GetText().GetChar( 0 );
        rDelim = uOther;
        nSaveButtonState = 3;
    }
    else
    {
        nSaveButtonState = 2;
        rDelim = cParaDelim;
        if(mpOtherBtn->IsChecked())
        {
            nSaveButtonState = 3;
            uOther = 0;
        }
    }


    sal_uInt16 nInsMode = 0;
    if (mpBorderCB->IsChecked())
        nInsMode |= tabopts::DEFAULT_BORDER;
    if (mpHeaderCB->IsChecked())
        nInsMode |= tabopts::HEADLINE;
    if (mpRepeatHeaderCB->IsEnabled() && mpRepeatHeaderCB->IsChecked())
        rInsTblOpts.mnRowsToRepeat = sal_uInt16( mpRepeatHeaderNF->GetValue() );
    else
        rInsTblOpts.mnRowsToRepeat = 0;
    if (!mpDontSplitCB->IsChecked())
        nInsMode |= tabopts::SPLIT_LAYOUT;

    if( pTAutoFmt )
        prTAFmt = new SwTableAutoFmt( *pTAutoFmt );

    rInsTblOpts.mnInsMode = nInsMode;
}


SwConvertTableDlg::SwConvertTableDlg( SwView& rView, bool bToTable )
    : SfxModalDialog(&rView.GetViewFrame()->GetWindow(), "ConvertTextTableDialog", "modules/swriter/ui/converttexttable.ui" )
    , sConvertTextTable(SW_RES(STR_CONVERT_TEXT_TABLE))
    , pTAutoFmt(0)
    , pShell(&rView.GetWrtShell())
{
    get(mpTabBtn, "tabs");
    get(mpSemiBtn, "semicolons");
    get(mpParaBtn, "paragraph");
    get(mpOtherBtn, "other");
    get(mpOtherEd, "othered");
    get(mpKeepColumn, "keepcolumn");
    get(mpOptions, "options");
    get(mpHeaderCB, "headingcb");
    get(mpRepeatHeaderCB, "repeatheading");
    get(mpRepeatRows, "repeatrows");
    get(mpRepeatHeaderNF, "repeatheadersb");
    get(mpDontSplitCB, "dontsplitcb");
    get(mpBorderCB, "bordercb");
    get(mpAutoFmtBtn, "autofmt");

    mpOtherEd->SetAccessibleName(String(SW_RES(STR_SYMBOL)));
    mpOtherEd->SetAccessibleRelationLabeledBy(mpOtherBtn);

    if(nSaveButtonState > -1)
    {
        switch (nSaveButtonState)
        {
            case 0:
                mpTabBtn->Check();
                mpKeepColumn->Check(bIsKeepColumn);
            break;
            case 1: mpSemiBtn->Check();break;
            case 2: mpParaBtn->Check();break;
            case 3:
                mpOtherBtn->Check();
                if(uOther)
                    mpOtherEd->SetText(rtl::OUString(uOther));
            break;
        }

    }
    if( bToTable )
    {
        SetText( sConvertTextTable );
        mpAutoFmtBtn->SetClickHdl(LINK(this, SwConvertTableDlg, AutoFmtHdl));
        mpAutoFmtBtn->Show();
        mpKeepColumn->Show();
        mpKeepColumn->Enable( mpTabBtn->IsChecked() );
    }
    else
    {
        //Einfuege-Optionen verstecken
        mpOptions->Hide();
    }
    mpKeepColumn->SaveValue();

    Link aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
    mpTabBtn->SetClickHdl( aLk );
    mpSemiBtn->SetClickHdl( aLk );
    mpParaBtn->SetClickHdl( aLk );
    mpOtherBtn->SetClickHdl(aLk );
    mpOtherEd->Enable( mpOtherBtn->IsChecked() );

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    sal_Bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTblFlags(bHTMLMode);
    sal_uInt16 nInsTblFlags = aInsOpts.mnInsMode;

    mpHeaderCB->Check( 0 != (nInsTblFlags & tabopts::HEADLINE) );
    mpRepeatHeaderCB->Check(aInsOpts.mnRowsToRepeat > 0);
    mpDontSplitCB->Check( 0 == (nInsTblFlags & tabopts::SPLIT_LAYOUT));
    mpBorderCB->Check( 0!= (nInsTblFlags & tabopts::DEFAULT_BORDER) );

    mpHeaderCB->SetClickHdl(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    mpRepeatHeaderCB->SetClickHdl(LINK(this, SwConvertTableDlg, ReapeatHeaderCheckBoxHdl));
    ReapeatHeaderCheckBoxHdl();
    CheckBoxHdl();
}

SwConvertTableDlg:: ~SwConvertTableDlg()
{
    delete pTAutoFmt;
}

IMPL_LINK( SwConvertTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwAutoFormatDlg* pDlg = pFact->CreateSwAutoFormatDlg(pButton, pShell, DLG_AUTOFMT_TABLE, sal_False, pTAutoFmt);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFmtOfIndex( pTAutoFmt );
    delete pDlg;
    return 0;
}

IMPL_LINK( SwConvertTableDlg, BtnHdl, Button*, pButton )
{
    if( pButton == mpTabBtn )
        mpKeepColumn->SetState( mpKeepColumn->GetSavedValue() );
    else
    {
        if( mpKeepColumn->IsEnabled() )
            mpKeepColumn->SaveValue();
        mpKeepColumn->Check( sal_True );
    }
    mpKeepColumn->Enable( mpTabBtn->IsChecked() );
    mpOtherEd->Enable( mpOtherBtn->IsChecked() );
    return 0;
}

IMPL_LINK_NOARG(SwConvertTableDlg, CheckBoxHdl)
{
    mpRepeatHeaderCB->Enable(mpHeaderCB->IsChecked());
    ReapeatHeaderCheckBoxHdl();

    return 0;
}

IMPL_LINK_NOARG(SwConvertTableDlg, ReapeatHeaderCheckBoxHdl)
{
    bool bEnable = mpHeaderCB->IsChecked() && mpRepeatHeaderCB->IsChecked();
    mpRepeatRows->Enable(bEnable);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
