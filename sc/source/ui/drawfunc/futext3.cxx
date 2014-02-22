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


#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <editeng/editobj.hxx>
#include <vcl/cursor.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/writingmodeitem.hxx>

#include "global.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "futext.hxx"
#include "docsh.hxx"
#include "postit.hxx"
#include "globstr.hrc"
#include "attrib.hxx"
#include "scitems.hxx"
#include "drawview.hxx"
#include "undocell.hxx"








void FuText::StopEditMode(bool /*bTextDirection*/)
{
    SdrObject* pObject = pView->GetTextEditObject();
    if( !pObject ) return;

    
    if ( pObject->GetLayer() == SC_LAYER_INTERN )
        pView->LockInternalLayer();

    ScViewData& rViewData = *pViewShell->GetViewData();
    ScDocument& rDoc = *rViewData.GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    OSL_ENSURE( pDrawLayer && (pDrawLayer == pDrDoc), "FuText::StopEditMode - missing or different drawing layers" );

    ScAddress aNotePos;
    ScPostIt* pNote = 0;
    if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObject, rViewData.GetTabNo() ) )
    {
        aNotePos = pCaptData->maStart;
        pNote = rDoc.GetNote( aNotePos );
        OSL_ENSURE( pNote && (pNote->GetCaption() == pObject), "FuText::StopEditMode - missing or invalid cell note" );
    }

    ScDocShell* pDocShell = rViewData.GetDocShell();
    ::svl::IUndoManager* pUndoMgr = rDoc.IsUndoEnabled() ? pDocShell->GetUndoManager() : 0;
    bool bNewNote = false;
    if( pNote && pUndoMgr )
    {
        /*  Put all undo actions already collected (e.g. create caption object)
            and all following undo actions (text changed) together into a ListAction. */
        SdrUndoGroup* pCalcUndo = pDrawLayer->GetCalcUndo();

        if(pCalcUndo)
        {
            const OUString aUndoStr = ScGlobal::GetRscString( STR_UNDO_EDITNOTE );
            pUndoMgr->EnterListAction( aUndoStr, aUndoStr );

            /*  Note has been created before editing, if first undo action is
                an insert action. Needed below to decide whether to drop the
                undo if editing a new note has been cancelled. */
            bNewNote = (pCalcUndo->GetActionCount() > 0) && pCalcUndo->GetAction( 0 )->ISA( SdrUndoNewObj );

            
            if( bNewNote )
                pUndoMgr->AddUndoAction( new ScUndoReplaceNote( *pDocShell, aNotePos, pNote->GetNoteData(), true, pCalcUndo ) );
            else
                pUndoMgr->AddUndoAction( pCalcUndo );
        }
    }

    if( pNote )
        rDoc.LockStreamValid(true);     

    /*  SdrObjEditView::SdrEndTextEdit() may try to delete the entire drawing
        object, if it does not contain text and has invisible border and fill.
        This must not happen for note caption objects. They will be removed
        below together with the cell note if the text is empty (independent of
        border and area formatting). It is possible to prevent automatic
        deletion by passing sal_True to this function. The return value changes
        from SDRENDTEXTEDIT_DELETED to SDRENDTEXTEDIT_SHOULDBEDELETED in this
        case. */
    /*SdrEndTextEditKind eResult =*/ pView->SdrEndTextEdit( pNote != 0 );

    
    pViewShell->SetDrawTextUndo( 0 );

    Cursor* pCur = pWindow->GetCursor();
    if( pCur && pCur->IsVisible() )
        pCur->Hide();

    if( pNote )
    {
        
        pNote->ShowCaptionTemp( aNotePos, false );

        
        pNote->AutoStamp();

        /*  If the entire text has been cleared, the cell note and its caption
            object have to be removed. */
        SdrTextObj* pTextObject = dynamic_cast< SdrTextObj* >( pObject );
        bool bDeleteNote = !pTextObject || !pTextObject->HasText();
        if( bDeleteNote )
        {
            if( pUndoMgr )
            {
                
                pDrawLayer->BeginCalcUndo(false);
                
                ScNoteData aNoteData( pNote->GetNoteData() );
                
                rDoc.ReleaseNote(aNotePos);
                
                pUndoMgr->AddUndoAction( new ScUndoReplaceNote( *pDocShell, aNotePos, aNoteData, false, pDrawLayer->GetCalcUndo() ) );
            }
            else
            {
                rDoc.ReleaseNote(aNotePos);
            }
            
            pNote = 0;
        }

        
        if( pUndoMgr )
        {
            pUndoMgr->LeaveListAction();

            /*  #i94039# Update the default name "Edit Note" of the undo action
                if the note has been created before editing or is deleted due
                to deleted text. If the note has been created *and* is deleted,
                the last undo action can be removed completely. Note: The
                function LeaveListAction() removes the last action by itself,
                if it is empty (when result is SDRENDTEXTEDIT_UNCHANGED). */
            if( bNewNote && bDeleteNote )
            {
                pUndoMgr->RemoveLastUndoAction();
            }
            else if( bNewNote || bDeleteNote )
            {
                SfxListUndoAction* pAction = dynamic_cast< SfxListUndoAction* >( pUndoMgr->GetUndoAction() );
                OSL_ENSURE( pAction, "FuText::StopEditMode - list undo action expected" );
                if( pAction )
                    pAction->SetComment( ScGlobal::GetRscString( bNewNote ? STR_UNDO_INSERTNOTE : STR_UNDO_DELETENOTE ) );
            }
        }

        
        rDoc.LockStreamValid(false);
        if (rDoc.IsStreamValid(aNotePos.Tab()))
            rDoc.SetStreamValid(aNotePos.Tab(), false);
    }
}


void FuText::StopDragMode(SdrObject* /*pObject*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
