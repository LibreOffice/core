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

#include <DrawViewShell.hxx>
#include <vcl/weld.hxx>
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

#include <app.hrc>
#include <strings.hrc>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <Window.hxx>
#include <fupoor.hxx>
#include <fusnapln.hxx>
#include <sdmod.hxx>
#include <Ruler.hxx>
#include <sdresid.hxx>
#include <GraphicViewShell.hxx>
#include <sdpage.hxx>
#include <slideshow.hxx>
#include <anminfo.hxx>
#include <sdpopup.hxx>
#include <drawview.hxx>
#include <svx/bmpmask.hxx>
#include <LayerTabBar.hxx>

#include <svx/svditer.hxx>

#include <navigatr.hxx>
#include <memory>

namespace sd {

#define PIPETTE_RANGE 0

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;

void DrawViewShell::DeleteActualPage()
{
    sal_uInt16          nPage = maTabControl->GetCurPagePos();

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
    if(!GetLayerTabControl()) // #i87182#
    {
        OSL_ENSURE(false, "No LayerTabBar (!)");
        return;
    }

    SdrLayerAdmin& rAdmin = GetDoc()->GetLayerAdmin();
    sal_uInt16 nId = GetLayerTabControl()->GetCurPageId();
    const OUString& rName = GetLayerTabControl()->GetLayerName(nId);
    if(LayerTabBar::IsRealNameOfStandardLayer(rName))
    {
        assert(false && "Standard layer may not be deleted.");
        return;
    }
    const OUString& rDisplayName(GetLayerTabControl()->GetPageText(nId));
    OUString aString(SdResId(STR_ASK_DELETE_LAYER));

    // replace placeholder
    aString = aString.replaceFirst("$", rDisplayName);

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   aString));
    if (xQueryBox->run() == RET_YES)
    {
        const SdrLayer* pLayer = rAdmin.GetLayer(rName);
        mpDrawView->DeleteLayer( pLayer->GetName() );

        /* in order to redraw TabBar and Window; should be initiated later on by
           a hint from Joe (as by a change if the layer order). */
        // ( View::Notify() --> ViewShell::ResetActualLayer() )

        mbIsLayerModeActive = false;    // so that ChangeEditMode() does something
        ChangeEditMode(GetEditMode(), true);
    }
}

bool DrawViewShell::KeyInput (const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    bool bRet = false;

    if ( !IsInputLocked() || ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE ) )
    {
        if(KEY_RETURN == rKEvt.GetKeyCode().GetCode()
            && rKEvt.GetKeyCode().IsMod1()
            && GetView()->IsTextEdit())
        {
            // this should be used for cursor travelling.
            SdPage* pActualPage = GetActualPage();
            const SdrMarkList& rMarkList = GetView()->GetMarkedObjectList();
            SdrTextObj* pCandidate = nullptr;

            if(pActualPage && 1 == rMarkList.GetMarkCount())
            {
                SdrMark* pMark = rMarkList.GetMark(0);

                // remember which object was the text in edit mode
                SdrObject* pOldObj = pMark->GetMarkedSdrObj();

                // end text edit now
                GetView()->SdrEndTextEdit();

                // look for a new candidate, a successor of pOldObj
                SdrObjListIter aIter(pActualPage, SdrIterMode::DeepNoGroups);
                bool bDidVisitOldObject(false);

                while(aIter.IsMore() && !pCandidate)
                {
                    SdrObject* pObj = aIter.Next();

                    if(auto pSdrTextObj = dynamic_cast<SdrTextObj *>( pObj ))
                    {
                        SdrInventor nInv(pObj->GetObjInventor());
                        sal_uInt16  nKnd(pObj->GetObjIdentifier());

                        if(SdrInventor::Default == nInv &&
                            (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd)
                            && bDidVisitOldObject)
                        {
                            pCandidate = pSdrTextObj;
                        }

                        if(pObj == pOldObj)
                        {
                            bDidVisitOldObject = true;
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
                    SID_ATTR_CHAR, SfxCallMode::ASYNCHRON);
            }
            else
            {
                // insert a new page with the same page layout
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_INSERTPAGE_QUICK, SfxCallMode::ASYNCHRON);
            }
        }
        else
        {
            bRet = ViewShell::KeyInput(rKEvt, pWin);
            //If object is marked , the corresponding entry is set true , else
            //the corresponding entry is set false .
            if(KEY_TAB == rKEvt.GetKeyCode().GetCode())
            {
               FreshNavigatrTree();
            }
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
        mbIsRulerDrag = true;
    }
    else
    {
        // #i34536# if no guide-lines are visible yet, that show them
        if( ! mpDrawView->IsHlplVisible())
            mpDrawView->SetHlplVisible();

        SdrHelpLineKind eKind;

        if ( rMEvt.IsMod1() )
            eKind = SdrHelpLineKind::Point;
        else if ( rRuler.IsHorizontal() )
            eKind = SdrHelpLineKind::Horizontal;
        else
            eKind = SdrHelpLineKind::Vertical;

        mpDrawView->BegDragHelpLine(aWPos, eKind);
        mbIsRulerDrag = true;
    }
}

//If object is marked , the corresponding entry is set true ,
//else the corresponding entry is set false .
void DrawViewShell::FreshNavigatrEntry()
{
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( SID_NAVIGATOR );
    if( pWindow )
    {
        SdNavigatorWin* pNavWin = static_cast<SdNavigatorWin*>( pWindow->GetContextWindow( SD_MOD() ) );
        if( pNavWin )
            pNavWin->FreshEntry();
    }
}

void DrawViewShell::FreshNavigatrTree()
{
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( SID_NAVIGATOR );
    if( pWindow )
    {
        SdNavigatorWin* pNavWin = static_cast<SdNavigatorWin*>( pWindow->GetContextWindow( SD_MOD() ) );
        if( pNavWin )
            pNavWin->FreshTree( GetDoc() );
    }
}

void DrawViewShell::MouseButtonDown(const MouseEvent& rMEvt,
    ::sd::Window* pWin)
{
    mbMouseButtonDown = true;
    mbMouseSelecting = false;

    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    if ( IsInputLocked() )
        return;

    ViewShell::MouseButtonDown(rMEvt, pWin);

    //If object is marked , the corresponding entry is set true ,
    //else the corresponding entry is set false .
    FreshNavigatrTree();
    if (mbPipette)
    {
        SfxChildWindow* pWnd = GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId());
        SvxBmpMask* pBmpMask = pWnd ? static_cast<SvxBmpMask*>(pWnd->GetWindow()) : nullptr;
        if (pBmpMask)
            pBmpMask->PipetteClicked();
    }
}

void DrawViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( IsMouseButtonDown() )
        mbMouseSelecting = true;

    if ( IsInputLocked() )
        return;

    if ( mpDrawView->IsAction() )
    {
        ::tools::Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

        if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
        {
            bool bInsideOtherWindow = false;

            if (mpContentWindow.get() != nullptr)
            {
                aOutputArea = ::tools::Rectangle(Point(0,0),
                    mpContentWindow->GetOutputSizePixel());

                Point aPos = mpContentWindow->GetPointerPosPixel();
                if ( aOutputArea.IsInside(aPos) )
                    bInsideOtherWindow = true;
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
    if (GetDoc())
    {
        ConfigureAppBackgroundColor();
        mpDrawView->SetApplicationBackgroundColor( mnAppBackgroundColor );
    }

    ViewShell::MouseMove(rMEvt, pWin);

    if( !mbMousePosFreezed )
        maMousePos = rMEvt.GetPosPixel();

    ::tools::Rectangle aRect;

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
        aRect = ::tools::Rectangle(maMousePos, maMousePos);
    }

    ShowMousePosInfo(aRect, pWin);

    SvxBmpMask* pBmpMask = nullptr;
    if (mbPipette && GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
    {
        SfxChildWindow* pWnd = GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId());
        pBmpMask = pWnd ? static_cast<SvxBmpMask*>(pWnd->GetWindow()) : nullptr;
    }

    if (!pBmpMask)
        return;

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

    pBmpMask->SetColor( Color( static_cast<sal_uInt8>( nRed / fDiv + .5 ),
                         static_cast<sal_uInt8>( nGreen / fDiv + .5 ),
                         static_cast<sal_uInt8>( nBlue / fDiv + .5 ) ) );
}

void DrawViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    mbMouseButtonDown = false;

    if ( !IsInputLocked() )
    {
        bool bIsSetPageOrg = mpDrawView->IsSetPageOrg();

        if (mbIsRulerDrag)
        {
            ::tools::Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if (aOutputArea.IsInside(rMEvt.GetPosPixel()))
            {
                mpDrawView->EndAction();

                if (bIsSetPageOrg)
                    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else if (rMEvt.IsLeft() && bIsSetPageOrg)
            {
                mpDrawView->BrkAction();
                SdPage* pPage = static_cast<SdPage*>( mpDrawView->GetSdrPageView()->GetPage() );
                Point aOrg(pPage->GetLeftBorder(), pPage->GetUpperBorder());
                mpDrawView->GetSdrPageView()->SetPageOrigin(aOrg);
                GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else
            {
                mpDrawView->BrkAction();
            }

            GetActiveWindow()->ReleaseMouse();
            mbIsRulerDrag = false;
        }
        else
            ViewShell::MouseButtonUp(rMEvt, pWin);
        //If object is marked , the corresponding entry is set true ,
        //else the corresponding entry is set false .
        FreshNavigatrTree();
    }
    mbMouseSelecting = false;
}

void DrawViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    // The command event is send to the window after a possible context
    // menu from an inplace client is closed. Now we have the chance to
    // deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == CommandEventId::ContextMenu ))
    {
        // Deactivate OLE object
        mpDrawView->UnmarkAll();
        SelectionHasChanged();
        return;
    }

    if ( IsInputLocked() )
        return;

    if( GetView() &&GetView()->getSmartTags().Command(rCEvt) )
        return;

    const bool bNativeShow (SlideShow::IsRunning(GetViewShellBase()));

    if( rCEvt.GetCommand() == CommandEventId::PasteSelection && !bNativeShow )
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSelection( GetActiveWindow() ) );

        if( aDataHelper.GetTransferable().is() )
        {
            Point       aPos;
            sal_Int8    nDnDAction = DND_ACTION_COPY;

            if( GetActiveWindow() )
                aPos = GetActiveWindow()->PixelToLogic( rCEvt.GetMousePosPixel() );

            if( !mpDrawView->InsertData( aDataHelper, aPos, nDnDAction, false ) )
            {
                INetBookmark    aINetBookmark( "", "" );

                if( ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                {
                    InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), "" );
                }
            }
        }
    }
    else if( rCEvt.GetCommand() == CommandEventId::ContextMenu && !bNativeShow &&
             pWin != nullptr && !mpDrawView->IsAction() && !SD_MOD()->GetWaterCan() )
    {
        OUString aPopupId; // Resource name for popup menu

        // is there a snap object under the cursor?
        SdrPageView* pPV;
        Point   aMPos = pWin->PixelToLogic( maMousePos );
        sal_uInt16  nHitLog = static_cast<sal_uInt16>(GetActiveWindow()->PixelToLogic(
            Size(FuPoor::HITPIX, 0 ) ).Width());
        sal_uInt16  nHelpLine;
        // for glue points
        SdrObject*  pObj = nullptr;
        sal_uInt16      nPickId = 0;
        // for field command
        OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
        const SvxFieldItem* pFldItem = nullptr;
        if( pOLV )
            pFldItem = pOLV->GetFieldAtSelection();

        // helper line
        if ( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
        {
            ShowSnapLineContextMenu(*pPV, nHelpLine, rCEvt.GetMousePosPixel());
            return;
        }
        // is glue point under cursor marked?
        else if( mpDrawView->PickGluePoint( aMPos, pObj, nPickId, pPV ) &&
                 mpDrawView->IsGluePointMarked( pObj, nPickId ) )
        {
            aPopupId = "gluepoint";
        }
        // field command?
        else if( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                             nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ||
                             nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                             nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ) )
        {
            LanguageType eLanguage( LANGUAGE_SYSTEM );

            // Format popup with outliner language, if possible
            if( pOLV->GetOutliner() )
            {
                ESelection aSelection( pOLV->GetSelection() );
                eLanguage = pOLV->GetOutliner()->GetLanguage( aSelection.nStartPara, aSelection.nStartPos );
            }

            //fdo#44998 if the outliner has captured the mouse events release the lock
            //so the SdFieldPopup can get them
            pOLV->ReleaseMouse();
            ScopedVclPtrInstance<SdFieldPopup> aFieldPopup( pFldItem->GetField(), eLanguage );

            if ( rCEvt.IsMouseEvent() )
                aMPos = rCEvt.GetMousePosPixel();
            else
                aMPos = Point( 20, 20 );
            aFieldPopup->Execute( pWin, aMPos );

            std::unique_ptr<SvxFieldData> pField(aFieldPopup->GetField());
            if( pField )
            {
                SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );
                // select field, so that it will be deleted on insert
                ESelection aSel = pOLV->GetSelection();
                bool bSel = true;
                if( aSel.nStartPos == aSel.nEndPos )
                {
                    bSel = false;
                    aSel.nEndPos++;
                }
                pOLV->SetSelection( aSel );

                pOLV->InsertField( aFieldItem );

                // reset selection back to original state
                if( !bSel )
                    aSel.nEndPos--;
                pOLV->SetSelection( aSel );
            }
        }
        else
        {
            // is something selected?
            if (mpDrawView->AreObjectsMarked() &&
                mpDrawView->GetMarkedObjectList().GetMarkCount() == 1 )
            {
                pObj = mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                if( HasCurrentFunction(SID_BEZIER_EDIT) && (dynamic_cast< SdrPathObj * >( pObj ) != nullptr ) )
                {
                    aPopupId = "bezier";
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
                                Link<SpellCallbackInfo&,void> aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

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
                                pOutlinerView->GetEditView().Invalidate();
                                UnlockInput();
                            }
                            else
                            {
                                if( (pObj->GetObjInventor() == SdrInventor::Default) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
                                {
                                    aPopupId = "tabletext";
                                }
                                else
                                {
                                    aPopupId = "drawtext";
                                }
                            }
                        }
                    }
                    else
                    {
                        SdrInventor nInv = pObj->GetObjInventor();
                        sal_uInt16  nId  = pObj->GetObjIdentifier();

                        if (nInv == SdrInventor::Default)
                        {
                            switch ( nId )
                            {
                                case OBJ_OUTLINETEXT:
                                case OBJ_CAPTION:
                                case OBJ_TITLETEXT:
                                case OBJ_TEXT:
                                    aPopupId = "textbox";
                                    break;

                                case OBJ_PATHLINE:
                                case OBJ_PLIN:
                                    aPopupId = "curve";
                                    break;

                                case OBJ_FREELINE:
                                case OBJ_EDGE:
                                    aPopupId = "connector";
                                    break;

                                case OBJ_LINE:
                                    aPopupId = "line";
                                    break;

                                case OBJ_MEASURE:
                                    aPopupId = "measure";
                                    break;

                                case OBJ_RECT:
                                case OBJ_CIRC:
                                case OBJ_FREEFILL:
                                case OBJ_PATHFILL:
                                case OBJ_POLY:
                                case OBJ_SECT:
                                case OBJ_CARC:
                                case OBJ_CCUT:
                                case OBJ_CUSTOMSHAPE:
                                    aPopupId = "draw";
                                    break;

                                case OBJ_GRUP:
                                    aPopupId = "group";
                                    break;

                                case OBJ_GRAF:
                                    aPopupId = "graphic";
                                    break;

                                case OBJ_OLE2:
                                    aPopupId = "oleobject";
                                    break;
                                case OBJ_MEDIA:
                                    aPopupId = "media";
                                    break;
                                case OBJ_TABLE:
                                    aPopupId = "table";
                                    break;
                            }
                        }
                        else if( nInv == SdrInventor::E3d )
                        {
                            if( nId == E3D_SCENE_ID )
                            {
                                if( !mpDrawView->IsGroupEntered() )
                                    aPopupId = "3dscene";
                                else
                                    aPopupId = "3dscene2";
                            }
                            else
                                aPopupId = "3dobject";
                        }
                        else if( nInv == SdrInventor::FmForm )
                        {
                            aPopupId = "form";
                        }
                    }
                }
            }

            // multiple selection
            else if (mpDrawView->AreObjectsMarked() &&
                mpDrawView->GetMarkedObjectList().GetMarkCount() > 1 )
            {
                aPopupId = "multiselect";
            }

            // nothing selected
            else
            {
                aPopupId = "page";
            }
        }
        // show Popup-Menu
        if (!aPopupId.isEmpty())
        {
            GetActiveWindow()->ReleaseMouse();

            if(rCEvt.IsMouseEvent())
                GetViewFrame()->GetDispatcher()->ExecutePopup( aPopupId );
            else
            {
                //don't open contextmenu at mouse position if not opened via mouse

                //middle of the window if nothing is marked
                Point aMenuPos(GetActiveWindow()->GetSizePixel().Width()/2
                        ,GetActiveWindow()->GetSizePixel().Height()/2);

                //middle of the bounding rect if something is marked
                if( mpDrawView->AreObjectsMarked() && mpDrawView->GetMarkedObjectList().GetMarkCount() >= 1 )
                {
                    ::tools::Rectangle aMarkRect;
                    mpDrawView->GetMarkedObjectList().TakeBoundRect(nullptr,aMarkRect);
                    aMenuPos = GetActiveWindow()->LogicToPixel( aMarkRect.Center() );

                    //move the point into the visible window area
                    if( aMenuPos.X() < 0 )
                        aMenuPos.setX( 0 );
                    if( aMenuPos.Y() < 0 )
                        aMenuPos.setY( 0 );
                    if( aMenuPos.X() > GetActiveWindow()->GetSizePixel().Width() )
                        aMenuPos.setX( GetActiveWindow()->GetSizePixel().Width() );
                    if( aMenuPos.Y() > GetActiveWindow()->GetSizePixel().Height() )
                        aMenuPos.setY( GetActiveWindow()->GetSizePixel().Height() );
                }

                //open context menu at that point
                GetViewFrame()->GetDispatcher()->ExecutePopup( aPopupId, GetActiveWindow(), &aMenuPos );
            }
            mbMousePosFreezed = false;
        }
    }
    else
    {
        ViewShell::Command(rCEvt, pWin);
    }
}

void DrawViewShell::ShowMousePosInfo(const ::tools::Rectangle& rRect,
    ::sd::Window const * pWin)
{
    if (mbHasRulers && pWin )
    {
        RulerLine   pHLines[2];
        RulerLine   pVLines[2];
        long        nHOffs = 0;
        long        nVOffs = 0;
        sal_uInt16      nCnt;

        if (mpHorizontalRuler.get() != nullptr)
            mpHorizontalRuler->SetLines();

        if (mpVerticalRuler.get() != nullptr)
            mpVerticalRuler->SetLines();

        if (mpHorizontalRuler.get() != nullptr)
        {
            nHOffs = mpHorizontalRuler->GetNullOffset() +
                     mpHorizontalRuler->GetPageOffset();
        }

        if (mpVerticalRuler.get() != nullptr)
        {
            nVOffs = mpVerticalRuler->GetNullOffset() +
                     mpVerticalRuler->GetPageOffset();
        }

        nCnt = 1;
        pHLines[0].nPos = rRect.Left() - nHOffs;
        pVLines[0].nPos = rRect.Top()  - nVOffs;

        if ( rRect.Right() != rRect.Left() || rRect.Bottom() != rRect.Top() )
        {
            pHLines[1].nPos = rRect.Right()  - nHOffs;
            pVLines[1].nPos = rRect.Bottom() - nVOffs;
            nCnt++;
        }

        if (mpHorizontalRuler.get() != nullptr)
            mpHorizontalRuler->SetLines(nCnt, pHLines);
        if (mpVerticalRuler.get() != nullptr)
            mpVerticalRuler->SetLines(nCnt, pVLines);
    }

    // display with coordinates in StatusBar
    OSL_ASSERT (GetViewShell()!=nullptr);
    if ( GetViewShell()->GetUIActiveClient() )
        return;

    SfxItemSet aSet(
        GetPool(),
        svl::Items<
            SID_CONTEXT, SID_CONTEXT,
            SID_ATTR_POSITION, SID_ATTR_SIZE>{});

    GetStatusBarState(aSet);

    aSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.SetState(aSet);
    rBindings.Invalidate(SID_CONTEXT);
    rBindings.Invalidate(SID_ATTR_POSITION);
    rBindings.Invalidate(SID_ATTR_SIZE);
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
    ScopedVclPtrInstance<PopupMenu> pMenu;

    if (rHelpLine.GetKind() == SdrHelpLineKind::Point)
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            SdResId(STR_POPUP_EDIT_SNAPPOINT));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            SdResId(STR_POPUP_DELETE_SNAPPOINT));
    }
    else
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            SdResId(STR_POPUP_EDIT_SNAPLINE));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            SdResId(STR_POPUP_DELETE_SNAPLINE));
    }

    pMenu->RemoveDisabledEntries(false);

    const sal_uInt16 nResult = pMenu->Execute(
        GetActiveWindow(),
        ::tools::Rectangle(rMouseLocation, Size(10,10)),
        PopupMenuFlags::ExecuteDown);
    switch (nResult)
    {
        case SID_SET_SNAPITEM:
        {
            SfxUInt32Item aHelpLineItem (ID_VAL_INDEX, nSnapLineIndex);
            const SfxPoolItem* aArguments[] = {&aHelpLineItem, nullptr};
            GetViewFrame()->GetDispatcher()->Execute(
                SID_SET_SNAPITEM,
                SfxCallMode::SLOT,
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
