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
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <IDocumentStatistics.hxx>

#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>

VclPtr<SfxTabPage> SwDocStatPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwDocStatPage>::Create(pParent, *rSet);
}

SwDocStatPage::SwDocStatPage(vcl::Window *pParent, const SfxItemSet &rSet)

    : SfxTabPage(pParent, "StatisticsInfoPage", "modules/swriter/ui/statisticsinfopage.ui", &rSet)

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
    SwDocShell* pDocShell = static_cast<SwDocShell*>( SfxObjectShell::Current() );
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(!pFEShell)
    {
        m_pUpdatePB->Show(false);
        m_pLineLbl->Show(false);
        m_pLineNo->Show(false);
    }

}

SwDocStatPage::~SwDocStatPage()
{
    disposeOnce();
}

void SwDocStatPage::dispose()
{
    m_pPageNo.clear();
    m_pTableNo.clear();
    m_pGrfNo.clear();
    m_pOLENo.clear();
    m_pParaNo.clear();
    m_pWordNo.clear();
    m_pCharNo.clear();
    m_pCharExclSpacesNo.clear();
    m_pLineLbl.clear();
    m_pLineNo.clear();
    m_pUpdatePB.clear();
    SfxTabPage::dispose();
}

// Description: fill ItemSet when changed
bool  SwDocStatPage::FillItemSet(SfxItemSet * /*rSet*/)
{
    return false;
}

void  SwDocStatPage::Reset(const SfxItemSet *)
{
}

// Description: update / set data
void SwDocStatPage::SetData(const SwDocStat &rStat)
{
    const LocaleDataWrapper& rLocaleData = GetSettings().GetUILocaleDataWrapper();
    m_pTableNo->SetText(rLocaleData.getNum(rStat.nTable, 0));
    m_pGrfNo->SetText(rLocaleData.getNum(rStat.nGrf, 0));
    m_pOLENo->SetText(rLocaleData.getNum(rStat.nOLE, 0));
    m_pPageNo->SetText(rLocaleData.getNum(rStat.nPage, 0));
    m_pParaNo->SetText(rLocaleData.getNum(rStat.nPara, 0));
    m_pWordNo->SetText(rLocaleData.getNum(rStat.nWord, 0));
    m_pCharNo->SetText(rLocaleData.getNum(rStat.nChar, 0));
    m_pCharExclSpacesNo->SetText(rLocaleData.getNum(rStat.nCharExcludingSpaces, 0));
}

// Description: update statistics
void SwDocStatPage::Update()
{
    SfxViewShell *pVSh = SfxViewShell::Current();
    SwViewShell *pSh = nullptr;
    if ( dynamic_cast< const SwView *>( pVSh ) !=  nullptr )
        pSh = static_cast<SwView*>(pVSh)->GetWrtShellPtr();
    else if ( dynamic_cast< const SwPagePreview *>( pVSh ) !=  nullptr )
        pSh = static_cast<SwPagePreview*>(pVSh)->GetViewShell();

    OSL_ENSURE( pSh, "Shell not found" );

    if (!pSh)
        return;

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), true );
    pSh->StartAction();
    aDocStat = pSh->GetDoc()->getIDocumentStatistics().GetUpdatedDocStat( false, true );
    pSh->EndAction();

    SetData(aDocStat);
}

IMPL_LINK_NOARG_TYPED(SwDocStatPage, UpdateHdl, Button*, void)
{
    Update();
    SwDocShell* pDocShell = static_cast<SwDocShell*>( SfxObjectShell::Current());
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(pFEShell)
        m_pLineNo->SetText( OUString::number( pFEShell->GetLineCount(false)));
    //pButton->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
