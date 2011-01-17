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

#ifndef SC_CRDLG_HXX
#define SC_CRDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "scui_def.hxx" //CHINA001

//CHINA001 #define SCRET_COLS 0x42
//CHINA001 #define SCRET_ROWS 0x43

//------------------------------------------------------------------------

class ScColOrRowDlg : public ModalDialog
{
public:
            ScColOrRowDlg( Window*          pParent,
                           const String&    rStrTitle,
                           const String&    rStrLabel,
                           sal_Bool             bColDefault = sal_True );
            ~ScColOrRowDlg();

private:
    FixedLine       aFlFrame;
    RadioButton     aBtnRows;
    RadioButton     aBtnCols;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    DECL_LINK( OkHdl, OKButton * );
};


#endif



