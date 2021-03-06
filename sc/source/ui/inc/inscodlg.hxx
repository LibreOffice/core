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

#pragma once

#include <vcl/weld.hxx>
#include <global.hxx>

#include "scui_def.hxx"

class ScInsertContentsDlg : public weld::GenericDialogController
{
public:
            ScInsertContentsDlg( weld::Window*      pParent,
                                 const OUString*   pStrTitle );
    virtual ~ScInsertContentsDlg() override;

    InsertDeleteFlags GetInsContentsCmdBits() const;
    ScPasteFunc       GetFormulaCmdBits() const;

    bool            IsSkipEmptyCells() const;
    bool            IsTranspose() const;
    bool            IsLink() const;
    InsCellCmd      GetMoveMode() const;

    void    SetOtherDoc( bool bSet );
    void    SetFillMode( bool bSet );
    void    SetChangeTrack( bool bSet );
    void    SetCellShiftDisabled( CellShiftDisabledFlags nDisable );

private:
    bool              bOtherDoc;
    bool              bFillMode;
    bool              bChangeTrack;
    bool              bMoveDownDisabled;
    bool              bMoveRightDisabled;
    void SetInsContentsCmdBits(const InsertDeleteFlags eFlags);
    void SetFormulaCmdBits(const ScPasteFunc eFlags);
    void SetCellCmdFlags(const InsCellCmd eFlags);
    void SetContentsFlags(const InsertContentsFlags eFlags);

    std::unique_ptr<weld::CheckButton>        mxBtnInsAll;
    std::unique_ptr<weld::CheckButton>        mxBtnInsStrings;
    std::unique_ptr<weld::CheckButton>        mxBtnInsNumbers;
    std::unique_ptr<weld::CheckButton>        mxBtnInsDateTime;
    std::unique_ptr<weld::CheckButton>        mxBtnInsFormulas;
    std::unique_ptr<weld::CheckButton>        mxBtnInsNotes;
    std::unique_ptr<weld::CheckButton>        mxBtnInsAttrs;
    std::unique_ptr<weld::CheckButton>        mxBtnInsObjects;

    std::unique_ptr<weld::CheckButton>        mxBtnSkipEmptyCells;
    std::unique_ptr<weld::CheckButton>        mxBtnTranspose;
    std::unique_ptr<weld::CheckButton>        mxBtnLink;

    std::unique_ptr<weld::RadioButton>     mxRbNoOp;
    std::unique_ptr<weld::RadioButton>     mxRbAdd;
    std::unique_ptr<weld::RadioButton>     mxRbSub;
    std::unique_ptr<weld::RadioButton>     mxRbMul;
    std::unique_ptr<weld::RadioButton>     mxRbDiv;

    std::unique_ptr<weld::RadioButton>     mxRbMoveNone;
    std::unique_ptr<weld::RadioButton>     mxRbMoveDown;
    std::unique_ptr<weld::RadioButton>     mxRbMoveRight;

    std::unique_ptr<weld::Button>      mxBtnShortCutPasteValuesOnly;
    std::unique_ptr<weld::Button>      mxBtnShortCutPasteValuesFormats;
    std::unique_ptr<weld::Button>      mxBtnShortCutPasteTranspose;
    std::unique_ptr<weld::Button>      mxBtnShortCutPasteFormats;

    std::unique_ptr<weld::CheckButton>     mxImmediately;

    static bool         bPreviousAllCheck;
    static InsertDeleteFlags nPreviousChecks;
    static InsertContentsFlags nPreviousChecks2;
    static ScPasteFunc  nPreviousFormulaChecks;
    static InsCellCmd   nPreviousMoveMode;

    void DisableChecks( bool bInsAllChecked );
    void TestModes();

    // Handler
    DECL_LINK( InsAllHdl, weld::ToggleButton&, void );
    DECL_LINK( LinkBtnHdl, weld::ToggleButton&, void );
    DECL_LINK( ShortCutHdl, weld::Button&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
