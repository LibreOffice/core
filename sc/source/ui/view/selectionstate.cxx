/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selectionstate.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:08:14 $
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

#ifndef SC_SELECTIONSTATE_HXX
#include "selectionstate.hxx"
#endif

#include <svx/editview.hxx>

#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

// ============================================================================

ScSelectionState::ScSelectionState( ScViewData& rViewData ) :
    meType( SC_SELECTTYPE_NONE )
{
    maCursor.SetTab( rViewData.GetTabNo() );
    ScSplitPos eWhich = rViewData.GetActivePart();

    if( rViewData.HasEditView( eWhich ) )
    {
        meType = SC_SELECTTYPE_EDITCELL;
        maCursor.SetCol( rViewData.GetEditViewCol() );
        maCursor.SetRow( rViewData.GetEditViewRow() );
        maEditSel = rViewData.GetEditView( eWhich )->GetSelection();
    }
    else
    {
        maCursor.SetCol( rViewData.GetCurX() );
        maCursor.SetRow( rViewData.GetCurY() );

        ScMarkData& rMarkData = rViewData.GetMarkData();
        rMarkData.MarkToMulti();
        if( rMarkData.IsMultiMarked() )
        {
            meType = SC_SELECTTYPE_SHEET;
            rMarkData.FillRangeListWithMarks( &maSheetSel, FALSE );
        }
        // else type is SC_SELECTTYPE_NONE - already initialized
    }
}

bool operator==( const ScSelectionState& rL, const ScSelectionState& rR )
{
    bool bEqual = rL.GetSelectionType() == rR.GetSelectionType();
    if( bEqual ) switch( rL.GetSelectionType() )
    {
        case SC_SELECTTYPE_EDITCELL:
            bEqual &= rL.GetEditSelection().IsEqual( rR.GetEditSelection() );
        // run through!
        case SC_SELECTTYPE_SHEET:
            bEqual &= (rL.GetSheetSelection() == rR.GetSheetSelection()) == TRUE;
        // run through!
        case SC_SELECTTYPE_NONE:
            bEqual &= rL.GetCellCursor() == rR.GetCellCursor();
        break;
    }
    return bEqual;
}

// ============================================================================

