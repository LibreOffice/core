/*************************************************************************
 *
 *  $RCSfile: drviews4.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-15 08:58:44 $
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

#include "DrawViewShell.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
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
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <vcl/cursor.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#include "app.hxx"
#ifndef SD_RULER_HXX
#include "Ruler.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "anminfo.hxx"
#include "sdpopup.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#ifndef _BMPMASK_HXX_ //autogen
#include <svx/bmpmask.hxx>
#endif
#include "LayerTabBar.hxx"

// #97016# IV
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

namespace sd {

#define PIPETTE_RANGE 0

#ifdef WNT
#pragma optimize ( "", off )
#endif


/*************************************************************************
|*
|* aktuelle Seite loeschen
|*
\************************************************************************/

void DrawViewShell::DeleteActualPage()
{
    USHORT          nPage = aTabControl.GetCurPageId() - 1;
    SdPage*         pPage = GetDoc()->GetSdPage(nPage,PK_STANDARD);
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

    if (QueryBox(GetActiveWindow(), WB_YES_NO, aString).Execute() == RET_YES)
    {
        USHORT nPageCount = GetDoc()->GetPageCount();
        DBG_ASSERT(nPageCount > 1, "aber das ist die letzte!");

        pDrView->EndTextEdit();

        pDrView->BegUndo();

        pDrView->AddUndo(new SdrUndoDelPage(*pPage));
        GetDoc()->RemovePage(pPage->GetPageNum());

        pPage = GetDoc()->GetSdPage(nPage, PK_NOTES);
        pDrView->AddUndo(new SdrUndoDelPage(*pPage));
        GetDoc()->RemovePage(pPage->GetPageNum());

        pDrView->EndUndo();
    }
}

/*************************************************************************
|*
|* aktuelle Ebene loeschen
|*
\************************************************************************/

void DrawViewShell::DeleteActualLayer()
{
    SdrLayerAdmin& rAdmin = GetDoc()->GetLayerAdmin();
    const String&  rName  = GetLayerTabControl()->GetPageText(GetLayerTabControl()->GetCurPageId());
    String         aString(SdResId(STR_ASK_DELETE_LAYER));

    // Platzhalter ersetzen
    USHORT nPos = aString.Search(sal_Unicode('$'));
    aString.Erase(nPos, 1);
    aString.Insert(rName, nPos);

    if (QueryBox(GetActiveWindow(), WB_YES_NO, aString).Execute() == RET_YES)
    {
        const SdrLayer* pLayer = rAdmin.GetLayer(rName, FALSE);
        pDrView->DeleteLayer( pLayer->GetName() );

        // damit TabBar und Window neu gezeichnet werden;
        // sollte spaeter wie beim Aendern der Layerfolge durch einen
        // Hint von Joe angestossen werden
        // ( View::Notify() --> ViewShell::ResetActualLayer() )

        mbIsLayerModeActive = false;    // damit ChangeEditMode() ueberhaupt was tut
        ChangeEditMode(GetEditMode(), true);
    }
}


/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL DrawViewShell::KeyInput (const KeyEvent& rKEvt, ::sd::Window* pWin)
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
    {
        // #97016# IV
        if(KEY_RETURN == rKEvt.GetKeyCode().GetCode()
            && rKEvt.GetKeyCode().IsMod1()
            && GetView()->IsTextEdit())
        {
            // this should be used for cursor travelling.
            SdPage* pActualPage = GetActualPage();
            const SdrMarkList& rMarkList = GetView()->GetMarkedObjectList();
            SdrTextObj* pCandidate = 0L;

            if(pActualPage && 1 == rMarkList.GetMarkCount())
            {
                SdrMark* pMark = rMarkList.GetMark(0);

                // remember which object was the text in edit mode
                SdrObject* pOldObj = pMark->GetObj();

                // end text edit now
                GetView()->EndTextEdit();

                // look for a new candidate, a successor of pOldObj
                SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);
                BOOL bDidVisitOldObject(FALSE);

                while(aIter.IsMore() && !pCandidate)
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj && pObj->ISA(SdrTextObj))
                    {
                        sal_uInt32 nInv(pObj->GetObjInventor());
                        sal_uInt16 nKnd(pObj->GetObjIdentifier());

                        if(SdrInventor == nInv &&
                            (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd)
                            && bDidVisitOldObject)
                        {
                            pCandidate = (SdrTextObj*)pObj;
                        }

                        if(pObj == pOldObj)
                        {
                            bDidVisitOldObject = TRUE;
                        }
                    }
                }
            }

            if(pCandidate)
            {
                // set the new candidate to text edit mode
                GetView()->UnMarkAll();
                GetView()->MarkObj(pCandidate, GetView()->GetPageViewPvNum(0));

                GetViewFrame()->GetDispatcher()->Execute(
                    SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON);
            }
            else
            {
                // insert a new page with the same page layout
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_INSERTPAGE_QUICK, SFX_CALLMODE_ASYNCHRON);
            }
        }
        else
        {
            bRet = ViewShell::KeyInput(rKEvt, pWin);
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* Vom Lineal ausgehenden Drag (Hilflinien, Ursprung) beginnen
|*
\************************************************************************/

void DrawViewShell::StartRulerDrag (
    const Ruler& rRuler,
    const MouseEvent& rMEvt)
{
    if(!pDrView->IsHlplVisible())
        return;

    GetActiveWindow()->CaptureMouse();

    Point aWPos = GetActiveWindow()->PixelToLogic(GetActiveWindow()->GetPointerPosPixel());

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

void DrawViewShell::MouseButtonDown(const MouseEvent& rMEvt,
    ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        ViewShell::MouseButtonDown(rMEvt, pWin);

        if ( bPipette )
            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->PipetteClicked();
    }
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/


void DrawViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        if ( pDrView->IsAction() )
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            {
                BOOL bInsideOtherWindow = FALSE;

                if (mpContentWindow.get() != NULL)
                {
                    aOutputArea = Rectangle(Point(0,0),
                        mpContentWindow->GetOutputSizePixel());

                    Point aPos = mpContentWindow->GetPointerPosPixel();
                    if ( aOutputArea.IsInside(aPos) )
                        bInsideOtherWindow = TRUE;
                }

                if (! GetActiveWindow()->HasFocus ())
                {
                    GetActiveWindow()->ReleaseMouse ();
                    pDrView->BrkAction ();
                    return;
                }
                else if ( bInsideOtherWindow )
                {
                    GetActiveWindow()->ReleaseMouse();
                    pWin->CaptureMouse ();
                }
            }
            else if ( pWin != GetActiveWindow() )
                 pWin->CaptureMouse();
        }

        // #109585#
        // Since the next MouseMove may execute a IsSolidDraggingNow() in
        // SdrCreateView::MovCreateObj and there the ApplicationBackgroundColor
        // is needed it is necessary to set it here.
        if(pDrView!=NULL && GetDoc()!=NULL)
        {
            svtools::ColorConfig aColorConfig;
            Color aFillColor;

            if(DOCUMENT_TYPE_IMPRESS == GetDoc()->GetDocumentType())
            {
                aFillColor = Color( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor );
            }
            else
            {
                aFillColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
            }

            pDrView->SetApplicationBackgroundColor(aFillColor);
        }

        ViewShell::MouseMove(rMEvt, pWin);

        if( !bMousePosFreezed )
            aMousePos = rMEvt.GetPosPixel();

        Rectangle aRect;

        if ( bIsRulerDrag )
        {
            Point aLogPos = GetActiveWindow()->PixelToLogic(aMousePos);
            pDrView->MovAction(aLogPos);
        }

        if ( pDrView->IsAction() )
        {
            pDrView->TakeActionRect(aRect);
            aRect = GetActiveWindow()->LogicToPixel(aRect);
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
                SetColor( Color( (BYTE) ( nRed / fDiv + .5 ),
                                 (BYTE) ( nGreen / fDiv + .5 ),
                                 (BYTE) ( nBlue / fDiv + .5 ) ) );
        }
    }
}


/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void DrawViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        FASTBOOL bIsSetPageOrg = pDrView->IsSetPageOrg();

        if (bIsRulerDrag)
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if (aOutputArea.IsInside(rMEvt.GetPosPixel()))
            {
                pDrView->EndAction();

                if (bIsSetPageOrg)
                    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else if (rMEvt.IsLeft() && bIsSetPageOrg)
            {
                pDrView->BrkAction();
                SdPage* pPage = (SdPage*) pDrView->GetPageViewPvNum(0)->GetPage();
                Point aOrg(pPage->GetLftBorder(), pPage->GetUppBorder());
                pDrView->GetPageViewPvNum(0)->SetPageOrigin(aOrg);
                GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else
            {
                pDrView->BrkAction();
            }

            GetActiveWindow()->ReleaseMouse();
            bIsRulerDrag = FALSE;
        }
        else
            ViewShell::MouseButtonUp(rMEvt, pWin);
    }
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void DrawViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        const BOOL bNativeShow = pFuSlideShow &&
                                 ( pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW ) &&
                                 !pFuSlideShow->IsLivePresentation();

        if( rCEvt.GetCommand() == COMMAND_PASTESELECTION && !bNativeShow )
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSelection( GetActiveWindow() ) );

            if( aDataHelper.GetTransferable().is() )
            {
                Point       aPos;
                sal_Int8    nDnDAction = DND_ACTION_COPY;

                if( GetActiveWindow() )
                    aPos = GetActiveWindow()->PixelToLogic( rCEvt.GetMousePosPixel() );

                if( !pDrView->InsertData( aDataHelper, aPos, nDnDAction, FALSE ) )
                {
                    String          aEmptyStr;
                    INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                    if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, NULL );
                    }
                }
            }
        }
        else if( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && !bNativeShow &&
                 pWin != NULL && !pDrView->IsAction() && !SD_MOD()->GetWaterCan() )
        {
            USHORT nSdResId = 0;          // ResourceID fuer Popup-Menue
            BOOL bGraphicShell = this->ISA(GraphicViewShell);

            // Ist ein Fangobjekt unter dem Mauszeiger?
            SdrPageView* pPV;
            Point   aMPos = pWin->PixelToLogic( aMousePos );
            USHORT  nHitLog = (USHORT) GetActiveWindow()->PixelToLogic(
                Size(FuPoor::HITPIX, 0 ) ).Width();
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
            if ( pDrView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
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
                LanguageType eLanguage( LANGUAGE_SYSTEM );

                // #101743# Format popup with outliner language, if possible
                if( pOLV->GetOutliner() )
                {
                    ESelection aSelection( pOLV->GetSelection() );
                    eLanguage = pOLV->GetOutliner()->GetLanguage( aSelection.nStartPara, aSelection.nStartPos );
                }

                SdFieldPopup aFieldPopup( pFldItem->GetField(), eLanguage );

                if ( rCEvt.IsMouseEvent() )
                    aMPos = rCEvt.GetMousePosPixel();
                else
                    aMPos = Point( 20, 20 );
                aFieldPopup.Execute( pWin, aMPos );

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
                if (pDrView->AreObjectsMarked() &&
                    pDrView->GetMarkedObjectList().GetMarkCount() == 1 )
                {
                    SdrObject* pObj = pDrView->GetMarkedObjectList().GetMark(0)->GetObj();

                    if ( pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT && pObj->ISA(SdrPathObj) )
                    {
                        nSdResId = RID_BEZIER_POPUP;
                    }
                    else
                    {
                        if( pDrView->GetTextEditObject() )
                        {
                            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();
                            Point aPos(rCEvt.GetMousePosPixel());

                            if ( pOLV )
                            {
                                if( (  rCEvt.IsMouseEvent() && pOLV->IsWrongSpelledWordAtPos(aPos) ) ||
                                    ( !rCEvt.IsMouseEvent() && pOLV->IsCursorAtWrongSpelledWord() ) )
                                {
                                    // #91457# Popup for Online-Spelling now handled by DrawDocShell
                                    // Link aLink = LINK(GetDoc(), SdDrawDocument, OnlineSpellCallback);
                                    Link aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

                                    if( !rCEvt.IsMouseEvent() )
                                    {
                                        aPos = GetActiveWindow()->LogicToPixel( pOLV->GetEditView().GetCursor()->GetPos() );
                                    }
                                    pOLV->ExecuteSpellPopup(aPos, &aLink);
                                }
                                else
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

                                    case OBJ_CUSTOMSHAPE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_CUSTOMSHAPE_POPUP :
                                                                    RID_DRAW_CUSTOMSHAPE_POPUP;
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
                else if (pDrView->AreObjectsMarked() &&
                    pDrView->GetMarkedObjectList().GetMarkCount() > 1 )
                {
                    // SdrObject* pObj = pDrView->GetMarkedObjectList().GetMark(0)->GetObj();

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
                GetActiveWindow()->ReleaseMouse();

                if(rCEvt.IsMouseEvent())
                    GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId));
                else
                {
                    //#106326# don't open contextmenu at mouse position if not opened via mouse

                    //middle of the window if nothing is marked
                    Point aMenuPos(GetActiveWindow()->GetSizePixel().Width()/2
                            ,GetActiveWindow()->GetSizePixel().Height()/2);

                    //middle of the bounding rect if something is marked
                    if( pDrView->AreObjectsMarked() && pDrView->GetMarkedObjectList().GetMarkCount() >= 1 )
                    {
                        Rectangle aMarkRect;
                        pDrView->GetMarkedObjectList().TakeBoundRect(NULL,aMarkRect);
                        aMenuPos = GetActiveWindow()->LogicToPixel( aMarkRect.Center() );

                        //move the point into the visible window area
                        if( aMenuPos.X() < 0 )
                            aMenuPos.X() = 0;
                        if( aMenuPos.Y() < 0 )
                            aMenuPos.Y() = 0;
                        if( aMenuPos.X() > GetActiveWindow()->GetSizePixel().Width() )
                            aMenuPos.X() = GetActiveWindow()->GetSizePixel().Width();
                        if( aMenuPos.Y() > GetActiveWindow()->GetSizePixel().Height() )
                            aMenuPos.Y() = GetActiveWindow()->GetSizePixel().Height();
                    }

                    //open context menu at that point
                    GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId),GetActiveWindow(),&aMenuPos);
                }
                bMousePosFreezed = FALSE;
            }
        }
        else
        {
            ViewShell::Command(rCEvt, pWin);
        }
    }
}

/*************************************************************************
|*
|* Linealmarkierungen anzeigen
|*
\************************************************************************/

void DrawViewShell::ShowMousePosInfo(const Rectangle& rRect,
    ::sd::Window* pWin)
{
    if (mbHasRulers && pWin )
    {
        RulerLine   pHLines[2];
        RulerLine   pVLines[2];
        long        nHOffs = 0L;
        long        nVOffs = 0L;
        USHORT      nCnt;

        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->SetLines();

        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->SetLines();

        if (mpHorizontalRuler.get() != NULL)
        {
            nHOffs = mpHorizontalRuler->GetNullOffset() +
                     mpHorizontalRuler->GetPageOffset();
        }

        if (mpVerticalRuler.get() != NULL)
        {
            nVOffs = mpVerticalRuler->GetNullOffset() +
                     mpVerticalRuler->GetPageOffset();
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

        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->SetLines(nCnt, pHLines);
        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->SetLines(nCnt, pVLines);
    }

    // StatusBar Koordinatenanzeige
    OSL_ASSERT (GetViewShell()!=NULL);
    if ( ! GetViewShell()->GetIPClient())
    {
        SfxItemSet aSet(GetPool(), SID_CONTEXT, SID_CONTEXT,
                                   SID_ATTR_POSITION, SID_ATTR_POSITION,
                                   SID_ATTR_SIZE, SID_ATTR_SIZE,
                                   0L);

//        GetStatusBarState(aSet);  nicht performant bei gedrueckter Modifiertaste!!

        aSet.Put( SfxStringItem( SID_CONTEXT, pDrView->GetStatusText() ) );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.SetState(aSet);
        rBindings.Invalidate(SID_CONTEXT);
        rBindings.Invalidate(SID_ATTR_POSITION);
        rBindings.Invalidate(SID_ATTR_SIZE);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::LockInput()
{
    nLockCount++;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::UnlockInput()
{
    DBG_ASSERT( nLockCount, "Input for this shell is not locked!" )
    if ( nLockCount )
        nLockCount--;
}


#ifdef WNT
#pragma optimize ( "", on )
#endif

} // end of namespace sd
