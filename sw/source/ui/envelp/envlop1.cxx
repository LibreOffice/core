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

#include "dbmgr.hxx"
#include <sfx2/app.hxx>
#include <vcl/builder.hxx>
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

#include <comphelper/processfactory.hxx>

#include <unomid.h>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;


//impl in envimg.cxx
extern SW_DLLPUBLIC String MakeSender();

SwEnvPreview::SwEnvPreview(Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
{
    SetMapMode(MapMode(MAP_PIXEL));
}

Size SwEnvPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(84 , 63), MAP_APPFONT);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwEnvPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new SwEnvPreview(pParent, 0);
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
        ((SwEnvDlg*) GetParentDialog())->aEnvItem;

    sal_uInt16 nPageW = (sal_uInt16) std::max(rItem.lWidth, rItem.lHeight),
           nPageH = (sal_uInt16) std::min(rItem.lWidth, rItem.lHeight);

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
                    SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert)
    : SfxTabDialog(pParent, "EnvDialog",
        "modules/swriter/ui/envdialog.ui", &rSet)
    , aEnvItem((const SwEnvItem&) rSet.Get(FN_ENVELOP))
    , pSh(pWrtSh)
    , pPrinter(pPrt)
    , pAddresseeSet(0)
    , pSenderSet(0)
    , m_nEnvPrintId(0)
{
    if (!bInsert)
    {
        GetUserButton()->SetText(get<PushButton>("modify")->GetText());
    }

    AddTabPage("envelope", SwEnvPage   ::Create, 0);
    AddTabPage("format", SwEnvFmtPage::Create, 0);
    m_nEnvPrintId = AddTabPage("printer", SwEnvPrtPage::Create, 0);
}

SwEnvDlg::~SwEnvDlg()
{
    delete pAddresseeSet;
    delete pSenderSet;
}

void SwEnvDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == m_nEnvPrintId)
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

SwEnvPage::SwEnvPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "EnvAddressPage",
        "modules/swriter/ui/envaddresspage.ui", rSet)
{
    get(m_pAddrEdit, "addredit");
    get(m_pDatabaseLB, "database");
    get(m_pTableLB, "table");
    get(m_pDBFieldLB, "field");
    get(m_pInsertBT, "insert");
    get(m_pSenderBox, "sender");
    get(m_pSenderEdit, "senderedit");
    get(m_pPreview, "preview");

    long nTextBoxHeight(m_pAddrEdit->GetTextHeight() * 10);
    long nTextBoxWidth(m_pAddrEdit->approximate_char_width() * 25);

    m_pAddrEdit->set_height_request(nTextBoxHeight);
    m_pAddrEdit->set_width_request(nTextBoxWidth);
    m_pSenderEdit->set_height_request(nTextBoxHeight);
    m_pSenderEdit->set_width_request(nTextBoxWidth);

    long nListBoxWidth = approximate_char_width() * 30;
    m_pTableLB->set_width_request(nListBoxWidth);
    m_pDatabaseLB->set_width_request(nListBoxWidth);
    m_pDBFieldLB->set_width_request(nListBoxWidth);

    SetExchangeSupport();
    pSh = GetParentSwEnvDlg()->pSh;

    // Install handlers
    m_pDatabaseLB->SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    m_pTableLB->SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    m_pInsertBT->SetClickHdl (LINK(this, SwEnvPage, FieldHdl        ));
    m_pSenderBox->SetClickHdl (LINK(this, SwEnvPage, SenderHdl       ));
    m_pPreview->SetBorderStyle( WINDOW_BORDER_MONO );

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

    if (pListBox == m_pDatabaseLB)
    {
        sActDBName = pListBox->GetSelectEntry();
        pSh->GetNewDBMgr()->GetTableNames(m_pTableLB, sActDBName);
        sActDBName += DB_DELIM;
    }
    else
        sActDBName.SetToken(1, DB_DELIM, m_pTableLB->GetSelectEntry());
    pSh->GetNewDBMgr()->GetColumnNames(m_pDBFieldLB, m_pDatabaseLB->GetSelectEntry(),
                                       m_pTableLB->GetSelectEntry());
    return 0;
}

IMPL_LINK_NOARG(SwEnvPage, FieldHdl)
{
    OUString aStr("<" + m_pDatabaseLB->GetSelectEntry() + "." +
                  m_pTableLB->GetSelectEntry() + "." +
                  OUString(m_pTableLB->GetEntryData(m_pTableLB->GetSelectEntryPos()) == 0 ? '0' : '1') + "." +
                  m_pDBFieldLB->GetSelectEntry() + ">");
    m_pAddrEdit->ReplaceSelected(aStr);
    Selection aSel = m_pAddrEdit->GetSelection();
    m_pAddrEdit->GrabFocus();
    m_pAddrEdit->SetSelection(aSel);
    return 0;
}

IMPL_LINK_NOARG(SwEnvPage, SenderHdl)
{
    const sal_Bool bEnable = m_pSenderBox->IsChecked();
    GetParentSwEnvDlg()->aEnvItem.bSend = bEnable;
    m_pSenderEdit->Enable(bEnable);
    if ( bEnable )
    {
        m_pSenderEdit->GrabFocus();
        if(m_pSenderEdit->GetText().isEmpty())
            m_pSenderEdit->SetText(MakeSender());
    }
    m_pPreview->Invalidate();
    return 0;
}

void SwEnvPage::InitDatabaseBox()
{
    if (pSh->GetNewDBMgr())
    {
        m_pDatabaseLB->Clear();
        Sequence<OUString> aDataNames = SwNewDBMgr::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();

        for (long i = 0; i < aDataNames.getLength(); i++)
            m_pDatabaseLB->InsertEntry(pDataNames[i]);

        String sDBName = sActDBName.GetToken( 0, DB_DELIM );
        String sTableName = sActDBName.GetToken( 1, DB_DELIM );
        m_pDatabaseLB->SelectEntry(sDBName);
        if (pSh->GetNewDBMgr()->GetTableNames(m_pTableLB, sDBName))
        {
            m_pTableLB->SelectEntry(sTableName);
            pSh->GetNewDBMgr()->GetColumnNames(m_pDBFieldLB, sDBName, sTableName);
        }
        else
            m_pDBFieldLB->Clear();

    }
}

SfxTabPage* SwEnvPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvPage(pParent, rSet);
}

void SwEnvPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParentSwEnvDlg()->aEnvItem);
    Reset(aSet);
}

int SwEnvPage::DeactivatePage(SfxItemSet* _pSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    if( _pSet )
        FillItemSet(*_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvPage::FillItem(SwEnvItem& rItem)
{
    rItem.aAddrText = m_pAddrEdit->GetText();
    rItem.bSend     = m_pSenderBox->IsChecked();
    rItem.aSendText = m_pSenderEdit->GetText();
}

sal_Bool SwEnvPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    rSet.Put(GetParentSwEnvDlg()->aEnvItem);
    return sal_True;
}

void SwEnvPage::Reset(const SfxItemSet& rSet)
{
    SwEnvItem aItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    m_pAddrEdit->SetText(convertLineEnd(aItem.aAddrText, GetSystemLineEnd()));
    m_pSenderEdit->SetText(convertLineEnd(aItem.aSendText, GetSystemLineEnd()));
    m_pSenderBox->Check  (aItem.bSend);
    m_pSenderBox->GetClickHdl().Call(m_pSenderBox);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
