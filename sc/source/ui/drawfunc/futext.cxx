/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <svx/svddef.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/svdotext.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svx/svdlegacy.hxx>

#include "futext.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "drawview.hxx"

// #98185# Create default drawing objects via keyboard
#include "scresid.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------

void lcl_InvalidateAttribs( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_HYPERLINK_GETLINK );
    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
}

void lcl_UpdateHyphenator( Outliner& rOutliner, SdrObject* pObj )
{
    // use hyphenator only if hyphenation attribute is set
    if ( pObj && ((const SfxBoolItem&)pObj->GetMergedItem(EE_PARA_HYPHENATE)).GetValue() ) {
            com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xHyphenator( LinguMgr::GetHyphenator() );
            rOutliner.SetHyphenator( xHyphenator );
    }
}

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

FuText::FuText(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq),
    pTextObj(NULL)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuText::~FuText()
{
//  StopEditMode();                 // in Deactivate !
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return (sal_True);                 // Event von der SdrView ausgewertet

    if ( pView->IsTextEdit() )
    {
        if( !IsSizingOrMovingNote(rMEvt) )
            StopEditMode();            // Danebengeklickt, Ende mit Edit
        pView->SetViewEditMode(SDREDITMODE_CREATE);
    }

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aLogicPos);

        sal_uLong nHdlNum = pView->GetHdlNum(pHdl);

        if (pHdl != NULL)
        {
            if (pView->HasMarkablePoints() && pView->IsPointMarkable(*pHdl))
            {
                sal_Bool bPointMarked=pView->IsPointMarked(*pHdl);

                if ( rMEvt.IsShift() )
                {
                    if (!bPointMarked)
                    {
                        pView->MarkPoint(*pHdl);
                    }
                    else
                    {
                        pView->MarkPoint(*pHdl, true); // unmark
                    }
                }
                else
                {
                    if (!bPointMarked)
                    {
                        pView->MarkPoints(0, true); // unmarkall
                        pView->MarkPoint(*pHdl);
                    }
                }
                pHdl=pView->GetHdlByIndex(nHdlNum);
            }
        }

        SdrObject* pObj;

        if ( pHdl != NULL || pView->IsMarkedObjHit(aLogicPos) )
        {
            if (pHdl == NULL &&
//              pView->TakeTextEditObject(aLogicPos, pObj, pPV) )
                pView->PickObj(aLogicPos, pView->getHitTolLog(), pObj, SDRSEARCH_PICKTEXTEDIT) )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                sal_Bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->SdrBeginTextEdit(pObj, pWindow, sal_True, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                    if ( pOLV->MouseButtonDown(rMEvt) )
                        return (sal_True); // Event an den Outliner
                }
            }
            else
            {
                // disable tail & circular move for caption objects.
                bool bDrag = false;
                const SdrObject* pSelected = pView->getSelectedIfSingle();

                if( pSelected )
                {
                    if( ScDrawLayer::IsNoteCaption( *pSelected ) )
                    {
                        if(pHdl->GetKind() != HDL_POLY && pHdl->GetKind() != HDL_CIRC)
                            bDrag = true;
                    }
                    else
                        bDrag = true;   // different object
                }
                else
                    bDrag = true;       // several objects

                if ( bDrag )
                {
                    aDragTimer.Start();
                    pView->BegDragObj(aLogicPos, pHdl);
                }
            }
        }
        else
        {
            sal_Bool bMacro = sal_False;

//          if (bMacro && pView->TakeMacroObject(aLogicPos,pObj,pPV))
            if (bMacro && pView->PickObj(aLogicPos, pView->getHitTolLog(), pObj, SDRSEARCH_PICKMACRO) )

            {
                pView->BegMacroObj(aLogicPos, 2.0, pObj, pWindow);
            }
            else
            {
                if (pView->IsEditMode())
                {
                    const bool bPointMode(pView->HasMarkablePoints());

                    if (!rMEvt.IsShift())
                    {
                        if (bPointMode)
                        {
                            pView->MarkPoints(0, true); // unmarkall
                        }
                        else
                        {
                            pView->UnmarkAll();
                        }

                        pView->SetDragMode(SDRDRAG_MOVE);
                        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_OBJECT_ROTATE );
                        rBindings.Invalidate( SID_OBJECT_MIRROR );
                        pHdl=pView->GetHdlByIndex(nHdlNum);
                    }

                    if ( pView->MarkObj(aLogicPos, 2.0, false, rMEvt.IsMod1()) )
                    {
                        aDragTimer.Start();

                        pHdl=pView->PickHandle(aLogicPos);

                        if (pHdl!=NULL)
                        {
                            pView->MarkPoint(*pHdl);
                            pHdl=pView->GetHdlByIndex(nHdlNum);
                        }

                        pView->BegDragObj(aLogicPos, pHdl);
                    }
                    else
                    {
                        if (bPointMode)
                        {
                            pView->BegMarkPoints(aLogicPos);
                        }
                        else
                        {
                            pView->BegMarkObj(aLogicPos);
                        }
                    }
                }
                else if (aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
                {
                    //  Notizen editieren -> keine neuen Textobjekte erzeugen,
                    //  stattdessen Textmodus verlassen

                    pViewShell->GetViewData()->GetDispatcher().
                        Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                }
                else
                {
                    /**********************************************************
                    * Objekt erzeugen
                    **********************************************************/
                    pView->BegCreateObj(aLogicPos);
                }
            }
        }
    }


    if (!bIsInDragMode)
    {
        pWindow->CaptureMouse();
//      ForcePointer(&rMEvt);
        lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );
    }

    pViewShell->SetActivePointer(pView->GetPreferedPointer(aLogicPos, pWindow ));

//  return (bReturn);
    return sal_True;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuText::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

//  pViewShell->SetActivePointer(aNewPointer);

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aMousePos(pWindow->GetInverseViewTransformation() * aPixelPos);
    pViewShell->SetActivePointer(pView->GetPreferedPointer(aMousePos, pWindow ));

    if (aDragTimer.IsActive() )
    {
        const basegfx::B2DPoint aOldPixel(pWindow->GetViewTransformation() * aMDPos);

        if ( fabs( aOldPixel.getX() - aPixelPos.getX() ) > SC_MAXDRAGMOVE ||
             fabs( aOldPixel.getY() - aPixelPos.getY() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->MouseMove(rMEvt, pWindow) )
        return (sal_True); // Event von der SdrView ausgewertet

    if ( pView->IsAction() )
    {
/*      aNewPointer = Pointer(POINTER_TEXT);
        pViewShell->SetActivePointer(aNewPointer);
*/
        const Point aPix(rMEvt.GetPosPixel());
        const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(aPix.X(), aPix.Y()));

        ForceScroll(aPix);
        pView->MovAction(aLogicPos);
    }

//  ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = sal_False;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aPnt(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return (sal_True); // Event von der SdrView ausgewertet

    if ( pView->IsDragObj() )
    {
        pView->EndDragObj( rMEvt.IsShift() );
        const SdrObject* pSelected = pView->getSelectedIfSingle();

        if (pSelected)
        {
              FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
              FuText* pText = static_cast<FuText*>(pPoor);
            pText->StopDragMode(*pSelected);
        }
    }
    else if ( pView->GetCreateObj() )
    {
        if (rMEvt.IsLeft())
        {
            pView->EndCreateObj(SDRCREATE_FORCEEND);
            if (aSfxRequest.GetSlot() == SID_DRAW_TEXT_MARQUEE)
            {
                //  Lauftext-Objekt erzeugen?
                SdrObject* pSelected = pView->getSelectedIfSingle();

                if (pSelected)
                {
                    // die fuer das Scrollen benoetigten Attribute setzen
                    SfxItemSet aItemSet( pDrDoc->GetItemPool(),
                                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, sal_False ) );
                    aItemSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SfxUInt16Item(SDRATTR_TEXT_ANICOUNT, 1 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                                    (sal_Int16)pWindow->PixelToLogic(Size(2,1)).Width()) );

                    pSelected->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }

            // #93382# init object different when vertical writing
            sal_uInt16 nSlotID(aSfxRequest.GetSlot());
            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotID);
            if(bVertical)
            {
                SdrTextObj* pSelected = dynamic_cast< SdrTextObj* >(pView->getSelectedIfSingle());

                if(pSelected)
                {
                    SfxItemSet aSet(pDrDoc->GetItemPool());

                    pSelected->SetVerticalWriting(sal_True);
                    aSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, sal_True));
                    aSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False));
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                    pSelected->SetMergedItemSet(aSet);
                }
            }

            SetInEditMode();

                //  Modus verlassen bei einzelnem Klick
                //  (-> fuconstr)

            if ( !pView->areSdrObjectsSelected() )
            {
                pView->MarkObj(aPnt, 2.0, false, rMEvt.IsMod1());

                SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
                if ( pView->areSdrObjectsSelected() )
                    rDisp.Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                else
                    rDisp.Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            }
        }
    }
    else if ( pView->IsAction() )
    {
        pView->EndAction();
    }
    else if( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->areSdrObjectsSelected() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, 2.0, false, rMEvt.IsMod1());

            SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
            if ( pView->areSdrObjectsSelected() )
                rDisp.Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

void FuText::ForcePointer(const MouseEvent* /* pMEvt */)
{
    pViewShell->SetActivePointer( aNewPointer );
}



/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool __EXPORT FuText::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    if ( pView->KeyInput(rKEvt, pWindow) )
    {
        bReturn = sal_True;
        lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );
    }
    else
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return (bReturn);
}



/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuText::Activate()
{
    pView->SetDragMode(SDRDRAG_MOVE);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OBJECT_ROTATE );
    rBindings.Invalidate( SID_OBJECT_MIRROR );

    sal_uInt16 nObj = OBJ_TEXT;

    pView->setSdrObjectCreationInfo(SdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_TEXT)));
    pView->SetViewEditMode(SDREDITMODE_CREATE);

    aNewPointer = Pointer(POINTER_TEXT);
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuConstruct::Activate();
}


/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuText::Deactivate()
{
    FuConstruct::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
    StopEditMode();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuText::SelectionHasChanged()
{
    pView->SetDragMode(SDRDRAG_MOVE);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OBJECT_ROTATE );
    rBindings.Invalidate( SID_OBJECT_MIRROR );
    pTextObj = dynamic_cast< SdrTextObj* >(pView->getSelectedIfSingle());

    if(pTextObj)
    {
        const sal_uInt16 nSdrObjKind = pTextObj->GetObjIdentifier();

        if (nSdrObjKind != OBJ_TEXT ||
            nSdrObjKind != OBJ_TITLETEXT ||
            nSdrObjKind != OBJ_OUTLINETEXT)
        {
            pTextObj = 0;
        }
    }

    if (!pTextObj)
    {
        pView->setSdrObjectCreationInfo(SdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_TEXT)));

        pView->SetViewEditMode(SDREDITMODE_CREATE);
    }
}

/*************************************************************************
|*
|* Objekt in Edit-Mode setzen
|*
\************************************************************************/

void FuText::SetInEditMode(SdrObject* pObj, const Point* pMousePixel,
                            sal_Bool bCursorToEnd, const KeyEvent* pInitialKey)
{
    /*  It is possible to pass a special (unselected) object in pObj, e.g. the
        caption object of a cell note. If pObj is 0, then the selected object
        is used. The layer will be relocked in FuText::StopEditMode(). */
    if ( pObj && (pObj->GetLayer() == SC_LAYER_INTERN) )
        pView->UnlockInternalLayer();

    if ( !pObj && pView->areSdrObjectsSelected() )
    {
        pObj = pView->getSelectedIfSingle();
    }

    pTextObj = NULL;

    if ( pObj )
    {
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            dynamic_cast< SdrTextObj* >(pObj))
        {
            const Rectangle aRect(sdr::legacy::GetLogicRect(*pObj));
            Point aPnt = aRect.Center();

            if ( pObj->HasTextEdit() )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                sal_Bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->SdrBeginTextEdit(pObj, pWindow, sal_True, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    pTextObj = (SdrTextObj*) pObj;
                    pView->SetViewEditMode(SDREDITMODE_EDIT);

                    //  set text cursor to click position or to end,
                    //  pass initial key event to outliner view
                    if ( pMousePixel || bCursorToEnd || pInitialKey )
                    {
                        OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                        if (pOLV)
                        {
                            if ( pMousePixel )
                            {
                                MouseEvent aEditEvt( *pMousePixel, 1, MOUSE_SYNTHETIC, MOUSE_LEFT, 0 );
                                pOLV->MouseButtonDown(aEditEvt);
                                pOLV->MouseButtonUp(aEditEvt);
                            }
                            else if ( bCursorToEnd )
                            {
                                ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                                pOLV->SetSelection(aNewSelection);
                            }

                            if ( pInitialKey )
                                pOLV->PostKeyEvent( *pInitialKey );
                        }
                    }
                }
            }
        }
    }
}

// #98185# Create default drawing objects via keyboard
SdrObject* FuText::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    // case SID_DRAW_TEXT:
    // case SID_DRAW_TEXT_VERTICAL:
    // case SID_DRAW_TEXT_MARQUEE:
    // case SID_DRAW_NOTEEDIT:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->getSdrModelFromSdrView(),
        pView->getSdrObjectCreationInfo());

    if(pObj)
    {
        SdrTextObj* pText = dynamic_cast< SdrTextObj* >(pObj);

        if(pText)
        {
            sdr::legacy::SetLogicRange(*pText, rRange);

            //  #105815# don't set default text, start edit mode instead
            // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
            // pText->SetText(aText);

            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nID);
            sal_Bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nID);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool());

                aSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, sal_True));
                aSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                aSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, sal_False ) );
                aSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
                aSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                aSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aSet.Put( SfxUInt16Item(SDRATTR_TEXT_ANICOUNT, 1 ) );
                aSet.Put( SdrTextAniAmountItem( (sal_Int16)pWindow->PixelToLogic(Size(2,1)).Width()) );

                pObj->SetMergedItemSetAndBroadcast(aSet);
            }

            SetInEditMode( pObj );      // #105815# start edit mode
        }
        else
        {
            DBG_ERROR("Object is NO text object");
        }
    }

    return pObj;
}

