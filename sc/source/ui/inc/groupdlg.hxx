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

#ifndef SC_GROUPDLG_HXX
#define SC_GROUPDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

//------------------------------------------------------------------------

class ScGroupDlg : public ModalDialog
{
public:
            ScGroupDlg( Window* pParent,
                        sal_uInt16  nResId,
                        sal_Bool    bUnGroup = sal_False,
                        sal_Bool    bRows    = sal_True  );
            ~ScGroupDlg();

    sal_Bool GetColsChecked() const;

private:
    FixedLine       aFlFrame;
    RadioButton     aBtnRows;
    RadioButton     aBtnCols;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};


#endif // SC_STRINDLG_HXX


