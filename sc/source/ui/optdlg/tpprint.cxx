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

ScTpPrintOptions::ScTpPrintOptions( TabPageParent pPage,
                                    const SfxItemSet& rCoreAttrs )
    : SfxTabPage(pPage, "modules/scalc/ui/optdlg.ui", "optCalcPrintPage", &rCoreAttrs )
    , m_xSkipEmptyPagesCB(m_xBuilder->weld_check_button("suppressCB"))
    , m_xSelectedSheetsCB(m_xBuilder->weld_check_button("printCB"))
    , m_xForceBreaksCB(m_xBuilder->weld_check_button("forceBreaksCB"))
{
}

ScTpPrintOptions::~ScTpPrintOptions()
{
}

VclPtr<SfxTabPage> ScTpPrintOptions::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<ScTpPrintOptions>::Create(pParent, *rAttrSet);
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

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCPRINTOPTIONS, false , &pItem))
        aOptions = static_cast<const ScTpPrintItem*>(pItem)->GetPrintOptions();
    else
    {
        // when called from print dialog and no options set, use configuration
        aOptions = SC_MOD()->GetPrintOptions();
    }

    if ( SfxItemState::SET == rCoreSet->GetItemState( SID_PRINT_SELECTEDSHEET, false , &pItem ) )
    {
        bool bChecked = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_xSelectedSheetsCB->set_active( bChecked );
    }
    else
    {
        m_xSelectedSheetsCB->set_active( !aOptions.GetAllSheets() );
    }

    m_xSkipEmptyPagesCB->set_active( aOptions.GetSkipEmpty() );
    m_xSkipEmptyPagesCB->save_state();
    m_xSelectedSheetsCB->save_state();
    m_xForceBreaksCB->set_active( aOptions.GetForceBreaks() );
    m_xForceBreaksCB->save_state();
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
