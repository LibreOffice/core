/*************************************************************************
 *
 *  $RCSfile: svdxcgv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "editeng.hxx"

#include "xexch.hxx"
#include "xflclit.hxx"

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#include "svdxcgv.hxx"
#include "svdoutl.hxx"
#include "svditext.hxx"
#include "svdetc.hxx"
#include "svdxout.hxx"
#include "svdundo.hxx"
#include "svdograf.hxx"
#include "svdoole2.hxx" // fuer kein OLE im SdrClipboardFormat
#include "svdorect.hxx"
#include "svdoedge.hxx" // fuer Konnektoren uebers Clipboard
#include "svdpage.hxx"
#include "svdpagv.hxx"
#include "svdtrans.hxx" // Fuer GetMapFactor zum umskalieren bei PasteModel
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "xoutbmp.hxx"

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

#ifndef _DTRANS_HXX
#include <so3/dtrans.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrExchangeView::SdrExchangeView(SdrModel* pModel1, OutputDevice* pOut):
    SdrObjEditView(pModel1,pOut)
{
}

SdrExchangeView::SdrExchangeView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrObjEditView(pModel1,pXOut)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Point SdrExchangeView::GetViewCenter(const OutputDevice* pOut) const
{
    Point aCenter;
    if (pOut==NULL) pOut=GetWin(0);
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

BOOL SdrExchangeView::ImpLimitToWorkArea(Point& rPt, const SdrPageView* pPV) const
{
    BOOL bRet(FALSE);

    if(!aMaxWorkArea.IsEmpty())
    {
        if(pPV)
            rPt += pPV->GetOffset();

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

        if(pPV)
            rPt -= pPV->GetOffset();
    }
    return bRet;
}

void SdrExchangeView::ImpGetPasteObjList(Point& rPos, SdrObjList*& rpLst)
{
    if (rpLst==NULL) {
        SdrPageView* pPV=GetPageView(rPos);
        if (pPV!=NULL) {
            rpLst=pPV->GetObjList();
            rPos-=pPV->GetOffset();
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
            SdrPageView* pPV=GetPageView(pPg);
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
    SdrPageView* pMarkPV=NULL;
    for ( USHORT nv = 0; nv < GetPageViewCount() && !pMarkPV; nv++ )
    {
        SdrPageView* pPV = GetPageViewPvNum(nv);
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos, pMarkPV );
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
    SdrPageView* pMarkPV=NULL;
    for ( USHORT nv = 0; nv < GetPageViewCount() && !pMarkPV; nv++ )
    {
        SdrPageView* pPV = GetPageViewPvNum(nv);
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos, pMarkPV );
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
    SdrPageView* pMarkPV=NULL;
    for ( USHORT nv = 0; nv < GetPageViewCount() && !pMarkPV; nv++ )
    {
        SdrPageView* pPV = GetPageViewPvNum(nv);
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos, pMarkPV );
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
    if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet,FALSE);
    pObj->NbcSetAttributes(aDefaultAttr,FALSE);
    SfxItemSet aTempAttr(pMod->GetItemPool());  // Keine Fuellung oder Linie
    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));
    pObj->NbcSetAttributes(aTempAttr, FALSE);
    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=pMod->GetScaleUnit();
    Fraction aMap=pMod->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);
    return TRUE;
}

BOOL SdrExchangeView::Paste(SvStream& rInput, USHORT eFormat, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    SdrPageView* pMarkPV=NULL;
    for ( USHORT nv = 0; nv < GetPageViewCount() && !pMarkPV; nv++ )
    {
        SdrPageView* pPV = GetPageViewPvNum(nv);
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos, pMarkPV );
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
    if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet,FALSE);
    pObj->NbcSetAttributes(aDefaultAttr,FALSE);
    SfxItemSet aTempAttr(pMod->GetItemPool());  // Keine Fuellung oder Linie
    aTempAttr.Put(XLineStyleItem(XLINE_NONE));
    aTempAttr.Put(XFillStyleItem(XFILL_NONE));
    pObj->NbcSetAttributes(aTempAttr, FALSE);
    pObj->NbcSetText(rInput,eFormat);
    pObj->FitFrameToTextSize();
    Size aSiz(pObj->GetLogicRect().GetSize());
    MapUnit eMap=pMod->GetScaleUnit();
    Fraction aMap=pMod->GetScaleFraction();
    ImpPasteObject(pObj,*pLst,aPos,aSiz,MapMode(eMap,Point(0,0),aMap,aMap),nOptions);
    return TRUE;
}

BOOL SdrExchangeView::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, UINT32 nOptions)
{
    const SdrModel* pSrcMod=&rMod;
    if (pSrcMod==pMod) return FALSE; // na so geht's ja nun nicht
    Point aPos(rPos);
    ImpGetPasteObjList(aPos,pLst);
    SdrPageView* pMarkPV=NULL;
    for ( USHORT nv = 0; nv < GetPageViewCount() && !pMarkPV; nv++ )
    {
        SdrPageView* pPV = GetPageViewPvNum(nv);
        if ( pPV->GetObjList() == pLst )
            pMarkPV=pPV;
    }

    ImpLimitToWorkArea( aPos, pMarkPV );
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
    BegUndo(ImpGetResStr(STR_ExchangePaste));
    for (nPg=0; nPg<nPgAnz; nPg++) {
        const SdrPage* pSrcPg=pSrcMod->GetPage(nPg);
        Rectangle aR=pSrcPg->GetAllObjBoundRect();
        if (bResize) ResizeRect(aR,aPt0,xResize,yResize);
        Point aDist(aPos-aR.Center());
        Size  aSiz(aDist.X(),aDist.Y());
        ULONG nDstObjAnz0=pDstLst->GetObjCount();
        ULONG nCloneErrCnt=0;
        ULONG nOb,nObAnz=pSrcPg->GetObjCount();
        BOOL bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;
        for (nOb=0; nOb<nObAnz; nOb++) {
            const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);
            SdrObject* pNeuObj=pSrcOb->Clone(pDstLst->GetPage(),pDstLst->GetModel());
            if (pNeuObj!=NULL) {
                if (bResize) {
                    pNeuObj->GetModel()->SetPasteResize(TRUE); // #51139#
                    pNeuObj->NbcResize(aPt0,xResize,yResize);
                    pNeuObj->GetModel()->SetPasteResize(FALSE); // #51139#
                }
                pNeuObj->NbcMove(aSiz);

                const SdrPage* pPg = pDstLst->GetPage();
                if ( pPg )
                    pNeuObj->SetLayer( pPg->GetLayerAdmin().GetLayerID(aAktLayer, TRUE) );

                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pDstLst->InsertObject(pNeuObj,CONTAINER_APPEND,&aReason);

                AddUndo(new SdrUndoNewObj(*pNeuObj));
                if (bMark) {
                    // Markhandles noch nicht sofort setzen!
                    // Das erledigt das ModelHasChanged der MarkView.
                    MarkObj(pNeuObj,pMarkPV,FALSE,TRUE);
                }
            } else {
                nCloneErrCnt++;
            }
        }
        // und nun zu den Konnektoren
        // Die neuen Objekte in pDstLst werden auf die der pSrcPg abgebildet
        // und so die Objektverbindungen hergestellt.
        // Aehnliche Implementation an folgenden Stellen:
        //    void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
        //    SdrModel* SdrExchangeView::GetMarkedObjModel() const
        //    BOOL SdrExchangeView::Paste(const SdrModel& rMod,...)
        //    void SdrEditView::CopyMarked()
        if (nCloneErrCnt==0) {
            for (nOb=0; nOb<nObAnz; nOb++) {
                const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);
                SdrEdgeObj* pSrcEdge=PTR_CAST(SdrEdgeObj,pSrcOb);
                if (pSrcEdge!=NULL) {
                    SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(TRUE);
                    SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(FALSE);
                    if (pSrcNode1!=NULL && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=NULL; // Listenuebergreifend
                    if (pSrcNode2!=NULL && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=NULL; // ist (noch) nicht
                    if (pSrcNode1!=NULL || pSrcNode2!=NULL) {
                        SdrObject* pDstEdgeTmp=pDstLst->GetObj(nDstObjAnz0+nOb);
                        SdrEdgeObj* pDstEdge=PTR_CAST(SdrEdgeObj,pDstEdgeTmp);
                        if (pDstEdge!=NULL) {
                            if (pSrcNode1!=NULL) {
                                ULONG nDstNode1=pSrcNode1->GetOrdNum();
                                SdrObject* pDstNode1=pDstLst->GetObj(nDstNode1+nDstObjAnz0);
                                if (pDstNode1!=NULL) { // Sonst grober Fehler!
                                    pDstEdge->ConnectToNode(TRUE,pDstNode1);
                                } else {
                                    DBG_ERROR("SdrExchangeView::Paste(): pDstNode1==NULL!");
                                }
                            }
                            if (pSrcNode2!=NULL) {
                                ULONG nDstNode2=pSrcNode2->GetOrdNum();
                                SdrObject* pDstNode2=pDstLst->GetObj(nDstNode2+nDstObjAnz0);
                                if (pDstNode2!=NULL) { // Node war sonst wohl nicht markiert
                                    pDstEdge->ConnectToNode(FALSE,pDstNode2);
                                } else {
                                    DBG_ERROR("SdrExchangeView::Paste(): pDstNode2==NULL!");
                                }
                            }
                        } else {
                            DBG_ERROR("SdrExchangeView::Paste(): pDstEdge==NULL!");
                        }
                    }
                }
            }
        } else {
#ifdef DBG_UTIL
            ByteString aStr("SdrExchangeView::Paste(): Fehler beim Clonen ");

            if(nCloneErrCnt == 1)
            {
                aStr += "eines Zeichenobjekts.";
            }
            else
            {
                aStr += "von ";
                aStr += nCloneErrCnt;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrExchangeView::CutMarked(ULONG nFormat)
{
    YankMarked(nFormat);
    XubString aStr;
    ImpTakeDescriptionStr(STR_ExchangeClpCut,aStr);
    BegUndo(aStr);
    DeleteMarkedObj();
    EndUndo();
}

void SdrExchangeView::ImpYank(ULONG nFormat, BOOL bClp) const
{
    if(HasMarkedObj())
    {
        String aStrEditEngineFormat("EditEngineFormat", gsl_getSystemTextEncoding());
        UINT32 nSdrFormat(SOT_FORMATSTR_ID_DRAWING);
        UINT32 nEEFormat(Exchange::RegisterFormatName(aStrEditEngineFormat));

        if(bClp)
        {
            // 1. Private
        }

        // 2. SdrExchangeFormat
        if (nFormat==SDR_ANYFORMAT || nFormat==nSdrFormat) {
            SdrModel* pModel=GetMarkedObjModel();
            pModel->SetStreamingSdrModel(TRUE);
            pModel->RemoveNotPersistentObjects(TRUE); // OLE, etc. entfernen
            SvMemoryStream aMemStream(4096,4096);
            aMemStream.SetVersion(SOFFICE_FILEFORMAT_NOW);
            // StyleSheetPool und Persist fehlt hier wohl noch ...
            pModel->GetItemPool().Store(aMemStream);
            aMemStream<<*pModel;
            if (bClp) Clipboard::CopyData(aMemStream.GetData(),aMemStream.GetSize(),nSdrFormat);
            else DragServer::CopyData(aMemStream.GetData(),aMemStream.GetSize(),nSdrFormat);
            delete pModel;
        }
        // 3. Metafile
        if (nFormat==SDR_ANYFORMAT || nFormat==FORMAT_GDIMETAFILE) {
            GDIMetaFile aMtf(GetMarkedObjMetaFile());
            if (bClp) Clipboard::CopyGDIMetaFile(aMtf);
            else DragServer::CopyGDIMetaFile(aMtf);
        }
        // 4. Bitmap
        if (nFormat==SDR_ANYFORMAT || nFormat==FORMAT_BITMAP) {
            Bitmap aBmp(GetMarkedObjBitmap());
            if (bClp) Clipboard::CopyBitmap(aBmp);
            else DragServer::CopyBitmap(aBmp);
        }
        // 5. EditEngine, 6. RTF, 7. Text
        if (nFormat==SDR_ANYFORMAT || nFormat==nEEFormat || nFormat==FORMAT_RTF || nFormat==FORMAT_STRING) {
            if (aMark.GetMarkCount()==1) {
                const SdrObject* pObj=aMark.GetMark(0)->GetObj();
                const OutlinerParaObject* pOLPara=pObj->GetOutlinerParaObject();
                if (pOLPara!=NULL) {
                    const SdrTextObj* pText=PTR_CAST(SdrTextObj,pObj); // DrawOutliner ggf. mit Rahmendefaults
                    SdrOutliner& rOutliner=pText!=NULL ? pText->ImpGetDrawOutliner() : pMod->GetDrawOutliner();
                    rOutliner.SetUpdateMode(TRUE);
                    rOutliner.SetText(*pOLPara);
                    XubString aStr(rOutliner.GetText(rOutliner.GetParagraph( 0 ), rOutliner.GetParagraphCount()));

                    if(aStr.Len())
                    {
                        if (nFormat==SDR_ANYFORMAT || nFormat==nEEFormat) {
                            SvMemoryStream aMemStream(4096,4096);
                            aMemStream.SetVersion(SOFFICE_FILEFORMAT_NOW);
                            ((EditEngine&)rOutliner.GetEditEngine()).Write(aMemStream,EE_FORMAT_BIN);
                            if (bClp) Clipboard::CopyData(aMemStream.GetData(),aMemStream.GetSize(),nEEFormat);
                            else DragServer::CopyData(aMemStream.GetData(),aMemStream.GetSize(),nEEFormat);
                        }
                        if (nFormat==SDR_ANYFORMAT || nFormat==FORMAT_RTF) {
                            SvMemoryStream aMemStream(4096,4096);
                            ((EditEngine&)rOutliner.GetEditEngine()).Write(aMemStream,EE_FORMAT_RTF);
                            if (bClp) Clipboard::CopyData(aMemStream.GetData(),aMemStream.GetSize(),FORMAT_RTF);
                            else DragServer::CopyData(aMemStream.GetData(),aMemStream.GetSize(),FORMAT_RTF);
                        }
                        if (nFormat==SDR_ANYFORMAT || nFormat==FORMAT_STRING) {
                            if (bClp) Clipboard::CopyString(aStr);
                            else DragServer::CopyString(aStr);
                        }
                    }
                }
            }
        }
    }
}

void SdrExchangeView::YankMarked(ULONG nFormat)
{
    if (HasMarkedObj()) {
        Clipboard::Clear();
        ImpYank(nFormat,TRUE);
    }
}

DropAction SdrExchangeView::DragDropMarked(Window& rWin, ULONG nFormat)
{
    if (HasMarkedObj()) {
        DragServer::Clear();
        BrkAction();
        ImpYank(nFormat,FALSE);
        SdrModel* pDragModel=NULL;
        if (nFormat==SDR_ANYFORMAT || nFormat==FORMAT_PRIVATE) {
            pDragModel=GetMarkedObjModel();
            DragServer::CopyPrivateData(pDragModel);
        }
        Pointer aCopShape(POINTER_COPYDATA);
        Pointer aMovShape(POINTER_MOVEDATA);
        SdrMarkList aOldML(aMark); // alte Marklist merken

        XubString aStr;
        ImpTakeDescriptionStr(STR_ExchangeDD,aStr);
        BegUndo(aStr);
        DropAction eAct=rWin.ExecuteDrag(aMovShape,aCopShape);
        if (pDragModel!=NULL) delete pDragModel;
        if (eAct==DROP_MOVE || eAct==DROP_DISCARD) { // alle Objekte loeschen, die vorher markiert waren
            DeleteMarked(aOldML);
        }
        EndUndo();
        DragServer::Clear();
        return eAct;
    } else return DROP_NONE;
}

BOOL SdrExchangeView::PasteClipboard(OutputDevice* pOut, ULONG nFormat, UINT32 nOptions)
{
    BOOL bRet=FALSE;
    if (!IsTextEdit()) UnmarkAllObj();
    BegUndo(ImpGetResStr(STR_ExchangeClpPaste));
    bRet=ImpPaste(nFormat,TRUE,0,GetViewCenter(pOut),nOptions);
    EndUndo();
    return bRet;
}

BOOL SdrExchangeView::PasteDragDrop(const Point& rPos, ULONG nFormat, USHORT nItemNum, UINT32 nOptions)
{
    BOOL bRet=FALSE;
    if (!IsTextEdit()) UnmarkAllObj();
    BegUndo(ImpGetResStr(STR_ExchangeDDPaste));
    USHORT i=nItemNum;
    USHORT nMax=i;
    if (i==SDR_ANYITEM) {
        i=0;
        nMax=DragServer::GetItemCount();
    } else nMax++;
    while (i<nMax) {
        if (ImpPaste(nFormat,FALSE,i,rPos,nOptions)) bRet=TRUE;
        i++;
    }
    EndUndo();
    return bRet;
}

BOOL SdrExchangeView::IsClipboardFormatSupported(ULONG nFormat) const
{
    BOOL bOk(FALSE);

    if(nFormat == SDR_ANYFORMAT)
    {
        String aStrEditEngineFormat("EditEngineFormat", gsl_getSystemTextEncoding());

        bOk = Clipboard::HasFormat(FORMAT_PRIVATE)     ||
              Clipboard::HasFormat(FORMAT_GDIMETAFILE) ||
              Clipboard::HasFormat(FORMAT_BITMAP)      ||
              Clipboard::HasFormat(FORMAT_RTF)         ||
              Clipboard::HasFormat(FORMAT_STRING)      ||
              Clipboard::HasFormat(SOT_FORMATSTR_ID_DRAWING) ||
              Clipboard::HasFormat(Exchange::RegisterFormatName(aStrEditEngineFormat));
    }
    else
    {
        bOk = IsExchangeFormatSupported(nFormat);
    }

    return bOk;
}

BOOL SdrExchangeView::IsDragDropFormatSupported(ULONG nFormat, USHORT nItemNum) const
{
    BOOL bOk=FALSE;
    USHORT i=nItemNum;
    USHORT nMax=i;

    if(i == SDR_ANYITEM)
    {
        i = 0;
        nMax = DragServer::GetItemCount();
    }
    else
        nMax++;

    while(i < nMax && !bOk)
    {
        if(nFormat == SDR_ANYFORMAT)
        {
            String aStrEditEngineFormat("EditEngineFormat", gsl_getSystemTextEncoding());

            bOk = DragServer::HasFormat(i, FORMAT_PRIVATE)     ||
                  DragServer::HasFormat(i, FORMAT_GDIMETAFILE) ||
                  DragServer::HasFormat(i, FORMAT_BITMAP)      ||
                  DragServer::HasFormat(i, FORMAT_RTF)         ||
                  DragServer::HasFormat(i, FORMAT_STRING)      ||
                  DragServer::HasFormat(i, SOT_FORMATSTR_ID_DRAWING) ||
                  DragServer::HasFormat(i, Exchange::RegisterFormatName(aStrEditEngineFormat));
        }
        else
        {
            bOk = IsExchangeFormatSupported(nFormat);
        }

        i++;
    }

    return bOk;
}

BOOL SdrExchangeView::IsExchangeFormatSupported(ULONG nFormat) const
{
    String aStrEditEngineFormat("EditEngineFormat", gsl_getSystemTextEncoding());
    BOOL bOk(nFormat == FORMAT_PRIVATE     ||
            nFormat == FORMAT_GDIMETAFILE ||
            nFormat == FORMAT_BITMAP      ||
            nFormat == FORMAT_RTF         ||
            nFormat == FORMAT_STRING      ||
            nFormat == SOT_FORMATSTR_ID_DRAWING ||
            nFormat == Exchange::RegisterFormatName(aStrEditEngineFormat));
   return bOk;
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
    AddUndo(new SdrUndoNewObj(*pObj));
    SdrPageView* pMarkPV=NULL;
    for (USHORT nv=0; nv<GetPageViewCount() && pMarkPV==NULL; nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        if (pPV->GetObjList()==&rLst) pMarkPV=pPV;
    }
    BOOL bMark=pMarkPV!=NULL && !IsTextEdit() && (nOptions&SDRINSERT_DONTMARK)==0;
    if (bMark) { // Obj in der ersten gefundenen PageView markieren
        MarkObj(pObj,pMarkPV);
    }
}

BOOL SdrExchangeView::ImpPaste(ULONG nFormat, BOOL bClp, USHORT nItemNum, const Point& rPos, UINT32 nOptions)
{
    BOOL bRet(FALSE);
    String aStrEditEngineFormat("EditEngineFormat", gsl_getSystemTextEncoding());
    UINT32 nSdrFormat(SOT_FORMATSTR_ID_DRAWING);
    UINT32 nEEFormat(Exchange::RegisterFormatName(aStrEditEngineFormat));

    // 1. Prioritaet fuer Private
    if (!bRet && (nFormat==FORMAT_PRIVATE || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(FORMAT_PRIVATE)) ||
         (!bClp && DragServer::HasFormat(nItemNum,FORMAT_PRIVATE)))) {
        SdrModel* pModel=(SdrModel*)DragServer::PastePrivateData(nItemNum);
        if (bClp) pModel=(SdrModel*)Clipboard::PastePrivateData();
        if (pModel!=NULL) {
            bRet=Paste(*pModel,rPos,NULL,nOptions);
        }
    }
    // 2. Prioritaet fuer SdrExchangeFormat
    if (!bRet && (nFormat==nSdrFormat || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(nSdrFormat)) ||
         (!bClp && DragServer::HasFormat(nItemNum,nSdrFormat))))
    {
        UINT32 nDataLen(0);
        char* pDataBuf = NULL;

        if(bClp)
        {
            nDataLen = Clipboard::GetDataLen(nSdrFormat);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                Clipboard::PasteData(pDataBuf, nDataLen, nSdrFormat);
            }
        }
        else
        {
            nDataLen = DragServer::GetDataLen(nItemNum, nSdrFormat);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                DragServer::PasteData(nItemNum, pDataBuf, nDataLen, nSdrFormat);
            }
        }

        if(nDataLen)
        {
            SvMemoryStream aMemStream(pDataBuf, nDataLen, STREAM_READ);

            aMemStream.SetVersion(SOFFICE_FILEFORMAT_NOW);

            SdrModel* pModel = pMod->AllocModel();

            pModel->SetStreamingSdrModel(TRUE);

            // StyleSheetPool und Persist fehlt hier wohl noch ...
            pModel->GetItemPool().Load(aMemStream);
            aMemStream >> *pModel;
            bRet = Paste(*pModel, rPos, NULL, nOptions);
        }

        if(pDataBuf != NULL)
            delete [] pDataBuf;
    }
    // 3. Prioritaet fuer Metafile
    if (!bRet && (nFormat==FORMAT_GDIMETAFILE || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(FORMAT_GDIMETAFILE)) ||
         (!bClp && DragServer::HasFormat(nItemNum,FORMAT_GDIMETAFILE)))) {
        GDIMetaFile aMtf;
        if (bClp) Clipboard::PasteGDIMetaFile(aMtf);
        else DragServer::PasteGDIMetaFile(nItemNum,aMtf);
        bRet=Paste(aMtf,rPos,NULL,nOptions);
    }
    // 4. Prioritaet fuer Bitmap
    if (!bRet && (nFormat==FORMAT_BITMAP || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(FORMAT_BITMAP)) ||
         (!bClp && DragServer::HasFormat(nItemNum,FORMAT_BITMAP)))) {
        Bitmap aBmp;
        if (bClp) aBmp=Clipboard::PasteBitmap();
        else aBmp=DragServer::PasteBitmap(nItemNum);
        bRet=Paste(aBmp,rPos,NULL,nOptions);
    }
    // 5. Prioritaet fuer EditEngineFormat
    if (!bRet && (nFormat==nEEFormat || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(nEEFormat)) ||
         (!bClp && DragServer::HasFormat(nItemNum,nEEFormat))))
    {
        UINT32 nDataLen(0);
        char* pDataBuf = NULL;

        if(bClp)
        {
            nDataLen = Clipboard::GetDataLen(nEEFormat);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                Clipboard::PasteData(pDataBuf, nDataLen, nEEFormat);
            }
        }
        else
        {
            nDataLen = DragServer::GetDataLen(nItemNum, nEEFormat);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                DragServer::PasteData(nItemNum, pDataBuf, nDataLen, nEEFormat);
            }
        }

        if(nDataLen)
        {
            SvMemoryStream aMemStream(pDataBuf, nDataLen, STREAM_READ);
            bRet = Paste(aMemStream, UINT16(EE_FORMAT_BIN), rPos, NULL, nOptions);
        }

        if(pDataBuf)
            delete [] pDataBuf;
    }

    // 6. Prioritaet fuer RTF
    if (!bRet && (nFormat==FORMAT_RTF || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(FORMAT_RTF)) ||
         (!bClp && DragServer::HasFormat(nItemNum,FORMAT_RTF))))
    {
        UINT32 nDataLen(0);
        char* pDataBuf = NULL;

        if(bClp)
        {
            nDataLen = Clipboard::GetDataLen(FORMAT_RTF);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                Clipboard::PasteData(pDataBuf, nDataLen, FORMAT_RTF);
            }
        }
        else
        {
            nDataLen = DragServer::GetDataLen(nItemNum, FORMAT_RTF);

            if(nDataLen)
            {
                pDataBuf = new char[nDataLen];
                DragServer::PasteData(nItemNum, pDataBuf, nDataLen, FORMAT_RTF);
            }
        }

        if(nDataLen)
        {
            SvMemoryStream aMemStream(pDataBuf, nDataLen, STREAM_READ);
            bRet = Paste(aMemStream, UINT16(EE_FORMAT_RTF), rPos, NULL, nOptions);
        }

        if(pDataBuf)
            delete [] pDataBuf;
    }
    // 7. Prioritaet fuer unformatierten Text
    if (!bRet && (nFormat==FORMAT_STRING || nFormat==SDR_ANYFORMAT) &&
        ((bClp && Clipboard::HasFormat(FORMAT_STRING)) ||
         (!bClp && DragServer::HasFormat(nItemNum,FORMAT_STRING)))) {
        XubString aStr;
        if (bClp) aStr=Clipboard::PasteString();
        else aStr=DragServer::PasteString(nItemNum);
        bRet=Paste(aStr,rPos,NULL,nOptions);
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Bitmap SdrExchangeView::GetMarkedObjBitmap( BOOL bNoVDevIfOneBmpMarked ) const
{

    Bitmap aBmp;

    if( HasMarkedObj() )
    {
        if( bNoVDevIfOneBmpMarked )
        {
            SdrObject*  pGrafObjTmp = aMark.GetMark( 0 )->GetObj();
            SdrGrafObj* pGrafObj = ( aMark.GetMarkCount() == 1 ) ? PTR_CAST( SdrGrafObj, pGrafObjTmp ) : NULL;

            if( pGrafObj && !pGrafObj->GetShearAngle() && ( pGrafObj->GetGraphicType() == GRAPHIC_BITMAP ) )
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

    if( HasMarkedObj() )
    {
        if( bNoVDevIfOneMtfMarked )
        {
            SdrObject*  pGrafObjTmp = aMark.GetMark( 0 )->GetObj();
            SdrGrafObj* pGrafObj = ( aMark.GetMarkCount() ==1 ) ? PTR_CAST( SdrGrafObj, pGrafObjTmp ) : NULL;

            if( pGrafObj && !pGrafObj->GetShearAngle() && pGrafObj->GetGraphicType() == GRAPHIC_GDIMETAFILE )
                aMtf = pGrafObj->GetTransformedGraphic().GetGDIMetaFile();
        }

        if( !aMtf.GetActionCount() )
        {
            VirtualDevice   aOut;
            Rectangle       aBound( GetMarkedObjBoundRect() );
            MapMode         aMap( pMod->GetScaleUnit(), Point(),
                                  pMod->GetScaleFraction(), pMod->GetScaleFraction() );

            aOut.EnableOutput( FALSE );
            aOut.SetMapMode( aMap );
            aMtf.Clear();
            aMtf.Record( &aOut );
            DrawMarkedObj( aOut, aBound.TopLeft() );
            aMtf.Stop();
            aMtf.WindStart();

            const Size  aExtSize( aOut.PixelToLogic( Size( 0, 0  ) ) );
            Size        aBoundSize( aBound.GetWidth() + ( aExtSize.Width() ),
                                    aBound.GetHeight() + ( aExtSize.Height() ) );

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundSize );
        }
    }

    return aMtf;
}

// -----------------------------------------------------------------------------

void SdrExchangeView::DrawMarkedObj(OutputDevice& rOut, const Point& rOfs) const
{
    // Wenn das sortieren der MarkList mal stoeren sollte,
    // werde ich sie mir wohl kopieren muessen.
    ((SdrExchangeView*)this)->aMark.ForceSort();
    // Hier kann noch optimiert werden ... (SetOffset)
    pXOut->SetOutDev(&rOut);
    SdrPaintInfoRec aInfoRec;
    aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;

    for (ULONG nm=0; nm<aMark.GetMarkCount(); nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        Point aOfs(-rOfs.X(),-rOfs.Y());
        aOfs+=pM->GetPageView()->GetOffset();
        if (aOfs!=pXOut->GetOffset()) pXOut->SetOffset(aOfs);
        pO->Paint(*pXOut,aInfoRec);
    }
    pXOut->SetOffset(Point(0,0));
}

SdrModel* SdrExchangeView::GetMarkedObjModel() const
{
    // Wenn das sortieren der MarkList mal stoeren sollte,
    // werde ich sie mir wohl kopieren muessen.
    ((SdrExchangeView*)this)->aMark.ForceSort();
    SdrModel* pNeuMod=pMod->AllocModel();
    SdrPage* pNeuPag=pNeuMod->AllocPage(FALSE);
    pNeuMod->InsertPage(pNeuPag);
    ULONG nCloneErrCnt=0;
    ULONG nMarkAnz=aMark.GetMarkCount();
    ULONG nm;
    for (nm=0; nm<nMarkAnz; nm++) {
        const SdrMark* pM=aMark.GetMark(nm);
        const SdrObject* pObj=pM->GetObj();
        SdrObject* pNeuObj=pObj->Clone(pNeuPag,pNeuMod);
        if (pNeuObj!=NULL) {
            Point aP(pM->GetPageView()->GetOffset());
            if (aP.X()!=0 || aP.Y()!=0) pNeuObj->NbcMove(Size(aP.X(),aP.Y()));
            SdrInsertReason aReason(SDRREASON_VIEWCALL);
            pNeuPag->InsertObject(pNeuObj,CONTAINER_APPEND,&aReason);
        } else {
            nCloneErrCnt++;
        }
    }
    // und nun zu den Konnektoren
    // Die Objekte in pNeuPag werden auf die MarkList abgebildet
    // und so die Objektverbindungen hergestellt.
    // Aehnliche Implementation an folgenden Stellen:
    //    void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
    //    SdrModel* SdrExchangeView::GetMarkedObjModel() const
    //    BOOL SdrExchangeView::Paste(const SdrModel& rMod,...)
    //    void SdrEditView::CopyMarked()
    if (nCloneErrCnt==0) {
        for (nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=aMark.GetMark(nm);
            SdrObject* pO=pM->GetObj();
            SdrEdgeObj* pSrcEdge=PTR_CAST(SdrEdgeObj,pO);
            if (pSrcEdge!=NULL) {
                SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(TRUE);
                SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(FALSE);
                if (pSrcNode1!=NULL && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=NULL; // Listenuebergreifend
                if (pSrcNode2!=NULL && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=NULL; // ist (noch) nicht
                if (pSrcNode1!=NULL || pSrcNode2!=NULL) {
                    SdrObject* pDstEdgeTmp=pNeuPag->GetObj(nm);
                    SdrEdgeObj* pDstEdge=PTR_CAST(SdrEdgeObj,pDstEdgeTmp);
                    if (pDstEdge!=NULL) {
                        if (pSrcNode1!=NULL) {
                            ULONG nDstNode1=((SdrMarkList*)&aMark)->FindObject(pSrcNode1);
                            SdrObject* pDstNode1=NULL;
                            if (nDstNode1!=CONTAINER_ENTRY_NOTFOUND) {
                                pDstNode1=pNeuPag->GetObj(nDstNode1);
                            }
                            if (pDstNode1!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(TRUE,pDstNode1);
                            }
                        }
                        if (pSrcNode2!=NULL) {
                            ULONG nDstNode2=((SdrMarkList*)&aMark)->FindObject(pSrcNode2);
                            SdrObject* pDstNode2=NULL;
                            if (nDstNode2!=CONTAINER_ENTRY_NOTFOUND) {
                                pDstNode2=pNeuPag->GetObj(nDstNode2);
                            }
                            if (pDstNode2!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(FALSE,pDstNode2);
                            }
                        }
                    } else {
                        DBG_ERROR("SdrExchangeView::GetMarkedObjModel(): pDstEdge==NULL!");
                    }
                }
            }
        }
    } else {
#ifdef DBG_UTIL
        ByteString aStr("SdrExchangeView::GetMarkedObjModel(): Fehler beim Clonen ");

        if(nCloneErrCnt == 1)
        {
            aStr += "eines Zeichenobjekts.";
        }
        else
        {
            aStr += "von ";
            aStr += nCloneErrCnt;
            aStr += " Zeichenobjekten.";
        }

        aStr += " Objektverbindungen werden nicht mitkopiert.";

        DBG_ERROR(aStr.GetBuffer());
#endif
    }
    return pNeuMod;
}

BOOL SdrExchangeView::Cut(ULONG nFormat)
{
    BOOL bOk=SdrObjEditView::Cut(nFormat);
    if (!bOk && HasMarkedObj()) {
        CutMarked(nFormat);
        bOk=TRUE;
    }
    return bOk;
}

BOOL SdrExchangeView::Yank(ULONG nFormat)
{
    BOOL bOk=SdrObjEditView::Yank(nFormat);
    if (!bOk && HasMarkedObj()) {
        YankMarked(nFormat);
        bOk=TRUE;
    }
    return bOk;
}

BOOL SdrExchangeView::Paste(Window* pWin, ULONG nFormat)
{
    BOOL bOk=SdrObjEditView::Paste(pWin,nFormat);
    if (!bOk) bOk=PasteClipboard(pWin,nFormat);
    return bOk;
}

BOOL SdrExchangeView::QueryDrop(DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    BOOL bOk(FALSE);

    if(!bOk)
    {
        if(GetDragMode() == SDRDRAG_GRADIENT || GetDragMode() == SDRDRAG_TRANSPARENCE)
        {
            const SdrHdlList& rHdlList = GetHdlList();

            for(UINT32 a=0;a<rHdlList.GetHdlCount();a++)
            {
                SdrHdl* pIAOHandle = rHdlList.GetHdl(a);

                if(pIAOHandle && pIAOHandle->GetKind() == HDL_COLR)
                {
                    // what was hit?
                    const B2dIAOGroup& rIAOGroup = pIAOHandle->GetIAOGroup();

                    if(rIAOGroup.IsHit(rDEvt.GetPosPixel()))
                    {
                        bOk = TRUE;
                        ((SdrHdlColor*)pIAOHandle)->SetSize(SDR_HANDLE_COLOR_SIZE_SELECTED);
                    }
                    else
                    {
                        ((SdrHdlColor*)pIAOHandle)->SetSize(SDR_HANDLE_COLOR_SIZE_NORMAL);
                    }
                }
            }

            // refresh IAO display
            RefreshAllIAOManagers();
        }
    }

    return bOk;
}

BOOL SdrExchangeView::Drop(const DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    BOOL bOk(FALSE);

#ifndef SVX_LIGHT
    if(!bOk)
    {
        if(GetDragMode() == SDRDRAG_GRADIENT || GetDragMode() == SDRDRAG_TRANSPARENCE)
        {
            const SdrHdlList& rHdlList = GetHdlList();

            for(UINT32 a=0;!bOk && a<rHdlList.GetHdlCount();a++)
            {
                SdrHdl* pIAOHandle = rHdlList.GetHdl(a);

                if(pIAOHandle && pIAOHandle->GetKind() == HDL_COLR)
                {
                    // what was hit?
                    const B2dIAOGroup& rIAOGroup = pIAOHandle->GetIAOGroup();

                    if(rIAOGroup.IsHit(rDEvt.GetPosPixel()))
                    {
                        SvData aData(SOT_FORMATSTR_ID_XFA);
                        XFillExchangeData* pFillData = NULL;
                        SvDataObjectRef pDataObj = SvDataObject::PasteDragServer(rDEvt);
                        const SvDataTypeList& rTypeList = pDataObj->GetTypeList();

                        if(rTypeList.Get(SOT_FORMATSTR_ID_XFA) || nFormat == SDR_ANYFORMAT)
                        {
                            if(pDataObj->GetData(&aData))
                            {
                                if(aData.GetData((SvDataCopyStream**)&pFillData, XFillExchangeData::StaticType(), TRANSFER_MOVE))
                                {
                                    XFillAttrSetItem* pSetItem = pFillData->GetXFillAttrSetItem();
                                    SfxItemSet rSet = pSetItem->GetItemSet();
                                    const XFillColorItem& rColItem = (XFillColorItem&)rSet.Get(XATTR_FILLCOLOR);
                                    Color aColor = rColItem.GetValue();

                                    ((SdrHdlColor*)pIAOHandle)->SetColor(aColor, TRUE);

                                    bOk = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

#endif // SVX_LIGHT
    return bOk;
}

