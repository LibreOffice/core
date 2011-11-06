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

#include "inscldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static sal_uInt8 nInsItemChecked=0;

//==================================================================

ScInsertCellDlg::ScInsertCellDlg( Window* pParent,sal_Bool bDisallowCellMove) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_INSCELL ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnCellsDown   ( this, ScResId( BTN_CELLSDOWN ) ),
    aBtnCellsRight  ( this, ScResId( BTN_CELLSRIGHT ) ),
    aBtnInsRows     ( this, ScResId( BTN_INSROWS ) ),
    aBtnInsCols     ( this, ScResId( BTN_INSCOLS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    if (bDisallowCellMove)
    {
        aBtnCellsDown.Disable();
        aBtnCellsRight.Disable();
        aBtnInsRows.Check();

        switch(nInsItemChecked)
        {
            case 2: aBtnInsRows   .Check();break;
            case 3: aBtnInsCols   .Check();break;
            default:aBtnInsRows   .Check();break;
        }
    }
    else
    {
        switch(nInsItemChecked)
        {
            case 0: aBtnCellsDown .Check();break;
            case 1: aBtnCellsRight.Check();break;
            case 2: aBtnInsRows   .Check();break;
            case 3: aBtnInsCols   .Check();break;
        }
    }
    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

InsCellCmd ScInsertCellDlg::GetInsCellCmd() const
{
    InsCellCmd nReturn = INS_NONE;

    if ( aBtnCellsDown.IsChecked() )
    {
        nInsItemChecked=0;
        nReturn = INS_CELLSDOWN;
    }
    else if ( aBtnCellsRight.IsChecked())
    {
        nInsItemChecked=1;
        nReturn = INS_CELLSRIGHT;
    }
    else if ( aBtnInsRows.IsChecked()   )
    {
        nInsItemChecked=2;
        nReturn = INS_INSROWS;
    }
    else if ( aBtnInsCols.IsChecked()   )
    {
        nInsItemChecked=3;
        nReturn = INS_INSCOLS;
    }

    return nReturn;
}

__EXPORT ScInsertCellDlg::~ScInsertCellDlg()
{
}



