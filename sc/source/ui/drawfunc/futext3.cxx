/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: futext3.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:42:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <svx/editeng.hxx>
#include <svx/outlobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <vcl/cursor.hxx>
#include <sfx2/objsh.hxx>
#include <svx/writingmodeitem.hxx>

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

// ------------------------------------------------------------------------------------
//  Editieren von Notiz-Legendenobjekten muss immer ueber StopEditMode beendet werden,
//  damit die Aenderungen ins Dokument uebernommen werden!
//  (Fontwork-Execute in drawsh und drtxtob passiert nicht fuer Legendenobjekte)
//  bTextDirection=TRUE means that this function is called from SID_TEXTDIRECTION_XXX(drtxtob.cxx).
// ------------------------------------------------------------------------------------

void FuText::StopEditMode(BOOL bTextDirection)
{
    BOOL bComment = FALSE;
    ScAddress aTabPos;
    BOOL bVertical = FALSE;

    SdrObject* pObject = pView->GetTextEditObject();
    if ( pObject && pObject->GetLayer()==SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj) )
    {
        ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObject, pViewShell->GetViewData()->GetTabNo() );
        if( pData )
        {
            aTabPos = ScAddress( pData->aStt);
            bComment = TRUE;
        }
        const SfxItemSet& rSet = pObject->GetMergedItemSet();
        bVertical = static_cast<const SvxWritingModeItem&> (rSet.Get (SDRATTR_TEXTDIRECTION)).GetValue() == com::sun::star::text::WritingMode_TB_RL;
    }

    ScDocument* pDoc = pViewShell->GetViewData()->GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());

    SfxObjectShell* pObjSh = pViewShell->GetViewData()->GetSfxDocShell();
    SfxUndoManager* pUndoMan = NULL;
    if (bUndo)
        pUndoMan = pObjSh->GetUndoManager();
    if ( bComment && bUndo)
    {
        // fade in, edit, fade out, note change together into a ListAction

        String aUndoStr = ScGlobal::GetRscString( STR_UNDO_EDITNOTE );
        pUndoMan->EnterListAction( aUndoStr, aUndoStr );

        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        SdrUndoGroup* pShowUndo = pModel->GetCalcUndo();
        if (pShowUndo)
            pUndoMan->AddUndoAction( pShowUndo );
    }

    SdrEndTextEditKind eResult = pView->SdrEndTextEdit();
    pViewShell->SetDrawTextUndo(NULL);  // or ScEndTextEdit (with drawview.hxx)

    Cursor* pCur = pWindow->GetCursor();
    if (pCur && pCur->IsVisible())
        pCur->Hide();

    if ( bComment )
    {
        ScPostIt aNote(pDoc);
        BOOL bWas = pDoc->GetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );
        if( bWas )
        {
           SdrLayer* pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
           if (pLockLayer && !pView->IsLayerLocked(pLockLayer->GetName()))
             pView->SetLayerLocked( pLockLayer->GetName(), TRUE );
        }

        //  Ignore if text unchanged. If called from a change in
        //  TextDirection mode then always enter as we need to
        //  store the new EditTextObject.

        if ( eResult != SDRENDTEXTEDIT_UNCHANGED || !bWas || !aNote.IsShown() || bTextDirection)
        {
            ::std::auto_ptr<EditTextObject> pEditText ;
            if ( eResult != SDRENDTEXTEDIT_DELETED )
            {
                OutlinerParaObject* pParaObj = pObject->GetOutlinerParaObject();
                if ( pParaObj )
                {
                    pParaObj->SetVertical(bVertical);
                    ScNoteEditEngine& rEE = pDoc->GetNoteEngine();
                    rEE.SetVertical(bVertical);
                    const EditTextObject& rTextObj = pParaObj->GetTextObject();
                    rEE.SetText(rTextObj);
                    sal_uInt16 nCount = rEE.GetParagraphCount();
                    for( sal_uInt16 nPara = 0; nPara < nCount; ++nPara )
                    {
                        String aParaText( rEE.GetText( nPara ) );
                        if( aParaText.Len() )
                        {
                            SfxItemSet aSet( rTextObj.GetParaAttribs( nPara));
                            rEE.SetParaAttribs(nPara, aSet);
                        }
                    }
                    pEditText.reset(rEE.CreateTextObject());
                }
            }
            Rectangle aNewRect;
            Rectangle aOldRect = aNote.GetRectangle();
            SdrCaptionObj* pCaption = static_cast<SdrCaptionObj*>(pObject);
            if(pCaption)
            {
                aNewRect = pCaption->GetLogicRect();
                if(aOldRect != aNewRect)
                    aNote.SetRectangle(aNewRect);
            }
            aNote.SetEditTextObject(pEditText.get());    // if pEditText is NULL, then aNote.mpEditObj will be reset().
            aNote.AutoStamp();
            aNote.SetItemSet(pCaption->GetMergedItemSet());

            BOOL bRemove = (!aNote.IsShown() || aNote.IsEmpty() || !bWas)  && !bTextDirection;
            if ( bRemove )
                aNote.SetShown( FALSE );
            pViewShell->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );  // with Undo

            if ( bRemove && eResult != SDRENDTEXTEDIT_DELETED )     // Object Delete ?
            {
                // Lock the internal layer here - UnLocked in SetInEditMode().
                SdrLayer* pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
                if (pLockLayer && !pView->IsLayerLocked(pLockLayer->GetName()))
                    pView->SetLayerLocked( pLockLayer->GetName(), TRUE );

                SdrPage* pPage = pDrDoc->GetPage( static_cast<sal_uInt16>(aTabPos.Tab()) );
                pDrDoc->AddUndo( new SdrUndoRemoveObj( *pObject ) );
                pPage->RemoveObject( pObject->GetOrdNum() );
                // #39351# RemoveObject loescht nicht (analog zu anderen Containern)
                // trotzden kein "delete pObject" mehr, das Objekt gehoert jetzt dem Undo
            }
        }
        if (pUndoMan)
            pUndoMan->LeaveListAction();

        // This repaint should not be necessary but it cleans
        // up the 'marks' left behind  by the note handles and outline
        // now that notes can simultaineously have handles and edit active.
        ScRange aDrawRange(pDoc->GetRange(aTabPos.Tab(), aNote.GetRectangle()));

        // Set Start/End Row to previous/next row to allow for handles.
        SCROW aStartRow = aDrawRange.aStart.Row();
        if(aStartRow > 0)
            aDrawRange.aStart.SetRow(aStartRow - 1);
        SCROW aEndRow = aDrawRange.aEnd.Row();
        if(aEndRow < MAXROW)
            aDrawRange.aEnd.SetRow(aEndRow + 1);
        ScDocShell* pDocSh = pViewShell->GetViewData()->GetDocShell();
        pDocSh->PostPaint( aDrawRange, PAINT_GRID| PAINT_EXTRAS);
    }
}

// Called following an EndDragObj() to update the new note rectangle position
void FuText::StopDragMode(SdrObject* pObject)
{
    BOOL bComment = FALSE;
    ScAddress aTabPos;

    if ( pObject && pObject->GetLayer()==SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj) )
    {
        ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObject, pViewShell->GetViewData()->GetTabNo() );
        if( pData )
        {
            aTabPos = pData->aStt;
            bComment = TRUE;
        }
    }

    if ( bComment )
    {
        ScDocument* pDoc = pViewShell->GetViewData()->GetDocument();
        if(pDoc)
        {
            ScPostIt aNote(pDoc);
            if(pDoc->GetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote ))
            {
                Rectangle aNewRect;
                Rectangle aOldRect = aNote.GetRectangle();
                SdrCaptionObj* pCaption = static_cast<SdrCaptionObj*>(pObject);
                if(pCaption)
                    aNewRect = pCaption->GetLogicRect();
                if(pCaption && aOldRect != aNewRect)
                {
                    aNote.SetRectangle(aNewRect);
                    OutlinerParaObject* pPObj = pCaption->GetOutlinerParaObject();
                    bool bVertical = (pPObj && pPObj->IsVertical());
                    // The new height/width is honoured if property item is reset.
                    if(!bVertical && aNewRect.Bottom() - aNewRect.Top() > aOldRect.Bottom() - aOldRect.Top())
                    {
                        if(pCaption->IsAutoGrowHeight() && !bVertical)
                        {
                            pCaption->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
                            aNote.SetItemSet(pCaption->GetMergedItemSet());
                        }
                    }
                    else if(bVertical && aNewRect.Right() - aNewRect.Left() > aOldRect.Right() - aOldRect.Left())
                    {
                        if(pCaption->IsAutoGrowWidth() && bVertical)
                        {
                            pCaption->SetMergedItem( SdrTextAutoGrowWidthItem( false ) );
                            aNote.SetItemSet(pCaption->GetMergedItemSet());
                        }
                    }
                    pViewShell->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );
                    // This repaint should not be necessary but it cleans
                    // up the 'marks' left behind  by the note handles
                    // now that notes can simultaineously have handles and edit active.
                    ScRange aDrawRange(pDoc->GetRange(aTabPos.Tab(), aOldRect));
                    // Set Start/End Row to previous/next row to allow for handles.
                    SCROW aStartRow = aDrawRange.aStart.Row();
                    if(aStartRow > 0)
                        aDrawRange.aStart.SetRow(aStartRow - 1);
                    SCROW aEndRow = aDrawRange.aEnd.Row();
                    if(aEndRow < MAXROW)
                        aDrawRange.aEnd.SetRow(aEndRow + 1);
                    ScDocShell* pDocSh = pViewShell->GetViewData()->GetDocShell();
                    pDocSh->PostPaint( aDrawRange, PAINT_GRID| PAINT_EXTRAS);
                }
            }
        }
    }
}

