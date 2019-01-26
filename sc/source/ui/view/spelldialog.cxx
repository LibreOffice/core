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

#include <spelldialog.hxx>

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/unolingu.hxx>
#include <selectionstate.hxx>

#include <spelleng.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <scmod.hxx>
#include <editable.hxx>
#include <undoblk.hxx>
#include <gridwin.hxx>
#include <refupdatecontext.hxx>

SFX_IMPL_CHILDWINDOW_WITHID( ScSpellDialogChildWindow, SID_SPELL_DIALOG )

ScSpellDialogChildWindow::ScSpellDialogChildWindow( vcl::Window* pParentP, sal_uInt16 nId,
        SfxBindings* pBindings, SAL_UNUSED_PARAMETER SfxChildWinInfo* /*pInfo*/ ) :
    svx::SpellDialogChildWindow( pParentP, nId, pBindings ),
    mpViewShell( nullptr ),
    mpViewData( nullptr ),
    mpDocShell( nullptr ),
    mpDoc( nullptr ),
    mbNeedNextObj( false ),
    mbOldIdleEnabled(true)
{
    Init();
}

ScSpellDialogChildWindow::~ScSpellDialogChildWindow()
{
    Reset();
}

SfxChildWinInfo ScSpellDialogChildWindow::GetInfo() const
{
    return svx::SpellDialogChildWindow::GetInfo();
}

void ScSpellDialogChildWindow::InvalidateSpellDialog()
{
    svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

// protected ------------------------------------------------------------------

svx::SpellPortions ScSpellDialogChildWindow::GetNextWrongSentence( bool /*bRecheck*/ )
{
    svx::SpellPortions aPortions;
    if( mxEngine.get() && mpViewData )
    {
        if( EditView* pEditView = mpViewData->GetSpellingView() )
        {
            // edit engine handles cell iteration internally
            do
            {
                if( mbNeedNextObj )
                    mxEngine->SpellNextDocument();
                mbNeedNextObj = !mxEngine->IsFinished() && !mxEngine->SpellSentence( *pEditView, aPortions );
            }
            while( mbNeedNextObj );
        }
    }
    return aPortions;
}

void ScSpellDialogChildWindow::ApplyChangedSentence( const svx::SpellPortions& rChanged, bool bRecheck )
{
    if( mxEngine.get() && mpViewData )
        if( EditView* pEditView = mpViewData->GetSpellingView() )
            mxEngine->ApplyChangedSentence( *pEditView, rChanged, bRecheck );
}

void ScSpellDialogChildWindow::GetFocus()
{
    if( IsSelectionChanged() )
    {
        Reset();
        InvalidateSpellDialog();
        Init();
    }
}

void ScSpellDialogChildWindow::LoseFocus()
{
}

// private --------------------------------------------------------------------

void ScSpellDialogChildWindow::Reset()
{
    if( mpViewShell && (mpViewShell == dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() ))  )
    {
        if( mxEngine.get() && mxEngine->IsAnyModified() )
        {
            const ScAddress& rCursor = mxOldSel->GetCellCursor();
            SCTAB nTab = rCursor.Tab();
            SCCOL nOldCol = rCursor.Col();
            SCROW nOldRow = rCursor.Row();
            SCCOL nNewCol = mpViewData->GetCurX();
            SCROW nNewRow = mpViewData->GetCurY();
            mpDocShell->GetUndoManager()->AddUndoAction( std::make_unique<ScUndoConversion>(
                mpDocShell, mpViewData->GetMarkData(),
                nOldCol, nOldRow, nTab, std::move(mxUndoDoc),
                nNewCol, nNewRow, nTab, std::move(mxRedoDoc),
                ScConversionParam( SC_CONVERSION_SPELLCHECK ) ) );

            sc::SetFormulaDirtyContext aCxt;
            mpDoc->SetAllFormulasDirty(aCxt);

            mpDocShell->SetDocumentModified();
        }

        mpViewData->SetSpellingView( nullptr );
        mpViewShell->KillEditView( true );
        mpDocShell->PostPaintGridAll();
        mpViewShell->UpdateInputHandler();
        mpDoc->EnableIdle(mbOldIdleEnabled);
    }
    mxEngine.reset();
    mxUndoDoc.reset();
    mxRedoDoc.reset();
    mxOldSel.reset();
    mxOldRangeList.clear();
    mpViewShell = nullptr;
    mpViewData = nullptr;
    mpDocShell = nullptr;
    mpDoc = nullptr;
    mbNeedNextObj = false;
    mbOldIdleEnabled = true;
}

void ScSpellDialogChildWindow::Init()
{
    if( mpViewShell )
        return;
    if( (mpViewShell = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() )) == nullptr  )
        return;

    mpViewData = &mpViewShell->GetViewData();

    // exit edit mode - TODO support spelling in edit mode
    if( mpViewData->HasEditView( mpViewData->GetActivePart() ) )
        SC_MOD()->InputEnterHandler();

    mxOldSel.reset( new ScSelectionState( *mpViewData ) );

    mpDocShell = mpViewData->GetDocShell();
    mpDoc = &mpDocShell->GetDocument();

    const ScAddress& rCursor = mxOldSel->GetCellCursor();
    SCCOL nCol = rCursor.Col();
    SCROW nRow = rCursor.Row();
    SCTAB nTab = rCursor.Tab();

    ScMarkData& rMarkData = mpViewData->GetMarkData();

    mxOldRangeList = new ScRangeList;
    rMarkData.FillRangeListWithMarks(mxOldRangeList.get(), true);

    rMarkData.MarkToMulti();

    switch( mxOldSel->GetSelectionType() )
    {
        case SC_SELECTTYPE_NONE:
        case SC_SELECTTYPE_SHEET:
        {
            // test if there is something editable
            ScEditableTester aTester( mpDoc, rMarkData );
            if( !aTester.IsEditable() )
            {
                // #i85751# Don't show a ErrorMessage here, because the vcl
                // parent of the InfoBox is not fully initialized yet.
                // This leads to problems in the modality behaviour of the
                // ScSpellDialogChildWindow.

                //mpViewShell->ErrorMessage( aTester.GetMessageId() );
                return;
            }
        }
        break;

        // edit mode exited, see TODO above
//        case SC_SELECTTYPE_EDITCELL:
//        break;

        default:
            OSL_FAIL( "ScSpellDialogChildWindow::Init - unknown selection type" );
    }

    mbOldIdleEnabled = mpDoc->IsIdleEnabled();
    mpDoc->EnableIdle(false);   // stop online spelling

    // *** create Undo/Redo documents *** -------------------------------------

    mxUndoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxUndoDoc->InitUndo( mpDoc, nTab, nTab );
    mxRedoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxRedoDoc->InitUndo( mpDoc, nTab, nTab );

    if ( rMarkData.GetSelectCount() > 1 )
    {
        for (const auto& rTab : rMarkData)
        {
            if( rTab != nTab )
            {
                mxUndoDoc->AddUndoTab( rTab, rTab );
                mxRedoDoc->AddUndoTab( rTab, rTab );
            }
        }
    }

    // *** create and init the edit engine *** --------------------------------

    mxEngine.reset( new ScSpellingEngine(
        mpDoc->GetEnginePool(), *mpViewData, mxUndoDoc.get(), mxRedoDoc.get(), LinguMgr::GetSpellChecker() ) );
    mxEngine->SetRefDevice( mpViewData->GetActiveWin() );

    mpViewShell->MakeEditView( mxEngine.get(), nCol, nRow );
    EditView* pEditView = mpViewData->GetEditView( mpViewData->GetActivePart() );
    mpViewData->SetSpellingView( pEditView );
    tools::Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    mxEngine->SetControlWord( EEControlBits::USECHARATTRIBS );
    mxEngine->EnableUndo( false );
    mxEngine->SetPaperSize( aRect.GetSize() );
    mxEngine->SetText( EMPTY_OUSTRING );
    mxEngine->ClearModifyFlag();

    mbNeedNextObj = true;
}

bool ScSpellDialogChildWindow::IsSelectionChanged()
{
    if (!mxOldRangeList || !mpViewShell
        || (mpViewShell != dynamic_cast<ScTabViewShell*>(SfxViewShell::Current())))
        return true;

    if( EditView* pEditView = mpViewData->GetSpellingView() )
        if( pEditView->GetEditEngine() != mxEngine.get() )
            return true;

    ScRangeList aCurrentRangeList;
    mpViewData->GetMarkData().FillRangeListWithMarks(&aCurrentRangeList, true);

    return (*mxOldRangeList != aCurrentRangeList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
