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
#include "selectionstate.hxx"

#include <editeng/editview.hxx>
#include "viewdata.hxx"

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
            rMarkData.FillRangeListWithMarks( &maSheetSel, false );
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
            bEqual &= ( rL.GetEditSelection().IsEqual( rR.GetEditSelection() ) != false );
        // run through!
        case SC_SELECTTYPE_SHEET:
            bEqual &= (rL.GetSheetSelection() == rR.GetSheetSelection()) == sal_True;
        // run through!
        case SC_SELECTTYPE_NONE:
            bEqual &= rL.GetCellCursor() == rR.GetCellCursor();
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    return bEqual;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
