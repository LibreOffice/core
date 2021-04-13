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

#include <inscodlg.hxx>

InsertDeleteFlags ScInsertContentsDlg::nPreviousChecks   = InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME | InsertDeleteFlags::STRING;
ScPasteFunc  ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::NONE;
InsertContentsFlags ScInsertContentsDlg::nPreviousChecks2 = InsertContentsFlags::NONE;
InsCellCmd ScInsertContentsDlg::nPreviousMoveMode = InsCellCmd::INS_NONE;

ScInsertContentsDlg::ScInsertContentsDlg(weld::Window* pParent,
                                         const OUString* pStrTitle )
    : GenericDialogController(pParent, "modules/scalc/ui/pastespecial.ui", "PasteSpecial")
    , bOtherDoc(false)
    , bFillMode(false)
    , bChangeTrack(false)
    , bMoveDownDisabled(false)
    , bMoveRightDisabled(false)
    , mxBtnInsAll(m_xBuilder->weld_check_button("paste_all"))
    , mxBtnInsStrings(m_xBuilder->weld_check_button("text"))
    , mxBtnInsNumbers(m_xBuilder->weld_check_button("numbers"))
    , mxBtnInsDateTime(m_xBuilder->weld_check_button("datetime"))
    , mxBtnInsFormulas(m_xBuilder->weld_check_button("formulas"))
    , mxBtnInsNotes(m_xBuilder->weld_check_button("comments"))
    , mxBtnInsAttrs(m_xBuilder->weld_check_button("formats"))
    , mxBtnInsObjects(m_xBuilder->weld_check_button("objects"))
    , mxBtnSkipEmptyCells(m_xBuilder->weld_check_button("skip_empty"))
    , mxBtnTranspose(m_xBuilder->weld_check_button("transpose"))
    , mxBtnLink(m_xBuilder->weld_check_button("link"))
    , mxRbNoOp(m_xBuilder->weld_radio_button("none"))
    , mxRbAdd(m_xBuilder->weld_radio_button("add"))
    , mxRbSub(m_xBuilder->weld_radio_button("subtract"))
    , mxRbMul(m_xBuilder->weld_radio_button("multiply"))
    , mxRbDiv(m_xBuilder->weld_radio_button("divide"))
    , mxRbMoveNone(m_xBuilder->weld_radio_button("no_shift"))
    , mxRbMoveDown(m_xBuilder->weld_radio_button("move_down"))
    , mxRbMoveRight(m_xBuilder->weld_radio_button("move_right"))
    , mxBtnShortCutPasteValuesOnly(m_xBuilder->weld_button("paste_values_only"))
    , mxBtnShortCutPasteValuesFormats(m_xBuilder->weld_button("paste_values_formats"))
    , mxBtnShortCutPasteTranspose(m_xBuilder->weld_button("paste_transpose"))
    , mxBtnShortCutPasteFormats(m_xBuilder->weld_button("paste_formats"))
    , mxImmediately(m_xBuilder->weld_check_button("cbImmediately"))
{
    if (pStrTitle)
        m_xDialog->set_title(*pStrTitle);

    SetInsContentsCmdBits( ScInsertContentsDlg::nPreviousChecks );
    SetFormulaCmdBits( ScInsertContentsDlg::nPreviousFormulaChecks );
    SetCellCmdFlags( ScInsertContentsDlg::nPreviousMoveMode );
    SetContentsFlags( ScInsertContentsDlg::nPreviousChecks2 );
    DisableChecks( mxBtnInsAll->get_active() );

    mxBtnInsAll->connect_toggled( LINK( this, ScInsertContentsDlg, InsAllHdl ) );
    mxBtnLink->connect_toggled( LINK( this, ScInsertContentsDlg, LinkBtnHdl ) );
    mxBtnShortCutPasteValuesOnly->connect_clicked( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
    mxBtnShortCutPasteValuesFormats->connect_clicked( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
    mxBtnShortCutPasteTranspose->connect_clicked( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
    mxBtnShortCutPasteFormats->connect_clicked( LINK( this, ScInsertContentsDlg, ShortCutHdl ) );
}

InsertDeleteFlags ScInsertContentsDlg::GetInsContentsCmdBits() const
{
    ScInsertContentsDlg::nPreviousChecks = InsertDeleteFlags::NONE;

    if ( mxBtnInsStrings->get_active() )
        ScInsertContentsDlg::nPreviousChecks = InsertDeleteFlags::STRING;
    if ( mxBtnInsNumbers->get_active() )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::VALUE;
    if ( mxBtnInsDateTime->get_active())
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::DATETIME;
    if ( mxBtnInsFormulas->get_active())
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::FORMULA;
    if ( mxBtnInsNotes->get_active()   )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::NOTE;
    if ( mxBtnInsAttrs->get_active()   )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::ATTRIB;
    if ( mxBtnInsObjects->get_active() )
        ScInsertContentsDlg::nPreviousChecks |= InsertDeleteFlags::OBJECTS;

    return ( mxBtnInsAll->get_active()
                ? InsertDeleteFlags::ALL
                : ScInsertContentsDlg::nPreviousChecks );
}

void ScInsertContentsDlg::SetInsContentsCmdBits(const InsertDeleteFlags eFlags)
{
    mxBtnInsNumbers->set_active((InsertDeleteFlags::VALUE & eFlags) == InsertDeleteFlags::VALUE);
    mxBtnInsDateTime->set_active((InsertDeleteFlags::DATETIME & eFlags) == InsertDeleteFlags::DATETIME);
    mxBtnInsStrings->set_active((InsertDeleteFlags::STRING & eFlags) == InsertDeleteFlags::STRING);
    mxBtnInsNotes->set_active((InsertDeleteFlags::NOTE & eFlags) == InsertDeleteFlags::NOTE);
    mxBtnInsFormulas->set_active((InsertDeleteFlags::FORMULA & eFlags) == InsertDeleteFlags::FORMULA);
    mxBtnInsAttrs->set_active((InsertDeleteFlags::ATTRIB & eFlags) == InsertDeleteFlags::ATTRIB);
    mxBtnInsObjects->set_active((InsertDeleteFlags::OBJECTS & eFlags) == InsertDeleteFlags::OBJECTS);
    mxBtnInsAll->set_active((InsertDeleteFlags::ALL & eFlags) == InsertDeleteFlags::ALL);
    DisableChecks( mxBtnInsAll->get_active() );
}

void ScInsertContentsDlg::SetFormulaCmdBits(const ScPasteFunc eFlags)
{
    switch( eFlags )
    {
        case ScPasteFunc::NONE: mxRbNoOp->set_active(true); break;
        case ScPasteFunc::ADD:  mxRbAdd->set_active(true); break;
        case ScPasteFunc::SUB:  mxRbSub->set_active(true); break;
        case ScPasteFunc::MUL:  mxRbMul->set_active(true); break;
        case ScPasteFunc::DIV:  mxRbDiv->set_active(true); break;
    }
}

void ScInsertContentsDlg::SetCellCmdFlags(const InsCellCmd eFlags)
{
    switch( eFlags )
    {
        case INS_NONE:       mxRbMoveNone->set_active(true); break;
        case INS_CELLSDOWN:  mxRbMoveDown->set_active(true); break;
        case INS_CELLSRIGHT: mxRbMoveRight->set_active(true); break;
        case INS_INSROWS_BEFORE:
        case INS_INSCOLS_BEFORE:
        case INS_INSROWS_AFTER:
        case INS_INSCOLS_AFTER: break;
    }
}

void ScInsertContentsDlg::SetContentsFlags(const InsertContentsFlags eFlags)
{
    mxBtnSkipEmptyCells->set_active(bool(InsertContentsFlags::NoEmpty & eFlags));
    mxBtnTranspose->set_active(bool(InsertContentsFlags::Trans & eFlags));
    mxBtnLink->set_active(bool(InsertContentsFlags::Link & eFlags));
}

InsCellCmd ScInsertContentsDlg::GetMoveMode() const
{
    if ( mxRbMoveDown->get_active() )
        return INS_CELLSDOWN;
    if ( mxRbMoveRight->get_active() )
        return INS_CELLSRIGHT;

    return INS_NONE;
}

bool ScInsertContentsDlg::IsSkipEmptyCells() const
{
    return mxBtnSkipEmptyCells->get_active();
}

bool ScInsertContentsDlg::IsTranspose() const
{
    return mxBtnTranspose->get_active();
}

bool ScInsertContentsDlg::IsLink() const
{
    return mxBtnLink->get_active();
}

void ScInsertContentsDlg::DisableChecks( bool bInsAllChecked )
{
    if ( bInsAllChecked )
    {
        mxBtnInsStrings->set_sensitive(false);
        mxBtnInsNumbers->set_sensitive(false);
        mxBtnInsDateTime->set_sensitive(false);
        mxBtnInsFormulas->set_sensitive(false);
        mxBtnInsNotes->set_sensitive(false);
        mxBtnInsAttrs->set_sensitive(false);
        mxBtnInsObjects->set_sensitive(false);
    }
    else
    {
        mxBtnInsStrings->set_sensitive(true);
        mxBtnInsNumbers->set_sensitive(true);
        mxBtnInsDateTime->set_sensitive(true);
        mxBtnInsFormulas->set_sensitive(true);
        mxBtnInsNotes->set_sensitive(true);
        mxBtnInsAttrs->set_sensitive(true);

        //  "Objects" is disabled for "Fill Tables"
        if ( bFillMode )
            mxBtnInsObjects->set_sensitive(false);
        else
            mxBtnInsObjects->set_sensitive(true);
    }
}

// Link to other document -> everything else is disabled

void ScInsertContentsDlg::TestModes()
{
    if ( bOtherDoc && mxBtnLink->get_active() )
    {
        mxBtnSkipEmptyCells->set_sensitive(false);
        mxBtnTranspose->set_sensitive(false);
        mxRbNoOp->set_sensitive(false);
        mxRbAdd->set_sensitive(false);
        mxRbSub->set_sensitive(false);
        mxRbMul->set_sensitive(false);
        mxRbDiv->set_sensitive(false);

        mxRbMoveNone->set_sensitive(false);
        mxRbMoveDown->set_sensitive(false);
        mxRbMoveRight->set_sensitive(false);

        mxBtnInsAll->set_sensitive(false);
        DisableChecks(true);
    }
    else
    {
        mxBtnSkipEmptyCells->set_sensitive(true);
        mxBtnTranspose->set_sensitive(!bFillMode);
        mxRbNoOp->set_sensitive(true);
        mxRbAdd->set_sensitive(true);
        mxRbSub->set_sensitive(true);
        mxRbMul->set_sensitive(true);
        mxRbDiv->set_sensitive(true);

        mxRbMoveNone->set_sensitive(!bFillMode && !bChangeTrack && !(bMoveDownDisabled && bMoveRightDisabled));
        mxRbMoveDown->set_sensitive(!bFillMode && !bChangeTrack && !bMoveDownDisabled);
        mxRbMoveRight->set_sensitive(!bFillMode && !bChangeTrack && !bMoveRightDisabled);

        mxBtnInsAll->set_sensitive(true);
        DisableChecks( mxBtnInsAll->get_active() );
    }
}

void ScInsertContentsDlg::SetOtherDoc( bool bSet )
{
    if ( bSet != bOtherDoc )
    {
        bOtherDoc = bSet;
        TestModes();
        if ( bSet )
            mxRbMoveNone->set_active(true);
    }
}

void ScInsertContentsDlg::SetFillMode( bool bSet )
{
    if ( bSet != bFillMode )
    {
        bFillMode = bSet;
        TestModes();
        if ( bSet )
            mxRbMoveNone->set_active(true);
    }
}

void ScInsertContentsDlg::SetChangeTrack( bool bSet )
{
    if ( bSet != bChangeTrack )
    {
        bChangeTrack = bSet;
        TestModes();
        if ( bSet )
            mxRbMoveNone->set_active(true);
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
        if ( bMoveDownDisabled && mxRbMoveDown->get_active() )
            mxRbMoveNone->set_active(true);
        if ( bMoveRightDisabled && mxRbMoveRight->get_active() )
            mxRbMoveNone->set_active(true);
    }
}

IMPL_LINK(ScInsertContentsDlg, ShortCutHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnShortCutPasteValuesOnly.get())
    {
        SetInsContentsCmdBits( InsertDeleteFlags::STRING | InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME );
        SetContentsFlags( InsertContentsFlags::NONE );
    }
    else if (&rBtn == mxBtnShortCutPasteValuesFormats.get())
    {
        SetInsContentsCmdBits( InsertDeleteFlags::STRING | InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME | InsertDeleteFlags::ATTRIB );
        SetContentsFlags( InsertContentsFlags::NONE );
    }
    else if (&rBtn == mxBtnShortCutPasteTranspose.get())
    {
        SetInsContentsCmdBits( InsertDeleteFlags::ALL );
        SetContentsFlags( InsertContentsFlags::Trans );
    }
    else if (&rBtn == mxBtnShortCutPasteFormats.get())
    {
        SetInsContentsCmdBits( InsertDeleteFlags::ATTRIB );
        SetContentsFlags( InsertContentsFlags::NONE );
    }
    else
        return;

    SetCellCmdFlags( InsCellCmd::INS_NONE );
    SetFormulaCmdBits(ScPasteFunc::NONE);
    if (mxImmediately->get_active()) m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScInsertContentsDlg, InsAllHdl, weld::ToggleButton&, void)
{
    DisableChecks( mxBtnInsAll->get_active() );
}

IMPL_LINK_NOARG(ScInsertContentsDlg, LinkBtnHdl, weld::ToggleButton&, void)
{
    TestModes();
}

ScInsertContentsDlg::~ScInsertContentsDlg()
{
    ScInsertContentsDlg::nPreviousChecks2 = InsertContentsFlags::NONE;
    if(mxBtnSkipEmptyCells->get_active())
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::NoEmpty;
    if( mxBtnTranspose->get_active())
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::Trans;
    if( mxBtnLink->get_active() )
        ScInsertContentsDlg::nPreviousChecks2 |= InsertContentsFlags::Link;

    if (!bFillMode)     // in FillMode, None is checked and all three options are disabled
    {
        if ( mxRbMoveNone->get_active() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;
        else if ( mxRbMoveDown->get_active() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSDOWN;
        else if ( mxRbMoveRight->get_active() )
            ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSRIGHT;
    }
}

ScPasteFunc  ScInsertContentsDlg::GetFormulaCmdBits() const
{
    ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::NONE;
    if(mxRbAdd->get_active())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::ADD;
    else if(mxRbSub->get_active())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::SUB;
    else if(mxRbMul->get_active())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::MUL;
    else if(mxRbDiv->get_active())
        ScInsertContentsDlg::nPreviousFormulaChecks = ScPasteFunc::DIV;
    return ScInsertContentsDlg::nPreviousFormulaChecks;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
