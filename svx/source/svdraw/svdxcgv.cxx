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
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
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
#include "fmobj.hxx"
#include <svx/svdlegacy.hxx>
#include <fmobj.hxx>
#include <vcl/svgdata.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrExchangeView::SdrExchangeView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrObjEditView(rModel1, pOut)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

basegfx::B2DPoint SdrExchangeView::GetViewCenter(const OutputDevice* pOut) const
{
    basegfx::B2DPoint aCenter;

    if(!pOut)
    {
        pOut = GetFirstOutputDevice();
    }

    if(pOut)
    {
        aCenter = pOut->GetLogicRange().getCenter();
    }

    return aCenter;
}

basegfx::B2DPoint SdrExchangeView::GetPastePos(SdrObjList* pLst, OutputDevice* pOut) const
{
    SdrPage* pSdrPage = 0;

    if(pLst)
    {
        pSdrPage = pLst->getSdrPageFromSdrObjList();
    }

    if(pSdrPage)
    {
        return pSdrPage->GetPageScale() * 0.5;
    }

    return GetViewCenter(pOut);
}

basegfx::B2DPoint SdrExchangeView::ImpLimitToWorkArea(const basegfx::B2DPoint& rPt) const
{
    const basegfx::B2DRange& rWorkArea = GetWorkArea();

    if(rWorkArea.isEmpty())
    {
        return rPt;
    }

    return rWorkArea.clamp(rPt);
}

void SdrExchangeView::ImpGetPasteObjList(SdrObjList*& rpLst)
{
    if(!rpLst)
    {
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            rpLst = pPV->GetCurrentObjectList();
        }
    }
}

bool SdrExchangeView::ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const
{
    bool bRet(false);
    rLayer=0;

    if(pObjList)
    {
        const SdrPage* pPg = pObjList->getSdrPageFromSdrObjList();

        if(pPg)
        {
            rLayer = pPg->GetPageLayerAdmin().GetLayerID(GetActiveLayer(), true);

            if(SDRLAYER_NOTFOUND == rLayer)
            {
                rLayer = 0;
            }

            SdrPageView* pPV = GetSdrPageView();

            if(pPV)
            {
                bRet=!pPV->GetLockedLayers().IsSet(rLayer) && pPV->GetVisibleLayers().IsSet(rLayer);
            }
        }
    }

    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrExchangeView::Paste(const GDIMetaFile& rMtf, const basegfx::B2DPoint& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    ImpGetPasteObjList(pLst);

    if(!pLst)
    {
        return false;
    }

    SdrLayerID nLayer;

    if(!ImpGetPasteLayer(pLst, nLayer))
    {
        return false;
    }

    const bool bUnmark(0 == (nOptions & (SDRINSERT_DONTMARK|SDRINSERT_ADDMARK)) && !IsTextEdit());

    if(bUnmark)
    {
        UnmarkAllObj();
    }

    const basegfx::B2DPoint aPos(ImpLimitToWorkArea(rPos));
    SdrGrafObj* pObj = new SdrGrafObj(getSdrModelFromSdrView(), Graphic(rMtf));

    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj, *pLst, aPos, basegfx::B2DVector(rMtf.GetPrefSize().getWidth(), rMtf.GetPrefSize().getHeight()), rMtf.GetPrefMapMode(), nOptions);

    return true;
}

bool SdrExchangeView::Paste(const Bitmap& rBmp, const basegfx::B2DPoint& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    ImpGetPasteObjList(pLst);

    if(!pLst)
    {
        return false;
    }

    SdrLayerID nLayer;

    if(!ImpGetPasteLayer(pLst, nLayer))
    {
        return false;
    }

    const bool bUnmark(0 == (nOptions & (SDRINSERT_DONTMARK|SDRINSERT_ADDMARK)) && !IsTextEdit());

    if(bUnmark)
    {
        UnmarkAllObj();
    }

    const basegfx::B2DPoint aPos(ImpLimitToWorkArea(rPos));
    SdrGrafObj* pObj = new SdrGrafObj(getSdrModelFromSdrView(), Graphic(rBmp));

    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj, *pLst, aPos, basegfx::B2DVector(rBmp.GetSizePixel().getWidth(), rBmp.GetSizePixel().getHeight()), MapMode(MAP_PIXEL), nOptions);

    return true;
}

bool SdrExchangeView::Paste(const XubString& rStr, const basegfx::B2DPoint& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    if(!rStr.Len())
    {
        return false;
    }

    ImpGetPasteObjList(pLst);

    if(!pLst)
    {
        return false;
    }

    SdrLayerID nLayer;

    if(!ImpGetPasteLayer(pLst, nLayer))
    {
        return false;
    }

    const bool bUnmark(0 == (nOptions & (SDRINSERT_DONTMARK|SDRINSERT_ADDMARK)) && !IsTextEdit());

    if(bUnmark)
    {
        UnmarkAllObj();
    }

    basegfx::B2DVector aTextScale(500.0, 500.0);
    SdrPage* pPage = pLst->getSdrPageFromSdrObjList();

    if(pPage)
    {
        aTextScale = pPage->GetPageScale();
    }

    SdrRectObj* pObj = new SdrRectObj(
        getSdrModelFromSdrView(),
        basegfx::tools::createScaleB2DHomMatrix(aTextScale),
        OBJ_TEXT,
        true);

    pObj->SetLayer(nLayer);
    pObj->SetText(rStr); // #32424# SetText vor SetAttr, weil SetAttr sonst unwirksam!

    if(GetDefaultStyleSheet())
    {
        pObj->SetStyleSheet(GetDefaultStyleSheet(), false);
    }

    pObj->SetMergedItemSet(GetDefaultAttr());

    SfxItemSet aTempAttr(pObj->GetObjectItemPool());  // Keine Fuellung oder Linie

    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));
    pObj->SetMergedItemSet(aTempAttr);

    pObj->FitFrameToTextSize();

    const basegfx::B2DVector aObjectSize(basegfx::absolute(pObj->getSdrObjectScale()));
    const MapUnit eMap(getSdrModelFromSdrView().GetExchangeObjectUnit());
    const Fraction aMap(getSdrModelFromSdrView().GetExchangeObjectScale());
    const basegfx::B2DPoint aPos(ImpLimitToWorkArea(rPos));

    ImpPasteObject(pObj, *pLst, aPos, aObjectSize, MapMode(eMap, Point(0,0), aMap, aMap), nOptions);

    return true;
}

bool SdrExchangeView::Paste(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat, const basegfx::B2DPoint& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    ImpGetPasteObjList(pLst);

    if(!pLst)
    {
        return false;
    }

    SdrLayerID nLayer;

    if(!ImpGetPasteLayer(pLst, nLayer))
    {
        return false;
    }

    const bool bUnmark(0 == (nOptions & (SDRINSERT_DONTMARK|SDRINSERT_ADDMARK)) && !IsTextEdit());

    if(bUnmark)
    {
        UnmarkAllObj();
    }

    basegfx::B2DVector aTextScale(500.0, 500.0);
    SdrPage* pPage = pLst->getSdrPageFromSdrObjList();

    if(pPage)
    {
        aTextScale = pPage->GetPageScale();
    }

    SdrRectObj* pObj = new SdrRectObj(
        getSdrModelFromSdrView(),
        basegfx::tools::createScaleB2DHomMatrix(aTextScale),
        OBJ_TEXT,
        true);

    pObj->SetLayer(nLayer);

    if(GetDefaultStyleSheet())
    {
        pObj->SetStyleSheet(GetDefaultStyleSheet(), false);
    }

    pObj->SetMergedItemSet(GetDefaultAttr());

    SfxItemSet aTempAttr(pObj->GetObjectItemPool());  // Keine Fuellung oder Linie

    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));
    pObj->SetMergedItemSet(aTempAttr);

    pObj->SetText(rInput, rBaseURL, eFormat);
    pObj->FitFrameToTextSize();

    const basegfx::B2DVector aObjectSize(basegfx::absolute(pObj->getSdrObjectScale()));
    const MapUnit eMap(getSdrModelFromSdrView().GetExchangeObjectUnit());
    const Fraction aMap(getSdrModelFromSdrView().GetExchangeObjectScale());
    const basegfx::B2DPoint aPos(ImpLimitToWorkArea(rPos));

    ImpPasteObject(pObj, *pLst, aPos, aObjectSize, MapMode(eMap, Point(0, 0), aMap, aMap), nOptions);

    // TTTT: Shold already be done in ImpPasteObject/onModelChange ?!?
    if(pObj && GetDefaultStyleSheet())
    {
        pObj->SetStyleSheet(GetDefaultStyleSheet(), true);
    }

    return true;
}

bool SdrExchangeView::Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pDstLst, sal_uInt32 nOptions)
{
    const SdrModel* pSrcMod=&rMod;

    if(pSrcMod == &getSdrModelFromSdrView())
    {
        return false; // na so geht's ja nun nicht
    }

    const bool bUndo(IsUndoEnabled());

    if( bUndo )
    {
        BegUndo(ImpGetResStr(STR_ExchangePaste));
    }

    if( mxSelectionController.is() && mxSelectionController->PasteObjModel( rMod ) )
    {
        if( bUndo )
        {
            EndUndo();
        }

        return true;
    }

    ImpGetPasteObjList(pDstLst);

    if(!pDstLst)
    {
        return false;
    }

    const bool bUnmark(0 == (nOptions & (SDRINSERT_DONTMARK|SDRINSERT_ADDMARK)) && !IsTextEdit());

    if (bUnmark)
    {
        UnmarkAllObj();
    }

    const basegfx::B2DPoint aPos(ImpLimitToWorkArea(rPos));

    // evtl. umskalieren bei unterschiedlicher MapUnit am Model
    // Dafuer erstmal die Faktoren berechnen
    const MapUnit eSrcUnit(pSrcMod->GetExchangeObjectUnit());
    const MapUnit eDstUnit(getSdrModelFromSdrView().GetExchangeObjectUnit());
    const bool bResize(eSrcUnit != eDstUnit);
    basegfx::B2DVector aResize(1.0, 1.0);

    if (bResize)
    {
        const FrPair aResizeFract(GetMapFactor(eSrcUnit, eDstUnit));

        aResize = basegfx::B2DVector(aResizeFract.X(), aResizeFract.Y());
    }

    const sal_uInt32 nPgAnz(pSrcMod->GetPageCount());
    sal_uInt32 nPg(0);

    for(; nPg < nPgAnz; nPg++)
    {
        const SdrPage* pSrcPg=pSrcMod->GetPage(nPg);
        basegfx::B2DPoint aRangeCenter(sdr::legacy::GetAllObjSnapRange(pSrcPg->getSdrObjectVector()).getCenter());

        if (bResize)
        {
            aRangeCenter *= aResize;
        }

        const basegfx::B2DPoint aOffset(aPos - aRangeCenter);
        sal_uInt32 nCloneErrCnt(0);
        sal_uInt32 nOb(0);
        const sal_uInt32 nObAnz(pSrcPg->GetObjCount());
        const bool bMark(!IsTextEdit() && 0 == (nOptions & SDRINSERT_DONTMARK));

        // #i13033# New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for(; nOb < nObAnz; nOb++)
        {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);
            SdrObject* pNeuObj = pSrcOb->CloneSdrObject(&pDstLst->getSdrModelFromSdrObjList());

            if(pNeuObj)
            {
                if(bResize)
                {
                    pNeuObj->SetPasteResize(true); // #51139#
                    sdr::legacy::transformSdrObject(*pNeuObj, basegfx::tools::createScaleB2DHomMatrix(aResize));
                    pNeuObj->SetPasteResize(false); // #51139#
                }

                // #i39861#
                sdr::legacy::transformSdrObject(*pNeuObj, basegfx::tools::createTranslateB2DHomMatrix(aOffset));

                const SdrPage* pPg = pDstLst->getSdrPageFromSdrObjList();

                if(pPg)
                {
                    // #i72535#
                    const SdrLayerAdmin& rAd = pPg->GetPageLayerAdmin();
                    SdrLayerID nLayer(0);

                    if(dynamic_cast< FmFormObj* >(pNeuObj))
                    {
                        // for FormControls, force to form layer
                        nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                    }
                    else
                    {
                        nLayer = rAd.GetLayerID(GetActiveLayer(), true);
                    }

                    if(SDRLAYER_NOTFOUND == nLayer)
                    {
                        nLayer = 0;
                    }

                    pNeuObj->SetLayer(nLayer);
                }

                pDstLst->InsertObjectToSdrObjList(*pNeuObj);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                }

                if (bMark)
                {
                    // Markhandles noch nicht sofort setzen!
                    // Das erledigt das LazyReactOnObjectChanges der MarkView.
                    MarkObj(*pNeuObj, false );
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
    {
        EndUndo();
    }

    return true;
}

bool SdrExchangeView::IsExchangeFormatSupported(sal_uInt32 nFormat) const
{
    return( FORMAT_PRIVATE == nFormat ||
            FORMAT_GDIMETAFILE == nFormat ||
            FORMAT_BITMAP == nFormat ||
            FORMAT_RTF == nFormat ||
            FORMAT_STRING == nFormat ||
            SOT_FORMATSTR_ID_DRAWING == nFormat ||
            SOT_FORMATSTR_ID_EDITENGINE == nFormat );
}

void SdrExchangeView::ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const basegfx::B2DPoint& rCenter, const basegfx::B2DVector& rScale, const MapMode& rMap, sal_uInt32 nOptions)
{
    const MapUnit eSrcMU(rMap.GetMapUnit());
    const MapUnit eDstMU(getSdrModelFromSdrView().GetExchangeObjectUnit());
    const FrPair aMapFact(GetMapFactor(eSrcMU, eDstMU));
    const Fraction aDstFr(getSdrModelFromSdrView().GetExchangeObjectScale());

    basegfx::B2DVector aScale(rScale);
    aScale *= basegfx::B2DVector(aMapFact.X(), aMapFact.Y());
    aScale *= basegfx::B2DVector(rMap.GetScaleX(), rMap.GetScaleY());
    aScale *= aDstFr;

    pObj->setSdrObjectTransformation(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aScale,
            rCenter - (aScale * 0.5)));

    rLst.InsertObjectToSdrObjList(*pObj);

    if(IsUndoEnabled())
    {
        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pObj));
    }

    const bool bMark(!IsTextEdit() && 0 == (nOptions & SDRINSERT_DONTMARK));

    if (bMark)
    {
        MarkObj(*pObj);
    }
}

BitmapEx SdrExchangeView::GetMarkedObjBitmapEx(bool bNoVDevIfOneBmpMarked) const
{
    BitmapEx aBmp;

    if(areSdrObjectsSelected())
    {
        if( bNoVDevIfOneBmpMarked )
        {
            SdrGrafObj* pSdrGrafObj(dynamic_cast< SdrGrafObj* >(getSelectedIfSingle()));

            if(pSdrGrafObj && (GRAPHIC_BITMAP == pSdrGrafObj->GetGraphicType()))
            {
                if(pSdrGrafObj->isEmbeddedSvg())
                {
                    aBmp = pSdrGrafObj->GetGraphic().getSvgData()->getReplacement();
                }
                else
                {
                    aBmp = pSdrGrafObj->GetTransformedGraphic().GetBitmap();
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

    if(areSdrObjectsSelected())
    {
        const Rectangle aBound(sdr::legacy::GetAllObjBoundRect(getSelectedSdrObjectVectorFromSdrMarkView()));
        const Size aBoundSize(aBound.GetWidth(), aBound.GetHeight());
        const MapMode aMap(getSdrModelFromSdrView().GetExchangeObjectUnit(), Point(), getSdrModelFromSdrView().GetExchangeObjectScale(), getSdrModelFromSdrView().GetExchangeObjectScale());

        if( bNoVDevIfOneMtfMarked )
        {
            SdrGrafObj* pSdrGrafObj(dynamic_cast< SdrGrafObj* >(getSelectedIfSingle()));

            if( pSdrGrafObj )
            {
                Graphic aGraphic( pSdrGrafObj->GetTransformedGraphic() );

                // #119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                aMtf = aGraphic.GetGDIMetaFile();
            }
        }

        if( !aMtf.GetActionCount() )
        {
            VirtualDevice aOut;
            const Size aDummySize(2, 2);

            aOut.SetOutputSizePixel( aDummySize );
            aOut.EnableOutput(false);
            aOut.SetMapMode( aMap );
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
            // the created MetaFile. The mentioned errors occur at output time since the integer
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

    if(areSdrObjectsSelected())
    {
        const SdrObject* pSingleSelected = getSelectedIfSingle();

        if(pSingleSelected)
        {
            aRet = GetObjGraphic(*pSingleSelected);
        }
        else
        {
            aRet = GetMarkedObjMetaFile(false);
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------------

Graphic GetObjGraphic(const SdrObject& rObj)
{
    Graphic aRet;

    // try to get a graphic from the object first
    const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(&rObj);
    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(&rObj);

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
        {
            aRet = *pSdrOle2Obj->GetGraphic();
        }
    }

        // if graphic could not be retrieved => go the hard way and create a MetaFile
    if(GRAPHIC_NONE == aRet.GetType() || GRAPHIC_DEFAULT == aRet.GetType())
    {
        const SdrModel& rSdrModel = rObj.getSdrModelFromSdrObject();
        const MapMode aMap(
            rSdrModel.GetExchangeObjectUnit(),
            Point(),
            rSdrModel.GetExchangeObjectScale(),
            rSdrModel.GetExchangeObjectScale());
        VirtualDevice aOut;
        GDIMetaFile aMtf;

        aOut.EnableOutput(false);
        aOut.SetMapMode(aMap);
        aMtf.Record(&aOut);
        rObj.SingleObjectPainter(aOut);
        aMtf.Stop();
        aMtf.WindStart();

        if(aMtf.GetActionCount())
        {
            // #i99268# replace the original offset from using XOutDev's SetOffset
            // NOT (as tried with #i92760#) with another MapMode which gets recorded
            // by the Metafile itself (what always leads to problems), but by
            // translating the result the hard way
            const basegfx::B2DRange aBoundRange(rObj.getObjectRange(0));
            aMtf.Move(-basegfx::fround(aBoundRange.getMinX()), -basegfx::fround(aBoundRange.getMinY()));

            aMtf.SetPrefMapMode(aMap);
            aMtf.SetPrefSize(Size(basegfx::fround(aBoundRange.getWidth()), basegfx::fround(aBoundRange.getHeight())));
            aRet = aMtf;
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------------

::std::vector< SdrObject* > SdrExchangeView::GetMarkedObjects() const
{
    ::std::vector< SdrObject* > aRetval;

    if(areSdrObjectsSelected())
    {
        ::std::vector< SdrObjectVector > aObjVectors(2);
        SdrObjectVector& rObjVector1 = aObjVectors[ 0 ];
        SdrObjectVector& rObjVector2 = aObjVectors[ 1 ];
        const SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrView().GetModelLayerAdmin();
        const sal_uInt32 nControlLayerId(rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), false));
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        sal_uInt32 n, nCount;

        for(n = 0, nCount = aSelection.size(); n < nCount; n++)
        {
            // objects on control layer first - to have them on top of all other objects
            if(nControlLayerId == aSelection[n]->GetLayer())
            {
                rObjVector2.push_back( aSelection[n] );
            }
            else
            {
                rObjVector1.push_back( aSelection[n] );
            }
        }

        for( n = 0, nCount = aObjVectors.size(); n < nCount; n++ )
        {
            SdrObjectVector& rObjVector = aObjVectors[ n ];

            for( sal_uInt32 i = 0; i < rObjVector.size(); i++ )
            {
                aRetval.push_back(rObjVector[ i ]);
            }
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
        sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aSdrObjects, aSdrObjects[0]->getSdrPageFromSdrObject());
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
    SdrModel* pNeuMod = getSdrModelFromSdrView().AllocModel();
    SdrPage* pNeuPag = pNeuMod->AllocPage(false);
    pNeuMod->InsertPage(pNeuPag);

    if( !mxSelectionController.is() || !mxSelectionController->GetMarkedObjModel( pNeuPag ) )
    {
        ::std::vector< SdrObjectVector > aObjVectors(2);
        SdrObjectVector& rObjVector1 = aObjVectors[ 0 ];
        SdrObjectVector& rObjVector2 = aObjVectors[ 1 ];
        const SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrView().GetModelLayerAdmin();
        const sal_uInt32 nControlLayerId(rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), false));
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        sal_uInt32 n, nCount, nCloneErrCnt = 0;

        for(n = 0, nCount = aSelection.size(); n < nCount; n++)
        {
            // prefer objects on control layer - to have them on top of all other objects
            if(nControlLayerId == aSelection[n]->GetLayer())
            {
                rObjVector2.push_back( aSelection[n] );
            }
            else
            {
                rObjVector1.push_back( aSelection[n] );
            }
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for( n = 0, nCount = aObjVectors.size(); n < nCount; n++ )
        {
            SdrObjectVector& rObjVector = aObjVectors[ n ];

            for( sal_uInt32 i = 0; i < rObjVector.size(); i++ )
            {
                const SdrObject* pObj = rObjVector[ i ];
                SdrObject*          pNeuObj;

                if(dynamic_cast< const SdrPageObj* >(pObj))
                {
                    // convert SdrPageObj's to a graphic representation, because
                    // virtual connection to referenced page gets lost in new model
                    pNeuObj = new SdrGrafObj(
                        *pNeuMod,
                        GetObjGraphic(*pObj),
                        pObj->getSdrObjectTransformation());
                }
                else
                {
                    // #116235#
                    pNeuObj = pObj->CloneSdrObject(pNeuMod);
                }

                if( pNeuObj )
                {
                    pNeuPag->InsertObjectToSdrObjList(*pNeuObj);

                    // #i13033#
                    aCloneList.AddPair(pObj, pNeuObj);
                }
                else
                {
                    nCloneErrCnt++;
                }
            }
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();

        if(nCloneErrCnt)
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

bool SdrExchangeView::Cut( sal_uInt32 /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::Cut: Not supported anymore" );
    return false;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::CutMarked( sal_uInt32 /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::CutMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

bool SdrExchangeView::Yank(sal_uInt32 /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Yank: Not supported anymore" );
    return false;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::YankMarked(sal_uInt32 /*nFormat*/)
{
    DBG_ERROR( "YankMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

bool SdrExchangeView::Paste(Window* /*pWin*/, sal_uInt32 /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Paste: Not supported anymore" );
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
