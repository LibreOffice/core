/*************************************************************************
 *
 *  $RCSfile: strindlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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

    //  HelpId for Edit different for different uses

    DBG_ASSERT( nHelpId == FID_TAB_APPEND || nHelpId == FID_TAB_RENAME ||
                nHelpId == HID_SC_ADD_AUTOFMT || nHelpId == HID_SC_RENAME_AUTOFMT,
                "unknown ID" );
    if ( nHelpId == FID_TAB_APPEND )
        aEdInput.SetHelpId( HID_SC_APPEND_NAME );
    else if ( nHelpId == FID_TAB_RENAME )
        aEdInput.SetHelpId( HID_SC_RENAME_NAME );
    else if ( nHelpId == HID_SC_ADD_AUTOFMT )
        aEdInput.SetHelpId( HID_SC_AUTOFMT_NAME );
    else if ( nHelpId == HID_SC_RENAME_AUTOFMT )
        aEdInput.SetHelpId( HID_SC_REN_AFMT_NAME );

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



