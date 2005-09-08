/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:08:44 $
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

#undef SC_DLLIMPLEMENTATION

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "groupdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScGroupDlg::ScGroupDlg( Window* pParent,
                        USHORT  nResId,
                        BOOL    bUngroup,
                        BOOL    bRows ) :
    ModalDialog     ( pParent, ScResId( nResId ) ),
    //
    aBtnRows        ( this, ScResId( BTN_GROUP_ROWS ) ),
    aBtnCols        ( this, ScResId( BTN_GROUP_COLS ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
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

BOOL ScGroupDlg::GetRowsChecked() const
{
    return aBtnRows.IsChecked();
}

//------------------------------------------------------------------------

BOOL ScGroupDlg::GetColsChecked() const
{
    return aBtnCols.IsChecked();
}

//------------------------------------------------------------------------

__EXPORT ScGroupDlg::~ScGroupDlg()
{
}



