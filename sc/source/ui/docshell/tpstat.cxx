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


#undef SC_DLLIMPLEMENTATION



#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"

#include "tpstat.hxx"


//========================================================================
// Dokumentinfo-Tabpage:
//========================================================================

SfxTabPage* ScDocStatPage::Create( Window *pParent, const SfxItemSet& rSet )
{
    return new ScDocStatPage( pParent, rSet );
}

//------------------------------------------------------------------------

ScDocStatPage::ScDocStatPage( Window *pParent, const SfxItemSet& rSet )
    :   SfxTabPage( pParent, "StatisticsInfoPage", "modules/scalc/ui/statisticsinfopage.ui", rSet )
{
    get(m_pFtTables,"nosheets");
    get(m_pFtCells,"nocells");
    get(m_pFtPages,"nopages");
    ScDocShell* pDocSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
    ScDocStat   aDocStat;

    if ( pDocSh )
        pDocSh->GetDocStat( aDocStat );

    VclFrame *pFrame = get<VclFrame>("StatisticsInfoPage");
    String aInfo = pFrame->get_label();
    aInfo += aDocStat.aDocName;
    pFrame->set_label(aInfo);
    m_pFtTables   ->SetText( OUString::number( aDocStat.nTableCount ) );
    m_pFtCells    ->SetText( OUString::number( aDocStat.nCellCount ) );
    m_pFtPages    ->SetText( OUString::number( aDocStat.nPageCount ) );

}

//------------------------------------------------------------------------

ScDocStatPage::~ScDocStatPage()
{
}

//------------------------------------------------------------------------

sal_Bool ScDocStatPage::FillItemSet( SfxItemSet& /* rSet */ )
{
    return false;
}

//------------------------------------------------------------------------

void ScDocStatPage::Reset( const SfxItemSet& /* rSet */ )
{
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
