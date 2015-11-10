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
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <rtl/strbuf.hxx>
#include <svx/xexch.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx>
#include "svx/svdstr.hrc"
#include "svdglob.hxx"
#include "svx/xoutbmp.hxx"
#include <vcl/metaact.hxx>
#include <svl/poolitem.hxx>
#include <svl/itempool.hxx>
#include <tools/bigint.hxx>
#include <sot/formats.hxx>
#include <clonelist.hxx>
#include <vcl/virdev.hxx>
#include <svl/style.hxx>
#include <fmobj.hxx>
#include <vcl/svgdata.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

using namespace com::sun::star;

SdrExchangeView::SdrExchangeView(SdrModel* pModel1, OutputDevice* pOut):
    SdrObjEditView(pModel1,pOut)
{
}

bool SdrExchangeView::ImpLimitToWorkArea(Point& rPt) const
{
    bool bRet(false);

    if(!maMaxWorkArea.IsEmpty())
    {
        if(rPt.X()<maMaxWorkArea.Left())
        {
            rPt.X() = maMaxWorkArea.Left();
            bRet = true;
        }

        if(rPt.X()>maMaxWorkArea.Right())
        {
            rPt.X() = maMaxWorkArea.Right();
            bRet = true;
        }

        if(rPt.Y()<maMaxWorkArea.Top())
        {
            rPt.Y() = maMaxWorkArea.Top();
            bRet = true;
        }

        if(rPt.Y()>maMaxWorkArea.Bottom())
        {
            rPt.Y() = maMaxWorkArea.Bottom();
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
    rLayer=0;
    if (pObjList!=nullptr) {
        const SdrPage* pPg=pObjList->GetPage();
        if (pPg!=nullptr) {
            rLayer=pPg->GetLayerAdmin().GetLayerID(maActualLayer,true);
            if (rLayer==SDRLAYER_NOTFOUND) rLayer=0;
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
    Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->GetPage();
    if (pPage!=nullptr) {
        aTextRect.SetSize(pPage->GetSize());
    }
    SdrRectObj* pObj=new SdrRectObj(OBJ_TEXT,aTextRect);
    pObj->SetModel(mpModel);
    pObj->SetLayer(nLayer);
    pObj->NbcSetText(rStr); // SetText before SetAttr, else SetAttr doesn't work!
    if (mpDefaultStyleSheet!=nullptr) pObj->NbcSetStyleSheet(mpDefaultStyleSheet, false);

    pObj->SetMergedItemSet(maDefaultAttr);

    SfxItemSet aTempAttr(mpModel->GetItemPool());  // no fill, no line
    aTempAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
    aTempAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=mpModel->GetScaleUnit();
    Fraction aMap=mpModel->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);
    return true;
}

bool SdrExchangeView::Paste(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==nullptr) return false;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return false;
    bool bUnmark=(nOptions&(SdrInsertFlags::DONTMARK|SdrInsertFlags::ADDMARK))==SdrInsertFlags::NONE && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->GetPage();
    if (pPage!=nullptr) {
        aTextRect.SetSize(pPage->GetSize());
    }
    SdrRectObj* pObj=new SdrRectObj(OBJ_TEXT,aTextRect);
    pObj->SetModel(mpModel);
    pObj->SetLayer(nLayer);
    if (mpDefaultStyleSheet!=nullptr) pObj->NbcSetStyleSheet(mpDefaultStyleSheet, false);

    pObj->SetMergedItemSet(maDefaultAttr);

    SfxItemSet aTempAttr(mpModel->GetItemPool());  // no fill, no line
    aTempAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
    aTempAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->NbcSetText(rInput,rBaseURL,eFormat);
    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=mpModel->GetScaleUnit();
    Fraction aMap=mpModel->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);

    // b4967543
    if(pObj->GetModel() && pObj->GetOutlinerParaObject())
    {
        SdrOutliner& rOutliner = pObj->GetModel()->GetHitTestOutliner();
        rOutliner.SetText(*pObj->GetOutlinerParaObject());

        if(1L == rOutliner.GetParagraphCount())
        {
            SfxStyleSheet* pCandidate = rOutliner.GetStyleSheet(0L);

            if(pCandidate)
            {
                if(pObj->GetModel()->GetStyleSheetPool() == &pCandidate->GetPool())
                {
                    pObj->NbcSetStyleSheet(pCandidate, true);
                }
            }
        }
    }

    return true;
}

bool SdrExchangeView::Paste(
    const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions,
    const OUString& rSrcShellID, const OUString& rDestShellID )
{
    const SdrModel* pSrcMod=&rMod;
    if (pSrcMod==mpModel)
        return false; // this can't work, right?

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(ImpGetResStr(STR_ExchangePaste));

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

    if(pPV)
    {
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos );
    if (pLst==nullptr)
        return false;

    bool bUnmark=(nOptions&(SdrInsertFlags::DONTMARK|SdrInsertFlags::ADDMARK))==SdrInsertFlags::NONE && !IsTextEdit();
    if (bUnmark)
        UnmarkAllObj();

    // Rescale, if the Model uses a different MapUnit.
    // Calculate the necessary factors first.
    MapUnit eSrcUnit=pSrcMod->GetScaleUnit();
    MapUnit eDstUnit=mpModel->GetScaleUnit();
    bool bResize=eSrcUnit!=eDstUnit;
    Fraction xResize,yResize;
    Point aPt0;
    if (bResize)
    {
        FrPair aResize(GetMapFactor(eSrcUnit,eDstUnit));
        xResize=aResize.X();
        yResize=aResize.Y();
    }
    SdrObjList*  pDstLst=pLst;
    sal_uInt16 nPg,nPgAnz=pSrcMod->GetPageCount();
    for (nPg=0; nPg<nPgAnz; nPg++)
    {
        const SdrPage* pSrcPg=pSrcMod->GetPage(nPg);

        // Use SnapRect, not BoundRect here
        Rectangle aR=pSrcPg->GetAllObjSnapRect();

        if (bResize)
            ResizeRect(aR,aPt0,xResize,yResize);
        Point aDist(aPos-aR.Center());
        Size  aSiz(aDist.X(),aDist.Y());
        size_t nCloneErrCnt = 0;
        const size_t nObjCount = pSrcPg->GetObjCount();
        bool bMark = pMarkPV!=nullptr && !IsTextEdit() && (nOptions&SdrInsertFlags::DONTMARK)==SdrInsertFlags::NONE;

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for (size_t nOb=0; nOb<nObjCount; ++nOb)
        {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

            SdrObject* pNeuObj = pSrcOb->CloneWithShellIDs(rSrcShellID, rDestShellID);

            if (pNeuObj!=nullptr)
            {
                if(bResize)
                {
                    pNeuObj->GetModel()->SetPasteResize(true);
                    pNeuObj->NbcResize(aPt0,xResize,yResize);
                    pNeuObj->GetModel()->SetPasteResize(false);
                }

                // #i39861#
                pNeuObj->SetModel(pDstLst->GetModel());
                pNeuObj->SetPage(pDstLst->GetPage());

                pNeuObj->NbcMove(aSiz);

                const SdrPage* pPg = pDstLst->GetPage();

                if(pPg)
                {
                    // #i72535#
                    const SdrLayerAdmin& rAd = pPg->GetLayerAdmin();
                    SdrLayerID nLayer(0);

                    if(dynamic_cast<const FmFormObj*>( pNeuObj) !=  nullptr)
                    {
                        // for FormControls, force to form layer
                        nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                    }
                    else
                    {
                        nLayer = rAd.GetLayerID(maActualLayer, true);
                    }

                    if(SDRLAYER_NOTFOUND == nLayer)
                    {
                        nLayer = 0;
                    }

                    pNeuObj->SetLayer(nLayer);
                }

                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pDstLst->InsertObject(pNeuObj, SAL_MAX_SIZE, &aReason);

                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));

                if (bMark) {
                    // Don't already set Markhandles!
                    // That is instead being done by ModelHasChanged in MarkView.
                    MarkObj(pNeuObj,pMarkPV,false,true);
                }

                // #i13033#
                aCloneList.AddPair(pSrcOb, pNeuObj);
            }
            else
            {
                nCloneErrCnt++;
            }
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();

        if(0L != nCloneErrCnt)
        {
#ifdef DBG_UTIL
            OStringBuffer aStr("SdrExchangeView::Paste(): Error when cloning ");

            if(nCloneErrCnt == 1)
            {
                aStr.append("a drawing object.");
            }
            else
            {
                aStr.append(static_cast<sal_Int32>(nCloneErrCnt));
                aStr.append(" drawing objects.");
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
    MapUnit eDstMU=mpModel->GetScaleUnit();
    FrPair aMapFact(GetMapFactor(eSrcMU,eDstMU));
    Fraction aDstFr(mpModel->GetScaleFraction());
    nSizX*=aMapFact.X().GetNumerator();
    nSizX*=rMap.GetScaleX().GetNumerator();
    nSizX*=aDstFr.GetDenominator();
    nSizX/=aMapFact.X().GetDenominator();
    nSizX/=rMap.GetScaleX().GetDenominator();
    nSizX/=aDstFr.GetNumerator();
    nSizY*=aMapFact.Y().GetNumerator();
    nSizY*=rMap.GetScaleY().GetNumerator();
    nSizX*=aDstFr.GetDenominator();
    nSizY/=aMapFact.Y().GetDenominator();
    nSizY/=rMap.GetScaleY().GetDenominator();
    nSizY/=aDstFr.GetNumerator();
    long xs=nSizX;
    long ys=nSizY;
    Point aPos(rCenter.X()-xs/2,rCenter.Y()-ys/2);
    Rectangle aR(aPos.X(),aPos.Y(),aPos.X()+xs,aPos.Y()+ys);
    pObj->SetLogicRect(aR);
    SdrInsertReason aReason(SDRREASON_VIEWCALL);
    rLst.InsertObject(pObj, SAL_MAX_SIZE, &aReason);

    if( IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));

    SdrPageView* pMarkPV=nullptr;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        if (pPV->GetObjList()==&rLst)
            pMarkPV=pPV;
    }

    bool bMark = pMarkPV!=nullptr && !IsTextEdit() && (nOptions&SdrInsertFlags::DONTMARK)==SdrInsertFlags::NONE;
    if (bMark)
    { // select object the first PageView we found
        MarkObj(pObj,pMarkPV);
    }
}

BitmapEx SdrExchangeView::GetMarkedObjBitmapEx(bool bNoVDevIfOneBmpMarked) const
{
    BitmapEx aBmp;

    if( AreObjectsMarked() )
    {
        if(1 == GetMarkedObjectCount())
        {
            if(bNoVDevIfOneBmpMarked)
            {
                SdrObject*  pGrafObjTmp = GetMarkedObjectByIndex( 0 );
                SdrGrafObj* pGrafObj = dynamic_cast<SdrGrafObj*>( pGrafObjTmp  );

                if( pGrafObj && ( pGrafObj->GetGraphicType() == GRAPHIC_BITMAP ) )
                {
                    aBmp = pGrafObj->GetTransformedGraphic().GetBitmapEx();
                }
            }
            else
            {
                const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(GetMarkedObjectByIndex(0));

                if(pSdrGrafObj && pSdrGrafObj->isEmbeddedSvg())
                {
                    aBmp = pSdrGrafObj->GetGraphic().getSvgData()->getReplacement();
                }
            }
        }

        if( !aBmp )
        {
            // choose conversion directly using primitives to bitmap to avoid
            // rendering errors with tiled bitmap fills (these will be tiled in a
            // in-between metafile, but tend to show 'gaps' since the target is *no*
            // bitmap rendering)
            ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());
            const sal_uInt32 nCount(aSdrObjects.size());

            if(nCount)
            {
                // collect sub-primitives as group objects, thus no expensive append
                // to existing sequence is needed
                drawinglayer::primitive2d::Primitive2DSequence xPrimitives(nCount);

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    SdrObject* pCandidate = aSdrObjects[a];
                    SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pCandidate);

                    if(pSdrGrafObj)
                    {
                        // #122753# To ensure existence of graphic content, force swap in
                        pSdrGrafObj->ForceSwapIn();
                    }

                    xPrimitives[a] = new drawinglayer::primitive2d::GroupPrimitive2D(
                        pCandidate->GetViewContact().getViewIndependentPrimitive2DSequence());
                }

                // get logic range
                const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                const basegfx::B2DRange aRange(
                    drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                        xPrimitives,
                        aViewInformation2D));

                if(!aRange.isEmpty())
                {
                    // if we have geometry and it has a range, convert to BitmapEx using
                    // common tooling
                    aBmp = convertPrimitive2DSequenceToBitmapEx(
                        xPrimitives,
                        aRange);
                }
            }
        }
    }

    return aBmp;
}



GDIMetaFile SdrExchangeView::GetMarkedObjMetaFile(bool bNoVDevIfOneMtfMarked) const
{
    GDIMetaFile aMtf;

    if( AreObjectsMarked() )
    {
        Rectangle   aBound( GetMarkedObjBoundRect() );
        Size        aBoundSize( aBound.GetWidth(), aBound.GetHeight() );
        MapMode     aMap( mpModel->GetScaleUnit(), Point(), mpModel->GetScaleFraction(), mpModel->GetScaleFraction() );

        if( bNoVDevIfOneMtfMarked )
        {
            SdrObject*  pGrafObjTmp = GetMarkedObjectByIndex( 0 );
            SdrGrafObj* pGrafObj = ( GetMarkedObjectCount() ==1 ) ? dynamic_cast<SdrGrafObj*>( pGrafObjTmp  ) : nullptr;

            if( pGrafObj )
            {
                Graphic aGraphic( pGrafObj->GetTransformedGraphic() );

                // #119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
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

            DrawMarkedObj(*pOut.get());

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

    if( AreObjectsMarked() )
    {
        if( ( 1 == GetMarkedObjectCount() ) && GetSdrMarkByIndex( 0 ) )
            aRet = SdrExchangeView::GetObjGraphic( mpModel, GetMarkedObjectByIndex( 0 ) );
        else
            aRet = GetMarkedObjMetaFile();
    }

    return aRet;
}



Graphic SdrExchangeView::GetObjGraphic( const SdrModel* pModel, const SdrObject* pObj )
{
    Graphic aRet;

    if( pModel && pObj )
    {
        // try to get a graphic from the object first
        const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
        const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);

        if(pSdrGrafObj)
        {
            if(pSdrGrafObj->isEmbeddedSvg())
            {
                // get Metafile for Svg content
                aRet = pSdrGrafObj->getMetafileFromEmbeddedSvg();
            }
            else
            {
                // Make behaviour coherent with metafile
                // recording below (which of course also takes
                // view-transformed objects)
                aRet = pSdrGrafObj->GetTransformedGraphic();
            }
        }
        else if(pSdrOle2Obj)
        {
            if ( pSdrOle2Obj->GetGraphic() )
                aRet = *pSdrOle2Obj->GetGraphic();
        }

        // if graphic could not be retrieved => go the hard way and create a MetaFile
        if( ( GRAPHIC_NONE == aRet.GetType() ) || ( GRAPHIC_DEFAULT == aRet.GetType() ) )
        {
            ScopedVclPtrInstance< VirtualDevice > pOut;
            GDIMetaFile     aMtf;
            const Rectangle aBoundRect( pObj->GetCurrentBoundRect() );
            const MapMode   aMap( pModel->GetScaleUnit(),
                                  Point(),
                                  pModel->GetScaleFraction(),
                                  pModel->GetScaleFraction() );

            pOut->EnableOutput( false );
            pOut->SetMapMode( aMap );
            aMtf.Record( pOut );
            pObj->SingleObjectPainter( *pOut.get() );
            aMtf.Stop();
            aMtf.WindStart();

            // #i99268# replace the original offset from using XOutDev's SetOffset
            // NOT (as tried with #i92760#) with another MapMode which gets recorded
            // by the Metafile itself (what always leads to problems), but by
            // moving the result directly
            aMtf.Move(-aBoundRect.Left(), -aBoundRect.Top());

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundRect.GetSize() );

            if( aMtf.GetActionSize() )
                aRet = aMtf;
        }
     }

     return aRet;
}



::std::vector< SdrObject* > SdrExchangeView::GetMarkedObjects() const
{
    SortMarkedObjects();
    ::std::vector< SdrObject* > aRetval;

    ::std::vector< ::std::vector< SdrMark* > >  aObjVectors( 2 );
    ::std::vector< SdrMark* >&                  rObjVector1 = aObjVectors[ 0 ];
    ::std::vector< SdrMark* >&                  rObjVector2 = aObjVectors[ 1 ];
    const SdrLayerAdmin&                        rLayerAdmin = mpModel->GetLayerAdmin();
    const sal_uInt32                            nControlLayerId = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), false );

    for( size_t n = 0, nCount = GetMarkedObjectCount(); n < nCount; ++n )
    {
        SdrMark* pMark = GetSdrMarkByIndex( n );

        // paint objects on control layer on top of all other objects
        if( nControlLayerId == pMark->GetMarkedSdrObj()->GetLayer() )
            rObjVector2.push_back( pMark );
        else
            rObjVector1.push_back( pMark );
    }

    for( size_t n = 0, nCount = aObjVectors.size(); n < nCount; ++n )
    {
        ::std::vector< SdrMark* >& rObjVector = aObjVectors[ n ];

        for( size_t i = 0; i < rObjVector.size(); ++i )
        {
            SdrMark*    pMark = rObjVector[ i ];
            aRetval.push_back(pMark->GetMarkedSdrObj());
        }
    }

    return aRetval;
}



void SdrExchangeView::DrawMarkedObj(OutputDevice& rOut) const
{
    ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());

    if(aSdrObjects.size())
    {
        sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aSdrObjects, aSdrObjects[0]->GetPage());
        sdr::contact::DisplayInfo aDisplayInfo;

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);
    }
}



SdrModel* SdrExchangeView::GetMarkedObjModel() const
{
    // Sorting the MarkList here might be problematic in the future, so
    // use a copy.
    SortMarkedObjects();
    SdrModel* pNeuMod=mpModel->AllocModel();
    SdrPage* pNeuPag=pNeuMod->AllocPage(false);
    pNeuMod->InsertPage(pNeuPag);

    if( !mxSelectionController.is() || !mxSelectionController->GetMarkedObjModel( pNeuPag ) )
    {
        ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for( size_t i(0); i < aSdrObjects.size(); i++ )
        {
            const SdrObject*    pObj = aSdrObjects[i];
            SdrObject*          pNeuObj;

            if( dynamic_cast<const SdrPageObj*>( pObj) !=  nullptr )
            {
                // convert SdrPageObj's to a graphic representation, because
                // virtual connection to referenced page gets lost in new model
                pNeuObj = new SdrGrafObj( GetObjGraphic( mpModel, pObj ), pObj->GetLogicRect() );
                pNeuObj->SetPage( pNeuPag );
                pNeuObj->SetModel( pNeuMod );
            }
            else
            {
                pNeuObj = pObj->Clone();
                pNeuObj->SetPage( pNeuPag );
                pNeuObj->SetModel( pNeuMod );
            }

            SdrInsertReason aReason(SDRREASON_VIEWCALL);
            pNeuPag->InsertObject(pNeuObj, SAL_MAX_SIZE, &aReason);

            // #i13033#
            aCloneList.AddPair(pObj, pNeuObj);
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();
    }
    return pNeuMod;
}



bool SdrExchangeView::Cut( sal_uIntPtr /*nFormat */)
{
    OSL_FAIL( "SdrExchangeView::Cut: Not supported any more." );
    return false;
}



bool SdrExchangeView::Yank(sal_uIntPtr /*nFormat*/)
{
    OSL_FAIL( "SdrExchangeView::Yank: Not supported any more." );
    return false;
}



bool SdrExchangeView::Paste(vcl::Window* /*pWin*/, sal_uIntPtr /*nFormat*/)
{
    OSL_FAIL( "SdrExchangeView::Paste: Not supported any more." );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
