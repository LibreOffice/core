/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strindlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:10:40 $
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

#include <tools/debug.hxx>

#include "strindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScStringInputDlg::ScStringInputDlg( Window*         pParent,
                                    const String&   rTitle,
                                    const String&   rEditTitle,
                                    const String&   rDefault,
                                    ULONG nHelpId    ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_STRINPUT ) ),
    //
    aEdInput        ( this, ScResId( ED_INPUT ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aFtEditTitle    ( this, ScResId( FT_LABEL ) )
{
    SetHelpId( nHelpId );
    SetText( rTitle );
    aFtEditTitle.SetText( rEditTitle );
    aEdInput.SetText( rDefault );
    aEdInput.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));

    //  HelpId for Edit different for different uses

    DBG_ASSERT( nHelpId == FID_TAB_APPEND || nHelpId == FID_TAB_RENAME ||
                nHelpId == HID_SC_ADD_AUTOFMT || nHelpId == HID_SC_RENAME_AUTOFMT ||
                nHelpId == SID_RENAME_OBJECT,
                "unknown ID" );
    if ( nHelpId == FID_TAB_APPEND )
        aEdInput.SetHelpId( HID_SC_APPEND_NAME );
    else if ( nHelpId == FID_TAB_RENAME )
        aEdInput.SetHelpId( HID_SC_RENAME_NAME );
    else if ( nHelpId == HID_SC_ADD_AUTOFMT )
        aEdInput.SetHelpId( HID_SC_AUTOFMT_NAME );
    else if ( nHelpId == HID_SC_RENAME_AUTOFMT )
        aEdInput.SetHelpId( HID_SC_REN_AFMT_NAME );
    else if ( nHelpId == SID_RENAME_OBJECT )
        aEdInput.SetHelpId( HID_SC_RENAME_OBJECT );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

void ScStringInputDlg::GetInputString( String& rString ) const
{
    rString = aEdInput.GetText();
}

__EXPORT ScStringInputDlg::~ScStringInputDlg()
{
}



