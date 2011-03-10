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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

#include <svl/eitem.hxx>

#include "tpprint.hxx"
#include "printopt.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "optdlg.hrc"

// -----------------------------------------------------------------------

static sal_uInt16 pPrintOptRanges[] =
{
    SID_SCPRINTOPTIONS,
    SID_SCPRINTOPTIONS,
    0
};

// -----------------------------------------------------------------------

ScTpPrintOptions::ScTpPrintOptions( Window*           pParent,
                                    const SfxItemSet& rCoreAttrs )
    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_PRINT ),
                          rCoreAttrs ),
        aPagesFL         ( this, ScResId( FL_PAGES ) ),
        aSkipEmptyPagesCB( this, ScResId( BTN_SKIPEMPTYPAGES ) ),
        aSheetsFL        ( this, ScResId( FL_SHEETS ) ),
        aSelectedSheetsCB( this, ScResId( BTN_SELECTEDSHEETS ) )
{
    FreeResource();
}

ScTpPrintOptions::~ScTpPrintOptions()
{
}

sal_uInt16* ScTpPrintOptions::GetRanges()
{
    return pPrintOptRanges;
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

// -----------------------------------------------------------------------

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
        aSelectedSheetsCB.Check( bChecked );
    }
    else
    {
        aSelectedSheetsCB.Check( !aOptions.GetAllSheets() );
    }

    aSkipEmptyPagesCB.Check( aOptions.GetSkipEmpty() );
    aSkipEmptyPagesCB.SaveValue();
    aSelectedSheetsCB.SaveValue();
}

// -----------------------------------------------------------------------

sal_Bool ScTpPrintOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    rCoreAttrs.ClearItem( SID_PRINT_SELECTEDSHEET );

    bool bSkipEmptyChanged = ( aSkipEmptyPagesCB.GetSavedValue() != aSkipEmptyPagesCB.IsChecked() );
    bool bSelectedSheetsChanged = ( aSelectedSheetsCB.GetSavedValue() != aSelectedSheetsCB.IsChecked() );

    if ( bSkipEmptyChanged || bSelectedSheetsChanged )
    {
        ScPrintOptions aOpt;
        aOpt.SetSkipEmpty( aSkipEmptyPagesCB.IsChecked() );
        aOpt.SetAllSheets( !aSelectedSheetsCB.IsChecked() );
        rCoreAttrs.Put( ScTpPrintItem( SID_SCPRINTOPTIONS, aOpt ) );
        if ( bSelectedSheetsChanged )
        {
            rCoreAttrs.Put( SfxBoolItem( SID_PRINT_SELECTEDSHEET, aSelectedSheetsCB.IsChecked() ) );
        }
        return sal_True;
    }
    else
    {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
