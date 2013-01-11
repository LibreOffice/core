/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vector>
#include <editeng/editeng.hxx>
#include <svx/xexch.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditext.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx> // fuer kein OLE im SdrClipboardFormat
#include <svx/svdorect.hxx>
#include <svx/svdoedge.hxx> // fuer Konnektoren uebers Clipboard
#include <svx/svdopage.hxx> // fuer Konnektoren uebers Clipboard
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx> // Fuer GetMapFactor zum umskalieren bei PasteModel
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrExchangeView::SdrExchangeView(SdrModel* pModel1, OutputDevice* pOut):
    SdrObjEditView(pModel1,pOut)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Point SdrExchangeView::GetViewCenter(const OutputDevice* pOut) const
{
    Point aCenter;
    if (pOut==NULL)
    {
        pOut = GetFirstOutputDevice();
    }
    if (pOut!=NULL) {
        Point aOfs=pOut->GetMapMode().GetOrigin();
        Size aOutSiz=pOut->GetOutputSize();
        aOutSiz.Width()/=2;
        aOutSiz.Height()/=2;
        aCenter.X()=aOutSiz.Width() -aOfs.X();
        aCenter.Y()=aOutSiz.Height()-aOfs.Y();
    }
    return aCenter;
}

Point SdrExchangeView::GetPastePos(SdrObjList* pLst, OutputDevice* pOut)
{
    Point aP(GetViewCenter(pOut));
    SdrPage* pPg=NULL;
    if (pLst!=NULL) pPg=pLst->GetPage();
    if (pPg!=NULL) {
        Size aSiz(pPg->GetSize());
        aP.X()=aSiz.Width()/2;
        aP.Y()=aSiz.Height()/2;
    }
    return aP;
}

sal_Bool SdrExchangeView::ImpLimitToWorkArea(Point& rPt) const
{
    sal_Bool bRet(sal_False);

    if(!aMaxWorkArea.IsEmpty())
    {
        if(rPt.X()<aMaxWorkArea.Left())
        {
            rPt.X() = aMaxWorkArea.Left();
            bRet = sal_True;
        }

        if(rPt.X()>aMaxWorkArea.Right())
        {
            rPt.X() = aMaxWorkArea.Right();
            bRet = sal_True;
        }

        if(rPt.Y()<aMaxWorkArea.Top())
        {
            rPt.Y() = aMaxWorkArea.Top();
            bRet = sal_True;
        }

        if(rPt.Y()>aMaxWorkArea.Bottom())
        {
            rPt.Y() = aMaxWorkArea.Bottom();
            bRet = sal_True;
        }
    }
    return bRet;
}

void SdrExchangeView::ImpGetPasteObjList(Point& /*rPos*/, SdrObjList*& rpLst)
{
    if (rpLst==NULL)
    {
        SdrPageView* pPV = GetSdrPageView();

        if (pPV!=NULL) {
            rpLst=pPV->GetObjList();
        }
    }
}

sal_Bool SdrExchangeView::ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const
{
    sal_Bool bRet=sal_False;
    rLayer=0;
    if (pObjList!=NULL) {
        const SdrPage* pPg=pObjList->GetPage();
        if (pPg!=NULL) {
            rLayer=pPg->GetLayerAdmin().GetLayerID(aAktLayer,sal_True);
            if (rLayer==SDRLAYER_NOTFOUND) rLayer=0;
            SdrPageView* pPV = GetSdrPageView();
            if (pPV!=NULL) {
                bRet=!pPV->GetLockedLayers().IsSet(rLayer) && pPV->GetVisibleLayers().IsSet(rLayer);
            }
        }
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrExchangeView::Paste(const GDIMetaFile& rMtf, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return sal_False;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return sal_False;
    sal_Bool bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    SdrGrafObj* pObj=new SdrGrafObj(Graphic(rMtf));
    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj,*pLst,aPos,rMtf.GetPrefSize(),rMtf.GetPrefMapMode(),nOptions);
    return sal_True;
}

sal_Bool SdrExchangeView::Paste(const Bitmap& rBmp, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return sal_False;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return sal_False;
    sal_Bool bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    SdrGrafObj* pObj=new SdrGrafObj(Graphic(rBmp));
    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj,*pLst,aPos,rBmp.GetSizePixel(),MapMode(MAP_PIXEL),nOptions);
    return sal_True;
}

sal_Bool SdrExchangeView::Paste(const XubString& rStr, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    if(!rStr.Len())
        return sal_False;

    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return sal_False;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return sal_False;
    sal_Bool bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->GetPage();
    if (pPage!=NULL) {
        aTextRect.SetSize(pPage->GetSize());
    }
    SdrRectObj* pObj=new SdrRectObj(OBJ_TEXT,aTextRect);
    pObj->SetModel(pMod);
    pObj->SetLayer(nLayer);
    pObj->NbcSetText(rStr); // #32424# SetText vor SetAttr, weil SetAttr sonst unwirksam!
    if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet, sal_False);

    pObj->SetMergedItemSet(aDefaultAttr);

    SfxItemSet aTempAttr(pMod->GetItemPool());  // Keine Fuellung oder Linie
    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=pMod->GetScaleUnit();
    Fraction aMap=pMod->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);
    return sal_True;
}

sal_Bool SdrExchangeView::Paste(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return sal_False;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return sal_False;
    sal_Bool bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    Rectangle aTextRect(0,0,500,500);
    SdrPage* pPage=pLst->GetPage();
    if (pPage!=NULL) {
        aTextRect.SetSize(pPage->GetSize());
    }
    SdrRectObj* pObj=new SdrRectObj(OBJ_TEXT,aTextRect);
    pObj->SetModel(pMod);
    pObj->SetLayer(nLayer);
    if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet, sal_False);

    pObj->SetMergedItemSet(aDefaultAttr);

    SfxItemSet aTempAttr(pMod->GetItemPool());  // Keine Fuellung oder Linie
    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));

    pObj->SetMergedItemSet(aTempAttr);

    pObj->NbcSetText(rInput,rBaseURL,eFormat);
    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=pMod->GetScaleUnit();
    Fraction aMap=pMod->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);

    // b4967543
    if(pObj && pObj->GetModel() && pObj->GetOutlinerParaObject())
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
                    pObj->NbcSetStyleSheet(pCandidate, sal_True);
                }
            }
        }
    }

    return sal_True;
}

sal_Bool SdrExchangeView::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    const SdrModel* pSrcMod=&rMod;
    if (pSrcMod==pMod)
        return sal_False; // na so geht's ja nun nicht

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(ImpGetResStr(STR_ExchangePaste));

    if( mxSelectionController.is() && mxSelectionController->PasteObjModel( rMod ) )
    {
        if( bUndo )
            EndUndo();
        return sal_True;
    }

    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    SdrPageView* pMarkPV=NULL;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos );
    if (pLst==NULL)
        return sal_False;

    sal_Bool bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark)
        UnmarkAllObj();

    // evtl. umskalieren bei unterschiedlicher MapUnit am Model
    // Dafuer erstmal die Faktoren berechnen
    MapUnit eSrcUnit=pSrcMod->GetScaleUnit();
    MapUnit eDstUnit=pMod->GetScaleUnit();
    sal_Bool bResize=eSrcUnit!=eDstUnit;
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

        // #104148# Use SnapRect, not BoundRect here
        Rectangle aR=pSrcPg->GetAllObjSnapRect();

        if (bResize)
            ResizeRect(aR,aPt0,xResize,yResize);
        Point aDist(aPos-aR.Center());
        Size  aSiz(aDist.X(),aDist.Y());
        //sal_uIntPtr nDstObjAnz0=pDstLst->GetObjCount();
        sal_uIntPtr nCloneErrCnt=0;
        sal_uIntPtr nOb,nObAnz=pSrcPg->GetObjCount();
        sal_Bool bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for (nOb=0; nOb<nObAnz; nOb++)
        {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

            // #116235#
            SdrObject* pNeuObj = pSrcOb->Clone();

            if (pNeuObj!=NULL)
            {
                if(bResize)
                {
                    pNeuObj->GetModel()->SetPasteResize(sal_True); // #51139#
                    pNeuObj->NbcResize(aPt0,xResize,yResize);
                    pNeuObj->GetModel()->SetPasteResize(sal_False); // #51139#
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

                    if(pNeuObj->ISA(FmFormObj))
                    {
                        // for FormControls, force to form layer
                        nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                    }
                    else
                    {
                        nLayer = rAd.GetLayerID(aAktLayer, sal_True);
                    }

                    if(SDRLAYER_NOTFOUND == nLayer)
                    {
                        nLayer = 0;
                    }

                    pNeuObj->SetLayer(nLayer);
                }

                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pDstLst->InsertObject(pNeuObj,CONTAINER_APPEND,&aReason);

                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));

                if (bMark) {
                    // Markhandles noch nicht sofort setzen!
                    // Das erledigt das ModelHasChanged der MarkView.
                    MarkObj(pNeuObj,pMarkPV,sal_False,sal_True);
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
            ByteString aStr("SdrExchangeView::Paste(): Fehler beim Clonen ");

            if(nCloneErrCnt == 1)
            {
                aStr += "eines Zeichenobjekts.";
            }
            else
            {
                aStr += "von ";
                aStr += ByteString::CreateFromInt32( nCloneErrCnt );
                aStr += " Zeichenobjekten.";
            }

            aStr += " Objektverbindungen werden nicht mitkopiert.";

            DBG_ERROR(aStr.GetBuffer());
#endif
        }
    }

    if( bUndo )
        EndUndo();

    return sal_True;
}

sal_Bool SdrExchangeView::IsExchangeFormatSupported(sal_uIntPtr nFormat) const
{
    return( FORMAT_PRIVATE == nFormat ||
            FORMAT_GDIMETAFILE == nFormat ||
            FORMAT_BITMAP == nFormat ||
            FORMAT_RTF == nFormat ||
            FORMAT_STRING == nFormat ||
            SOT_FORMATSTR_ID_DRAWING == nFormat ||
            SOT_FORMATSTR_ID_EDITENGINE == nFormat );
}

void SdrExchangeView::ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const Point& rCenter, const Size& rSiz, const MapMode& rMap, sal_uInt32 nOptions)
{
    BigInt nSizX(rSiz.Width());
    BigInt nSizY(rSiz.Height());
    MapUnit eSrcMU=rMap.GetMapUnit();
    MapUnit eDstMU=pMod->GetScaleUnit();
    FrPair aMapFact(GetMapFactor(eSrcMU,eDstMU));
    Fraction aDstFr(pMod->GetScaleFraction());
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
    rLst.InsertObject(pObj,CONTAINER_APPEND,&aReason);

    if( IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));

    SdrPageView* pMarkPV=NULL;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        if (pPV->GetObjList()==&rLst)
            pMarkPV=pPV;
    }

    sal_Bool bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;
    if (bMark)
    { // Obj in der ersten gefundenen PageView markieren
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
                SdrGrafObj* pGrafObj = ( GetMarkedObjectCount() == 1 ) ? PTR_CAST( SdrGrafObj, pGrafObjTmp ) : NULL;

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
                    xPrimitives[a] = new drawinglayer::primitive2d::GroupPrimitive2D(
                        aSdrObjects[a]->GetViewContact().getViewIndependentPrimitive2DSequence());
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
                        aRange,
                        500000);
                }
            }
        }
    }

    return aBmp;
}

// -----------------------------------------------------------------------------

GDIMetaFile SdrExchangeView::GetMarkedObjMetaFile(bool bNoVDevIfOneMtfMarked) const
{
    GDIMetaFile aMtf;

    if( AreObjectsMarked() )
    {
        Rectangle   aBound( GetMarkedObjBoundRect() );
        Size        aBoundSize( aBound.GetWidth(), aBound.GetHeight() );
        MapMode     aMap( pMod->GetScaleUnit(), Point(), pMod->GetScaleFraction(), pMod->GetScaleFraction() );

        if( bNoVDevIfOneMtfMarked )
        {
            SdrObject*  pGrafObjTmp = GetMarkedObjectByIndex( 0 );
            SdrGrafObj* pGrafObj = ( GetMarkedObjectCount() ==1 ) ? PTR_CAST( SdrGrafObj, pGrafObjTmp ) : NULL;

            if( pGrafObj )
            {
                Graphic aGraphic( pGrafObj->GetTransformedGraphic() );

                // #119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                aMtf = aGraphic.GetGDIMetaFile();
            }
        }

        if( !aMtf.GetActionCount() )
        {
            VirtualDevice aOut;
            const Size aDummySize(2, 2);

            aOut.SetOutputSizePixel(aDummySize);
            aOut.EnableOutput(false);
            aOut.SetMapMode(aMap);
            aMtf.Clear();
            aMtf.Record(&aOut);

            DrawMarkedObj(aOut);

            aMtf.Stop();
            aMtf.WindStart();

            // moving the result is more reliable then setting a relative MapMode at the VDev (used
            // before), also see #i99268# in GetObjGraphic() below. Some draw actions at
            // the OutDev are simply not handled correctly when a MapMode is set at the
            // target devive, e.g. MetaFloatTransparentAction. Even the Move for this action
            // was missing the manipulation of the embedded Metafile
            aMtf.Move(-aBound.Left(), -aBound.Top());

            aMtf.SetPrefMapMode( aMap );

            // removed PrefSize extension. It is principially wrong to set a reduced size at
            // the created MetaFile. The mentioned errors occurr at output time since the integer
            // MapModes from VCL lead to errors. It is now corrected in the VCLRenderer for
            // primitives (and may later be done in breaking up a MetaFile to primitives)
            aMtf.SetPrefSize(aBoundSize);
        }
    }

    return aMtf;
}

// -----------------------------------------------------------------------------

Graphic SdrExchangeView::GetAllMarkedGraphic() const
{
    Graphic aRet;

    if( AreObjectsMarked() )
    {
        if( ( 1 == GetMarkedObjectCount() ) && GetSdrMarkByIndex( 0 ) )
            aRet = SdrExchangeView::GetObjGraphic( pMod, GetMarkedObjectByIndex( 0 ) );
        else
            aRet = GetMarkedObjMetaFile(false);
    }

    return aRet;
}

// -----------------------------------------------------------------------------

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
                // #110981# Make behaviour coherent with metafile
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
            VirtualDevice   aOut;
            GDIMetaFile     aMtf;
            const Rectangle aBoundRect( pObj->GetCurrentBoundRect() );
            const MapMode   aMap( pModel->GetScaleUnit(),
                                  Point(),
                                  pModel->GetScaleFraction(),
                                  pModel->GetScaleFraction() );

            aOut.EnableOutput( sal_False );
            aOut.SetMapMode( aMap );
            aMtf.Record( &aOut );
            pObj->SingleObjectPainter( aOut ); // #110094#-17
            aMtf.Stop();
            aMtf.WindStart();

            // #i99268# replace the original offset from using XOutDev's SetOffset
            // NOT (as tried with #i92760#) with another MapMode which gets recorded
            // by the Metafile itself (what always leads to problems), but by
            // moving the result directly
            aMtf.Move(-aBoundRect.Left(), -aBoundRect.Top());

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundRect.GetSize() );

            if( aMtf.GetActionCount() )
                aRet = aMtf;
        }
     }

     return aRet;
}

// -----------------------------------------------------------------------------

::std::vector< SdrObject* > SdrExchangeView::GetMarkedObjects() const
{
    SortMarkedObjects();
    ::std::vector< SdrObject* > aRetval;

    ::std::vector< ::std::vector< SdrMark* > >  aObjVectors( 2 );
    ::std::vector< SdrMark* >&                  rObjVector1 = aObjVectors[ 0 ];
    ::std::vector< SdrMark* >&                  rObjVector2 = aObjVectors[ 1 ];
    const SdrLayerAdmin&                        rLayerAdmin = pMod->GetLayerAdmin();
    const sal_uInt32                            nControlLayerId = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), sal_False );
    sal_uInt32                                  n, nCount;

    for( n = 0, nCount = GetMarkedObjectCount(); n < nCount; n++ )
    {
        SdrMark* pMark = GetSdrMarkByIndex( n );

        // paint objects on control layer on top of all otherobjects
        if( nControlLayerId == pMark->GetMarkedSdrObj()->GetLayer() )
            rObjVector2.push_back( pMark );
        else
            rObjVector1.push_back( pMark );
    }

    for( n = 0, nCount = aObjVectors.size(); n < nCount; n++ )
    {
        ::std::vector< SdrMark* >& rObjVector = aObjVectors[ n ];

        for( sal_uInt32 i = 0; i < rObjVector.size(); i++ )
        {
            SdrMark*    pMark = rObjVector[ i ];
            aRetval.push_back(pMark->GetMarkedSdrObj());
        }
    }

    return aRetval;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

SdrModel* SdrExchangeView::GetMarkedObjModel() const
{
    // Wenn das sortieren der MarkList mal stoeren sollte,
    // werde ich sie mir wohl kopieren muessen.
    SortMarkedObjects();
    SdrModel* pNeuMod=pMod->AllocModel();
    SdrPage* pNeuPag=pNeuMod->AllocPage(sal_False);
    pNeuMod->InsertPage(pNeuPag);

    if( !mxSelectionController.is() || !mxSelectionController->GetMarkedObjModel( pNeuPag ) )
    {
        ::std::vector< SdrObject* > aSdrObjects(GetMarkedObjects());

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;
        sal_uInt32 nCloneErrCnt(0);

        for( sal_uInt32 i(0); i < aSdrObjects.size(); i++ )
        {
            const SdrObject*    pObj = aSdrObjects[i];
            SdrObject*          pNeuObj;

            if( pObj->ISA( SdrPageObj ) )
            {
                // convert SdrPageObj's to a graphic representation, because
                // virtual connection to referenced page gets lost in new model
                pNeuObj = new SdrGrafObj( GetObjGraphic( pMod, pObj ), pObj->GetLogicRect() );
                pNeuObj->SetPage( pNeuPag );
                pNeuObj->SetModel( pNeuMod );
            }
            else
            {
                // #116235#
                // pNeuObj = pObj->Clone( pNeuPag, pNeuMod );
                pNeuObj = pObj->Clone();
                pNeuObj->SetPage( pNeuPag );
                pNeuObj->SetModel( pNeuMod );
            }

            if( pNeuObj )
            {
                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pNeuPag->InsertObject(pNeuObj,CONTAINER_APPEND,&aReason);

                // #i13033#
                aCloneList.AddPair(pObj, pNeuObj);
            }
            else
                nCloneErrCnt++;
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();

        if(0L != nCloneErrCnt)
        {
#ifdef DBG_UTIL
            ByteString aStr("SdrExchangeView::GetMarkedObjModel(): Fehler beim Clonen ");

            if(nCloneErrCnt == 1)
            {
                aStr += "eines Zeichenobjekts.";
            }
            else
            {
                aStr += "von ";
                aStr += ByteString::CreateFromInt32( nCloneErrCnt );
                aStr += " Zeichenobjekten.";
            }

            aStr += " Objektverbindungen werden nicht mitkopiert.";

            DBG_ERROR(aStr.GetBuffer());
#endif
        }
    }
    return pNeuMod;
}

// -----------------------------------------------------------------------------

sal_Bool SdrExchangeView::Cut( sal_uIntPtr /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::Cut: Not supported anymore" );
    return sal_False;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::CutMarked( sal_uIntPtr /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::CutMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

sal_Bool SdrExchangeView::Yank(sal_uIntPtr /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Yank: Not supported anymore" );
    return sal_False;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::YankMarked(sal_uIntPtr /*nFormat*/)
{
    DBG_ERROR( "YankMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

sal_Bool SdrExchangeView::Paste(Window* /*pWin*/, sal_uIntPtr /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Paste: Not supported anymore" );
    return sal_False;
}

// eof
