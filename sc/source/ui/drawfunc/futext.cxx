/*************************************************************************
 *
 *  $RCSfile: futext.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:29:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <svx/svddef.hxx>
#include <svx/svdoutl.hxx>
#include <svx/outlobj.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdview.hxx>
#include <svx/unolingu.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>

#include "futext.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"

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

FuText::FuText(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
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

BOOL __EXPORT FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return (TRUE);                 // Event von der SdrView ausgewertet

    if ( pView->IsTextEdit() )
    {
        StopEditMode();                    // Danebengeklickt, Ende mit Edit
        pView->SetCreateMode();
    }

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

        ULONG nHdlNum = pView->GetHdlNum(pHdl);

        if (pHdl != NULL)
        {
            if (pView->HasMarkablePoints() && pView->IsPointMarkable(*pHdl))
            {
                BOOL bPointMarked=pView->IsPointMarked(*pHdl);

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
                pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKTEXTEDIT) )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                USHORT nSlotID = aSfxRequest.GetSlot();
                BOOL bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                {
                    if ( nSlotID == SID_DRAW_NOTEEDIT )
                        pOPO->SetVertical( FALSE );         // notes are always horizontal
                    else
                        bVertical = pOPO->IsVertical();     // content wins
                }
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->BegTextEdit(pObj, pPV, pWindow, (FASTBOOL)TRUE, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                    if ( pOLV->MouseButtonDown(rMEvt) )
                        return (TRUE); // Event an den Outliner
                }
            }
            else
            {
                aDragTimer.Start();
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
            }
        }
        else
        {
            BOOL bMacro = FALSE;

//          if (bMacro && pView->TakeMacroObject(aMDPos,pObj,pPV))
            if (bMacro && pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKMACRO) )

            {
                pView->BegMacroObj(aMDPos,pObj,pPV,pWindow);
            }
            else
            {
                if (pView->IsEditMode())
                {
                    BOOL bPointMode=pView->HasMarkablePoints();

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

                    if ( pView->MarkObj(aMDPos, -2, FALSE, rMEvt.IsMod1()) )
                    {
                        aDragTimer.Start();

                        pHdl=pView->HitHandle(aMDPos,*pWindow);

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
                            pView->BegMarkPoints(aMDPos, (OutputDevice*) NULL);
                        }
                        else
                        {
                            pView->BegMarkObj(aMDPos, (OutputDevice*) NULL);
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
    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuText::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

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
        return (TRUE); // Event von der SdrView ausgewertet

    if ( pView->IsAction() )
    {
/*      aNewPointer = Pointer(POINTER_TEXT);
        pViewShell->SetActivePointer(aNewPointer);
*/
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

//  ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return (TRUE); // Event von der SdrView ausgewertet

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
                    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                    // die fuer das Scrollen benoetigten Attribute setzen
                    SfxItemSet aItemSet( pDrDoc->GetItemPool(),
                                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                    aItemSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SdrTextAniCountItem( 1 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                                    (INT16)pWindow->PixelToLogic(Size(2,1)).Width()) );
                    pObj->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }

            // #93382# init object different when vertical writing
            sal_uInt16 nSlotID(aSfxRequest.GetSlot());
            BOOL bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotID);
            if(bVertical)
            {
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                    if(pObj && pObj->ISA(SdrTextObj))
                    {
                        SdrTextObj* pText = (SdrTextObj*)pObj;
                        SfxItemSet aSet(pDrDoc->GetItemPool());

                        pText->SetVerticalWriting(TRUE);

                        aSet.Put(SdrTextAutoGrowWidthItem(TRUE));
                        aSet.Put(SdrTextAutoGrowHeightItem(FALSE));
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
                pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1());

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

/*  aNewPointer = Pointer(POINTER_TEXT);
    pViewShell->SetActivePointer(aNewPointer);
*/
    ForcePointer(&rMEvt);
    pWindow->ReleaseMouse();

    if ( !pView->AreObjectsMarked() )
    {
        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pView->PickObj(aPnt, pObj, pPV) )
        {
            if ( pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1()) )
            {
                UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                if (nSdrObjKind != OBJ_TEXT &&
                    nSdrObjKind != OBJ_TITLETEXT &&
                    nSdrObjKind != OBJ_OUTLINETEXT &&
                    ! pObj->ISA(SdrTextObj) )
                {
//                  pViewShell->GetViewData()->GetDispatcher().Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
            }
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

void FuText::ForcePointer(const MouseEvent* pMEvt)
{
    pViewShell->SetActivePointer( aNewPointer );

/*
    if ( !pView->IsAction() )
    {
        Point aPnt(pWindow->PixelToLogic( pWindow->ScreenToOutputPixel(
                   Pointer::GetPosPixel() ) ) );
        SdrHdl* pHdl=pView->HitHandle(aPnt, *pWindow);

        if (pHdl!=NULL)
        {
            pViewShell->SetActivePointer(pHdl->GetPointer() );
        }
        else
        {
            SdrObject* pObj;
            SdrPageView* pPV;

            if ( pView->IsMarkedHit(aPnt) )
            {
                if ( pView->TakeTextEditObject(aPnt, pObj, pPV) )
                {
                    pViewShell->SetActivePointer(Pointer(POINTER_TEXT));
                }
                else
                {
                    pViewShell->SetActivePointer(Pointer(POINTER_MOVE));
                }
            }
            else
            {
//              if ( pView->TakeMacroObject(aPnt, pObj, pPV) )
                if ( pView->PickObj(aPnt, pObj, pPV, SDRSEARCH_PICKMACRO) )
                {
                    pViewShell->SetActivePointer( pObj->GetMacroPointer() );
                }
                else
                {
                    pViewShell->SetActivePointer( aNewPointer );
                }
            }
        }
    }
*/
}



/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuText::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->KeyInput(rKEvt, pWindow) )
    {
        bReturn = TRUE;
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
        USHORT nObj = OBJ_TEXT;

/*      UINT16 nIdent;
        UINT32 nInvent;
        pView->TakeCurrentObj(nIdent, nInvent);
*/
        pView->SetCurrentObj(nObj);

        pView->SetCreateMode();
    }

    aNewPointer = Pointer(POINTER_TEXT);
//  aNewPointer = Pointer(POINTER_CROSS);               //! ???

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
            SdrObject* pObj = pMark->GetObj();

            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

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
        USHORT nObj = OBJ_TEXT;
        UINT16 nIdent;
        UINT32 nInvent;
        pView->TakeCurrentObj(nIdent, nInvent);

//        if (! pView->IsEditMode() )
//        {
//            if (nIdent == OBJ_TEXT)
//            {
//                nObj = OBJ_TEXT;
//            }
//            else if (nIdent == OBJ_OUTLINETEXT)
//            {
//                nObj = OBJ_OUTLINETEXT;
//            }
//            else if (nIdent == OBJ_TITLETEXT)
//            {
//                nObj = OBJ_TITLETEXT;
//            }
//        }

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
                            BOOL bCursorToEnd, const KeyEvent* pInitialKey)
{
    //  pObj != NULL, wenn ein spezielles (nicht markiertes) Objekt editiert werden soll
    //  (-> Legendenobjekt von Notizen)
    //  wenn pObj == NULL, markiertes Objekt nehmen

    SdrLayer* pLockLayer = NULL;
    if ( pObj && pObj->GetLayer() == SC_LAYER_INTERN )
    {
        //  auf gelocktem Layer kann nicht editiert werden, darum den Layer
        //  temporaer auf nicht gelockt setzen

        pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
        if (pLockLayer)
            pView->SetLayerLocked( pLockLayer->GetName(), FALSE );
    }

    if ( !pObj && pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetObj();
        }
    }

    pTextObj = NULL;

    if ( pObj )
    {
        UINT16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            pObj->ISA(SdrTextObj))
        {
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            Rectangle aRect = pObj->GetLogicRect();
            Point aPnt = aRect.Center();

            if ( pObj->HasTextEdit() )
            {
                SdrOutliner* pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                USHORT nSlotID = aSfxRequest.GetSlot();
                BOOL bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                {
                    if ( nSlotID == SID_DRAW_NOTEEDIT )
                        pOPO->SetVertical( FALSE );         // notes are always horizontal
                    else
                        bVertical = pOPO->IsVertical();     // content wins
                }
                pO->SetVertical( bVertical );

                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->BegTextEdit(pObj, pPV, pWindow, (FASTBOOL)TRUE, pO) )
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

    if (pLockLayer)
        pView->SetLayerLocked( pLockLayer->GetName(), TRUE );
}

// #98185# Create default drawing objects via keyboard
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

            //  #105815# don't set default text, start edit mode instead
            // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
            // pText->SetText(aText);

            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nID);
            sal_Bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nID);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SdrTextObj* pText = (SdrTextObj*)pObj;
                SfxItemSet aSet(pDrDoc->GetItemPool());

                aSet.Put(SdrTextAutoGrowWidthItem(TRUE));
                aSet.Put(SdrTextAutoGrowHeightItem(FALSE));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                aSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                aSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                aSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( (INT16)pWindow->PixelToLogic(Size(2,1)).Width()) );

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

