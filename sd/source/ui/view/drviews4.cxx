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


#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include "DrawViewShell.hxx"
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/fmglob.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <svx/globl3d.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/editview.hxx>
#include <vcl/cursor.hxx>


#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fusnapln.hxx"
#include "sdmod.hxx"
#include "Ruler.hxx"
#include "sdresid.hxx"
#include "GraphicViewShell.hxx"
#include "sdpage.hxx"
#include "slideshow.hxx"
#include "anminfo.hxx"
#include "sdpopup.hxx"
#include "drawview.hxx"
#include <svx/bmpmask.hxx>
#include "LayerTabBar.hxx"

#include <svx/svditer.hxx>

namespace sd {

#define PIPETTE_RANGE 0

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;


void DrawViewShell::DeleteActualPage()
{
    sal_uInt16          nPage = maTabControl.GetCurPageId() - 1;

    mpDrawView->SdrEndTextEdit();

    try
    {
        Reference<XDrawPagesSupplier> xDrawPagesSupplier( GetDoc()->getUnoModel(), UNO_QUERY_THROW );
        Reference<XDrawPages> xPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        Reference< XDrawPage > xPage( xPages->getByIndex( nPage ), UNO_QUERY_THROW );
        xPages->remove( xPage );
    }
    catch( Exception& )
    {
        OSL_FAIL("SelectionManager::DeleteSelectedMasterPages(), exception caught!");
    }
}


void DrawViewShell::DeleteActualLayer()
{
    SdrLayerAdmin& rAdmin = GetDoc()->GetLayerAdmin();
    const OUString& rName = GetLayerTabControl()->GetPageText(GetLayerTabControl()->GetCurPageId());
    OUString aString(SD_RESSTR(STR_ASK_DELETE_LAYER));

    // replace placeholder
    aString = aString.replaceFirst("$", rName);

    if (QueryBox(GetActiveWindow(), WB_YES_NO, aString).Execute() == RET_YES)
    {
        const SdrLayer* pLayer = rAdmin.GetLayer(rName, sal_False);
        mpDrawView->DeleteLayer( pLayer->GetName() );

        /* in order to redraw TabBar and Window; should be initiated later on by
           a hint from Joe (as by a change if the layer order). */
        // ( View::Notify() --> ViewShell::ResetActualLayer() )

        mbIsLayerModeActive = false;    // so that ChangeEditMode() does something
        ChangeEditMode(GetEditMode(), true);
    }
}



sal_Bool DrawViewShell::KeyInput (const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    sal_Bool bRet = sal_False;

    if ( !IsInputLocked() || ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE ) )
    {
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
                sal_Bool bDidVisitOldObject(sal_False);

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
                            bDidVisitOldObject = sal_True;
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

/**
 * Start with Drag from ruler (helper lines, origin)
 */
void DrawViewShell::StartRulerDrag (
    const Ruler& rRuler,
    const MouseEvent& rMEvt)
{
    GetActiveWindow()->CaptureMouse();

    Point aWPos = GetActiveWindow()->PixelToLogic(GetActiveWindow()->GetPointerPosPixel());

    if ( rRuler.GetExtraRect().IsInside(rMEvt.GetPosPixel()) )
    {
        mpDrawView->BegSetPageOrg(aWPos);
        mbIsRulerDrag = sal_True;
    }
    else
    {
        // #i34536# if no guide-lines are visible yet, that show them
        if( ! mpDrawView->IsHlplVisible())
            mpDrawView->SetHlplVisible( sal_True );

        SdrHelpLineKind eKind;

        if ( rMEvt.IsMod1() )
            eKind = SDRHELPLINE_POINT;
        else if ( rRuler.IsHorizontal() )
            eKind = SDRHELPLINE_HORIZONTAL;
        else
            eKind = SDRHELPLINE_VERTICAL;

        mpDrawView->BegDragHelpLine(aWPos, eKind);
        mbIsRulerDrag = sal_True;
    }
}


void DrawViewShell::MouseButtonDown(const MouseEvent& rMEvt,
    ::sd::Window* pWin)
{
    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    if ( !IsInputLocked() )
    {
        ViewShell::MouseButtonDown(rMEvt, pWin);

        if ( mbPipette )
            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->PipetteClicked();
    }
}



void DrawViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        if ( mpDrawView->IsAction() )
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            {
                sal_Bool bInsideOtherWindow = sal_False;

                if (mpContentWindow.get() != NULL)
                {
                    aOutputArea = Rectangle(Point(0,0),
                        mpContentWindow->GetOutputSizePixel());

                    Point aPos = mpContentWindow->GetPointerPosPixel();
                    if ( aOutputArea.IsInside(aPos) )
                        bInsideOtherWindow = sal_True;
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

        // Since the next MouseMove may execute a IsSolidDraggingNow() in
        // SdrCreateView::MovCreateObj and there the ApplicationBackgroundColor
        // is needed it is necessary to set it here.
        if(mpDrawView!=NULL && GetDoc()!=NULL)
        {
            svtools::ColorConfig aColorConfig;
            Color aFillColor;

            aFillColor = Color( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor );

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
                SetColor( Color( (sal_uInt8) ( nRed / fDiv + .5 ),
                                 (sal_uInt8) ( nGreen / fDiv + .5 ),
                                 (sal_uInt8) ( nBlue / fDiv + .5 ) ) );
        }
    }
}



void DrawViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        bool bIsSetPageOrg = mpDrawView->IsSetPageOrg();

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
            mbIsRulerDrag = sal_False;
        }
        else
            ViewShell::MouseButtonUp(rMEvt, pWin);
    }
}


void DrawViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    // The command event is send to the window after a possible context
    // menu from an inplace client is closed. Now we have the chance to
    // deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        // Deactivate OLE object
        mpDrawView->UnmarkAll();
        SelectionHasChanged();
        return;
    }

    if ( !IsInputLocked() )
    {
        if( GetView() &&GetView()->getSmartTags().Command(rCEvt) )
            return;

        const bool bNativeShow (SlideShow::IsRunning(GetViewShellBase()));

        if( rCEvt.GetCommand() == COMMAND_PASTESELECTION && !bNativeShow )
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSelection( GetActiveWindow() ) );

            if( aDataHelper.GetTransferable().is() )
            {
                Point       aPos;
                sal_Int8    nDnDAction = DND_ACTION_COPY;

                if( GetActiveWindow() )
                    aPos = GetActiveWindow()->PixelToLogic( rCEvt.GetMousePosPixel() );

                if( !mpDrawView->InsertData( aDataHelper, aPos, nDnDAction, sal_False ) )
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
            sal_uInt16 nSdResId = 0;          // ResourceID for popup menu
            sal_Bool bGraphicShell = this->ISA(GraphicViewShell);

            // is there a snap object under the cursor?
            SdrPageView* pPV;
            Point   aMPos = pWin->PixelToLogic( maMousePos );
            sal_uInt16  nHitLog = (sal_uInt16) GetActiveWindow()->PixelToLogic(
                Size(FuPoor::HITPIX, 0 ) ).Width();
            sal_uInt16  nHelpLine;
            // for glue points
            SdrObject*  pObj = NULL;
            sal_uInt16      nPickId = 0;
            // for field command
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
            const SvxFieldItem* pFldItem = NULL;
            if( pOLV )
                pFldItem = pOLV->GetFieldAtSelection();

            // helper line
            if ( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
            {
                nSdResId = RID_DRAW_SNAPOBJECT_POPUP;
                ShowSnapLineContextMenu(*pPV, nHelpLine, rCEvt.GetMousePosPixel());
                return;
            }
            // is glue point under cursor marked?
            else if( mpDrawView->PickGluePoint( aMPos, pObj, nPickId, pPV ) &&
                     mpDrawView->IsGluePointMarked( pObj, nPickId ) )
            {
                nSdResId = RID_DRAW_GLUEPOINT_POPUP;
            }
            // field command?
            else if( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                                 pFldItem->GetField()->ISA( SvxExtTimeField ) ||
                                 pFldItem->GetField()->ISA( SvxExtFileField ) ||
                                 pFldItem->GetField()->ISA( SvxAuthorField ) ) )
            {
                LanguageType eLanguage( LANGUAGE_SYSTEM );

                // Format popup with outliner language, if possible
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
                    // select field, so that it will be deleted on insert
                    ESelection aSel = pOLV->GetSelection();
                    sal_Bool bSel = sal_True;
                    if( aSel.nStartPos == aSel.nEndPos )
                    {
                        bSel = sal_False;
                        aSel.nEndPos++;
                    }
                    pOLV->SetSelection( aSel );

                    pOLV->InsertField( aFieldItem );

                    // reset selection back to original state
                    if( !bSel )
                        aSel.nEndPos--;
                    pOLV->SetSelection( aSel );

                    delete pField;
                }
            }
            else
            {
                // is something selected?
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
                                    // Popup for Online-Spelling now handled by DrawDocShell
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
                                {
                                    if( (pObj->GetObjInventor() == SdrInventor) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
                                    {
                                        nSdResId = RID_DRAW_TABLEOBJ_INSIDE_POPUP;
                                    }
                                    else
                                    {
                                        nSdResId = RID_DRAW_TEXTOBJ_INSIDE_POPUP;
                                    }
                                }
                            }
                        }
                        else
                        {
                            sal_uInt32 nInv = pObj->GetObjInventor();
                            sal_uInt16 nId = pObj->GetObjIdentifier();

                            if (nInv == SdrInventor)
                            {
                                switch ( nId )
                                {
                                    case OBJ_OUTLINETEXT:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_OUTLINETEXTOBJ_POPUP :
                                                                    RID_DRAW_OUTLINETEXTOBJ_POPUP;
                                        break;

                                    case OBJ_CAPTION:
                                    case OBJ_TITLETEXT:
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
                            else if( nInv == E3dInventor )
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

                // multiple selection
                else if (mpDrawView->AreObjectsMarked() &&
                    mpDrawView->GetMarkedObjectList().GetMarkCount() > 1 )
                {
                    nSdResId = bGraphicShell ? RID_GRAPHIC_MULTISELECTION_POPUP :
                                                RID_DRAW_MULTISELECTION_POPUP;
                }

                // nothing selected
                else
                {
                    nSdResId = bGraphicShell ? RID_GRAPHIC_NOSEL_POPUP :
                                                RID_DRAW_NOSEL_POPUP;
                }
            }
            // show Popup-Menu
            if (nSdResId)
            {
                GetActiveWindow()->ReleaseMouse();

                if(rCEvt.IsMouseEvent())
                    GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId));
                else
                {
                    //don't open contextmenu at mouse position if not opened via mouse

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
                mbMousePosFreezed = sal_False;
            }
        }
        else
        {
            ViewShell::Command(rCEvt, pWin);
        }
    }
}


void DrawViewShell::ShowMousePosInfo(const Rectangle& rRect,
    ::sd::Window* pWin)
{
    if (mbHasRulers && pWin )
    {
        RulerLine   pHLines[2];
        RulerLine   pVLines[2];
        long        nHOffs = 0L;
        long        nVOffs = 0L;
        sal_uInt16      nCnt;

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

    // display with coordinates in StatusBar
    OSL_ASSERT (GetViewShell()!=NULL);
    if ( !GetViewShell()->GetUIActiveClient() )
    {
        SfxItemSet aSet(GetPool(), SID_CONTEXT, SID_CONTEXT,
                                   SID_ATTR_POSITION, SID_ATTR_POSITION,
                                   SID_ATTR_SIZE, SID_ATTR_SIZE,
                                   0L);

        GetStatusBarState(aSet);

        aSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.SetState(aSet);
        rBindings.Invalidate(SID_CONTEXT);
        rBindings.Invalidate(SID_ATTR_POSITION);
        rBindings.Invalidate(SID_ATTR_SIZE);
    }
}

void DrawViewShell::LockInput()
{
    mnLockCount++;
}

void DrawViewShell::UnlockInput()
{
    DBG_ASSERT( mnLockCount, "Input for this shell is not locked!" );
    if ( mnLockCount )
        mnLockCount--;
}




void DrawViewShell::ShowSnapLineContextMenu (
    SdrPageView& rPageView,
    const sal_uInt16 nSnapLineIndex,
    const Point& rMouseLocation)
{
    const SdrHelpLine& rHelpLine (rPageView.GetHelpLines()[nSnapLineIndex]);
    ::boost::scoped_ptr<PopupMenu> pMenu (new PopupMenu ());

    if (rHelpLine.GetKind() == SDRHELPLINE_POINT)
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            SD_RESSTR(STR_POPUP_EDIT_SNAPPOINT));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            SD_RESSTR(STR_POPUP_DELETE_SNAPPOINT));
    }
    else
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            SD_RESSTR(STR_POPUP_EDIT_SNAPLINE));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            SD_RESSTR(STR_POPUP_DELETE_SNAPLINE));
    }

    pMenu->RemoveDisabledEntries(sal_False, sal_False);

    const sal_uInt16 nResult = pMenu->Execute(
        GetActiveWindow(),
        Rectangle(rMouseLocation, Size(10,10)),
        POPUPMENU_EXECUTE_DOWN);
    switch (nResult)
    {
        case SID_SET_SNAPITEM:
        {
            SfxUInt32Item aHelpLineItem (ID_VAL_INDEX, nSnapLineIndex);
            const SfxPoolItem* aArguments[] = {&aHelpLineItem, NULL};
            GetViewFrame()->GetDispatcher()->Execute(
                SID_SET_SNAPITEM,
                SFX_CALLMODE_SLOT,
                aArguments);
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            rPageView.DeleteHelpLine(nSnapLineIndex);
        }
        break;

        default:
            break;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
