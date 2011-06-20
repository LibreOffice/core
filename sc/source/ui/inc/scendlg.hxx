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

#ifndef SC_SCENDLG_HXX
#define SC_SCENDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/ctrlbox.hxx>

//===================================================================

class ScNewScenarioDlg : public ModalDialog
{
public:
        ScNewScenarioDlg( Window* pParent, const String& rName, sal_Bool bEdit = false, sal_Bool bSheetProtected = false );
        ~ScNewScenarioDlg();

    void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, sal_uInt16 nFlags );

    void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, sal_uInt16& rFlags ) const;

private:
    FixedLine           aFlName;
    Edit                aEdName;
    FixedLine           aFlComment;
    MultiLineEdit       aEdComment;
    FixedLine           aFlOptions;
    CheckBox            aCbShowFrame;
    ColorListBox        aLbColor;
    //CheckBox          aCbPrintFrame;
    CheckBox            aCbTwoWay;
    //CheckBox          aCbAttrib;
    //CheckBox          aCbValue;
    CheckBox            aCbCopyAll;
    CheckBox            aCbProtect;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    const String        aDefScenarioName;
    sal_Bool                bIsEdit;

    DECL_LINK( OkHdl, OKButton * );
    DECL_LINK( EnableHdl, CheckBox * );
};


#endif // SC_SCENDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
