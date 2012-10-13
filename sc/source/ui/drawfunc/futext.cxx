/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svx/svddef.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdotext.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>

#include "futext.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "drawview.hxx"

// Create default drawing objects via keyboard
#include "scresid.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------

static void lcl_InvalidateAttribs( SfxBindings& rBindings )
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

static void lcl_UpdateHyphenator( Outliner& rOutliner, SdrObject* pObj )
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

sal_Bool FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return (sal_True);                 // Event von der SdrView ausgewertet

    if ( pView->IsTextEdit() )
    {
        if( !IsSizingOrMovingNote(rMEvt) )
            StopEditMode();            // Danebengeklickt, Ende mit Edit
        pView->SetCreateMode();
    }

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aMDPos);

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
                        pView->UnmarkPoint(*pHdl);
                    }
                }
                else
                {
                    if (!bPointMarked)
                    {
                        pView->UnmarkAllPoints();
                        pView->MarkPoint(*pHdl);
                    }
                }
                pHdl=pView->GetHdl(nHdlNum);
            }
        }

        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos) )
        {
            if (pHdl == NULL &&
//              pView->TakeTextEditObject(aMDPos, pObj, pPV) )
                pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKTEXTEDIT) )
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
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, sal_True, pO) )
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
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( rMarkList.GetMarkCount() == 1 )
                {
                    SdrObject* pMarkedObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( ScDrawLayer::IsNoteCaption( pMarkedObj ) )
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
                    pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                }
            }
        }
        else
        {
            sal_Bool bMacro = false;

//          if (bMacro && pView->TakeMacroObject(aMDPos,pObj,pPV))
            if (bMacro && pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO) )

            {
                pView->BegMacroObj(aMDPos,pObj,pPV,pWindow);
            }
            else
            {
                if (pView->IsEditMode())
                {
                    sal_Bool bPointMode=pView->HasMarkablePoints();

                    if (!rMEvt.IsShift())
                    {
                        if (bPointMode)
                        {
                            pView->UnmarkAllPoints();
                        }
                        else
                        {
                            pView->UnmarkAll();
                        }

                        pView->SetDragMode(SDRDRAG_MOVE);
                        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_OBJECT_ROTATE );
                        rBindings.Invalidate( SID_OBJECT_MIRROR );
                        pHdl=pView->GetHdl(nHdlNum);
                    }

                    if ( pView->MarkObj(aMDPos, -2, false, rMEvt.IsMod1()) )
                    {
                        aDragTimer.Start();

                        pHdl=pView->PickHandle(aMDPos);

                        if (pHdl!=NULL)
                        {
                            pView->MarkPoint(*pHdl);
                            pHdl=pView->GetHdl(nHdlNum);
                        }

                        pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                    }
                    else
                    {
                        if (bPointMode)
                        {
                            pView->BegMarkPoints(aMDPos);
                        }
                        else
                        {
                            pView->BegMarkObj(aMDPos);
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
                    pView->BegCreateObj(aMDPos, (OutputDevice*) NULL);
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

    pViewShell->SetActivePointer(pView->GetPreferedPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

//  return (bReturn);
    return sal_True;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuText::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = false;

//  pViewShell->SetActivePointer(aNewPointer);

    pViewShell->SetActivePointer(pView->GetPreferedPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( Abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             Abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->MouseMove(rMEvt, pWindow) )
        return (sal_True); // Event von der SdrView ausgewertet

    if ( pView->IsAction() )
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = false;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return (sal_True); // Event von der SdrView ausgewertet

    if ( pView->IsDragObj() )
    {
        pView->EndDragObj( rMEvt.IsShift() );
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
              SdrMark* pMark = rMarkList.GetMark(0);
              SdrObject* pObj = pMark->GetMarkedSdrObj();
              FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
              FuText* pText = static_cast<FuText*>(pPoor);
            pText->StopDragMode(pObj );
        }
        pView->ForceMarkedToAnotherPage();
    }
    else if ( pView->IsCreateObj() )
    {
        if (rMEvt.IsLeft())
        {
            pView->EndCreateObj(SDRCREATE_FORCEEND);
            if (aSfxRequest.GetSlot() == SID_DRAW_TEXT_MARQUEE)
            {
                //  Lauftext-Objekt erzeugen?

                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if (rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                    // die fuer das Scrollen benoetigten Attribute setzen
                    SfxItemSet aItemSet( pDrDoc->GetItemPool(),
                                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( SdrTextAutoGrowWidthItem( false ) );
                    aItemSet.Put( SdrTextAutoGrowHeightItem( false ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SdrTextAniCountItem( 1 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                                    (sal_Int16)pWindow->PixelToLogic(Size(2,1)).Width()) );
                    pObj->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }

            // init object different when vertical writing
            sal_uInt16 nSlotID(aSfxRequest.GetSlot());
            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotID);
            if(bVertical)
            {
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    if(pObj && pObj->ISA(SdrTextObj))
                    {
                        SdrTextObj* pText = (SdrTextObj*)pObj;
                        SfxItemSet aSet(pDrDoc->GetItemPool());

                        pText->SetVerticalWriting(sal_True);

                        aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
                        aSet.Put(SdrTextAutoGrowHeightItem(false));
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                        pText->SetMergedItemSet(aSet);
                    }
                }
            }

            SetInEditMode();

                //  Modus verlassen bei einzelnem Klick
                //  (-> fuconstr)

            if ( !pView->AreObjectsMarked() )
            {
                pView->MarkObj(aPnt, -2, false, rMEvt.IsMod1());

                SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
                if ( pView->AreObjectsMarked() )
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

        if ( !pView->AreObjectsMarked() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, -2, false, rMEvt.IsMod1());

            SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
            if ( pView->AreObjectsMarked() )
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

sal_Bool FuText::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = false;

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

//  Sofort in den Edit Mode setzen
//  SetInEditMode();

//  if (!pTextObj)
    {
        /**********************************************************************
        * Kein Textobjekt im EditMode, daher CreateMode setzen
        **********************************************************************/
        sal_uInt16 nObj = OBJ_TEXT;

        pView->SetCurrentObj(nObj);

        pView->SetCreateMode();
    }

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

    pTextObj = NULL;

    if ( pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nSdrObjKind == OBJ_TEXT ||
                nSdrObjKind == OBJ_TITLETEXT ||
                nSdrObjKind == OBJ_OUTLINETEXT /* ||
                pObj->ISA(SdrTextObj) */ )
            {
                pTextObj = (SdrTextObj*) pObj;
            }
        }
    }

    if (!pTextObj)
    {
        /**********************************************************************
        * Kein Textobjekt im EditMode, daher CreateMode setzen
        **********************************************************************/
        sal_uInt16 nObj = OBJ_TEXT;
        sal_uInt16 nIdent;
        sal_uInt32 nInvent;
        pView->TakeCurrentObj(nIdent, nInvent);

        pView->SetCurrentObj(nObj);

        pView->SetCreateMode();
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

    if ( !pObj && pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetMarkedSdrObj();
        }
    }

    pTextObj = NULL;

    if ( pObj )
    {
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            pObj->ISA(SdrTextObj))
        {
            SdrPageView* pPV = pView->GetSdrPageView();

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
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, sal_True, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    pTextObj = (SdrTextObj*) pObj;
                    pView->SetEditMode();

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

// Create default drawing objects via keyboard
SdrObject* FuText::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_TEXT:
    // case SID_DRAW_TEXT_VERTICAL:
    // case SID_DRAW_TEXT_MARQUEE:
    // case SID_DRAW_NOTEEDIT:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrTextObj))
        {
            SdrTextObj* pText = (SdrTextObj*)pObj;
            pText->SetLogicRect(rRectangle);

            //  don't set default text, start edit mode instead
            // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
            // pText->SetText(aText);

            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nID);
            sal_Bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nID);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool());

                aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
                aSet.Put(SdrTextAutoGrowHeightItem(false));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                aSet.Put( SdrTextAutoGrowWidthItem( false ) );
                aSet.Put( SdrTextAutoGrowHeightItem( false ) );
                aSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                aSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( (sal_Int16)pWindow->PixelToLogic(Size(2,1)).Width()) );

                pObj->SetMergedItemSetAndBroadcast(aSet);
            }

            SetInEditMode( pObj );      // start edit mode
        }
        else
        {
            OSL_FAIL("Object is NO text object");
        }
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
