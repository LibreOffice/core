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



//------------------------------------------------------------------

#include "inscodlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

bool       ScInsertContentsDlg::bPreviousAllCheck = false;
sal_uInt16 ScInsertContentsDlg::nPreviousChecks   = (IDF_VALUE | IDF_DATETIME | IDF_STRING);
sal_uInt16 ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_NOFUNC;
sal_uInt16 ScInsertContentsDlg::nPreviousChecks2 = 0;
sal_uInt16 ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;   // enum InsCellCmd

//-----------------------------------------------------------------------

ScInsertContentsDlg::ScInsertContentsDlg( Window*       pParent,
                                          sal_uInt16        nCheckDefaults,
                                          const OUString* pStrTitle )

 :  ModalDialog     ( pParent, "PasteSpecial", "modules/scalc/ui/pastespecial.ui" ),
    bOtherDoc       ( false ),
    bFillMode       ( false ),
    bChangeTrack    ( false ),
    bMoveDownDisabled( false ),
    bMoveRightDisabled( false )
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

    if ( pStrTitle )
        SetText( *pStrTitle );

    if ( nCheckDefaults != 0 )
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
        case PASTE_NOFUNC: mpRbNoOp->Check(true); break;
        case PASTE_ADD:    mpRbAdd->Check(true); break;
        case PASTE_SUB:    mpRbSub->Check(true); break;
        case PASTE_MUL:    mpRbMul->Check(true); break;
        case PASTE_DIV:    mpRbDiv->Check(true); break;
    }

    switch( ScInsertContentsDlg::nPreviousMoveMode )
    {
        case INS_NONE:       mpRbMoveNone->Check(true); break;
        case INS_CELLSDOWN:  mpRbMoveDown->Check(true); break;
        case INS_CELLSRIGHT: mpRbMoveRight->Check(true); break;
    }

    mpBtnSkipEmptyCells->Check( ( ScInsertContentsDlg::nPreviousChecks2 & INS_CONT_NOEMPTY ) != 0);
    mpBtnTranspose->Check( ( ScInsertContentsDlg::nPreviousChecks2    & INS_CONT_TRANS ) != 0);
    mpBtnLink->Check( ( ScInsertContentsDlg::nPreviousChecks2             & INS_CONT_LINK  ) != 0);

    DisableChecks( mpBtnInsAll->IsChecked() );


    mpBtnInsAll->SetClickHdl( LINK( this, ScInsertContentsDlg, InsAllHdl ) );
    mpBtnLink->SetClickHdl( LINK( this, ScInsertContentsDlg, LinkBtnHdl ) );

}

//------------------------------------------------------------------------

sal_uInt16 ScInsertContentsDlg::GetInsContentsCmdBits() const
{
    ScInsertContentsDlg::nPreviousChecks = 0;

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

    return ( (ScInsertContentsDlg::bPreviousAllCheck)
                ? IDF_ALL
                : ScInsertContentsDlg::nPreviousChecks );
}

//------------------------------------------------------------------------

InsCellCmd ScInsertContentsDlg::GetMoveMode()
{
    if ( mpRbMoveDown->IsChecked() )
        return INS_CELLSDOWN;
    if ( mpRbMoveRight->IsChecked() )
        return INS_CELLSRIGHT;

    return INS_NONE;
}

//------------------------------------------------------------------------

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
            mpRbMoveNone->Check(true);
    }
}

void ScInsertContentsDlg::SetFillMode( bool bSet )
{
    if ( bSet != bFillMode )
    {
        bFillMode = bSet;
        TestModes();
        if ( bSet )
            mpRbMoveNone->Check(true);
    }
}

void ScInsertContentsDlg::SetChangeTrack( bool bSet )
{
    if ( bSet != bChangeTrack )
    {
        bChangeTrack = bSet;
        TestModes();
        if ( bSet )
            mpRbMoveNone->Check(true);
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
            mpRbMoveNone->Check(true);
        if ( bMoveRightDisabled && mpRbMoveRight->IsChecked() )
            mpRbMoveNone->Check(true);
    }
}


//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScInsertContentsDlg, InsAllHdl)
{
    DisableChecks( mpBtnInsAll->IsChecked() );

    return 0;
}

IMPL_LINK_NOARG(ScInsertContentsDlg, LinkBtnHdl)
{
    TestModes();

    return 0;
}

ScInsertContentsDlg::~ScInsertContentsDlg()
{
    ScInsertContentsDlg::nPreviousChecks2 = 0;
    if(mpBtnSkipEmptyCells->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_NOEMPTY;
    if( mpBtnTranspose->IsChecked())
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_TRANS;
    if( mpBtnLink->IsChecked() )
        ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_LINK;

    if (!bFillMode)     // im FillMode ist None gecheckt und alle 3 disabled
    {
        if ( mpRbMoveNone->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;
        else if ( mpRbMoveDown->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSDOWN;
        else if ( mpRbMoveRight->IsChecked() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSRIGHT;
    }
}

sal_uInt16  ScInsertContentsDlg::GetFormulaCmdBits() const
{
    ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_NOFUNC;
    if(mpRbAdd->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_ADD;
    else if(mpRbSub->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_SUB;
    else if(mpRbMul->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_MUL;
    else if(mpRbDiv->IsChecked())
        ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_DIV;
    // Bits fuer Checkboxen ausblenden
    return ScInsertContentsDlg::nPreviousFormulaChecks;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
