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

#include <svx/svdocapt.hxx>
#include <svx/svdundo.hxx>
#include <vcl/cursor.hxx>

#include <global.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <futext.hxx>
#include <docsh.hxx>
#include <postit.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <drawview.hxx>
#include <undocell.hxx>

//  Editing of Note-Key-Objects has to be stopped always via StopEditMode,
//  so that changes are taken over into the document!
//  (Fontwork-Execute in drawsh and drtxtob does not happen for Key-Objects)

void FuText::StopEditMode()
{
    SdrObject* pObject = pView->GetTextEditObject();
    if( !pObject ) return;

    // relock the internal layer that has been unlocked in FuText::SetInEditMode()
    if ( pObject->GetLayer() == SC_LAYER_INTERN )
        pView->LockInternalLayer();

    ScViewData& rViewData = rViewShell.GetViewData();
    ScDocument& rDoc = *rViewData.GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    OSL_ENSURE( pDrawLayer && (pDrawLayer == pDrDoc), "FuText::StopEditMode - missing or different drawing layers" );

    ScAddress aNotePos;
    ScPostIt* pNote = nullptr;
    if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObject, rViewData.GetTabNo() ) )
    {
        aNotePos = pCaptData->maStart;
        pNote = rDoc.GetNote( aNotePos );
        OSL_ENSURE( pNote && (pNote->GetCaption() == pObject), "FuText::StopEditMode - missing or invalid cell note" );
    }

    ScDocShell* pDocShell = rViewData.GetDocShell();
    SfxUndoManager* pUndoMgr = rDoc.IsUndoEnabled() ? pDocShell->GetUndoManager() : nullptr;
    bool bNewNote = false;
    if( pNote && pUndoMgr )
    {
        /*  Put all undo actions already collected (e.g. create caption object)
            and all following undo actions (text changed) together into a ListAction. */
        std::unique_ptr<SdrUndoGroup> pCalcUndo = pDrawLayer->GetCalcUndo();

        if(pCalcUndo)
        {
            const OUString aUndoStr = ScResId( STR_UNDO_EDITNOTE );
            pUndoMgr->EnterListAction( aUndoStr, aUndoStr, 0, rViewShell.GetViewShellId() );

            /*  Note has been created before editing, if first undo action is
                an insert action. Needed below to decide whether to drop the
                undo if editing a new note has been cancelled. */
            bNewNote = (pCalcUndo->GetActionCount() > 0) && dynamic_cast< SdrUndoNewObj* >(pCalcUndo->GetAction( 0 ));

            // create a "insert note" undo action if needed
            if( bNewNote )
                pUndoMgr->AddUndoAction( std::make_unique<ScUndoReplaceNote>( *pDocShell, aNotePos, pNote->GetNoteData(), true, std::move(pCalcUndo) ) );
            else
                pUndoMgr->AddUndoAction( std::move(pCalcUndo) );
        }
    }

    if( pNote )
        rDoc.LockStreamValid(true);     // only the affected sheet is invalidated below

    /*  SdrObjEditView::SdrEndTextEdit() may try to delete the entire drawing
        object, if it does not contain text and has invisible border and fill.
        This must not happen for note caption objects. They will be removed
        below together with the cell note if the text is empty (independent of
        border and area formatting). It is possible to prevent automatic
        deletion by passing sal_True to this function. The return value changes
        from SdrEndTextEditKind::Deleted to SdrEndTextEditKind::ShouldBeDeleted in this
        case. */
    /*SdrEndTextEditKind eResult =*/ pView->SdrEndTextEdit( pNote != nullptr );

    // or ScEndTextEdit (with drawview.hxx)
    rViewShell.SetDrawTextUndo( nullptr );

    vcl::Cursor* pCur = pWindow->GetCursor();
    if( pCur && pCur->IsVisible() )
        pCur->Hide();

    if( pNote )
    {
        ScTabView::OnLOKNoteStateChanged( pNote );

        // hide the caption object if it is in hidden state
        pNote->ShowCaptionTemp( aNotePos, false );

        // update author and date
        pNote->AutoStamp();

        /*  If the entire text has been cleared, the cell note and its caption
            object have to be removed. */
        SdrTextObj* pTextObject = dynamic_cast< SdrTextObj* >( pObject );
        bool bDeleteNote = !pTextObject || !pTextObject->HasText();
        if( bDeleteNote )
        {
            if( pUndoMgr )
            {
                // collect the "remove object" drawing undo action created by DeleteNote()
                pDrawLayer->BeginCalcUndo(false);
                // rescue note data before deletion
                ScNoteData aNoteData( pNote->GetNoteData() );
                // delete note from document (removes caption, but does not delete it)
                rDoc.ReleaseNote(aNotePos);
                // create undo action for removed note
                pUndoMgr->AddUndoAction( std::make_unique<ScUndoReplaceNote>( *pDocShell, aNotePos, aNoteData, false, pDrawLayer->GetCalcUndo() ) );
            }
            else
            {
                rDoc.ReleaseNote(aNotePos);
            }
            // ScDocument::DeleteNote has deleted the note that pNote points to
            pNote = nullptr;
        }

        // finalize the undo list action
        if( pUndoMgr )
        {
            pUndoMgr->LeaveListAction();

            /*  #i94039# Update the default name "Edit Note" of the undo action
                if the note has been created before editing or is deleted due
                to deleted text. If the note has been created *and* is deleted,
                the last undo action can be removed completely. Note: The
                function LeaveListAction() removes the last action by itself,
                if it is empty (when result is SdrEndTextEditKind::Unchanged). */
            if( bNewNote && bDeleteNote )
            {
                pUndoMgr->RemoveLastUndoAction();
            }
            else if( bNewNote || bDeleteNote )
            {
                SfxListUndoAction* pAction = dynamic_cast< SfxListUndoAction* >( pUndoMgr->GetUndoAction() );
                OSL_ENSURE( pAction, "FuText::StopEditMode - list undo action expected" );
                if( pAction )
                    pAction->SetComment( ScResId( bNewNote ? STR_UNDO_INSERTNOTE : STR_UNDO_DELETENOTE ) );
            }
        }

        // invalidate stream positions only for the affected sheet
        rDoc.LockStreamValid(false);
        rDoc.SetStreamValid(aNotePos.Tab(), false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
