/*************************************************************************
 *
 *  $RCSfile: envlop1.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 11:24:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "dbmgr.hxx"
#ifndef _SBA_SBAOBJ_HXX //autogen
#include <offmgr/sbaobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#include "wrtsh.hxx"
#include "errhdl.hxx"
#include "cmdid.h"
#include "helpid.h"
#include "envfmt.hxx"
#include "envlop.hxx"
#include "envprt.hxx"
#include "fmtcol.hxx"
#include "poolfmt.hxx"
#include "view.hxx"

#include "envlop.hrc"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace rtl;
#define C2U(char) rtl::OUString::createFromAscii(char)

// --------------------------------------------------------------------------



SwEnvPreview::SwEnvPreview(SfxTabPage* pParent, const ResId& rResID) :

    Window(pParent, rResID)

{
    SetMapMode(MapMode(MAP_PIXEL));
}

// --------------------------------------------------------------------------



SwEnvPreview::~SwEnvPreview()
{
}

// ----------------------------------------------------------------------------



void SwEnvPreview::Paint(const Rectangle &)
{
    const SwEnvItem& rItem =
        ((SwEnvDlg*) GetParent()->GetParent()->GetParent())->aEnvItem;

    USHORT nPageW = (USHORT) Max(rItem.lWidth, rItem.lHeight),
           nPageH = (USHORT) Min(rItem.lWidth, rItem.lHeight);

    float fx = (float) GetOutputSizePixel().Width () / nPageW,
          fy = (float) GetOutputSizePixel().Height() / nPageH,
          f  = fx < fy ? fx : fy;

    // Umschlag
    long   nW = (USHORT) (f * nPageW),
           nH = (USHORT) (f * nPageH),
           nX = (GetOutputSizePixel().Width () - nW) / 2,
           nY = (GetOutputSizePixel().Height() - nH) / 2;
    SetFillColor(Color(COL_WHITE));
    DrawRect(Rectangle(Point(nX, nY), Size(nW, nH)));

    // Absender
    if (rItem.bSend)
    {
        long   nSendX = nX + (USHORT) (f * rItem.lSendFromLeft),
               nSendY = nY + (USHORT) (f * rItem.lSendFromTop ),
               nSendW = (USHORT) (f * (rItem.lAddrFromLeft - rItem.lSendFromLeft)),
               nSendH = (USHORT) (f * (rItem.lAddrFromTop  - rItem.lSendFromTop  - 566));
        SetFillColor(Color(COL_GRAY));
        DrawRect(Rectangle(Point(nSendX, nSendY), Size(nSendW, nSendH)));
    }

    // Empfaenger
    long   nAddrX = nX + (USHORT) (f * rItem.lAddrFromLeft),
           nAddrY = nY + (USHORT) (f * rItem.lAddrFromTop ),
           nAddrW = (USHORT) (f * (nPageW - rItem.lAddrFromLeft - 566)),
           nAddrH = (USHORT) (f * (nPageH - rItem.lAddrFromTop  - 566));
    SetFillColor(Color(COL_GRAY));
    DrawRect(Rectangle(Point(nAddrX, nAddrY), Size(nAddrW, nAddrH)));

    // Briefmarke
    long   nStmpW = (USHORT) (f * 1417 /* 2,5 cm */),
           nStmpH = (USHORT) (f * 1701 /* 3,0 cm */),
           nStmpX = nX + nW - (USHORT) (f * 566) - nStmpW,
           nStmpY = nY + (USHORT) (f * 566);

    SetFillColor(Color(COL_WHITE));
    DrawRect(Rectangle(Point(nStmpX, nStmpY), Size(nStmpW, nStmpH)));
}

// --------------------------------------------------------------------------



SwEnvDlg::SwEnvDlg(Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell* pWrtSh, Printer* pPrt, BOOL bInsert) :

    SfxTabDialog(pParent, SW_RES(DLG_ENV), &rSet, FALSE, &aEmptyStr),
    sInsert(ResId(ST_INSERT)),
    sChange(ResId(ST_CHANGE)),
    aEnvItem((const SwEnvItem&) rSet.Get(FN_ENVELOP)),
    pSh(pWrtSh),
    pPrinter(pPrt),
    pAddresseeSet(0),
    pSenderSet(0)
{
    FreeResource();

    GetOKButton().SetText(String(SW_RES(STR_BTN_NEWDOC)));
    GetOKButton().SetHelpId(HID_ENVELOP_PRINT);
    GetOKButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird
    if (GetUserButton())
    {
        GetUserButton()->SetText(bInsert ? sInsert : sChange);
        GetUserButton()->SetHelpId(HID_ENVELOP_INSERT);
    }

    AddTabPage(TP_ENV_ENV, SwEnvPage   ::Create, 0);
    AddTabPage(TP_ENV_FMT, SwEnvFmtPage::Create, 0);
    AddTabPage(TP_ENV_PRT, SwEnvPrtPage::Create, 0);
}

// --------------------------------------------------------------------------



SwEnvDlg::~SwEnvDlg()
{
    delete pAddresseeSet;
    delete pSenderSet;
}

// --------------------------------------------------------------------------



void SwEnvDlg::PageCreated(USHORT nId, SfxTabPage &rPage)
{
    if (nId == TP_ENV_PRT)
    {
        ((SwEnvPrtPage*)&rPage)->SetPrt(pPrinter);
    }
}

// --------------------------------------------------------------------------

short SwEnvDlg::Ok()
{
    short nRet = SfxTabDialog::Ok();

    if (nRet == RET_OK || nRet == RET_USER)
    {
        if (pAddresseeSet)
        {
            SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool(RES_POOLCOLL_JAKETADRESS);
            pColl->SetAttr(*pAddresseeSet);
        }
        if (pSenderSet)
        {
            SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool(RES_POOLCOLL_SENDADRESS);
            pColl->SetAttr(*pSenderSet);
        }
    }

    return nRet;
}

// --------------------------------------------------------------------------



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

    // Handler installieren
    aDatabaseLB    .SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    aTableLB       .SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    aInsertBT      .SetClickHdl (LINK(this, SwEnvPage, FieldHdl        ));
    aSenderBox     .SetClickHdl (LINK(this, SwEnvPage, SenderHdl       ));

    sActDBName = pSh->GetDBName();
    InitDatabaseBox();
}

// --------------------------------------------------------------------------



SwEnvPage::~SwEnvPage()
{
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPage, DatabaseHdl, ListBox *, pListBox )
{
    SwWait aWait( *pSh->GetView().GetDocShell(), TRUE );

    if (pListBox == &aDatabaseLB)
        pSh->GetNewDBMgr()->GetTableNames(&aTableLB, sActDBName);
    pSh->GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, aTableLB.GetSelectEntry());
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPage, FieldHdl, Button *, EMPTYARG )
{
    String aStr ( '<' );
    aStr += aDatabaseLB.GetSelectEntry();
    aStr += '.';
//  aStr += DB_DELIM;
    aStr += aTableLB.GetSelectEntry();
    aStr += '.';
//  aStr += DB_DELIM;
    aStr += aDBFieldLB.GetSelectEntry();
    aStr += '>';
    aAddrEdit.ReplaceSelected(aStr);
    Selection aSel = aAddrEdit.GetSelection();
    aAddrEdit.GrabFocus();
    aAddrEdit.SetSelection(aSel);
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPage, SenderHdl, Button *, EMPTYARG )
{
    const BOOL bEnable = aSenderBox.IsChecked();
    GetParent()->aEnvItem.bSend = bEnable;
    aSenderEdit.Enable(bEnable);
    if ( bEnable )
        aSenderEdit.GrabFocus();
    aPreview.Invalidate();
    return 0;
}

// --------------------------------------------------------------------------



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
            pSh->GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, sTableName);
        }
        else
            aDBFieldLB.Clear();

    }
}

// --------------------------------------------------------------------------



SfxTabPage* SwEnvPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvPage(pParent, rSet);
}

// --------------------------------------------------------------------------



void SwEnvPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParent()->aEnvItem);
    Reset(aSet);
}

// --------------------------------------------------------------------------



int SwEnvPage::DeactivatePage(SfxItemSet* pSet)
{
    FillItem(GetParent()->aEnvItem);
    FillItemSet(*pSet);
    return SfxTabPage::LEAVE_PAGE;
}

// --------------------------------------------------------------------------



void SwEnvPage::FillItem(SwEnvItem& rItem)
{
    rItem.aAddrText = aAddrEdit  .GetText();
    rItem.bSend     = aSenderBox .IsChecked();
    rItem.aSendText = aSenderEdit.GetText();
}

// --------------------------------------------------------------------------



BOOL SwEnvPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParent()->aEnvItem);
    rSet.Put(GetParent()->aEnvItem);
    return TRUE;
}

// ----------------------------------------------------------------------------



void SwEnvPage::Reset(const SfxItemSet& rSet)
{
    SwEnvItem aItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    aAddrEdit  .SetText(String(aItem.aAddrText).ConvertLineEnd());
    aSenderEdit.SetText(String(aItem.aSendText).ConvertLineEnd());
    aSenderBox .Check  (aItem.bSend);
    aSenderBox.GetClickHdl().Call(&aSenderBox);
}


