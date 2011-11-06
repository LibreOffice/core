/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            rMarkData.FillRangeListWithMarks( &maSheetSel, sal_False );
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
            bEqual &= ( rL.GetEditSelection().IsEqual( rR.GetEditSelection() ) != sal_False );
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

