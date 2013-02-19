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

#include <stdio.h>
#include <ctype.h>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <globals.hrc>
#include <docsh.hxx>
#include <pview.hxx>
#include <doc.hxx>
#include <docstdlg.hxx>
#include <modcfg.hxx>

// for statistics fields
#include <fldmgr.hxx>
#include <fldbas.hxx>

SfxTabPage *  SwDocStatPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwDocStatPage(pParent, rSet);
}

SwDocStatPage::SwDocStatPage(Window *pParent, const SfxItemSet &rSet)

    : SfxTabPage(pParent, "StatisticsInfoPage", "modules/swriter/ui/statisticsinfopage.ui", rSet)

{
    get(m_pPageNo, "nopages");
    get(m_pTableNo, "notables");
    get(m_pGrfNo, "nogrfs");
    get(m_pOLENo, "nooles");
    get(m_pParaNo, "noparas");
    get(m_pWordNo, "nowords");
    get(m_pCharNo, "nochars");
    get(m_pCharExclSpacesNo, "nocharsexspaces");

    get(m_pLineLbl, "lineft");
    get(m_pLineNo, "nolines");
    get(m_pUpdatePB, "update");

    Update();
    m_pUpdatePB->SetClickHdl(LINK(this, SwDocStatPage, UpdateHdl));
    //#111684# is the current view a page preview no SwFEShell can be found -> hide the update button
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(!pFEShell)
    {
        m_pUpdatePB->Show(sal_False);
        m_pLineLbl->Show(sal_False);
        m_pLineNo->Show(sal_False);
    }

}


 SwDocStatPage::~SwDocStatPage()
{
}

/*--------------------------------------------------------------------
    Description:    fill ItemSet when changed
 --------------------------------------------------------------------*/


sal_Bool  SwDocStatPage::FillItemSet(SfxItemSet & /*rSet*/)
{
    return sal_False;
}

void  SwDocStatPage::Reset(const SfxItemSet &/*rSet*/)
{
}
/*------------------------------------------------------------------------
 Description:   update / set data
------------------------------------------------------------------------*/


void SwDocStatPage::SetData(const SwDocStat &rStat)
{
    m_pTableNo->SetText(OUString::number( rStat.nTbl ));
    m_pGrfNo->SetText(OUString::number( rStat.nGrf ));
    m_pOLENo->SetText(OUString::number( rStat.nOLE ));
    m_pPageNo->SetText(OUString::number( rStat.nPage ));
    m_pParaNo->SetText(OUString::number( rStat.nPara ));
    m_pWordNo->SetText(OUString::number( rStat.nWord ));
    m_pCharNo->SetText(OUString::number( rStat.nChar ));
    m_pCharExclSpacesNo->SetText(OUString::number( rStat.nCharExcludingSpaces ));
}

/*------------------------------------------------------------------------
 Description:   update statistics
------------------------------------------------------------------------*/


void SwDocStatPage::Update()
{
    SfxViewShell *pVSh = SfxViewShell::Current();
    ViewShell *pSh = 0;
    if ( pVSh->ISA(SwView) )
        pSh = ((SwView*)pVSh)->GetWrtShellPtr();
    else if ( pVSh->ISA(SwPagePreView) )
        pSh = ((SwPagePreView*)pVSh)->GetViewShell();

    OSL_ENSURE( pSh, "Shell not found" );

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), sal_True );
    pSh->StartAction();
    aDocStat = pSh->GetDoc()->GetUpdatedDocStat();
    pSh->EndAction();

    SetData(aDocStat);
}

IMPL_LINK_NOARG(SwDocStatPage, UpdateHdl)
{
    Update();
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(pFEShell)
        m_pLineNo->SetText( OUString::number( pFEShell->GetLineCount(sal_False)));
    //pButton->Disable();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
