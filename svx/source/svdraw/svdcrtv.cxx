/*************************************************************************
 *
 *  $RCSfile: svdcrtv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "svdcrtv.hxx"
#include "xattr.hxx"
#include "svdio.hxx"
#include "svdxout.hxx"
#include "svdundo.hxx"
#include "svdocapt.hxx" // Spezialbehandlung: Nach dem Create transparente Fuellung
#include "svdcrtmt.hxx"
#include "svdvmark.hxx"
#include "svdoedge.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdetc.hxx"

#define XOR_CREATE_PEN          PEN_SOLID

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrConnectMarker: public SdrViewUserMarker
{
    const SdrObject*            pAktObj;
    const SdrPageView*          pAktPV;

    virtual void Draw(OutputDevice* pOut, FASTBOOL bHiding, FASTBOOL bNoSaveDC);

public:
    ImpSdrConnectMarker(SdrCreateView* pView): SdrViewUserMarker(pView),pAktObj(NULL),pAktPV(NULL) {}
    ~ImpSdrConnectMarker() {}
    void SetTargetObject(const SdrObject* pObj);
}; // svdvmark

void ImpSdrConnectMarker::Draw(OutputDevice* pOut, FASTBOOL bHiding, FASTBOOL bNoSaveDC)
{
    SdrViewUserMarker::Draw(pOut,bHiding,bNoSaveDC);

    const SdrCreateView* pView=(SdrCreateView*)GetView();
    const SdrObject* pObj=pAktObj;
    const SdrPageView* pPV=pAktPV;

    if (pObj!=NULL && pView!=NULL && pOut!=NULL)
    {
        RasterOp eRop0=pOut->GetRasterOp();
        BOOL bMap0=pOut->IsMapModeEnabled();
        pOut->SetRasterOp(ROP_INVERT);

        Color aOldLineColor, aOldFillColor;

        if (!bNoSaveDC)
        {
            aOldLineColor = pOut->GetLineColor();
            aOldFillColor = pOut->GetFillColor();
        }
        Point aPvOfs; if (pPV!=NULL) aPvOfs=pPV->GetOffset();
        if (pView->IsAutoVertexConnectors())
        {
            for (USHORT i=0; i<4; i++) {
                SdrGluePoint aGluePoint(pObj->GetVertexGluePoint(i));
                aGluePoint.Draw(*pOut,pObj);
            }
        }
        if (pView->IsAutoCornerConnectors())
        {
            for (USHORT i=0; i<4; i++)
            {
                SdrGluePoint aGluePoint(pObj->GetVertexGluePoint(i));
                aGluePoint.Draw(*pOut,pObj);
            }
        }
        if (!bNoSaveDC)
        {
            pOut->SetLineColor( aOldLineColor );
            if ( aOldFillColor != pOut->GetFillColor() )
                pOut->SetFillColor( aOldFillColor );
        }
        pOut->SetRasterOp(eRop0);
        pOut->EnableMapMode(bMap0);
    }
}

void ImpSdrConnectMarker::SetTargetObject(const SdrObject* pObj)
{
    if (pAktObj!=pObj) {
        BOOL bVisible=IsVisible();
        if (bVisible) Hide();
        pAktObj=pObj;
        if (bVisible) Show();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@  @@@@  @@@@@@ @@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@   @@
//  @@     @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@ @ @@
//  @@     @@@@@  @@@@  @@@@@@   @@   @@@@   @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@ @@@
//   @@@@  @@  @@ @@@@@ @@  @@   @@   @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCreateView::ImpClearVars()
{
    nAktInvent=SdrInventor;
    nAktIdent=OBJ_NONE;
    pCurrentLibObj=NULL;
    bCurrentLibObjMoveNoResize=FALSE;
    bCurrentLibObjSetDefAttr=FALSE;
    bCurrentLibObjSetDefLayer=FALSE;
    pLibObjDragMeth=NULL;
    pAktCreate=NULL;
    pCreatePV=NULL;
    bAutoTextEdit=FALSE;
    b1stPointAsCenter=FALSE;
    aAktCreatePointer=Pointer(POINTER_CROSS);
    bUseIncompatiblePathCreateInterface=FALSE;
    bAutoClosePolys=TRUE;
    nAutoCloseDistPix=5;
    nFreeHandMinDistPix=10;

    pConnectMarker=new ImpSdrConnectMarker(this);
    pConnectMarker->SetLineWidth(2);
    pConnectMarker->SetAnimateDelay(500);
    pConnectMarker->SetAnimateCount(3);
    pConnectMarker->SetAnimateSpeed(100);
    pConnectMarker->SetAnimateToggle(TRUE);
}

void SdrCreateView::ImpMakeCreateAttr()
{
}

SdrCreateView::SdrCreateView(SdrModel* pModel1, OutputDevice* pOut):
    SdrDragView(pModel1,pOut)
{
    ImpClearVars();
    ImpMakeCreateAttr();
}

SdrCreateView::SdrCreateView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrDragView(pModel1,pXOut)
{
    ImpClearVars();
    ImpMakeCreateAttr();
}

SdrCreateView::~SdrCreateView()
{
    delete pConnectMarker;
    delete pCurrentLibObj;
    delete pAktCreate;
}

void SdrCreateView::ImpDelCreateAttr()
{
}

BOOL SdrCreateView::IsAction() const
{
    return SdrDragView::IsAction() || pAktCreate!=NULL;
}

void SdrCreateView::MovAction(const Point& rPnt)
{
    SdrDragView::MovAction(rPnt);
    if (pAktCreate!=NULL) {
        MovCreateObj(rPnt);
    }
}

void SdrCreateView::EndAction()
{
    if (pAktCreate!=NULL) EndCreateObj(SDRCREATE_FORCEEND);
    SdrDragView::EndAction();
}

void SdrCreateView::BckAction()
{
    if (pAktCreate!=NULL) BckCreateObj();
    SdrDragView::BckAction();
}

void SdrCreateView::BrkAction()
{
    SdrDragView::BrkAction();
    BrkCreateObj();
}

void SdrCreateView::TakeActionRect(Rectangle& rRect) const
{
    if (pAktCreate!=NULL) {
        rRect=aDragStat.GetActionRect();
        if (rRect.IsEmpty()) {
            rRect=Rectangle(aDragStat.GetPrev(),aDragStat.GetNow());
        }
    } else {
        SdrDragView::TakeActionRect(rRect);
    }
}

void SdrCreateView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
{
    SdrDragView::ToggleShownXor(pOut,pRegion);
    if (pAktCreate!=NULL && aDragStat.IsShown()) {
        DrawCreateObj(pOut,TRUE);
    }
}

BOOL SdrCreateView::CheckEdgeMode()
{
    UINT32 nInv=nAktInvent;
    UINT16 nIdn=nAktIdent;
    if (pAktCreate!=NULL) {
        nInv=pAktCreate->GetObjInventor();
        nIdn=pAktCreate->GetObjIdentifier();
        // wird vom EdgeObj gemanaged
        if (nAktInvent==SdrInventor && nAktIdent==OBJ_EDGE) return FALSE;
    }
    if (!IsCreateMode() || nAktInvent!=SdrInventor || nAktIdent!=OBJ_EDGE || pCurrentLibObj!=NULL) {
        if (pConnectMarker->IsVisible()) pConnectMarker->Hide();
        pConnectMarker->SetTargetObject(NULL);
        return FALSE;
    } else {
        // TRUE heisst: MouseMove soll Connect checken
        return !IsAction();
    }
}

void SdrCreateView::SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& rPV)
{
    if (rCon.pObj!=NULL) {
        Rectangle aNewRect;
        USHORT nNewDist=0;
        if (rCon.bBestConn || rCon.bBestVertex) {
            aNewRect=rCon.pObj->GetBoundRect();
            nNewDist=2;
        } else {
            nNewDist=aHdl.GetHdlSize()+2;
            SdrGluePoint aGP;
            rCon.TakeGluePoint(aGP,TRUE);
            Point aPt(aGP.GetPos());
            aNewRect=Rectangle(aPt,aPt);
        }
        Point aPvOfs(rPV.GetOffset());
        aNewRect.Move(aPvOfs.X(),aPvOfs.Y());
        if (!pConnectMarker->IsVisible() ||
            pConnectMarker->GetRectangle()==NULL ||
            *pConnectMarker->GetRectangle()!=aNewRect ||
            pConnectMarker->GetPixelDistance()!=nNewDist)
        {
            pConnectMarker->Hide();
            pConnectMarker->SetRectangle(aNewRect);
            pConnectMarker->SetPixelDistance(nNewDist);
            pConnectMarker->SetAnimate(TRUE);
            pConnectMarker->SetTargetObject(rCon.pObj);
            pConnectMarker->Show();
        }
    } else {
        HideConnectMarker();
    }
}

void SdrCreateView::HideConnectMarker()
{
    pConnectMarker->Hide();
    pConnectMarker->SetTargetObject(NULL);
    pConnectMarker->SetAnimate(FALSE);
}

BOOL SdrCreateView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if (CheckEdgeMode() && pWin!=NULL) {
        Point aPos(pWin->PixelToLogic(rMEvt.GetPosPixel()));
        SdrPageView* pPV=GetPageView(aPos);
        if (pPV!=NULL) {
            // Defaultete Hit-Toleranz bei IsMarkedHit() mal aendern !!!!
            BOOL bMarkHit=HitHandle(aPos,*pWin)!=NULL || IsMarkedObjHit(aPos);
            SdrObjConnection aCon;
            Point aPvOfs(pPV->GetOffset());
            aPos-=aPvOfs;
            if (!bMarkHit) SdrEdgeObj::ImpFindConnector(aPos,*pPV,aCon,NULL,pWin);
            SetConnectMarker(aCon,*pPV);
        }
    }
    return SdrDragView::MouseMove(rMEvt,pWin);
}

BOOL SdrCreateView::IsTextTool() const
{
    return eEditMode==SDREDITMODE_CREATE && pCurrentLibObj==NULL && nAktInvent==SdrInventor && (nAktIdent==OBJ_TEXT || nAktIdent==OBJ_TEXTEXT || nAktIdent==OBJ_TITLETEXT || nAktIdent==OBJ_OUTLINETEXT);
}

BOOL SdrCreateView::IsEdgeTool() const
{
    return eEditMode==SDREDITMODE_CREATE && pCurrentLibObj==NULL && nAktInvent==SdrInventor && (nAktIdent==OBJ_EDGE);
}

BOOL SdrCreateView::IsMeasureTool() const
{
    return eEditMode==SDREDITMODE_CREATE && pCurrentLibObj==NULL && nAktInvent==SdrInventor && (nAktIdent==OBJ_MEASURE);
}

void SdrCreateView::SetCurrentObj(UINT16 nIdent, UINT32 nInvent)
{
    if (pCurrentLibObj!=NULL) {
        delete pCurrentLibObj;
        pCurrentLibObj=NULL;
    }
    if (nAktInvent!=nInvent || nAktIdent!=nIdent) {
        nAktInvent=nInvent;
        nAktIdent=nIdent;
        SdrObject* pObj=SdrObjFactory::MakeNewObject(nInvent,nIdent,NULL,NULL);
        if (pObj!=NULL) {
            // Auf pers. Wunsch von Marco:
            // Mauszeiger bei Textwerkzeug immer I-Beam. Fadenkreuz
            // mit kleinem I-Beam erst bai MouseButtonDown
            if (IsTextTool()) aAktCreatePointer=POINTER_TEXT;
            else aAktCreatePointer=pObj->GetCreatePointer();
            delete pObj;
        } else {
            aAktCreatePointer=Pointer(POINTER_CROSS);
        }
    }
    CheckEdgeMode();
    ImpSetGlueVisible3(IsEdgeTool());
}

void SdrCreateView::SetCurrentLibObj(SdrObject* pObj, BOOL bMoveNoResize, BOOL bSetDefAttr, BOOL bSetDefLayer)
{
    if (pCurrentLibObj!=NULL && pObj!=pCurrentLibObj) delete pCurrentLibObj;
    pCurrentLibObj=pObj;
    bCurrentLibObjMoveNoResize=bMoveNoResize;
    bCurrentLibObjSetDefAttr=bSetDefAttr;
    bCurrentLibObjSetDefLayer=bSetDefLayer;
    if (pCurrentLibObj!=NULL) {
        pCurrentLibObj->SetModel(pMod);
    }
    aAktCreatePointer=Pointer(POINTER_CROSS);
    nAktInvent=SdrInventor;
    nAktIdent=OBJ_NONE;
    pConnectMarker->Hide();
    ImpSetGlueVisible3(FALSE);
}

BOOL SdrCreateView::ImpBegCreateObj(UINT32 nInvent, UINT16 nIdent, const Point& rPnt, OutputDevice* pOut,
    short nMinMov, SdrPageView* pPV, const Rectangle& rLogRect)
{
    BOOL bRet=FALSE;
    SetSolidDraggingNow(IsSolidDragging());
    SetSolidDraggingCheck(FALSE);
    UnmarkAllObj();
    BrkAction();
    pConnectMarker->Hide();
    DBG_ASSERT(pLibObjDragMeth==NULL,"SdrCreateView::ImpBegCreateObj(): pLibObjDragMeth!=NULL");
    pLibObjDragMeth=NULL;
    if (pPV!=NULL) pCreatePV=pPV;
    else pCreatePV=GetPageView(rPnt);
    if (pCreatePV!=NULL) { // ansonsten keine Seite angemeldet!
        String aLay(aAktLayer);

        if(nInvent == SdrInventor && nIdent == OBJ_MEASURE && aMeasureLayer.Len())
        {
            aLay = aMeasureLayer;
        }

        SdrLayerID nLayer=pCreatePV->GetPage()->GetLayerAdmin().GetLayerID(aLay,TRUE);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
        if (!pCreatePV->GetLockedLayers().IsSet(nLayer) && pCreatePV->GetVisibleLayers().IsSet(nLayer)) {
            pAktCreate=SdrObjFactory::MakeNewObject(nInvent,nIdent,pCreatePV->GetPage(),pMod);
            Point aPnt(rPnt-pCreatePV->GetOffset());
            if (nAktInvent!=SdrInventor || (nAktIdent!=USHORT(OBJ_EDGE) &&
                                            nAktIdent!=USHORT(OBJ_FREELINE) &&
                                            nAktIdent!=USHORT(OBJ_FREEFILL) )) { // Kein Fang fuer Edge und Freihand!
                aPnt=GetSnapPos(aPnt,pCreatePV);
            }
            if (pAktCreate!=NULL) {
                BOOL bStartEdit=FALSE; // nach Ende von Create automatisch TextEdit starten
                if (pDefaultStyleSheet!=NULL) pAktCreate->NbcSetStyleSheet(pDefaultStyleSheet,FALSE);
                pAktCreate->NbcSetAttributes(aDefaultAttr,FALSE);
                if (HAS_BASE(SdrCaptionObj,pAktCreate)) {
                    SfxItemSet aSet(pMod->GetItemPool());
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    pAktCreate->NbcSetAttributes(aSet,FALSE);
                    bStartEdit=TRUE;
                }
                if (nInvent==SdrInventor && (nIdent==OBJ_TEXT || nIdent==OBJ_TEXTEXT ||
                                             nIdent==OBJ_TITLETEXT || nIdent==OBJ_OUTLINETEXT)) {
                    // Fuer alle Textrahmen default keinen Hintergrund und keine Umrandung
                    SfxItemSet aSet(pMod->GetItemPool());
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // Falls einer auf Solid umschaltet
                    aSet.Put(XLineStyleItem(XLINE_NONE));
                    pAktCreate->NbcSetAttributes(aSet,FALSE);
                    bStartEdit=TRUE;
                }
                if (!rLogRect.IsEmpty()) pAktCreate->NbcSetLogicRect(rLogRect);
                aDragStat.Reset(aPnt);
                aDragStat.SetView((SdrView*)this);
                aDragStat.SetPageView(pCreatePV);
                aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
                pDragWin=pOut;
                if (pAktCreate->BegCreate(aDragStat)) {
                    ShowCreateObj(pOut,TRUE);
                    bRet=TRUE;
                } else {
                    delete pAktCreate;
                    pAktCreate=NULL;
                    pCreatePV=NULL;
                }
            }
        }
    }
    return bRet;
}

BOOL SdrCreateView::BegCreateObj(const Point& rPnt, OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    if ( !pCurrentLibObj )
        return ImpBegCreateObj(nAktInvent,nAktIdent,rPnt,pOut,nMinMov,pPV,Rectangle());
    else
        return BegCreateLibObj(rPnt,pCurrentLibObj->Clone(),bCurrentLibObjMoveNoResize,
                               bCurrentLibObjSetDefAttr,bCurrentLibObjSetDefLayer,pOut,nMinMov,pPV);
}

BOOL SdrCreateView::BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz,
    OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    return ImpBegCreateObj(SdrInventor,OBJ_CAPTION,rPnt,pOut,nMinMov,pPV,
                           Rectangle(rPnt,Size(rObjSiz.Width()+1,rObjSiz.Height()+1)));
}

BOOL SdrCreateView::BegCreateLibObj(const Point& rPnt, SdrObject* pObj, BOOL bMoveNoResize,
    BOOL bSetDefAttr, BOOL bSetDefLayer,
    OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    SetSolidDraggingNow(FALSE); // noch kein SolidDragging bei LibObjs
    BOOL bRet=FALSE;
    UnmarkAllObj();
    BrkAction();
    pConnectMarker->Hide();
    DBG_ASSERT(pLibObjDragMeth==NULL,"SdrCreateView::BegCreateLibObj(): pLibObjDragMeth!=NULL");
    pLibObjDragMeth=NULL;
    if (pPV!=NULL) pCreatePV=pPV;
    else pCreatePV=GetPageView(rPnt);
    if (pObj!=NULL && pCreatePV!=NULL) {
        bRet=TRUE;
        pObj->SetModel(pMod);
        if (bSetDefAttr) {
            if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet,FALSE);
            pObj->NbcSetAttributes(aDefaultAttr,FALSE);
        }
        if (bSetDefLayer) {
            SdrLayerID nLayer=pCreatePV->GetPage()->GetLayerAdmin().GetLayerID(aAktLayer,TRUE);
            if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
            if (pCreatePV->GetLockedLayers().IsSet(nLayer) || !pCreatePV->GetVisibleLayers().IsSet(nLayer)) {
                bRet=FALSE; // Layer gesperrt oder nicht sichtbar!
            }
        }
        if (bRet) {
            pAktCreate=pObj;
            Point aPnt(rPnt-pCreatePV->GetOffset());
            aPnt=GetSnapPos(aPnt,pCreatePV);
            aDragStat.Reset(aPnt);
            aDragStat.SetView((SdrView*)this);
            aDragStat.SetPageView(pCreatePV);
            aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
            pDragWin=pOut;
            if (bMoveNoResize) {
                pLibObjDragMeth=new ImpSdrCreateLibObjMove(*this);
            } else {
                pLibObjDragMeth=new ImpSdrCreateLibObjResize(*this);
            }
            bRet=pLibObjDragMeth->Beg();
        }
    }
    if (!bRet) {
        if (pObj!=NULL) delete pObj;
        if (pLibObjDragMeth!=NULL) { delete pLibObjDragMeth; pLibObjDragMeth=NULL; }
        pAktCreate=NULL;
        pCreatePV=NULL;
    }
    return bRet;
}

void SdrCreateView::MovCreateObj(const Point& rPnt)
{
    if (pAktCreate!=NULL) {
        if (IsSolidDraggingNow() && !IsSolidDraggingCheck()) {
            // Z.B. fuer Fill+Linelose Textrahmen bei SolidDragging
            SetSolidDraggingCheck(TRUE);
            SfxItemSet aSet(pMod->GetItemPool());
            pAktCreate->TakeAttributes(aSet,FALSE,FALSE);
            XFillStyle eFill=((XFillStyleItem&)(aSet.Get(XATTR_FILLSTYLE))).GetValue();
            XLineStyle eLine=((XLineStyleItem&)(aSet.Get(XATTR_LINESTYLE))).GetValue();
            if (eLine==XLINE_NONE && eFill==XFILL_NONE) {
                SetSolidDraggingNow(FALSE);
            }
        }
        Point aPnt(rPnt-pCreatePV->GetOffset());
        if (!aDragStat.IsNoSnap()) {
            aPnt=GetSnapPos(aPnt,pCreatePV);
        }
        if (IsOrtho()) {
            if (aDragStat.IsOrtho8Possible()) OrthoDistance8(aDragStat.GetPrev(),aPnt,IsBigOrtho());
            else if (aDragStat.IsOrtho4Possible()) OrthoDistance4(aDragStat.GetPrev(),aPnt,IsBigOrtho());
        }
        ImpLimitToWorkArea(aPnt,pCreatePV);
        if (aPnt==aDragStat.GetNow()) return;
        if (pLibObjDragMeth==NULL) {
            BOOL bMerk=aDragStat.IsMinMoved();
            if (aDragStat.CheckMinMoved(aPnt)) {
                Rectangle aBound;
                if (IsSolidDraggingNow()) aBound=pAktCreate->GetBoundRect();
                XPolyPolygon aXPP1;
                if (!bMerk) aDragStat.NextPoint();
                aDragStat.NextMove(aPnt);
                pAktCreate->MovCreate(aDragStat);
                pAktCreate->TakeCreatePoly(aDragStat,aXPP1);
                XPolyPolygon aXPP2(aXPP1); // kopieren, weil wird in DrawCreateDiff geaendert
                if (!IsSolidDraggingNow()) {
                    DrawCreateObjDiff(pCreatePV->DragPoly(),aXPP1);
                }
                pCreatePV->DragPoly()=aXPP2;
                if (IsSolidDraggingNow()) {
                    aBound.Union(pAktCreate->GetBoundRect());
                    SdrObjList* pOL=pCreatePV->GetObjList();
                    SdrInsertReason aReason(SDRREASON_VIEWCALL);
                    pOL->NbcInsertObject(pAktCreate,CONTAINER_APPEND,&aReason);
                    Point aPvOfs(pCreatePV->GetOffset());
                    USHORT nAnz=pDragWin==NULL ? GetWinCount() : 1;
                    for (USHORT i=0; i<nAnz; i++) {
                        USHORT nWinNum=SDRVIEWWIN_NOTFOUND;
                        OutputDevice* pOut=pDragWin;
                        if (pOut==NULL) {
                            nWinNum=i;
                            pOut=GetWin(nWinNum);
                        } else {
                            nWinNum=aWinList.Find(pOut);
                        }
                        VirtualDevice aVDev(*pOut);
                        Size a2Pix(pOut->PixelToLogic(Size(2,2)));
                        MapMode aMap(pOut->GetMapMode());
                        aVDev.SetMapMode(aMap);
                        #ifdef MAC
                            Color aMixedColor( RGB_COLORDATA( 223, 223, 223 ) );
                        #else
                            Color aMixedColor( RGB_COLORDATA( 234, 234, 234 ) );
                        #endif
                        aVDev.SetBackground( Wallpaper( aMixedColor ) );
                        aVDev.SetOutputSize(pOut->GetOutputSize());
                        Rectangle aDirtyArea(aBound);
                        aDirtyArea.Move(aPvOfs.X(),aPvOfs.Y());
                        InitRedraw(&aVDev,aDirtyArea);
                        if (nWinNum!=SDRVIEWWIN_NOTFOUND) {
                            if (IsShownXorVisibleWinNum(nWinNum)) {
                                ToggleShownXor(&aVDev,NULL);
                            }
                        }
                        Point aCopyOfs(aDirtyArea.TopLeft());
                        aCopyOfs.X()-=a2Pix.Width();
                        aCopyOfs.Y()-=a2Pix.Height();
                        Size aCopySize(aBound.Right()-aBound.Left(),aBound.Bottom()-aBound.Top());
                        aCopySize.Width()+=2*a2Pix.Width();
                        aCopySize.Height()+=2*a2Pix.Height();
                        pOut->DrawOutDev(aCopyOfs,aCopySize,aCopyOfs,aCopySize,aVDev);
                    }
                    pOL->NbcRemoveObject(pOL->GetObjCount()-1);
                    // Die Page brauchen die Objekte
                    // hier mal bitte eine Optimierung vornehmen
                    pAktCreate->SetPage(pCreatePV->GetPage());
                }
            }
        } else {
            pLibObjDragMeth->Mov(aPnt);
        }
    }
}

BOOL SdrCreateView::EndCreateObj(SdrCreateCmd eCmd)
{
    BOOL bRet=FALSE;
    SdrObject* pObjMerk=pAktCreate;
    SdrPageView* pPVMerk=pCreatePV;
    if (pAktCreate!=NULL) {
        if (pLibObjDragMeth==NULL) {
            ULONG nAnz=aDragStat.GetPointAnz();
            if (nAnz<=1 && eCmd==SDRCREATE_FORCEEND) {
                BrkCreateObj(); // Objekte mit nur einem Punkt gibt's nicht (zumindest noch nicht)
                return FALSE; // FALSE=Event nicht ausgewertet
            }
            BOOL bPntsEq=nAnz>1;
            ULONG i=1;
            Point aP0=aDragStat.GetPoint(0);
            while (bPntsEq && i<nAnz) { bPntsEq=aP0==aDragStat.GetPoint(i); i++; }
            if (pAktCreate->EndCreate(aDragStat,eCmd)) {
                if (!IsSolidDraggingNow()) HideCreateObj(pDragWin,TRUE);
                if (!bPntsEq) { // sonst Brk, weil alle Punkte gleich sind.
                    SdrObject* pObj=pAktCreate;
                    pAktCreate=NULL;
                    SdrLayerAdmin& rAd=pCreatePV->GetPage()->GetLayerAdmin();
                    SdrLayerID nLayer=rAd.GetLayerID(aAktLayer,TRUE);
                    if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
                    pObj->SetLayer(nLayer);
                    InsertObject(pObj,*pCreatePV,IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);
                    pCreatePV=NULL;
                    bRet=TRUE; // TRUE=Event ausgewertet
                } else {
                    BrkCreateObj();
                }
            } else { // Mehr Punkte
                if (eCmd==SDRCREATE_FORCEEND || // nix da, Ende erzwungen
                    nAnz==0 ||                             // keine Punkte da (kann eigentlich nicht vorkommen)
                    (nAnz<=1 && !aDragStat.IsMinMoved())) { // MinMove nicht erfuellt
                    BrkCreateObj();
                } else {
                    XPolyPolygon aXPP1;
                    pAktCreate->TakeCreatePoly(aDragStat,aXPP1);
                    XPolyPolygon aXPP2(aXPP1); // kopieren, weil wird in DrawCreateDiff geaendert
                    if (!IsSolidDraggingNow()) DrawCreateObjDiff(pCreatePV->DragPoly(),aXPP1);
                    pCreatePV->DragPoly()=aXPP2;
                    aDragStat.ResetMinMoved(); // NextPoint gibt's bei MovCreateObj()
                    bRet=TRUE;
                }
            }
            if (bRet && pObjMerk!=NULL && IsTextEditAfterCreate()) {
                SdrTextObj* pText=PTR_CAST(SdrTextObj,pObjMerk);
                if (pText!=NULL && pText->IsTextFrame()) {
                    BegTextEdit(pText,pPVMerk,(Window*)NULL,TRUE,(SdrOutliner*)NULL,(OutlinerView*)NULL);
                }
            }
        } else {
            if (pLibObjDragMeth->End(FALSE)) {
                pAktCreate=NULL;
                pCreatePV=NULL;
                SdrLayerAdmin& rAd=pPVMerk->GetPage()->GetLayerAdmin();
                SdrLayerID nLayer=rAd.GetLayerID(aAktLayer,TRUE);
                if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
                pObjMerk->SetLayer(nLayer);
                InsertObject(pObjMerk,*pPVMerk);
                bRet=TRUE;
            } else {
                delete pAktCreate;
                bRet=FALSE;
            }
            delete pLibObjDragMeth;
            pLibObjDragMeth=NULL;
            pAktCreate=NULL;
            pCreatePV=NULL;
        }
        if (pAktCreate==NULL) {
            SetSolidDraggingNow(FALSE);
            SetSolidDraggingCheck(FALSE);
        }
    }
    return bRet;
}

void SdrCreateView::BckCreateObj()
{
    if (pAktCreate!=NULL) {
        if (aDragStat.GetPointAnz()<=2 || pLibObjDragMeth!=NULL) {
            BrkCreateObj();
        } else {
            HideCreateObj(pDragWin,TRUE);
            aDragStat.PrevPoint();
            if (pAktCreate->BckCreate(aDragStat)) {
                ShowCreateObj(pDragWin,TRUE);
            } else {
                BrkCreateObj();
            }
        }
    }
}

void SdrCreateView::BrkCreateObj()
{
    if (pAktCreate!=NULL) {
        if (pLibObjDragMeth==NULL) {
            if (!IsSolidDraggingNow()) {
                HideCreateObj(pDragWin,TRUE);
            } else {
                Rectangle aBound(pAktCreate->GetBoundRect());
                Point aPvOfs(pCreatePV->GetOffset());
                aBound.Move(aPvOfs.X(),aPvOfs.Y());
                InvalidateAllWin(aBound);
            }
            pAktCreate->BrkCreate(aDragStat);
        } else {
            pLibObjDragMeth->Brk();
            delete pLibObjDragMeth;
            pLibObjDragMeth=NULL;
        }
        delete pAktCreate;
        pAktCreate=NULL;
        pCreatePV=NULL;
        SetSolidDraggingNow(FALSE);
        SetSolidDraggingCheck(FALSE);
    }
}

void SdrCreateView::DrawCreateObjDiff(XPolyPolygon& rXPP0, XPolyPolygon& rXPP1)
{
    if (IsSolidDraggingNow()) return;
    USHORT nPolyAnz0=rXPP0.Count();
    USHORT nPolyAnz1=rXPP1.Count();
    USHORT nMinPolyAnz=Min(nPolyAnz0,nPolyAnz1);
    USHORT nPolyNum;
    for (nPolyNum=nMinPolyAnz; nPolyNum>0;) { // unveraenderte Polygone entfernen
        nPolyNum--;
        if (rXPP0[nPolyNum]==rXPP1[nPolyNum]) {
            rXPP0.Remove(nPolyNum);
            rXPP1.Remove(nPolyNum);
        }
    }
    nPolyAnz0=rXPP0.Count();
    nPolyAnz1=rXPP1.Count();
    nMinPolyAnz=Min(nPolyAnz0,nPolyAnz1);
    BOOL bNeedDot=FALSE;
    for (nPolyNum=nMinPolyAnz; nPolyNum>0;) { // unveraenderte Polygonteile entfernen
        nPolyNum--;
        const XPolygon& rXP0=rXPP0[nPolyNum];
        const XPolygon& rXP1=rXPP1[nPolyNum];
        USHORT nPntAnz0=rXP0.GetPointCount();
        USHORT nPntAnz1=rXP1.GetPointCount();
        USHORT nMinPntAnz=Min(nPntAnz0,nPntAnz1);
        USHORT nEquCnt0=0;
        USHORT nEquCnt1=0;
        for (USHORT nPntNum=0; nPntNum<nMinPntAnz; nPntNum++) {
            if (rXP0[nPntNum]==rXP1[nPntNum]) {
                nEquCnt0=nEquCnt1;
                BOOL bCtrl0=(nPntNum<nMinPntAnz) && rXP0.IsControl(nPntNum+1);
                BOOL bCtrl1=(nPntNum<nMinPntAnz) && rXP1.IsControl(nPntNum+1);
                if (bCtrl0==bCtrl1) {
                    nEquCnt1=nPntNum+1;
                    if (bCtrl0) {
                        nPntNum++;
                        if (rXP0[nPntNum]==rXP1[nPntNum]) {
                            nPntNum++;
                            if (rXP0[nPntNum]==rXP1[nPntNum]) {
                                nEquCnt1=nPntNum+1;
                            } else nPntNum=nMinPntAnz; // sowas wie break.
                        } else nPntNum=nMinPntAnz; // sowas wie break.
                    }
                } else nPntNum=nMinPntAnz; // sowas wie break.
            } else nPntNum=nMinPntAnz; // sowas wie break.
        }
        if (nEquCnt0!=0) {
            rXPP0[nPolyNum].Remove(0,nEquCnt0);
            rXPP1[nPolyNum].Remove(0,nEquCnt0);
#ifndef MAC // anderes Polygonpainting beim Mac
            if (nPolyNum==nMinPolyAnz-1 &&
                (rXPP0[nPolyNum].GetPointCount()<=1 ||
                 rXPP1[nPolyNum].GetPointCount()<=1)) bNeedDot=TRUE;
#endif
        }
    }
    aDragStat.SetShown(TRUE);
    USHORT nOutNum=0;
    do {
        Window* pO= (Window*)pDragWin;
        if (pO==NULL) {
            pO=(Window*)GetWin(nOutNum);
            nOutNum++;
        }
        if (pO!=NULL) {
            ImpSdrHdcMerk aHDCMerk(*pO,SDRHDC_SAVEALL,bRestoreColors);
            RasterOp eRop0=pO->GetRasterOp();
            pO->SetRasterOp(ROP_INVERT);
            pXOut->SetOutDev(pO);

            Color aBlackColor( COL_BLACK );
            Color aTranspColor( COL_TRANSPARENT );
            pXOut->OverrideLineColor( aBlackColor );
            pXOut->OverrideFillColor( aTranspColor );
            pXOut->SetOffset(pCreatePV->GetOffset());
            USHORT nAnz=rXPP0.Count();
            USHORT i;
            for (i=0; i<nAnz; i++) {
                pXOut->DrawXPolyLine(rXPP0[i]);
            }
            nAnz=rXPP1.Count();
            for (i=0; i<nAnz; i++) {
                pXOut->DrawXPolyLine(rXPP1[i]);
                if (bNeedDot && nPolyNum==nMinPolyAnz-1) {
                    Point aPt(rXPP1[i][0]);
                    //pO->InvertRect(Rectangle(aPt,aPt));
                    pO->Invert( Rectangle( aPt, aPt ) );
                }
            }
            pXOut->SetOffset(Point(0,0));
            pO->SetRasterOp(eRop0);
            if (bRestoreColors) aHDCMerk.Restore(*pO);
        }
    } while (pDragWin==NULL && nOutNum<GetWinCount());
}

void SdrCreateView::DrawCreateObj(OutputDevice* pOut, BOOL bFull) const
{
    if (IsSolidDraggingNow()) return;
    if (IsCreateObj()) {
        USHORT i=0;
        do {
            OutputDevice* pO=pOut;
            if (pO==NULL) {
                pO=GetWin(i);
                i++;
            }
            if (pO!=NULL) {
                ImpSdrHdcMerk aHDCMerk(*pO,SDRHDC_SAVEALL,bRestoreColors);
                RasterOp eRop0=pO->GetRasterOp();
                pO->SetRasterOp(ROP_INVERT);
                pXOut->SetOutDev(pO);
                Color aBlackColor( COL_BLACK );
                Color aTranspColor( COL_TRANSPARENT );
                pXOut->OverrideLineColor( aBlackColor );
                pXOut->OverrideFillColor( aTranspColor );
                pXOut->SetOffset(pCreatePV->GetOffset());
                XPolyPolygon aXPP(pCreatePV->DragPoly());
                USHORT nAnz=aXPP.Count();
                for (USHORT i=0; i<nAnz; i++) {
                    pXOut->DrawXPolyLine(aXPP[i]);
                }
                pXOut->SetOffset(Point(0,0));
                pO->SetRasterOp(eRop0);
                if (bRestoreColors) aHDCMerk.Restore(*pO);
            }
        } while (pOut==NULL && i<GetWinCount());
    }
}

void SdrCreateView::ShowCreateObj(OutputDevice* pOut, BOOL bFull)
{
    if (IsCreateObj() && !aDragStat.IsShown()) {
        XPolyPolygon aXPP;
        if (pLibObjDragMeth==NULL) {
            pAktCreate->TakeCreatePoly(aDragStat,pCreatePV->DragPoly());
        }
        DrawCreateObj(pOut,bFull);
        aDragStat.SetShown(TRUE);
    }
}

void SdrCreateView::HideCreateObj(OutputDevice* pOut, BOOL bFull)
{
    if (IsCreateObj() && aDragStat.IsShown()) {
        DrawCreateObj(pOut,bFull);
        aDragStat.SetShown(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* new interface src537 */
BOOL SdrCreateView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    if(pAktCreate)
    {
        pAktCreate->TakeAttributes(rTargetSet, FALSE, bOnlyHardAttr);
        return TRUE;
    }
    else
    {
        return SdrDragView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

BOOL SdrCreateView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    if (pAktCreate!=NULL) {
        pAktCreate->SetAttributes(rSet,bReplaceAll);
        return TRUE;
    } else {
        return SdrDragView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrCreateView::GetStyleSheet(BOOL& rOk) const
{
    if (pAktCreate!=NULL) {
        rOk=TRUE;
        return pAktCreate->GetStyleSheet();
    } else {
        return SdrDragView::GetStyleSheet(rOk);
    }
}

BOOL SdrCreateView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    if (pAktCreate!=NULL) {
        pAktCreate->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        return TRUE;
    } else {
        return SdrDragView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCreateView::WriteRecords(SvStream& rOut) const
{
    SdrDragView::WriteRecords(rOut);
    {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCROBJECT);
        rOut<<nAktInvent;
        rOut<<nAktIdent;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCRFLAGS);
        rOut<<BOOL(b1stPointAsCenter);
    } {
        // in der CreateView (statt ObjEditView) weil sonst inkompatibel.
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWTEXTEDIT);
        rOut<<BOOL(bQuickTextEditMode);
    } {
        // in der CreateView (statt ObjEditView) weil sonst inkompatibel.
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWMACRO);
        rOut<<BOOL(bMacroMode);
    }
}

BOOL SdrCreateView::ReadRecord(const SdrIOHeader& rViewHead,
    const SdrNamedSubRecord& rSubHead,
    SvStream& rIn)
{
    BOOL bRet=FALSE;
    if (rSubHead.GetInventor()==SdrInventor) {
        bRet=TRUE;
        switch (rSubHead.GetIdentifier()) {
            case SDRIORECNAME_VIEWCROBJECT: {
                UINT32 nInvent;
                UINT16 nIdent;
                rIn>>nInvent;
                rIn>>nIdent;
                SetCurrentObj(nIdent,nInvent);
            } break;
            case SDRIORECNAME_VIEWCRFLAGS: {
                BOOL bTmp; rIn>>bTmp; b1stPointAsCenter=bTmp;
            } break;
            case SDRIORECNAME_VIEWTEXTEDIT: {
                // in der CreateView (statt ObjEditView) weil sonst inkompatibel.
                BOOL bTmp; rIn>>bTmp; bQuickTextEditMode=bTmp;
            } break;
            case SDRIORECNAME_VIEWMACRO: {
                // in der CreateView (statt ObjEditView) weil sonst inkompatibel.
                BOOL bTmp; rIn>>bTmp; bMacroMode=bTmp;
            } break;
            default: bRet=FALSE;
        }
    }
    if (!bRet) bRet=SdrDragView::ReadRecord(rViewHead,rSubHead,rIn);
    return bRet;
}

