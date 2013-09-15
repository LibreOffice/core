/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        ScNewScenarioDlg( Window* pParent, const OUString& rName, sal_Bool bEdit = false, sal_Bool bSheetProtected = false );
        ~ScNewScenarioDlg();

    void SetScenarioData( const OUString& rName, const OUString& rComment,
                          const Color& rColor, sal_uInt16 nFlags );

    void GetScenarioData( OUString& rName, OUString& rComment,
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
    const OUString      aDefScenarioName;
    sal_Bool            bIsEdit;

    DECL_LINK(OkHdl, void *);
    DECL_LINK( EnableHdl, CheckBox * );
};


#endif // SC_SCENDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
