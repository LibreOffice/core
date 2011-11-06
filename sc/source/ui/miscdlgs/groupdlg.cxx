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

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "groupdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScGroupDlg::ScGroupDlg( Window* pParent,
                        sal_uInt16  nResId,
                        sal_Bool    bUngroup,
                        sal_Bool    bRows ) :
    ModalDialog     ( pParent, ScResId( nResId ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnRows        ( this, ScResId( BTN_GROUP_ROWS ) ),
    aBtnCols        ( this, ScResId( BTN_GROUP_COLS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    aFlFrame.SetText( String( ScResId(bUngroup ? STR_UNGROUP : STR_GROUP) ) );

    if ( bRows )
        aBtnRows.Check();
    else
        aBtnCols.Check();

    //-------------
    FreeResource();
    aBtnRows.GrabFocus();
}

//------------------------------------------------------------------------

sal_Bool ScGroupDlg::GetColsChecked() const
{
    return aBtnCols.IsChecked();
}

//------------------------------------------------------------------------

__EXPORT ScGroupDlg::~ScGroupDlg()
{
}



