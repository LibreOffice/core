/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpprint.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:17:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



#include "tpprint.hxx"
#include "printopt.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "optdlg.hrc"

// -----------------------------------------------------------------------

static USHORT pPrintOptRanges[] =
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
        aPagesFL         ( this, ResId( FL_PAGES ) ),
        aSkipEmptyPagesCB( this, ResId( BTN_SKIPEMPTYPAGES ) ),
        aSheetsFL        ( this, ResId( FL_SHEETS ) ),
        aSelectedSheetsCB( this, ResId( BTN_SELECTEDSHEETS ) )
{
    FreeResource();
}

ScTpPrintOptions::~ScTpPrintOptions()
{
}

USHORT* ScTpPrintOptions::GetRanges()
{
    return pPrintOptRanges;
}

SfxTabPage* ScTpPrintOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new ScTpPrintOptions( pParent, rAttrSet );
}

int ScTpPrintOptions::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void ScTpPrintOptions::Reset( const SfxItemSet& rCoreSet )
{
    ScPrintOptions aOptions;

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCPRINTOPTIONS, FALSE , &pItem))
        aOptions = ((const ScTpPrintItem*)pItem)->GetPrintOptions();
    else
    {
        // when called from print dialog and no options set, use configuration
        aOptions = SC_MOD()->GetPrintOptions();
    }

    aSkipEmptyPagesCB.Check( aOptions.GetSkipEmpty() );
    aSelectedSheetsCB.Check( !aOptions.GetAllSheets() );
    aSkipEmptyPagesCB.SaveValue();
    aSelectedSheetsCB.SaveValue();
}

// -----------------------------------------------------------------------

BOOL ScTpPrintOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    if (   aSkipEmptyPagesCB.GetSavedValue() != aSkipEmptyPagesCB.IsChecked()
        || aSelectedSheetsCB.GetSavedValue() != aSelectedSheetsCB.IsChecked() )
    {
        ScPrintOptions aOpt;

        aOpt.SetSkipEmpty( aSkipEmptyPagesCB.IsChecked() );
        aOpt.SetAllSheets( !aSelectedSheetsCB.IsChecked() );

        rCoreAttrs.Put( ScTpPrintItem( SID_SCPRINTOPTIONS, aOpt ) );
        return TRUE;
    }
    else
        return FALSE;
}


