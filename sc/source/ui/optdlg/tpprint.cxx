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

#include "tpprint.hxx"
#include "printopt.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "optdlg.hrc"

ScTpPrintOptions::ScTpPrintOptions( Window*           pParent,
                                    const SfxItemSet& rCoreAttrs )
    :   SfxTabPage      ( pParent,
                          "optCalcPrintPage",
                          "modules/scalc/ui/optdlg.ui",
                          rCoreAttrs )
{
    get( m_pSkipEmptyPagesCB , "suppressCB" );
    get( m_pSelectedSheetsCB , "printCB" );
    get( m_pForceBreaksCB, "forceBreaksCB" );
}

ScTpPrintOptions::~ScTpPrintOptions()
{
}

SfxTabPage* ScTpPrintOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new ScTpPrintOptions( pParent, rAttrSet );
}

int ScTpPrintOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

void ScTpPrintOptions::Reset( const SfxItemSet& rCoreSet )
{
    ScPrintOptions aOptions;

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCPRINTOPTIONS, false , &pItem))
        aOptions = ((const ScTpPrintItem*)pItem)->GetPrintOptions();
    else
    {
        // when called from print dialog and no options set, use configuration
        aOptions = SC_MOD()->GetPrintOptions();
    }

    if ( SFX_ITEM_SET == rCoreSet.GetItemState( SID_PRINT_SELECTEDSHEET, false , &pItem ) )
    {
        sal_Bool bChecked = ( (const SfxBoolItem*)pItem )->GetValue();
        m_pSelectedSheetsCB->Check( bChecked );
    }
    else
    {
        m_pSelectedSheetsCB->Check( !aOptions.GetAllSheets() );
    }

    m_pSkipEmptyPagesCB->Check( aOptions.GetSkipEmpty() );
    m_pSkipEmptyPagesCB->SaveValue();
    m_pSelectedSheetsCB->SaveValue();
    m_pForceBreaksCB->Check( aOptions.GetForceBreaks() );
    m_pForceBreaksCB->SaveValue();
}

bool ScTpPrintOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    rCoreAttrs.ClearItem( SID_PRINT_SELECTEDSHEET );

    bool bSkipEmptyChanged = ( m_pSkipEmptyPagesCB->GetSavedValue() != TriState(m_pSkipEmptyPagesCB->IsChecked()) );
    bool bSelectedSheetsChanged = ( m_pSelectedSheetsCB->GetSavedValue() != TriState(m_pSelectedSheetsCB->IsChecked()) );
    bool bForceBreaksChanged = ( m_pForceBreaksCB->GetSavedValue() != TriState(m_pForceBreaksCB->IsChecked()) );

    if ( bSkipEmptyChanged || bSelectedSheetsChanged || bForceBreaksChanged )
    {
        ScPrintOptions aOpt;
        aOpt.SetSkipEmpty( m_pSkipEmptyPagesCB->IsChecked() );
        aOpt.SetAllSheets( !m_pSelectedSheetsCB->IsChecked() );
        aOpt.SetForceBreaks( m_pForceBreaksCB->IsChecked() );
        rCoreAttrs.Put( ScTpPrintItem( SID_SCPRINTOPTIONS, aOpt ) );
        if ( bSelectedSheetsChanged )
        {
            rCoreAttrs.Put( SfxBoolItem( SID_PRINT_SELECTEDSHEET, m_pSelectedSheetsCB->IsChecked() ) );
        }
        return true;
    }
    else
    {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
