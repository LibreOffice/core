/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
                              sal_Bool bCol = sal_False,
                              sal_Bool bRow = sal_False )
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

    sal_Bool IsCol() { return aBtnCol.IsChecked(); }
    sal_Bool IsRow() { return aBtnRow.IsChecked(); }

private:
    FixedLine       aFlColRow;
    CheckBox        aBtnRow;
    CheckBox        aBtnCol;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};


#endif



