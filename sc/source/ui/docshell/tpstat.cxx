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

#include <document.hxx>
#include <docsh.hxx>

#include <tpstat.hxx>

// Dokumentinfo-Tabpage:

std::unique_ptr<SfxTabPage> ScDocStatPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<ScDocStatPage>( pPage, pController, *rSet );
}

ScDocStatPage::ScDocStatPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/statisticsinfopage.ui"_ustr, u"StatisticsInfoPage"_ustr, &rSet)
    , m_xFtTables(m_xBuilder->weld_label(u"nosheets"_ustr))
    , m_xFtCells(m_xBuilder->weld_label(u"nocells"_ustr))
    , m_xFtPages(m_xBuilder->weld_label(u"nopages"_ustr))
    , m_xFtFormula(m_xBuilder->weld_label(u"noformula"_ustr))
    , m_xFrame(m_xBuilder->weld_frame(u"StatisticsInfoPage"_ustr))
{
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current()  );
    ScDocStat   aDocStat;

    if ( pDocSh )
        pDocSh->GetDocStat( aDocStat );

    OUString aInfo = m_xFrame->get_label() + aDocStat.aDocName;
    m_xFrame->set_label(aInfo);
    m_xFtTables->set_label( OUString::number( aDocStat.nTableCount ) );
    m_xFtCells->set_label( OUString::number( aDocStat.nCellCount ) );
    m_xFtPages->set_label( OUString::number( aDocStat.nPageCount ) );
    m_xFtFormula->set_label( OUString::number( aDocStat.nFormulaCount ) );

}

ScDocStatPage::~ScDocStatPage()
{
}

bool ScDocStatPage::FillItemSet( SfxItemSet* /* rSet */ )
{
    return false;
}

void ScDocStatPage::Reset( const SfxItemSet* /* rSet */ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
