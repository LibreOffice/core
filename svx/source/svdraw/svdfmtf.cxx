/*************************************************************************
 *
 *  $RCSfile: svdfmtf.cxx,v $
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

#include "svdfmtf.hxx"
#include "editdata.hxx"
#include <math.h>
#include "xpoly.hxx"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _EEITEMID_HXX //autogen
#include <eeitemid.hxx>
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include <fhgtitem.hxx>
#endif

#ifndef _SVX_WGHTITEM_HXX //autogen
#include <wghtitem.hxx>
#endif

#ifndef _SVX_POSTITEM_HXX //autogen
#include <postitem.hxx>
#endif

#ifndef _SVX_UDLNITEM_HXX //autogen
#include <udlnitem.hxx>
#endif

#ifndef _SVX_CRSDITEM_HXX //autogen
#include <crsditem.hxx>
#endif

#ifndef _SVX_SHDDITEM_HXX //autogen
#include <shdditem.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XGRAD_HXX //autogen
#include <xgrad.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <xflgrit.hxx>
#endif

#ifndef _SVX_FWDTITEM_HXX //autogen
#include <fwdtitem.hxx>
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include <fontitem.hxx>
#endif

#ifndef _SVX_AKRNITEM_HXX //autogen
#include <akrnitem.hxx>
#endif

#ifndef _SVX_WRLMITEM_HXX //autogen
#include <wrlmitem.hxx>
#endif

#ifndef _SVX_ITEM_HXX //autogen
#include <cntritem.hxx>
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include <colritem.hxx>
#endif

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#include "xflhtit.hxx"
#include "svdattr.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdobj.hxx"
#include "svditext.hxx"
#include "svdotext.hxx"
#include "svdorect.hxx"
#include "svdocirc.hxx"
#include "svdograf.hxx"
#include "svdopath.hxx"
#include "svdetc.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpSdrGDIMetaFileImport::ImpSdrGDIMetaFileImport(SdrModel& rModel):
    nMapScalingOfs(0),
    pLineAttr(NULL),pFillAttr(NULL),pTextAttr(NULL),pPage(NULL),pModel(NULL),nLayer(0),
    bPenDirty(TRUE),bBrsDirty(TRUE),bFntDirty(TRUE),
    bLastObjWasPolyWithoutLine(FALSE),bNoLine(FALSE),bNoFill(FALSE),bLastObjWasLine(FALSE)
{
    aVD.EnableOutput(FALSE);
    pLineAttr=new SfxItemSet(rModel.GetItemPool(),XATTR_LINE_FIRST,XATTR_LINE_LAST);
    pFillAttr=new SfxItemSet(rModel.GetItemPool(),XATTR_FILL_FIRST,XATTR_FILL_LAST);
    pTextAttr=new SfxItemSet(rModel.GetItemPool(),EE_ITEMS_START,EE_ITEMS_END);
    pModel=&rModel;
}

ImpSdrGDIMetaFileImport::~ImpSdrGDIMetaFileImport()
{
    delete pLineAttr;
    delete pFillAttr;
    delete pTextAttr;
}

ULONG ImpSdrGDIMetaFileImport::DoImport(const GDIMetaFile& rMtf,
    SdrObjList& rOL,
    ULONG nInsPos,
    SvdProgressInfo *pProgrInfo)
{
    pPage=rOL.GetPage();
    GDIMetaFile* pTmpMtf=NULL;
    GDIMetaFile* pMtf = (GDIMetaFile*) &rMtf;
    ULONG nActionAnz=pMtf->GetActionCount();

    BOOL bError = FALSE;
    if(65000 < nActionAnz)
    {
        nActionAnz = 65000;
        bError = TRUE;
    }

    if(pProgrInfo)
        pProgrInfo->SetActionCount(nActionAnz);

    ULONG nActionsToReport = 0;

    for( MetaAction* pAct = pMtf->FirstAction(); pAct; pAct = pMtf->NextAction() )
    {
        switch (pAct->GetType())
        {
            case META_PIXEL_ACTION          : DoAction((MetaPixelAction          &)*pAct); break;
            case META_POINT_ACTION          : DoAction((MetaPointAction          &)*pAct); break;
            case META_LINE_ACTION           : DoAction((MetaLineAction           &)*pAct); break;
            case META_RECT_ACTION           : DoAction((MetaRectAction           &)*pAct); break;
            case META_ROUNDRECT_ACTION      : DoAction((MetaRoundRectAction      &)*pAct); break;
            case META_ELLIPSE_ACTION        : DoAction((MetaEllipseAction        &)*pAct); break;
            case META_ARC_ACTION            : DoAction((MetaArcAction            &)*pAct); break;
            case META_PIE_ACTION            : DoAction((MetaPieAction            &)*pAct); break;
            case META_CHORD_ACTION          : DoAction((MetaChordAction          &)*pAct); break;
            case META_POLYLINE_ACTION       : DoAction((MetaPolyLineAction       &)*pAct); break;
            case META_POLYGON_ACTION        : DoAction((MetaPolygonAction        &)*pAct); break;
            case META_POLYPOLYGON_ACTION    : DoAction((MetaPolyPolygonAction    &)*pAct); break;
            case META_TEXT_ACTION           : DoAction((MetaTextAction           &)*pAct); break;
            case META_TEXTARRAY_ACTION      : DoAction((MetaTextArrayAction      &)*pAct); break;
            case META_STRETCHTEXT_ACTION    : DoAction((MetaStretchTextAction    &)*pAct); break;
            case META_BMP_ACTION            : DoAction((MetaBmpAction            &)*pAct); break;
            case META_BMPSCALE_ACTION       : DoAction((MetaBmpScaleAction       &)*pAct); break;
            case META_BMPEX_ACTION          : DoAction((MetaBmpExAction          &)*pAct); break;
            case META_BMPEXSCALE_ACTION     : DoAction((MetaBmpExScaleAction     &)*pAct); break;
            case META_LINECOLOR_ACTION      : DoAction((MetaLineColorAction      &)*pAct); break;
            case META_FILLCOLOR_ACTION      : DoAction((MetaFillColorAction      &)*pAct); break;
            case META_TEXTCOLOR_ACTION      : DoAction((MetaTextColorAction      &)*pAct); break;
                case META_TEXTFILLCOLOR_ACTION  : DoAction((MetaTextFillColorAction  &)*pAct); break;
            case META_FONT_ACTION           : DoAction((MetaFontAction           &)*pAct); break;
            case META_MAPMODE_ACTION        : DoAction((MetaMapModeAction        &)*pAct); break;
            case META_CLIPREGION_ACTION     : DoAction((MetaClipRegionAction     &)*pAct); break;
            case META_MOVECLIPREGION_ACTION : DoAction((MetaMoveClipRegionAction &)*pAct); break;
            case META_ISECTRECTCLIPREGION_ACTION: DoAction((MetaISectRectClipRegionAction&)*pAct); break;
            case META_ISECTREGIONCLIPREGION_ACTION: DoAction((MetaISectRegionClipRegionAction&)*pAct); break;
            case META_RASTEROP_ACTION       : DoAction((MetaRasterOpAction       &)*pAct); break;
            case META_PUSH_ACTION           : DoAction((MetaPushAction           &)*pAct); break;
            case META_POP_ACTION            : DoAction((MetaPopAction            &)*pAct); break;
            case META_HATCH_ACTION          : DoAction((MetaHatchAction          &)*pAct); break;
            case META_COMMENT_ACTION        : DoAction((MetaCommentAction        &)*pAct, pMtf); break;

        }

        if(pProgrInfo != NULL)
        {
          nActionsToReport++;
          if(nActionsToReport >= 16) // Alle 16 Action updaten
          {
            if(!pProgrInfo->ReportActions(nActionsToReport))
              break;
            nActionsToReport = 0;
          }
        }
    }

    if(pProgrInfo != NULL)
    {
        pProgrInfo->ReportActions(nActionsToReport);
        nActionsToReport = 0;
    }

    // MapMode-Scaling  vornehmen
    MapScaling();
    // Objekte in vorgegebenes Rechteck hineinskalieren
    ULONG nAnz=aTmpList.GetObjCount();

    // Beim berechnen der Fortschrittsanzeige wird GetActionCount()*3 benutzt.
    // Da in aTmpList allerdings weniger eintraege als GetActionCount()
    // existieren koennen, muessen hier die zuviel vermuteten Actionen wieder
    // hinzugefuegt werden.
    nActionsToReport = (pMtf->GetActionCount() - nAnz)*2;

    if (!aScaleRect.IsEmpty()) {
        Point aOfs=aScaleRect.TopLeft();
        Size aMtfSize(pMtf->GetPrefSize());
        Fraction aScaleX(1,1);
        Fraction aScaleY(1,1);
        if (aMtfSize.Width()!=0) aScaleX=Fraction(aScaleRect.GetWidth()-1,aMtfSize.Width());
        if (aMtfSize.Height()!=0) aScaleY=Fraction(aScaleRect.GetHeight()-1,aMtfSize.Height());
        FASTBOOL bMov=aOfs.X()!=0 || aOfs.Y()!=0;
        FASTBOOL bSiz=aScaleX.GetNumerator()!=1 || aScaleX.GetDenominator()!=1 || aScaleY.GetNumerator()!=1 || aScaleY.GetDenominator()!=1;

        if (bMov || bSiz) {
            for (ULONG i=0; i<nAnz; i++)
            {
                SdrObject* pObj=aTmpList.GetObj(i);
                if (bSiz)
                {
                    pObj->NbcResize(Point(),aScaleX,aScaleY);
                }
                if (bMov)
                {
                    pObj->NbcMove(Size(aOfs.X(),aOfs.Y()));
                }

                if(pProgrInfo != NULL)
                {
                    nActionsToReport++;
                    if(nActionsToReport >= 32) // Alle 32 Action updaten
                    {
                        pProgrInfo->ReportRescales(nActionsToReport);
                        nActionsToReport = 0;
                    }
                }
            }
        }
    }

    // Alle noch nicht gemeldeten Rescales melden
    if(pProgrInfo)
    {
        pProgrInfo->ReportRescales(nActionsToReport);
        pProgrInfo->SetInsertCount(nAnz);
    }
    nActionsToReport = 0;

    // alle in aTmpList zwischengespeicherten Objekte nun in rOL ab der Position nInsPos einfuegen
    if (nInsPos>rOL.GetObjCount()) nInsPos=rOL.GetObjCount();
    SdrInsertReason aReason(SDRREASON_VIEWCALL);
    for (ULONG i=0; i<nAnz; i++)
    {
         SdrObject* pObj=aTmpList.GetObj(i);
         rOL.NbcInsertObject(pObj,nInsPos,&aReason);
         nInsPos++;

        if(pProgrInfo != NULL)
        {
            nActionsToReport++;
            if(nActionsToReport >= 32) // Alle 32 Action updaten
            {
                pProgrInfo->ReportInserts(nActionsToReport);
                nActionsToReport = 0;
            }
        }
    }
    if (pTmpMtf!=NULL) delete pTmpMtf;

    // ein letztesmal alle verbliebennen Inserts reporten
    if(pProgrInfo != NULL)
    {
        pProgrInfo->ReportInserts(nActionsToReport);
        if(bError)
            pProgrInfo->ReportError();
    }

    return aTmpList.GetObjCount();
}

void ImpSdrGDIMetaFileImport::SetAttributes(SdrObject* pObj, FASTBOOL bForceTextAttr)
{
    bNoLine=FALSE; bNoFill=FALSE;
    FASTBOOL bLine=TRUE && !bForceTextAttr;
    FASTBOOL bFill=pObj==NULL || pObj->IsClosedObj() && !bForceTextAttr;
    FASTBOOL bText=bForceTextAttr || (pObj!=NULL && pObj->GetOutlinerParaObject()!=NULL);

    if (bLine && bPenDirty)
    {
        if( aVD.IsLineColor() )
        {
            pLineAttr->Put(XLineStyleItem(XLINE_SOLID));
            pLineAttr->Put(XLineColorItem(String(), aVD.GetLineColor()));
        }
        else
            pLineAttr->Put(XLineStyleItem(XLINE_NONE));

        bPenDirty=FALSE;
    }
    else
        bNoLine=TRUE;

    if (bFill && bBrsDirty)
    {
        if( aVD.IsFillColor() )
        {
            pFillAttr->Put(XFillStyleItem(XFILL_SOLID));
            pFillAttr->Put(XFillColorItem(String(), aVD.GetFillColor()));
        }
        else
            pFillAttr->Put(XFillStyleItem(XFILL_NONE));

        bBrsDirty=FALSE;
    }
    else
        bNoFill=TRUE;

    if (bText && bFntDirty)
    {
        Font aFnt(aVD.GetFont());
        pTextAttr->Put(SvxFontItem(aFnt.GetFamily(),aFnt.GetName(),aFnt.GetStyleName(),aFnt.GetPitch(),aFnt.GetCharSet()));
        pTextAttr->Put(SvxPostureItem(aFnt.GetItalic()));
        pTextAttr->Put(SvxWeightItem(aFnt.GetWeight()));
        pTextAttr->Put(SvxFontHeightItem((ULONG)aFnt.GetSize().Height()));
        pTextAttr->Put(SvxFontWidthItem((USHORT)aFnt.GetSize().Width()));
        pTextAttr->Put(SvxUnderlineItem(aFnt.GetUnderline()));
        pTextAttr->Put(SvxCrossedOutItem(aFnt.GetStrikeout()));
        pTextAttr->Put(SvxShadowedItem(aFnt.IsShadow()));
        pTextAttr->Put(SvxAutoKernItem(aFnt.IsKerning()));
        pTextAttr->Put(SvxWordLineModeItem(aFnt.IsWordLineMode()));
        pTextAttr->Put(SvxContourItem(aFnt.IsOutline()));
        pTextAttr->Put(SvxColorItem(aFnt.GetColor()));
        //... svxfont textitem svditext
        bFntDirty=FALSE;
    }
    if (pObj!=NULL)
    {
        pObj->SetLayer(nLayer);
        if (bLine) pObj->NbcSetAttributes(*pLineAttr,FALSE);
        if (bFill) pObj->NbcSetAttributes(*pFillAttr,FALSE);
        if (bText) pObj->NbcSetAttributes(*pTextAttr,FALSE);
    }
}

void ImpSdrGDIMetaFileImport::InsertObj(SdrObject* pObj)
{
    aTmpList.InsertObject(pObj);
    if (HAS_BASE(SdrPathObj,pObj)) {
        FASTBOOL bClosed=pObj->IsClosedObj();
        bLastObjWasPolyWithoutLine=bNoLine && bClosed;
        bLastObjWasLine=!bClosed;
    } else {
        bLastObjWasPolyWithoutLine=FALSE;
        bLastObjWasLine=FALSE;
    }
}

/**************************************************************************************************/

void ImpSdrGDIMetaFileImport::DoAction(MetaPixelAction& rAct)
{
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPointAction& rAct)
{
}

void ImpSdrGDIMetaFileImport::DoAction(MetaLineAction& rAct)
{
    XPolygon aXP(2);
    aXP[0]=rAct.GetStartPoint();
    aXP[1]=rAct.GetEndPoint();
    if (!bLastObjWasLine || !CheckLastLineMerge(aXP)) {
        SdrPathObj* pPath=new SdrPathObj(OBJ_LINE,XPolyPolygon(aXP));
        SetAttributes(pPath);
        InsertObj(pPath);
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaRectAction& rAct)
{
    SdrRectObj* pRect=new SdrRectObj(rAct.GetRect());
    SetAttributes(pRect);
    InsertObj(pRect);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaRoundRectAction& rAct)
{
    SdrRectObj* pRect=new SdrRectObj(rAct.GetRect());
    SetAttributes(pRect);
    long nRad=(rAct.GetHorzRound()+rAct.GetVertRound())/2;
    if (nRad!=0) {
        SfxItemSet aSet(*pLineAttr->GetPool(),SDRATTR_ECKENRADIUS,SDRATTR_ECKENRADIUS);
        aSet.Put(SdrEckenradiusItem(nRad));
        pRect->NbcSetAttributes(aSet,FALSE);
    }
    InsertObj(pRect);
}

/**************************************************************************************************/

void ImpSdrGDIMetaFileImport::DoAction(MetaEllipseAction& rAct)
{
    SdrCircObj* pCirc=new SdrCircObj(OBJ_CIRC,rAct.GetRect());
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaArcAction& rAct)
{
    Point aCenter(rAct.GetRect().Center());
    long nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    long nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc=new SdrCircObj(OBJ_CARC,rAct.GetRect(),nStart,nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPieAction& rAct)
{
    Point aCenter(rAct.GetRect().Center());
    long nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    long nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc=new SdrCircObj(OBJ_SECT,rAct.GetRect(),nStart,nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaChordAction& rAct)
{
    Point aCenter(rAct.GetRect().Center());
    long nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    long nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc=new SdrCircObj(OBJ_CCUT,rAct.GetRect(),nStart,nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

/**************************************************************************************************/

FASTBOOL ImpSdrGDIMetaFileImport::CheckLastLineMerge(const XPolygon& rSrcPoly)
{
    if (bLastObjWasLine && !bPenDirty && rSrcPoly.GetPointCount()!=0) {
        SdrObject* pTmpObj=aTmpList.GetObj(aTmpList.GetObjCount()-1);
        SdrPathObj* pLastPoly=PTR_CAST(SdrPathObj,pTmpObj);
        if (pLastPoly!=NULL) {
            if (pLastPoly->GetPathPoly().Count()==1) {
                FASTBOOL bOk=FALSE;
                XPolygon aDstPoly(pLastPoly->GetPathPoly()[0]);
                USHORT nMaxDstPnt=aDstPoly.GetPointCount();
                if (nMaxDstPnt!=0) {
                    nMaxDstPnt--;
                    USHORT nMaxSrcPnt=USHORT(rSrcPoly.GetPointCount()-1);
                    if (aDstPoly[nMaxDstPnt]==rSrcPoly[0]) {
                        aDstPoly.Remove(nMaxDstPnt,1); // der ist sonst doppelt
                        aDstPoly.Insert(nMaxDstPnt,rSrcPoly);
                        bOk=TRUE;
                    } else if (aDstPoly[0]==rSrcPoly[nMaxSrcPnt]) {
                        aDstPoly.Remove(0,1); // der ist sonst doppelt
                        aDstPoly.Insert(0,rSrcPoly);
                        bOk=TRUE;
                    } else if (aDstPoly[0]==rSrcPoly[0]) {
                        for (USHORT i=1; i<=nMaxSrcPnt; i++) {
                            aDstPoly.Insert(0,rSrcPoly[i],rSrcPoly.GetFlags(i));
                        }
                        bOk=TRUE;
                    } else if (aDstPoly[nMaxDstPnt]==rSrcPoly[nMaxSrcPnt]) {
                        for (USHORT i=0; i<nMaxSrcPnt; i++) {
                            aDstPoly.Insert(nMaxDstPnt+1,rSrcPoly[i],rSrcPoly.GetFlags(i));
                        }
                        bOk=TRUE;
                    }
                }
                if (bOk) {
                    pLastPoly->NbcSetPathPoly(XPolyPolygon(aDstPoly));
                }
                return bOk;
            }
        }
    }
    return FALSE;
}

FASTBOOL ImpSdrGDIMetaFileImport::CheckLastPolyLineAndFillMerge(const XPolyPolygon& rPoly)
{
    if (bLastObjWasPolyWithoutLine) {
        SdrObject* pTmpObj=aTmpList.GetObj(aTmpList.GetObjCount()-1);
        SdrPathObj* pLastPoly=PTR_CAST(SdrPathObj,pTmpObj);
        if (pLastPoly!=NULL) {
            if (pLastPoly->GetPathPoly()==rPoly) {
                SetAttributes(NULL);
                if (!bNoLine && bNoFill) {
                    pLastPoly->NbcSetAttributes(*pLineAttr,FALSE);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPolyLineAction& rAct)
{
    XPolygon aXP(rAct.GetPolygon());
    if (!bLastObjWasLine || !CheckLastLineMerge(aXP)) {
        if (!bLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(XPolyPolygon(aXP))) {
            SdrPathObj* pPath=new SdrPathObj(OBJ_PLIN,XPolyPolygon(aXP));
            SetAttributes(pPath);
            InsertObj(pPath);
        }
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPolygonAction& rAct)
{
    XPolygon aXP(rAct.GetPolygon());
    if (aXP.GetPointCount()!=0) {
        Point aPt(aXP[0]);
        USHORT nPntAnz=aXP.GetPointCount();
        if (aPt==aXP[nPntAnz-1]) { // Polygon ggf. schliessen
            aXP.Insert(nPntAnz,aPt,XPOLY_NORMAL);
        }
        if (!bLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(XPolyPolygon(aXP))) {
            SdrPathObj* pPath=new SdrPathObj(OBJ_POLY,XPolyPolygon(aXP));
            SetAttributes(pPath);
            InsertObj(pPath);
        }
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPolyPolygonAction& rAct)
{
    XPolyPolygon aXPP(rAct.GetPolyPolygon());
    USHORT nPolyAnz=aXPP.Count();
    for (USHORT nPolyNum=nPolyAnz; nPolyNum>0;) {
        nPolyNum--;
        USHORT nPntAnz=aXPP[nPolyNum].GetPointCount();
        if (nPntAnz>0) {
            Point aPt(aXPP[nPolyNum][0]);
            if (aPt==aXPP[nPolyNum][nPntAnz-1]) { // Polygon ggf. schliessen
                aXPP[nPolyNum].Insert(nPntAnz,aPt,XPOLY_NORMAL);
            }
        } else {
            aXPP.Remove(nPolyNum); // leere Polys entfernen
        }
    }
    if (aXPP.Count()!=0) {
        if (!bLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(XPolyPolygon(aXPP))) {
            SdrPathObj* pPath=new SdrPathObj(OBJ_POLY,aXPP);
            SetAttributes(pPath);
            InsertObj(pPath);
        }
    }
}

/**************************************************************************************************/

void ImpSdrGDIMetaFileImport::ImportText(const Point& rPos, const XubString& rStr)
{
    // calc text box size, add 5% to make it fit safely
    Size aSize((aVD.GetTextWidth(rStr) * 105L) / 100L, aVD.GetTextHeight());
    FontMetric aFontMetric(aVD.GetFontMetric());
    Font aFnt(aVD.GetFont());
    FontAlign eAlg(aFnt.GetAlign());

    // calc rectangle for new object, rPos is on Baseline
    Rectangle aTextRect(rPos + Point(0, -aFontMetric.GetAscent()), aSize);

    // correct aTextRect when eAlg != ALIGN_BASELINE
    if(ALIGN_BASELINE != eAlg)
    {
        const OutputDevice* pOut = NULL;

        if(pModel)
            pOut = pModel->GetRefDevice();

        if(!pOut)
        {
            pOut = Application::GetDefaultDevice();
        }

        if(pOut)
        {
            FontMetric aMetr(pOut->GetFontMetric(aFnt));
            static BOOL bDoItAtAll = TRUE;

            if(eAlg == ALIGN_TOP)
            {
                if(bDoItAtAll)
                    aTextRect.Move(0, -(INT32)aMetr.GetDescent());
            }

            if(eAlg == ALIGN_BOTTOM)
            {
                if(bDoItAtAll)
                    aTextRect.Move(0, (INT32)aMetr.GetAscent());
            }
        }
    }

    SdrRectObj* pText=new SdrRectObj(OBJ_TEXT,aTextRect);
    pText->SetModel(pModel);
    pText->SetLayer(nLayer);
    pText->NbcSetText(rStr);
    SetAttributes(pText, TRUE);

    if (!aFnt.IsTransparent()) {
        SfxItemSet aAttr(*pFillAttr->GetPool(),XATTR_FILL_FIRST,XATTR_FILL_LAST);
        pFillAttr->Put(XFillStyleItem(XFILL_SOLID));
        pFillAttr->Put(XFillColorItem(String(), aFnt.GetFillColor()));
        pText->NbcSetAttributes(aAttr,FALSE);
    }
    ULONG nWink=aFnt.GetOrientation();
    if (nWink!=0) {
        nWink*=10;
        double a=nWink*nPi180;
        double nSin=sin(a);
        double nCos=cos(a);
        pText->NbcRotate(rPos,nWink,nSin,nCos);
    }
    InsertObj(pText);
    // Das fehlt noch:
    // FontAlign GetAlign() const { return pImpFont->eAlign; }
    // short     GetCharOrientation() const { return pImpFont->nCharOrientation; }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTextAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());
    ImportText(rAct.GetPoint(),aStr);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTextArrayAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());
    ImportText(rAct.GetPoint(),aStr);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaStretchTextAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());
    ImportText(rAct.GetPoint(),aStr);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpAction& rAct)
{
    Rectangle aRect(rAct.GetPoint(),rAct.GetBitmap().GetSizePixel());
    aRect.Right()++; aRect.Bottom()++;
    SdrGrafObj* pGraf=new SdrGrafObj(Graphic(rAct.GetBitmap()),aRect);
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpScaleAction& rAct)
{
    Rectangle aRect(rAct.GetPoint(),rAct.GetSize());
    aRect.Right()++; aRect.Bottom()++;
    SdrGrafObj* pGraf=new SdrGrafObj(Graphic(rAct.GetBitmap()),aRect);
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExAction& rAct)
{
    Rectangle aRect(rAct.GetPoint(),rAct.GetBitmapEx().GetSizePixel());
    aRect.Right()++; aRect.Bottom()++;
    SdrGrafObj* pGraf=new SdrGrafObj( rAct.GetBitmapEx(), aRect );
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExScaleAction& rAct)
{
    Rectangle aRect(rAct.GetPoint(),rAct.GetSize());
    aRect.Right()++; aRect.Bottom()++;
    SdrGrafObj* pGraf=new SdrGrafObj( rAct.GetBitmapEx(), aRect );
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction( MetaHatchAction& rAct )
{
    XPolyPolygon aXPP(rAct.GetPolyPolygon());
    USHORT nPolyAnz=aXPP.Count();
    for (USHORT nPolyNum=nPolyAnz; nPolyNum>0;)
    {
        nPolyNum--;
        USHORT nPntAnz=aXPP[nPolyNum].GetPointCount();
        if (nPntAnz>0)
        {
            Point aPt(aXPP[nPolyNum][0]);
            if (aPt==aXPP[nPolyNum][nPntAnz-1])
            { // Polygon ggf. schliessen
                aXPP[nPolyNum].Insert(nPntAnz,aPt,XPOLY_NORMAL);
            }
        }
        else
        {
            aXPP.Remove(nPolyNum); // leere Polys entfernen
        }
    }
    if( aXPP.Count()!=0 )
    {
        if (!bLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(XPolyPolygon(aXPP)))
        {
            const Hatch&    rHatch = rAct.GetHatch();
            SdrPathObj*     pPath=new SdrPathObj(OBJ_POLY,aXPP);
            SfxItemSet      aHatchAttr( pModel->GetItemPool(),
                                        XATTR_FILLSTYLE, XATTR_FILLSTYLE,
                                        XATTR_FILLHATCH, XATTR_FILLHATCH, 0 );
            XHatchStyle     eStyle;

            switch( rHatch.GetStyle() )
            {
                case( HATCH_TRIPLE ): eStyle = XHATCH_TRIPLE; break;
                case( HATCH_DOUBLE ): eStyle = XHATCH_DOUBLE; break;
                default: eStyle = XHATCH_SINGLE; break;
            }

            SetAttributes( pPath );
            aHatchAttr.Put( XFillStyleItem( XFILL_HATCH ) );
            aHatchAttr.Put( XFillHatchItem( String(), XHatch( rHatch.GetColor(), eStyle,
                                                              rHatch.GetDistance(), rHatch.GetAngle() ) ) );
            pPath->NbcSetAttributes(aHatchAttr,FALSE);
            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaLineColorAction& rAct)
{
    Color aOldLineColor( aVD.GetLineColor() );

    rAct.Execute(&aVD);

    if( aOldLineColor != aVD.GetLineColor() )
        bPenDirty = TRUE;
}

void ImpSdrGDIMetaFileImport::DoAction(MetaMapModeAction& rAct)
{
    MapScaling();
    rAct.Execute(&aVD);
    bLastObjWasPolyWithoutLine=FALSE;
    bLastObjWasLine=FALSE;
}

void ImpSdrGDIMetaFileImport::MapScaling()
{
    ULONG nAnz=aTmpList.GetObjCount();
    const MapMode& rMap=aVD.GetMapMode();
    Point aMapOrg(rMap.GetOrigin());
    Fraction aMapScaleX(rMap.GetScaleX());
    Fraction aMapScaleY(rMap.GetScaleX());
    FASTBOOL bMov=aMapOrg.X()!=0 || aMapOrg.Y()!=0;
    FASTBOOL bSiz=aMapScaleX.GetNumerator()!=1 || aMapScaleX.GetDenominator()!=1 || aMapScaleY.GetNumerator()!=1 || aMapScaleY.GetDenominator()!=1;
    if (bMov || bSiz) {
        for (ULONG i=nMapScalingOfs; i<nAnz; i++) {
            SdrObject* pObj=aTmpList.GetObj(i);
            if (bMov) {
                pObj->NbcMove(Size(aMapOrg.X(),aMapOrg.Y()));
            }
        }
    }
    nMapScalingOfs=nAnz;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction( MetaCommentAction& rAct, GDIMetaFile* pMtf )
{
    ByteString aSkipComment;

    if( rAct.GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
    {
        MetaGradientExAction* pAct = (MetaGradientExAction*) pMtf->NextAction();

        if( pAct && pAct->GetType() == META_GRADIENTEX_ACTION )
        {
            XPolyPolygon    aXPP(pAct->GetPolyPolygon());
            USHORT          nPolyAnz=aXPP.Count();

            for( USHORT nPolyNum= nPolyAnz; nPolyNum > 0; )
            {
                nPolyNum--;
                USHORT nPntAnz=aXPP[nPolyNum].GetPointCount();
                if (nPntAnz>0)
                {
                    Point aPt(aXPP[nPolyNum][0]);
                    if (aPt==aXPP[nPolyNum][nPntAnz-1])
                        aXPP[nPolyNum].Insert(nPntAnz,aPt,XPOLY_NORMAL);
                }
                else
                    aXPP.Remove(nPolyNum);
            }

            if( aXPP.Count()!=0 )
            {
                if (!bLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(XPolyPolygon(aXPP)))
                {
                    const Gradient& rGrad = pAct->GetGradient();
                    SdrPathObj*     pPath=new SdrPathObj(OBJ_POLY,aXPP);
                    SfxItemSet      aGradAttr( pModel->GetItemPool(),
                                               XATTR_FILLSTYLE, XATTR_FILLSTYLE,
                                               XATTR_FILLGRADIENT, XATTR_FILLGRADIENT, 0 );
                    XGradient       aXGradient;

                    aXGradient.SetGradientStyle((XGradientStyle)rGrad.GetStyle());
                    aXGradient.SetStartColor(rGrad.GetStartColor());
                    aXGradient.SetEndColor(rGrad.GetEndColor());
                    aXGradient.SetAngle((USHORT)rGrad.GetAngle());
                    aXGradient.SetBorder(rGrad.GetBorder());
                    aXGradient.SetXOffset(rGrad.GetOfsX());
                    aXGradient.SetYOffset(rGrad.GetOfsY());
                    aXGradient.SetStartIntens(rGrad.GetStartIntensity());
                    aXGradient.SetEndIntens(rGrad.GetEndIntensity());
                    aXGradient.SetSteps(rGrad.GetSteps());

                    SetAttributes( pPath );
                    aGradAttr.Put( XFillStyleItem( XFILL_GRADIENT ) );
                    aGradAttr.Put( XFillGradientItem( String(), aXGradient ) );
                    pPath->NbcSetAttributes(aGradAttr,FALSE);
                    InsertObj(pPath);
                }
            }

            aSkipComment = "XGRAD_SEQ_END";
        }
    }

    if(aSkipComment.Len())
    {
        MetaAction* pSkipAct = pMtf->NextAction();

        while( pSkipAct
            && ((pSkipAct->GetType() != META_COMMENT_ACTION )
                || (((MetaCommentAction*)pSkipAct)->GetComment().CompareIgnoreCaseToAscii(aSkipComment.GetBuffer()) != COMPARE_EQUAL)))
        {
            pSkipAct = pMtf->NextAction();
        }
    }
}

