/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namecrea.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:08:22 $
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

#include "namecrea.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScNameCreateDlg::ScNameCreateDlg( Window * pParent, USHORT nFlags )
    : ModalDialog( pParent, ScResId(RID_SCDLG_NAMES_CREATE) ),
    aTopBox         ( this, ScResId( BTN_TOP ) ),
    aLeftBox        ( this, ScResId( BTN_LEFT ) ),
    aBottomBox      ( this, ScResId( BTN_BOTTOM ) ),
    aRightBox       ( this, ScResId( BTN_RIGHT ) ),
    aOKButton       ( this, ScResId( BTN_OK ) ),
    aCancelButton   ( this, ScResId( BTN_CANCEL ) ),
    aHelpButton     ( this, ScResId( BTN_HELP ) ),
    aFixedLine      ( this, ScResId( FL_FRAME ) )
{
    aTopBox.Check   ( (nFlags & NAME_TOP)   ? TRUE : FALSE );
    aLeftBox.Check  ( (nFlags & NAME_LEFT)  ? TRUE : FALSE );
    aBottomBox.Check( (nFlags & NAME_BOTTOM)? TRUE : FALSE );
    aRightBox.Check ( (nFlags & NAME_RIGHT) ? TRUE : FALSE );

    FreeResource();
}

//------------------------------------------------------------------

USHORT ScNameCreateDlg::GetFlags() const
{
    USHORT  nResult = 0;

    nResult |= aTopBox.IsChecked()      ? NAME_TOP:     0 ;
    nResult |= aLeftBox.IsChecked()     ? NAME_LEFT:    0 ;
    nResult |= aBottomBox.IsChecked()   ? NAME_BOTTOM:  0 ;
    nResult |= aRightBox.IsChecked()    ? NAME_RIGHT:   0 ;

    return nResult;
}




