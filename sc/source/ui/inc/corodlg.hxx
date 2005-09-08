/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: corodlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:16:39 $
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

#ifndef SC_CORODLG_HXX
#define SC_CORODLG_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#include "sc.hrc"
#include "scresid.hxx"

//------------------------------------------------------------------------

class ScColRowLabelDlg : public ModalDialog
{
public:
            ScColRowLabelDlg( Window* pParent,
                              BOOL bCol = FALSE,
                              BOOL bRow = FALSE )
                : ModalDialog( pParent, ScResId( RID_SCDLG_CHARTCOLROW ) ),
                  aFlColRow  ( this, ScResId(6) ),
                  aBtnRow    ( this, ScResId(2) ),
                  aBtnCol    ( this, ScResId(1) ),
                  aBtnOk     ( this, ScResId(3) ),
                  aBtnCancel ( this, ScResId(4) ),
                  aBtnHelp   ( this, ScResId(5) )
                {
                    FreeResource();
                    aBtnCol.Check( bCol );
                    aBtnRow.Check( bRow );
                }

    BOOL IsCol() { return aBtnCol.IsChecked(); }
    BOOL IsRow() { return aBtnRow.IsChecked(); }

private:
    FixedLine       aFlColRow;
    CheckBox        aBtnRow;
    CheckBox        aBtnCol;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};


#endif



