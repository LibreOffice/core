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

#ifndef SC_INSCODLG_HXX
#define SC_INSCODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "global.hxx"

//------------------------------------------------------------------------
#include "scui_def.hxx"

class ScInsertContentsDlg : public ModalDialog
{
public:
            ScInsertContentsDlg( Window*          pParent,
                                 sal_uInt16       nCheckDefaults = 0,
                                 const OUString*  pStrTitle = NULL );
            ~ScInsertContentsDlg();

    sal_uInt16      GetInsContentsCmdBits() const;
    sal_uInt16      GetFormulaCmdBits() const;
    sal_Bool        IsSkipEmptyCells() const {return aBtnSkipEmptyCells.IsChecked();}
    sal_Bool        IsTranspose() const {return aBtnTranspose.IsChecked();}
    sal_Bool        IsLink() const {return aBtnLink.IsChecked();}
    InsCellCmd  GetMoveMode();

    void    SetOtherDoc( sal_Bool bSet );
    void    SetFillMode( sal_Bool bSet );
    void    SetChangeTrack( sal_Bool bSet );
    void    SetCellShiftDisabled( int nDisable );

private:
    FixedLine       aFlFrame;
    CheckBox        aBtnInsAll;
    CheckBox        aBtnInsStrings;
    CheckBox        aBtnInsNumbers;
    CheckBox        aBtnInsDateTime;
    CheckBox        aBtnInsFormulas;
    CheckBox        aBtnInsNotes;
    CheckBox        aBtnInsAttrs;
    CheckBox        aBtnInsObjects;

    FixedLine       aFlSep1;
    FixedLine       aFlOptions;
    CheckBox        aBtnSkipEmptyCells;
    CheckBox        aBtnTranspose;
    CheckBox        aBtnLink;

    FixedLine       aFlOperation;
    RadioButton     aRbNoOp;
    RadioButton     aRbAdd;
    RadioButton     aRbSub;
    RadioButton     aRbMul;
    RadioButton     aRbDiv;

    FixedLine       aFlSep2;
    FixedLine       aFlMove;
    RadioButton     aRbMoveNone;
    RadioButton     aRbMoveDown;
    RadioButton     aRbMoveRight;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_Bool            bOtherDoc;
    sal_Bool            bFillMode;
    sal_Bool            bChangeTrack;
    sal_Bool            bMoveDownDisabled;
    sal_Bool            bMoveRightDisabled;

    static sal_Bool     bPreviousAllCheck;
    static sal_uInt16   nPreviousChecks;
    static sal_uInt16   nPreviousChecks2;
    static sal_uInt16   nPreviousFormulaChecks;
    static sal_uInt16   nPreviousMoveMode;          // enum InsCellCmd

    void DisableChecks( sal_Bool bInsAllChecked = sal_True );
    void TestModes();

    // Handler
    DECL_LINK( InsAllHdl, void* );
    DECL_LINK( LinkBtnHdl, void* );
};


#endif // SC_INSCODLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
