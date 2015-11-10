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
    rBindings.Invalidate( SID_ATTR_CHAR_BACK_COLOR );
    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
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
    rBindings.Invalidate( SID_TABLE_VERT_NONE );
    rBindings.Invalidate( SID_TABLE_VERT_CENTER );
    rBindings.Invalidate( SID_TABLE_VERT_BOTTOM );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );
}

static void lcl_UpdateHyphenator( Outliner& rOutliner, SdrObject* pObj )
{
    // use hyphenator only if hyphenation attribute is set
    if ( pObj && static_cast<const SfxBoolItem&>(pObj->GetMergedItem(EE_PARA_HYPHENATE)).GetValue() ) {
            css::uno::Reference<css::linguistic2::XHyphenator> xHyphenator( LinguMgr::GetHyphenator() );
            rOutliner.SetHyphenator( xHyphenator );
    }
}

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

FuText::FuText(ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq),
    pTextObj(nullptr)
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

bool FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());
    bool bStraightEnter = true;

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return true;                 // Event von der SdrView ausgewertet

    if ( pView->IsTextEdit() )
    {
        if ( IsEditingANote() )
        {
            if( !IsSizingOrMovingNote(rMEvt) )
            {
                StopEditMode();            // Danebengeklickt, Ende mit Edit
                bStraightEnter = false;
            }
        }
        else
        {
            StopEditMode();            // Clicked outside, ending edit.
            pView->UnmarkAll();
            bStraightEnter = false;
        }
        pView->SetCreateMode();
    }

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        const size_t nHdlNum = pView->GetHdlNum(pHdl);

        if (pHdl != nullptr)
        {
            if (pView->HasMarkablePoints() && pView->IsPointMarkable(*pHdl))
            {
                bool bPointMarked=pView->IsPointMarked(*pHdl);

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

        if ( pHdl != nullptr || pView->IsMarkedHit(aMDPos) )
        {
            if (pHdl == nullptr &&
//              pView->TakeTextEditObject(aMDPos, pObj, pPV) )
                pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SdrSearchOptions::PICKTEXTEDIT) )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, true, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                    if ( pOLV->MouseButtonDown(rMEvt) )
                        return true; // Event an den Outliner
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
                    pView->BegDragObj(aMDPos, nullptr, pHdl);
                }
            }
        }
        else
        {
            if (pView->IsEditMode())
            {
                bool bPointMode=pView->HasMarkablePoints();

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
                }

                if ( pView->MarkObj(aMDPos, -2, false, rMEvt.IsMod1()) )
                {
                    aDragTimer.Start();

                    pHdl=pView->PickHandle(aMDPos);

                    if (pHdl!=nullptr)
                    {
                        pView->MarkPoint(*pHdl);
                        pHdl=pView->GetHdl(nHdlNum);
                    }

                    pView->BegDragObj(aMDPos, nullptr, pHdl);
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

                pViewShell->GetViewData().GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
            }
            else
            {
                if (bStraightEnter)//Hack for that silly idea that creating text fields is inside the text routine
                {
                    /**********************************************************
                    * Objekt erzeugen
                    **********************************************************/
                    // Hack  to align object to nearest grid position where object
                    // would be anchored ( if it were cell anchored )
                    // Get grid offset for current position ( note: aPnt is
                    // also adjusted )
                    Point aGridOff = CurrentGridSyncOffsetAndPos( aMDPos );

                    bool bRet = pView->BegCreateObj(aMDPos);
                    if ( bRet )
                    pView->GetCreateObj()->SetGridOffset( aGridOff );
                }
                else if (pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::BEFOREMARK))
                {
                    pView->UnmarkAllObj();
                    ScViewData& rViewData = pViewShell->GetViewData();
                    rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
                    pView->MarkObj(pObj,pPV);

                    pHdl=pView->PickHandle(aMDPos);
                    pView->BegDragObj(aMDPos, nullptr, pHdl);
                    return true;
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

    pViewShell->SetActivePointer(pView->GetPreferredPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));
    if (!bStraightEnter)
    {
            pView->UnmarkAll();
            ScViewData& rViewData = pViewShell->GetViewData();
            rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
    }

    return true;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuText::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = false;

//  pViewShell->SetActivePointer(aNewPointer);

    pViewShell->SetActivePointer(pView->GetPreferredPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( std::abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             std::abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt(pWindow->PixelToLogic(aPix));
    // if object is being created then more than likely the mouse
    // position has been 'adjusted' for the current zoom, need to
    // restore the mouse position here to ensure resize works as expected
    if ( pView->GetCreateObj() )
        aPnt -= pView->GetCreateObj()->GetGridOffset();

    if ( pView->MouseMove(rMEvt, pWindow) )
        return true; // Event von der SdrView ausgewertet

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return true; // Event von der SdrView ausgewertet

    if ( pView->IsDragObj() )
    {
        pView->EndDragObj( rMEvt.IsShift() );
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

                    aItemSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                    aItemSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
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
            bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotID);
            if(bVertical)
            {
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    if(pObj && dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
                    {
                        SdrTextObj* pText = static_cast<SdrTextObj*>(pObj);
                        SfxItemSet aSet(pDrDoc->GetItemPool());

                        pText->SetVerticalWriting(true);

                        aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                        aSet.Put(makeSdrTextAutoGrowHeightItem(false));
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

                SfxDispatcher& rDisp = pViewShell->GetViewData().GetDispatcher();
                if ( pView->AreObjectsMarked() )
                    rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
                else
                    rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
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

            SfxDispatcher& rDisp = pViewShell->GetViewData().GetDispatcher();
            if ( pView->AreObjectsMarked() )
                rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
        }
    }

    return bReturn;
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

bool FuText::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    if ( pView->KeyInput(rKEvt, pWindow) )
    {
        bReturn = true;
        lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );
    }
    else
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return bReturn;
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

    aNewPointer = Pointer(PointerStyle::Text);

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
|* Objekt in Edit-Mode setzen
|*
\************************************************************************/

void FuText::SetInEditMode(SdrObject* pObj, const Point* pMousePixel,
                            bool bCursorToEnd, const KeyEvent* pInitialKey)
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

    pTextObj = nullptr;

    if ( pObj )
    {
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
        {
            SdrPageView* pPV = pView->GetSdrPageView();

            if ( pObj->HasTextEdit() )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, true, pO) )
                {
                    //  Toggle out of paste mode if we are in it, otherwise
                    //  pressing return in this object will instead go to the
                    //  sheet and be considered an overwrite-cell instruction
                    pViewShell->GetViewData().SetPasteMode(SC_PASTE_NONE);
                    pViewShell->UpdateCopySourceOverlay();

                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    pTextObj = static_cast<SdrTextObj*>(pObj);
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
                                MouseEvent aEditEvt( *pMousePixel, 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0 );
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
        nullptr, pDrDoc);

    if(pObj)
    {
        if(dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
        {
            SdrTextObj* pText = static_cast<SdrTextObj*>(pObj);
            pText->SetLogicRect(rRectangle);

            //  don't set default text, start edit mode instead
            // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
            // pText->SetText(aText);

            bool bVertical = (SID_DRAW_TEXT_VERTICAL == nID);
            bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nID);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool());

                aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
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
