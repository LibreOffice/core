/*************************************************************************
 *
 *  $RCSfile: tpprint.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-05-29 19:40:15 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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


