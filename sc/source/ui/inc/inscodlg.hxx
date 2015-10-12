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
    virtual void dispose() override;

    InsertDeleteFlags GetInsContentsCmdBits() const;
    ScPasteFunc       GetFormulaCmdBits() const;
    bool            IsSkipEmptyCells() const;
    bool            IsTranspose() const;
    bool            IsLink() const;
    InsCellCmd      GetMoveMode();

    void    SetOtherDoc( bool bSet );
    void    SetFillMode( bool bSet );
    void    SetChangeTrack( bool bSet );
    void    SetCellShiftDisabled( int nDisable );

private:
    VclPtr<CheckBox>        mpBtnInsAll;
    VclPtr<CheckBox>        mpBtnInsStrings;
    VclPtr<CheckBox>        mpBtnInsNumbers;
    VclPtr<CheckBox>        mpBtnInsDateTime;
    VclPtr<CheckBox>        mpBtnInsFormulas;
    VclPtr<CheckBox>        mpBtnInsNotes;
    VclPtr<CheckBox>        mpBtnInsAttrs;
    VclPtr<CheckBox>        mpBtnInsObjects;

    VclPtr<CheckBox>        mpBtnSkipEmptyCells;
    VclPtr<CheckBox>        mpBtnTranspose;
    VclPtr<CheckBox>        mpBtnLink;

    VclPtr<RadioButton>     mpRbNoOp;
    VclPtr<RadioButton>     mpRbAdd;
    VclPtr<RadioButton>     mpRbSub;
    VclPtr<RadioButton>     mpRbMul;
    VclPtr<RadioButton>     mpRbDiv;

    VclPtr<RadioButton>     mpRbMoveNone;
    VclPtr<RadioButton>     mpRbMoveDown;
    VclPtr<RadioButton>     mpRbMoveRight;

    VclPtr<PushButton>      mpBtnShortCutPasteValuesOnly;
    VclPtr<PushButton>      mpBtnShortCutPasteValuesFormats;
    VclPtr<PushButton>      mpBtnShortCutPasteTranspose;

    bool              bOtherDoc;
    bool              bFillMode;
    bool              bChangeTrack;
    bool              bMoveDownDisabled;
    bool              bMoveRightDisabled;
    bool              bUsedShortCut;

    InsertDeleteFlags nShortCutInsContentsCmdBits;
    ScPasteFunc       nShortCutFormulaCmdBits;
    bool              bShortCutSkipEmptyCells;
    bool              bShortCutTranspose;
    bool              bShortCutIsLink;
    InsCellCmd        nShortCutMoveMode;

    static bool         bPreviousAllCheck;
    static InsertDeleteFlags nPreviousChecks;
    static sal_uInt16   nPreviousChecks2;
    static ScPasteFunc  nPreviousFormulaChecks;
    static sal_uInt16   nPreviousMoveMode;          // enum InsCellCmd

    void DisableChecks( bool bInsAllChecked = true );
    void TestModes();

    // Handler
    DECL_LINK_TYPED( InsAllHdl, Button*, void );
    DECL_LINK_TYPED( LinkBtnHdl, Button*, void );
    DECL_LINK_TYPED( ShortCutHdl, Button*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_INSCODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
