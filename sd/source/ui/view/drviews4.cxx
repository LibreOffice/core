/*************************************************************************
 *
 *  $RCSfile: drviews4.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_RULER_HXX
#include <svx/ruler.hxx>
#endif
#ifndef _GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "docshell.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "fupoor.hxx"
#include "app.hxx"
#include "sdruler.hxx"
#include "sdresid.hxx"
#include "drviewsh.hxx"
#include "grviewsh.hxx"
#include "sdpage.hxx"
#include "fuslshow.hxx"
#include "anminfo.hxx"
#include "sdpopup.hxx"
#include "drawview.hxx"

#ifndef _BMPMASK_HXX_ //autogen
#include <svx/bmpmask.hxx>
#endif

#define PIPETTE_RANGE 0

#ifdef WNT
#pragma optimize ( "", off )
#endif


/*************************************************************************
|*
|* aktuelle Seite loeschen
|*
\************************************************************************/

void SdDrawViewShell::DeleteActualPage()
{
    USHORT          nPage = aTabControl.GetCurPageId() - 1;
    SdPage*         pPage = pDoc->GetSdPage(nPage,PK_STANDARD);
    String          aString(SdResId(STR_ASK_DELETE_PAGE));
    String          aPageName(pPage->GetName());

    // dynamische Seitentitel beachten
    if (aPageName.Len() == 0)
    {
        aPageName += String(SdResId(STR_PAGE));
        aPageName += String::CreateFromInt32( (sal_Int32)nPage + 1 );           // an der UI beginnen Seiten bei 1
    }

    // Platzhalter ersetzen
    USHORT nPos = aString.Search(sal_Unicode('$'));
    aString.Erase(nPos, 1);
    aString.Insert(aPageName, nPos);

    if (QueryBox(pWindow, WB_YES_NO, aString).Execute() == RET_YES)
    {
        USHORT nPageCount = pDoc->GetPageCount();
        DBG_ASSERT(nPageCount > 1, "aber das ist die letzte!");

        pDrView->EndTextEdit();

        pDrView->BegUndo();

        pDrView->AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(pPage->GetPageNum());

        pPage = pDoc->GetSdPage(nPage, PK_NOTES);
        pDrView->AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(pPage->GetPageNum());

        pDrView->EndUndo();
    }
}

/*************************************************************************
|*
|* aktuelle Ebene loeschen
|*
\************************************************************************/

void SdDrawViewShell::DeleteActualLayer()
{
    SdrLayerAdmin& rAdmin = pDoc->GetLayerAdmin();
    const String&  rName  = aLayerTab.GetPageText(aLayerTab.GetCurPageId());
    String         aString(SdResId(STR_ASK_DELETE_LAYER));

    // Platzhalter ersetzen
    USHORT nPos = aString.Search(sal_Unicode('$'));
    aString.Erase(nPos, 1);
    aString.Insert(rName, nPos);

    if (QueryBox(pWindow, WB_YES_NO, aString).Execute() == RET_YES)
    {
        const SdrLayer* pLayer = rAdmin.GetLayer(rName, FALSE);
        pDrView->DeleteLayer( pLayer->GetName() );

        // damit TabBar und Window neu gezeichnet werden;
        // sollte spaeter wie beim Aendern der Layerfolge durch einen
        // Hint von Joe angestossen werden
        // ( View::Notify() --> ViewShell::ResetActualLayer() )

        bLayerMode = FALSE;     // damit ChangeEditMode() ueberhaupt was tut
        ChangeEditMode(GetEditMode(), TRUE);
    }
}


/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL SdDrawViewShell::KeyInput(const KeyEvent& rKEvt, SdWindow* pWin)
{
//    // Praesentation auf Zeichentisch ein- oder ausschalten
//    // (nur zu Testzwecken!)
//    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
//    {
//        if (((SdDrawView*) pView)->GetSlideShow())
//        {
//            ((SdDrawView*) pView)->SetAnimationMode(FALSE);
//        }
//        else
//        {
//            ((SdDrawView*) pView)->SetAnimationMode(TRUE);
//        }
//    }

    BOOL bRet = FALSE;

    if ( !IsInputLocked() || ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE ) )
        bRet = SdViewShell::KeyInput(rKEvt, pWin);

    return bRet;
}

/*************************************************************************
|*
|* Vom Lineal ausgehenden Drag (Hilflinien, Ursprung) beginnen
|*
\************************************************************************/

void SdDrawViewShell::StartRulerDrag(const SdRuler& rRuler,
                                     const MouseEvent& rMEvt)
{
    if(!pDrView->IsHlplVisible())
        return;

    pWindow->CaptureMouse();

    Point aWPos = pWindow->PixelToLogic(pWindow->GetPointerPosPixel());

    if ( rRuler.GetExtraRect().IsInside(rMEvt.GetPosPixel()) )
    {
        pDrView->BegSetPageOrg(aWPos);
    }
    else
    {
        SdrHelpLineKind eKind;

        if ( rMEvt.IsMod1() )
            eKind = SDRHELPLINE_POINT;
        else if ( rRuler.IsHorizontal() )
            eKind = SDRHELPLINE_HORIZONTAL;
        else
            eKind = SDRHELPLINE_VERTICAL;

        pDrView->BegDragHelpLine(aWPos, eKind);
    }
    bIsRulerDrag = TRUE;
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void SdDrawViewShell::MouseButtonDown(const MouseEvent& rMEvt, SdWindow* pWin)
{
    if ( !IsInputLocked() )
    {
        SdViewShell::MouseButtonDown(rMEvt, pWin);

        if ( bPipette )
            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->PipetteClicked();
    }
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/


void SdDrawViewShell::MouseMove(const MouseEvent& rMEvt, SdWindow* pWin)
{
    if ( !IsInputLocked() )
    {
        if ( pDrView->IsAction() )
        {
            Rectangle aOutputArea(Point(0,0), pWindow->GetOutputSizePixel());

            if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            {
                BOOL bInsideOtherWindow = FALSE;

                for (USHORT nX = 0; nX < MAX_HSPLIT_CNT; nX++)
                {
                    for (USHORT nY = 0; nY < MAX_VSPLIT_CNT; nY++)
                    {
                        if ( pWinArray[nX][nY] )
                        {
                            aOutputArea = Rectangle(Point(0,0),
                                        pWinArray[nX][nY]->GetOutputSizePixel());

                            Point aPos = pWinArray[nX][nY]->GetPointerPosPixel();
                            if ( aOutputArea.IsInside(aPos) )
                                bInsideOtherWindow = TRUE;
                        }
                    }
                }

                if (! pWindow->HasFocus ())
                {
                    pWindow->ReleaseMouse ();
                    pDrView->BrkAction ();
                    return;
                }
                else if ( bInsideOtherWindow )
                {
                    pWindow->ReleaseMouse();
                    pWin->CaptureMouse ();
                }
            }
            else if ( pWin != pWindow )
                 pWin->CaptureMouse();
        }

        SdViewShell::MouseMove(rMEvt, pWin);

        if( !bMousePosFreezed )
            aMousePos = rMEvt.GetPosPixel();

        Rectangle aRect;

        if ( bIsRulerDrag )
        {
            Point aLogPos = pWindow->PixelToLogic(aMousePos);
            pDrView->MovAction(aLogPos);
        }

        if ( pDrView->IsAction() )
        {
            pDrView->TakeActionRect(aRect);
            aRect = pWindow->LogicToPixel(aRect);
        }
        else
        {
            aRect = Rectangle(aMousePos, aMousePos);
        }

        ShowMousePosInfo(aRect, pWin);

        if ( bPipette && GetViewFrame()->HasChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() ) )
        {
            const long      nStartX = aMousePos.X() - PIPETTE_RANGE;
            const long      nEndX = aMousePos.X() + PIPETTE_RANGE;
            const long      nStartY = aMousePos.Y() - PIPETTE_RANGE;
            const long      nEndY = aMousePos.Y() + PIPETTE_RANGE;
            long            nRed = 0;
            long            nGreen = 0;
            long            nBlue = 0;
            const double    fDiv = ( ( PIPETTE_RANGE << 1 ) + 1 ) * ( ( PIPETTE_RANGE << 1 ) + 1 );

            for ( long nY = nStartY; nY <= nEndY; nY++ )
            {
                for( long nX = nStartX; nX <= nEndX; nX++ )
                {
                    const Color aCol( pWin->GetPixel( pWin->PixelToLogic( Point( nX, nY ) ) ) );

                    nRed += aCol.GetRed();
                    nGreen += aCol.GetGreen();
                    nBlue += aCol.GetBlue();
                }
            }

            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->
                SetColor( Color( (USHORT) ( nRed / fDiv + .5 ),
                                 (USHORT) ( nGreen / fDiv + .5 ),
                                 (USHORT) ( nBlue / fDiv + .5 ) ) );
        }
    }
}


/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void SdDrawViewShell::MouseButtonUp(const MouseEvent& rMEvt, SdWindow* pWin)
{
    if ( !IsInputLocked() )
    {
        FASTBOOL bIsSetPageOrg = pDrView->IsSetPageOrg();

        if (bIsRulerDrag)
        {
            Rectangle aOutputArea(Point(0,0), pWindow->GetOutputSizePixel());

            if (aOutputArea.IsInside(rMEvt.GetPosPixel()))
            {
                pDrView->EndAction();

                if (bIsSetPageOrg)
                    SFX_BINDINGS().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else if (rMEvt.IsLeft() && bIsSetPageOrg)
            {
                pDrView->BrkAction();
                SdPage* pPage = (SdPage*) pDrView->GetPageViewPvNum(0)->GetPage();
                Point aOrg(pPage->GetLftBorder(), pPage->GetUppBorder());
                pDrView->GetPageViewPvNum(0)->SetPageOrigin(aOrg);
                SFX_BINDINGS().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else
            {
                pDrView->BrkAction();
            }

            pWindow->ReleaseMouse();
            bIsRulerDrag = FALSE;
        }
        else
            SdViewShell::MouseButtonUp(rMEvt, pWin);
    }
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void SdDrawViewShell::Command(const CommandEvent& rCEvt, SdWindow* pWin)
{
    if ( !IsInputLocked() )
    {
        const BOOL bNativeShow = pFuSlideShow &&
                                 ( pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW ) &&
                                 !pFuSlideShow->IsLivePresentation();

        if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
             pWin != NULL && !pDrView->IsAction() &&
             !SD_MOD()->GetWaterCan() && !bNativeShow )
        {
            USHORT nSdResId = 0;          // ResourceID fuer Popup-Menue
            BOOL bGraphicShell = this->ISA( SdGraphicViewShell );

            // Ist ein Fangobjekt unter dem Mauszeiger?
            SdrPageView* pPV;
            Point   aMPos = pWin->PixelToLogic( aMousePos );
            USHORT  nHitLog = (USHORT) pWindow->PixelToLogic( Size( HITPIX, 0 ) ).Width();
            USHORT  nHelpLine;
            // fuer Klebepunkt
            SdrObject*  pObj = NULL;
            USHORT      nId = 0;
            // fuer Feldbefehl
            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();
            const SvxFieldItem* pFldItem = NULL;
            if( pOLV )
                pFldItem = pOLV->GetFieldAtSelection();
                //pFldItem = pOLV->GetFieldUnderMousePointer();

            // Hilfslinie
            if ( pDrView->PickHelpLine( aMPos, nHitLog, *pWindow, nHelpLine, pPV) )
            {
                nSdResId = RID_DRAW_SNAPOBJECT_POPUP;
                bMousePosFreezed = TRUE;
            }
            // Klebepunkt unter dem Mauszeiger markiert?
            else if( pDrView->PickGluePoint( aMPos, pObj, nId, pPV ) &&
                     pDrView->IsGluePointMarked( pObj, nId ) )
            {
                nSdResId = RID_DRAW_GLUEPOINT_POPUP;
            }
            // Feldbefehl ?
            else if( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                                 pFldItem->GetField()->ISA( SvxExtTimeField ) ||
                                 pFldItem->GetField()->ISA( SvxExtFileField ) ||
                                 pFldItem->GetField()->ISA( SvxAuthorField ) ) )
            {
                SdFieldPopup aFieldPopup( pFldItem->GetField() );

                if ( rCEvt.IsMouseEvent() )
#ifdef VCL
                    aMPos = rCEvt.GetMousePosPixel();
#else
                    aMPos = pWin->OutputToScreenPixel( rCEvt.GetMousePosPixel() );
#endif
                else
                    aMPos = Point( 20, 20 );
#ifdef VCL
                aFieldPopup.Execute( pWin, aMPos );
#else
                aFieldPopup.Execute( aMPos );
#endif

                SvxFieldData* pField = aFieldPopup.GetField();
                if( pField )
                {
                    SvxFieldItem aFieldItem( *pField );
                    //pOLV->DeleteSelected(); <-- fehlt leider !
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOLV->GetSelection();
                    BOOL bSel = TRUE;
                    if( aSel.nStartPos == aSel.nEndPos )
                    {
                        bSel = FALSE;
                        aSel.nEndPos++;
                    }
                    pOLV->SetSelection( aSel );

                    pOLV->InsertField( aFieldItem );

                    // Selektion wird wieder in den Ursprungszustand gebracht
                    if( !bSel )
                        aSel.nEndPos--;
                    pOLV->SetSelection( aSel );

                    delete pField;
                }
            }
            else
            {
                // ist etwas selektiert?
                if (pDrView->HasMarkedObj() &&
                    pDrView->GetMarkList().GetMarkCount() == 1 )
                {
                    SdrObject* pObj = pDrView->GetMarkList().GetMark(0)->GetObj();

                    if ( pFuActual->GetSlotID() == SID_BEZIER_EDIT &&
                         pObj->ISA(SdrPathObj) )
                    {
                        nSdResId = RID_BEZIER_POPUP;
                    }
                    else
                    {
                        if( pDrView->GetTextEditObject() )
                        {
                            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();
                            Point aPos(rCEvt.GetMousePosPixel());

                            if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
                            {
                                // Popup fuer Online-Spelling
                                Link aLink = LINK(pDoc, SdDrawDocument, OnlineSpellCallback);
                                pOLV->ExecuteSpellPopup(aPos, &aLink);
                            }
                            else
                            {
                                nSdResId = RID_DRAW_TEXTOBJ_INSIDE_POPUP;
                            }
                        }
                        else
                        {
                            UINT32 nInv = pObj->GetObjInventor();
                            UINT16 nId = pObj->GetObjIdentifier();

                            if (nInv == SdrInventor)
                            {
                                switch ( nId )
                                {
                                    case OBJ_CAPTION:
                                    case OBJ_TITLETEXT:
                                    case OBJ_OUTLINETEXT:
                                    case OBJ_TEXT:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_TEXTOBJ_POPUP :
                                                                    RID_DRAW_TEXTOBJ_POPUP;
                                        break;

                                    case OBJ_PATHLINE:
                                    case OBJ_PLIN:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_POLYLINEOBJ_POPUP :
                                                                    RID_DRAW_POLYLINEOBJ_POPUP;
                                        break;

                                    case OBJ_FREELINE:
                                    case OBJ_EDGE:      // Connector
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_EDGEOBJ_POPUP :
                                                                    RID_DRAW_EDGEOBJ_POPUP;
                                        break;

                                    case OBJ_LINE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_LINEOBJ_POPUP :
                                                                    RID_DRAW_LINEOBJ_POPUP;
                                        break;

                                    case OBJ_MEASURE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_MEASUREOBJ_POPUP :
                                                                    RID_DRAW_MEASUREOBJ_POPUP;
                                        break;

                                    case OBJ_RECT:
                                    case OBJ_CIRC:
                                    case OBJ_FREEFILL:
                                    case OBJ_PATHFILL:
                                    case OBJ_POLY:
                                    case OBJ_SECT:
                                    case OBJ_CARC:
                                    case OBJ_CCUT:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GEOMOBJ_POPUP :
                                                                    RID_DRAW_GEOMOBJ_POPUP;
                                        break;

                                    case OBJ_GRUP:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GROUPOBJ_POPUP :
                                                                    RID_DRAW_GROUPOBJ_POPUP;
                                        break;

                                    case OBJ_GRAF:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GRAPHIC_POPUP :
                                                                    RID_DRAW_GRAPHIC_POPUP;
                                        break;

                                    case OBJ_OLE2:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_OLE2_POPUP :
                                                                    RID_DRAW_OLE2_POPUP;
                                        break;
                                }
                            }
                            else if( nInv == E3dInventor /*&& nId == E3D_POLYSCENE_ID*/)
                            {
                                if( nId == E3D_POLYSCENE_ID || nId == E3D_SCENE_ID )
                                {
                                    if( !pDrView->IsGroupEntered() )
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_3DSCENE_POPUP :
                                                                RID_DRAW_3DSCENE_POPUP;
                                    else
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_3DSCENE2_POPUP :
                                                                RID_DRAW_3DSCENE2_POPUP;
                                }
                                else
                                    nSdResId = bGraphicShell ? RID_GRAPHIC_3DOBJ_POPUP :
                                                                RID_DRAW_3DOBJ_POPUP;
                            }
                            else if( nInv == FmFormInventor )
                            {
                                nSdResId = RID_FORM_CONTROL_POPUP;
                            }
                        }
                    }
                }

                // Mehrfachselektion
                else if (pDrView->HasMarkedObj() &&
                    pDrView->GetMarkList().GetMarkCount() > 1 )
                {
                    // SdrObject* pObj = pDrView->GetMarkList().GetMark(0)->GetObj();

                    nSdResId = bGraphicShell ? RID_GRAPHIC_MULTISELECTION_POPUP :
                                                RID_DRAW_MULTISELECTION_POPUP;
                }

                // nichts selektiert
                else
                {
                    nSdResId = bGraphicShell ? RID_GRAPHIC_NOSEL_POPUP :
                                                RID_DRAW_NOSEL_POPUP;
                }
            }
            // Popup-Menue anzeigen
            if (nSdResId)
            {
                pWindow->ReleaseMouse();
                GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId));
                bMousePosFreezed = FALSE;
            }
        }
        else
        {
            SdViewShell::Command(rCEvt, pWin);
        }
    }
}

/*************************************************************************
|*
|* Linealmarkierungen anzeigen
|*
\************************************************************************/

void SdDrawViewShell::ShowMousePosInfo(const Rectangle& rRect, SdWindow* pWin)
{
    if ( bHasRuler && pWin )
    {
        RulerLine   pHLines[2];
        RulerLine   pVLines[2];
        long        nHOffs = 0L;
        long        nVOffs = 0L;
        USHORT      nCnt;
        USHORT      nX, nY,
                    nCol = 0,
                    nRow = 0;

        for (nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        {
            if ( pHRulerArray[nX] )
                pHRulerArray[nX]->SetLines();

            for (nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            {
                if ( pVRulerArray[nY] && nX == 0 )
                    pVRulerArray[nY]->SetLines();

                if ( pWinArray[nX][nY] == pWin )
                {
                    nCol = nX;
                    nRow = nY;
                }
            }
        }

        if (pHRulerArray[nCol])
        {
            nHOffs = pHRulerArray[nCol]->GetNullOffset() +
                     pHRulerArray[nCol]->GetPageOffset();
        }

        if (pVRulerArray[nRow])
        {
            nVOffs = pVRulerArray[nRow]->GetNullOffset() +
                     pVRulerArray[nRow]->GetPageOffset();
        }

        nCnt = 1;
        pHLines[0].nPos = rRect.Left() - nHOffs;
        pVLines[0].nPos = rRect.Top()  - nVOffs;
        pHLines[0].nStyle = 0;
        pVLines[0].nStyle = 0;

        if ( rRect.Right() != rRect.Left() || rRect.Bottom() != rRect.Top() )
        {
            pHLines[1].nPos = rRect.Right()  - nHOffs;
            pVLines[1].nPos = rRect.Bottom() - nVOffs;
            pHLines[1].nStyle = 0;
            pVLines[1].nStyle = 0;
            nCnt++;
        }

        if (pHRulerArray[nCol])
            pHRulerArray[nCol]->SetLines(nCnt, pHLines);
        if (pVRulerArray[nRow])
            pVRulerArray[nRow]->SetLines(nCnt, pVLines);
    }

    // StatusBar Koordinatenanzeige
    if( !GetIPClient() )
    {
        SfxItemSet aSet(GetPool(), SID_CONTEXT, SID_CONTEXT,
                                   SID_ATTR_POSITION, SID_ATTR_POSITION,
                                   SID_ATTR_SIZE, SID_ATTR_SIZE,
                                   0L);

//        GetStatusBarState(aSet);  nicht performant bei gedrueckter Modifiertaste!!

        aSet.Put( SfxStringItem( SID_CONTEXT, pDrView->GetStatusText() ) );
        SFX_BINDINGS().SetState(aSet);
        SFX_BINDINGS().Invalidate(SID_CONTEXT);
        SFX_BINDINGS().Invalidate(SID_ATTR_POSITION);
        SFX_BINDINGS().Invalidate(SID_ATTR_SIZE);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::LockInput()
{
    nLockCount++;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::UnlockInput()
{
    DBG_ASSERT( nLockCount, "Input for this shell is not locked!" )
    if ( nLockCount )
        nLockCount--;
}


#ifdef WNT
#pragma optimize ( "", on )
#endif


