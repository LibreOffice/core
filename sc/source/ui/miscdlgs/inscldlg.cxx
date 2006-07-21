/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inscldlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:06:14 $
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



//------------------------------------------------------------------

#include "inscldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static BYTE nInsItemChecked=0;

//==================================================================

ScInsertCellDlg::ScInsertCellDlg( Window* pParent,BOOL bDisallowCellMove) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_INSCELL ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnCellsDown   ( this, ScResId( BTN_CELLSDOWN ) ),
    aBtnCellsRight  ( this, ScResId( BTN_CELLSRIGHT ) ),
    aBtnInsRows     ( this, ScResId( BTN_INSROWS ) ),
    aBtnInsCols     ( this, ScResId( BTN_INSCOLS ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) )
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



