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

#ifndef SC_LBSELDLG_HXX
#define SC_LBSELDLG_HXX


#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif


#include <vcl/dialog.hxx>

//------------------------------------------------------------------------

class ScSelEntryDlg : public ModalDialog
{
private:
    FixedLine       aFlLbTitle;
    ListBox         aLb;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    DECL_LINK( DblClkHdl, void * );

public:
            ScSelEntryDlg(      Window* pParent,
                                sal_uInt16  nResId,
                          const String& aTitle,
                          const String& aLbTitle,
                                List&   aEntryList );
            ~ScSelEntryDlg();

    String GetSelectEntry() const;
//UNUSED2008-05  sal_uInt16 GetSelectEntryPos() const;
};


#endif // SC_LBSELDLG_HXX


