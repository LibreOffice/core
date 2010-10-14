/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifndef _DLGEJPG_HXX_
#define _DLGEJPG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/fltcall.hxx>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen
|*
\************************************************************************/
class FilterConfigItem;
class DlgExportEJPG : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    FixedInfo           aFiDescr;
    NumericField        aNumFldQuality;
    FixedLine           aGrpQuality;
    RadioButton         aRbGray;
    RadioButton         aRbRGB;
    FixedLine           aGrpColors;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    FilterConfigItem*   pConfigItem;

    DECL_LINK( OK, void * );

public:
            DlgExportEJPG( FltCallDialogParameter& rDlgPara );
            ~DlgExportEJPG();
};

#endif // _DLGEJPG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
