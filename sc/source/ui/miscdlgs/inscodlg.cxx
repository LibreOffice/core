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
#include "miscdlgs.hrc"

bool       ScInsertContentsDlg::bPreviousAllCheck = false;
InsertDeleteFlags ScInsertContentsDlg::nPreviousChecks   = (IDF_VALUE | IDF_DATETIME | IDF_STRING);
sal_uInt16 ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::NONE;
sal_uInt16 ScInsertContentsDlg::nPreviousChecks2 = 0;
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
    nShortCutInsContentsCmdBits( IDF_NONE ),
    nShortCutFormulaCmdBits(0),
    bShortCutSkipEmptyCells(false),
    bShortCutTranspose(false),
    bShortCutIsLink(false),
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

    if ( nCheckDefaults != IDF_NONE )
    {
        ScInsertContentsDlg::nPreviousChecks = nCheckDefaults;
        ScInsertContentsDlg::bPreviousAllCheck = false;
        ScInsertContentsDlg::nPreviousChecks2 = 0;
    }

    mpBtnInsAll->Check     ( ScInsertContentsDlg::bPreviousAllCheck );
    mpBtnInsStrings->Check ( IS_SET( IDF_STRING,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsNumbers->Check ( IS_SET( IDF_VALUE,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsDateTime->Check( IS_SET( IDF_DATETIME,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsFormulas->Check( IS_SET( IDF_FORMULA,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsNotes->Check   ( IS_SET( IDF_NOTE,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsAttrs->Check   ( IS_SET( IDF_ATTRIB,
                                   ScInsertContentsDlg::nPreviousChecks ) );
    mpBtnInsObjects->Check ( IS_SET( IDF_OBJECTS,
                                   ScInsertContentsDlg::nPreviousChecks ) );

    switch( ScInsertContentsDlg::nPreviousFormulaChecks )
    {
        case PASTE_NOFUNC: mpRbNoOp->Check(); break;
        case PASTE_ADD:    mpRbAdd->Check(); break;
        case PASTE_SUB:    mpRbSub->Check(); break;
        case PASTE_MUL:    mpRbMul->Check(); break;
        case PASTE_DIV:    mpRbDiv->Check(); break;
    }

    switch( ScInsertContentsDlg::nPreviousMoveMode )
    {
        case INS_NONE:       mpRbMoveNone->Check(); break;
        case INS_CELLSDOWN:  mpRbMoveDown->Check(); break;
        case INS_CELLSRIGHT: mpRbMoveRight->Check(); break;
    }

    mpBtnSkipEmptyCells->Check( ( ScInsertContentsDlg::nPreviousChecks2 & INS_CONT_NOEMPTY ) != 0);
    mpBtnTranspose->Check( ( ScInsertContentsDlg::nPreviousChecks2    & INS_CONT_TRANS ) != 0);
    mpBtnLink->Check( ( ScInsertContentsDlg::nPreviousChecks2             & INS_CONT_LINK  ) != 0);

    DisableChecks( mpBtnInsAll->IsChecked() );

    mpBtnInsAll->SetClickHdl( LINK( this, ScInsertContentsDlg, InsAllHdl ) );
    mpBtnLink->SetClickHdl( LINK( this, ScInsertContentsDlg, LinkBtnHdl ) );

    mpBtnShortCutPasteValuesOnly->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );;
    mpBtnShortCutPasteValuesFormats->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );;
    mpBtnShortCutPasteTranspose->SetClickHdl( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
}

InsertDeleteFlags ScInsertContentsDlg::GetInsContentsCmdBits() const
{
    ScInsertContentsDlg::nPreviousChecks = IDF_NONE;

    if ( mpBtnInsStrings->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks = IDF_STRING;
    if ( mpBtnInsNumbers->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks |= IDF_VALUE;
    if ( mpBtnInsDateTime->IsChecked())
        ScInsertContentsDlg::nPreviousChecks |= IDF_DATETIME;
    if ( mpBtnInsFormulas->IsChecked())
        ScInsertContentsDlg::nPreviousChecks |= IDF_FORMULA;
    if ( mpBtnInsNotes->IsChecked()   )
        ScInsertContentsDlg::nPreviousChecks |= IDF_NOTE;
    if ( mpBtnInsAttrs->IsChecked()   )
        ScInsertContentsDlg::nPreviousChecks |= IDF_ATTRIB;
    if ( mpBtnInsObjects->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks |= IDF_OBJECTS;

    ScInsertContentsDlg::bPreviousAllCheck = mpBtnInsAll->IsChecked();

    if (bUsedShortCut)
        return nShortCutInsContentsCmdBits;

    return ( (ScInsertContentsDlg::bPreviousAllCheck)
                ? IDF_ALL
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
        return bShortCutSkipEmptyCells;
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
        return bShortCutIsLink;
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
        DisableChecks();
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

void ScInsertContentsDlg::SetCellShiftDisabled( int nDisable )
{
    bool bDown = ((nDisable & SC_CELL_SHIFT_DISABLE_DOWN) != 0);
    bool bRight = ((nDisable & SC_CELL_SHIFT_DISABLE_RIGHT) != 0);
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

IMPL_LINK_TYPED( ScInsertContentsDlg, ShortCutHdl, Button*, pBtn, void )
{
    if ( pBtn == mpBtnShortCutPasteValuesOnly )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = IDF_STRING | IDF_VALUE | IDF_DATETIME;
        nShortCutFormulaCmdBits = ScPasteFunc::NONE;
        bShortCutSkipEmptyCells = false;
        bShortCutTranspose = false;
        bShortCutIsLink = false;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
    else if ( pBtn == mpBtnShortCutPasteValuesFormats )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = IDF_STRING | IDF_VALUE | IDF_DATETIME | IDF_ATTRIB;
        nShortCutFormulaCmdBits = ScPasteFunc::NONE;
        bShortCutSkipEmptyCells = false;
        bShortCutTranspose = false;
        bShortCutIsLink = false;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
    else if ( pBtn == mpBtnShortCutPasteTranspose )
    {
        bUsedShortCut = true;
        nShortCutInsContentsCmdBits = IDF_ALL;
        nShortCutFormulaCmdBits = ScPasteFunc::NONE;
        bShortCutSkipEmptyCells = false;
        bShortCutTranspose = true;
        bShortCutIsLink = false;
        nShortCutMoveMode = INS_NONE;
        EndDialog(RET_OK);
    }
}


IMPL_LINK_NOARG_TYPED(ScInsertContentsDlg, InsAllHdl, Button*, void)
{
    DisableChecks( mpBtnInsAll->IsChecked() );
}

IMPL_LINK_NOARG_TYPED(ScInsertContentsDlg, LinkBtnHdl, Button*, void)
{
    TestModes();
}

ScInsertContentsDlg::~ScInsertContentsDlg()
{
    disposeOnce();
}

void ScInsertContentsDlg::dispose()
{
    ScInsertContentsDlg::nPreviousChecks2 = 0;
    if(mpBtnSkipEmptyCells->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_NOEMPTY;
    if( mpBtnTranspose->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_TRANS;
    if( mpBtnLink->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_LINK;

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
        return nShortCutFormulaCmdBits;
    return ScInsertContentsDlg::nPreviousFormulaChecks;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
