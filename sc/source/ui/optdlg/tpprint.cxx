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

#include <svl/eitem.hxx>

#include <tpprint.hxx>
#include <printopt.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <officecfg/Office/Calc.hxx>

ScTpPrintOptions::ScTpPrintOptions( weld::Container* pPage, weld::DialogController* pController,
                                    const SfxItemSet& rCoreAttrs )
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optdlg.ui"_ustr, u"optCalcPrintPage"_ustr, &rCoreAttrs )
    , m_xSkipEmptyPagesCB(m_xBuilder->weld_check_button(u"suppressCB"_ustr))
    , m_xSkipEmptyPagesImg(m_xBuilder->weld_widget(u"locksuppressCB"_ustr))
    , m_xSelectedSheetsCB(m_xBuilder->weld_check_button(u"printCB"_ustr))
    , m_xSelectedSheetsImg(m_xBuilder->weld_widget(u"lockprintCB"_ustr))
    , m_xForceBreaksCB(m_xBuilder->weld_check_button(u"forceBreaksCB"_ustr))
    , m_xForceBreaksImg(m_xBuilder->weld_widget(u"lockforceBreaksCB"_ustr))
{
}

ScTpPrintOptions::~ScTpPrintOptions()
{
}

std::unique_ptr<SfxTabPage> ScTpPrintOptions::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<ScTpPrintOptions>(pPage, pController, *rAttrSet);
}

DeactivateRC ScTpPrintOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

void ScTpPrintOptions::Reset( const SfxItemSet* rCoreSet )
{
    ScPrintOptions aOptions;

    if(const ScTpPrintItem* pItem = rCoreSet->GetItemIfSet(SID_SCPRINTOPTIONS, false))
        aOptions = pItem->GetPrintOptions();
    else
    {
        // when called from print dialog and no options set, use configuration
        aOptions = SC_MOD()->GetPrintOptions();
    }

    if ( const SfxBoolItem* pItem = rCoreSet->GetItemIfSet( SID_PRINT_SELECTEDSHEET, false))
    {
        bool bChecked = pItem->GetValue();
        m_xSelectedSheetsCB->set_active( bChecked );
    }
    else
    {
        m_xSelectedSheetsCB->set_active( !aOptions.GetAllSheets() );
    }

    m_xSkipEmptyPagesCB->set_active( aOptions.GetSkipEmpty() );
    m_xForceBreaksCB->set_active(aOptions.GetForceBreaks());

    m_xSkipEmptyPagesCB->set_sensitive(!officecfg::Office::Calc::Print::Page::EmptyPages::isReadOnly());
    m_xSkipEmptyPagesImg->set_visible(officecfg::Office::Calc::Print::Page::EmptyPages::isReadOnly());
    m_xSelectedSheetsCB->set_sensitive(!officecfg::Office::Calc::Print::Other::AllSheets::isReadOnly());
    m_xSelectedSheetsImg->set_visible(officecfg::Office::Calc::Print::Other::AllSheets::isReadOnly());
    m_xForceBreaksCB->set_sensitive(!officecfg::Office::Calc::Print::Page::ForceBreaks::isReadOnly());
    m_xForceBreaksImg->set_visible(officecfg::Office::Calc::Print::Page::ForceBreaks::isReadOnly());

    m_xSkipEmptyPagesCB->save_state();
    m_xSelectedSheetsCB->save_state();
    m_xForceBreaksCB->save_state();
}

OUString ScTpPrintOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"suppressCB"_ustr, u"forceBreaksCB"_ustr, u"printCB"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool ScTpPrintOptions::FillItemSet( SfxItemSet* rCoreAttrs )
{
    rCoreAttrs->ClearItem( SID_PRINT_SELECTEDSHEET );

    bool bSkipEmptyChanged = m_xSkipEmptyPagesCB->get_state_changed_from_saved();
    bool bSelectedSheetsChanged = m_xSelectedSheetsCB->get_state_changed_from_saved();
    bool bForceBreaksChanged = m_xForceBreaksCB->get_state_changed_from_saved();

    if ( bSkipEmptyChanged || bSelectedSheetsChanged || bForceBreaksChanged )
    {
        ScPrintOptions aOpt;
        aOpt.SetSkipEmpty( m_xSkipEmptyPagesCB->get_active() );
        aOpt.SetAllSheets( !m_xSelectedSheetsCB->get_active() );
        aOpt.SetForceBreaks( m_xForceBreaksCB->get_active() );
        rCoreAttrs->Put( ScTpPrintItem( aOpt ) );
        if ( bSelectedSheetsChanged )
        {
            rCoreAttrs->Put( SfxBoolItem( SID_PRINT_SELECTEDSHEET, m_xSelectedSheetsCB->get_active() ) );
        }
        return true;
    }
    else
    {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
