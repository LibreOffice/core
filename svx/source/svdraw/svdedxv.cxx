/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/i18n/WordType.hpp>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdedxv.hxx>
#include <svl/solar.hrc>
#include <svl/itemiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/hatch.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <editeng/editstat.hxx>
#include <vcl/cursor.hxx>
#include <editeng/unotext.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include "svx/svditer.hxx"
#include "svx/svdpagv.hxx"
#include "svx/svdpage.hxx"
#include "svx/svdetc.hxx"
#include "svx/svdotable.hxx"
#include <svx/selectioncontroller.hxx>
#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif
#include <svx/svddrgv.hxx>
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svx/globl3d.hxx"
#include <editeng/outliner.hxx>
#include <editeng/adjustitem.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrundomanager.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>



void SdrObjEditView::ImpClearVars()
{
    bQuickTextEditMode=true;
    bMacroMode=true;
    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    pTextEditCursorMerker=NULL;
    pEditPara=NULL;
    bTextEditNewObj=false;
    bMacroDown=false;
    pMacroObj=NULL;
    pMacroPV=NULL;
    pMacroWin=NULL;
    nMacroTol=0;
    bTextEditDontDelete=false;
    bTextEditOnlyOneView=false;
}

SdrObjEditView::SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrGlueEditView(pModel1,pOut),
    mpOldTextEditUndoManager(0)
{
    ImpClearVars();
}

SdrObjEditView::~SdrObjEditView()
{
    pTextEditWin = NULL;            
    if (IsTextEdit())
        SdrEndTextEdit();
    delete pTextEditOutliner;
    delete mpOldTextEditUndoManager;
}



sal_Bool SdrObjEditView::IsAction() const
{
    return IsMacroObj() || SdrGlueEditView::IsAction();
}

void SdrObjEditView::MovAction(const Point& rPnt)
{
    if (IsMacroObj()) MovMacroObj(rPnt);
    SdrGlueEditView::MovAction(rPnt);
}

void SdrObjEditView::EndAction()
{
    if (IsMacroObj()) EndMacroObj();
    SdrGlueEditView::EndAction();
}

void SdrObjEditView::BckAction()
{
    BrkMacroObj();
    SdrGlueEditView::BckAction();
}

void SdrObjEditView::BrkAction()
{
    BrkMacroObj();
    SdrGlueEditView::BrkAction();
}

void SdrObjEditView::TakeActionRect(Rectangle& rRect) const
{
    if (IsMacroObj()) {
        rRect=pMacroObj->GetCurrentBoundRect();
    } else {
        SdrGlueEditView::TakeActionRect(rRect);
    }
}

void SdrObjEditView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrGlueEditView::Notify(rBC,rHint);
    
    SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
    if (pSdrHint!=NULL && pTextEditOutliner!=NULL) {
        SdrHintKind eKind=pSdrHint->GetKind();
        if (eKind==HINT_REFDEVICECHG) {
            pTextEditOutliner->SetRefDevice(pMod->GetRefDevice());
        }
        if (eKind==HINT_DEFAULTTABCHG) {
            pTextEditOutliner->SetDefTab(pMod->GetDefaultTabulator());
        }
        if (eKind==HINT_DEFFONTHGTCHG) {
            
        }
        if (eKind==HINT_MODELSAVED) {
            pTextEditOutliner->ClearModifyFlag();
        }
    }
}

void SdrObjEditView::ModelHasChanged()
{
    SdrGlueEditView::ModelHasChanged();
    if (mxTextEditObj.is() && !mxTextEditObj->IsInserted()) SdrEndTextEdit(); 
    
    if (IsTextEdit()) {
        SdrTextObj* pTextObj=dynamic_cast<SdrTextObj*>( mxTextEditObj.get() );
        if (pTextObj!=NULL) {
            sal_uIntPtr nOutlViewAnz=pTextEditOutliner->GetViewCount();
            bool bAreaChg=false;
            bool bAnchorChg=false;
            bool bColorChg=false;
            bool bContourFrame=pTextObj->IsContourTextFrame();
            EVAnchorMode eNewAnchor(ANCHOR_VCENTER_HCENTER);
            Rectangle aOldArea(aMinTextEditArea);
            aOldArea.Union(aTextEditArea);
            Color aNewColor;
            { 
                Size aPaperMin1;
                Size aPaperMax1;
                Rectangle aEditArea1;
                Rectangle aMinArea1;
                pTextObj->TakeTextEditArea(&aPaperMin1,&aPaperMax1,&aEditArea1,&aMinArea1);

                Point aPvOfs(pTextObj->GetTextEditOffset());
                
                
                
                aEditArea1 += pTextObj->GetGridOffset();
                aMinArea1 += pTextObj->GetGridOffset();
                aEditArea1.Move(aPvOfs.X(),aPvOfs.Y());
                aMinArea1.Move(aPvOfs.X(),aPvOfs.Y());
                Rectangle aNewArea(aMinArea1);
                aNewArea.Union(aEditArea1);

                if (aNewArea!=aOldArea || aEditArea1!=aTextEditArea || aMinArea1!=aMinTextEditArea ||
                    pTextEditOutliner->GetMinAutoPaperSize()!=aPaperMin1 || pTextEditOutliner->GetMaxAutoPaperSize()!=aPaperMax1) {
                    aTextEditArea=aEditArea1;
                    aMinTextEditArea=aMinArea1;
                    pTextEditOutliner->SetUpdateMode(sal_False);
                    pTextEditOutliner->SetMinAutoPaperSize(aPaperMin1);
                    pTextEditOutliner->SetMaxAutoPaperSize(aPaperMax1);
                    pTextEditOutliner->SetPaperSize(Size(0,0)); 
                    if (!bContourFrame) {
                        pTextEditOutliner->ClearPolygon();
                        sal_uIntPtr nStat=pTextEditOutliner->GetControlWord();
                        nStat|=EE_CNTRL_AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                    } else {
                        sal_uIntPtr nStat=pTextEditOutliner->GetControlWord();
                        nStat&=~EE_CNTRL_AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                        Rectangle aAnchorRect;
                        pTextObj->TakeTextAnchorRect(aAnchorRect);
                        pTextObj->ImpSetContourPolygon(*pTextEditOutliner,aAnchorRect, sal_True);
                    }
                    for (sal_uIntPtr nOV=0; nOV<nOutlViewAnz; nOV++) {
                        OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                        sal_uIntPtr nStat0=pOLV->GetControlWord();
                        sal_uIntPtr nStat=nStat0;
                        
                        if (!bContourFrame) nStat|=EV_CNTRL_AUTOSIZE;
                        else nStat&=~EV_CNTRL_AUTOSIZE;
                        if (nStat!=nStat0) pOLV->SetControlWord(nStat);
                    }
                    pTextEditOutliner->SetUpdateMode(sal_True);
                    bAreaChg=true;
                }
            }
            if (pTextEditOutlinerView!=NULL) { 
                EVAnchorMode eOldAnchor=pTextEditOutlinerView->GetAnchorMode();
                eNewAnchor=(EVAnchorMode)pTextObj->GetOutlinerViewAnchorMode();
                bAnchorChg=eOldAnchor!=eNewAnchor;
                Color aOldColor(pTextEditOutlinerView->GetBackgroundColor());
                aNewColor = GetTextEditBackgroundColor(*this);
                bColorChg=aOldColor!=aNewColor;
            }
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            if (bContourFrame || bAreaChg || bAnchorChg || bColorChg)
            {
                for (sal_uIntPtr nOV=0; nOV<nOutlViewAnz; nOV++)
                {
                    OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                    { 
                        Window* pWin=pOLV->GetWindow();
                        Rectangle aTmpRect(aOldArea);
                        sal_uInt16 nPixSiz=pOLV->GetInvalidateMore()+1;
                        Size aMore(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                        aTmpRect.Left()-=aMore.Width();
                        aTmpRect.Right()+=aMore.Width();
                        aTmpRect.Top()-=aMore.Height();
                        aTmpRect.Bottom()+=aMore.Height();
                        InvalidateOneWin(*pWin,aTmpRect);
                    }
                    if (bAnchorChg)
                        pOLV->SetAnchorMode(eNewAnchor);
                    if (bColorChg)
                        pOLV->SetBackgroundColor( aNewColor );

                    pOLV->SetOutputArea(aTextEditArea); 
                    ImpInvalidateOutlinerView(*pOLV);
                }
                pTextEditOutlinerView->ShowCursor();
            }
        }
        ImpMakeTextCursorAreaVisible();
    }
}





void SdrObjEditView::TextEditDrawing(SdrPaintWindow& rPaintWindow) const
{
    
    if(IsTextEdit())
    {
        const SdrOutliner* pActiveOutliner = GetTextEditOutliner();

        if(pActiveOutliner)
        {
            const sal_uInt32 nViewAnz(pActiveOutliner->GetViewCount());

            if(nViewAnz)
            {
                const Region& rRedrawRegion = rPaintWindow.GetRedrawRegion();
                const Rectangle aCheckRect(rRedrawRegion.GetBoundRect());

                for(sal_uInt32 i(0); i < nViewAnz; i++)
                {
                    OutlinerView* pOLV = pActiveOutliner->GetView(i);

                    if(pOLV->GetWindow() == &rPaintWindow.GetOutputDevice())
                    {
                        ImpPaintOutlinerView(*pOLV, aCheckRect, rPaintWindow.GetTargetOutputDevice());
                        return;
                    }
                }
            }
        }
    }
}

void SdrObjEditView::ImpPaintOutlinerView(OutlinerView& rOutlView, const Rectangle& rRect, OutputDevice& rTargetDevice) const
{
    const SdrTextObj* pText = PTR_CAST(SdrTextObj,GetTextEditObject());
    bool bTextFrame(pText && pText->IsTextFrame());
    bool bFitToSize(0 != (pTextEditOutliner->GetControlWord() & EE_CNTRL_STRETCHING));
    bool bModifyMerk(pTextEditOutliner->IsModified()); 
    Rectangle aBlankRect(rOutlView.GetOutputArea());
    aBlankRect.Union(aMinTextEditArea);
    Rectangle aPixRect(rTargetDevice.LogicToPixel(aBlankRect));
    aBlankRect.Intersection(rRect);
    rOutlView.GetOutliner()->SetUpdateMode(sal_True); 
    rOutlView.Paint(aBlankRect, &rTargetDevice);

    if(!bModifyMerk)
    {
        
        pTextEditOutliner->ClearModifyFlag();
    }

    if(bTextFrame && !bFitToSize)
    {
        
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        drawinglayer::processor2d::BaseProcessor2D* pProcessor = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
            rTargetDevice,
            aViewInformation2D);

        if(pProcessor)
        {
            const bool bMerk(rTargetDevice.IsMapModeEnabled());
            const basegfx::B2DRange aRange(aPixRect.Left(), aPixRect.Top(), aPixRect.Right(), aPixRect.Bottom());
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
            const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
            const sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::OverlayRectanglePrimitive(
                    aRange,
                    aHilightColor.getBColor(),
                    fTransparence,
                    std::max(6, nPixSiz - 2), 
                    0.0, 
                    0.0));
            const drawinglayer::primitive2d::Primitive2DSequence aSequence(&xReference, 1);

            rTargetDevice.EnableMapMode(false);
            pProcessor->process(aSequence);
            rTargetDevice.EnableMapMode(bMerk);
            delete pProcessor;
        }
    }

    rOutlView.ShowCursor();
}

void SdrObjEditView::ImpInvalidateOutlinerView(OutlinerView& rOutlView) const
{
    Window* pWin = rOutlView.GetWindow();

    if(pWin)
    {
        const SdrTextObj* pText = PTR_CAST(SdrTextObj,GetTextEditObject());
        bool bTextFrame(pText && pText->IsTextFrame());
        bool bFitToSize(pText && pText->IsFitToSize());

        if(bTextFrame && !bFitToSize)
        {
            Rectangle aBlankRect(rOutlView.GetOutputArea());
            aBlankRect.Union(aMinTextEditArea);
            Rectangle aPixRect(pWin->LogicToPixel(aBlankRect));
            sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);

            aPixRect.Left()--;
            aPixRect.Top()--;
            aPixRect.Right()++;
            aPixRect.Bottom()++;

            {
                
                Size aMaxXY(pWin->GetOutputSizePixel());
                long a(2 * nPixSiz);
                long nMaxX(aMaxXY.Width() + a);
                long nMaxY(aMaxXY.Height() + a);

                if (aPixRect.Left  ()<-a) aPixRect.Left()=-a;
                if (aPixRect.Top   ()<-a) aPixRect.Top ()=-a;
                if (aPixRect.Right ()>nMaxX) aPixRect.Right ()=nMaxX;
                if (aPixRect.Bottom()>nMaxY) aPixRect.Bottom()=nMaxY;
            }

            Rectangle aOuterPix(aPixRect);
            aOuterPix.Left()-=nPixSiz;
            aOuterPix.Top()-=nPixSiz;
            aOuterPix.Right()+=nPixSiz;
            aOuterPix.Bottom()+=nPixSiz;

            bool bMerk(pWin->IsMapModeEnabled());
            pWin->EnableMapMode(false);
            pWin->Invalidate(aOuterPix);
            pWin->EnableMapMode(bMerk);
        }
    }
}

OutlinerView* SdrObjEditView::ImpMakeOutlinerView(Window* pWin, bool /*bNoPaint*/, OutlinerView* pGivenView) const
{
    
    Color aBackground(GetTextEditBackgroundColor(*this));
    SdrTextObj* pText = dynamic_cast< SdrTextObj * >( mxTextEditObj.get() );
    bool bTextFrame=pText!=NULL && pText->IsTextFrame();
    bool bContourFrame=pText!=NULL && pText->IsContourTextFrame();
    
    OutlinerView* pOutlView=pGivenView;
    pTextEditOutliner->SetUpdateMode(sal_False);
    if (pOutlView==NULL) pOutlView=new OutlinerView(pTextEditOutliner,pWin);
    else pOutlView->SetWindow(pWin);
    
    sal_uIntPtr nStat=pOutlView->GetControlWord();
    nStat&=~EV_CNTRL_AUTOSCROLL;
    
    if (!bContourFrame) nStat|=EV_CNTRL_AUTOSIZE;
    if (bTextFrame) {
        sal_uInt16 nPixSiz=aHdl.GetHdlSize()*2+1;
        nStat|=EV_CNTRL_INVONEMORE;
        pOutlView->SetInvalidateMore(nPixSiz);
    }
    pOutlView->SetControlWord(nStat);
    pOutlView->SetBackgroundColor( aBackground );
    if (pText!=NULL)
    {
        pOutlView->SetAnchorMode((EVAnchorMode)(pText->GetOutlinerViewAnchorMode()));
        pTextEditOutliner->SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)pText->GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    }
    
    pTextEditOutliner->SetUpdateMode(sal_True);
    pOutlView->SetOutputArea(aTextEditArea);
    ImpInvalidateOutlinerView(*pOutlView);
    return pOutlView;
}

IMPL_LINK(SdrObjEditView,ImpOutlinerStatusEventHdl,EditStatus*,pEditStat)
{
    if(pTextEditOutliner )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj * >( mxTextEditObj.get() );
        if( pTextObj )
        {
            pTextObj->onEditOutlinerStatusEvent( pEditStat );
        }
    }
    return 0;
}

IMPL_LINK(SdrObjEditView,ImpOutlinerCalcFieldValueHdl,EditFieldInfo*,pFI)
{
    bool bOk=false;
    OUString& rStr=pFI->GetRepresentation();
    rStr = "";
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );
    if (pTextObj!=NULL) {
        Color* pTxtCol=NULL;
        Color* pFldCol=NULL;
        bOk=pTextObj->CalcFieldValue(pFI->GetField(),pFI->GetPara(),pFI->GetPos(),true,pTxtCol,pFldCol,rStr);
        if (bOk) {
            if (pTxtCol!=NULL) {
                pFI->SetTxtColor(*pTxtCol);
                delete pTxtCol;
            }
            if (pFldCol!=NULL) {
                pFI->SetFldColor(*pFldCol);
                delete pFldCol;
            } else {
                pFI->SetFldColor(Color(COL_LIGHTGRAY)); 
            }
        }
    }
    Outliner& rDrawOutl=pMod->GetDrawOutliner(pTextObj);
    Link aDrawOutlLink=rDrawOutl.GetCalcFieldValueHdl();
    if (!bOk && aDrawOutlLink.IsSet()) {
        aDrawOutlLink.Call(pFI);
        bOk = !rStr.isEmpty();
    }
    if (!bOk && aOldCalcFieldValueLink.IsSet()) {
        return aOldCalcFieldValueLink.Call(pFI);
    }
    return 0;
}

IMPL_LINK(SdrObjEditView, EndTextEditHdl, SdrUndoManager*, /*pUndoManager*/)
{
    SdrEndTextEdit();
    return 0;
}

SdrUndoManager* SdrObjEditView::getSdrUndoManagerForEnhancedTextEdit() const
{
    
    return GetModel() ? dynamic_cast< SdrUndoManager* >(GetModel()->GetSdrUndoManager()) : 0;
}

sal_Bool SdrObjEditView::SdrBeginTextEdit(
    SdrObject* pObj, SdrPageView* pPV, Window* pWin,
    sal_Bool bIsNewObj, SdrOutliner* pGivenOutliner,
    OutlinerView* pGivenOutlinerView,
    sal_Bool bDontDeleteOutliner, sal_Bool bOnlyOneView,
    sal_Bool bGrabFocus)
{
    SdrEndTextEdit();

    if( dynamic_cast< SdrTextObj* >( pObj ) == 0 )
        return sal_False; 

    if(bGrabFocus && pWin)
    {
        
        pWin->GrabFocus(); 
    }

    bTextEditDontDelete=bDontDeleteOutliner && pGivenOutliner!=NULL;
    bTextEditOnlyOneView=bOnlyOneView;
    bTextEditNewObj=bIsNewObj;
    const sal_uInt32 nWinAnz(PaintWindowCount());
    sal_uInt32 i;
    bool bBrk(false);
    

    if(!pObj)
    {
        bBrk = true;
    }

    if(!bBrk && !pWin)
    {
        for(i = 0L; i < nWinAnz && !pWin; i++)
        {
            SdrPaintWindow* pPaintWindow = GetPaintWindow(i);

            if(OUTDEV_WINDOW == pPaintWindow->GetOutputDevice().GetOutDevType())
            {
                pWin = (Window*)(&pPaintWindow->GetOutputDevice());
            }
        }

        
        if(!pWin)
        {
            bBrk = true;
        }
    }

    if(!bBrk && !pPV)
    {
        pPV = GetSdrPageView();

        
        if(!pPV)
        {
            bBrk = true;
        }
    }

    if(pObj && pPV)
    {
        
        if(pPV->GetLockedLayers().IsSet(pObj->GetLayer()))
        {
            bBrk = true;
        }
    }

    if(pTextEditOutliner)
    {
        OSL_FAIL("SdrObjEditView::SdrBeginTextEdit(): Old Outliner still exists.");
        delete pTextEditOutliner;
        pTextEditOutliner = 0L;
    }

    if(!bBrk)
    {
        pTextEditWin=pWin;
        pTextEditPV=pPV;
        mxTextEditObj.reset( pObj );
        pTextEditOutliner=pGivenOutliner;
        if (pTextEditOutliner==NULL)
            pTextEditOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, mxTextEditObj->GetModel() );

        {
            SvtAccessibilityOptions aOptions;
            pTextEditOutliner->ForceAutoColor( aOptions.GetIsAutomaticFontColor() );
        }

        bool bEmpty = mxTextEditObj->GetOutlinerParaObject()==NULL;

        aOldCalcFieldValueLink=pTextEditOutliner->GetCalcFieldValueHdl();
        
        pTextEditOutliner->SetCalcFieldValueHdl(LINK(this,SdrObjEditView,ImpOutlinerCalcFieldValueHdl));
        pTextEditOutliner->SetBeginPasteOrDropHdl(LINK(this,SdrObjEditView,BeginPasteOrDropHdl));
        pTextEditOutliner->SetEndPasteOrDropHdl(LINK(this,SdrObjEditView, EndPasteOrDropHdl));

        
        pTextEditOutliner->setVisualizedPage(pPV ? pPV->GetPage() : 0);

        pTextEditOutliner->SetTextObjNoInit( dynamic_cast< SdrTextObj* >( mxTextEditObj.get() ) );

        if(mxTextEditObj->BegTextEdit(*pTextEditOutliner))
        {
            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );
            DBG_ASSERT( pTextObj, "svx::SdrObjEditView::BegTextEdit(), no text object?" );
            if( !pTextObj )
                return sal_False;

            
            pTextObj->SetTextAnimationAllowed(sal_False);

            
            if (pTextEditOutliner->GetViewCount()!=0)
            {
                OutlinerView* pTmpOLV=pTextEditOutliner->RemoveView(static_cast<size_t>(0));
                if(pTmpOLV!=NULL && pTmpOLV!=pGivenOutlinerView)
                    delete pTmpOLV;
            }

            
            
            
            pTextObj->TakeTextEditArea(NULL,NULL,&aTextEditArea,&aMinTextEditArea);

            Rectangle aTextRect;
            Rectangle aAnchorRect;
            pTextObj->TakeTextRect(*pTextEditOutliner, aTextRect, true,
                &aAnchorRect /* Give sal_True here, not sal_False */);

            if ( !pTextObj->IsContourTextFrame() )
            {
                
                if (pTextObj->IsFitToSize())
                    aTextRect = aAnchorRect;
            }

            aTextEditArea = aTextRect;

            
            
            

            Point aPvOfs(pTextObj->GetTextEditOffset());
            aTextEditArea += pTextObj->GetGridOffset();
            aTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            aMinTextEditArea += pTextObj->GetGridOffset();
            aMinTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            pTextEditCursorMerker=pWin->GetCursor();

            aHdl.SetMoveOutside(sal_True);

            
            
            
            AdjustMarkHdl();

            pTextEditOutlinerView=ImpMakeOutlinerView(pWin,!bEmpty,pGivenOutlinerView);

            
            sal_uIntPtr i2,nCount = pTextEditOutliner->GetViewCount();
            for( i2 = 0; i2 < nCount; i2++ )
            {
                if( pTextEditOutliner->GetView(i2) == pTextEditOutlinerView )
                    break;
            }

            if( i2 == nCount )
                pTextEditOutliner->InsertView(pTextEditOutlinerView,0);

            aHdl.SetMoveOutside(sal_False);
            aHdl.SetMoveOutside(sal_True);

            
            if(!bOnlyOneView)
            {
                for(i = 0L; i < nWinAnz; i++)
                {
                    SdrPaintWindow* pPaintWindow = GetPaintWindow(i);
                    OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

                    if(&rOutDev != pWin && OUTDEV_WINDOW == rOutDev.GetOutDevType())
                    {
                        OutlinerView* pOutlView = ImpMakeOutlinerView((Window*)(&rOutDev), !bEmpty, 0L);
                        pTextEditOutliner->InsertView(pOutlView, (sal_uInt16)i);
                    }
                }
            }

            pTextEditOutlinerView->ShowCursor();
            pTextEditOutliner->SetStatusEventHdl(LINK(this,SdrObjEditView,ImpOutlinerStatusEventHdl));
#ifdef DBG_UTIL
            if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
            pTextEditOutliner->ClearModifyFlag();

            if(pWin)
            {
                bool bExtraInvalidate(false);

                if(!bExtraInvalidate)
                {
                    if(pTextObj->IsFitToSize())
                        bExtraInvalidate = true;
                }

                if(bExtraInvalidate)
                {
                    pWin->Invalidate(aTextEditArea);
                }
            }

            if( GetModel() )
            {
                SdrHint aHint(*pTextObj);
                aHint.SetKind(HINT_BEGEDIT);
                GetModel()->Broadcast(aHint);
            }

            pTextEditOutliner->setVisualizedPage(0);

            if( mxSelectionController.is() )
                mxSelectionController->onSelectionHasChanged();

            if(IsUndoEnabled() && GetModel() && !GetModel()->GetDisableTextEditUsesCommonUndoManager())
            {
                SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

                if(pSdrUndoManager)
                {
                    
                    
                    
                    if(mpOldTextEditUndoManager)
                    {
                        
                        OSL_ENSURE(false, "Deleting forgotten old TextEditUndoManager, should be checked (!)");
                        delete mpOldTextEditUndoManager;
                        mpOldTextEditUndoManager = 0;
                    }

                    mpOldTextEditUndoManager = pTextEditOutliner->SetUndoManager(pSdrUndoManager);
                    pSdrUndoManager->SetEndTextEditHdl(LINK(this, SdrObjEditView, EndTextEditHdl));
                }
                else
                {
                    OSL_ENSURE(false, "The document undo manager is not derived from SdrUndoManager (!)");
                }
            }

            return sal_True; 
        }
        else
        {
            bBrk = true;
            pTextEditOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);
            pTextEditOutliner->SetBeginPasteOrDropHdl(Link());
            pTextEditOutliner->SetEndPasteOrDropHdl(Link());

        }
    }
    if (pTextEditOutliner != NULL)
    {
        pTextEditOutliner->setVisualizedPage(0);
    }

    
    if(!bDontDeleteOutliner)
    {
        if(pGivenOutliner!=NULL)
        {
            delete pGivenOutliner;
            pTextEditOutliner = NULL;
        }
        if(pGivenOutlinerView!=NULL)
        {
            delete pGivenOutlinerView;
            pGivenOutlinerView = NULL;
        }
    }
    if( pTextEditOutliner!=NULL )
    {
        delete pTextEditOutliner;
    }

    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    mxTextEditObj.reset(0);
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    aHdl.SetMoveOutside(sal_False);

    return sal_False;
}

SdrEndTextEditKind SdrObjEditView::SdrEndTextEdit(sal_Bool bDontDeleteReally)
{
    SdrEndTextEditKind eRet=SDRENDTEXTEDIT_UNCHANGED;
    SdrTextObj* pTEObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );
    Window*       pTEWin         =pTextEditWin;
    SdrOutliner*  pTEOutliner    =pTextEditOutliner;
    OutlinerView* pTEOutlinerView=pTextEditOutlinerView;
    Cursor*       pTECursorMerker=pTextEditCursorMerker;
    SdrUndoManager* pUndoEditUndoManager = 0;
    bool bNeedToUndoSavedRedoTextEdit(false);

    if(IsUndoEnabled() && GetModel() && pTEObj && pTEOutliner && !GetModel()->GetDisableTextEditUsesCommonUndoManager())
    {
        
        ::svl::IUndoManager* pOriginal = pTEOutliner->SetUndoManager(mpOldTextEditUndoManager);
        mpOldTextEditUndoManager = 0;

        if(pOriginal)
        {
            
            SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

            if(pSdrUndoManager && dynamic_cast< SdrUndoManager* >(pOriginal) == pSdrUndoManager)
            {
                if(pSdrUndoManager->isEndTextEditTriggeredFromUndo())
                {
                    
                    
                    
                    pUndoEditUndoManager = pSdrUndoManager;

                    
                    
                    
                    
                    while(pSdrUndoManager->GetRedoActionCount())
                    {
                        bNeedToUndoSavedRedoTextEdit = true;
                        pSdrUndoManager->Redo();
                    }
                }

                
                
                pSdrUndoManager->SetEndTextEditHdl(Link());
            }
            else
            {
                OSL_ENSURE(false, "Got UndoManager back in SdrEndTextEdit which is NOT the expected document UndoManager (!)");
                delete pOriginal;
            }
        }
    }

    if( GetModel() && mxTextEditObj.is() )
    {
        SdrHint aHint(*mxTextEditObj.get());
        aHint.SetKind(HINT_ENDEDIT);
        GetModel()->Broadcast(aHint);
    }

    mxTextEditObj.reset(0);
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    pTextEditCursorMerker=NULL;
    aTextEditArea=Rectangle();

    if (pTEOutliner!=NULL)
    {
        sal_Bool bModified=pTEOutliner->IsModified();
        if (pTEOutlinerView!=NULL)
        {
            pTEOutlinerView->HideCursor();
        }
        if (pTEObj!=NULL)
        {
            pTEOutliner->CompleteOnlineSpelling();

            SdrUndoObjSetText* pTxtUndo = 0;

            if( bModified )
            {
                sal_Int32 nText;
                for( nText = 0; nText < pTEObj->getTextCount(); ++nText )
                    if( pTEObj->getText( nText ) == pTEObj->getActiveText() )
                        break;

                pTxtUndo = dynamic_cast< SdrUndoObjSetText* >( GetModel()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTEObj, nText ) );
            }
            DBG_ASSERT( !bModified || pTxtUndo, "svx::SdrObjEditView::EndTextEdit(), could not create undo action!" );
            
            
            pTEOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);
            pTEOutliner->SetBeginPasteOrDropHdl(Link());
            pTEOutliner->SetEndPasteOrDropHdl(Link());

            const bool bUndo = IsUndoEnabled();
            if( bUndo )
            {
                OUString aObjName(pTEObj->TakeObjNameSingul());
                BegUndo(ImpGetResStr(STR_UndoObjSetText),aObjName);
            }

            pTEObj->EndTextEdit(*pTEOutliner);

            if( (pTEObj->GetRotateAngle() != 0) || (pTEObj && pTEObj->ISA(SdrTextObj) && ((SdrTextObj*)pTEObj)->IsFontwork())  )
            {
                pTEObj->ActionChanged();
            }

            if (pTxtUndo!=NULL)
            {
                pTxtUndo->AfterSetText();
                if (!pTxtUndo->IsDifferent())
                {
                    delete pTxtUndo;
                    pTxtUndo=NULL;
                }
            }
            
            SdrUndoAction* pDelUndo=NULL;
            bool bDelObj=false;
            SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pTEObj);
            if (pTextObj!=NULL && bTextEditNewObj)
            {
                bDelObj=pTextObj->IsTextFrame() &&
                        !pTextObj->HasText() &&
                        !pTextObj->IsEmptyPresObj() &&
                        !pTextObj->HasFill() &&
                        !pTextObj->HasLine();

                if(pTEObj->IsInserted() && bDelObj && pTextObj->GetObjInventor()==SdrInventor && !bDontDeleteReally)
                {
                    SdrObjKind eIdent=(SdrObjKind)pTextObj->GetObjIdentifier();
                    if(eIdent==OBJ_TEXT || eIdent==OBJ_TEXTEXT)
                    {
                        pDelUndo= GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pTEObj);
                    }
                }
            }
            if (pTxtUndo!=NULL)
            {
                if( bUndo )
                    AddUndo(pTxtUndo);
                eRet=SDRENDTEXTEDIT_CHANGED;
            }
            if (pDelUndo!=NULL)
            {
                if( bUndo )
                {
                    AddUndo(pDelUndo);
                }
                else
                {
                    delete pDelUndo;
                }
                eRet=SDRENDTEXTEDIT_DELETED;
                DBG_ASSERT(pTEObj->GetObjList()!=NULL,"SdrObjEditView::SdrEndTextEdit(): Fatal: Object edited doesn't have an ObjList!");
                if (pTEObj->GetObjList()!=NULL)
                {
                    pTEObj->GetObjList()->RemoveObject(pTEObj->GetOrdNum());
                    CheckMarked(); 
                }
            }
            else if (bDelObj)
            { 
                eRet=SDRENDTEXTEDIT_SHOULDBEDELETED;
            }

            if( bUndo )
                EndUndo(); 

            
            if(pTEObj->ISA(SdrTextObj))
            {
                ((SdrTextObj*)pTEObj)->SetTextAnimationAllowed(sal_True);
            }

            
            
            
            AdjustMarkHdl();
        }
        
        for (sal_uIntPtr i=pTEOutliner->GetViewCount(); i>0;)
        {
            i--;
            OutlinerView* pOLV=pTEOutliner->GetView(i);
            sal_uInt16 nMorePix=pOLV->GetInvalidateMore() + 10;
            Window* pWin=pOLV->GetWindow();
            Rectangle aRect(pOLV->GetOutputArea());
            pTEOutliner->RemoveView(i);
            if (!bTextEditDontDelete || i!=0)
            {
                
                delete pOLV;
            }
            aRect.Union(aTextEditArea);
            aRect.Union(aMinTextEditArea);
            aRect=pWin->LogicToPixel(aRect);
            aRect.Left()-=nMorePix;
            aRect.Top()-=nMorePix;
            aRect.Right()+=nMorePix;
            aRect.Bottom()+=nMorePix;
            aRect=pWin->PixelToLogic(aRect);
            InvalidateOneWin(*pWin,aRect);
            pWin->SetFillColor();
            pWin->SetLineColor(COL_BLACK);
            pWin->DrawPixel(aRect.TopLeft());
            pWin->DrawPixel(aRect.TopRight());
            pWin->DrawPixel(aRect.BottomLeft());
            pWin->DrawPixel(aRect.BottomRight());
        }
        
        if (!bTextEditDontDelete) delete pTEOutliner;
        else pTEOutliner->Clear();
        if (pTEWin!=NULL) {
            pTEWin->SetCursor(pTECursorMerker);
        }
        aHdl.SetMoveOutside(sal_False);
        if (eRet!=SDRENDTEXTEDIT_UNCHANGED)
        {
            GetMarkedObjectListWriteAccess().SetNameDirty();
        }
#ifdef DBG_UTIL
        if (pItemBrowser)
        {
            GetMarkedObjectListWriteAccess().SetNameDirty();
            pItemBrowser->SetDirty();
        }
#endif
    }

    if( pTEObj &&
        pTEObj->GetModel() &&
        !pTEObj->GetModel()->isLocked() &&
        pTEObj->GetBroadcaster())
    {
        SdrHint aHint(HINT_ENDEDIT);
        aHint.SetObject(pTEObj);
        ((SfxBroadcaster*)pTEObj->GetBroadcaster())->Broadcast(aHint);
    }

    if(pUndoEditUndoManager)
    {
        if(bNeedToUndoSavedRedoTextEdit)
        {
            
            
            
            pUndoEditUndoManager->Undo();
        }

        
        
        pUndoEditUndoManager->Undo();
    }

    return eRet;
}



bool SdrObjEditView::IsTextEdit() const
{
    return mxTextEditObj.is();
}


SdrPageView* SdrObjEditView::GetTextEditPageView() const
{
    return pTextEditPV;
}



OutlinerView* SdrObjEditView::ImpFindOutlinerView(Window* pWin) const
{
    if (pWin==NULL) return NULL;
    if (pTextEditOutliner==NULL) return NULL;
    OutlinerView* pNewView=NULL;
    sal_uIntPtr nWinAnz=pTextEditOutliner->GetViewCount();
    for (sal_uIntPtr i=0; i<nWinAnz && pNewView==NULL; i++) {
        OutlinerView* pView=pTextEditOutliner->GetView(i);
        if (pView->GetWindow()==pWin) pNewView=pView;
    }
    return pNewView;
}

void SdrObjEditView::SetTextEditWin(Window* pWin)
{
    if(mxTextEditObj.is() && pWin!=NULL && pWin!=pTextEditWin)
    {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);
        if (pNewView!=NULL && pNewView!=pTextEditOutlinerView)
        {
            if (pTextEditOutlinerView!=NULL)
            {
                pTextEditOutlinerView->HideCursor();
            }
            pTextEditOutlinerView=pNewView;
            pTextEditWin=pWin;
            pWin->GrabFocus(); 
            pNewView->ShowCursor();
            ImpMakeTextCursorAreaVisible();
        }
    }
}

sal_Bool SdrObjEditView::IsTextEditHit(const Point& rHit, short nTol) const
{
    sal_Bool bOk=sal_False;
    if(mxTextEditObj.is())
    {
        nTol=ImpGetHitTolLogic(nTol,NULL);
        
        nTol=nTol/3;
        nTol=0; 
        if (!bOk)
        {
            Rectangle aEditArea;
            OutlinerView* pOLV=pTextEditOutliner->GetView(0);
            if (pOLV!=NULL)
            {
                aEditArea.Union(pOLV->GetOutputArea());
            }
            aEditArea.Left()-=nTol;
            aEditArea.Top()-=nTol;
            aEditArea.Right()+=nTol;
            aEditArea.Bottom()+=nTol;
            bOk=aEditArea.IsInside(rHit);
            if (bOk)
            { 
                Point aPnt(rHit); aPnt-=aEditArea.TopLeft();
                long nHitTol = 2000;
                OutputDevice* pRef = pTextEditOutliner->GetRefDevice();
                if( pRef )
                    nHitTol = pRef->LogicToLogic( nHitTol, MAP_100TH_MM, pRef->GetMapMode().GetMapUnit() );

                bOk = pTextEditOutliner->IsTextPos( aPnt, (sal_uInt16)nHitTol );
            }
        }
    }
    return bOk;
}

sal_Bool SdrObjEditView::IsTextEditFrameHit(const Point& rHit) const
{
    sal_Bool bOk=sal_False;
    if(mxTextEditObj.is())
    {
        SdrTextObj* pText= dynamic_cast<SdrTextObj*>(mxTextEditObj.get());
        OutlinerView* pOLV=pTextEditOutliner->GetView(0);
        if( pOLV )
        {
            Window* pWin=pOLV->GetWindow();
            if (pText!=NULL && pText->IsTextFrame() && pOLV!=NULL && pWin!=NULL) {
                sal_uInt16 nPixSiz=pOLV->GetInvalidateMore();
                Rectangle aEditArea(aMinTextEditArea);
                aEditArea.Union(pOLV->GetOutputArea());
                if (!aEditArea.IsInside(rHit)) {
                    Size aSiz(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                    aEditArea.Left()-=aSiz.Width();
                    aEditArea.Top()-=aSiz.Height();
                    aEditArea.Right()+=aSiz.Width();
                    aEditArea.Bottom()+=aSiz.Height();
                    bOk=aEditArea.IsInside(rHit);
                }
            }
        }
    }
    return bOk;
}



sal_Bool SdrObjEditView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    if(pTextEditOutlinerView)
    {
        if (pTextEditOutlinerView->PostKeyEvent(rKEvt, pWin))
        {
            if( pMod )
            {
                if( pTextEditOutliner && pTextEditOutliner->IsModified() )
                    pMod->SetChanged( true );
            }

            if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
            if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
            ImpMakeTextCursorAreaVisible();
            return sal_True;
        }
    }
    return SdrGlueEditView::KeyInput(rKEvt,pWin);
}

sal_Bool SdrObjEditView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        sal_Bool bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            if (pWin)
            {
                Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
                if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
                if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
                if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
                if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            }
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonDown(aMEvt)) {
                if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return sal_True;
            }
        }
    }
    return SdrGlueEditView::MouseButtonDown(rMEvt,pWin);
}

sal_Bool SdrObjEditView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        sal_Bool bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
            if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
            if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
            if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonUp(aMEvt)) {
#ifdef DBG_UTIL
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return sal_True;
            }
        }
    }
    return SdrGlueEditView::MouseButtonUp(rMEvt,pWin);
}

sal_Bool SdrObjEditView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        sal_Bool bSelMode=pTextEditOutliner->IsInSelectionMode();
        sal_Bool bPostIt=bSelMode;
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
            if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
            if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
            if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseMove(aMEvt) && bSelMode) {
#ifdef DBG_UTIL
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return sal_True;
            }
        }
    }
    return SdrGlueEditView::MouseMove(rMEvt,pWin);
}

sal_Bool SdrObjEditView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    
    if (pTextEditOutlinerView!=NULL)
    {
        if (rCEvt.GetCommand()==COMMAND_STARTDRAG) {
            sal_Bool bPostIt=pTextEditOutliner->IsInSelectionMode() || !rCEvt.IsMouseEvent();
            if (!bPostIt && rCEvt.IsMouseEvent()) {
                Point aPt(rCEvt.GetMousePosPixel());
                if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
                else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
                bPostIt=IsTextEditHit(aPt,nHitTolLog);
            }
            if (bPostIt) {
                Point aPixPos(rCEvt.GetMousePosPixel());
                if (rCEvt.IsMouseEvent()) {
                    Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
                    if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
                    if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
                    if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
                    if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
                }
                CommandEvent aCEvt(aPixPos,rCEvt.GetCommand(),rCEvt.IsMouseEvent());
                
                pTextEditOutlinerView->Command(aCEvt);
                if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return sal_True;
            }
        }
        else
        {
            pTextEditOutlinerView->Command(rCEvt);
            return sal_True;
        }
    }
    return SdrGlueEditView::Command(rCEvt,pWin);
}



sal_Bool SdrObjEditView::ImpIsTextEditAllSelected() const
{
    sal_Bool bRet=sal_False;
    if (pTextEditOutliner!=NULL && pTextEditOutlinerView!=NULL)
    {
        if(SdrTextObj::HasTextImpl( pTextEditOutliner ) )
        {
            const sal_Int32 nParaAnz=pTextEditOutliner->GetParagraphCount();
            Paragraph* pLastPara=pTextEditOutliner->GetParagraph( nParaAnz > 1 ? nParaAnz - 1 : 0 );

            ESelection aESel(pTextEditOutlinerView->GetSelection());
            if (aESel.nStartPara==0 && aESel.nStartPos==0 && aESel.nEndPara==(nParaAnz-1))
            {
                if( pTextEditOutliner->GetText(pLastPara).getLength() == aESel.nEndPos )
                    bRet = sal_True;
            }
            
            if (!bRet && aESel.nEndPara==0 && aESel.nEndPos==0 && aESel.nStartPara==(nParaAnz-1))
            {
                if(pTextEditOutliner->GetText(pLastPara).getLength() == aESel.nStartPos)
                    bRet = sal_True;
            }
        }
        else
        {
            bRet=sal_True;
        }
    }
    return bRet;
}

void SdrObjEditView::ImpMakeTextCursorAreaVisible()
{
    if (pTextEditOutlinerView!=NULL && pTextEditWin!=NULL) {
        Cursor* pCsr=pTextEditWin->GetCursor();
        if (pCsr!=NULL) {
            Size aSiz(pCsr->GetSize());
            if (aSiz.Width()!=0 && aSiz.Height()!=0) {
                MakeVisible(Rectangle(pCsr->GetPos(),aSiz),*pTextEditWin);
            }
        }
    }
}

sal_uInt16 SdrObjEditView::GetScriptType() const
{
    sal_uInt16 nScriptType = 0;

    if( IsTextEdit() )
    {
        if( mxTextEditObj->GetOutlinerParaObject() )
            nScriptType = mxTextEditObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();

        if( pTextEditOutlinerView )
            nScriptType = pTextEditOutlinerView->GetSelectedScriptType();
    }
    else
    {
        sal_uInt32 nMarkCount( GetMarkedObjectCount() );

        for( sal_uInt32 i = 0; i < nMarkCount; i++ )
        {
            OutlinerParaObject* pParaObj = GetMarkedObjectByIndex( i )->GetOutlinerParaObject();

            if( pParaObj )
            {
                nScriptType |= pParaObj->GetTextObject().GetScriptType();
            }
        }
    }

    if( nScriptType == 0 )
        nScriptType = SCRIPTTYPE_LATIN;

    return nScriptType;
}

sal_Bool SdrObjEditView::GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const
{
    if( mxSelectionController.is() )
        if( mxSelectionController->GetAttributes( rTargetSet, bOnlyHardAttr ) )
            return sal_True;

    if(IsTextEdit())
    {
        DBG_ASSERT(pTextEditOutlinerView!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutlinerView=NULL");
        DBG_ASSERT(pTextEditOutliner!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutliner=NULL");

        
        if(!bOnlyHardAttr && mxTextEditObj->GetStyleSheet())
            rTargetSet.Put(mxTextEditObj->GetStyleSheet()->GetItemSet());

        
        rTargetSet.Put( mxTextEditObj->GetMergedItemSet() );

        if( mxTextEditObj->GetOutlinerParaObject() )
            rTargetSet.Put( SvxScriptTypeItem( mxTextEditObj->GetOutlinerParaObject()->GetTextObject().GetScriptType() ) );

        if(pTextEditOutlinerView)
        {
            
            rTargetSet.Put(pTextEditOutlinerView->GetAttribs(), false);
            rTargetSet.Put( SvxScriptTypeItem( pTextEditOutlinerView->GetSelectedScriptType() ) );
        }

        if(GetMarkedObjectCount()==1 && GetMarkedObjectByIndex(0)==mxTextEditObj.get())
        {
            MergeNotPersistAttrFromMarked(rTargetSet, bOnlyHardAttr);
        }

        return sal_True;
    }
    else
    {
        return SdrGlueEditView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

sal_Bool SdrObjEditView::SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll)
{
    sal_Bool bRet=sal_False;
    bool bTextEdit=pTextEditOutlinerView!=NULL && mxTextEditObj.is();
    sal_Bool bAllTextSelected=ImpIsTextEditAllSelected();
    const SfxItemSet* pSet=&rSet;

    if (!bTextEdit)
    {
        
        if( mxSelectionController.is() )
            bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );

        if( !bRet )
        {
            bRet=SdrGlueEditView::SetAttributes(*pSet,bReplaceAll);
        }
    }
    else
    {
#ifdef DBG_UTIL
        {
            bool bHasEEFeatureItems=false;
            SfxItemIter aIter(rSet);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while (!bHasEEFeatureItems && pItem!=NULL)
            {
                if (!IsInvalidItem(pItem))
                {
                    sal_uInt16 nW=pItem->Which();
                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END)
                        bHasEEFeatureItems=true;
                }

                pItem=aIter.NextItem();
            }

            if(bHasEEFeatureItems)
            {
                OUString aMessage("SdrObjEditView::SetAttributes(): Setting EE_FEATURE items at the SdrView does not make sense! It only leads to overhead and unreadable documents.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif

        sal_Bool bOnlyEEItems;
        bool bNoEEItems=!SearchOutlinerItems(*pSet,bReplaceAll,&bOnlyEEItems);
        
        
        if (bAllTextSelected || bNoEEItems)
        {
            if( mxSelectionController.is() )
                bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );

            if( !bRet )
            {
                const bool bUndo = IsUndoEnabled();

                if( bUndo )
                {
                    OUString aStr;
                    ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);
                    BegUndo(aStr);
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));

                    
                    
                    
                    
                    
                    
                    bool bRescueText = dynamic_cast< SdrTextObj* >(mxTextEditObj.get());

                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get(),false,!bNoEEItems || bRescueText));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(*pSet, bReplaceAll);

                FlushComeBackTimer(); 
                bRet=sal_True;
            }
        }
        else if (!bOnlyEEItems)
        {
            
            
            
            sal_uInt16* pNewWhichTable=RemoveWhichRange(pSet->GetRanges(),EE_ITEMS_START,EE_ITEMS_END);
            SfxItemSet aSet(pMod->GetItemPool(),pNewWhichTable);
            delete[] pNewWhichTable;
            SfxWhichIter aIter(aSet);
            sal_uInt16 nWhich=aIter.FirstWhich();
            while (nWhich!=0)
            {
                const SfxPoolItem* pItem;
                SfxItemState eState=pSet->GetItemState(nWhich,false,&pItem);
                if (eState==SFX_ITEM_SET) aSet.Put(*pItem);
                nWhich=aIter.NextWhich();
            }


            if( mxSelectionController.is() )
                bRet=mxSelectionController->SetAttributes(aSet,bReplaceAll );

            if( !bRet )
            {
                if( IsUndoEnabled() )
                {
                    OUString aStr;
                    ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);
                    BegUndo(aStr);
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get(),false,false));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(aSet, bReplaceAll);

                if (GetMarkedObjectCount()==1 && GetMarkedObjectByIndex(0)==mxTextEditObj.get())
                {
                    SetNotPersistAttrToMarked(aSet,bReplaceAll);
                }
            }
            FlushComeBackTimer();
            bRet=sal_True;
        }
        if(!bNoEEItems)
        {
            
            if (bReplaceAll) {
                pTextEditOutlinerView->RemoveAttribs( sal_True );
            }
            pTextEditOutlinerView->SetAttribs(rSet);

#ifdef DBG_UTIL
            if (pItemBrowser!=NULL)
                pItemBrowser->SetDirty();
#endif

            ImpMakeTextCursorAreaVisible();
        }
        bRet=sal_True;
    }
    return bRet;
}

SfxStyleSheet* SdrObjEditView::GetStyleSheet() const
{
    SfxStyleSheet* pSheet = 0;

    if( mxSelectionController.is() )
    {
        if( mxSelectionController->GetStyleSheet( pSheet ) )
            return pSheet;
    }

    if ( pTextEditOutlinerView )
    {
        pSheet = pTextEditOutlinerView->GetStyleSheet();
    }
    else
    {
        pSheet = SdrGlueEditView::GetStyleSheet();
    }
    return pSheet;
}

sal_Bool SdrObjEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    if( mxSelectionController.is() )
    {
        if( mxSelectionController->SetStyleSheet( pStyleSheet, bDontRemoveHardAttr ) )
            return sal_True;
    }

    
    
    if( NULL != pTextEditOutlinerView )
    {
        Outliner* pOutliner = pTextEditOutlinerView->GetOutliner();

        const sal_Int32 nParaCount = pOutliner->GetParagraphCount();
        for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
        {
            pOutliner->SetStyleSheet( nPara, pStyleSheet );
        }
    }

    return SdrGlueEditView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
}



void SdrObjEditView::AddWindowToPaintView(OutputDevice* pNewWin)
{
    SdrGlueEditView::AddWindowToPaintView(pNewWin);

    if(mxTextEditObj.is() && !bTextEditOnlyOneView && pNewWin->GetOutDevType()==OUTDEV_WINDOW)
    {
        OutlinerView* pOutlView=ImpMakeOutlinerView((Window*)pNewWin,false,NULL);
        pTextEditOutliner->InsertView(pOutlView);
    }
}

void SdrObjEditView::DeleteWindowFromPaintView(OutputDevice* pOldWin)
{
    SdrGlueEditView::DeleteWindowFromPaintView(pOldWin);

    if(mxTextEditObj.is() && !bTextEditOnlyOneView && pOldWin->GetOutDevType()==OUTDEV_WINDOW)
    {
        for (sal_uIntPtr i=pTextEditOutliner->GetViewCount(); i>0;) {
            i--;
            OutlinerView* pOLV=pTextEditOutliner->GetView(i);
            if (pOLV && pOLV->GetWindow()==(Window*)pOldWin) {
                delete pTextEditOutliner->RemoveView(i);
            }
        }
    }
}

sal_Bool SdrObjEditView::IsTextEditInSelectionMode() const
{
    return pTextEditOutliner!=NULL && pTextEditOutliner->IsInSelectionMode();
}





sal_Bool SdrObjEditView::BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, Window* pWin)
{
    sal_Bool bRet=sal_False;
    BrkMacroObj();
    if (pObj!=NULL && pPV!=NULL && pWin!=NULL && pObj->HasMacro()) {
        nTol=ImpGetHitTolLogic(nTol,NULL);
        pMacroObj=pObj;
        pMacroPV=pPV;
        pMacroWin=pWin;
        bMacroDown=false;
        nMacroTol=sal_uInt16(nTol);
        aMacroDownPos=rPnt;
        MovMacroObj(rPnt);
    }
    return bRet;
}

void SdrObjEditView::ImpMacroUp(const Point& rUpPos)
{
    if (pMacroObj!=NULL && bMacroDown)
    {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rUpPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.pOut=pMacroWin;
        pMacroObj->PaintMacro(*pMacroWin,Rectangle(),aHitRec);
        bMacroDown=false;
    }
}

void SdrObjEditView::ImpMacroDown(const Point& rDownPos)
{
    if (pMacroObj!=NULL && !bMacroDown)
    {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rDownPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=true;
        aHitRec.pOut=pMacroWin;
        pMacroObj->PaintMacro(*pMacroWin,Rectangle(),aHitRec);
        bMacroDown=true;
    }
}

void SdrObjEditView::MovMacroObj(const Point& rPnt)
{
    if (pMacroObj!=NULL) {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rPnt;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=bMacroDown;
        aHitRec.pOut=pMacroWin;
        sal_Bool bDown=pMacroObj->IsMacroHit(aHitRec);
        if (bDown) ImpMacroDown(rPnt);
        else ImpMacroUp(rPnt);
    }
}

void SdrObjEditView::BrkMacroObj()
{
    if (pMacroObj!=NULL) {
        ImpMacroUp(aMacroDownPos);
        pMacroObj=NULL;
        pMacroPV=NULL;
        pMacroWin=NULL;
    }
}

sal_Bool SdrObjEditView::EndMacroObj()
{
    if (pMacroObj!=NULL && bMacroDown) {
        ImpMacroUp(aMacroDownPos);
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=aMacroDownPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=true;
        aHitRec.pOut=pMacroWin;
        bool bRet=pMacroObj->DoMacro(aHitRec);
        pMacroObj=NULL;
        pMacroPV=NULL;
        pMacroWin=NULL;
        return bRet;
    } else {
        BrkMacroObj();
        return sal_False;
    }
}

/** fills the given any with a XTextCursor for the current text selection.
    Leaves the any untouched if there currently is no text selected */
void SdrObjEditView::getTextSelection( ::com::sun::star::uno::Any& rSelection )
{
    if( IsTextEdit() )
    {
        OutlinerView* pOutlinerView = GetTextEditOutlinerView();
        if( pOutlinerView && pOutlinerView->HasSelection() )
        {
            SdrObject* pObj = GetTextEditObject();

            if( pObj )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xText( pObj->getUnoShape(), ::com::sun::star::uno::UNO_QUERY );
                if( xText.is() )
                {
                    SvxUnoTextBase* pRange = SvxUnoTextBase::getImplementation( xText );
                    if( pRange )
                    {
                        rSelection <<= pRange->createTextCursorBySelection( pOutlinerView->GetSelection() );
                    }
                }
            }
        }
    }
}

namespace sdr { namespace table {
extern rtl::Reference< sdr::SelectionController > CreateTableController( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController );
} }

/* check if we have a single selection and that single object likes
    to handle the mouse and keyboard events itself

    TODO: the selection controller should be queried from the
    object specific view contact. Currently this method only
    works for tables.
*/
void SdrObjEditView::MarkListHasChanged()
{
    SdrGlueEditView::MarkListHasChanged();

    if( mxSelectionController.is() )
    {
        mxLastSelectionController = mxSelectionController;
        mxSelectionController->onSelectionHasChanged();
    }

    mxSelectionController.clear();

    const SdrMarkList& rMarkList=GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj= rMarkList.GetMark(0)->GetMarkedSdrObj();
        
        if( pObj && (pObj->GetObjInventor() == SdrInventor ) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
        {
            mxSelectionController = sdr::table::CreateTableController( this, pObj, mxLastSelectionController );
            if( mxSelectionController.is() )
            {
                mxLastSelectionController.clear();
                mxSelectionController->onSelectionHasChanged();
            }
        }
    }
}

IMPL_LINK( SdrObjEditView, EndPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    OnEndPasteOrDrop( pInfos );
    return 0;
}

IMPL_LINK( SdrObjEditView, BeginPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    OnBeginPasteOrDrop( pInfos );
    return 0;
}

void SdrObjEditView::OnBeginPasteOrDrop( PasteOrDropInfos* )
{
    
}

void SdrObjEditView::OnEndPasteOrDrop( PasteOrDropInfos* )
{
    
}

sal_uInt16 SdrObjEditView::GetSelectionLevel() const
{
    sal_uInt16 nLevel = 0xFFFF;
    if( IsTextEdit() )
    {
        DBG_ASSERT(pTextEditOutlinerView!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutlinerView=NULL");
        DBG_ASSERT(pTextEditOutliner!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutliner=NULL");
        if( pTextEditOutlinerView )
        {
            
            ESelection aSelect = pTextEditOutlinerView->GetSelection();
            sal_uInt16 nStartPara = ::std::min( aSelect.nStartPara, aSelect.nEndPara );
            sal_uInt16 nEndPara = ::std::max( aSelect.nStartPara, aSelect.nEndPara );
            
            nLevel = 0;
            for( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                sal_uInt16 nParaDepth = 1 << pTextEditOutliner->GetDepth( nPara );
                if( !(nLevel & nParaDepth) )
                    nLevel += nParaDepth;
            }
            
            
            
            
            if( nLevel == 0)
                nLevel = 0xFFFF;
        }
    }
    return nLevel;
}

bool SdrObjEditView::SupportsFormatPaintbrush( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const
{
    if( nObjectInventor != SdrInventor && nObjectInventor != E3dInventor )
        return false;
    switch(nObjectIdentifier)
    {
        case OBJ_NONE:
        case OBJ_GRUP:
            return false;
        case OBJ_LINE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_TEXT:
        case OBJ_TEXTEXT:
        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
        case OBJ_GRAF:
        case OBJ_OLE2:
        case OBJ_TABLE:
            return true;
        case OBJ_EDGE:
        case OBJ_CAPTION:
            return false;
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
            return true;
        case OBJ_PAGE:
        case OBJ_MEASURE:
        case OBJ_DUMMY:
        case OBJ_FRAME:
        case OBJ_UNO:
            return false;
        case OBJ_CUSTOMSHAPE:
            return true;
        default:
            return false;
    }
}

static const sal_uInt16* GetFormatRangeImpl( bool bTextOnly )
{
    static const sal_uInt16 gRanges[] = {
        SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
        SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
        SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,
        XATTR_LINE_FIRST, XATTR_LINE_LAST,
        XATTR_FILL_FIRST, XATTRSET_FILL,
        EE_PARA_START, EE_PARA_END,
        EE_CHAR_START, EE_CHAR_END,
        0,0
    };
    return &gRanges[ bTextOnly ? 10 : 0];
}

bool SdrObjEditView::TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  )
{
    if( mxSelectionController.is() && mxSelectionController->TakeFormatPaintBrush(rFormatSet) )
        return true;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() >= 1 )
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();

        rFormatSet.reset( new SfxItemSet( GetModel()->GetItemPool(), GetFormatRangeImpl( pOLV != NULL ) ) );
        if( pOLV )
        {
            rFormatSet->Put( pOLV->GetAttribs() );
        }
        else
        {
            const sal_Bool bOnlyHardAttr = sal_False;
            rFormatSet->Put( GetAttrFromMarked(bOnlyHardAttr) );
        }
        return true;
    }

    return false;
}

static SfxItemSet CreatePaintSet( const sal_uInt16 *pRanges, SfxItemPool& rPool, const SfxItemSet& rSourceSet, const SfxItemSet& rTargetSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    SfxItemSet aPaintSet( rPool, pRanges );

    while( *pRanges )
    {
        sal_uInt16 nWhich = *pRanges++;
        const sal_uInt16 nLastWhich = *pRanges++;

        if( bNoCharacterFormats && (nWhich == EE_CHAR_START) )
            continue;

        if( bNoParagraphFormats && (nWhich == EE_PARA_START ) )
            continue;

        for( ; nWhich < nLastWhich; nWhich++ )
        {
            const SfxPoolItem* pSourceItem = rSourceSet.GetItem( nWhich );
            const SfxPoolItem* pTargetItem = rTargetSet.GetItem( nWhich );

            if( (pSourceItem && !pTargetItem) || (pSourceItem && pTargetItem && !((*pSourceItem) == (*pTargetItem)) ) )
            {
                aPaintSet.Put( *pSourceItem );
            }
        }
    }
    return aPaintSet;
}

void SdrObjEditView::ApplyFormatPaintBrushToText( SfxItemSet& rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
    if(pParaObj)
    {
        SdrOutliner& rOutliner = rTextObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        sal_Int32 nParaCount(rOutliner.GetParagraphCount());

        if(nParaCount)
        {
            for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
            {
                if( !bNoCharacterFormats )
                    rOutliner.QuickRemoveCharAttribs( nPara, /* remove all */0 );

                SfxItemSet aSet(rOutliner.GetParaAttribs(nPara));
                aSet.Put(CreatePaintSet( GetFormatRangeImpl(true), *aSet.GetPool(), rFormatSet, aSet, bNoCharacterFormats, bNoParagraphFormats ) );
                rOutliner.SetParaAttribs(nPara, aSet);
            }

            OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, nParaCount);
            rOutliner.Clear();

            rTextObj.NbcSetOutlinerParaObjectForText(pTemp,pText);
        }
    }
}

void SdrObjEditView::ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    if( !mxSelectionController.is() || !mxSelectionController->ApplyFormatPaintBrush( rFormatSet, bNoCharacterFormats, bNoParagraphFormats ) )
    {
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        OutlinerView* pOLV = GetTextEditOutlinerView();

        const SfxItemSet& rShapeSet = pObj->GetMergedItemSet();

        if( !pOLV )
        {
            
            
            
            

            const sal_uInt16* pRanges = rFormatSet.GetRanges();
            bool bTextOnly = true;

            while( *pRanges )
            {
                if( (*pRanges != EE_PARA_START) && (*pRanges != EE_CHAR_START) )
                {
                    bTextOnly = false;
                    break;
                }
                pRanges += 2;
            }

            if( !bTextOnly )
            {
                SfxItemSet aPaintSet( CreatePaintSet( GetFormatRangeImpl(false), *rShapeSet.GetPool(), rFormatSet, rShapeSet, bNoCharacterFormats, bNoParagraphFormats ) );
                const sal_Bool bReplaceAll = sal_False;
                SetAttrToMarked(aPaintSet, bReplaceAll);
            }

            
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObj);
            if( pTextObj )
            {
                sal_Int32 nText = pTextObj->getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = pTextObj->getText( nText );
                    ApplyFormatPaintBrushToText( rFormatSet, *pTextObj, pText, bNoCharacterFormats, bNoParagraphFormats );
                }
            }
        }
        else
        {
            ::Outliner* pOutliner = pOLV->GetOutliner();
            if( pOutliner )
            {
                const EditEngine& rEditEngine = pOutliner->GetEditEngine();

                ESelection aSel( pOLV->GetSelection() );
                if( !aSel.HasRange() )
                    pOLV->SetSelection( rEditEngine.GetWord( aSel, com::sun::star::i18n::WordType::DICTIONARY_WORD ) );

                const sal_Bool bRemoveParaAttribs = !bNoParagraphFormats;
                pOLV->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
                SfxItemSet aSet( pOLV->GetAttribs() );
                SfxItemSet aPaintSet( CreatePaintSet(GetFormatRangeImpl(true), *aSet.GetPool(), rFormatSet, aSet, bNoCharacterFormats, bNoParagraphFormats ) );
                pOLV->SetAttribs( aPaintSet );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
