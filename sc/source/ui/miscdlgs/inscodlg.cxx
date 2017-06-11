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

#undef SC_DLLIMPLEMENTATION

#include "inscodlg.hxx"
#include "scresid.hxx"
#include "strings.hrc"

bool       ScInsertContentsDlg::bPreviousAllCheck = false;
InsertDeleteFlags ScInsertContentsDlg::nPreviousChecks   = (InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME | InsertDeleteFlags::STRING);
ScPasteFunc  ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::NONE;
InsertContentsFlags ScInsertContentsDlg::nPreviousChecks2 = InsertContentsFlags::NONE;
sal_uInt16 ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;   // enum InsCellCmd

ScInsertContentsDlg::ScInsertContentsDlg( vcl::Window*       pParent,
                                          InsertDeleteFlags nCheckDefaults,
                                          const OUString* pStrTitle )

 :  ModalDialog     ( pParent, "PasteSpecial", "modules/scalc/ui/pastespecial.ui" ),
    bOtherDoc       ( false ),
    bFillMode       ( false ),
    bChangeTrack    ( false ),
    bMoveDownDisabled( false ),
    bMoveRightDisabled( false ),
    bUsedShortCut   ( false ),
    nShortCutInsContentsCmdBits( InsertDeleteFlags::NONE ),
    bShortCutTranspose(false),
    nShortCutMoveMode(INS_NONE)
{
    get( mpBtnInsAll, "paste_all" );
    get( mpBtnInsStrings, "text" );
    get( mpBtnInsNumbers, "numbers" );
    get( mpBtnInsDateTime, "datetime" );
    get( mpBtnInsFormulas, "formulas" );
    get( mpBtnInsNotes, "comments" );
    get( mpBtnInsAttrs, "formats" );
    get( mpBtnInsObjects, "objects" );
    get( mpBtnSkipEmptyCells, "skip_empty" );
    get( mpBtnTranspose, "transpose" );
    get( mpBtnLink, "link" );
    get( mpRbNoOp, "none" );
    get( mpRbAdd, "add" );
    get( mpRbSub, "subtract" );
    get( mpRbMul, "multiply" );
    get( mpRbDiv, "divide" );
    get( mpRbMoveNone, "no_shift" );
    get( mpRbMoveDown, "move_down" );
    get( mpRbMoveRight, "move_right" );
    get( mpBtnShortCutPasteValuesOnly, "paste_values_only");
    get( mpBtnShortCutPasteValuesFormats, "paste_values_formats");
    get( mpBtnShortCutPasteTranspose, "paste_transpose");

    if ( pStrTitle )
        SetText( *pStrTitle );

    if ( nCheckDefaults != InsertDeleteFlags::NONE )
    {
        ScInsertContentsDlg::nPreviousChecks = nCheckDefaults;
        ScInsertContentsDlg::bPreviousAllCheck = false;
        ScInsertContentsDlg::nPreviousChecks2 = InsertContentsFlags::NONE;
    }

    mpBtnInsAll->Check     ( ScInsertContentsDlg::bPreviousAllCheck );
    mpBtnInsStrings->Check ( IS_SET( InsertDeleteFlags::STRING,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsNumbers->Check ( IS_SET( InsertDeleteFlags::VALUE,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsDateTime->Check( IS_SET( InsertDeleteFlags::DATETIME,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsFormulas->Check( IS_SET( InsertDeleteFlags::FORMULA,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsNotes->Check   ( IS_SET( InsertDeleteFlags::NOTE,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsAttrs->Check   ( IS_SET( InsertDeleteFlags::ATTRIB,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsObjects->Check ( IS_SET( InsertDeleteFlags::OBJECTS,
                                   ScInsertContentsDlg::nPreviousChecks ) );

    switch( ScInsertContentsDlg::nPreviousFormulaChecks )
    {
        case ScPasteFunc::NONE: mpRbNoOp->Check(); break;
        case ScPasteFunc::ADD:    mpRbAdd->Check(); break;
        case ScPasteFunc::SUB:    mpRbSub->Check(); break;
        case ScPasteFunc::MUL:    mpRbMul->Check(); break;
        case ScPasteFunc::DIV:    mpRbDiv->Check(); break;
    }

    switch( ScInsertContentsDlg::nPreviousMoveMode )
    {
        case INS_NONE:       mpRbMoveNone->Check(); break;
        case INS_CELLSDOWN:  mpRbMoveDown->Check(); break;
        case INS_CELLSRIGHT: mpRbMoveRight->Check(); break;
    }

    mpBtnSkipEmptyCells->Check( bool( ScInsertContentsDlg::nPreviousChecks2 & InsertContentsFlags::NoEmpty ));
    mpBtnTranspose->Check( bool( ScInsertContentsDlg::nPreviousChecks2    & InsertContentsFlags::Trans ));
    mpBtnLink->Check( bool( ScInsertContentsDlg::nPreviousChecks2             & InsertContentsFlags::Link  ));

    DisableChecks( mpBtnInsAll->IsChecked() );

    mpBtnInsAll->SetClickHdl( LINK( this, ScInsertContentsDlg, InsAllHdl ) );
    mpBtnLink->SetClickHdl( LINK( this, ScInsertContentsDlg, LinkBtnHdl ) );

    mpBtnShortCutPasteValuesOnly->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
    mpBtnShortCutPasteValuesFormats->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
    mpBtnShortCutPasteTranspose->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
}

InsertDeleteFlags ScInsertContentsDlg::GetInsContentsCmdBits() const
{
    ScInsertContentsDlg::nPreviousChecks = InsertDeleteFlags::NONE;

    if ( mpBtnInsStrings->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks = InsertDeleteFlags::STRING;
    if ( mpBtnInsNumbers->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::VALUE;
    if ( mpBtnInsDateTime->IsChecked())
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::DATETIME;
    if ( mpBtnInsFormulas->IsChecked())
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::FORMULA;
    if ( mpBtnInsNotes->IsChecked()   )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::NOTE;
    if ( mpBtnInsAttrs->IsChecked()   )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::ATTRIB;
    if ( mpBtnInsObjects->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::OBJECTS;

    ScInsertContentsDlg::bPreviousAllCheck = mpBtnInsAll->IsChecked();

    if (bUsedShortCut)
        return nShortCutInsContentsCmdBits;

    return ( (ScInsertContentsDlg::bPreviousAllCheck)
                ? InsertDeleteFlags::ALL
                : ScInsertContentsDlg::nPreviousChecks );
}

InsCellCmd ScInsertContentsDlg::GetMoveMode()
{
    if (bUsedShortCut)
        return nShortCutMoveMode;
    if ( mpRbMoveDown->IsChecked() )
        return INS_CELLSDOWN;
    if ( mpRbMoveRight->IsChecked() )
        return INS_CELLSRIGHT;

    return INS_NONE;
}

bool ScInsertContentsDlg::IsSkipEmptyCells() const
{
    if (bUsedShortCut)
        return false;
    return mpBtnSkipEmptyCells->IsChecked();
}

bool ScInsertContentsDlg::IsTranspose() const
{
    if (bUsedShortCut)
        return bShortCutTranspose;
    return mpBtnTranspose->IsChecked();
}

bool ScInsertContentsDlg::IsLink() const
{
    if (bUsedShortCut)
        return false;
    return mpBtnLink->IsChecked();
}

void ScInsertContentsDlg::DisableChecks( bool bInsAllChecked )
{
    if ( bInsAllChecked )
    {
        mpBtnInsStrings->Disable();
        mpBtnInsNumbers->Disable();
        mpBtnInsDateTime->Disable();
        mpBtnInsFormulas->Disable();
        mpBtnInsNotes->Disable();
        mpBtnInsAttrs->Disable();
        mpBtnInsObjects->Disable();
    }
    else
    {
        mpBtnInsStrings->Enable();
        mpBtnInsNumbers->Enable();
        mpBtnInsDateTime->Enable();
        mpBtnInsFormulas->Enable();
        mpBtnInsNotes->Enable();
        mpBtnInsAttrs->Enable();

        //  "Objects" is disabled for "Fill Tables"
        if ( bFillMode )
            mpBtnInsObjects->Disable();
        else
            mpBtnInsObjects->Enable();
    }
}

// Link to other document -> everything else is disabled

void ScInsertContentsDlg::TestModes()
{
    if ( bOtherDoc && mpBtnLink->IsChecked() )
    {
        mpBtnSkipEmptyCells->Disable();
        mpBtnTranspose->Disable();
        mpRbNoOp->Disable();
        mpRbAdd->Disable();
        mpRbSub->Disable();
        mpRbMul->Disable();
        mpRbDiv->Disable();

        mpRbMoveNone->Disable();
        mpRbMoveDown->Disable();
        mpRbMoveRight->Disable();

        mpBtnInsAll->Disable();
        DisableChecks(true);
    }
    else
    {
        mpBtnSkipEmptyCells->Enable();
        mpBtnTranspose->Enable(!bFillMode);
        mpRbNoOp->Enable();
        mpRbAdd->Enable();
        mpRbSub->Enable();
        mpRbMul->Enable();
        mpRbDiv->Enable();

        mpRbMoveNone->Enable(!bFillMode && !bChangeTrack && !(bMoveDownDisabled && bMoveRightDisabled));
        mpRbMoveDown->Enable(!bFillMode && !bChangeTrack && !bMoveDownDisabled);
        mpRbMoveRight->Enable(!bFillMode && !bChangeTrack && !bMoveRightDisabled);

        mpBtnInsAll->Enable();
        DisableChecks( mpBtnInsAll->IsChecked() );
    }
}

void ScInsertContentsDlg::SetOtherDoc( bool bSet )
{
    if ( bSet != bOtherDoc )
    {
        bOtherDoc = bSet;
        TestModes();
        if ( bSet )
            mpRbMoveNone->Check();
    }
}

void ScInsertContentsDlg::SetFillMode( bool bSet )
{
    if ( bSet != bFillMode )
    {
        bFillMode = bSet;
        TestModes();
        if ( bSet )
            mpRbMoveNone->Check();
    }
}

void ScInsertContentsDlg::SetChangeTrack( bool bSet )
{
    if ( bSet != bChangeTrack )
    {
        bChangeTrack = bSet;
        TestModes();
        if ( bSet )
            mpRbMoveNone->Check();
    }
}

void ScInsertContentsDlg::SetCellShiftDisabled( CellShiftDisabledFlags nDisable )
{
    bool bDown(nDisable & CellShiftDisabledFlags::Down);
    bool bRight(nDisable & CellShiftDisabledFlags::Right);
    if ( bDown != bMoveDownDisabled || bRight != bMoveRightDisabled )
    {
        bMoveDownDisabled = bDown;
        bMoveRightDisabled = bRight;
        TestModes();
        if ( bMoveDownDisabled && mpRbMoveDown->IsChecked() )
            mpRbMoveNone->Check();
        if ( bMoveRightDisabled && mpRbMoveRight->IsChecked() )
            mpRbMoveNone->Check();
    }
}

IMPL_LINK( ScInsertContentsDlg, ShortCutHdl, Button*, pBtn, void )
{
    if ( pBtn == mpBtnShortCutPasteValuesOnly )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = InsertDeleteFlags::STRING | InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME;
        bShortCutTranspose = false;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
    else if ( pBtn == mpBtnShortCutPasteValuesFormats )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = InsertDeleteFlags::STRING | InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME | InsertDeleteFlags::ATTRIB;
        bShortCutTranspose = false;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
    else if ( pBtn == mpBtnShortCutPasteTranspose )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = InsertDeleteFlags::ALL;
        bShortCutTranspose = true;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
}


IMPL_LINK_NOARG(ScInsertContentsDlg, InsAllHdl, Button*, void)
{
    DisableChecks( mpBtnInsAll->IsChecked() );
}

IMPL_LINK_NOARG(ScInsertContentsDlg, LinkBtnHdl, Button*, void)
{
    TestModes();
}

ScInsertContentsDlg::~ScInsertContentsDlg()
{
    disposeOnce();
}

void ScInsertContentsDlg::dispose()
{
    ScInsertContentsDlg::nPreviousChecks2 = InsertContentsFlags::NONE;
    if(mpBtnSkipEmptyCells->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::NoEmpty;
    if( mpBtnTranspose->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::Trans;
    if( mpBtnLink->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::Link;

    if (!bFillMode)     // in FillMode, None is checked and all three options are disabled
    {
        if ( mpRbMoveNone->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;
        else if ( mpRbMoveDown->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSDOWN;
        else if ( mpRbMoveRight->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSRIGHT;
    }
    mpBtnInsAll.clear();
    mpBtnInsStrings.clear();
    mpBtnInsNumbers.clear();
    mpBtnInsDateTime.clear();
    mpBtnInsFormulas.clear();
    mpBtnInsNotes.clear();
    mpBtnInsAttrs.clear();
    mpBtnInsObjects.clear();
    mpBtnSkipEmptyCells.clear();
    mpBtnTranspose.clear();
    mpBtnLink.clear();
    mpRbNoOp.clear();
    mpRbAdd.clear();
    mpRbSub.clear();
    mpRbMul.clear();
    mpRbDiv.clear();
    mpRbMoveNone.clear();
    mpRbMoveDown.clear();
    mpRbMoveRight.clear();
    mpBtnShortCutPasteValuesOnly.clear();
    mpBtnShortCutPasteValuesFormats.clear();
    mpBtnShortCutPasteTranspose.clear();
    ModalDialog::dispose();
}

ScPasteFunc  ScInsertContentsDlg::GetFormulaCmdBits() const
{
    ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::NONE;
    if(mpRbAdd->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::ADD;
    else if(mpRbSub->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::SUB;
    else if(mpRbMul->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::MUL;
    else if(mpRbDiv->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::DIV;
    if (bUsedShortCut)
        return ScPasteFunc::NONE;
    return ScInsertContentsDlg::nPreviousFormulaChecks;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
