/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delcldlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:04:33 $
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

#include "delcldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static BYTE nDelItemChecked=0;

//==================================================================

ScDeleteCellDlg::ScDeleteCellDlg( Window* pParent, BOOL bDisallowCellMove ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DELCELL ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnCellsUp     ( this, ScResId( BTN_CELLSUP ) ),
    aBtnCellsLeft   ( this, ScResId( BTN_CELLSLEFT ) ),
    aBtnDelRows     ( this, ScResId( BTN_DELROWS ) ),
    aBtnDelCols     ( this, ScResId( BTN_DELCOLS ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) )
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



