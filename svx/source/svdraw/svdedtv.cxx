/*************************************************************************
 *
 *  $RCSfile: svdedtv.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-15 16:11:33 $
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

#include <vcl/metaact.hxx>
#include "svdedtv.hxx"
#include "svdxout.hxx"
#include "svdundo.hxx"
#include "svdograf.hxx"  // fuer Possibilities
#include "svdopath.hxx"
#include "svdoole2.hxx"
#include "svdopage.hxx"
#include "svdoedge.hxx"
#include "svdlayer.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdpoev.hxx"  // fuer die PolyPossiblities
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//  @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ImpResetPossibilityFlags()
{
    bReadOnly               =FALSE;

    bGroupPossible          =FALSE;
    bUnGroupPossible        =FALSE;
    bGrpEnterPossible       =FALSE;
    bDeletePossible         =FALSE;
    bToTopPossible          =FALSE;
    bToBtmPossible          =FALSE;
    bReverseOrderPossible   =FALSE;

    bImportMtfPossible      =FALSE;
    bCombinePossible        =FALSE;
    bDismantlePossible      =FALSE;
    bCombineNoPolyPolyPossible =FALSE;
    bDismantleMakeLinesPossible=FALSE;
    bOrthoDesiredOnMarked   =FALSE;

    bMoreThanOneNotMovable  =FALSE;
    bOneOrMoreMovable       =FALSE;
    bMoreThanOneNoMovRot    =FALSE;
    bContortionPossible     =FALSE;
    bAllPolys               =FALSE;
    bOneOrMorePolys         =FALSE;
    bMoveAllowed            =FALSE;
    bResizeFreeAllowed      =FALSE;
    bResizePropAllowed      =FALSE;
    bRotateFreeAllowed      =FALSE;
    bRotate90Allowed        =FALSE;
    bMirrorFreeAllowed      =FALSE;
    bMirror45Allowed        =FALSE;
    bMirror90Allowed        =FALSE;
    bTransparenceAllowed    =FALSE;
    bGradientAllowed        =FALSE;
    bShearAllowed           =FALSE;
    bEdgeRadiusAllowed      =FALSE;
    bCanConvToPath          =FALSE;
    bCanConvToPoly          =FALSE;
    bCanConvToContour       =FALSE;
    bCanConvToPathLineToArea=FALSE;
    bCanConvToPolyLineToArea=FALSE;
    bMoveProtect            =FALSE;
    bResizeProtect          =FALSE;
}

void SdrEditView::ImpClearVars()
{
    ImpResetPossibilityFlags();
    bPossibilitiesDirty=TRUE;   // << war von Purify angemeckert
    bCombineError=FALSE;
    bBundleVirtObj=FALSE;
}

SdrEditView::SdrEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrMarkView(pModel1,pOut)
{
    ImpClearVars();
}

SdrEditView::SdrEditView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrMarkView(pModel1,pXOut)
{
    ImpClearVars();
}

SdrEditView::~SdrEditView()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrLayer* SdrEditView::InsertNewLayer(const XubString& rName, USHORT nPos)
{
    SdrLayerAdmin& rLA=pMod->GetLayerAdmin();
    USHORT nMax=rLA.GetLayerCount();
    if (nPos>nMax) nPos=nMax;
    SdrLayer* pNewLayer=rLA.NewLayer(rName,nPos);
    AddUndo(new SdrUndoNewLayer(nPos,rLA,*pMod));
    pMod->SetChanged();
    return pNewLayer;
}

BOOL SdrEditView::ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const
{
    BOOL bDelAll=TRUE;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG nObjNum=nObjAnz; nObjNum>0 && bDelAll;) {
        nObjNum--;
        SdrObject* pObj=pOL->GetObj(nObjNum);
        SdrObjList* pSubOL=pObj->GetSubList();
        if (pSubOL!=NULL) { // Gruppenobjekt
            if (!ImpDelLayerCheck(pSubOL,nDelID)) bDelAll=FALSE; // Rekursion
        } else {
            if (pObj->GetLayer()!=nDelID) bDelAll=FALSE;
        }
    }
    return bDelAll;
}

void SdrEditView::ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID)
{
    ULONG nObjAnz=pOL->GetObjCount();
    pOL->GetObj(0)->GetOrdNum(); // Sicherstellen, dass die OrdNums stimmen
    for (ULONG nObjNum=nObjAnz; nObjNum>0;) {
        nObjNum--;
        SdrObject* pObj=pOL->GetObj(nObjNum);
        SdrObjList* pSubOL=pObj->GetSubList();
        if (pSubOL!=NULL) { // Gruppenobjekt
            if (ImpDelLayerCheck(pSubOL,nDelID)) {
                AddUndo(new SdrUndoDelObj(*pObj,TRUE));
                pOL->RemoveObject(nObjNum);
            } else {
                ImpDelLayerDelObjs(pSubOL,nDelID);
            }
        } else {
            if (pObj->GetLayer()==nDelID) {
                AddUndo(new SdrUndoDelObj(*pObj,TRUE));
                pOL->RemoveObject(nObjNum);
            }
        }
    }
}

void SdrEditView::DeleteLayer(const XubString& rName)
{
    SdrLayerAdmin& rLA=pMod->GetLayerAdmin();
    SdrLayer* pLayer=rLA.GetLayer(rName,TRUE);
    USHORT nLayerNum=rLA.GetLayerPos(pLayer);
    if (nLayerNum!=SDRLAYER_NOTFOUND) {
        SdrLayerID nDelID=pLayer->GetID();
        BegUndo(ImpGetResStr(STR_UndoDelLayer));
        BOOL bMaPg=TRUE;
        for (USHORT nPageKind=0; nPageKind<2; nPageKind++) { // MasterPages und Zeichenseiten
            USHORT nPgAnz=bMaPg ? pMod->GetMasterPageCount() : pMod->GetPageCount();
            for (USHORT nPgNum=0; nPgNum<nPgAnz; nPgNum++) { // ueber alle Seiten
                SdrPage* pPage=bMaPg ? pMod->GetMasterPage(nPgNum) : pMod->GetPage(nPgNum);
                ULONG nObjAnz=pPage->GetObjCount();
                if (nObjAnz!=0) pPage->GetObj(0)->GetOrdNum(); // Sicherstellen, dass die OrdNums stimmen
                for (ULONG nObjNum=nObjAnz; nObjNum>0;) {
                    nObjNum--;
                    SdrObject* pObj=pPage->GetObj(nObjNum);
                    SdrObjList* pSubOL=pObj->GetSubList();
                    if (pSubOL!=NULL) { // Gruppenobjekt
                        if (ImpDelLayerCheck(pSubOL,nDelID)) {
                            AddUndo(new SdrUndoDelObj(*pObj,TRUE));
                            pPage->RemoveObject(nObjNum);
                        } else {
                            ImpDelLayerDelObjs(pSubOL,nDelID);
                        }
                    } else {
                        if (pObj->GetLayer()==nDelID) {
                            AddUndo(new SdrUndoDelObj(*pObj,TRUE));
                            pPage->RemoveObject(nObjNum);
                        }
                    }
                }
            }
            bMaPg=FALSE;
        }
        AddUndo(new SdrUndoDelLayer(nLayerNum,rLA,*pMod));
        rLA.RemoveLayer(nLayerNum);
        EndUndo();
        pMod->SetChanged();
    }
}

void SdrEditView::MoveLayer(const XubString& rName, USHORT nNewPos)
{
    SdrLayerAdmin& rLA=pMod->GetLayerAdmin();
    SdrLayer* pLayer=rLA.GetLayer(rName,TRUE);
    USHORT nLayerNum=rLA.GetLayerPos(pLayer);
    if (nLayerNum!=SDRLAYER_NOTFOUND) {
        AddUndo(new SdrUndoMoveLayer(nLayerNum,rLA,*pMod,nNewPos));
        rLA.MoveLayer(nLayerNum,nNewPos);
        pMod->SetChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::EndUndo()
{
    pMod->EndUndo();
    if (pMod->GetUndoBracketLevel()==0) {
        ImpBroadcastEdgesOfMarkedNodes();
    }
}

void SdrEditView::ImpBroadcastEdgesOfMarkedNodes()
{
    ForceEdgesOfMarkedNodes();
    ULONG nMarkedEdgeAnz=aMarkedEdges.GetMarkCount();
    USHORT i;
    for (i=0; i<nMarkedEdgeAnz; i++) {
        SdrMark* pEM=aMarkedEdges.GetMark(i);
        SdrObject* pEdgeTmp=pEM->GetObj();
        SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pEdgeTmp);
        if (pEdge!=NULL && TRUE) {
            SdrObject* pObj1=pEdge->GetConnectedNode(FALSE);
            SdrObject* pObj2=pEdge->GetConnectedNode(TRUE);
            if (pObj1!=NULL && !IsObjMarked(pObj1) && !pEdge->CheckNodeConnection(FALSE)) pEdge->DisconnectFromNode(FALSE);
            if (pObj2!=NULL && !IsObjMarked(pObj2) && !pEdge->CheckNodeConnection(TRUE)) pEdge->DisconnectFromNode(TRUE);
        }
    }
    nMarkedEdgeAnz=aMarkedEdgesOfMarkedNodes.GetMarkCount();
    for (i=0; i<nMarkedEdgeAnz; i++) {
        SdrMark* pEM=aMarkedEdgesOfMarkedNodes.GetMark(i);
        SdrObject* pEdgeTmp=pEM->GetObj();
        SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pEdgeTmp);
        if (pEdge!=NULL) {
            pEdge->SetEdgeTrackDirty();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ####   ###   ####  #### # ####  # #    # ##### # #####  ####
//  #   # #   # #     #     # #   # # #    #   #   # #     #
//  ####  #   #  ###   ###  # ####  # #    #   #   # ####   ###
//  #     #   #     #     # # #   # # #    #   #   # #         #
//  #      ###  ####  ####  # ####  # #### #   #   # ##### ####
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::MarkListHasChanged()
{
    SdrMarkView::MarkListHasChanged();
    bPossibilitiesDirty=TRUE;
}

void SdrEditView::ModelHasChanged()
{
    SdrMarkView::ModelHasChanged();
    bPossibilitiesDirty=TRUE;
}

BOOL SdrEditView::IsResizeAllowed(BOOL bProp) const
{
    ForcePossibilities();
    if (bResizeProtect) return FALSE;
    if (bProp) return bResizePropAllowed;
    return bResizeFreeAllowed;
}

BOOL SdrEditView::IsRotateAllowed(BOOL b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return FALSE;
    if (b90Deg) return bRotate90Allowed;
    return bRotateFreeAllowed;
}

BOOL SdrEditView::IsMirrorAllowed(BOOL b45Deg, BOOL b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return FALSE;
    if (b90Deg) return bMirror90Allowed;
    if (b45Deg) return bMirror45Allowed;
    return bMirrorFreeAllowed && !bMoveProtect;
}

BOOL SdrEditView::IsTransparenceAllowed() const
{
    ForcePossibilities();
    return bTransparenceAllowed;
}

BOOL SdrEditView::IsGradientAllowed() const
{
    ForcePossibilities();
    return bGradientAllowed;
}

BOOL SdrEditView::IsShearAllowed() const
{
    ForcePossibilities();
    if (bResizeProtect) return FALSE;
    return bShearAllowed;
}

BOOL SdrEditView::IsEdgeRadiusAllowed() const
{
    ForcePossibilities();
    return bEdgeRadiusAllowed;
}

BOOL SdrEditView::IsCrookAllowed(BOOL bNoContortion) const
{
    // CrookMode fehlt hier (weil kein Rotate bei Shear ...)
    ForcePossibilities();
    if (bNoContortion) {
        if (!bRotateFreeAllowed) return FALSE; // Crook is nich
        return !bMoveProtect && bMoveAllowed;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

BOOL SdrEditView::IsDistortAllowed(BOOL bNoContortion) const
{
    ForcePossibilities();
    if (bNoContortion) {
        return FALSE;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

BOOL SdrEditView::IsCombinePossible(BOOL bNoPolyPoly) const
{
    ForcePossibilities();
    if (bNoPolyPoly) return bCombineNoPolyPolyPossible;
    else return bCombinePossible;
}

BOOL SdrEditView::IsDismantlePossible(BOOL bMakeLines) const
{
    ForcePossibilities();
    if (bMakeLines) return bDismantleMakeLinesPossible;
    else return bDismantlePossible;
}

void SdrEditView::CheckPossibilities()
{
    if (bSomeObjChgdFlag) bPossibilitiesDirty=TRUE;

    if(bSomeObjChgdFlag)
    {
        // This call IS necessary to correct the MarkList, in which
        // no longer to the model belonging objects still can reside.
        // These ones nned to be removed.
        CheckMarked();
    }

    if (bPossibilitiesDirty) {
        ImpResetPossibilityFlags();
        aMark.ForceSort();
        ULONG nMarkAnz=aMark.GetMarkCount();
        if (nMarkAnz!=0) {
            bReverseOrderPossible=nMarkAnz>=2;

            ULONG nMovableCount=0;
            bGroupPossible=nMarkAnz>=2;
            bCombinePossible=nMarkAnz>=2;
            if (nMarkAnz==1) {
                // bCombinePossible gruendlicher checken
                // fehlt noch ...
                const SdrObject* pObj=aMark.GetMark(0)->GetObj();
                const SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
                BOOL bGroup=pObj->GetSubList()!=NULL;
                BOOL bHasText=pObj->GetOutlinerParaObject()!=NULL;
                if (bGroup || bHasText) {
                    bCombinePossible=TRUE;
                } else {
                    // folgendes Statemant macht IMHO keinen Sinn (Bugfix am 27-11-1995 Combine und Fontwork):
                    //bCombinePossible=pPath->GetPathPoly().Count()>1;
                }
            }
            bCombineNoPolyPolyPossible=bCombinePossible;
            bDeletePossible=TRUE;
            // Zu den Transformationen erstmal ja sagen
            bMoveAllowed      =TRUE;
            bResizeFreeAllowed=TRUE;
            bResizePropAllowed=TRUE;
            bRotateFreeAllowed=TRUE;
            bRotate90Allowed  =TRUE;
            bMirrorFreeAllowed=TRUE;
            bMirror45Allowed  =TRUE;
            bMirror90Allowed  =TRUE;
            bShearAllowed     =TRUE;
            bEdgeRadiusAllowed=FALSE;
            bContortionPossible=TRUE;
            bCanConvToContour = TRUE;

            // these ones are only allowed when single object is selected
            bTransparenceAllowed = (nMarkAnz == 1);
            bGradientAllowed = (nMarkAnz == 1);
            if(bGradientAllowed)
            {
                // gradient depends on fillstyle
                const SdrMark* pM = aMark.GetMark(0);
                const SdrObject* pObj = pM->GetObj();


                XFillStyle eFillStyle = ((XFillStyleItem&)(pObj->GetItem(XATTR_FILLSTYLE))).GetValue();

                if(eFillStyle != XFILL_GRADIENT)
                {
                    bGradientAllowed = FALSE;
                }
            }

            BOOL bNoMovRotFound=FALSE;
            const SdrPageView* pPV0=NULL;
            for (ULONG nm=0; nm<nMarkAnz; nm++) {
                const SdrMark* pM=aMark.GetMark(nm);
                const SdrObject* pObj=pM->GetObj();
                const SdrPageView* pPV=pM->GetPageView();
                if (pPV!=pPV0) {
                    if (pPV->IsReadOnly()) bReadOnly=TRUE;
                    pPV0=pPV;
                }
                SdrObjTransformInfoRec aInfo;
                pObj->TakeObjInfo(aInfo);
                BOOL bMovPrt=pObj->IsMoveProtect();
                BOOL bSizPrt=pObj->IsResizeProtect();
                if (!bMovPrt && aInfo.bMoveAllowed) nMovableCount++; // Menge der MovableObjs zaehlen
                if (bMovPrt) bMoveProtect=TRUE;
                if (bSizPrt) bResizeProtect=TRUE;

                // not allowed when not allowed at one object
                if(!aInfo.bTransparenceAllowed)
                    bTransparenceAllowed = FALSE;

                // Wenn einer was nicht kann, duerfen's alle nicht
                if (!aInfo.bMoveAllowed      ) bMoveAllowed      =FALSE;
                if (!aInfo.bResizeFreeAllowed) bResizeFreeAllowed=FALSE;
                if (!aInfo.bResizePropAllowed) bResizePropAllowed=FALSE;
                if (!aInfo.bRotateFreeAllowed) bRotateFreeAllowed=FALSE;
                if (!aInfo.bRotate90Allowed  ) bRotate90Allowed  =FALSE;
                if (!aInfo.bMirrorFreeAllowed) bMirrorFreeAllowed=FALSE;
                if (!aInfo.bMirror45Allowed  ) bMirror45Allowed  =FALSE;
                if (!aInfo.bMirror90Allowed  ) bMirror90Allowed  =FALSE;
                if (!aInfo.bShearAllowed     ) bShearAllowed     =FALSE;
                if (aInfo.bEdgeRadiusAllowed) bEdgeRadiusAllowed=TRUE;
                if (aInfo.bNoContortion      ) bContortionPossible=FALSE;
                // Fuer Crook mit Contortion: Alle Objekte muessen
                // Movable und Rotatable sein, ausser maximal 1
                if (!bMoreThanOneNoMovRot) {
                    if (!aInfo.bMoveAllowed || !aInfo.bResizeFreeAllowed) {
                        bMoreThanOneNoMovRot=bNoMovRotFound;
                        bNoMovRotFound=TRUE;
                    }
                }

                // when one member cannot be converted, no conversion is possible
                if(!aInfo.bCanConvToContour)
                    bCanConvToContour = FALSE;

                // Ungroup
                if (!bUnGroupPossible) bUnGroupPossible=pObj->GetSubList()!=NULL;
                // ConvertToCurve: Wenn mind. einer konvertiert werden kann ist das ok.
                if (aInfo.bCanConvToPath          ) bCanConvToPath          =TRUE;
                if (aInfo.bCanConvToPoly          ) bCanConvToPoly          =TRUE;
                if (aInfo.bCanConvToPathLineToArea) bCanConvToPathLineToArea=TRUE;
                if (aInfo.bCanConvToPolyLineToArea) bCanConvToPolyLineToArea=TRUE;
                // Combine/Dismantle
                if (bCombinePossible) {
                    bCombinePossible=ImpCanConvertForCombine(pObj);
                    bCombineNoPolyPolyPossible=bCombinePossible;
                }
                if (!bDismantlePossible) bDismantlePossible=ImpCanDismantle(pObj,FALSE);
                if (!bDismantleMakeLinesPossible) bDismantleMakeLinesPossible=ImpCanDismantle(pObj,TRUE);
                // OrthoDesiredOnMarked checken
                if (!bOrthoDesiredOnMarked && !aInfo.bNoOrthoDesired) bOrthoDesiredOnMarked=TRUE;
                // ImportMtf checken

                if (!bImportMtfPossible) {
                    BOOL bGraf=HAS_BASE(SdrGrafObj,pObj);
                    BOOL bOle2=HAS_BASE(SdrOle2Obj,pObj);

                    if( bGraf && ((SdrGrafObj*)pObj)->HasGDIMetaFile() && !((SdrGrafObj*)pObj)->IsEPS() )
                        bImportMtfPossible = TRUE;

                    if (bOle2)
                        bImportMtfPossible=((SdrOle2Obj*)pObj)->HasGDIMetaFile();
                }
            }
            bMoreThanOneNotMovable=nMovableCount<nMarkAnz-1;
            bOneOrMoreMovable=nMovableCount!=0;
            bGrpEnterPossible=bUnGroupPossible;
        }
        ImpCheckToTopBtmPossible();
        ((SdrPolyEditView*)this)->ImpCheckPolyPossibilities();
        bPossibilitiesDirty=FALSE;

        if (bReadOnly) {
            BOOL bMerker1=bGrpEnterPossible;
            ImpResetPossibilityFlags();
            bReadOnly=TRUE;
            bGrpEnterPossible=bMerker1;
        }
        if (bMoveAllowed) {
            // Verschieben von angeklebten Verbindern unterbinden
            // Derzeit nur fuer Einfachselektion implementiert.
            if (nMarkAnz==1) {
                SdrObject* pObj=aMark.GetMark(0)->GetObj();
                SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pObj);
                if (pEdge!=NULL) {
                    SdrObject* pNode1=pEdge->GetConnectedNode(TRUE);
                    SdrObject* pNode2=pEdge->GetConnectedNode(FALSE);
                    if (pNode1!=NULL || pNode2!=NULL) bMoveAllowed=FALSE;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ForceMarkedObjToAnotherPage()
{
    BOOL bFlg=FALSE;
    for (ULONG nm=0; nm<aMark.GetMarkCount(); nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        Rectangle aObjRect(pObj->GetBoundRect());
        aObjRect+=pM->GetPageView()->GetOffset(); // auf View-Koordinaten
        Rectangle aPgRect(pM->GetPageView()->GetPageRect());
        if (!aObjRect.IsOver(aPgRect)) {
            BOOL bFnd=FALSE;
            SdrPageView* pPV;
            for (USHORT nv=GetPageViewCount(); nv>0 && !bFnd;) {
                nv--;
                pPV=GetPageViewPvNum(nv);
                bFnd=aObjRect.IsOver(pPV->GetPageRect());
            }
            if (bFnd) {
                pM->GetPageView()->GetObjList()->RemoveObject(pObj->GetOrdNum());
                Point aDelta(pM->GetPageView()->GetOffset()-pPV->GetOffset());
                pObj->Move(Size(aDelta.X(),aDelta.Y()));
                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pPV->GetObjList()->InsertObject(pObj,CONTAINER_APPEND,&aReason);
                pM->SetPageView(pPV);
                InvalidateAllWin(aObjRect);
                bFlg=TRUE;
            }
        }
    }
    if (bFlg) {
        MarkListHasChanged();
    }
}

void SdrEditView::DeleteMarked(SdrMarkList& rMark)
{
    if (rMark.GetMarkCount()!=0) {
        rMark.ForceSort();
        BegUndo();
        ULONG nMarkAnz=rMark.GetMarkCount();
        ULONG nm;
        for (nm=nMarkAnz; nm>0;) {
            nm--;
            SdrMark* pM=rMark.GetMark(nm);
            AddUndo(new SdrUndoDelObj(*pM->GetObj()));
        }
        // Sicherstellen, dass die OrderNums stimmen:
        rMark.GetMark(0)->GetObj()->GetOrdNum();
        for (nm=nMarkAnz; nm>0;) {
            nm--;
            SdrMark* pM=rMark.GetMark(nm);
            SdrObject*   pObj=pM->GetObj();
            SdrPageView* pPV =pM->GetPageView();
            SdrObjList*  pOL =pObj->GetObjList(); //#52680#
            UINT32 nOrdNum=pObj->GetOrdNumDirect();
            SdrObject* pChkObj=pOL->RemoveObject(nOrdNum);
            DBG_ASSERT(pChkObj==pObj,"DeleteMarked(MarkList): pChkObj!=pObj beim RemoveObject()");
        }
        EndUndo();
    }
}

void SdrEditView::DeleteMarkedObj()
{
    if (aMark.GetMarkCount()!=0) {
        nSpecialCnt=0;
        BrkAction();
        HideMarkHdl(NULL);
        BegUndo(ImpGetResStr(STR_EditDelete),aMark.GetMarkDescription(),SDRREPFUNC_OBJ_DELETE);
        DeleteMarked(aMark);
        aMark.Clear();
        aHdl.Clear();
        EndUndo();
        MarkListHasChanged();
    }
}

void SdrEditView::CopyMarkedObj()
{
    aMark.ForceSort();
    ForceEdgesOfMarkedNodes();

    SdrMarkList aSourceObjectsForCopy(aMark);
    // Folgende Schleife Anstatt MarkList::Merge(), damit
    // ich jeweils mein Flag an die MarkEntries setzen kann.
    ULONG nEdgeAnz=aEdgesOfMarkedNodes.GetMarkCount();
    for (ULONG nEdgeNum=0; nEdgeNum<nEdgeAnz; nEdgeNum++) {
        SdrMark aM(*aEdgesOfMarkedNodes.GetMark(nEdgeNum));
        aM.SetUser(1);
        aSourceObjectsForCopy.InsertEntry(aM);
    }
    aSourceObjectsForCopy.ForceSort();

    SdrMarkList aCopiedObjects;
    aMark.Clear();
    ULONG nCloneErrCnt=0;
    ULONG nMarkAnz=aSourceObjectsForCopy.GetMarkCount();
    ULONG nm;
    for (nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aSourceObjectsForCopy.GetMark(nm);
        SdrObject* pO=pM->GetObj()->Clone();
        if (pO!=NULL) {
            SdrInsertReason aReason(SDRREASON_VIEWCALL);
            pM->GetPageView()->GetObjList()->InsertObject(pO,CONTAINER_APPEND,&aReason);
            AddUndo(new SdrUndoCopyObj(*pO));
            SdrMark aME(*pM);
            aME.SetObj(pO);
            aCopiedObjects.InsertEntry(aME);
            if (pM->GetUser()==0) { // Sonst war's nur eine mitzukierende Edge
                aMark.InsertEntry(aME);
            }
        } else {
            nCloneErrCnt++;
        }
    }
    // und nun zu den Konnektoren
    // Die Objekte der MarkList aCopiedObjects werden auf die MarkList
    // aSourceObjectsForCopy abgebildet und so die Objektverbindungen
    // hergestellt.
    // Aehnliche Implementation an folgenden Stellen:
    //    void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
    //    SdrModel* SdrExchangeView::GetMarkedObjModel() const
    //    BOOL SdrExchangeView::Paste(const SdrModel& rMod,...)
    //    void SdrEditView::CopyMarkedObj()
    if (nCloneErrCnt==0) {
        for (nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=aSourceObjectsForCopy.GetMark(nm);
            SdrObject* pO=pM->GetObj();
            SdrEdgeObj* pSrcEdge=PTR_CAST(SdrEdgeObj,pO);
            if (pSrcEdge!=NULL) {
                SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(TRUE);
                SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(FALSE);
                if (pSrcNode1!=NULL && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=NULL; // Listenuebergreifend
                if (pSrcNode2!=NULL && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=NULL; // ist (noch) nicht
                if (pSrcNode1!=NULL || pSrcNode2!=NULL) {
                    SdrMark* pEdgeMark=aCopiedObjects.GetMark(nm);
                    BOOL bMitkopiert=pEdgeMark->GetUser()!=0;
                    SdrObject* pDstEdgeTmp=pEdgeMark->GetObj();
                    SdrEdgeObj* pDstEdge=PTR_CAST(SdrEdgeObj,pDstEdgeTmp);
                    if (pDstEdge!=NULL) {
                        if (pSrcNode1!=NULL) {
                            ULONG nDstNode1=aSourceObjectsForCopy.FindObject(pSrcNode1);
                            SdrObject* pDstNode1=NULL;
                            if (nDstNode1!=CONTAINER_ENTRY_NOTFOUND) {
                                pDstNode1=aCopiedObjects.GetMark(nDstNode1)->GetObj();
                            } else if (bMitkopiert) {
                                pDstNode1=pSrcNode1; // an den SourceNode Connekten wenn Edge mitkopiert
                            }
                            if (pDstNode1!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(TRUE,pDstNode1);
                            }
                        }
                        if (pSrcNode2!=NULL) {
                            ULONG nDstNode2=aSourceObjectsForCopy.FindObject(pSrcNode2);
                            SdrObject* pDstNode2=NULL;
                            if (nDstNode2!=CONTAINER_ENTRY_NOTFOUND) {
                                pDstNode2=aCopiedObjects.GetMark(nDstNode2)->GetObj();
                            } else if (bMitkopiert) {
                                pDstNode2=pSrcNode2; // an den SourceNode Connekten wenn Edge mitkopiert
                            }
                            if (pDstNode2!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(FALSE,pDstNode2);
                            }
                        }
                    } else {
                        DBG_ERROR("SdrEditView::CopyMarkedObj(): pDstEdge==NULL!");
                    }
                }
            }
        }
    } else {
#ifdef DBG_UTIL
        ByteString aStr("SdrEditView::CopyMarkedObj(): Fehler beim Clonen ");

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
    MarkListHasChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrEditView::InsertObject(SdrObject* pObj, SdrPageView& rPV, ULONG nOptions)
{
    if ((nOptions & SDRINSERT_SETDEFLAYER)!=0) {
        SdrLayerID nLayer=rPV.GetPage()->GetLayerAdmin().GetLayerID(aAktLayer,TRUE);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
        if (rPV.GetLockedLayers().IsSet(nLayer) || !rPV.GetVisibleLayers().IsSet(nLayer)) {
            delete pObj; // Layer gesperrt oder nicht sichtbar
            return FALSE;
        }
        pObj->NbcSetLayer(nLayer);
    }
    if ((nOptions & SDRINSERT_SETDEFATTR)!=0) {
        if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet,FALSE);
        pObj->SetItemSet(aDefaultAttr);
    }
    if (!pObj->IsInserted()) {
        SdrInsertReason aReason(SDRREASON_VIEWCALL);
        if ((nOptions & SDRINSERT_NOBROADCAST)!=0) {
            rPV.GetObjList()->NbcInsertObject(pObj,CONTAINER_APPEND,&aReason);
        } else {
            rPV.GetObjList()->InsertObject(pObj,CONTAINER_APPEND,&aReason);
        }
    }
    AddUndo(new SdrUndoNewObj(*pObj));
    if ((nOptions & SDRINSERT_DONTMARK)==0) {
        if ((nOptions & SDRINSERT_ADDMARK)==0) UnmarkAllObj();
        MarkObj(pObj,&rPV);
    }
    return TRUE;
}

void SdrEditView::ReplaceObject(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, BOOL bMark)
{
    SdrObjList* pOL=pOldObj->GetObjList();
    AddUndo(new SdrUndoReplaceObj(*pOldObj,*pNewObj));
    pOL->ReplaceObject(pNewObj,pOldObj->GetOrdNum());
    if (bMark) MarkObj(pNewObj,&rPV);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN
void SdrEditView::BegUndo()             // Undo-Klammerung auf
{
    pMod->BegUndo();
}

void SdrEditView::BegUndo(const String& rComment) // Undo-Klammerung auf
{
    pMod->BegUndo(rComment);
}

void SdrEditView::BegUndo(const String& rComment, const String& rObjDescr,
        SdrRepeatFunc eFunc) // Undo-Klammerung auf
{
    pMod->BegUndo(rComment,rObjDescr,eFunc);
}

void SdrEditView::BegUndo(SdrUndoGroup* pUndoGrp) // Undo-Klammerung auf
{
    pMod->BegUndo(pUndoGrp);
}

void SdrEditView::AddUndo(SdrUndoAction* pUndo)   // Action hinzufuegen
{
    pMod->AddUndo(pUndo);
}
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
void SdrEditView::SetUndoComment(const String& rComment)
{
    pMod->SetUndoComment(rComment);
}

void SdrEditView::SetUndoComment(const String& rComment,
                                const String& rObjDescr)
{
    pMod->SetUndoComment(rComment,rObjDescr);
}
#endif

