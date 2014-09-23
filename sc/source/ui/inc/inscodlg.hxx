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

#ifndef INCLUDED_SC_SOURCE_UI_INC_INSCODLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_INSCODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "global.hxx"

#include "scui_def.hxx"

class ScInsertContentsDlg : public ModalDialog
{
public:
            ScInsertContentsDlg( vcl::Window*          pParent,
                                 InsertDeleteFlags nCheckDefaults = IDF_NONE,
                                 const OUString*  pStrTitle = NULL );
            virtual ~ScInsertContentsDlg();

    InsertDeleteFlags GetInsContentsCmdBits() const;
    sal_uInt16      GetFormulaCmdBits() const;
    bool            IsSkipEmptyCells() const;
    bool            IsTranspose() const;
    bool            IsLink() const;
    InsCellCmd      GetMoveMode();

    void    SetOtherDoc( bool bSet );
    void    SetFillMode( bool bSet );
    void    SetChangeTrack( bool bSet );
    void    SetCellShiftDisabled( int nDisable );

private:
    CheckBox*        mpBtnInsAll;
    CheckBox*        mpBtnInsStrings;
    CheckBox*        mpBtnInsNumbers;
    CheckBox*        mpBtnInsDateTime;
    CheckBox*        mpBtnInsFormulas;
    CheckBox*        mpBtnInsNotes;
    CheckBox*        mpBtnInsAttrs;
    CheckBox*        mpBtnInsObjects;

    CheckBox*        mpBtnSkipEmptyCells;
    CheckBox*        mpBtnTranspose;
    CheckBox*        mpBtnLink;

    RadioButton*     mpRbNoOp;
    RadioButton*     mpRbAdd;
    RadioButton*     mpRbSub;
    RadioButton*     mpRbMul;
    RadioButton*     mpRbDiv;

    RadioButton*     mpRbMoveNone;
    RadioButton*     mpRbMoveDown;
    RadioButton*     mpRbMoveRight;

    PushButton*      mpBtnShortCutPasteValuesOnly;
    PushButton*      mpBtnShortCutPasteValuesFormats;
    PushButton*      mpBtnShortCutPasteTranspose;

    bool              bOtherDoc;
    bool              bFillMode;
    bool              bChangeTrack;
    bool              bMoveDownDisabled;
    bool              bMoveRightDisabled;
    bool              bUsedShortCut;

    InsertDeleteFlags nShortCutInsContentsCmdBits;
    sal_uInt16        nShortCutFormulaCmdBits;
    bool              bShortCutSkipEmptyCells;
    bool              bShortCutTranspose;
    bool              bShortCutIsLink;
    InsCellCmd        nShortCutMoveMode;

    static bool         bPreviousAllCheck;
    static InsertDeleteFlags nPreviousChecks;
    static sal_uInt16   nPreviousChecks2;
    static sal_uInt16   nPreviousFormulaChecks;
    static sal_uInt16   nPreviousMoveMode;          // enum InsCellCmd

    void DisableChecks( bool bInsAllChecked = true );
    void TestModes();

    // Handler
    DECL_LINK( InsAllHdl, void* );
    DECL_LINK( LinkBtnHdl, void* );
    DECL_LINK( ShortCutHdl, PushButton* );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_INSCODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
