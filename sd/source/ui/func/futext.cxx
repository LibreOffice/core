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

#include <futext.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editerr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svxerr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <editeng/editstat.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/flditem.hxx>
#include <svl/style.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdotable.hxx>
#include <svx/svxids.hrc>
#include <sfx2/docfile.hxx>
#include <editeng/outlobj.hxx>
#include <svtools/langtab.hxx>

#include <editeng/frmdiritem.hxx>

#include <svx/svdetc.hxx>
#include <editeng/editview.hxx>

#include <sdresid.hxx>
#include <app.hrc>

#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <View.hxx>
#include <Outliner.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdmod.hxx>
#include <FrameView.hxx>
#include <ToolBarManager.hxx>
#include <DrawDocShell.hxx>
#include <strings.hrc>
#include <pres.hxx>
#include <optsitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace sd {

static const sal_uInt16 SidArray[] = {
    SID_STYLE_FAMILY2,                //    5542
    SID_STYLE_FAMILY5,                //    5545
    SID_REDO,                         //    5700
    SID_UNDO,                         //    5701
    SID_CUT,                          //    5710
    SID_COPY,                         //    5711
    SID_ATTR_TABSTOP,                 //   10002
    SID_ATTR_CHAR_FONT,               //   10007
    SID_ATTR_CHAR_POSTURE,            //   10008
    SID_ATTR_CHAR_WEIGHT,             //   10009
    SID_ATTR_CHAR_SHADOWED,           //   10010
    SID_ATTR_CHAR_STRIKEOUT,          //   10013
    SID_ATTR_CHAR_UNDERLINE,          //   10014
    SID_ATTR_CHAR_FONTHEIGHT,         //   10015
    SID_ATTR_CHAR_COLOR,              //   10017
    SID_ATTR_CHAR_KERNING,            //   10018
    SID_ATTR_CHAR_CASEMAP,            //   10019
    SID_ATTR_PARA_ADJUST_LEFT,        //   10028
    SID_ATTR_PARA_ADJUST_RIGHT,       //   10029
    SID_ATTR_PARA_ADJUST_CENTER,      //   10030
    SID_ATTR_PARA_ADJUST_BLOCK,       //   10031
    SID_ATTR_PARA_LINESPACE_10,       //   10034
    SID_ATTR_PARA_LINESPACE_15,       //   10035
    SID_ATTR_PARA_LINESPACE_20,       //   10036
    SID_ATTR_PARA_ULSPACE,            //   10042
    SID_ATTR_PARA_LRSPACE,            //   10043
    SID_ATTR_TRANSFORM_POS_X,         //   10088
    SID_ATTR_TRANSFORM_POS_Y,         //   10089
    SID_ATTR_TRANSFORM_WIDTH,         //   10090
    SID_ATTR_TRANSFORM_HEIGHT,        //   10091
    SID_ATTR_TRANSFORM_ROT_X,         //   10093
    SID_ATTR_TRANSFORM_ROT_Y,         //   10094
    SID_ATTR_TRANSFORM_ANGLE,         //   10095 //Added
    SID_OUTLINE_UP,                   //   10150
    SID_OUTLINE_DOWN,                 //   10151
    SID_OUTLINE_LEFT,                 //   10152
    SID_OUTLINE_RIGHT,                //   10153
    SID_ATTR_TRANSFORM_PROTECT_POS,   //   10236
    SID_ATTR_TRANSFORM_PROTECT_SIZE,  //   10237 //Added
    SID_FORMTEXT_STYLE,               //   10257
    SID_SET_SUPER_SCRIPT,             //   10294
    SID_SET_SUB_SCRIPT,               //   10295
    SID_ATTR_TRANSFORM_AUTOWIDTH,     //   10310
    SID_ATTR_TRANSFORM_AUTOHEIGHT,    //   10311 //Added
    SID_HYPERLINK_GETLINK,            //   10361
    SID_CHARMAP,                      //   10503
    SID_TEXTDIRECTION_LEFT_TO_RIGHT,  //   10907
    SID_TEXTDIRECTION_TOP_TO_BOTTOM,  //   10908
    SID_ATTR_PARA_LEFT_TO_RIGHT,      //   10950
    SID_ATTR_PARA_RIGHT_TO_LEFT,      //   10951
    SID_PARASPACE_INCREASE,           //   11145
    SID_PARASPACE_DECREASE,           //   11146
    FN_NUM_BULLET_ON,                 //   20138
                            0 };


/**
 * base class for text functions
 */
FuText::FuText( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
: FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
, bFirstObjCreated(false)
, bJustEndedEdit(false)
, rRequest (rReq)
{
}

rtl::Reference<FuPoor> FuText::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuText( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

void FuText::disposing()
{
    if(mpView)
    {
        if(mpView->SdrEndTextEdit() == SdrEndTextEditKind::Deleted)
            mxTextObj.reset( nullptr );

        // reset the RequestHandler of the used Outliner to the handler of the document
        ::Outliner* pOutliner = mpView->GetTextEditOutliner();

        if (pOutliner)
            pOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));
    }
}

/*************************************************************************
|*
|* Execute functionality of this class:
|*
|* #71422: Start the functionality of this class in this method
|* and not in the ctor.
|* If you construct an object of this class and you put the
|* address of this object to pFuActual you've got a problem,
|* because some methods inside DoExecute use the pFuActual-Pointer.
|* If the code inside DoExecute is executed inside the ctor,
|* the value of pFuActual is not right. And the value will not
|* be right until the ctor finished !!!
|*
\************************************************************************/
void FuText::DoExecute( SfxRequest& )
{
    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBarShell(
        ToolBarManager::ToolBarGroup::Function,
        ToolbarId::Draw_Text_Toolbox_Sd);

    mpView->SetCurrentObj(OBJ_TEXT);
    mpView->SetEditMode(SdrViewEditMode::Edit);

    MouseEvent aMEvt(mpWindow->GetPointerPosPixel());

    if (nSlotId == SID_TEXTEDIT)
    {
        // Try to select an object
        SdrPageView* pPV = mpView->GetSdrPageView();
        SdrViewEvent aVEvt;
        mpView->PickAnything(aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
        mpView->MarkObj(aVEvt.pRootObj, pPV);

        mxTextObj.reset( dynamic_cast< SdrTextObj* >( aVEvt.pObj ) );
    }
    else if (mpView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            mxTextObj.reset( dynamic_cast< SdrTextObj* >( pObj ) );
        }
    }

    // check for table
    if (mpView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if( pObj && (pObj->GetObjInventor() == SdrInventor::Default ) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
            {
                mpViewShell->GetViewShellBase().GetToolBarManager()->AddToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Draw_Table_Toolbox);
            }
        }
    }

    bool bQuickDrag = true;

    const SfxItemSet* pArgs = rRequest.GetArgs();

    if (pArgs

        // test for type before using
        && SID_TEXTEDIT == nSlotId
        && SfxItemState::SET == pArgs->GetItemState(SID_TEXTEDIT)

        && static_cast<const SfxUInt16Item&>(pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
    {
        // Selection by doubleclick -> don't allow QuickDrag
        bQuickDrag = false;
    }

    SetInEditMode(aMEvt, bQuickDrag);
}

bool FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    bMBDown = true;
    bJustEndedEdit = false;

    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

    if (eHit == SdrHitKind::TextEdit)
    {
        // hit text -> SdrView handles event
        if (mpView->MouseButtonDown(rMEvt, mpWindow))
            return true;
    }

    if (rMEvt.GetClicks() == 1)
    {
        if (mpView->IsTextEdit() && eHit != SdrHitKind::MarkedObject && eHit != SdrHitKind::Handle)
        {
            // finish text input
            if(mpView->SdrEndTextEdit() == SdrEndTextEditKind::Deleted)
            {
                /* Bugfix from MBA: during a double click onto the unused? area
                   in text mode, we get with the second click eHit =
                   SdrHitKind::TextEditObj since it goes to the TextObject which was
                   created with the first click. But this is removed by
                   SdrEndTextEdit since it is empty. But it is still in the mark
                   list. The call MarkObj further below accesses then the dead
                   object. As a simple fix, we determine eHit after
                   SdrEndTextEdit again, this returns then SdrHitKind::NONE. */
                mxTextObj.reset( nullptr );
                eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
            }

            mpView->SetCurrentObj(OBJ_TEXT);
            mpView->SetEditMode(SdrViewEditMode::Edit);
        }

        if (rMEvt.IsLeft() || rMEvt.IsRight())
        {
            mpWindow->CaptureMouse();
            SdrPageView* pPV = mpView->GetSdrPageView();

            if (eHit == SdrHitKind::TextEdit)
            {
                SetInEditMode(rMEvt, false);
            }
            else
            {
                // Don't remark table when clicking in it, mark change triggers a lot of updating
                bool bMarkChanges = true;
                rtl::Reference< sdr::SelectionController > xSelectionController(mpView->getSelectionController());
                if (eHit == SdrHitKind::TextEditObj && xSelectionController.is())
                {
                    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                    if (rMarkList.GetMarkCount() == 1 && rMarkList.GetMark(0)->GetMarkedSdrObj() == aVEvt.pRootObj)
                        bMarkChanges = false;
                }

                if (eHit != SdrHitKind::Handle)
                {
                    // deselect selection
                    if (!rMEvt.IsShift() && eHit == SdrHitKind::TextEditObj)
                    {
                        if(bMarkChanges)
                        {
                            mpView->UnmarkAll();
                            mpView->SetDragMode(SdrDragMode::Move);
                        }
                    }
                }

                if ( aVEvt.eEvent == SdrEventKind::ExecuteUrl                   ||
                     eHit == SdrHitKind::Handle                                 ||
                     eHit == SdrHitKind::MarkedObject                           ||
                     eHit == SdrHitKind::TextEditObj                            ||
                     ( eHit == SdrHitKind::UnmarkedObject && bFirstObjCreated &&
                       !bPermanent ) )
                {
                    // Handle, hit marked or unmarked object
                    if (eHit == SdrHitKind::TextEditObj)
                    {
                        /* hit text of unmarked object:
                           select object and set to EditMode */
                        if (bMarkChanges)
                            mpView->MarkObj(aVEvt.pRootObj, pPV);

                        if (auto pSdrTextObj = dynamic_cast<SdrTextObj *>( aVEvt.pObj ))
                        {
                            mxTextObj.reset( pSdrTextObj );
                        }

                        SetInEditMode(rMEvt, true);
                    }
                    else if (aVEvt.eEvent == SdrEventKind::ExecuteUrl && !rMEvt.IsMod2())
                    {
                        // execute URL
                        mpWindow->ReleaseMouse();
                        SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                        SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                        SfxBoolItem aBrowseItem( SID_BROWSE, true );
                        SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                        mpWindow->ReleaseMouse();

                        if (rMEvt.IsMod1())
                        {
                            // open in new frame
                            pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                { &aStrItem, &aBrowseItem, &aReferer });
                        }
                        else
                        {
                            // open in current frame
                            SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                            pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
                        }
                    }
                    else
                    {
                        // drag object or handle

                        // #i78748#
                        // do the EndTextEdit first, it will delete the handles and force a
                        // recreation. This will make aVEvt.pHdl to point to a deleted handle,
                        // thus it is necessary to reset it and to get it again.

                        // #i112855#
                        // cl: I'm not sure why we checked here also for mxTextObj->GetOutlinerParaObject
                        // this caused SdrEndTextEdit() to be called also when not in text editing and
                        // this does not make sense and caused troubles. (see issue 112855)

                        if( mpView->IsTextEdit() )
                        {
                            mpView->SdrEndTextEdit();
                            bJustEndedEdit = true;

                            if(aVEvt.pHdl)
                            {
                                // force new handle identification, the pointer will be dead here
                                // since SdrEndTextEdit has reset (deleted) the handles.
                                aVEvt.pHdl = nullptr;
                                mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
                            }
                        }

                        if (!aVEvt.pHdl)
                        {
                            if( eHit == SdrHitKind::UnmarkedObject )
                            {
                                if ( !rMEvt.IsShift() )
                                    mpView->UnmarkAll();

                                mpView->MarkObj(aVEvt.pRootObj, pPV);
                            }

                            // Drag object
                            bFirstMouseMove = true;
                            aDragTimer.Start();
                        }

                        if ( ! rMEvt.IsRight())
                        {
                            // we need to pick again since SdrEndTextEdit can rebuild the handles list
                            eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
                            if( (eHit == SdrHitKind::Handle) || (eHit == SdrHitKind::MarkedObject) )
                            {
                                sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                                mpView->BegDragObj(aMDPos, nullptr, aVEvt.pHdl, nDrgLog);
                            }
                        }
                        bReturn = true;
                    }
                }
                else if ( nSlotId != SID_TEXTEDIT &&
                          (bPermanent || !bFirstObjCreated) )
                {
                    // create object
                    mpView->SetCurrentObj(OBJ_TEXT);
                    mpView->SetEditMode(SdrViewEditMode::Create);
                    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                    mpView->BegCreateObj(aMDPos, nullptr, nDrgLog);
                }
                else
                {
                    // select
                    if( !rMEvt.IsShift() )
                        mpView->UnmarkAll();

                    mpView->BegMarkObj( aMDPos );
                }
            }
        }
    }
    else if ( rMEvt.GetClicks() == 2 && !mpView->IsTextEdit() )
    {
        MouseEvent aMEvt( mpWindow->GetPointerPosPixel() );
        SetInEditMode( aMEvt, false );
    }

    if (!bIsInDragMode)
    {
        ForcePointer(&rMEvt);
        mpViewShell->GetViewFrame()->GetBindings().Invalidate(SidArray);
    }

    return bReturn;
}

bool FuText::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = false;
        else
            aDragTimer.Stop();
    }

    if (!bReturn && mpView->IsAction() && !mpDocSh->IsReadOnly())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(mpWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        mpView->MovAction(aPnt);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

void FuText::ImpSetAttributesForNewTextObject(SdrTextObj* pTxtObj)
{
    if(mpDoc->GetDocumentType() == DocumentType::Impress)
    {
        if( nSlotId == SID_ATTR_CHAR )
        {
            /* Create Impress text object (rescales to line height)
               We get the correct height during the subsequent creation of the
               object, otherwise we draw to much */
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(makeSdrTextMinFrameHeightItem(0));
            aSet.Put(makeSdrTextAutoGrowWidthItem(false));
            aSet.Put(makeSdrTextAutoGrowHeightItem(true));
            pTxtObj->SetMergedItemSet(aSet);
            pTxtObj->AdjustTextFrameWidthAndHeight();
            aSet.Put(makeSdrTextMaxFrameHeightItem(pTxtObj->GetLogicRect().GetSize().Height()));
            pTxtObj->SetMergedItemSet(aSet);
        }
        else if( nSlotId == SID_ATTR_CHAR_VERTICAL )
        {
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(makeSdrTextMinFrameWidthItem(0));
            aSet.Put(makeSdrTextAutoGrowWidthItem(true));
            aSet.Put(makeSdrTextAutoGrowHeightItem(false));

            // Needs to be set since default is SDRTEXTHORZADJUST_BLOCK
            aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
            pTxtObj->SetMergedItemSet(aSet);
            pTxtObj->AdjustTextFrameWidthAndHeight();
            aSet.Put(makeSdrTextMaxFrameWidthItem(pTxtObj->GetLogicRect().GetSize().Width()));
            pTxtObj->SetMergedItemSet(aSet);
        }
    }
    else
    {
        if( nSlotId == SID_ATTR_CHAR_VERTICAL )
        {
            // draw text object, needs to be initialized when vertical text is used
            SfxItemSet aSet(mpViewShell->GetPool());

            aSet.Put(makeSdrTextAutoGrowWidthItem(true));
            aSet.Put(makeSdrTextAutoGrowHeightItem(false));

            // Set defaults for vertical click-n'drag text object, pool defaults are:
            // SdrTextVertAdjustItem: SDRTEXTVERTADJUST_TOP
            // SdrTextHorzAdjustItem: SDRTEXTHORZADJUST_BLOCK
            // Analog to that:
            aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK));
            aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

            pTxtObj->SetMergedItemSet(aSet);
        }
    }
}

void FuText::ImpSetAttributesFitToSize(SdrTextObj* pTxtObj)
{
    // FitToSize (fit to frame)
    SfxItemSet aSet(mpViewShell->GetPool(), svl::Items<SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWWIDTH>{});
    aSet.Put(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_PROPORTIONAL));
    aSet.Put(makeSdrTextAutoGrowHeightItem(false));
    aSet.Put(makeSdrTextAutoGrowWidthItem(false));
    pTxtObj->SetMergedItemSet(aSet);
    pTxtObj->AdjustTextFrameWidthAndHeight();
}

void FuText::ImpSetAttributesFitToSizeVertical(SdrTextObj* pTxtObj)
{
    SfxItemSet aSet(mpViewShell->GetPool(),
        svl::Items<SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWWIDTH>{});
    aSet.Put(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_PROPORTIONAL));
    aSet.Put(makeSdrTextAutoGrowHeightItem(false));
    aSet.Put(makeSdrTextAutoGrowWidthItem(false));
    pTxtObj->SetMergedItemSet(aSet);
    pTxtObj->AdjustTextFrameWidthAndHeight();
}

void FuText::ImpSetAttributesFitCommon(SdrTextObj* pTxtObj)
{
    // Normal Textobject
    if (mpDoc->GetDocumentType() != DocumentType::Impress)
        return;

    if( nSlotId == SID_ATTR_CHAR )
    {
        // Impress text object (rescales to line height)
        SfxItemSet aSet(mpViewShell->GetPool());
        aSet.Put(makeSdrTextMinFrameHeightItem(0));
        aSet.Put(makeSdrTextMaxFrameHeightItem(0));
        aSet.Put(makeSdrTextAutoGrowHeightItem(true));
        aSet.Put(makeSdrTextAutoGrowWidthItem(false));
        pTxtObj->SetMergedItemSet(aSet);
    }
    else if( nSlotId == SID_ATTR_CHAR_VERTICAL )
    {
        SfxItemSet aSet(mpViewShell->GetPool());
        aSet.Put(makeSdrTextMinFrameWidthItem(0));
        aSet.Put(makeSdrTextMaxFrameWidthItem(0));
        aSet.Put(makeSdrTextAutoGrowWidthItem(true));
        aSet.Put(makeSdrTextAutoGrowHeightItem(false));
        pTxtObj->SetMergedItemSet(aSet);
    }

    pTxtObj->AdjustTextFrameWidthAndHeight();
}

bool FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;
    if (aDragTimer.IsActive())
    {
        aDragTimer.Stop();
        bIsInDragMode = false;
    }

    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if( (mpView && mpView->MouseButtonUp(rMEvt, mpWindow)) || rMEvt.GetClicks() == 2 )
        return true; // handle event from SdrView

    bool bEmptyTextObj = false;

    if (mxTextObj.is())
    {
        bool bReset = true;

        if (mpView)
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1
                && ( rMarkList.GetMark(0)->GetMarkedSdrObj() == mxTextObj.get()) )
            {
                if( mxTextObj.is() && !GetTextObj()->GetOutlinerParaObject() )
                    bEmptyTextObj = true;
                else
                    bFirstObjCreated = true;
                bReset = false;
            }
        }

        if (bReset)
        {
            mxTextObj.reset( nullptr );
        }
    }

    if( mpView && mpView->IsDragObj())
    {
        // object was moved
        FrameView* pFrameView = mpViewShell->GetFrameView();
        bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(false);
        }

        mpView->SetDragWithCopy(bDragWithCopy);
        mpView->EndDragObj( mpView->IsDragWithCopy() );
        mpView->ForceMarkedToAnotherPage();
        mpView->SetCurrentObj(OBJ_TEXT);

        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if (bJustEndedEdit)
        {
            bJustEndedEdit = false;
            FuPoor::cancel();
        }
        if ((rMEvt.GetClicks() != 2) &&
            !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsRight() &&
            std::abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
            std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            /*************************************************************
            * From text mode, you don't want to rotate immediately.
            **************************************************************/
            SdrPageView* pPV;
            SdrObject* pObj = mpView->PickObj(aMDPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::BEFOREMARK);
            if (pObj && pPV->IsObjMarkable(pObj))
            {
                mpView->UnmarkAllObj();
                mpView->MarkObj(pObj,pPV);
                return bReturn;
            }
        }
    }
    else if( mpView && mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        // object was created
        mxTextObj.reset( dynamic_cast< SdrTextObj* >( mpView->GetCreateObj() ) );

        if( mxTextObj.is() )
        {
            //AW outliner needs to be set to vertical when there is no
            // outliner object up to now; also it needs to be set back to not
            // vertical when there was a vertical one used last time.
            OutlinerParaObject* pOPO = GetTextObj()->GetOutlinerParaObject();
            SdrOutliner& rOutl(mxTextObj->getSdrModelFromSdrObject().GetDrawOutliner(GetTextObj()));
            bool bVertical((pOPO && pOPO->IsVertical())
                || nSlotId == SID_ATTR_CHAR_VERTICAL
                || nSlotId == SID_TEXT_FITTOSIZE_VERTICAL);
            rOutl.SetVertical(bVertical);

            // Before ImpSetAttributesForNewTextObject the vertical writing mode
            // needs to be set at the object. This is done here at the OutlinerParaObject
            // directly to not mirror the layout text items involved. These items will be set
            // from ImpSetAttributesForNewTextObject and below.
            OutlinerParaObject* pPara = GetTextObj()->GetOutlinerParaObject();

            if(!pPara)
            {
                GetTextObj()->ForceOutlinerParaObject();
                pPara = GetTextObj()->GetOutlinerParaObject();
            }

            if(pPara && bVertical != pPara->IsVertical())
            {
                // set ParaObject orientation accordingly
                pPara->SetVertical(bVertical);
            }

            ImpSetAttributesForNewTextObject(GetTextObj());
        }

        if (!mpView->EndCreateObj(SdrCreateCmd::ForceEnd))
        {
            // it was not possible to create text object
            mxTextObj.reset(nullptr);
        }
        else if (nSlotId == SID_TEXT_FITTOSIZE)
        {
            ImpSetAttributesFitToSize(GetTextObj());

            SetInEditMode(rMEvt, false);
        }
        else if ( nSlotId == SID_TEXT_FITTOSIZE_VERTICAL )
        {
            ImpSetAttributesFitToSizeVertical(GetTextObj());

            SetInEditMode(rMEvt, false);
        }
        else
        {
            ImpSetAttributesFitCommon(GetTextObj());

            // thereby the handles and the gray frame are correct
            mpView->AdjustMarkHdl();
            mpView->PickHandle(aPnt);
            SetInEditMode(rMEvt, false);
        }
    }
    else if ( mpView && mpView->IsAction())
    {
        mpView->EndAction();
    }

    ForcePointer(&rMEvt);
    mpWindow->ReleaseMouse();
    sal_uInt16 nDrgLog1 = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

    if ( mpView && !mpView->AreObjectsMarked() &&
         std::abs(aMDPos.X() - aPnt.X()) < nDrgLog1 &&
         std::abs(aMDPos.Y() - aPnt.Y()) < nDrgLog1 &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV2 = mpView->GetSdrPageView();
        SdrViewEvent aVEvt;
        mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
        mpView->MarkObj(aVEvt.pRootObj, pPV2);
    }

    if ( !mxTextObj.is() && mpView )
    {
        if ( ( (!bEmptyTextObj   &&  bPermanent) ||
             (!bFirstObjCreated && !bPermanent) ) &&
              !mpDocSh->IsReadOnly()               &&
              nSlotId != SID_TEXTEDIT )
        {
            // text body (left-justified AutoGrow)
            mpView->SetCurrentObj(OBJ_TEXT);
            mpView->SetEditMode(SdrViewEditMode::Create);
            sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            mpView->BegCreateObj(aMDPos, nullptr, nDrgLog);

            bool bSnapEnabled = mpView->IsSnapEnabled();

            if (bSnapEnabled)
                mpView->SetSnapEnabled(false);

            aPnt.AdjustX(nDrgLog + nDrgLog );
            aPnt.AdjustY(nDrgLog + nDrgLog );
            mpView->MovAction(aPnt);

            mxTextObj.reset( dynamic_cast< SdrTextObj* >( mpView->GetCreateObj() ) );

            if(mxTextObj.is())
            {
                GetTextObj()->SetDisableAutoWidthOnDragging(true);
            }

            if(!mpView->EndCreateObj(SdrCreateCmd::ForceEnd))
            {
                mxTextObj.reset(nullptr);
            }

            if(bSnapEnabled)
                mpView->SetSnapEnabled(bSnapEnabled);

            if(mxTextObj.is())
            {
                SfxItemSet aSet(mpViewShell->GetPool());
                aSet.Put(makeSdrTextMinFrameHeightItem(0));
                aSet.Put(makeSdrTextMinFrameWidthItem(0));
                aSet.Put(makeSdrTextAutoGrowHeightItem(true));
                aSet.Put(makeSdrTextAutoGrowWidthItem(true));

                if(nSlotId == SID_ATTR_CHAR_VERTICAL)
                {
                    // Here, all items which need to be different from pool default need to be set
                    // again on the newly created text object.
                    // Since this is a simple click text object, it is first created, then SetVertical()
                    // is used, then ImpSetAttributesForNewTextObject is called and then the object is
                    // deleted again since not the minimum drag distance was travelled. Then, a new
                    // click text object is created and thus all that stuff needs to be set again here.

                    // Before using the new object the vertical writing mode
                    // needs to be set. This is done here at the OutlinerParaObject
                    // directly to not mirror the layout text items involved. These items will be set
                    // below.
                    OutlinerParaObject* pPara = GetTextObj()->GetOutlinerParaObject();

                    if(!pPara)
                    {
                        GetTextObj()->ForceOutlinerParaObject();
                        pPara = GetTextObj()->GetOutlinerParaObject();
                    }

                    if(pPara && !pPara->IsVertical())
                    {
                        // set ParaObject orientation accordingly
                        pPara->SetVertical(true);
                    }

                    aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                    // Analog to the else case below, for vertical simple click texts
                    // one of the default set items from ImpSetAttributesForNewTextObject
                    // needs to be adapted to non-block mode.
                    const SfxItemSet& rSet = mpView->GetDefaultAttr();
                    SvxFrameDirection eDirection = rSet.Get(EE_PARA_WRITINGDIR).GetValue();

                    if(SvxFrameDirection::Horizontal_RL_TB == eDirection || SvxFrameDirection::Vertical_RL_TB == eDirection)
                    {
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM));
                    }
                    else
                    {
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                    }
                }
                else
                {
                    // This is for Format/Page settings. Since this also leads
                    // to the object defaults to be changed, i think this code can be
                    // removed. CL. wanted to take a look before adding this.

                    // Look in the object defaults if left-to-right is wanted. If
                    // yes, set text anchoring to right to let the box grow to left.
                    const SfxItemSet& rSet = mpView->GetDefaultAttr();
                    SvxFrameDirection eDirection = rSet.Get(EE_PARA_WRITINGDIR).GetValue();

                    if(SvxFrameDirection::Horizontal_RL_TB == eDirection)
                    {
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                    }
                    else
                    {
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
                    }
                }

                GetTextObj()->SetMergedItemSet(aSet);
                GetTextObj()->SetDisableAutoWidthOnDragging(true);
                SetInEditMode(rMEvt, false);
            }

            bFirstObjCreated = true;
        }
        else
        {
            // switch to selection
            if (mpView->SdrEndTextEdit() == SdrEndTextEditKind::Deleted)
            {
                mxTextObj.reset(nullptr);
            }

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                      SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
        }
    }
    if (bJustEndedEdit)
    {
        bJustEndedEdit = false;
        FuPoor::cancel();
    }
    bMBDown = false;
    FuConstruct::MouseButtonUp(rMEvt);
    return bReturn;
}

/**
 * handle keyboard events
 * @returns sal_True if the event was handled, sal_False otherwise
 */
bool FuText::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    vcl::KeyCode nCode = rKEvt.GetKeyCode();
    bool bShift = nCode.IsShift();

    if(mxTextObj.is())
    {
        // maybe object is deleted, test if it's equal to the selected object
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        SdrObject* pSelectedObj = nullptr;

        if(1 == rMarkList.GetMarkCount())
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pSelectedObj = pMark->GetMarkedSdrObj();
        }

        if(mxTextObj.get() != pSelectedObj)
        {
            mxTextObj.reset(nullptr);
        }
    }

    if ( mxTextObj.is() && mxTextObj->GetObjInventor() == SdrInventor::Default && mxTextObj->GetObjIdentifier() == OBJ_TITLETEXT && rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // title text object: always soft breaks
        bShift = true;
    }

    sal_uInt16 nKey = nCode.GetCode();
    vcl::KeyCode aKeyCode (nKey, bShift, nCode.IsMod1(), nCode.IsMod2(), nCode.IsMod3() );
    KeyEvent aKEvt(rKEvt.GetCharCode(), aKeyCode);

    bool bOK = true;

    if (mpDocSh->IsReadOnly())
    {
        bOK = !EditEngine::DoesKeyChangeText(aKEvt);
    }
    if( aKeyCode.GetCode() == KEY_PAGEUP || aKeyCode.GetCode() == KEY_PAGEDOWN )
    {
        bOK = false;   // default handling in base class
    }

    if (bOK && mpView->KeyInput(aKEvt, mpWindow) )
    {
        bReturn = true;

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    }
    else if (aKeyCode == KEY_ESCAPE)
    {
        bReturn = cancel();
    }

    if( bPermanent )
    {
        mpView->SetCurrentObj(OBJ_TEXT);
        mpView->SetEditMode(SdrViewEditMode::Create);
    }

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(aKEvt);
    }

    return bReturn;
}

void FuText::Activate()
{
    mpView->SetQuickTextEditMode(mpViewShell->GetFrameView()->IsQuickEdit());

    // #i89661# it's no longer necessary to make it so big here, it's fine tuned
    // for text objects in SdrMarkView::CheckSingleSdrObjectHit
    mpView->SetHitTolerancePixel( 2 * HITPIX );

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->ShowCursor(/*bGotoCursor=*/true, /*bActivate=*/true);

    FuConstruct::Activate();

    if( pOLV )
        mpView->SetEditMode(SdrViewEditMode::Edit);
}

void FuText::Deactivate()
{
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->HideCursor(/*bDeactivate=*/true);

    mpView->SetHitTolerancePixel( HITPIX );

    FuConstruct::Deactivate();
}

/**
 * Sets the object into the edit mode.
 */
void FuText::SetInEditMode(const MouseEvent& rMEvt, bool bQuickDrag)
{
    SdrPageView* pPV = mpView->GetSdrPageView();
    if( mxTextObj.is() && (mxTextObj->getSdrPageFromSdrObject() == pPV->GetPage()) )
    {
        mpView->SetCurrentObj(OBJ_TEXT);

        if( bPermanent )
            mpView->SetEditMode(SdrViewEditMode::Create);
        else
            mpView->SetEditMode(SdrViewEditMode::Edit);

        bool bEmptyOutliner = false;

        if (!GetTextObj()->GetOutlinerParaObject() && mpView->GetTextEditOutliner())
        {
            ::Outliner* pOutl = mpView->GetTextEditOutliner();
            sal_Int32 nParagraphCnt = pOutl->GetParagraphCount();
            Paragraph* p1stPara = pOutl->GetParagraph( 0 );

            if (nParagraphCnt==1 && p1stPara)
            {
                // with only one paragraph
                if (pOutl->GetText(p1stPara).isEmpty())
                {
                    bEmptyOutliner = true;
                }
            }
        }

        if (GetTextObj() != mpView->GetTextEditObject() || bEmptyOutliner)
        {
            SdrInventor nInv = mxTextObj->GetObjInventor();
            sal_uInt16  nSdrObjKind = mxTextObj->GetObjIdentifier();

            if (nInv == SdrInventor::Default && GetTextObj()->HasTextEdit() &&
                (nSdrObjKind == OBJ_TEXT ||
                 nSdrObjKind == OBJ_TITLETEXT ||
                 nSdrObjKind == OBJ_OUTLINETEXT || !mxTextObj->IsEmptyPresObj() ) )
            {
                // create new outliner (owned by SdrObjEditView)
                std::unique_ptr<SdrOutliner> pOutl = SdrMakeOutliner(OutlinerMode::OutlineObject, *mpDoc);

                if (bEmptyOutliner)
                    mpView->SdrEndTextEdit(true);

                SdrTextObj* pTextObj = GetTextObj();
                if( pTextObj )
                {
                    OutlinerParaObject* pOPO = pTextObj->GetOutlinerParaObject();
                    if( pOPO && pOPO->IsVertical() )
                        pOutl->SetVertical( true, pOPO->IsTopToBottom());
                    else if (nSlotId == SID_ATTR_CHAR_VERTICAL || nSlotId == SID_TEXT_FITTOSIZE_VERTICAL)
                        pOutl->SetVertical( true );

                    if( pTextObj->getTextCount() > 1 )
                    {
                        Point aPix(rMEvt.GetPosPixel());
                        Point aPnt(mpWindow->PixelToLogic(aPix));
                        pTextObj->setActiveText( pTextObj->CheckTextHit(aPnt ) );
                    }

                    if (mpView->SdrBeginTextEdit(pTextObj, pPV, mpWindow, true, pOutl.release()) && mxTextObj->GetObjInventor() == SdrInventor::Default)
                    {
                        //tdf#102293 flush overlay before going on to pass clicks down to
                        //the outline view which will want to paint selections
                        for (sal_uInt32 b = 0; b < pPV->PageWindowCount(); ++b)
                        {
                            const SdrPageWindow& rPageWindow = *pPV->GetPageWindow(b);
                            if (!rPageWindow.GetPaintWindow().OutputToWindow())
                                continue;
                            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
                            if (!xManager.is())
                                continue;
                            xManager->flush();
                        }

                        bFirstObjCreated = true;
                        DeleteDefaultText();

                        OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                        nSdrObjKind = mxTextObj->GetObjIdentifier();

                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

                        if (eHit == SdrHitKind::TextEdit)
                        {
                            // hit text
                            if (nSdrObjKind == OBJ_TEXT ||
                                nSdrObjKind == OBJ_TITLETEXT ||
                                nSdrObjKind == OBJ_OUTLINETEXT ||
                                nSdrObjKind == OBJ_TABLE ||
                                nSlotId == SID_TEXTEDIT ||
                                !bQuickDrag)
                            {
                                pOLV->MouseButtonDown(rMEvt);
                                pOLV->MouseMove(rMEvt);
                                pOLV->MouseButtonUp(rMEvt);
                            }

                            if (mpViewShell->GetFrameView()->IsQuickEdit() && bQuickDrag && GetTextObj()->GetOutlinerParaObject())
                            {
                                pOLV->MouseButtonDown(rMEvt);
                            }
                        }
                        else
                        {
                            // Move cursor to end of text
                            ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                            if (pOLV != nullptr)
                                pOLV->SetSelection(aNewSelection);
                        }
                    }
                    else
                    {
                        mpView->RestoreDefaultText( mxTextObj.get() );
                    }
                }
            }
        }
    }
    else
    {
        mxTextObj.reset(nullptr);
    }
}

/**
 * Text entry is started, if necessary delete the default text.
 */
void FuText::DeleteDefaultText()
{
    if ( !(mxTextObj.is() && mxTextObj->IsEmptyPresObj()) )
        return;

    SdPage* pPage = static_cast<SdPage*>( mxTextObj->getSdrPageFromSdrObject() );

    if (!pPage)
        return;

    PresObjKind ePresObjKind = pPage->GetPresObjKind(mxTextObj.get());

    if ( !((ePresObjKind == PRESOBJ_TITLE   ||
          ePresObjKind == PRESOBJ_OUTLINE ||
          ePresObjKind == PRESOBJ_NOTES   ||
          ePresObjKind == PRESOBJ_TEXT) &&
          !pPage->IsMasterPage()) )
        return;

    ::Outliner* pOutliner = mpView->GetTextEditOutliner();
    SfxStyleSheet* pSheet = pOutliner->GetStyleSheet( 0 );
    bool bIsUndoEnabled = pOutliner->IsUndoEnabled();
    if( bIsUndoEnabled )
        pOutliner->EnableUndo(false);

    pOutliner->SetText( OUString(), pOutliner->GetParagraph( 0 ) );

    if( bIsUndoEnabled )
        pOutliner->EnableUndo(true);

    if (pSheet &&
        (ePresObjKind == PRESOBJ_NOTES || ePresObjKind == PRESOBJ_TEXT))
        pOutliner->SetStyleSheet(0, pSheet);

    mxTextObj->SetEmptyPresObj(true);
}

bool FuText::RequestHelp(const HelpEvent& rHEvt)
{
    bool bReturn = false;

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if ((Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled()) &&
        mxTextObj.is() && pOLV && pOLV->GetFieldUnderMousePointer())
    {
        OUString aHelpText;
        const SvxFieldItem* pFieldItem = pOLV->GetFieldUnderMousePointer();
        const SvxFieldData* pField = pFieldItem->GetField();

        if (auto pURLField = dynamic_cast< const SvxURLField *>( pField ))
        {
            // URL-Field
            aHelpText = INetURLObject::decode( pURLField->GetURL(), INetURLObject::DecodeMechanism::WithCharset );
        }
        if (!aHelpText.isEmpty())
        {
            ::tools::Rectangle aLogicPix = mpWindow->LogicToPixel(mxTextObj->GetLogicRect());
            ::tools::Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                                  mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

            if (Help::IsBalloonHelpEnabled())
            {
                Help::ShowBalloon( static_cast<vcl::Window*>(mpWindow), rHEvt.GetMousePosPixel(), aScreenRect, aHelpText);
                bReturn = true;
            }
            else if (Help::IsQuickHelpEnabled())
            {
                Help::ShowQuickHelp( static_cast<vcl::Window*>(mpWindow), aScreenRect, aHelpText);
                bReturn = true;
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuConstruct::RequestHelp(rHEvt);
    }

    return bReturn;
}

void FuText::ReceiveRequest(SfxRequest& rReq)
{
    nSlotId = rReq.GetSlot();

    // then we call the base class (besides others, nSlotId is NOT set there)
    FuPoor::ReceiveRequest(rReq);

    if (!(nSlotId == SID_TEXTEDIT || mpViewShell->GetFrameView()->IsQuickEdit() || SID_ATTR_CHAR == nSlotId))
        return;

    MouseEvent aMEvt(mpWindow->GetPointerPosPixel());

    mxTextObj.reset(nullptr);

    if (nSlotId == SID_TEXTEDIT)
    {
        // are we currently editing?
        mxTextObj.reset( mpView->GetTextEditObject() );

        if (!mxTextObj.is())
        {
            // Try to select an object
            SdrPageView* pPV = mpView->GetSdrPageView();
            SdrViewEvent aVEvt;
            mpView->PickAnything(aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
            mpView->MarkObj(aVEvt.pRootObj, pPV);

            if (auto pSdrTextObj = dynamic_cast< SdrTextObj *>( aVEvt.pObj ))
            {
                mxTextObj.reset( pSdrTextObj );
            }
        }
    }
    else if (mpView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

            if( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr)
            {
                mxTextObj.reset( static_cast< SdrTextObj* >( pObj ) );
            }
        }
    }

    bool bQuickDrag = true;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if (pArgs

        // test for type before using
        && SID_TEXTEDIT == nSlotId
        && SfxItemState::SET == pArgs->GetItemState(SID_TEXTEDIT)

        && static_cast<const SfxUInt16Item&>( pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
    {
        // selection with double click -> do not allow QuickDrag
        bQuickDrag = false;
    }

    SetInEditMode(aMEvt, bQuickDrag);
}

void FuText::DoubleClick(const MouseEvent& )
{
    // Nothing to do
}

/** Removed the insertion of default text and putting a new text
    object directly into edit mode.
*/
SdrObjectUniquePtr FuText::CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle)
{
    SdrObjectUniquePtr pObj( SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->GetCurrentObjInventor(),
        mpView->GetCurrentObjIdentifier(),
        nullptr) );

    if(pObj)
    {
        if( auto pText = dynamic_cast< SdrTextObj *>( pObj.get() ) )
        {
            pText->SetLogicRect(rRectangle);

            bool bVertical = (SID_ATTR_CHAR_VERTICAL == nID || SID_TEXT_FITTOSIZE_VERTICAL == nID);
            pText->SetVerticalWriting(bVertical);

            ImpSetAttributesForNewTextObject(pText);

            if (nSlotId == SID_TEXT_FITTOSIZE)
            {
                ImpSetAttributesFitToSize(pText);
            }
            else if ( nSlotId == SID_TEXT_FITTOSIZE_VERTICAL )
            {
                ImpSetAttributesFitToSizeVertical(pText);
            }
            else
            {
                ImpSetAttributesFitCommon(pText);
            }

            // Put text object into edit mode.
            SdrPageView* pPV = mpView->GetSdrPageView();
            mpView->SdrBeginTextEdit(pText, pPV);
        }
        else
        {
            OSL_FAIL("Object is NO text object");
        }
    }

    return pObj;
}

/** is called when the current function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuText::cancel()
{
    if ( mpView->IsTextEdit() )
    {
        if(mpView->SdrEndTextEdit() == SdrEndTextEditKind::Deleted)
            mxTextObj.reset(nullptr);

        mpView->SetCurrentObj(OBJ_TEXT);
        mpView->SetEditMode(SdrViewEditMode::Edit);
        return true;
    }
    else
    {
        return false;
    }
}

void FuText::ChangeFontSize( bool bGrow, OutlinerView* pOLV, const FontList* pFontList, ::sd::View* pView )
{
    if( !pFontList || !pView )
        return;

    if( pOLV )
    {
        pOLV->GetEditView().ChangeFontSize( bGrow, pFontList );
    }
    else
    {

        pView->BegUndo(SdResId(bGrow ? STR_GROW_FONT_SIZE : STR_SHRINK_FONT_SIZE));
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        for( size_t nMark = 0; nMark < rMarkList.GetMarkCount(); ++nMark )
        {
            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( rMarkList.GetMark(nMark)->GetMarkedSdrObj() );
            if( pTextObj )
            {
                rtl::Reference<sdr::SelectionController> xSelectionController(pView->getSelectionController());
                if (xSelectionController.is() && xSelectionController->ChangeFontSize(bGrow, pFontList))
                {
                    continue;
                }
                for( sal_Int32 nText = 0; nText < pTextObj->getTextCount(); nText++ )
                {
                    pTextObj->setActiveText( nText );

                    // Put text object into edit mode.
                    SdrPageView* pPV = pView->GetSdrPageView();
                    pView->SdrBeginTextEdit(pTextObj, pPV);

                    pOLV = pView->GetTextEditOutlinerView();
                    if( pOLV )
                    {
                        EditEngine* pEditEngine = pOLV->GetEditView().GetEditEngine();
                        if( pEditEngine )
                        {
                            ESelection aSel;
                            aSel.nEndPara = pEditEngine->GetParagraphCount()-1;
                            aSel.nEndPos = pEditEngine->GetTextLen(aSel.nEndPara);
                            pOLV->SetSelection(aSel);
                        }

                        ChangeFontSize( bGrow, pOLV, pFontList, pView );
                    }

                    pView->SdrEndTextEdit();
                }

                SfxItemSet aShapeSet( pTextObj->GetMergedItemSet() );
                if( EditView::ChangeFontSize( bGrow, aShapeSet, pFontList ) )
                {
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT ) );
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT_CJK ) );
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT_CTL ) );
                }
            }
        }
        pView->EndUndo();
    }
}

void FuText::InvalidateBindings()
{
    mpViewShell->GetViewFrame()->GetBindings().Invalidate(SidArray);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
