/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "spelldialog.hxx"

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/unolingu.hxx>
#include "selectionstate.hxx"

#include "spelleng.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "editable.hxx"
#include "undoblk.hxx"



SFX_IMPL_CHILDWINDOW_WITHID( ScSpellDialogChildWindow, SID_SPELL_DIALOG )

ScSpellDialogChildWindow::ScSpellDialogChildWindow( Window* pParentP, sal_uInt16 nId,
        SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    ::svx::SpellDialogChildWindow( pParentP, nId, pBindings, pInfo ),
    mpViewShell( 0 ),
    mpViewData( 0 ),
    mpDocShell( 0 ),
    mpDoc( 0 ),
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
    return ::svx::SpellDialogChildWindow::GetInfo();
}

void ScSpellDialogChildWindow::InvalidateSpellDialog()
{
    ::svx::SpellDialogChildWindow::InvalidateSpellDialog();
}



::svx::SpellPortions ScSpellDialogChildWindow::GetNextWrongSentence( bool /*bRecheck*/ )
{
    ::svx::SpellPortions aPortions;
    if( mxEngine.get() && mpViewData )
    {
        if( EditView* pEditView = mpViewData->GetSpellingView() )
        {
            
            do
            {
                if( mbNeedNextObj )
                    mxEngine->SpellNextDocument();
                mbNeedNextObj = !mxEngine->IsFinished() && !mxEngine->SpellSentence( *pEditView, aPortions, false );
            }
            while( mbNeedNextObj );
        }

        
        if( mxEngine->IsFinished() )
            GetBindings().GetDispatcher()->Execute( SID_SPELL_DIALOG, SFX_CALLMODE_ASYNCHRON );
    }
    return aPortions;
}

void ScSpellDialogChildWindow::ApplyChangedSentence( const ::svx::SpellPortions& rChanged, bool bRecheck )
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



void ScSpellDialogChildWindow::Reset()
{
    if( mpViewShell && (mpViewShell == PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) )
    {
        if( mxEngine.get() && mxEngine->IsAnyModified() )
        {
            const ScAddress& rCursor = mxOldSel->GetCellCursor();
            SCTAB nTab = rCursor.Tab();
            SCCOL nOldCol = rCursor.Col();
            SCROW nOldRow = rCursor.Row();
            SCCOL nNewCol = mpViewData->GetCurX();
            SCROW nNewRow = mpViewData->GetCurY();
            mpDocShell->GetUndoManager()->AddUndoAction( new ScUndoConversion(
                mpDocShell, mpViewData->GetMarkData(),
                nOldCol, nOldRow, nTab, mxUndoDoc.release(),
                nNewCol, nNewRow, nTab, mxRedoDoc.release(),
                ScConversionParam( SC_CONVERSION_SPELLCHECK ) ) );
            mpDoc->SetDirty();
            mpDocShell->SetDocumentModified();
        }

        mpViewData->SetSpellingView( 0 );
        mpViewShell->KillEditView( true );
        mpDocShell->PostPaintGridAll();
        mpViewShell->UpdateInputHandler();
        mpDoc->EnableIdle(mbOldIdleEnabled);
    }
    mxEngine.reset();
    mxUndoDoc.reset();
    mxRedoDoc.reset();
    mxOldSel.reset();
    mxOldRangeList.reset();
    mpViewShell = 0;
    mpViewData = 0;
    mpDocShell = 0;
    mpDoc = 0;
    mbNeedNextObj = false;
    mbOldIdleEnabled = true;
}

void ScSpellDialogChildWindow::Init()
{
    if( mpViewShell )
        return;
    if( (mpViewShell = PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) == 0 )
        return;

    mpViewData = mpViewShell->GetViewData();

    
    if( mpViewData->HasEditView( mpViewData->GetActivePart() ) )
        SC_MOD()->InputEnterHandler();

    mxOldSel.reset( new ScSelectionState( *mpViewData ) );

    mpDocShell = mpViewData->GetDocShell();
    mpDoc = mpDocShell->GetDocument();

    const ScAddress& rCursor = mxOldSel->GetCellCursor();
    SCCOL nCol = rCursor.Col();
    SCROW nRow = rCursor.Row();
    SCTAB nTab = rCursor.Tab();

    ScMarkData& rMarkData = mpViewData->GetMarkData();

    mxOldRangeList.reset(new ScRangeList);
    rMarkData.FillRangeListWithMarks(mxOldRangeList.get(), true);

    rMarkData.MarkToMulti();

    switch( mxOldSel->GetSelectionType() )
    {
        case SC_SELECTTYPE_NONE:
        case SC_SELECTTYPE_SHEET:
        {
            
            ScEditableTester aTester( mpDoc, rMarkData );
            if( !aTester.IsEditable() )
            {
                
                
                
                

                
                return;
            }
        }
        break;

        



        default:
            OSL_FAIL( "ScSpellDialogChildWindow::Init - unknown selection type" );
    }

    mbOldIdleEnabled = mpDoc->IsIdleEnabled();
    mpDoc->EnableIdle(false);   

    

    mxUndoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxUndoDoc->InitUndo( mpDoc, nTab, nTab );
    mxRedoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxRedoDoc->InitUndo( mpDoc, nTab, nTab );

    if ( rMarkData.GetSelectCount() > 1 )
    {
        ScMarkData::iterator itr = rMarkData.begin(), itrEnd = rMarkData.end();
        for (; itr != itrEnd; ++itr)
        {
            if( *itr != nTab )
            {
                mxUndoDoc->AddUndoTab( *itr, *itr );
                mxRedoDoc->AddUndoTab( *itr, *itr );
            }
        }
    }

    

    mxEngine.reset( new ScSpellingEngine(
        mpDoc->GetEnginePool(), *mpViewData, mxUndoDoc.get(), mxRedoDoc.get(), LinguMgr::GetSpellChecker() ) );
    mxEngine->SetRefDevice( mpViewData->GetActiveWin() );

    mpViewShell->MakeEditView( mxEngine.get(), nCol, nRow );
    EditView* pEditView = mpViewData->GetEditView( mpViewData->GetActivePart() );
    mpViewData->SetSpellingView( pEditView );
    Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    mxEngine->SetControlWord( EE_CNTRL_USECHARATTRIBS );
    mxEngine->EnableUndo( false );
    mxEngine->SetPaperSize( aRect.GetSize() );
    mxEngine->SetText( EMPTY_OUSTRING );
    mxEngine->ClearModifyFlag();

    mbNeedNextObj = true;
}

bool ScSpellDialogChildWindow::IsSelectionChanged()
{
    if( !mxOldRangeList.get() || !mpViewShell || (mpViewShell != PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) )
        return true;

    if( EditView* pEditView = mpViewData->GetSpellingView() )
        if( pEditView->GetEditEngine() != mxEngine.get() )
            return true;

    ScRangeList aCurrentRangeList;
    mpViewData->GetMarkData().FillRangeListWithMarks(&aCurrentRangeList, true);

    return (*mxOldRangeList != aCurrentRangeList);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
