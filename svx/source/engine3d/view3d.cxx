/*************************************************************************
 *
 *  $RCSfile: view3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
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

#define ITEMID_COLOR 0

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif

#ifndef _SVDOPATH_HXX
#include "svdopath.hxx"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDPOOL_HXX
#include "svdpool.hxx"
#endif

#ifndef _SVDORECT_HXX
#include "svdorect.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif

#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include "obj3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif

#ifndef _E3D_DLIGHT3D_HXX
#include "dlight3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _E3D_DRAGMT3D_HXX
#include "dragmt3d.hxx"
#endif

#ifndef _E3D_VIEW3D_HXX
#include "view3d.hxx"
#endif

#ifndef _SVDUNDO_HXX
#include "svdundo.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX
#include "xflclit.hxx"
#endif

#ifndef _SVX_XLNCLIT_HXX
#include "xlnclit.hxx"
#endif

#ifndef _SVDOGRAF_HXX
#include <svdograf.hxx>
#endif

#ifndef _SVX_XBTMPIT_HXX
#include <xbtmpit.hxx>
#endif

#ifndef _SVX_XFLBMTIT_HXX
#include <xflbmtit.hxx>
#endif

#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif

#include "xlnwtit.hxx"

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

TYPEINIT1(E3dView, SdrView);

long Scalar (Point aPoint1,
             Point aPoint2);

Point ScaleVector (Point  aPoint,
                   double nScale);

double NormVector (Point aPoint);

BOOL LineCutting (Point aP1,
                  Point aP2,
                  Point aP3,
                  Point aP4);

long Point2Line (Point aP1,
                 Point aP2,
                 Point aP3);

long DistPoint2Line (Point u,
                     Point v1,
                     Point v);

/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

E3dView::E3dView(SdrModel* pModel, OutputDevice* pOut) :
    SdrView(pModel, pOut)
{
    InitView ();
}

/*************************************************************************
|*
|* Konstruktor 2
|*
\************************************************************************/

E3dView::E3dView(SdrModel* pModel, ExtOutputDevice* pExtOut) :
    SdrView(pModel, pExtOut)
{
    InitView ();
}

/*************************************************************************
|*
|* Konstruktor 3
|*
\************************************************************************/

E3dView::E3dView(SdrModel* pModel) :
    SdrView(pModel)
{
    InitView ();
}

/*************************************************************************
|*
|* DrawMarkedObj ueberladen, da eventuell nur einzelne 3D-Objekte
|* gezeichnet werden sollen
|*
\************************************************************************/

void E3dView::DrawMarkedObj(OutputDevice& rOut, const Point& rOfs) const
{
    // Existieren 3D-Objekte, deren Szenen nicht selektiert sind?
    BOOL bSpecialHandling = FALSE;
    E3dScene *pScene = NULL;

    long nCnt = aMark.GetMarkCount();
    for(long nObjs = 0;nObjs < nCnt;nObjs++)
    {
        SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
        if(pObj && pObj->ISA(E3dCompoundObject))
        {
            // zugehoerige Szene
            pScene = ((E3dCompoundObject*)pObj)->GetScene();
            if(pScene && !IsObjMarked(pScene))
                bSpecialHandling = TRUE;
        }
        // Alle SelectionFlags zuruecksetzen
        if(pObj && pObj->ISA(E3dObject))
        {
            pScene = ((E3dObject*)pObj)->GetScene();
            if(pScene)
                pScene->SetSelected(FALSE);
        }
    }

    if(bSpecialHandling)
    {
        // SelectionFlag bei allen zu 3D Objekten gehoerigen
        // Szenen und deren Objekten auf nicht selektiert setzen
        long nObjs;
        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // zugehoerige Szene
                pScene = ((E3dCompoundObject*)pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(FALSE);
            }
        }

        // bei allen direkt selektierten Objekten auf selektiert setzen
        SdrMark* pM = NULL;

        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dObject))
            {
                // Objekt markieren
                E3dObject* p3DObj = (E3dObject*)pObj;
                p3DObj->SetSelected(TRUE);
                pScene = p3DObj->GetScene();
                pM = aMark.GetMark(nObjs);
            }
        }

        if(pScene)
        {
            // code from parent
            ((E3dView*)this)->aMark.ForceSort();
            pXOut->SetOutDev(&rOut);
            SdrPaintInfoRec aInfoRec;
            aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;

            Point aOfs(-rOfs.X(), -rOfs.Y());
            aOfs += pM->GetPageView()->GetOffset();

            if(aOfs != pXOut->GetOffset())
                pXOut->SetOffset(aOfs);

            pScene->SetDrawOnlySelected(TRUE);
            pScene->Paint(*pXOut,aInfoRec);
            pScene->SetDrawOnlySelected(FALSE);

            pXOut->SetOffset(Point(0,0));
        }

        // SelectionFlag zuruecksetzen
        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // zugehoerige Szene
                pScene = ((E3dCompoundObject*)pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(FALSE);
            }
        }
    }
    else
    {
        // call parent
        SdrExchangeView::DrawMarkedObj(rOut, rOfs);
    }
}

/*************************************************************************
|*
|* Model holen ueberladen, da bei einzelnen 3D Objekten noch eine Szene
|* untergeschoben werden muss
|*
\************************************************************************/

SdrModel* E3dView::GetMarkedObjModel() const
{
    // Existieren 3D-Objekte, deren Szenen nicht selektiert sind?
    BOOL bSpecialHandling = FALSE;
    E3dScene *pScene = NULL;

    long nCnt = aMark.GetMarkCount();
    for(long nObjs = 0;nObjs < nCnt;nObjs++)
    {
        SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
        if(pObj && pObj->ISA(E3dCompoundObject))
        {
            // zugehoerige Szene
            pScene = ((E3dCompoundObject*)pObj)->GetScene();
            if(pScene && !IsObjMarked(pScene))
                bSpecialHandling = TRUE;
        }
        // Alle SelectionFlags zuruecksetzen
        if(pObj && pObj->ISA(E3dObject))
        {
            pScene = ((E3dObject*)pObj)->GetScene();
            if(pScene)
                pScene->SetSelected(FALSE);
        }
    }

    SdrModel* pNewModel = 0L;
    if(bSpecialHandling)
    {
        // SelectionFlag bei allen zu 3D Objekten gehoerigen
        // Szenen und deren Objekten auf nicht selektiert setzen
        long nObjs;
        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // zugehoerige Szene
                pScene = ((E3dCompoundObject*)pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(FALSE);
            }
        }
        // bei allen direkt selektierten Objekten auf selektiert setzen
        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dObject))
            {
                // Objekt markieren
                E3dObject* p3DObj = (E3dObject*)pObj;
                p3DObj->SetSelected(TRUE);
            }
        }

        // Neue MarkList generieren, die die betroffenen
        // Szenen als markierte Objekte enthaelt
        SdrMarkList aOldML(aMark); // alte Marklist merken
        SdrMarkList aNewML; // neue leere Marklist
        ((E3dView*)this)->aMark = aNewML;

        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aOldML.GetMark(nObjs)->GetObj();
            if(pObj)
            {
                if(pObj->ISA(E3dCompoundObject))
                {
                    // zugehoerige Szene holen
                    pScene = ((E3dCompoundObject*)pObj)->GetScene();
                    if(pScene)
                        pObj = pScene;
                }

                // Keine Objekte doppelt markieren
                // (dies koennten nur Szenen sein)
                if(!IsObjMarked(pObj))
                {
                    USHORT nAnz=GetPageViewCount();
                    for (USHORT nv=0; nv<nAnz; nv++) {
                        SdrPageView* pPV=GetPageViewPvNum(nv);
                        ((E3dView*)this)->MarkObj(pObj,pPV,FALSE,TRUE);
                    }
                }
            }
        }

        // call parent
        pNewModel = SdrView::GetMarkedObjModel();

        // Alle Szenen im kopierten Model in Ihren Ausdehnungen Korrigieren
        // und IsSelected zuruecksetzen
        if(pNewModel)
        {
            for(UINT16 nPg=0; nPg < pNewModel->GetPageCount(); nPg++)
            {
                const SdrPage* pSrcPg=pNewModel->GetPage(nPg);
                UINT32 nObAnz=pSrcPg->GetObjCount();

                // Unterobjekte von Szenen einfuegen
                for(UINT32 nOb=0; nOb<nObAnz; nOb++)
                {
                    const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);
                    if(pSrcOb->ISA(E3dScene))
                    {
                        pScene = (E3dScene*)pSrcOb;
                        pScene->CorrectSceneDimensions();
                        pScene->SetSelected(FALSE);
                    }
                }
            }
        }

        // Alte Liste wieder setzen
        ((E3dView*)this)->aMark = aOldML;

        // SelectionFlag zuruecksetzen
        for(nObjs = 0;nObjs < nCnt;nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // zugehoerige Szene
                pScene = ((E3dCompoundObject*)pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(FALSE);
            }
        }
    }
    else
    {
        // call parent
        pNewModel = SdrView::GetMarkedObjModel();
    }

    // model zurueckgeben
    return pNewModel;
}

/*************************************************************************
|*
|* Bei Paste muss - falls in eine Scene eingefuegt wird - die
|* Objekte der Szene eingefuegt werden, die Szene selbst aber nicht
|*
\************************************************************************/

BOOL E3dView::Paste(const SdrModel& rMod, const Point& rPos,
    SdrObjList* pLst, UINT32 nOptions)
{
    BOOL bRetval = FALSE;

    // Liste holen
    Point aPos(rPos);
    SdrObjList* pDestList = pLst;
    ImpGetPasteObjList(aPos,pDestList);
    if(pDestList==NULL)
        return FALSE;

    // Owner der Liste holen
    SdrObject* pOwner = pDestList->GetOwnerObj();
    if(pOwner && pOwner->ISA(E3dScene))
    {
        E3dScene* pDestScene = (E3dScene*)pOwner;
        B3dCamera& rCameraSetDst = pDestScene->GetCameraSet();
        BOOL bDstInserted = FALSE;
        BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXCHANGE_PASTE));

        // Alle Objekte aus E3dScenes kopieren und direkt einfuegen
        for(UINT16 nPg=0; nPg < rMod.GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=rMod.GetPage(nPg);
            UINT32 nObAnz=pSrcPg->GetObjCount();

            // Unterobjekte von Szenen einfuegen
            for(UINT32 nOb=0; nOb<nObAnz; nOb++)
            {
                const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);
                if(pSrcOb->ISA(E3dScene))
                {
                    E3dScene* pSrcScene = (E3dScene*)pSrcOb;
                    B3dCamera& rCameraSetSrc = pSrcScene->GetCameraSet();

                    for(UINT32 i = 0;i < pSrcScene->GetSubList()->GetObjCount(); i++)
                    {
                        SdrObject* pObj = pSrcScene->GetSubList()->GetObj(i);
                        if(pObj && pObj->ISA(E3dCompoundObject))
                        {
                            // Kopieren
                            E3dObject* pNew = (E3dObject*)pObj->Clone(
                                pDestScene->GetPage(), pDestScene->GetModel());
                            if(pNew)
                            {
                                // Neues Objekt in Szene einfuegen
                                pNew->NbcSetLayer(pObj->GetLayer());
                                pNew->NbcSetStyleSheet(pObj->GetStyleSheet(), TRUE);
                                pDestScene->Insert3DObj(pNew);
                                bDstInserted = TRUE;

                                // Transformation ObjectToEye Src
                                Matrix4D aMatSrc;
                                aMatSrc = ((E3dCompoundObject*)pObj)->GetFullTransform();
                                aMatSrc *= rCameraSetSrc.GetOrientation();

                                // Tanslation und scale von source
                                B3dVolume aDevVolSrc = rCameraSetSrc.GetDeviceVolume();

                                // auf Augkoordinaten umstellen
                                double fTemp = aDevVolSrc.MinVec().Z();
                                aDevVolSrc.MinVec().Z() = -aDevVolSrc.MaxVec().Z();
                                aDevVolSrc.MaxVec().Z() = -fTemp;

                                Vector3D aProjScaleSrc(
                                    2.0 / aDevVolSrc.GetWidth(),
                                    2.0 / aDevVolSrc.GetHeight(),
                                    2.0 / aDevVolSrc.GetDepth());
                                Vector3D aProjTransSrc(
                                    -1.0 * ((aDevVolSrc.MaxVec().X() + aDevVolSrc.MinVec().X()) / aDevVolSrc.GetWidth()),
                                    -1.0 * ((aDevVolSrc.MaxVec().Y() + aDevVolSrc.MinVec().Y()) / aDevVolSrc.GetHeight()),
                                    -1.0 * ((aDevVolSrc.MaxVec().Z() + aDevVolSrc.MinVec().Z()) / aDevVolSrc.GetDepth()));
                                Vector3D aViewScaleSrc(rCameraSetSrc.GetScale());
                                aViewScaleSrc.Z() = 1.0;

                                // Tanslation und scale von dest
                                B3dVolume aDevVolDst = rCameraSetDst.GetDeviceVolume();

                                // auf Augkoordinaten umstellen
                                fTemp = aDevVolDst.MinVec().Z();
                                aDevVolDst.MinVec().Z() = -aDevVolDst.MaxVec().Z();
                                aDevVolDst.MaxVec().Z() = -fTemp;

                                Vector3D aProjScaleDst(
                                    2.0 / aDevVolDst.GetWidth(),
                                    2.0 / aDevVolDst.GetHeight(),
                                    2.0 / aDevVolDst.GetDepth());
                                Vector3D aProjTransDst(
                                    -1.0 * ((aDevVolDst.MaxVec().X() + aDevVolDst.MinVec().X()) / aDevVolDst.GetWidth()),
                                    -1.0 * ((aDevVolDst.MaxVec().Y() + aDevVolDst.MinVec().Y()) / aDevVolDst.GetHeight()),
                                    -1.0 * ((aDevVolDst.MaxVec().Z() + aDevVolDst.MinVec().Z()) / aDevVolDst.GetDepth()));
                                Vector3D aViewScaleDst(rCameraSetDst.GetScale());
                                aViewScaleDst.Z() = 1.0;

                                // Groesse des Objektes in Augkoordinaten Src
                                Volume3D aObjVolSrc;
                                aObjVolSrc.Union(((E3dCompoundObject*)pObj)->GetBoundVolume().GetTransformVolume(aMatSrc));

                                // Vorlaeufige Groesse in Augkoordinaten Dst
                                Matrix4D aMatZwi = aMatSrc;
                                aMatZwi.Scale(aProjScaleSrc);
                                aMatZwi.Translate(aProjTransSrc);
                                aMatZwi.Scale(aViewScaleSrc);

                                Matrix4D aMatDst;
                                aMatDst.Scale(aProjScaleDst);
                                aMatDst.Translate(aProjTransDst);
                                aMatDst.Scale(aViewScaleDst);
                                aMatDst.Invert();

                                aMatZwi *= aMatDst;

                                Volume3D aObjVolDst;
                                aObjVolDst.Union(((E3dCompoundObject*)pObj)->GetBoundVolume().GetTransformVolume(aMatZwi));

                                // Beide verhaeltnistiefen berechnen und mitteln
                                double fDepthOne = (aObjVolSrc.GetDepth() * aObjVolDst.GetWidth()) / aObjVolSrc.GetWidth();
                                double fDepthTwo = (aObjVolSrc.GetDepth() * aObjVolDst.GetHeight()) / aObjVolSrc.GetHeight();
                                double fWantedDepth = (fDepthOne + fDepthTwo) / 2.0;

                                // Faktor zum Tiefe anpassen bilden
                                double fFactor = fWantedDepth / aObjVolDst.GetDepth();
                                Vector3D aDepthScale(1.0, 1.0, fFactor);

                                // Endgueltige Transformation bilden
                                aMatSrc.Scale(aProjScaleSrc);
                                aMatSrc.Translate(aProjTransSrc);
                                aMatSrc.Scale(aViewScaleSrc);
                                aMatSrc.Scale(aDepthScale);

                                aMatDst = pDestScene->GetFullTransform();
                                aMatDst *= rCameraSetDst.GetOrientation();
                                aMatDst.Scale(aProjScaleDst);
                                aMatDst.Translate(aProjTransDst);
                                aMatDst.Scale(aViewScaleDst);
                                aMatDst.Invert();

                                aMatSrc *= aMatDst;

                                // Neue Objekttransformation setzen
                                pNew->NbcSetTransform(aMatSrc);

                                // Undo anlegen
                                AddUndo(new SdrUndoNewObj(*pNew));
                            }
                        }
                    }
                }
            }
        }
        EndUndo();

        // DestScene anpassen
        if(bDstInserted)
        {
            pDestScene->SetRectsDirty();
            pDestScene->CorrectSceneDimensions();
        }
    }
    else
    {
        // call parent
        bRetval = SdrView::Paste(rMod, rPos, pLst, nOptions);
    }

    // und Rueckgabewert liefern
    return bRetval;
}

/*************************************************************************
|*
|* 3D-Konvertierung moeglich?
|*
\************************************************************************/

BOOL E3dView::IsConvertTo3DObjPossible() const
{
    BOOL bAny3D(FALSE);
    BOOL bGroupSelected(FALSE);
    BOOL bRetval(TRUE);

    for(INT32 a=0;!bAny3D && a<aMark.GetMarkCount();a++)
    {
        SdrObject *pObj = aMark.GetMark(a)->GetObj();
        if(pObj)
        {
            ImpIsConvertTo3DPossible(pObj, bAny3D, bGroupSelected);
        }
    }

    bRetval = !bAny3D
        && (
           IsConvertToPolyObjPossible(FALSE)
        || IsConvertToPathObjPossible(FALSE)
        || IsImportMtfPossible());
    return bRetval;
}

void E3dView::ImpIsConvertTo3DPossible(SdrObject* pObj, BOOL& rAny3D,
    BOOL& rGroupSelected) const
{
    if(pObj)
    {
        if(pObj->ISA(E3dObject))
        {
            rAny3D = TRUE;
        }
        else
        {
            if(pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj, IM_DEEPNOGROUPS);
                while(aIter.IsMore())
                {
                    SdrObject* pNewObj = aIter.Next();
                    ImpIsConvertTo3DPossible(pNewObj, rAny3D, rGroupSelected);
                }
                rGroupSelected = TRUE;
            }
        }
    }
}

/*************************************************************************
|*
|* 3D-Konvertierung zu Extrude ausfuehren
|*
\************************************************************************/

void E3dView::ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj)
{
    SfxItemPool& rPool = pMod->GetItemPool();
    SfxItemSet aSet(rPool);
    pObj->TakeAttributes(aSet, TRUE, FALSE);
    BOOL bChange(FALSE);

    if(pObj->ISA(SdrTextObj))
    {
        const SvxColorItem& rTextColorItem = (const SvxColorItem&)aSet.Get(rPool.GetWhich(SID_ATTR_CHAR_COLOR));
        if(rTextColorItem.GetValue() == RGB_Color(COL_BLACK))
        {
            // Bei schwarzen Textobjekten wird die Farbe auf grau gesetzt
            SvxColorItem aNewTextColorItem(RGB_Color(COL_GRAY), rPool.GetWhich(SID_ATTR_CHAR_COLOR));
            aSet.Put(aNewTextColorItem);
            bChange = TRUE;
        }
    }

    if(bChange)
    {
        // UNDO fuer Textfarbe Aenderung (nur bei nicht
        // temporaeren Objekten)
        if(pObj->GetPage())
            AddUndo(new SdrUndoAttrObj(*pObj, FALSE, FALSE));
        pObj->NbcSetAttributes(aSet, FALSE);
    }
}

void E3dView::ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj)
{
    SfxItemPool& rPool = pMod->GetItemPool();
    SfxItemSet aSet(rPool);
    pObj->TakeAttributes(aSet, TRUE, FALSE);
    BOOL bChange(FALSE);

    if(pObj->ISA(SdrPathObj))
    {
        INT32 nLineWidth = ((const XLineWidthItem&)(aSet.Get(XATTR_LINEWIDTH))).GetValue();
        XLineStyle eLineStyle = (XLineStyle)((const XLineStyleItem&)aSet.Get(XATTR_LINESTYLE)).GetValue();
        XFillStyle eFillStyle = ITEMVALUE(aSet, XATTR_FILLSTYLE, XFillStyleItem);

        if(((SdrPathObj*)pObj)->IsClosed() && eLineStyle == XLINE_SOLID && !nLineWidth && eFillStyle != XFILL_NONE)
        {
            aSet.Put(XLineStyleItem(XLINE_NONE));
            aSet.Put(XLineWidthItem(0L));
            bChange = TRUE;
        }
    }

    if(bChange)
    {
        // UNDO fuer Textfarbe Aenderung (nur bei nicht
        // temporaeren Objekten)
        if(pObj->GetPage())
            AddUndo(new SdrUndoAttrObj(*pObj, FALSE, FALSE));
        pObj->NbcSetAttributes(aSet, FALSE);
    }
}

void E3dView::ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, BOOL bExtrude, double fDepth, Matrix4D& rLatheMat)
{
    // Einzelnes PathObject, dieses umwanden
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);

    if(pPath)
    {
        E3dDefaultAttributes aDefault = Get3DDefaultAttributes();
        if(bExtrude)
            aDefault.SetDefaultExtrudeCharacterMode(TRUE);
        else
            aDefault.SetDefaultLatheCharacterMode(TRUE);

        // ItemSet des Ursprungsobjektes holen
        SfxItemPool& rPool = pMod->GetItemPool();
        SfxItemSet aSet(rPool);
        pObj->TakeAttributes(aSet, FALSE, TRUE);
        XFillStyle eFillStyle = ITEMVALUE(aSet, XATTR_FILLSTYLE, XFillStyleItem);

        // Linienstil ausschalten
        aSet.Put(XLineStyleItem(XLINE_NONE));

        // Feststellen, ob ein FILL_Attribut gesetzt ist.
        if(!pPath->IsClosed() || eFillStyle == XFILL_NONE)
        {
            // Das SdrPathObj ist nicht gefuellt, lasse die
            // vordere und hintere Flaeche weg. Ausserdem ist
            // eine beidseitige Darstellung notwendig.
            aDefault.SetDefaultExtrudeCloseFront(FALSE);
            aDefault.SetDefaultExtrudeCloseBack(FALSE);
            aDefault.SetDefaultDoubleSided(TRUE);

            // Fuellattribut setzen
            aSet.Put(XFillStyleItem(XFILL_SOLID));

            // Fuellfarbe muss auf Linienfarbe, da das Objekt vorher
            // nur eine Linie war
            Color aColorLine = ((const XLineColorItem&)(aSet.Get(XATTR_LINECOLOR))).GetValue();
            aSet.Put(XFillColorItem(String(), aColorLine));
        }

        // Neues Extrude-Objekt erzeugen
        E3dObject* p3DObj = NULL;
        if(bExtrude)
        {
            p3DObj = new E3dExtrudeObj(aDefault, pPath->GetPathPoly(), fDepth);
        }
        else
        {
            PolyPolygon3D aPolyPoly3D(pPath->GetPathPoly(), aDefault.GetDefaultLatheScale());
            aPolyPoly3D.Transform(rLatheMat);
            p3DObj = new E3dLatheObj(aDefault, aPolyPoly3D);
        }

        // Attribute setzen
        if(p3DObj)
        {
            p3DObj->NbcSetLayer(pObj->GetLayer());
            p3DObj->NbcSetAttributes(aSet, FALSE);
            p3DObj->NbcSetStyleSheet(pObj->GetStyleSheet(), TRUE);

            // Neues 3D-Objekt einfuegen
            pScene->Insert3DObj(p3DObj);
        }
    }
}

void E3dView::ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, BOOL bExtrude, double fDepth, Matrix4D& rLatheMat)
{
    if(pObj)
    {
        // change text color attribute for not so dark colors
        if(pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj, IM_DEEPWITHGROUPS);
            while(aIter.IsMore())
            {
                SdrObject* pGroupMember = aIter.Next();
                ImpChangeSomeAttributesFor3DConversion(pGroupMember);
            }
        }
        else
            ImpChangeSomeAttributesFor3DConversion(pObj);

        // convert completely to path objects
        SdrObject* pNewObj1 = pObj->ConvertToPolyObj(FALSE, FALSE);

        // change text color attribute for not so dark colors
        if(pNewObj1->IsGroupObject())
        {
            SdrObjListIter aIter(*pNewObj1, IM_DEEPWITHGROUPS);
            while(aIter.IsMore())
            {
                SdrObject* pGroupMember = aIter.Next();
                ImpChangeSomeAttributesFor3DConversion2(pGroupMember);
            }
        }
        else
            ImpChangeSomeAttributesFor3DConversion2(pNewObj1);

        // convert completely to path objects
        SdrObject* pNewObj2 = pObj->ConvertToContourObj(pNewObj1, TRUE);

        // add all to flat scene
        if(pNewObj2->IsGroupObject())
        {
            SdrObjListIter aIter(*pNewObj2, IM_DEEPWITHGROUPS);
            while(aIter.IsMore())
            {
                SdrObject* pGroupMember = aIter.Next();
                ImpCreateSingle3DObjectFlat(pScene, pGroupMember, bExtrude, fDepth, rLatheMat);
            }
        }
        else
            ImpCreateSingle3DObjectFlat(pScene, pNewObj2, bExtrude, fDepth, rLatheMat);

        // delete zwi object
        if(pNewObj2 != pObj && pNewObj2 != pNewObj1)
            delete pNewObj2;

        if(pNewObj1 != pObj)
            delete pNewObj1;
    }
}

/*************************************************************************
|*
|* 3D-Konvertierung zu Extrude steuern
|*
\************************************************************************/

void E3dView::ConvertMarkedObjTo3D(BOOL bExtrude, Vector3D aPnt1, Vector3D aPnt2)
{
    if(HasMarkedObj())
    {
        // Undo anlegen
        if(bExtrude)
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXTRUDE));
        else
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_LATHE));

        // Neue Szene fuer zu erzeugende 3D-Objekte anlegen
        E3dScene* pScene = new E3dPolyScene(Get3DDefaultAttributes());

        // Rechteck bestimmen und evtl. korrigieren
        Rectangle aRect = GetAllMarkedRect();
        if(aRect.GetWidth() <= 1)
            aRect.SetSize(Size(500, aRect.GetHeight()));
        if(aRect.GetHeight() <= 1)
            aRect.SetSize(Size(aRect.GetWidth(), 500));

        // Tiefe relativ zur Groesse der Selektion bestimmen
        double fDepth = 0.0;
        double fRot3D = 0.0;
        Matrix4D aLatheMat;

        if(bExtrude)
        {
            double fW = (double)aRect.GetWidth();
            double fH = (double)aRect.GetHeight();
            fDepth = sqrt(fW*fW + fH*fH) / 6.0;
        }
        if(!bExtrude)
        {
            // Transformation fuer Polygone Rotationskoerper erstellen
            if(aPnt1 != aPnt2)
            {
                // Rotation um Kontrollpunkt1 mit eigestelltem Winkel
                // fuer 3D Koordinaten
                Vector3D aDiff = aPnt1 - aPnt2;
                fRot3D = atan2(aDiff.Y(), aDiff.X()) - F_PI2;

                if(fabs(fRot3D) < SMALL_DVALUE)
                    fRot3D = 0.0;

                if(fRot3D != 0.0)
                {
                    aLatheMat.Translate(-aPnt2);
                    aLatheMat.RotateZ(-fRot3D);
                    aLatheMat.Translate(aPnt2);
                }
            }

            if(aPnt2.X() != 0.0)
            {
                // Translation auf Y=0 - Achse
                aLatheMat.TranslateX(-aPnt2.X());
            }
            else
            {
                aLatheMat.Translate((double)-aRect.Left());
            }

            // Inverse Matrix bilden, um die Zielausdehnung zu bestimmen
            Matrix4D aInvLatheMat = aLatheMat;
            aInvLatheMat.Invert();

            // SnapRect Ausdehnung mittels Spiegelung an der Rotationsachse
            // erweitern
            for(UINT32 a=0;a<aMark.GetMarkCount();a++)
            {
                SdrMark* pMark = aMark.GetMark(a);
                SdrObject* pObj = pMark->GetObj();
                Rectangle aTurnRect = pObj->GetSnapRect();
                Vector3D aRot;
                Point aRotPnt;

                aRot = Vector3D(aTurnRect.Left(), -aTurnRect.Top(), 0.0);
                aRot *= aLatheMat;
                aRot.X() = -aRot.X();
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.X() + 0.5), (long)(-aRot.Y() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = Vector3D(aTurnRect.Left(), -aTurnRect.Bottom(), 0.0);
                aRot *= aLatheMat;
                aRot.X() = -aRot.X();
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.X() + 0.5), (long)(-aRot.Y() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = Vector3D(aTurnRect.Right(), -aTurnRect.Top(), 0.0);
                aRot *= aLatheMat;
                aRot.X() = -aRot.X();
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.X() + 0.5), (long)(-aRot.Y() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = Vector3D(aTurnRect.Right(), -aTurnRect.Bottom(), 0.0);
                aRot *= aLatheMat;
                aRot.X() = -aRot.X();
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.X() + 0.5), (long)(-aRot.Y() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));
            }
        }

        // Ueber die Selektion gehen und in 3D wandeln, komplett mit
        // Umwandeln in SdrPathObject, auch Schriften
        for(UINT32 a=0;a<aMark.GetMarkCount();a++)
        {
            SdrMark* pMark = aMark.GetMark(a);
            SdrObject* pObj = pMark->GetObj();

            ImpCreate3DObject(pScene, pObj, bExtrude, fDepth, aLatheMat);
        }

        if(pScene->GetSubList() && pScene->GetSubList()->GetObjCount() != 0)
        {
            // Alle angelegten Objekte Tiefenarrangieren
            if(bExtrude)
                DoDepthArrange(pScene, fDepth);

            // 3D-Objekte auf die Mitte des Gesamtrechtecks zentrieren
            Vector3D aCenter = pScene->GetCenter();
            Matrix4D aMatrix;

            aMatrix.Translate(-aCenter);
            pScene->ApplyTransform(aMatrix);

            // Szene initialisieren
            pScene->NbcSetSnapRect(aRect);
            Volume3D aBoundVol = pScene->GetBoundVolume();
            InitScene(pScene, (double)aRect.GetWidth(),
                (double)aRect.GetHeight(), aBoundVol.GetDepth());

            // Transformationen initialisieren, damit bei RecalcSnapRect()
            // richtig gerechnet wird
            pScene->InitTransformationSet();

            // Szene anstelle des ersten selektierten Objektes einfuegen
            // und alle alten Objekte weghauen
            SdrObject* pRepObj = aMark.GetMark(0)->GetObj();
            SdrPageView* pPV = aMark.GetMark(0)->GetPageView();
            MarkObj(pRepObj, pPV, TRUE);
            ReplaceObject(pRepObj, *pPV, pScene, FALSE);
            DeleteMarked();
            MarkObj(pScene, pPV);

            // Rotationskoerper um Rotationsachse drehen
            if(!bExtrude && fRot3D != 0.0)
            {
                pScene->RotateZ(fRot3D);
            }

            // Default-Rotation setzen
            double XRotateDefault = 20;
            pScene->RotateX(DEG2RAD(XRotateDefault));
            pScene->SetSortingMode(E3D_SORT_FAST_SORTING|E3D_SORT_IN_PARENTS|E3D_SORT_TEST_LENGTH);

            // SnapRects der Objekte ungueltig
            pScene->CorrectSceneDimensions();
            pScene->SetSnapRect(aRect);
        }
        else
        {
            // Es wurden keine 3D Objekte erzeugt, schmeiss alles weg
            delete pScene;
        }

        // Undo abschliessen
        EndUndo();
    }
}

/*************************************************************************
|*
|* Alle enthaltenen Extrude-Objekte Tiefenarrangieren
|*
\************************************************************************/

struct E3dDepthNeighbour
{
    E3dDepthNeighbour*  pNext;
    E3dExtrudeObj*      pObj;

    E3dDepthNeighbour() { pNext = NULL; pObj = NULL; }
};

struct E3dDepthLayer
{
    E3dDepthLayer*      pDown;
    E3dDepthNeighbour*  pNext;

    E3dDepthLayer() { pDown = NULL; pNext = NULL; }
    ~E3dDepthLayer() { while(pNext) { E3dDepthNeighbour* pSucc = pNext->pNext; delete pNext; pNext = pSucc; }}
};

void E3dView::DoDepthArrange(E3dScene* pScene, double fDepth)
{
    if(pScene && pScene->GetSubList() && pScene->GetSubList()->GetObjCount() > 1)
    {
        SdrObjList* pSubList = pScene->GetSubList();
        SdrObjListIter aIter(*pSubList, IM_FLAT);
        E3dDepthLayer* pBaseLayer = NULL;
        E3dDepthLayer* pLayer = NULL;
        INT32 nNumLayers = 0;
        SfxItemPool& rPool = pMod->GetItemPool();

        while(aIter.IsMore())
        {
            E3dObject* pSubObj = (E3dObject*)aIter.Next();

            if(pSubObj && pSubObj->ISA(E3dExtrudeObj))
            {
                E3dExtrudeObj* pExtrudeObj = (E3dExtrudeObj*)pSubObj;
                const PolyPolygon3D& rExtrudePoly = pExtrudeObj->GetExtrudePolygon();

                SfxItemSet aLocalSet(rPool);
                pExtrudeObj->TakeAttributes(aLocalSet, FALSE, TRUE);
                XFillStyle eLocalFillStyle = ITEMVALUE(aLocalSet, XATTR_FILLSTYLE, XFillStyleItem);
                Color aLocalColor = ((const XFillColorItem&)(aLocalSet.Get(XATTR_FILLCOLOR))).GetValue();

                // ExtrudeObj einordnen
                if(pLayer)
                {
                    // Gibt es eine Ueberschneidung mit einem Objekt dieses
                    // Layers?
                    BOOL bOverlap(FALSE);
                    E3dDepthNeighbour* pAct = pLayer->pNext;

                    while(!bOverlap && pAct)
                    {
                        // ueberlappen sich pAct->pObj und pExtrudeObj ?
                        const PolyPolygon3D& rActPoly = pAct->pObj->GetExtrudePolygon();
                        bOverlap = rExtrudePoly.DoesOverlap(rActPoly, DEGREE_FLAG_X|DEGREE_FLAG_Y);

                        if(bOverlap)
                        {
                            // second ciriteria: is another fillstyle or color used?
                            SfxItemSet aCompareSet(rPool);
                            pAct->pObj->TakeAttributes(aCompareSet, FALSE, TRUE);

                            XFillStyle eCompareFillStyle = ITEMVALUE(aCompareSet, XATTR_FILLSTYLE, XFillStyleItem);

                            if(eLocalFillStyle == eCompareFillStyle)
                            {
                                if(eLocalFillStyle == XFILL_SOLID)
                                {
                                    Color aCompareColor = ((const XFillColorItem&)(aCompareSet.Get(XATTR_FILLCOLOR))).GetValue();

                                    if(aCompareColor == aLocalColor)
                                    {
                                        bOverlap = FALSE;
                                    }
                                }
                                else if(eLocalFillStyle == XFILL_NONE)
                                {
                                    bOverlap = FALSE;
                                }
                            }
                        }

                        pAct = pAct->pNext;
                    }

                    if(bOverlap)
                    {
                        // ja, beginne einen neuen Layer
                        pLayer->pDown = new E3dDepthLayer;
                        pLayer = pLayer->pDown;
                        nNumLayers++;
                        pLayer->pNext = new E3dDepthNeighbour;
                        pLayer->pNext->pObj = pExtrudeObj;
                    }
                    else
                    {
                        // nein, Objekt kann in aktuellen Layer
                        E3dDepthNeighbour* pNewNext = new E3dDepthNeighbour;
                        pNewNext->pObj = pExtrudeObj;
                        pNewNext->pNext = pLayer->pNext;
                        pLayer->pNext = pNewNext;
                    }
                }
                else
                {
                    // erster Layer ueberhaupt
                    pBaseLayer = new E3dDepthLayer;
                    pLayer = pBaseLayer;
                    nNumLayers++;
                    pLayer->pNext = new E3dDepthNeighbour;
                    pLayer->pNext->pObj = pExtrudeObj;
                }
            }
        }

        // Anzahl Layer steht fest
        if(nNumLayers > 1)
        {
            // Arrangement ist notwendig
            double fMinDepth = fDepth * 0.8;
            double fStep = (fDepth - fMinDepth) / (double)nNumLayers;
            pLayer = pBaseLayer;

            while(pLayer)
            {
                // an pLayer entlangspazieren
                E3dDepthNeighbour* pAct = pLayer->pNext;

                while(pAct)
                {
                    // Anpassen
                    pAct->pObj->SetExtrudeDepth(fMinDepth);

                    // Naechster Eintrag
                    pAct = pAct->pNext;
                }

                // naechster Layer
                pLayer = pLayer->pDown;
                fMinDepth += fStep;
            }
        }

        // angelegte Strukturen aufraeumen
        while(pBaseLayer)
        {
            pLayer = pBaseLayer->pDown;
            delete pBaseLayer;
            pBaseLayer = pLayer;
        }
    }
}

/*************************************************************************
|*
|* Drag beginnen, vorher ggf. Drag-Methode fuer 3D-Objekte erzeugen
|*
\************************************************************************/

BOOL E3dView::BegDragObj(const Point& rPnt, OutputDevice* pOut,
    SdrHdl* pHdl, short nMinMov,
    SdrDragMethod* pForcedMeth)
{
    if (b3dCreationActive && aMark.GetMarkCount())
    {
        // bestimme alle selektierten Polygone und gebe die gespiegelte Hilfsfigur aus
        if (!pMirrorPolygon && !pMirroredPolygon)
        {
            CreateMirrorPolygons ();
            ShowMirrorPolygons (aRef1, aRef2);
        }
    }
    else
    {
        BOOL bOwnActionNecessary;
        if (pHdl == NULL)
        {
           bOwnActionNecessary = TRUE;
        }
        else if (pHdl->IsVertexHdl() || pHdl->IsCornerHdl())
        {
           bOwnActionNecessary = TRUE;
        }
        else
        {
           bOwnActionNecessary = FALSE;
        }

        if(bOwnActionNecessary && aMark.GetMarkCount() >= 1)
        {
            E3dDragConstraint eConstraint = E3DDRAG_CONSTR_XYZ;
            BOOL bThereAreRootScenes = FALSE;
            BOOL bThereAre3DObjects = FALSE;
            long nCnt = aMark.GetMarkCount();
            for(long nObjs = 0;nObjs < nCnt;nObjs++)
            {
                SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
                if(pObj)
                {
                    if(pObj->ISA(E3dScene) && ((E3dScene*)pObj)->GetScene() == pObj)
                        bThereAreRootScenes = TRUE;
                    if(pObj->ISA(E3dObject))
                        bThereAre3DObjects = TRUE;
                }
            }
            if( bThereAre3DObjects )
            {
                eDragHdl = ( pHdl == NULL ? HDL_MOVE : pHdl->GetKind() );
                switch ( eDragMode )
                {
                    case SDRDRAG_ROTATE:
                    case SDRDRAG_SHEAR:
                    {
                        switch ( eDragHdl )
                        {
                            case HDL_LEFT:
                            case HDL_RIGHT:
                            {
                                eConstraint = E3DDRAG_CONSTR_X;
                            }
                            break;

                            case HDL_UPPER:
                            case HDL_LOWER:
                            {
                                eConstraint = E3DDRAG_CONSTR_Y;
                            }
                            break;

                            case HDL_UPLFT:
                            case HDL_UPRGT:
                            case HDL_LWLFT:
                            case HDL_LWRGT:
                            {
                                eConstraint = E3DDRAG_CONSTR_Z;
                            }
                            break;
                        }

                        // die nicht erlaubten Rotationen ausmaskieren
                        eConstraint = E3dDragConstraint(eConstraint& eDragConstraint);
                        String aTmp = SfxIniManager::Get()->Get( SFX_KEY_3D_SHOWFULL );
                        pForcedMeth = new E3dDragRotate(*this, aMark, eDragDetail, eConstraint,
                            aTmp.Len() && aTmp.GetChar(0) != sal_Unicode('0') );
                    }
                    break;

                    case SDRDRAG_MOVE:
                    {
                        if(!bThereAreRootScenes)
                        {
                            String aTmp = SfxIniManager::Get()->Get( SFX_KEY_3D_SHOWFULL );
                            pForcedMeth = new E3dDragMove(*this, aMark, eDragDetail, eDragHdl, eConstraint,
                                aTmp.Len() && aTmp.GetChar(0) != sal_Unicode('0') );
                        }
                    }
                    break;

                    // spaeter mal
                    case SDRDRAG_MIRROR:
                    case SDRDRAG_CROOK:
                    case SDRDRAG_DISTORT:
                    case SDRDRAG_TRANSPARENCE:
                    case SDRDRAG_GRADIENT:
                    default:
                    {
                        long nCnt = aMark.GetMarkCount();
                        for(long nObjs = 0;nObjs < nCnt;nObjs++)
                        {
                            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
                            if(pObj && pObj->ISA(E3dObject))
                                ((E3dObject*) pObj)->SetDragDetail(eDragDetail);
                        }
                    }
                    break;
                }
            }
        }
    }
    return SdrView::BegDragObj(rPnt, pOut, pHdl, nMinMov, pForcedMeth);
}

/*************************************************************************
|*
|* Pruefen, obj 3D-Szene markiert ist
|*
\************************************************************************/

BOOL E3dView::HasMarkedScene()
{
    return (GetMarkedScene() != NULL);
}

/*************************************************************************
|*
|* Pruefen, obj 3D-Szene markiert ist
|*
\************************************************************************/

E3dScene* E3dView::GetMarkedScene()
{
    ULONG nCnt = aMark.GetMarkCount();

    for ( ULONG i = 0; i < nCnt; i++ )
        if ( aMark.GetMark(i)->GetObj()->ISA(E3dScene) )
            return (E3dScene*) aMark.GetMark(i)->GetObj();

    return NULL;
}

/*************************************************************************
|*
|* aktuelles 3D-Zeichenobjekt setzen, dafuer Szene erzeugen
|*
\************************************************************************/

void E3dView::SetCurrent3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj != NULL, "Nana, wer steckt denn hier 'nen NULL-Zeiger rein?");
    E3dScene* pScene = NULL;

    const Volume3D& rVolume = p3DObj->GetBoundVolume();
    double fW = rVolume.GetWidth();
    double fH = rVolume.GetHeight();
    Rectangle aRect(0,0, (long) fW, (long) fH);

    pScene = new E3dPolyScene(Get3DDefaultAttributes());

    InitScene(pScene, fW, fH, rVolume.MaxVec().Z() + ((fW + fH) / 4.0));

    pScene->Insert3DObj(p3DObj);
    pScene->NbcSetSnapRect(aRect);
    SetCurrentLibObj(pScene);
}

/*************************************************************************
|*
|* neu erzeugte Szene initialisieren
|*
\************************************************************************/

void E3dView::InitScene(E3dScene* pScene, double fW, double fH, double fCamZ)
{
    Camera3D aCam(pScene->GetCamera());

    aCam.SetAutoAdjustProjection(FALSE);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    Vector3D aLookAt;
    Vector3D aCamPos (aDefaultCamPos.X (), aDefaultCamPos.Y (), fCamZ < aDefaultCamPos.Z ()
                                                                    ? aDefaultCamPos.Z ()
                                                                    : fCamZ);

    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(fDefaultCamFocal);
    aCam.SetDefaults(aDefaultCamPos, aLookAt, fDefaultCamFocal);
    pScene->SetCamera(aCam);
}

/*************************************************************************
|*
|* startsequenz fuer die erstellung eines 3D-Rotationskoerpers
|*
\************************************************************************/

void E3dView::Start3DCreation ()
{
    b3dCreationActive = TRUE;

    if (aMark.GetMarkCount())
    {
        // irgendwelche Markierungen ermitteln und ausschalten
        BOOL bVis = IsMarkHdlShown();

        if (bVis) HideMarkHdl(NULL);

        // bestimme die koordinaten fuer JOEs Mirrorachse
        // entgegen der normalen Achse wird diese an die linke Seite des Objektes
        // positioniert
        long          nOutMin = 0;
        long          nOutMax = 0;
        long          nMinLen = 0;
        long          nObjDst = 0;
        long          nOutHgt = 0;
        OutputDevice* pOut    = GetWin(0);

        // erstmal Darstellungsgrenzen bestimmen
        if (pOut != NULL)
        {
            nMinLen = pOut->PixelToLogic(Size(0,50)).Height();
            nObjDst = pOut->PixelToLogic(Size(0,20)).Height();

            long nDst = pOut->PixelToLogic(Size(0,10)).Height();

            nOutMin =  -pOut->GetMapMode().GetOrigin().Y();
            nOutMax =  pOut->GetOutputSize().Height() - 1 + nOutMin;
            nOutMin += nDst;
            nOutMax -= nDst;

            if (nOutMax - nOutMin < nDst)
            {
                nOutMin += nOutMax + 1;
                nOutMin /= 2;
                nOutMin -= (nDst + 1) / 2;
                nOutMax  = nOutMin + nDst;
            }

            nOutHgt = nOutMax - nOutMin;

            long nTemp = nOutHgt / 4;
            if (nTemp > nMinLen) nMinLen = nTemp;
        }

        // und dann die Markierungen oben und unten an das Objekt heften
        Rectangle aR;
        for(UINT32 nMark = 0; nMark < aMark.GetMarkCount(); nMark++)
        {
            XPolyPolygon aXPP;
            SdrObject* pMark = aMark.GetMark(nMark)->GetObj();
            pMark->TakeXorPoly(aXPP, FALSE);
            aR.Union(aXPP.GetBoundRect());
        }
        Point     aCenter(aR.Center());
        long      nMarkHgt = aR.GetHeight() - 1;
        long      nHgt     = nMarkHgt + nObjDst * 2;

        if (nHgt < nMinLen) nHgt = nMinLen;

        long nY1 = aCenter.Y() - (nHgt + 1) / 2;
        long nY2 = nY1 + nHgt;

        if (pOut && (nMinLen > nOutHgt)) nMinLen = nOutHgt;
        if (pOut)
        {
            if (nY1 < nOutMin)
            {
                nY1 = nOutMin;
                if (nY2 < nY1 + nMinLen) nY2 = nY1 + nMinLen;
            }
            if (nY2 > nOutMax)
            {
                nY2 = nOutMax;
                if (nY1 > nY2 - nMinLen) nY1 = nY2 - nMinLen;
            }
        }

        aRef1.X() = aR.Left();    // Initial Achse um 2/100mm nach links
        aRef1.Y() = nY1;
        aRef2.X() = aRef1.X();
        aRef2.Y() = nY2;

        // Markierungen einschalten
        SetMarkHandles();

        if (bVis) ShowMarkHdl(NULL);
        if (HasMarkedObj()) MarkListHasChanged();

        // SpiegelPolygone SOFORT zeigen
        CreateMirrorPolygons ();
        const SdrHdlList &aHdlList = GetHdlList ();
        ShowMirrorPolygons (aHdlList.GetHdl (HDL_REF1)->GetPos (),
                            aHdlList.GetHdl (HDL_REF2)->GetPos ());
    }
}

/*************************************************************************
|*
|* was passiert bei einer Mausbewegung, wenn das Objekt erstellt wird ?
|*
\************************************************************************/

void E3dView::MovAction(const Point& rPnt)
{
    if (b3dCreationActive)
    {
        SdrHdl* pHdl = GetDragHdl();

        if (pHdl)
        {
            SdrHdlKind eHdlKind = pHdl->GetKind();

            // reagiere nur bei einer spiegelachse
            if ((eHdlKind == HDL_REF1) ||
                (eHdlKind == HDL_REF2) ||
                (eHdlKind == HDL_MIRX))
            {
                const SdrHdlList &aHdlList = GetHdlList ();

                // loesche das gespiegelte Polygon, spiegele das Original und zeichne es neu
                b3dCreationActive = FALSE;  // Damit in DrawDragObj() gezeichnet wird
                b3dCreationActive = TRUE;   // restaurieren (Trick)
                ShowMirrored ();
                SdrView::MovAction (rPnt);
                ShowMirrorPolygons (aHdlList.GetHdl (HDL_REF1)->GetPos (),
                                    aHdlList.GetHdl (HDL_REF2)->GetPos ());
            }
        }
        else
        {
            SdrView::MovAction (rPnt);
        }
    }
    else
    {
        SdrView::MovAction (rPnt);
    }
}

/*************************************************************************
|*
|* Schluss. Objekt und evtl. Unterobjekte ueber ImpCreate3DLathe erstellen
|*          [FG] Mit dem Parameterwert TRUE (SDefault: FALSE) wird einfach ein
|*               Rotationskoerper erzeugt, ohne den Benutzer die Lage der
|*               Achse fetlegen zu lassen. Es reicht dieser Aufruf, falls
|*               ein Objekt selektiert ist. (keine Initialisierung noetig)
|*
\************************************************************************/

void E3dView::End3DCreation(BOOL bUseDefaultValuesForMirrorAxes)
{
    if(HasMarkedObj())
    {
        if(bUseDefaultValuesForMirrorAxes)
        {
            Rectangle aRect = GetAllMarkedRect();
            if(aRect.GetWidth() <= 1)
                aRect.SetSize(Size(500, aRect.GetHeight()));
            if(aRect.GetHeight() <= 1)
                aRect.SetSize(Size(aRect.GetWidth(), 500));

            Vector3D aPnt1(aRect.Left(), -aRect.Top(), 0.0);
            Vector3D aPnt2(aRect.Left(), -aRect.Bottom(), 0.0);

            ConvertMarkedObjTo3D(FALSE, aPnt1, aPnt2);
        }
        else
        {
            // Hilfsfigur ausschalten
            ShowMirrored();

            // irgendwo kassieren wir eine Rekursion, also unterbinden
            b3dCreationActive = FALSE;

            // bestimme aus den Handlepositionen und den Versatz der Punkte
            const SdrHdlList &aHdlList = GetHdlList();
            Point aMirrorRef1 = aHdlList.GetHdl(HDL_REF1)->GetPos();
            Point aMirrorRef2 = aHdlList.GetHdl(HDL_REF2)->GetPos();

            Vector3D aPnt1(aMirrorRef1.X(), -aMirrorRef1.Y(), 0.0);
            Vector3D aPnt2(aMirrorRef2.X(), -aMirrorRef2.Y(), 0.0);

            ConvertMarkedObjTo3D(FALSE, aPnt1, aPnt2);
        }
    }

    ResetCreationActive();
}

/*************************************************************************
|*
|* stelle das Mirrorobjekt dar
|*
\************************************************************************/

void E3dView::ShowMirrored ()
{
    if (b3dCreationActive)
    {
        OutputDevice  *pOut = GetWin(0);
        RasterOp      eRop0 = pOut->GetRasterOp();

        Color aOldLineColor( pXOut->GetLineColor() );
        Color aOldFillColor( pXOut->GetFillColor() );
        Color aNewLineColor( COL_BLACK );
        Color aNewFillColor( COL_TRANSPARENT );


        // invertiere die Darstellung
        pOut->SetRasterOp(ROP_INVERT);
        pXOut->SetOutDev(pOut);
        pXOut->OverrideLineColor( aNewLineColor );
        pXOut->OverrideFillColor( aNewFillColor );

        for (long nMark = 0;
                  nMark < nPolyCnt;
                  nMark ++)
        {
            const XPolyPolygon &rXPP    = pMirroredPolygon [nMark];
            USHORT             nPolyAnz = rXPP.Count();

            for (USHORT nPolyNum = 0;
                       nPolyNum < nPolyAnz;
                       nPolyNum ++)
            {
                const XPolygon &rXP = rXPP [nPolyNum];

                pXOut->DrawXPolyLine(rXP);
            }
        }

        pXOut->OverrideLineColor( aOldLineColor );
        pXOut->OverrideFillColor( aOldFillColor );
        pOut->SetRasterOp(eRop0);
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dView::~E3dView ()
{
    __DELETE(nPolyCnt) pMirrorPolygon;
    __DELETE(nPolyCnt) pMirroredPolygon;
    __DELETE(nPolyCnt) pMarkedObjs;
}

/*************************************************************************
|*
|* Bestimme Anzahl der Polygone und kopiere in die Spiegelpolygone
|*
\************************************************************************/

void E3dView::CreateMirrorPolygons ()
{
    nPolyCnt         = aMark.GetMarkCount();
    pMirrorPolygon   = new XPolyPolygon [nPolyCnt];
    pMirroredPolygon = new XPolyPolygon [nPolyCnt];
    pMarkedObjs      = new SdrObject* [nPolyCnt];
    pMyPV            = aMark.GetMark(0)->GetPageView();

    for (long nMark = nPolyCnt;
              nMark > 0;
        )
    {
        SdrMark   *pMark = aMark.GetMark(-- nMark);
        SdrObject *pObj  = pMark->GetObj();

        pObj->TakeXorPoly (pMirrorPolygon [nMark], FALSE);
        pMarkedObjs [nMark] = pObj;
    }
}

/*************************************************************************
|*
|* spiegele die originalpolygone und stelle sie als hilfsfigur dar
|*
\************************************************************************/

void E3dView::ShowMirrorPolygons (Point aMirrorPoint1,
                                  Point aMirrorPoint2)
{
    for (long nMark = 0;
              nMark < nPolyCnt;
              nMark ++)
    {
        pMirroredPolygon [nMark] = pMirrorPolygon [nMark];
        MirrorXPoly(pMirroredPolygon [nMark], aMirrorPoint1, aMirrorPoint2);
    }

    if (nPolyCnt) ShowMirrored ();
}

/*************************************************************************
|*
|* beende das erzeugen und loesche die polygone
|*
\************************************************************************/

void E3dView::ResetCreationActive ()
{
    __DELETE(nPolyCnt) pMirrorPolygon;
    __DELETE(nPolyCnt) pMirroredPolygon;
    __DELETE(nPolyCnt) pMarkedObjs;

    pMarkedObjs       = 0;
    pMirrorPolygon    =
    pMirroredPolygon  = 0;
    b3dCreationActive = FALSE;
    nPolyCnt          = 0;
}

/*************************************************************************
|*
|* Skalarprodukt zweier Punktvektoren
|*
\************************************************************************/

long Scalar (Point aPoint1,
             Point aPoint2)
{
    return aPoint1.X () * aPoint2.X () + aPoint1.Y () * aPoint2.Y ();
}

/*************************************************************************
|*
|* Skalarprodukt zweier Punktvektoren
|*
\************************************************************************/

Point ScaleVector (Point  aPoint,
                   double nScale)
{
    return Point ((long) ((double) aPoint.X () * nScale), (long) ((double) aPoint.Y () * nScale));
}

/*************************************************************************
|*
|* Skalarprodukt zweier Punktvektoren
|*
\************************************************************************/

double NormVector (Point aPoint)
{
    return sqrt ((double) Scalar (aPoint, aPoint));
}

/*************************************************************************
|*
|* Pruefe, ob sich zwei Geradensegemente schneiden
|* Dazu wird ueber einfache Determinanten bestimmt, wie die Endpunkte
|* zu der jeweils anderen Gerade liegen.
|*
\************************************************************************/

BOOL LineCutting (Point aP1,
                  Point aP2,
                  Point aP3,
                  Point aP4)
{
    long nS1 = Point2Line (aP1, aP3, aP4);
    long nS2 = Point2Line (aP2, aP3, aP4);
    long nS3 = Point2Line (aP3, aP1, aP2);
    long nS4 = Point2Line (aP4, aP1, aP2);

    // die werte koennen reichlich gross werden, also geht eine multiplikation daneben
    BOOL bCut (((nS1 < 0) && (nS2 > 0) || (nS1 > 0) && (nS2 < 0)) &&
              ((nS3 < 0) && (nS4 > 0) || (nS3 > 0) && (nS4 < 0)));

    if (bCut)
    {
        BOOL bStop = bCut;
    }

    return ((nS1 < 0) && (nS2 > 0) || (nS1 > 0) && (nS2 < 0)) &&
           ((nS3 < 0) && (nS4 > 0) || (nS3 > 0) && (nS4 < 0));
}

/*************************************************************************
|*
|* Bestimme, ob sich ein Punkt aP1 rechts oder links eines Geradensegments,
|* definiert durch aP2 und aP3, befindet.
|* >0 : rechts, <0 : links, =0 : auf dem Geradensegment
|* Die Vektoren (Punkte) liegen in der homogenen Form vor, wobei die
|* Skalierung =1 gesetzt ist (schneller und einfacher).
|*
\************************************************************************/

long Point2Line (Point aP1,
                 Point aP2,
                 Point aP3)
{
    return (aP2.X () * aP3.Y () - aP2.Y () * aP3.X ()) -
           (aP1.X () * aP3.Y () - aP1.Y () * aP3.X ()) +
           (aP1.X () * aP2.Y () - aP1.Y () * aP2.X ());
}

/*************************************************************************
|*
|* Bestimme den Abstand eines Punktes u zu einem Geradensegment,
|* definiert durch v1 und v.
|*
\************************************************************************/

long DistPoint2Line (Point u,
                     Point v1,
                     Point v)
{
    Point w = v1 - v;

    return (long) NormVector (v - ScaleVector (w, (double) Scalar (v - u, w) / (double) Scalar (w, w)) - u);
}

/*************************************************************************
|*
|* Klasse initialisieren
|*
\************************************************************************/

void E3dView::InitView ()
{
    eDragConstraint          = E3DDRAG_CONSTR_XYZ;
    eDragDetail              = E3DDETAIL_ONEBOX;
    b3dCreationActive        = FALSE;
    pMirrorPolygon           = 0;
    pMirroredPolygon         = 0;
    nPolyCnt                 = 0;
    pMyPV                    = 0;
    pMarkedObjs              = 0;
    fDefaultScaleX           =
    fDefaultScaleY           =
    fDefaultScaleZ           = 1.0;
    fDefaultRotateX          =
    fDefaultRotateY          =
    fDefaultRotateZ          = 0.0;
    fDefaultExtrusionDeepth  = 1000; // old: 2000;
    fDefaultCamFocal         = 100;
    fDefaultLightIntensity   = 0.8; // old: 0.6;
    fDefaultAmbientIntensity = 0.4;
    nHDefaultSegments        = 12;
    nVDefaultSegments        = 12;
    aDefaultLightColor       = RGB_Color(COL_WHITE);
    aDefaultAmbientColor     = RGB_Color(COL_BLACK);
    aDefaultCamPos           = Vector3D (0, 0, 100);
    aDefaultLightPos         = Vector3D (1, 1, 1); // old: Vector3D (0, 0, 1);
    aDefaultLightPos.Normalize();
    bDoubleSided             = FALSE;
}

/*************************************************************************
|*
|* Zeige eine Hilfsfigur
|*
\************************************************************************/

void E3dView::ShowDragObj (OutputDevice *pOut)
{
    SdrView::ShowDragObj (pOut);
}

/*************************************************************************
|*
|* Verdecke eine Hilfsfigur
|*
\************************************************************************/

void E3dView::HideDragObj (OutputDevice *pOut)
{
    SdrView::HideDragObj (pOut);
}

/*************************************************************************
|*
|* Zeige eine Hilfsfigur
|*
\************************************************************************/

void E3dView::DrawDragObj (OutputDevice *pOut,
                           BOOL     bFull) const
{
    if (!b3dCreationActive)
    {
        SdrView::DrawDragObj (pOut, bFull);
    }
}

/*************************************************************************
|*
|* Koennen die selektierten Objekte aufgebrochen werden?
|*
\************************************************************************/

BOOL E3dView::IsBreak3DObjPossible() const
{
    ULONG nCount = aMark.GetMarkCount();

    if (nCount > 0)
    {
        ULONG i = 0;

        while (i < nCount)
        {
            SdrObject* pObj = aMark.GetMark(i)->GetObj();

            if (pObj && pObj->ISA(E3dObject))
            {
                if(!(((E3dObject*)pObj)->IsBreakObjPossible()))
                    return FALSE;
            }
            else
            {
                return FALSE;
            }

            i++;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/*************************************************************************
|*
|* Selektierte Lathe-Objekte aufbrechen
|*
\************************************************************************/

void E3dView::Break3DObj()
{
    if(IsBreak3DObjPossible())
    {
        // ALLE selektierten Objekte werden gewandelt
        UINT32 nCount = aMark.GetMarkCount();

        BegUndo(String(SVX_RESSTR(RID_SVX_3D_UNDO_BREAK_LATHE)));
        for(UINT32 a=0;a<nCount;a++)
        {
            E3dObject* pObj = (E3dObject*)aMark.GetMark(a)->GetObj();
            BreakSingle3DObj(pObj);
        }
        DeleteMarked();
        EndUndo();
    }
}

void E3dView::BreakSingle3DObj(E3dObject* pObj)
{
    if(pObj->ISA(E3dScene))
    {
        SdrObjList* pSubList = pObj->GetSubList();
        SdrObjListIter aIter(*pSubList, IM_FLAT);

        while(aIter.IsMore())
        {
            E3dObject* pSubObj = (E3dObject*)aIter.Next();
            BreakSingle3DObj(pSubObj);
        }
    }
    else
    {
        SdrAttrObj* pNewObj = pObj->GetBreakObj();
        if(pNewObj)
        {
            InsertObject(pNewObj, *GetPageViewPvNum(0), SDRINSERT_DONTMARK);
            pNewObj->SendRepaintBroadcast();
        }
    }
}

/*************************************************************************
|*
|* Szenen mischen
|*
\************************************************************************/
// Wird bisher noch nirgenswo (weder im Draw oder Chart) aufgerufen

void E3dView::MergeScenes ()
{
    ULONG nCount = aMark.GetMarkCount();

    if (nCount > 0)
    {
        ULONG     nObj    = 0;
        SdrObject *pObj   = aMark.GetMark(nObj)->GetObj();
        E3dScene  *pScene = new E3dPolyScene(Get3DDefaultAttributes());
        Volume3D  aBoundVol;
        Rectangle aAllBoundRect (GetMarkedObjBoundRect ());
        Point     aCenter (aAllBoundRect.Center());

        while (pObj)
        {
            if (pObj->ISA(E3dScene))
            {
                /**********************************************************
                * Es ist eine 3D-Scene oder 3D-PolyScene
                **********************************************************/
                SdrObjList* pSubList = ((E3dObject*) pObj)->GetSubList();

                SdrObjListIter aIter(*pSubList, IM_FLAT);

                while (aIter.IsMore())
                {
                    /******************************************************
                    * LatheObjekte suchen
                    ******************************************************/
                    SdrObject* pSubObj = aIter.Next();

                    if (!pSubObj->ISA(E3dLight))
                    {
                        E3dObject *pNewObj = 0;

                        switch (pSubObj->GetObjIdentifier())
                        {
                            case E3D_OBJECT_ID:
                                pNewObj = new E3dObject;
                                *(E3dObject*)pNewObj = *(E3dObject*)pSubObj;
                                break;

                            case E3D_POLYOBJ_ID :
                                pNewObj = new E3dPolyObj;
                                *(E3dPolyObj*)pNewObj= *(E3dPolyObj*)pSubObj;
                                break;

                            case E3D_CUBEOBJ_ID :
                                pNewObj = new E3dCubeObj;
                                *(E3dCubeObj*)pNewObj = *(E3dCubeObj*)pSubObj;
                                break;

                            case E3D_SPHEREOBJ_ID:
                                pNewObj = new E3dSphereObj;
                                *(E3dSphereObj*)pNewObj = *(E3dSphereObj*)pSubObj;
                                break;

                            case E3D_POINTOBJ_ID:
                                pNewObj = new E3dPointObj;
                                *(E3dPointObj*)pNewObj = *(E3dPointObj*)pSubObj;
                                break;

                            case E3D_EXTRUDEOBJ_ID:
                                pNewObj = new E3dExtrudeObj;
                                *(E3dExtrudeObj*)pNewObj = *(E3dExtrudeObj*)pSubObj;
                                break;

                            case E3D_LATHEOBJ_ID:
                                pNewObj = new E3dLatheObj;
                                *(E3dLatheObj*)pNewObj = *(E3dLatheObj*)pSubObj;
                                break;

                            case E3D_LABELOBJ_ID:
                                pNewObj = new E3dLabelObj;
                                *(E3dLabelObj*)pNewObj = *(E3dLabelObj*)pSubObj;
                                break;

                            case E3D_COMPOUNDOBJ_ID:
                                pNewObj = new E3dCompoundObject;
                                *(E3dCompoundObject*)pNewObj = *(E3dCompoundObject*)pSubObj;
                                break;
                        }

                        Rectangle aBoundRect = pSubObj->GetBoundRect ();

                        Matrix4D aMatrix;
                        aMatrix.Translate(Vector3D(aBoundRect.Left () - aCenter.X (), aCenter.Y(), 0));
                        pNewObj->ApplyTransform(aMatrix);

                        if (pNewObj) aBoundVol.Union (pNewObj->GetBoundVolume());
                        pScene->Insert3DObj (pNewObj);
                    }
                }
            }

            nObj++;

            if (nObj < nCount)
            {
                pObj = aMark.GetMark(nObj)->GetObj();
            }
            else
            {
                pObj = NULL;
            }
        }

        double fW = aAllBoundRect.GetWidth();
        double fH = aAllBoundRect.GetHeight();
        Rectangle aRect(0,0, (long) fW, (long) fH);

        InitScene(pScene, fW, fH, aBoundVol.MaxVec().Z() +  + ((fW + fH) / 4.0));

        pScene->FitSnapRectToBoundVol();
        pScene->NbcSetSnapRect(aRect);

        Camera3D &aCamera  = (Camera3D&) pScene->GetCamera ();
        Vector3D aMinVec (aBoundVol.MinVec ());
        Vector3D aMaxVec (aBoundVol.MaxVec ());
        double fDeepth = fabs (aMaxVec.Z () - aMinVec.Z ());

        aCamera.SetPRP (Vector3D (0, 0, 1000));
        aCamera.SetPosition (Vector3D (aDefaultCamPos.X (), aDefaultCamPos.Y (),
                                       aDefaultCamPos.Z () + fDeepth / 2));
        aCamera.SetFocalLength(fDefaultCamFocal);
        pScene->SetCamera (aCamera);

        // SnapRects der Objekte ungueltig
        pScene->SetRectsDirty();

        // Transformationen initialisieren, damit bei RecalcSnapRect()
        // richtig gerechnet wird
        pScene->InitTransformationSet();

        InsertObject (pScene, *(aMark.GetMark(0)->GetPageView()));

        // SnapRects der Objekte ungueltig
        pScene->SetRectsDirty();
    }
}

/*************************************************************************
|*
|* Possibilities, hauptsaechlich gruppieren/ungruppieren
|*
\************************************************************************/
void E3dView::CheckPossibilities()
{
    // call parent
    SdrView::CheckPossibilities();

    // Weitere Flags bewerten
    if(bGroupPossible || bUnGroupPossible || bGrpEnterPossible)
    {
        INT32 nMarkCnt = aMark.GetMarkCount();
        BOOL bCoumpound = FALSE;
        BOOL b3DObject = FALSE;
        for(INT32 nObjs = 0L; (nObjs < nMarkCnt) && !bCoumpound; nObjs++)
        {
            SdrObject *pObj = aMark.GetMark(nObjs)->GetObj();
            if(pObj && pObj->ISA(E3dCompoundObject))
                bCoumpound = TRUE;
            if(pObj && pObj->ISA(E3dObject))
                b3DObject = TRUE;
        }

        // Bisher: Es sind ZWEI oder mehr beliebiger Objekte selektiert.
        // Nachsehen, ob CompoundObjects beteiligt sind. Falls ja,
        // das Gruppieren verbieten.
        if(bGroupPossible && bCoumpound)
            bGroupPossible = FALSE;

        if(bUnGroupPossible && b3DObject)
            bUnGroupPossible = FALSE;

        if(bGrpEnterPossible && bCoumpound)
            bGrpEnterPossible = FALSE;
    }

//  bGroupPossible
//  bCombinePossible
//  bUnGroupPossible
//  bGrpEnterPossible
}



