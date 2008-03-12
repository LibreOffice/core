/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdxcgv.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:57:56 $
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
#include "precompiled_svx.hxx"

#include <vector>
#include <svx/editeng.hxx>
#include "xexch.hxx"
#include <svx/xflclit.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdoutl.hxx>
#include "svditext.hxx"
#include <svx/svdetc.hxx>
#include "svdxout.hxx"
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx> // fuer kein OLE im SdrClipboardFormat
#include <svx/svdorect.hxx>
#include <svx/svdoedge.hxx> // fuer Konnektoren uebers Clipboard
#include <svx/svdopage.hxx> // fuer Konnektoren uebers Clipboard
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx> // Fuer GetMapFactor zum umskalieren bei PasteModel
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "xoutbmp.hxx"

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

// #i13033#
#ifndef _CLONELIST_HXX_
#include <clonelist.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

// b4967543
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

// #i72535#
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

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

BOOL SdrExchangeView::ImpLimitToWorkArea(Point& rPt) const
{
    BOOL bRet(FALSE);

    if(!aMaxWorkArea.IsEmpty())
    {
        if(rPt.X()<aMaxWorkArea.Left())
        {
            rPt.X() = aMaxWorkArea.Left();
            bRet = TRUE;
        }

        if(rPt.X()>aMaxWorkArea.Right())
        {
            rPt.X() = aMaxWorkArea.Right();
            bRet = TRUE;
        }

        if(rPt.Y()<aMaxWorkArea.Top())
        {
            rPt.Y() = aMaxWorkArea.Top();
            bRet = TRUE;
        }

        if(rPt.Y()>aMaxWorkArea.Bottom())
        {
            rPt.Y() = aMaxWorkArea.Bottom();
            bRet = TRUE;
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

BOOL SdrExchangeView::ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const
{
    BOOL bRet=FALSE;
    rLayer=0;
    if (pObjList!=NULL) {
        const SdrPage* pPg=pObjList->GetPage();
        if (pPg!=NULL) {
            rLayer=pPg->GetLayerAdmin().GetLayerID(aAktLayer,TRUE);
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

BOOL SdrExchangeView::Paste(const GDIMetaFile& rMtf, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return FALSE;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return FALSE;
    BOOL bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    SdrGrafObj* pObj=new SdrGrafObj(Graphic(rMtf));
    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj,*pLst,aPos,rMtf.GetPrefSize(),rMtf.GetPrefMapMode(),nOptions);
    return TRUE;
}

BOOL SdrExchangeView::Paste(const Bitmap& rBmp, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return FALSE;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return FALSE;
    BOOL bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();
    SdrGrafObj* pObj=new SdrGrafObj(Graphic(rBmp));
    pObj->SetLayer(nLayer);
    ImpPasteObject(pObj,*pLst,aPos,rBmp.GetSizePixel(),MapMode(MAP_PIXEL),nOptions);
    return TRUE;
}

BOOL SdrExchangeView::Paste(const XubString& rStr, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    if(!rStr.Len())
        return FALSE;

    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return FALSE;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return FALSE;
    BOOL bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
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
    return TRUE;
}

BOOL SdrExchangeView::Paste(SvStream& rInput, const String& rBaseURL, USHORT eFormat, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    ImpLimitToWorkArea( aPos );
    if (pLst==NULL) return FALSE;
    SdrLayerID nLayer;
    if (!ImpGetPasteLayer(pLst,nLayer)) return FALSE;
    BOOL bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
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

    return TRUE;
}

BOOL SdrExchangeView::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    const SdrModel* pSrcMod=&rMod;
    if (pSrcMod==pMod) return FALSE; // na so geht's ja nun nicht

    BegUndo(ImpGetResStr(STR_ExchangePaste));

    if( mxSelectionController.is() && mxSelectionController->PasteObjModel( rMod ) )
    {
        EndUndo();
        return TRUE;
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
    if (pLst==NULL) return FALSE;
    BOOL bUnmark=(nOptions&(SDRINSERT_DONTMARK|SDRINSERT_ADDMARK))==0 && !IsTextEdit();
    if (bUnmark) UnmarkAllObj();

    // evtl. umskalieren bei unterschiedlicher MapUnit am Model
    // Dafuer erstmal die Faktoren berechnen
    MapUnit eSrcUnit=pSrcMod->GetScaleUnit();
    MapUnit eDstUnit=pMod->GetScaleUnit();
    BOOL bResize=eSrcUnit!=eDstUnit;
    Fraction xResize,yResize;
    Point aPt0;
    if (bResize) {
        FrPair aResize(GetMapFactor(eSrcUnit,eDstUnit));
        xResize=aResize.X();
        yResize=aResize.Y();
    }
    SdrObjList*  pDstLst=pLst;
    USHORT nPg,nPgAnz=pSrcMod->GetPageCount();
    for (nPg=0; nPg<nPgAnz; nPg++) {
        const SdrPage* pSrcPg=pSrcMod->GetPage(nPg);

        // #104148# Use SnapRect, not BoundRect here
        Rectangle aR=pSrcPg->GetAllObjSnapRect();

        if (bResize) ResizeRect(aR,aPt0,xResize,yResize);
        Point aDist(aPos-aR.Center());
        Size  aSiz(aDist.X(),aDist.Y());
        //ULONG nDstObjAnz0=pDstLst->GetObjCount();
        ULONG nCloneErrCnt=0;
        ULONG nOb,nObAnz=pSrcPg->GetObjCount();
        BOOL bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for (nOb=0; nOb<nObAnz; nOb++) {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

            // #116235#
            SdrObject* pNeuObj = pSrcOb->Clone();

            if (pNeuObj!=NULL)
            {
                if(bResize)
                {
                    pNeuObj->GetModel()->SetPasteResize(TRUE); // #51139#
                    pNeuObj->NbcResize(aPt0,xResize,yResize);
                    pNeuObj->GetModel()->SetPasteResize(FALSE); // #51139#
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
                        nLayer = rAd.GetLayerID(aAktLayer, TRUE);
                    }

                    if(SDRLAYER_NOTFOUND == nLayer)
                    {
                        nLayer = 0;
                    }

                    pNeuObj->SetLayer(nLayer);
                }

                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pDstLst->InsertObject(pNeuObj,CONTAINER_APPEND,&aReason);

                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                if (bMark) {
                    // Markhandles noch nicht sofort setzen!
                    // Das erledigt das ModelHasChanged der MarkView.
                    MarkObj(pNeuObj,pMarkPV,FALSE,TRUE);
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
    EndUndo();
    return TRUE;
}

BOOL SdrExchangeView::IsExchangeFormatSupported(ULONG nFormat) const
{
    return( FORMAT_PRIVATE == nFormat ||
            FORMAT_GDIMETAFILE == nFormat ||
            FORMAT_BITMAP == nFormat ||
            FORMAT_RTF == nFormat ||
            FORMAT_STRING == nFormat ||
            SOT_FORMATSTR_ID_DRAWING == nFormat ||
            SOT_FORMATSTR_ID_EDITENGINE == nFormat );
}

void SdrExchangeView::ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const Point& rCenter, const Size& rSiz, const MapMode& rMap, UINT32 nOptions)
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
    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));
    SdrPageView* pMarkPV=NULL;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        if (pPV->GetObjList()==&rLst) pMarkPV=pPV;
    }

    BOOL bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;
    if (bMark) { // Obj in der ersten gefundenen PageView markieren
        MarkObj(pObj,pMarkPV);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Bitmap SdrExchangeView::GetMarkedObjBitmap( BOOL bNoVDevIfOneBmpMarked ) const
{
    Bitmap aBmp;

    if( AreObjectsMarked() )
    {
        if( bNoVDevIfOneBmpMarked )
        {
            SdrObject*  pGrafObjTmp = GetMarkedObjectByIndex( 0 );
            SdrGrafObj* pGrafObj = ( GetMarkedObjectCount() == 1 ) ? PTR_CAST( SdrGrafObj, pGrafObjTmp ) : NULL;

            if( pGrafObj && ( pGrafObj->GetGraphicType() == GRAPHIC_BITMAP ) )
                aBmp = pGrafObj->GetTransformedGraphic().GetBitmap();
        }

        if( !aBmp )
        {
            const Graphic aGraphic( GetMarkedObjMetaFile( bNoVDevIfOneBmpMarked ) );
            aBmp = aGraphic.GetBitmap();
        }
    }

    return aBmp;
}

// -----------------------------------------------------------------------------

GDIMetaFile SdrExchangeView::GetMarkedObjMetaFile( BOOL bNoVDevIfOneMtfMarked ) const
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

                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    const Point aPos;

                    aMtf.AddAction( new MetaBmpExScaleAction( aPos, aBoundSize, aGraphic.GetBitmapEx() ) );
                    aMtf.SetPrefMapMode( aMap );
                    aMtf.SetPrefSize( aBoundSize );
                }
                else
                    aMtf = aGraphic.GetGDIMetaFile();
            }
        }

        if( !aMtf.GetActionCount() )
        {
            VirtualDevice   aOut;
            Size            aDummySize( 2, 2 );

            aOut.SetOutputSizePixel( aDummySize );
            aOut.EnableOutput( FALSE );
            aOut.SetMapMode( aMap );

            aMtf.Clear();
            aMtf.Record( &aOut );

            DrawMarkedObj( aOut, aBound.TopLeft() );

            aMtf.Stop();
            aMtf.WindStart();
            aMtf.SetPrefMapMode( aMap );

            // #i8506# Add something to the prefsize, to prevent
            // the draw shapes from clipping away the right/bottom-
            // most line. Honestly, I have not the slightest idea
            // why exactly 32 does the trick here, but that's the
            // smallest number which still works even for the highest
            // zoom level.
            // See also #108486# for further details.
            aMtf.SetPrefSize( Size(aBoundSize.Width()+32,
                                   aBoundSize.Height()+32) );
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
            aRet = GetMarkedObjMetaFile( FALSE );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

Graphic SdrExchangeView::GetObjGraphic( SdrModel* pModel, SdrObject* pObj )
{
    Graphic aRet;

    if( pModel && pObj )
    {
        // try to get a graphic from the object first
        if( pObj->ISA( SdrGrafObj ) )
        {
            // #110981# Make behaviour coherent with metafile
            // recording below (which of course also takes
            // view-transformed objects)
            aRet = static_cast< SdrGrafObj* >( pObj )->GetTransformedGraphic();
        }
        else if( pObj->ISA( SdrOle2Obj ) )
        {
            SdrOle2Obj* pOLEObj = static_cast< SdrOle2Obj* >( pObj );
            if ( pOLEObj->GetGraphic() )
                aRet = *pOLEObj->GetGraphic();
        }

        // if graphic could not be retrieved => go the hard way and create a MetaFile
        if( ( GRAPHIC_NONE == aRet.GetType() ) || ( GRAPHIC_DEFAULT == aRet.GetType() ) )
        {
            VirtualDevice   aOut;
            XOutputDevice aXOut( &aOut);
            SdrPaintInfoRec aInfoRec;
            GDIMetaFile     aMtf;
            const Rectangle aBoundRect( pObj->GetCurrentBoundRect() );
            const MapMode   aMap( pModel->GetScaleUnit(),
                                  Point(),
                                  pModel->GetScaleFraction(),
                                  pModel->GetScaleFraction() );

            aOut.EnableOutput( FALSE );
            aOut.SetMapMode( aMap );
            aMtf.Record( &aOut );

            aXOut.SetOffset( Point( -aBoundRect.Left(), -aBoundRect.Top() ) );
            aInfoRec.nPaintMode |= SDRPAINTMODE_ANILIKEPRN;
            pObj->SingleObjectPainter( aXOut, aInfoRec ); // #110094#-17

            aMtf.Stop();
            aMtf.WindStart();
            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundRect.GetSize() );

            if( aMtf.GetActionCount() )
                aRet = aMtf;
        }
     }

     return aRet;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::DrawMarkedObj(OutputDevice& rOut, const Point& rOfs) const
{
    SortMarkedObjects();
    pXOut->SetOutDev(&rOut);
    SdrPaintInfoRec aInfoRec;
    aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;

    ::std::vector< ::std::vector< SdrMark* > >  aObjVectors( 2 );
    ::std::vector< SdrMark* >&                  rObjVector1 = aObjVectors[ 0 ];
    ::std::vector< SdrMark* >&                  rObjVector2 = aObjVectors[ 1 ];
    const SdrLayerAdmin&                        rLayerAdmin = pMod->GetLayerAdmin();
    const sal_uInt32                            nControlLayerId = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), FALSE );
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
            Point       aOfs( -rOfs.X(),-rOfs.Y() );

            if( aOfs != pXOut->GetOffset() )
                pXOut->SetOffset(aOfs);

            pMark->GetMarkedSdrObj()->SingleObjectPainter( *pXOut, aInfoRec ); // #110094#-17
        }
    }

    pXOut->SetOffset( Point(0,0) );
}

// -----------------------------------------------------------------------------

SdrModel* SdrExchangeView::GetMarkedObjModel() const
{
    // Wenn das sortieren der MarkList mal stoeren sollte,
    // werde ich sie mir wohl kopieren muessen.
    SortMarkedObjects();
    SdrModel* pNeuMod=pMod->AllocModel();
    SdrPage* pNeuPag=pNeuMod->AllocPage(FALSE);
    pNeuMod->InsertPage(pNeuPag);

    if( !mxSelectionController.is() || !mxSelectionController->GetMarkedObjModel( pNeuPag ) )
    {
        ::std::vector< ::std::vector< SdrMark* > >  aObjVectors( 2 );
        ::std::vector< SdrMark* >&                  rObjVector1 = aObjVectors[ 0 ];
        ::std::vector< SdrMark* >&                  rObjVector2 = aObjVectors[ 1 ];
        const SdrLayerAdmin&                        rLayerAdmin = pMod->GetLayerAdmin();
        const sal_uInt32                            nControlLayerId = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), FALSE );
        sal_uInt32                                  n, nCount, nCloneErrCnt = 0;

        for( n = 0, nCount = GetMarkedObjectCount(); n < nCount; n++ )
        {
            SdrMark* pMark = GetSdrMarkByIndex( n );

            // paint objects on control layer on top of all otherobjects
            if( nControlLayerId == pMark->GetMarkedSdrObj()->GetLayer() )
                rObjVector2.push_back( pMark );
            else
                rObjVector1.push_back( pMark );
        }

        // #i13033#
        // New mechanism to re-create the connections of cloned connectors
        CloneList aCloneList;

        for( n = 0, nCount = aObjVectors.size(); n < nCount; n++ )
        {
            ::std::vector< SdrMark* >& rObjVector = aObjVectors[ n ];

            for( sal_uInt32 i = 0; i < rObjVector.size(); i++ )
            {
                   const SdrMark*      pMark = rObjVector[ i ];
                const SdrObject*    pObj = pMark->GetMarkedSdrObj();
                SdrObject*          pNeuObj;

                if( pObj->ISA( SdrPageObj ) )
                {
                    // convert SdrPageObj's to a graphic representation, because
                    // virtual connection to referenced page gets lost in new model
                    pNeuObj = new SdrGrafObj( GetObjGraphic( pMod, const_cast< SdrObject* >( pObj ) ), pObj->GetLogicRect() );
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

BOOL SdrExchangeView::Cut( ULONG /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::Cut: Not supported anymore" );
    return FALSE;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::CutMarked( ULONG /*nFormat */)
{
    DBG_ERROR( "SdrExchangeView::CutMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

BOOL SdrExchangeView::Yank(ULONG /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Yank: Not supported anymore" );
    return FALSE;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::YankMarked(ULONG /*nFormat*/)
{
    DBG_ERROR( "YankMarked: Not supported anymore" );
}

// -----------------------------------------------------------------------------

BOOL SdrExchangeView::Paste(Window* /*pWin*/, ULONG /*nFormat*/)
{
    DBG_ERROR( "SdrExchangeView::Paste: Not supported anymore" );
    return FALSE;
}
