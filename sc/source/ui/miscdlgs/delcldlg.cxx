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

#include "delcldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static sal_uInt8 nDelItemChecked=0;

//==================================================================

ScDeleteCellDlg::ScDeleteCellDlg( Window* pParent, sal_Bool bDisallowCellMove ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DELCELL ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnCellsUp     ( this, ScResId( BTN_CELLSUP ) ),
    aBtnCellsLeft   ( this, ScResId( BTN_CELLSLEFT ) ),
    aBtnDelRows     ( this, ScResId( BTN_DELROWS ) ),
    aBtnDelCols     ( this, ScResId( BTN_DELCOLS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{

    if (bDisallowCellMove)
    {
        aBtnCellsUp.Disable();
        aBtnCellsLeft.Disable();

        switch(nDelItemChecked)
        {
            case 2: aBtnDelRows.Check();break;
            case 3: aBtnDelCols.Check();break;
            default:aBtnDelRows.Check();break;
        }
    }
    else
    {
        switch(nDelItemChecked)
        {
            case 0: aBtnCellsUp.Check();break;
            case 1: aBtnCellsLeft.Check();break;
            case 2: aBtnDelRows.Check();break;
            case 3: aBtnDelCols.Check();break;
        }
    }

    FreeResource();
}

//------------------------------------------------------------------------

DelCellCmd ScDeleteCellDlg::GetDelCellCmd() const
{
    DelCellCmd nReturn = DEL_NONE;

    if ( aBtnCellsUp.IsChecked()   )
    {
        nDelItemChecked=0;
        nReturn = DEL_CELLSUP;
    }
    else if ( aBtnCellsLeft.IsChecked() )
    {
        nDelItemChecked=1;
        nReturn = DEL_CELLSLEFT;
    }
    else if ( aBtnDelRows.IsChecked()   )
    {
        nDelItemChecked=2;
        nReturn = DEL_DELROWS;
    }
    else if ( aBtnDelCols.IsChecked()   )
    {
        nDelItemChecked=3;
        nReturn = DEL_DELCOLS;
    }

    return nReturn;
}

__EXPORT ScDeleteCellDlg::~ScDeleteCellDlg()
{
}



