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
#include "table.hrc"
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
    else if( mpOtherBtn->IsChecked() && !mpOtherEd->GetText().isEmpty() )
    {
        uOther = mpOtherEd->GetText()[0];
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
        rInsTableOpts.mnRowsToRepeat = sal_uInt16( mpRepeatHeaderNF->GetValue() );
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!mpDontSplitCB->IsChecked())
        nInsMode |= tabopts::SPLIT_LAYOUT;

    if( pTAutoFormat )
        prTAFormat = new SwTableAutoFormat( *pTAutoFormat );

    rInsTableOpts.mnInsMode = nInsMode;
}

SwConvertTableDlg::SwConvertTableDlg( SwView& rView, bool bToTable )
    : SfxModalDialog(&rView.GetViewFrame()->GetWindow(), "ConvertTextTableDialog", "modules/swriter/ui/converttexttable.ui" )
    , sConvertTextTable(SW_RES(STR_CONVERT_TEXT_TABLE))
    , pTAutoFormat(0)
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
    get(mpAutoFormatBtn, "autofmt");

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
                    mpOtherEd->SetText(OUString(uOther));
            break;
        }

    }
    if( bToTable )
    {
        SetText( sConvertTextTable );
        mpAutoFormatBtn->SetClickHdl(LINK(this, SwConvertTableDlg, AutoFormatHdl));
        mpAutoFormatBtn->Show();
        mpKeepColumn->Show();
        mpKeepColumn->Enable( mpTabBtn->IsChecked() );
    }
    else
    {
        //hide insert options
        mpOptions->Hide();
    }
    mpKeepColumn->SaveValue();

    Link<Button*,void> aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
    mpTabBtn->SetClickHdl( aLk );
    mpSemiBtn->SetClickHdl( aLk );
    mpParaBtn->SetClickHdl( aLk );
    mpOtherBtn->SetClickHdl(aLk );
    mpOtherEd->Enable( mpOtherBtn->IsChecked() );

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    mpHeaderCB->Check( 0 != (nInsTableFlags & tabopts::HEADLINE) );
    mpRepeatHeaderCB->Check(aInsOpts.mnRowsToRepeat > 0);
    mpDontSplitCB->Check( 0 == (nInsTableFlags & tabopts::SPLIT_LAYOUT));
    mpBorderCB->Check( 0!= (nInsTableFlags & tabopts::DEFAULT_BORDER) );

    mpHeaderCB->SetClickHdl(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    mpRepeatHeaderCB->SetClickHdl(LINK(this, SwConvertTableDlg, ReapeatHeaderCheckBoxHdl));
    ReapeatHeaderCheckBoxHdl();
    CheckBoxHdl();
}

SwConvertTableDlg:: ~SwConvertTableDlg()
{
    disposeOnce();
}

void SwConvertTableDlg::dispose()
{
    delete pTAutoFormat;
    mpTabBtn.clear();
    mpSemiBtn.clear();
    mpParaBtn.clear();
    mpOtherBtn.clear();
    mpOtherEd.clear();
    mpKeepColumn.clear();
    mpOptions.clear();
    mpHeaderCB.clear();
    mpRepeatHeaderCB.clear();
    mpRepeatRows.clear();
    mpRepeatHeaderNF.clear();
    mpDontSplitCB.clear();
    mpBorderCB.clear();
    mpAutoFormatBtn.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_TYPED( SwConvertTableDlg, AutoFormatHdl, Button*, pButton, void )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    std::unique_ptr<AbstractSwAutoFormatDlg> pDlg(pFact->CreateSwAutoFormatDlg(pButton, pShell, false, pTAutoFormat));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFormatOfIndex( pTAutoFormat );
}

IMPL_LINK_TYPED( SwConvertTableDlg, BtnHdl, Button*, pButton, void )
{
    if( pButton == mpTabBtn )
        mpKeepColumn->SetState( mpKeepColumn->GetSavedValue() );
    else
    {
        if( mpKeepColumn->IsEnabled() )
            mpKeepColumn->SaveValue();
        mpKeepColumn->Check();
    }
    mpKeepColumn->Enable( mpTabBtn->IsChecked() );
    mpOtherEd->Enable( mpOtherBtn->IsChecked() );
}

IMPL_LINK_NOARG_TYPED(SwConvertTableDlg, CheckBoxHdl, Button*, void)
{
    mpRepeatHeaderCB->Enable(mpHeaderCB->IsChecked());
    ReapeatHeaderCheckBoxHdl();
}

IMPL_LINK_NOARG_TYPED(SwConvertTableDlg, ReapeatHeaderCheckBoxHdl, Button*, void)
{
    bool bEnable = mpHeaderCB->IsChecked() && mpRepeatHeaderCB->IsChecked();
    mpRepeatRows->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
