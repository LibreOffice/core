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

#include "dbmgr.hxx"
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <swwait.hxx>
#include <viewopt.hxx>

#include "wrtsh.hxx"
#include "cmdid.h"
#include "helpid.h"
#include "envfmt.hxx"
#include "envlop.hxx"
#include "envprt.hxx"
#include "fmtcol.hxx"
#include "poolfmt.hxx"
#include "view.hxx"

#include "envlop.hrc"
#include <comphelper/processfactory.hxx>

#include <unomid.h>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;


//impl in envimg.cxx
extern SW_DLLPUBLIC String MakeSender();

SwEnvPreview::SwEnvPreview(SfxTabPage* pParent, const ResId& rResID) :

    Window(pParent, rResID)

{
    SetMapMode(MapMode(MAP_PIXEL));
}

SwEnvPreview::~SwEnvPreview()
{
}

void SwEnvPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( DATACHANGED_SETTINGS == rDCEvt.GetType() )
        SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );
}

void SwEnvPreview::Paint(const Rectangle &)
{
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    const SwEnvItem& rItem =
        ((SwEnvDlg*) GetParent()->GetParent()->GetParent())->aEnvItem;

    sal_uInt16 nPageW = (sal_uInt16) Max(rItem.lWidth, rItem.lHeight),
           nPageH = (sal_uInt16) Min(rItem.lWidth, rItem.lHeight);

    float fx = (float)GetOutputSizePixel().Width () / (float)nPageW,
          fy = (float)GetOutputSizePixel().Height() / (float)nPageH,
          f  = 0.8f * ( fx < fy ? fx : fy );

    Color aBack = rSettings.GetWindowColor( );
    Color aFront = SwViewOption::GetFontColor();
    Color aMedium = Color(  ( aBack.GetRed() + aFront.GetRed() ) / 2,
                            ( aBack.GetGreen() + aFront.GetGreen() ) / 2,
                            ( aBack.GetBlue() + aFront.GetBlue() ) / 2
                        );

    SetLineColor( aFront );

    // Envelope
    long   nW = (sal_uInt16) (f * nPageW),
           nH = (sal_uInt16) (f * nPageH),
           nX = (GetOutputSizePixel().Width () - nW) / 2,
           nY = (GetOutputSizePixel().Height() - nH) / 2;
    SetFillColor( aBack );
    DrawRect(Rectangle(Point(nX, nY), Size(nW, nH)));

    // Sender
    if (rItem.bSend)
    {
        long   nSendX = nX + (sal_uInt16) (f * rItem.lSendFromLeft),
               nSendY = nY + (sal_uInt16) (f * rItem.lSendFromTop ),
               nSendW = (sal_uInt16) (f * (rItem.lAddrFromLeft - rItem.lSendFromLeft)),
               nSendH = (sal_uInt16) (f * (rItem.lAddrFromTop  - rItem.lSendFromTop  - 566));
        SetFillColor( aMedium );

        DrawRect(Rectangle(Point(nSendX, nSendY), Size(nSendW, nSendH)));
    }

    // Addressee
    long   nAddrX = nX + (sal_uInt16) (f * rItem.lAddrFromLeft),
           nAddrY = nY + (sal_uInt16) (f * rItem.lAddrFromTop ),
           nAddrW = (sal_uInt16) (f * (nPageW - rItem.lAddrFromLeft - 566)),
           nAddrH = (sal_uInt16) (f * (nPageH - rItem.lAddrFromTop  - 566));
    SetFillColor( aMedium );
    DrawRect(Rectangle(Point(nAddrX, nAddrY), Size(nAddrW, nAddrH)));

    // Stamp
    long   nStmpW = (sal_uInt16) (f * 1417 /* 2,5 cm */),
           nStmpH = (sal_uInt16) (f * 1701 /* 3,0 cm */),
           nStmpX = nX + nW - (sal_uInt16) (f * 566) - nStmpW,
           nStmpY = nY + (sal_uInt16) (f * 566);

    SetFillColor( aBack );
    DrawRect(Rectangle(Point(nStmpX, nStmpY), Size(nStmpW, nStmpH)));
}

SwEnvDlg::SwEnvDlg(Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert) :

    SfxTabDialog(pParent, SW_RES(DLG_ENV), &rSet, sal_False, &aEmptyStr),
    sInsert(SW_RES(ST_INSERT)),
    sChange(SW_RES(ST_CHANGE)),
    aEnvItem((const SwEnvItem&) rSet.Get(FN_ENVELOP)),
    pSh(pWrtSh),
    pPrinter(pPrt),
    pAddresseeSet(0),
    pSenderSet(0)
{
    FreeResource();

    GetOKButton().SetText(String(SW_RES(STR_BTN_NEWDOC)));
    GetOKButton().SetHelpId(HID_ENVELOP_PRINT);
    GetOKButton().SetHelpText(aEmptyStr);   // in order for generated help text to get used
    if (GetUserButton())
    {
        GetUserButton()->SetText(bInsert ? sInsert : sChange);
        GetUserButton()->SetHelpId(HID_ENVELOP_INSERT);
    }

    AddTabPage(TP_ENV_ENV, SwEnvPage   ::Create, 0);
    AddTabPage(TP_ENV_FMT, SwEnvFmtPage::Create, 0);
    AddTabPage(TP_ENV_PRT, SwEnvPrtPage::Create, 0);
}

SwEnvDlg::~SwEnvDlg()
{
    delete pAddresseeSet;
    delete pSenderSet;
}

void SwEnvDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == TP_ENV_PRT)
    {
        ((SwEnvPrtPage*)&rPage)->SetPrt(pPrinter);
    }
}

short SwEnvDlg::Ok()
{
    short nRet = SfxTabDialog::Ok();

    if (nRet == RET_OK || nRet == RET_USER)
    {
        if (pAddresseeSet)
        {
            SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool(RES_POOLCOLL_JAKETADRESS);
            pColl->SetFmtAttr(*pAddresseeSet);
        }
        if (pSenderSet)
        {
            SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool(RES_POOLCOLL_SENDADRESS);
            pColl->SetFmtAttr(*pSenderSet);
        }
    }

    return nRet;
}

SwEnvPage::SwEnvPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_ENV_ENV), rSet),

    aAddrText      (this, SW_RES(TXT_ADDR   )),
    aAddrEdit      (this, SW_RES(EDT_ADDR   )),
    aDatabaseFT    (this, SW_RES(FT_DATABASE)),
    aDatabaseLB    (this, SW_RES(LB_DATABASE)),
    aTableFT       (this, SW_RES(FT_TABLE   )),
    aTableLB       (this, SW_RES(LB_TABLE   )),
    aInsertBT      (this, SW_RES(BTN_INSERT )),
    aDBFieldFT     (this, SW_RES(FT_DBFIELD )),
    aDBFieldLB     (this, SW_RES(LB_DBFIELD )),
    aSenderBox     (this, SW_RES(BOX_SEND   )),
    aSenderEdit    (this, SW_RES(EDT_SEND   )),
    aPreview       (this, SW_RES(WIN_PREVIEW))

{
    FreeResource();
    SetExchangeSupport();
    pSh = GetParent()->pSh;

    // Install handlers
    aDatabaseLB    .SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    aTableLB       .SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    aInsertBT      .SetClickHdl (LINK(this, SwEnvPage, FieldHdl        ));
    aSenderBox     .SetClickHdl (LINK(this, SwEnvPage, SenderHdl       ));
    aPreview.SetBorderStyle( WINDOW_BORDER_MONO );

    SwDBData aData = pSh->GetDBData();
    sActDBName = aData.sDataSource;
    sActDBName += DB_DELIM;
    sActDBName += (String)aData.sCommand;
    InitDatabaseBox();
}

SwEnvPage::~SwEnvPage()
{
}

IMPL_LINK( SwEnvPage, DatabaseHdl, ListBox *, pListBox )
{
    SwWait aWait( *pSh->GetView().GetDocShell(), sal_True );

    if (pListBox == &aDatabaseLB)
    {
        sActDBName = pListBox->GetSelectEntry();
        pSh->GetNewDBMgr()->GetTableNames(&aTableLB, sActDBName);
        sActDBName += DB_DELIM;
    }
    else
        sActDBName.SetToken(1, DB_DELIM, aTableLB.GetSelectEntry());
        pSh->GetNewDBMgr()->GetColumnNames(&aDBFieldLB, aDatabaseLB.GetSelectEntry(),
                                           aTableLB.GetSelectEntry());
    return 0;
}

IMPL_LINK_NOARG(SwEnvPage, FieldHdl)
{
    rtl::OUStringBuffer aStr;
    aStr.append('<');
    aStr.append(aDatabaseLB.GetSelectEntry());
    aStr.append('.');
    aStr.append(aTableLB.GetSelectEntry());
    aStr.append('.');
    aStr.append(aTableLB.GetEntryData(aTableLB.GetSelectEntryPos()) == 0 ? '0' : '1');
    aStr.append('.');
    aStr.append(aDBFieldLB.GetSelectEntry());
    aStr.append('>');
    aAddrEdit.ReplaceSelected(aStr.makeStringAndClear());
    Selection aSel = aAddrEdit.GetSelection();
    aAddrEdit.GrabFocus();
    aAddrEdit.SetSelection(aSel);
    return 0;
}

IMPL_LINK_NOARG(SwEnvPage, SenderHdl)
{
    const sal_Bool bEnable = aSenderBox.IsChecked();
    GetParent()->aEnvItem.bSend = bEnable;
    aSenderEdit.Enable(bEnable);
    if ( bEnable )
    {
        aSenderEdit.GrabFocus();
        if(!aSenderEdit.GetText().Len())
            aSenderEdit.SetText(MakeSender());
    }
    aPreview.Invalidate();
    return 0;
}

void SwEnvPage::InitDatabaseBox()
{
    if (pSh->GetNewDBMgr())
    {
        aDatabaseLB.Clear();
        Sequence<OUString> aDataNames = SwNewDBMgr::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();

        for (long i = 0; i < aDataNames.getLength(); i++)
            aDatabaseLB.InsertEntry(pDataNames[i]);

        String sDBName = sActDBName.GetToken( 0, DB_DELIM );
        String sTableName = sActDBName.GetToken( 1, DB_DELIM );
        aDatabaseLB.SelectEntry(sDBName);
        if (pSh->GetNewDBMgr()->GetTableNames(&aTableLB, sDBName))
        {
            aTableLB.SelectEntry(sTableName);
            pSh->GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sDBName, sTableName);
        }
        else
            aDBFieldLB.Clear();

    }
}

SfxTabPage* SwEnvPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvPage(pParent, rSet);
}

void SwEnvPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParent()->aEnvItem);
    Reset(aSet);
}

int SwEnvPage::DeactivatePage(SfxItemSet* _pSet)
{
    FillItem(GetParent()->aEnvItem);
    if( _pSet )
        FillItemSet(*_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvPage::FillItem(SwEnvItem& rItem)
{
    rItem.aAddrText = aAddrEdit  .GetText();
    rItem.bSend     = aSenderBox .IsChecked();
    rItem.aSendText = aSenderEdit.GetText();
}

sal_Bool SwEnvPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParent()->aEnvItem);
    rSet.Put(GetParent()->aEnvItem);
    return sal_True;
}

void SwEnvPage::Reset(const SfxItemSet& rSet)
{
    SwEnvItem aItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    aAddrEdit  .SetText(convertLineEnd(aItem.aAddrText, GetSystemLineEnd()));
    aSenderEdit.SetText(convertLineEnd(aItem.aSendText, GetSystemLineEnd()));
    aSenderBox .Check  (aItem.bSend);
    aSenderBox.GetClickHdl().Call(&aSenderBox);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
