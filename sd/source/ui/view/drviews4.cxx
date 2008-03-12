/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drviews4.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:56:32 $
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
#include "precompiled_sd.hxx"

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
#ifndef _SFX_CLIENTSH_HXX
#include <sfx2/ipclient.hxx>
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
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
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

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif


/*************************************************************************
|*
|* aktuelle Seite loeschen
|*
\************************************************************************/

void DrawViewShell::DeleteActualPage()
{
    USHORT          nPage = maTabControl.GetCurPageId() - 1;
    SdPage*         pPage = GetDoc()->GetSdPage(nPage,PK_STANDARD);

#ifdef DBG_UTIL
    USHORT nPageCount = GetDoc()->GetPageCount();
    DBG_ASSERT(nPageCount > 1, "aber das ist die letzte!");
#endif

    mpDrawView->SdrEndTextEdit();

    mpDrawView->BegUndo();

    mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
    GetDoc()->RemovePage(pPage->GetPageNum());

    pPage = GetDoc()->GetSdPage(nPage, PK_NOTES);
    mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
    GetDoc()->RemovePage(pPage->GetPageNum());

    mpDrawView->EndUndo();
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
        mpDrawView->DeleteLayer( pLayer->GetName() );

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
                SdrObject* pOldObj = pMark->GetMarkedSdrObj();

                // end text edit now
                GetView()->SdrEndTextEdit();

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
                GetView()->MarkObj(pCandidate, GetView()->GetSdrPageView());

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
    GetActiveWindow()->CaptureMouse();

    Point aWPos = GetActiveWindow()->PixelToLogic(GetActiveWindow()->GetPointerPosPixel());

    if ( rRuler.GetExtraRect().IsInside(rMEvt.GetPosPixel()) )
    {
        mpDrawView->BegSetPageOrg(aWPos);
        mbIsRulerDrag = TRUE;
    }
    else
    {
        // #i34536# if no guide-lines are visible yet, that show them
        if( ! mpDrawView->IsHlplVisible())
            mpDrawView->SetHlplVisible( TRUE );

        SdrHelpLineKind eKind;

        if ( rMEvt.IsMod1() )
            eKind = SDRHELPLINE_POINT;
        else if ( rRuler.IsHorizontal() )
            eKind = SDRHELPLINE_HORIZONTAL;
        else
            eKind = SDRHELPLINE_VERTICAL;

        mpDrawView->BegDragHelpLine(aWPos, eKind);
        mbIsRulerDrag = TRUE;
    }
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void DrawViewShell::MouseButtonDown(const MouseEvent& rMEvt,
    ::sd::Window* pWin)
{
    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    // See #126086# and #128122#
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    BOOL bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    if ( !IsInputLocked() )
    {
        ViewShell::MouseButtonDown(rMEvt, pWin);

        if ( mbPipette )
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
        if ( mpDrawView->IsAction() )
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
                    mpDrawView->BrkAction ();
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
        if(mpDrawView!=NULL && GetDoc()!=NULL)
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

            mpDrawView->SetApplicationBackgroundColor(aFillColor);
        }

        ViewShell::MouseMove(rMEvt, pWin);

        if( !mbMousePosFreezed )
            maMousePos = rMEvt.GetPosPixel();

        Rectangle aRect;

        if ( mbIsRulerDrag )
        {
            Point aLogPos = GetActiveWindow()->PixelToLogic(maMousePos);
            mpDrawView->MovAction(aLogPos);
        }

        if ( mpDrawView->IsAction() )
        {
            mpDrawView->TakeActionRect(aRect);
            aRect = GetActiveWindow()->LogicToPixel(aRect);
        }
        else
        {
            aRect = Rectangle(maMousePos, maMousePos);
        }

        ShowMousePosInfo(aRect, pWin);

        if ( mbPipette && GetViewFrame()->HasChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() ) )
        {
            const long      nStartX = maMousePos.X() - PIPETTE_RANGE;
            const long      nEndX = maMousePos.X() + PIPETTE_RANGE;
            const long      nStartY = maMousePos.Y() - PIPETTE_RANGE;
            const long      nEndY = maMousePos.Y() + PIPETTE_RANGE;
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
        FASTBOOL bIsSetPageOrg = mpDrawView->IsSetPageOrg();

        if (mbIsRulerDrag)
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if (aOutputArea.IsInside(rMEvt.GetPosPixel()))
            {
                mpDrawView->EndAction();

                if (bIsSetPageOrg)
                    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else if (rMEvt.IsLeft() && bIsSetPageOrg)
            {
                mpDrawView->BrkAction();
                SdPage* pPage = (SdPage*) mpDrawView->GetSdrPageView()->GetPage();
                Point aOrg(pPage->GetLftBorder(), pPage->GetUppBorder());
                mpDrawView->GetSdrPageView()->SetPageOrigin(aOrg);
                GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else
            {
                mpDrawView->BrkAction();
            }

            GetActiveWindow()->ReleaseMouse();
            mbIsRulerDrag = FALSE;
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
    // The command event is send to the window after a possible context
    // menu from an inplace client is closed. Now we have the chance to
    // deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    // For more information, see #126086# and #128122#
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    BOOL bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        // Deactivate OLE object
        mpDrawView->UnmarkAll();
        SelectionHasChanged();
        return;
    }

    if ( !IsInputLocked() )
    {
        const BOOL bNativeShow = mpSlideShow &&( mpSlideShow->getAnimationMode() == ANIMATIONMODE_SHOW );

        if( rCEvt.GetCommand() == COMMAND_PASTESELECTION && !bNativeShow )
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSelection( GetActiveWindow() ) );

            if( aDataHelper.GetTransferable().is() )
            {
                Point       aPos;
                sal_Int8    nDnDAction = DND_ACTION_COPY;

                if( GetActiveWindow() )
                    aPos = GetActiveWindow()->PixelToLogic( rCEvt.GetMousePosPixel() );

                if( !mpDrawView->InsertData( aDataHelper, aPos, nDnDAction, FALSE ) )
                {
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
                 pWin != NULL && !mpDrawView->IsAction() && !SD_MOD()->GetWaterCan() )
        {
            USHORT nSdResId = 0;          // ResourceID fuer Popup-Menue
            BOOL bGraphicShell = this->ISA(GraphicViewShell);

            // Ist ein Fangobjekt unter dem Mauszeiger?
            SdrPageView* pPV;
            Point   aMPos = pWin->PixelToLogic( maMousePos );
            USHORT  nHitLog = (USHORT) GetActiveWindow()->PixelToLogic(
                Size(FuPoor::HITPIX, 0 ) ).Width();
            USHORT  nHelpLine;
            // fuer Klebepunkt
            SdrObject*  pObj = NULL;
            USHORT      nPickId = 0;
            // fuer Feldbefehl
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
            const SvxFieldItem* pFldItem = NULL;
            if( pOLV )
                pFldItem = pOLV->GetFieldAtSelection();
                //pFldItem = pOLV->GetFieldUnderMousePointer();

            // Hilfslinie
            if ( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
            {
                nSdResId = RID_DRAW_SNAPOBJECT_POPUP;
                mbMousePosFreezed = TRUE;
            }
            // Klebepunkt unter dem Mauszeiger markiert?
            else if( mpDrawView->PickGluePoint( aMPos, pObj, nPickId, pPV ) &&
                     mpDrawView->IsGluePointMarked( pObj, nPickId ) )
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
                    SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );
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
                if (mpDrawView->AreObjectsMarked() &&
                    mpDrawView->GetMarkedObjectList().GetMarkCount() == 1 )
                {
                    pObj = mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                    if( HasCurrentFunction(SID_BEZIER_EDIT) && (dynamic_cast< SdrPathObj * >( pObj ) != 0 ) )
                    {
                        nSdResId = RID_BEZIER_POPUP;
                    }
                    else
                    {
                        if( mpDrawView->GetTextEditObject() )
                        {
                            OutlinerView* pOutlinerView = mpDrawView->GetTextEditOutlinerView();
                            Point aPos(rCEvt.GetMousePosPixel());

                            if ( pOutlinerView )
                            {
                                if( (  rCEvt.IsMouseEvent() && pOutlinerView->IsWrongSpelledWordAtPos(aPos) ) ||
                                    ( !rCEvt.IsMouseEvent() && pOutlinerView->IsCursorAtWrongSpelledWord() ) )
                                {
                                    // #91457# Popup for Online-Spelling now handled by DrawDocShell
                                    // Link aLink = LINK(GetDoc(), SdDrawDocument, OnlineSpellCallback);
                                    Link aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

                                    if( !rCEvt.IsMouseEvent() )
                                    {
                                        aPos = GetActiveWindow()->LogicToPixel( pOutlinerView->GetEditView().GetCursor()->GetPos() );
                                    }
                                    // While showing the spell context menu
                                    // we lock the input so that another
                                    // context menu can not be opened during
                                    // that time (crash #i43235#).  In order
                                    // to not lock the UI completely we
                                    // first release the mouse.
                                    GetActiveWindow()->ReleaseMouse();
                                    LockInput();
                                    pOutlinerView->ExecuteSpellPopup(aPos, &aLink);
                                    UnlockInput();
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
                                    case OBJ_MEDIA:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_MEDIA_POPUP :
                                                                    RID_DRAW_MEDIA_POPUP;
                                        break;
                                    case OBJ_TABLE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_TABLE_POPUP :
                                                                    RID_DRAW_TABLE_POPUP;
                                        break;
                                }
                            }
                            else if( nInv == E3dInventor /*&& nId == E3D_POLYSCENE_ID*/)
                            {
                                if( nId == E3D_POLYSCENE_ID || nId == E3D_SCENE_ID )
                                {
                                    if( !mpDrawView->IsGroupEntered() )
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
                else if (mpDrawView->AreObjectsMarked() &&
                    mpDrawView->GetMarkedObjectList().GetMarkCount() > 1 )
                {
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
                    if( mpDrawView->AreObjectsMarked() && mpDrawView->GetMarkedObjectList().GetMarkCount() >= 1 )
                    {
                        Rectangle aMarkRect;
                        mpDrawView->GetMarkedObjectList().TakeBoundRect(NULL,aMarkRect);
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
                mbMousePosFreezed = FALSE;
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
    if ( !GetViewShell()->GetUIActiveClient() )
    {
        SfxItemSet aSet(GetPool(), SID_CONTEXT, SID_CONTEXT,
                                   SID_ATTR_POSITION, SID_ATTR_POSITION,
                                   SID_ATTR_SIZE, SID_ATTR_SIZE,
                                   0L);

//        GetStatusBarState(aSet);  nicht performant bei gedrueckter Modifiertaste!!

        aSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

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
    mnLockCount++;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::UnlockInput()
{
    DBG_ASSERT( mnLockCount, "Input for this shell is not locked!" )
    if ( mnLockCount )
        mnLockCount--;
}


#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

} // end of namespace sd
