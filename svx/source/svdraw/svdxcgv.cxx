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

#include <vector>
#include <unordered_set>
#include <editeng/editdata.hxx>
#include <rtl/strbuf.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <tools/bigint.hxx>
#include <clonelist.hxx>
#include <vcl/virdev.hxx>
#include <svl/style.hxx>
#include <fmobj.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/converters.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdotable.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

SdrExchangeView::SdrExchangeView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrObjEditView(rSdrModel, pOut)
{
}

bool SdrExchangeView::ImpLimitToWorkArea(Point& rPt) const
{
    bool bRet(false);

    if(!maMaxWorkArea.IsEmpty())
    {
        if(rPt.X()<maMaxWorkArea.Left())
        {
            rPt.setX( maMaxWorkArea.Left() );
            bRet = true;
        }

        if(rPt.X()>maMaxWorkArea.Right())
        {
            rPt.setX( maMaxWorkArea.Right() );
            bRet = true;
        }

        if(rPt.Y()<maMaxWorkArea.Top())
        {
            rPt.setY( maMaxWorkArea.Top() );
            bRet = true;
        }

        if(rPt.Y()>maMaxWorkArea.Bottom())
        {
            rPt.setY( maMaxWorkArea.Bottom() );
            bRet = true;
        }
    }
    return bRet;
}

void SdrExchangeView::ImpGetPasteObjList(Point& /*rPos*/, SdrObjList*& rpLst)
{
    if (rpLst==nullptr)
    {
        SdrPageView* pPV = GetSdrPageView();

        if (pPV!=nullptr) {
            rpLst=pPV->GetObjList();
        }
    }
}

bool SdrExchangeView::ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const
{
    bool bRet=false;
    rLayer=SdrLayerID(0);
    if (pObjList!=nullptr) {
        const SdrPage* pPg=pObjList->getSdrPageFromSdrObjList();
        if (pPg!=nullptr) {
            rLayer=pPg->GetLayerAdmin().GetLayerID(maActualLayer);
            if (rLayer==SDRLAYER_NOTFOUND) rLayer=SdrLayerID(0);
            SdrPageView* pPV = GetSdrPageView();
            if (pPV!=nullptr) {
                bRet=!pPV->GetLockedLayers().IsSet(rLayer) && pPV->GetVisibleLayers().IsSet(rLayer);
            }
        }
    }
    return bRet;
}

bool SdrExchangeView::Paste(const OUString& rStr, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions)
{
    if (rStr.isEmpty())
        return false;

    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==nullptr) return false;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return false;
    bool bUnmark = (nOptions & (SdrInsertFlags::DONTMARK|SdrInsertFlags::ADDMARK))==SdrInsertFlags::NONE && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    tools::Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->getSdrPageFromSdrObjList();
    if (pPage!=nullptr) {
        aTextRect.SetSize(pPage->GetSize());
    }
    rtl::Reference<SdrRectObj> pObj = new SdrRectObj(
        getSdrModelFromSdrView(), aTextRect, SdrObjKind::Text);

    pObj->SetLayer(nLayer);
    pObj->NbcSetText(rStr); // SetText before SetAttr, else SetAttr doesn't work!
    if (mpDefaultStyleSheet!=nullptr) pObj->NbcSetStyleSheet(mpDefaultStyleSheet, false);

    pObj->SetMergedItemSet(maDefaultAttr);

    SfxItemSet aTempAttr(GetModel().GetItemPool());  // no fill, no line
    aTempAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
    aTempAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap = GetModel().GetScaleUnit();
    ImpPasteObject(pObj.get(), *pLst, aPos, aSiz, MapMode(eMap), nOptions);
    return true;
}

bool SdrExchangeView::Paste(SvStream& rInput, EETextFormat eFormat, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==nullptr) return false;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return false;
    bool bUnmark=(nOptions&(SdrInsertFlags::DONTMARK|SdrInsertFlags::ADDMARK))==SdrInsertFlags::NONE && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    tools::Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->getSdrPageFromSdrObjList();
    if (pPage!=nullptr) {
        aTextRect.SetSize(pPage->GetSize());
    }
    rtl::Reference<SdrRectObj> pObj = new SdrRectObj(
        getSdrModelFromSdrView(), aTextRect, SdrObjKind::Text);

    pObj->SetLayer(nLayer);
    if (mpDefaultStyleSheet!=nullptr) pObj->NbcSetStyleSheet(mpDefaultStyleSheet, false);

    pObj->SetMergedItemSet(maDefaultAttr);

    SfxItemSet aTempAttr(GetModel().GetItemPool());  // no fill, no line
    aTempAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
    aTempAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->NbcSetText(rInput,OUString(),eFormat);
    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap = GetModel().GetScaleUnit();
    ImpPasteObject(pObj.get(), *pLst, aPos, aSiz, MapMode(eMap), nOptions);

    // b4967543
    if(pObj->GetOutlinerParaObject())
    {
        SdrOutliner& rOutliner = pObj->getSdrModelFromSdrObject().GetHitTestOutliner();
        rOutliner.SetText(*pObj->GetOutlinerParaObject());

        if(1 == rOutliner.GetParagraphCount())
        {
            SfxStyleSheet* pCandidate = rOutliner.GetStyleSheet(0);

            if(pCandidate)
            {
                if(pObj->getSdrModelFromSdrObject().GetStyleSheetPool() == pCandidate->GetPool())
                {
                    pObj->NbcSetStyleSheet(pCandidate, true);
                }
            }
        }
    }

    return true;
}

bool SdrExchangeView::Paste(
    const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions)
{
    const SdrModel* pSrcMod=&rMod;
    if (pSrcMod == &GetModel())
        return false; // this can't work, right?

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SvxResId(STR_ExchangePaste));

    if( mxSelectionController.is() && mxSelectionController->PasteObjModel( rMod ) )
    {
        if( bUndo )
            EndUndo();
        return true;
    }

    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    SdrPageView* pMarkPV=nullptr;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV && pPV->GetObjList() == pLst )
        pMarkPV=pPV;

    ImpLimitToWorkArea( aPos );
    if (pLst==nullptr)
        return false;

    bool bUnmark=(nOptions&(SdrInsertFlags::DONTMARK|SdrInsertFlags::ADDMARK))==SdrInsertFlags::NONE && !IsTextEdit();
    if (bUnmark)
        UnmarkAllObj();

    // Rescale, if the Model uses a different MapUnit.
    // Calculate the necessary factors first.
    MapUnit eSrcUnit = pSrcMod->GetScaleUnit();
    MapUnit eDstUnit = GetModel().GetScaleUnit();
    bool bResize=eSrcUnit!=eDstUnit;
    Fraction aXResize,aYResize;
    Point aPt0;
    if (bResize)
    {
        FrPair aResize(GetMapFactor(eSrcUnit,eDstUnit));
        aXResize=aResize.X();
        aYResize=aResize.Y();
    }
    SdrObjList*  pDstLst=pLst;
    sal_uInt16 nPg,nPgCount=pSrcMod->GetPageCount();
    for (nPg=0; nPg<nPgCount; nPg++)
    {
        const SdrPage* pSrcPg=pSrcMod->GetPage(nPg);

        // Use SnapRect, not BoundRect here
        tools::Rectangle aR=pSrcPg->GetAllObjSnapRect();

        if (bResize)
            ResizeRect(aR,aPt0,aXResize,aYResize);
        Point aDist(aPos-aR.Center());
        Size  aSiz(aDist.X(),aDist.Y());
        size_t nCloneErrCnt = 0;
        const size_t nObjCount = pSrcPg->GetObjCount();
        bool bMark = pMarkPV!=nullptr && !IsTextEdit() && (nOptions&SdrInsertFlags::DONTMARK)==SdrInsertFlags::NONE;

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;
        std::unordered_set<rtl::OUString> aNameSet;
        for (size_t nOb=0; nOb<nObjCount; ++nOb)
        {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

            rtl::Reference<SdrObject> pNewObj(pSrcOb->CloneSdrObject(GetModel()));

            if (pNewObj!=nullptr)
            {
                if(bResize)
                {
                    pNewObj->getSdrModelFromSdrObject().SetPasteResize(true);
                    pNewObj->NbcResize(aPt0,aXResize,aYResize);
                    pNewObj->getSdrModelFromSdrObject().SetPasteResize(false);
                }

                // #i39861#
                pNewObj->NbcMove(aSiz);

                const SdrPage* pPg = pDstLst->getSdrPageFromSdrObjList();

                if(pPg)
                {
                    // #i72535#
                    const SdrLayerAdmin& rAd = pPg->GetLayerAdmin();
                    SdrLayerID nLayer(0);

                    if(dynamic_cast<const FmFormObj*>( pNewObj.get()) !=  nullptr)
                    {
                        // for FormControls, force to form layer
                        nLayer = rAd.GetLayerID(rAd.GetControlLayerName());
                    }
                    else
                    {
                        nLayer = rAd.GetLayerID(maActualLayer);
                    }

                    if(SDRLAYER_NOTFOUND == nLayer)
                    {
                        nLayer = SdrLayerID(0);
                    }

                    pNewObj->SetLayer(nLayer);
                }

                pDstLst->InsertObjectThenMakeNameUnique(pNewObj.get(), aNameSet);

                if( bUndo )
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pNewObj));

                if (bMark) {
                    // Don't already set Markhandles!
                    // That is instead being done by ModelHasChanged in MarkView.
                    MarkObj(pNewObj.get(),pMarkPV,false,true);
                }

                // #i13033#
                aCloneList.AddPair(pSrcOb, pNewObj.get());
            }
            else
            {
                nCloneErrCnt++;
            }
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();

        if(0 != nCloneErrCnt)
        {
#ifdef DBG_UTIL
            OStringBuffer aStr("SdrExchangeView::Paste(): Error when cloning ");

            if(nCloneErrCnt == 1)
            {
                aStr.append("a drawing object.");
            }
            else
            {
                aStr.append(OString::number(static_cast<sal_Int32>(nCloneErrCnt))
                    + " drawing objects.");
            }

            aStr.append(" Not copying object connectors.");

            OSL_FAIL(aStr.getStr());
#endif
        }
    }

    if( bUndo )
        EndUndo();

    return true;
}

void SdrExchangeView::ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const Point& rCenter, const Size& rSiz, const MapMode& rMap, SdrInsertFlags nOptions)
{
    BigInt nSizX(rSiz.Width());
    BigInt nSizY(rSiz.Height());
    MapUnit eSrcMU=rMap.GetMapUnit();
    MapUnit eDstMU = GetModel().GetScaleUnit();
    FrPair aMapFact(GetMapFactor(eSrcMU,eDstMU));
    nSizX *= double(aMapFact.X() * rMap.GetScaleX());
    nSizY *= double(aMapFact.Y() * rMap.GetScaleY());
    tools::Long xs=nSizX;
    tools::Long ys=nSizY;
    // set the pos to 0, 0 for online case
    bool isLOK = comphelper::LibreOfficeKit::isActive();
    Point aPos(isLOK ? 0 : rCenter.X()-xs/2, isLOK ? 0 : rCenter.Y()-ys/2);
    tools::Rectangle aR(aPos.X(),aPos.Y(),aPos.X()+xs,aPos.Y()+ys);
    pObj->SetLogicRect(aR);
    rLst.InsertObject(pObj, SAL_MAX_SIZE);

    if( IsUndoEnabled() )
        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pObj));

    SdrPageView* pMarkPV=nullptr;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV && pPV->GetObjList()==&rLst)
        pMarkPV=pPV;

    bool bMark = pMarkPV!=nullptr && !IsTextEdit() && (nOptions&SdrInsertFlags::DONTMARK)==SdrInsertFlags::NONE;
    if (bMark)
    { // select object the first PageView we found
        MarkObj(pObj,pMarkPV);
    }
}

Bitmap SdrExchangeView::GetMarkedObjBitmap(bool bNoVDevIfOneBmpMarked, const sal_uInt32 nMaximumQuadraticPixels, const std::optional<Size>& rTargetDPI) const
{
    Bitmap aBmp;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if(1 == rMarkList.GetMarkCount())
    {
        if (auto pGrafObj
            = dynamic_cast<const SdrGrafObj*>(rMarkList.GetMark(0)->GetMarkedSdrObj()))
        {
            if(bNoVDevIfOneBmpMarked)
            {
                if (pGrafObj->GetGraphicType() == GraphicType::Bitmap)
                    aBmp = Bitmap(pGrafObj->GetTransformedGraphic().GetBitmapEx());
            }
            else
            {
                if (pGrafObj->isEmbeddedVectorGraphicData())
                    aBmp = Bitmap(pGrafObj->GetGraphic().getVectorGraphicData()->getReplacement());
            }
        }
    }

    if (aBmp.IsEmpty() && rMarkList.GetMarkCount() != 0)
    {
        // choose conversion directly using primitives to bitmap to avoid
        // rendering errors with tiled bitmap fills (these will be tiled in a
        // in-between metafile, but tend to show 'gaps' since the target is *no*
        // bitmap rendering)
        ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());
        const size_t nCount(aSdrObjects.size());

        // collect sub-primitives as group objects, thus no expensive append
        // to existing sequence is needed
        drawinglayer::primitive2d::Primitive2DContainer xPrimitives(nCount);

        for (size_t a(0); a < nCount; a++)
        {
            const SdrObject* pCandidate = aSdrObjects[a];

            if (auto pSdrGrafObj = dynamic_cast<const SdrGrafObj*>(pCandidate))
            {
                // #122753# To ensure existence of graphic content, force swap in
                pSdrGrafObj->ForceSwapIn();
            }

            drawinglayer::primitive2d::Primitive2DContainer xRetval;
            pCandidate->GetViewContact().getViewIndependentPrimitive2DContainer(xRetval);
            xPrimitives[a] = new drawinglayer::primitive2d::GroupPrimitive2D(
                std::move(xRetval));
        }

        // get logic range
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        const basegfx::B2DRange aRange(xPrimitives.getB2DRange(aViewInformation2D));

        if(!aRange.isEmpty())
        {
            o3tl::Length eRangeUnit = o3tl::Length::mm100;

            if (GetModel().IsWriter())
            {
                eRangeUnit = o3tl::Length::twip;
            }

            // if we have geometry and it has a range, convert to BitmapEx using
            // common tooling
            aBmp = drawinglayer::convertPrimitive2DContainerToBitmap(
                std::move(xPrimitives),
                aRange,
                nMaximumQuadraticPixels,
                eRangeUnit,
                rTargetDPI);
        }
    }

    return aBmp;
}


GDIMetaFile SdrExchangeView::GetMarkedObjMetaFile(bool bNoVDevIfOneMtfMarked) const
{
    GDIMetaFile aMtf;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() != 0 )
    {
        tools::Rectangle   aBound( GetMarkedObjBoundRect() );
        Size        aBoundSize( aBound.GetWidth(), aBound.GetHeight() );
        MapMode aMap(GetModel().GetScaleUnit());

        if (bNoVDevIfOneMtfMarked && rMarkList.GetMarkCount() == 1)
        {
            if (auto pGrafObj
                = dynamic_cast<const SdrGrafObj*>(rMarkList.GetMark(0)->GetMarkedSdrObj()))
            {
                Graphic aGraphic( pGrafObj->GetTransformedGraphic() );

                // #119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                aMtf = aGraphic.GetGDIMetaFile();
            }
        }

        if( !aMtf.GetActionSize() )
        {
            ScopedVclPtrInstance< VirtualDevice > pOut;
            const Size aDummySize(2, 2);

            pOut->SetOutputSizePixel(aDummySize);
            pOut->EnableOutput(false);
            pOut->SetMapMode(aMap);
            aMtf.Clear();
            aMtf.Record(pOut);

            DrawMarkedObj(*pOut);

            aMtf.Stop();
            aMtf.WindStart();

            // moving the result is more reliable then setting a relative MapMode at the VDev (used
            // before), also see #i99268# in GetObjGraphic() below. Some draw actions at
            // the OutDev are simply not handled correctly when a MapMode is set at the
            // target device, e.g. MetaFloatTransparentAction. Even the Move for this action
            // was missing the manipulation of the embedded Metafile
            aMtf.Move(-aBound.Left(), -aBound.Top());

            aMtf.SetPrefMapMode( aMap );

            // removed PrefSize extension. It is principally wrong to set a reduced size at
            // the created MetaFile. The mentioned errors occur at output time since the integer
            // MapModes from VCL lead to errors. It is now corrected in the VCLRenderer for
            // primitives (and may later be done in breaking up a MetaFile to primitives)
            aMtf.SetPrefSize(aBoundSize);
        }
    }

    return aMtf;
}


Graphic SdrExchangeView::GetAllMarkedGraphic() const
{
    Graphic aRet;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() != 0 )
    {
        if( ( 1 == rMarkList.GetMarkCount() ) && rMarkList.GetMark( 0 ) )
            aRet = SdrExchangeView::GetObjGraphic(*rMarkList.GetMark(0)->GetMarkedSdrObj());
        else
            aRet = GetMarkedObjMetaFile();
    }

    return aRet;
}


// tdf#155479 bSVG: need to know it's SVG export, default is false
Graphic SdrExchangeView::GetObjGraphic(const SdrObject& rSdrObject, bool bSVG)
{
    Graphic aRet;

    if (!rSdrObject.HasText())
    {
        // try to get a graphic from the object first
        const SdrGrafObj* pSdrGrafObj(dynamic_cast<const SdrGrafObj*>(&rSdrObject));
        const SdrOle2Obj* pSdrOle2Obj(dynamic_cast<const SdrOle2Obj*>(&rSdrObject));

        if (pSdrGrafObj)
        {
            if (pSdrGrafObj->isEmbeddedVectorGraphicData())
            {
                // get Metafile for Svg content
                aRet = pSdrGrafObj->getMetafileFromEmbeddedVectorGraphicData();
            }
            else
            {
                // Make behaviour coherent with metafile
                // recording below (which of course also takes
                // view-transformed objects)
                aRet = pSdrGrafObj->GetTransformedGraphic();
            }
        }
        else if (pSdrOle2Obj)
        {
            if (const Graphic* pGraphic = pSdrOle2Obj->GetGraphic())
            {
                aRet = *pGraphic;
            }
        }
        else
        {
            // Support extracting a snapshot from video media, if possible.
            const SdrMediaObj* pSdrMediaObj = dynamic_cast<const SdrMediaObj*>(&rSdrObject);
            if (pSdrMediaObj)
            {
                const css::uno::Reference<css::graphic::XGraphic>& xGraphic
                    = pSdrMediaObj->getSnapshot();
                if (xGraphic.is())
                    aRet = Graphic(xGraphic);
            }
        }
    }

    // if graphic could not be retrieved => go the hard way and create a MetaFile
    if((GraphicType::NONE == aRet.GetType()) || (GraphicType::Default == aRet.GetType()))
    {
        ScopedVclPtrInstance< VirtualDevice > pOut;
        GDIMetaFile aMtf;
        const tools::Rectangle aBoundRect(rSdrObject.GetCurrentBoundRect());
        const MapMode aMap(rSdrObject.getSdrModelFromSdrObject().GetScaleUnit());

        pOut->EnableOutput(false);
        pOut->SetMapMode(aMap);
        aMtf.Record(pOut);
        aMtf.setSVG(bSVG);
        rSdrObject.SingleObjectPainter(*pOut);
        aMtf.Stop();
        aMtf.WindStart();

        // #i99268# replace the original offset from using XOutDev's SetOffset
        // NOT (as tried with #i92760#) with another MapMode which gets recorded
        // by the Metafile itself (what always leads to problems), but by
        // moving the result directly
        aMtf.Move(-aBoundRect.Left(), -aBoundRect.Top());
        aMtf.SetPrefMapMode(aMap);
        aMtf.SetPrefSize(aBoundRect.GetSize());

        if(aMtf.GetActionSize())
        {
            aRet = aMtf;
        }
    }

    return aRet;
}


::std::vector< SdrObject* > SdrExchangeView::GetMarkedObjects() const
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    ::std::vector< SdrObject* > aRetval;

    ::std::vector< ::std::vector< SdrMark* > >  aObjVectors( 2 );
    ::std::vector< SdrMark* >&                  rObjVector1 = aObjVectors[ 0 ];
    ::std::vector< SdrMark* >&                  rObjVector2 = aObjVectors[ 1 ];
    const SdrLayerAdmin& rLayerAdmin = GetModel().GetLayerAdmin();
    const SdrLayerID                            nControlLayerId = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName() );

    for( size_t n = 0, nCount = rMarkList.GetMarkCount(); n < nCount; ++n )
    {
        SdrMark* pMark = rMarkList.GetMark( n );

        // paint objects on control layer on top of all other objects
        if( nControlLayerId == pMark->GetMarkedSdrObj()->GetLayer() )
            rObjVector2.push_back( pMark );
        else
            rObjVector1.push_back( pMark );
    }

    for(const std::vector<SdrMark*> & rObjVector : aObjVectors)
    {
        for(SdrMark* pMark : rObjVector)
        {
            aRetval.push_back(pMark->GetMarkedSdrObj());
        }
    }

    return aRetval;
}


void SdrExchangeView::DrawMarkedObj(OutputDevice& rOut) const
{
    ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());

    if(!aSdrObjects.empty())
    {
        SdrPage* pPage = aSdrObjects[0]->getSdrPageFromSdrObject();
        sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, std::move(aSdrObjects), pPage);
        sdr::contact::DisplayInfo aDisplayInfo;

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);
    }
}

std::unique_ptr<SdrModel> SdrExchangeView::CreateMarkedObjModel() const
{
    // Sorting the MarkList here might be problematic in the future, so
    // use a copy.
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    std::unique_ptr<SdrModel> pNewModel(GetModel().AllocModel());
    rtl::Reference<SdrPage> pNewPage = pNewModel->AllocPage(false);
    pNewModel->InsertPage(pNewPage.get());
    ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());

    // #i13033#
    // New mechanism to re-create the connections of cloned connectors
    CloneList aCloneList;

    for(SdrObject* pObj : aSdrObjects)
    {
        assert(pObj);

        rtl::Reference<SdrObject> pNewObj;

        if(nullptr != dynamic_cast< const SdrPageObj* >(pObj))
        {
            // convert SdrPageObj's to a graphic representation, because
            // virtual connection to referenced page gets lost in new model
            pNewObj = new SdrGrafObj(
                *pNewModel,
                GetObjGraphic(*pObj),
                pObj->GetLogicRect());
        }
        else if(nullptr != dynamic_cast< const sdr::table::SdrTableObj* >(pObj))
        {
            // check if we have a valid selection *different* from whole table
            // being selected
            if(mxSelectionController.is())
            {
                pNewObj = mxSelectionController->GetMarkedSdrObjClone(*pNewModel);
            }
        }

        if(!pNewObj)
        {
            // not cloned yet
            if(pObj->GetObjIdentifier() == SdrObjKind::OLE2 && nullptr == GetModel().GetPersist())
            {
                // tdf#125520 - former fix was wrong, the SdrModel
                // has to have a GetPersist() already, see task.
                // We can still warn here when this is not the case
                SAL_WARN( "svx", "OLE gets cloned Persist, EmbeddedObjectContainer will not be copied" );
            }

            // use default way
            pNewObj = pObj->CloneSdrObject(*pNewModel);
        }

        if(pNewObj)
        {
            pNewPage->InsertObject(pNewObj.get(), SAL_MAX_SIZE);

            // #i13033#
            aCloneList.AddPair(pObj, pNewObj.get());
        }
    }

    // #i13033#
    // New mechanism to re-create the connections of cloned connectors
    aCloneList.CopyConnections();

    return pNewModel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
