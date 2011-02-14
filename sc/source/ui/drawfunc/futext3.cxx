/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#include "tabvwsh.hxx"          // oder GetDocument irgendwo
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

// ------------------------------------------------------------------------------------
//  Editieren von Notiz-Legendenobjekten muss immer ueber StopEditMode beendet werden,
//  damit die Aenderungen ins Dokument uebernommen werden!
//  (Fontwork-Execute in drawsh und drtxtob passiert nicht fuer Legendenobjekte)
//  bTextDirection=sal_True means that this function is called from SID_TEXTDIRECTION_XXX(drtxtob.cxx).
// ------------------------------------------------------------------------------------

void FuText::StopEditMode(sal_Bool /*bTextDirection*/)
{
    SdrObject* pObject = pView->GetTextEditObject();
    if( !pObject ) return;

    // relock the internal layer that has been unlocked in FuText::SetInEditMode()
    if ( pObject->GetLayer() == SC_LAYER_INTERN )
        pView->LockInternalLayer();

    ScViewData& rViewData = *pViewShell->GetViewData();
    ScDocument& rDoc = *rViewData.GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    DBG_ASSERT( pDrawLayer && (pDrawLayer == pDrDoc), "FuText::StopEditMode - missing or different drawing layers" );

    ScAddress aNotePos;
    ScPostIt* pNote = 0;
    if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObject, rViewData.GetTabNo() ) )
    {
        aNotePos = pCaptData->maStart;
        pNote = rDoc.GetNote( aNotePos );
        DBG_ASSERT( pNote && (pNote->GetCaption() == pObject), "FuText::StopEditMode - missing or invalid cell note" );
    }

    ScDocShell* pDocShell = rViewData.GetDocShell();
    ::svl::IUndoManager* pUndoMgr = rDoc.IsUndoEnabled() ? pDocShell->GetUndoManager() : 0;
    bool bNewNote = false;
    if( pNote && pUndoMgr )
    {
        /*  Put all undo actions already collected (e.g. create caption object)
            and all following undo actions (text changed) together into a ListAction. */
        String aUndoStr = ScGlobal::GetRscString( STR_UNDO_EDITNOTE );
        pUndoMgr->EnterListAction( aUndoStr, aUndoStr );
        if( SdrUndoGroup* pCalcUndo = pDrawLayer->GetCalcUndo() )
        {
            /*  Note has been created before editing, if first undo action is
                an insert action. Needed below to decide whether to drop the
                undo if editing a new note has been cancelled. */
            bNewNote = (pCalcUndo->GetActionCount() > 0) && pCalcUndo->GetAction( 0 )->ISA( SdrUndoNewObj );
            // create a "insert note" undo action if needed
            if( bNewNote )
                pUndoMgr->AddUndoAction( new ScUndoReplaceNote( *pDocShell, aNotePos, pNote->GetNoteData(), true, pCalcUndo ) );
            else
                pUndoMgr->AddUndoAction( pCalcUndo );
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
        from SDRENDTEXTEDIT_DELETED to SDRENDTEXTEDIT_SHOULDBEDELETED in this
        case. */
    /*SdrEndTextEditKind eResult =*/ pView->SdrEndTextEdit( pNote != 0 );

    // or ScEndTextEdit (with drawview.hxx)
    pViewShell->SetDrawTextUndo( 0 );

    Cursor* pCur = pWindow->GetCursor();
    if( pCur && pCur->IsVisible() )
        pCur->Hide();

    if( pNote )
    {
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
                pDrawLayer->BeginCalcUndo();
                // rescue note data before deletion
                ScNoteData aNoteData( pNote->GetNoteData() );
                // delete note from document (removes caption, but does not delete it)
                rDoc.DeleteNote( aNotePos );
                // create undo action for removed note
                pUndoMgr->AddUndoAction( new ScUndoReplaceNote( *pDocShell, aNotePos, aNoteData, false, pDrawLayer->GetCalcUndo() ) );
            }
            else
            {
                rDoc.DeleteNote( aNotePos );
            }
            // ScDocument::DeleteNote has deleted the note that pNote points to
            pNote = 0;
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
                if it is empty (when result is SDRENDTEXTEDIT_UNCHANGED). */
            if( bNewNote && bDeleteNote )
            {
                pUndoMgr->RemoveLastUndoAction();
            }
            else if( bNewNote || bDeleteNote )
            {
                SfxListUndoAction* pAction = dynamic_cast< SfxListUndoAction* >( pUndoMgr->GetUndoAction() );
                DBG_ASSERT( pAction, "FuText::StopEditMode - list undo action expected" );
                if( pAction )
                    pAction->SetComment( ScGlobal::GetRscString( bNewNote ? STR_UNDO_INSERTNOTE : STR_UNDO_DELETENOTE ) );
            }
        }

        // invalidate stream positions only for the affected sheet
        rDoc.LockStreamValid(false);
        if (rDoc.IsStreamValid(aNotePos.Tab()))
            rDoc.SetStreamValid(aNotePos.Tab(), sal_False);
    }
}

// Called following an EndDragObj() to update the new note rectangle position
void FuText::StopDragMode(SdrObject* /*pObject*/)
{
#if 0 // DR
    ScViewData& rViewData = *pViewShell->GetViewData();
    if( ScDrawObjData* pData = ScDrawLayer::GetNoteCaptionData( pObject, rViewData.GetTabNo() ) )
    {
        ScDocument& rDoc = *rViewData.GetDocument();
        const ScAddress& rPos = pData->maStart;
        ScPostIt* pNote = rDoc.GetNote( rPos );
        DBG_ASSERT( pNote && (pNote->GetCaption() == pObject), "FuText::StopDragMode - missing or invalid cell note" );
        if( pNote )
        {
            Rectangle aOldRect = pNote->CalcRectangle( rDoc, rPos );
            Rectangle aNewRect = pObject->GetLogicRect();
            if( aOldRect != aNewRect )
            {
                pNote->UpdateFromRectangle( rDoc, rPos, aNewRect );
                OutlinerParaObject* pPObj = pCaption->GetOutlinerParaObject();
                bool bVertical = (pPObj && pPObj->IsVertical());
                // The new height/width is honoured if property item is reset.
                if(!bVertical && aNewRect.Bottom() - aNewRect.Top() > aOldRect.Bottom() - aOldRect.Top())
                {
                    if(pCaption->IsAutoGrowHeight() && !bVertical)
                    {
                        pCaption->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
                        aNote.SetItemSet( *pDoc, pCaption->GetMergedItemSet() );
                    }
                }
                else if(bVertical && aNewRect.Right() - aNewRect.Left() > aOldRect.Right() - aOldRect.Left())
                {
                    if(pCaption->IsAutoGrowWidth() && bVertical)
                    {
                        pCaption->SetMergedItem( SdrTextAutoGrowWidthItem( false ) );
                        aNote.SetItemSet( *pDoc, pCaption->GetMergedItemSet() );
                    }
                }
                pViewShell->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );

                // This repaint should not be necessary but it cleans
                // up the 'marks' left behind  by the note handles
                // now that notes can simultaineously have handles and edit active.
                ScRange aDrawRange = rDoc.GetRange( rPos.Tab(), aOldRect );
                // Set Start/End Row to previous/next row to allow for handles.
                if( aDrawRange.aStart.Row() > 0 )
                    aDrawRange.aStart.IncRow( -1 );
                if( aDrawRange.aEnd.Row() < MAXROW )
                    aDrawRange.aEnd.IncRow( 1 );
                ScDocShell* pDocSh = rViewData.GetDocShell();
                pDocSh->PostPaint( aDrawRange, PAINT_GRID| PAINT_EXTRAS);
            }
        }
    }
#endif
}

