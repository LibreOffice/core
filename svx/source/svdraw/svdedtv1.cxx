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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <math.h>
#include <svl/aeitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <tools/bigint.hxx>
#include <vcl/msgbox.hxx>

#include "svx/svdglob.hxx"  // StringCache
#include "svx/svditer.hxx"
#include "svx/svdstr.hrc"   // names taken from the resource

#include <svx/AffineMatrixItem.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/obj3d.hxx>
#include <svx/rectenum.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdattr.hxx>  // for Get/SetGeoAttr
#include <svx/svdedtv.hxx>
#include <svx/svdetc.hxx>   // for SearchOutlinerItems
#include <svx/svdlayer.hxx> // for MergeNotPersistAttr
#include <svx/svdopath.hxx>  // for Crook
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svxids.hrc>   // for SID_ATTR_TRANSFORM_...
#include <svx/sxallitm.hxx>
#include <svx/sxmovitm.hxx>
#include <svx/sxreaitm.hxx>
#include <svx/sxreoitm.hxx>
#include <svx/sxroaitm.hxx>
#include <svx/sxrooitm.hxx>
#include <svx/sxsalitm.hxx>
#include <svx/sxsoitm.hxx>
#include <svx/sxtraitm.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// EditView
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetMarkedObjRect(const Rectangle& rRect, sal_Bool bCopy)
{
    DBG_ASSERT(!rRect.IsEmpty(),"SetMarkedObjRect() with an empty Rect does not make sense.");
    if (rRect.IsEmpty()) return;
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz==0) return;
    Rectangle aR0(GetMarkedObjRect());
    DBG_ASSERT(!aR0.IsEmpty(),"SetMarkedObjRect(): GetMarkedObjRect() is empty.");
    if (aR0.IsEmpty()) return;
    long x0=aR0.Left();
    long y0=aR0.Top();
    long w0=aR0.Right()-x0;
    long h0=aR0.Bottom()-y0;
    long x1=rRect.Left();
    long y1=rRect.Top();
    long w1=rRect.Right()-x1;
    long h1=rRect.Bottom()-y1;
    OUString aStr;
    ImpTakeDescriptionStr(STR_EditPosSize,aStr);
    if (bCopy)
        aStr+=ImpGetResStr(STR_EditWithCopy);

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(aStr);

    if (bCopy)
        CopyMarkedObj();

    for (sal_uIntPtr nm=0; nm<nAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));

        Rectangle aR1(pO->GetSnapRect());
        if (!aR1.IsEmpty())
        {
            if (aR1==aR0)
            {
                aR1=rRect;
            }
            else
            { // transform aR1 to aR0 after rRect
                aR1.Move(-x0,-y0);
                BigInt l(aR1.Left());
                BigInt r(aR1.Right());
                BigInt t(aR1.Top());
                BigInt b(aR1.Bottom());
                if (w0!=0) {
                    l*=w1; l/=w0;
                    r*=w1; r/=w0;
                } else {
                    l=0; r=w1;
                }
                if (h0!=0) {
                    t*=h1; t/=h0;
                    b*=h1; b/=h0;
                } else {
                    t=0; b=h1;
                }
                aR1.Left  ()=long(l);
                aR1.Right ()=long(r);
                aR1.Top   ()=long(t);
                aR1.Bottom()=long(b);
                aR1.Move(x1,y1);
            }
            pO->SetSnapRect(aR1);
        } else {
            OSL_FAIL("SetMarkedObjRect(): pObj->GetSnapRect() returns empty Rect");
        }
    }
    if( bUndo )
        EndUndo();
}

std::vector< SdrUndoAction* > SdrEditView::CreateConnectorUndo( SdrObject& rO )
{
    std::vector< SdrUndoAction* > vUndoActions;

    if ( rO.GetBroadcaster() )
    {
        const SdrPage* pPage = rO.GetPage();
        if ( pPage )
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            while( aIter.IsMore() )
            {
                SdrObject* pPartObj = aIter.Next();
                if ( pPartObj->ISA( SdrEdgeObj ) )
                {
                    if ( ( pPartObj->GetConnectedNode( sal_False ) == &rO ) ||
                         ( pPartObj->GetConnectedNode( sal_True  ) == &rO ) )
                    {
                        vUndoActions.push_back( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject( *pPartObj ) );
                    }
                }
            }
        }
    }
    return vUndoActions;
}

void SdrEditView::AddUndoActions( std::vector< SdrUndoAction* >& rUndoActions )
{
    std::vector< SdrUndoAction* >::iterator aUndoActionIter( rUndoActions.begin() );
    while( aUndoActionIter != rUndoActions.end() )
        AddUndo( *aUndoActionIter++ );
}

void SdrEditView::MoveMarkedObj(const Size& rSiz, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        XubString aStr(ImpGetResStr(STR_EditMove));
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        // meeds its own UndoGroup because of its parameters
        BegUndo(aStr,GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_MOVE);
    }

    if (bCopy)
        CopyMarkedObj();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
        {
            std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
            AddUndoActions( vConnectorUndoActions );
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*pO,rSiz));
        }
        pO->Move(rSiz);
    }

    if( bUndo )
        EndUndo();
}

void SdrEditView::ResizeMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_EditResize,aStr);
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
        {
            std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
            AddUndoActions( vConnectorUndoActions );
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
        }
        pO->Resize(rRef,xFact,yFact);
    }

    if( bUndo )
        EndUndo();
}
void SdrEditView::ResizeMultMarkedObj(const Point& rRef,
    const Fraction& xFact,
    const Fraction& yFact,
    const bool bCopy,
    const bool bWdh,
    const bool bHgt)
{
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_EditResize,aStr);
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
        {
            std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
            AddUndoActions( vConnectorUndoActions );
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
        }

        Fraction aFrac(1,1);
        if (bWdh && bHgt)
            pO->Resize(rRef, xFact, yFact);
        else if (bWdh)
            pO->Resize(rRef, xFact, aFrac);
        else if (bHgt)
            pO->Resize(rRef, aFrac, yFact);
    }
    if( bUndo )
        EndUndo();
}

long SdrEditView::GetMarkedObjRotate() const
{
    long nRetval(0);

    if(GetMarkedObjectCount())
    {
        SdrMark* pM = GetSdrMarkByIndex(0);
        SdrObject* pO = pM->GetMarkedSdrObj();

        nRetval = pO->GetRotateAngle();
    }

    return nRetval;
    //sal_Bool b1st=sal_True;
    //sal_Bool bOk=sal_True;
    //long nWink=0;
    //sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    //for (sal_uIntPtr nm=0; nm<nMarkAnz && bOk; nm++) {
    //  SdrMark* pM=GetSdrMarkByIndex(nm);
    //  SdrObject* pO=pM->GetMarkedSdrObj();
    //  long nWink2=pO->GetRotateAngle();
    //  if (b1st) nWink=nWink2;
    //  else if (nWink2!=nWink) bOk=sal_False;
    //  b1st=sal_False;
    //}
    //if (!bOk) nWink=0;
    //return nWink;
}

void SdrEditView::RotateMarkedObj(const Point& rRef, long nWink, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_EditRotate,aStr);
        if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    const sal_uInt32 nMarkAnz(GetMarkedObjectCount());

    if(nMarkAnz)
    {
        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

        for(sal_uInt32 nm(0); nm < nMarkAnz; nm++)
        {
            SdrMark* pM = GetSdrMarkByIndex(nm);
            SdrObject* pO = pM->GetMarkedSdrObj();

            if( bUndo )
            {
                // extra undo actions for changed connector which now may hold its laid out path (SJ)
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );

                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            // set up a scene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pO))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pO));
            }

            pO->Rotate(rRef,nWink,nSin,nCos);
        }

        // fire scene updaters
        while(!aUpdaters.empty())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }
    }

    if( bUndo )
        EndUndo();
}

void SdrEditView::MirrorMarkedObj(const Point& rRef1, const Point& rRef2, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        OUString aStr;
        Point aDif(rRef2-rRef1);
        if (aDif.X()==0) ImpTakeDescriptionStr(STR_EditMirrorHori,aStr);
        else if (aDif.Y()==0) ImpTakeDescriptionStr(STR_EditMirrorVert,aStr);
        else if (std::abs(aDif.X()) == std::abs(aDif.Y())) ImpTakeDescriptionStr(STR_EditMirrorDiag,aStr);
        else ImpTakeDescriptionStr(STR_EditMirrorFree,aStr);
        if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    const sal_uInt32 nMarkAnz(GetMarkedObjectCount());

    if(nMarkAnz)
    {
        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

        for(sal_uInt32 nm(0); nm < nMarkAnz; nm++)
        {
            SdrMark* pM = GetSdrMarkByIndex(nm);
            SdrObject* pO = pM->GetMarkedSdrObj();

            if( bUndo )
            {
                // extra undo actions for changed connector which now may hold its laid out path (SJ)
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );

                AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            // set up a scene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pO))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pO));
            }

            pO->Mirror(rRef1,rRef2);
        }

        // fire scene updaters
        while(!aUpdaters.empty())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }
    }

    if( bUndo )
        EndUndo();
}

void SdrEditView::MirrorMarkedObjHorizontal(sal_Bool bCopy)
{
    Point aCenter(GetMarkedObjRect().Center());
    Point aPt2(aCenter);
    aPt2.Y()++;
    MirrorMarkedObj(aCenter,aPt2,bCopy);
}

void SdrEditView::MirrorMarkedObjVertical(sal_Bool bCopy)
{
    Point aCenter(GetMarkedObjRect().Center());
    Point aPt2(aCenter);
    aPt2.X()++;
    MirrorMarkedObj(aCenter,aPt2,bCopy);
}

long SdrEditView::GetMarkedObjShear() const
{
    bool b1st=true;
    bool bOk=true;
    long nWink=0;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz && bOk; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        long nWink2=pO->GetShearAngle();
        if (b1st) nWink=nWink2;
        else if (nWink2!=nWink) bOk=false;
        b1st=false;
    }
    if (nWink>SDRMAXSHEAR) nWink=SDRMAXSHEAR;
    if (nWink<-SDRMAXSHEAR) nWink=-SDRMAXSHEAR;
    if (!bOk) nWink=0;
    return nWink;
}

void SdrEditView::ShearMarkedObj(const Point& rRef, long nWink, bool bVShear, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_EditShear,aStr);
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    double nTan=tan(nWink*nPi180);
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
        {
            std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
            AddUndoActions( vConnectorUndoActions );
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
        }
        pO->Shear(rRef,nWink,nTan,bVShear);
    }

    if( bUndo )
        EndUndo();
}

void SdrEditView::ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad,
    SdrCrookMode eMode, sal_Bool bVertical, sal_Bool bNoContortion, sal_Bool bRotate, const Rectangle& rMarkRect)
{
    SdrPathObj* pPath=PTR_CAST(SdrPathObj,pO);
    bool bDone = false;

    if(pPath!=NULL && !bNoContortion)
    {
        XPolyPolygon aXPP(pPath->GetPathPoly());
        switch (eMode) {
            case SDRCROOK_ROTATE : CrookRotatePoly (aXPP,rRef,rRad,bVertical);           break;
            case SDRCROOK_SLANT  : CrookSlantPoly  (aXPP,rRef,rRad,bVertical);           break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aXPP,rRef,rRad,bVertical,rMarkRect); break;
        } // switch
        pPath->SetPathPoly(aXPP.getB2DPolyPolygon());
        bDone = true;
    }

    if(!bDone && !pPath && pO->IsPolyObj() && 0L != pO->GetPointCount())
    {
        // for PolyObj's, but NOT for SdrPathObj's, e.g. the measurement object
        sal_uInt32 nPtAnz(pO->GetPointCount());
        XPolygon aXP((sal_uInt16)nPtAnz);
        sal_uInt32 nPtNum;

        for(nPtNum = 0L; nPtNum < nPtAnz; nPtNum++)
        {
            Point aPt(pO->GetPoint(nPtNum));
            aXP[(sal_uInt16)nPtNum]=aPt;
        }

        switch (eMode)
        {
            case SDRCROOK_ROTATE : CrookRotatePoly (aXP,rRef,rRad,bVertical);           break;
            case SDRCROOK_SLANT  : CrookSlantPoly  (aXP,rRef,rRad,bVertical);           break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aXP,rRef,rRad,bVertical,rMarkRect); break;
        }

        for(nPtNum = 0L; nPtNum < nPtAnz; nPtNum++)
        {
            // broadcasting could be optimized here, but for the
            // current two points of the measurement object, it's fine
            pO->SetPoint(aXP[(sal_uInt16)nPtNum],nPtNum);
        }

        bDone = true;
    }

    if(!bDone)
    {
        // for all others or if bNoContortion
        Point aCtr0(pO->GetSnapRect().Center());
        Point aCtr1(aCtr0);
        sal_Bool bRotOk(sal_False);
        double nSin(0.0), nCos(1.0);
        double nWink(0.0);

        if(0 != rRad.X() && 0 != rRad.Y())
        {
            bRotOk = bRotate;

            switch (eMode)
            {
                case SDRCROOK_ROTATE : nWink=CrookRotateXPoint (aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical); bRotOk=bRotate; break;
                case SDRCROOK_SLANT  : nWink=CrookSlantXPoint  (aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical);           break;
                case SDRCROOK_STRETCH: nWink=CrookStretchXPoint(aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical,rMarkRect); break;
            }
        }

        aCtr1 -= aCtr0;

        if(bRotOk)
            pO->Rotate(aCtr0, Round(nWink/nPi180), nSin, nCos);

        pO->Move(Size(aCtr1.X(),aCtr1.Y()));
    }
}

void SdrEditView::CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode,
    bool bVertical, bool bNoContortion, bool bCopy)
{
    Rectangle aMarkRect(GetMarkedObjRect());
    const bool bUndo = IsUndoEnabled();

    bool bRotate=bNoContortion && eMode==SDRCROOK_ROTATE && IsRotateAllowed(sal_False);

    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(bNoContortion?STR_EditCrook:STR_EditCrookContortion,aStr);
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));

        const SdrObjList* pOL=pO->GetSubList();
        if (bNoContortion || pOL==NULL) {
            ImpCrookObj(pO,rRef,rRad,eMode,bVertical,bNoContortion,bRotate,aMarkRect);
        } else {
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            while (aIter.IsMore()) {
                SdrObject* pO1=aIter.Next();
                ImpCrookObj(pO1,rRef,rRad,eMode,bVertical,bNoContortion,bRotate,aMarkRect);
            }
        }
    }

    if( bUndo )
        EndUndo();
}

void SdrEditView::ImpDistortObj(SdrObject* pO, const Rectangle& rRef, const XPolygon& rDistortedRect, sal_Bool bNoContortion)
{
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pO);

    if(!bNoContortion && pPath)
    {
        XPolyPolygon aXPP(pPath->GetPathPoly());
        aXPP.Distort(rRef, rDistortedRect);
        pPath->SetPathPoly(aXPP.getB2DPolyPolygon());
    }
    else if(pO->IsPolyObj())
    {
        // e. g. for the measurement object
        sal_uInt32 nPtAnz(pO->GetPointCount());
        XPolygon aXP((sal_uInt16)nPtAnz);
        sal_uInt32 nPtNum;

        for(nPtNum = 0L; nPtNum < nPtAnz; nPtNum++)
        {
            Point aPt(pO->GetPoint(nPtNum));
            aXP[(sal_uInt16)nPtNum]=aPt;
        }

        aXP.Distort(rRef, rDistortedRect);

        for(nPtNum = 0L; nPtNum < nPtAnz; nPtNum++)
        {
            // broadcasting could be optimized here, but for the
            // current two points of the measurement object it's fine
            pO->SetPoint(aXP[(sal_uInt16)nPtNum],nPtNum);
        }
    }
}

void SdrEditView::DistortMarkedObj(const Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion, bool bCopy)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_EditDistort,aStr);
        if (bCopy)
            aStr+=ImpGetResStr(STR_EditWithCopy);
        BegUndo(aStr);
    }

    if (bCopy)
        CopyMarkedObj();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if( bUndo )
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));

        Rectangle aRefRect(rRef);
        XPolygon  aRefPoly(rDistortedRect);
        const SdrObjList* pOL=pO->GetSubList();
        if (bNoContortion || pOL==NULL) {
            ImpDistortObj(pO,aRefRect,aRefPoly,bNoContortion);
        } else {
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            while (aIter.IsMore()) {
                SdrObject* pO1=aIter.Next();
                ImpDistortObj(pO1,aRefRect,aRefPoly,bNoContortion);
            }
        }
    }
    if( bUndo )
        EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetNotPersistAttrToMarked(const SfxItemSet& rAttr, sal_Bool /*bReplaceAll*/)
{
    // bReplaceAll has no effect here
    Rectangle aAllSnapRect(GetMarkedObjRect());
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef1XItem*)pPoolItem)->GetValue();
        SetRef1(Point(n,GetRef1().Y()));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef1YItem*)pPoolItem)->GetValue();
        SetRef1(Point(GetRef1().X(),n));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef2XItem*)pPoolItem)->GetValue();
        SetRef2(Point(n,GetRef2().Y()));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef2YItem*)pPoolItem)->GetValue();
        SetRef2(Point(GetRef2().X(),n));
    }
    long nAllPosX=0; sal_Bool bAllPosX=false;
    long nAllPosY=0; sal_Bool bAllPosY=false;
    long nAllWdt=0;  sal_Bool bAllWdt=false;
    long nAllHgt=0;  sal_Bool bAllHgt=false;
    bool bDoIt=false;
    if (rAttr.GetItemState(SDRATTR_ALLPOSITIONX,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        nAllPosX=((const SdrAllPositionXItem*)pPoolItem)->GetValue();
        bAllPosX=true; bDoIt=true;
    }
    if (rAttr.GetItemState(SDRATTR_ALLPOSITIONY,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        nAllPosY=((const SdrAllPositionYItem*)pPoolItem)->GetValue();
        bAllPosY=true; bDoIt=true;
    }
    if (rAttr.GetItemState(SDRATTR_ALLSIZEWIDTH,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        nAllWdt=((const SdrAllSizeWidthItem*)pPoolItem)->GetValue();
        bAllWdt=true; bDoIt=true;
    }
    if (rAttr.GetItemState(SDRATTR_ALLSIZEHEIGHT,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        nAllHgt=((const SdrAllSizeHeightItem*)pPoolItem)->GetValue();
        bAllHgt=true; bDoIt=true;
    }
    if (bDoIt) {
        Rectangle aRect(aAllSnapRect); // TODO: change this for PolyPt's and GluePt's!!!
        if (bAllPosX) aRect.Move(nAllPosX-aRect.Left(),0);
        if (bAllPosY) aRect.Move(0,nAllPosY-aRect.Top());
        if (bAllWdt)  aRect.Right()=aAllSnapRect.Left()+nAllWdt;
        if (bAllHgt)  aRect.Bottom()=aAllSnapRect.Top()+nAllHgt;
        SetMarkedObjRect(aRect);
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEXALL,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        Fraction aXFact=((const SdrResizeXAllItem*)pPoolItem)->GetValue();
        ResizeMarkedObj(aAllSnapRect.TopLeft(),aXFact,Fraction(1,1));
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYALL,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        Fraction aYFact=((const SdrResizeYAllItem*)pPoolItem)->GetValue();
        ResizeMarkedObj(aAllSnapRect.TopLeft(),Fraction(1,1),aYFact);
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEALL,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrRotateAllItem*)pPoolItem)->GetValue();
        RotateMarkedObj(aAllSnapRect.Center(),nAngle);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARALL,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrHorzShearAllItem*)pPoolItem)->GetValue();
        ShearMarkedObj(aAllSnapRect.Center(),nAngle,sal_False);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARALL,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrVertShearAllItem*)pPoolItem)->GetValue();
        ShearMarkedObj(aAllSnapRect.Center(),nAngle,sal_True);
    }

    const bool bUndo = IsUndoEnabled();

    // TODO: check if WhichRange is necessary.
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        const SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        if( bUndo )
            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

        pObj->ApplyNotPersistAttr(rAttr);
    }
}

void SdrEditView::MergeNotPersistAttrFromMarked(SfxItemSet& rAttr, sal_Bool /*bOnlyHardAttr*/) const
{
    // bOnlyHardAttr has no effect here.
    // TODO: Take into account the origin and PvPos.
    Rectangle aAllSnapRect(GetMarkedObjRect()); // TODO: change this for PolyPt's and GluePt's!!!
    long nAllSnapPosX=aAllSnapRect.Left();
    long nAllSnapPosY=aAllSnapRect.Top();
    long nAllSnapWdt=aAllSnapRect.GetWidth()-1;
    long nAllSnapHgt=aAllSnapRect.GetHeight()-1;
    // TODO: could go into CheckPossibilities
    bool bMovProtect = false, bMovProtectDC = false;
    bool bSizProtect = false, bSizProtectDC = false;
    bool bPrintable = true, bPrintableDC = false;
    bool bVisible = true, bVisibleDC = false;
    SdrLayerID nLayerId=0; bool bLayerDC=false;
    OUString aObjName;
    sal_Bool bObjNameDC=false,bObjNameSet=false;
    long nSnapPosX=0;      bool bSnapPosXDC=false;
    long nSnapPosY=0;      bool bSnapPosYDC=false;
    long nSnapWdt=0;       bool bSnapWdtDC=false;
    long nSnapHgt=0;       bool bSnapHgtDC=false;
    long nLogicWdt=0;      bool bLogicWdtDC=false,bLogicWdtDiff=false;
    long nLogicHgt=0;      bool bLogicHgtDC=false,bLogicHgtDiff=false;
    long nRotAngle=0;      bool bRotAngleDC=false;
    long nShrAngle=0;      bool bShrAngleDC=false;
    Rectangle aSnapRect;
    Rectangle aLogicRect;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++) {
        const SdrMark* pM=GetSdrMarkByIndex(nm);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        if (nm==0) {
            nLayerId=pObj->GetLayer();
            bMovProtect=pObj->IsMoveProtect();
            bSizProtect=pObj->IsResizeProtect();
            bPrintable =pObj->IsPrintable();
            bVisible = pObj->IsVisible();
            Rectangle aSnapRect2(pObj->GetSnapRect());
            Rectangle aLogicRect2(pObj->GetLogicRect());
            nSnapPosX=aSnapRect2.Left();
            nSnapPosY=aSnapRect2.Top();
            nSnapWdt=aSnapRect2.GetWidth()-1;
            nSnapHgt=aSnapRect2.GetHeight()-1;
            nLogicWdt=aLogicRect2.GetWidth()-1;
            nLogicHgt=aLogicRect2.GetHeight()-1;
            bLogicWdtDiff=nLogicWdt!=nSnapWdt;
            bLogicHgtDiff=nLogicHgt!=nSnapHgt;
            nRotAngle=pObj->GetRotateAngle();
            nShrAngle=pObj->GetShearAngle();
        } else {
            if (!bLayerDC      && nLayerId   !=pObj->GetLayer())        bLayerDC = true;
            if (!bMovProtectDC && bMovProtect!=pObj->IsMoveProtect())   bMovProtectDC = true;
            if (!bSizProtectDC && bSizProtect!=pObj->IsResizeProtect()) bSizProtectDC = true;
            if (!bPrintableDC  && bPrintable !=pObj->IsPrintable())     bPrintableDC = true;
            if (!bVisibleDC    && bVisible !=pObj->IsVisible())         bVisibleDC=true;
            if (!bRotAngleDC   && nRotAngle  !=pObj->GetRotateAngle())  bRotAngleDC=true;
            if (!bShrAngleDC   && nShrAngle  !=pObj->GetShearAngle())   bShrAngleDC=true;
            if (!bSnapWdtDC || !bSnapHgtDC || !bSnapPosXDC || !bSnapPosYDC || !bLogicWdtDiff || !bLogicHgtDiff) {
                aSnapRect=pObj->GetSnapRect();
                if (nSnapPosX!=aSnapRect.Left()) bSnapPosXDC=true;
                if (nSnapPosY!=aSnapRect.Top()) bSnapPosYDC=true;
                if (nSnapWdt!=aSnapRect.GetWidth()-1) bSnapWdtDC=true;
                if (nSnapHgt!=aSnapRect.GetHeight()-1) bSnapHgtDC=true;
            }
            if (!bLogicWdtDC || !bLogicHgtDC || !bLogicWdtDiff || !bLogicHgtDiff) {
                aLogicRect=pObj->GetLogicRect();
                if (nLogicWdt!=aLogicRect.GetWidth()-1) bLogicWdtDC=true;
                if (nLogicHgt!=aLogicRect.GetHeight()-1) bLogicHgtDC=true;
                if (!bLogicWdtDiff && aSnapRect.GetWidth()!=aLogicRect.GetWidth()) bLogicWdtDiff=true;
                if (!bLogicHgtDiff && aSnapRect.GetHeight()!=aLogicRect.GetHeight()) bLogicHgtDiff=true;
            }
        }
        if (!bObjNameDC ) {
            if (!bObjNameSet) {
                aObjName=pObj->GetName();
            } else {
                if (!aObjName.equals(pObj->GetName()))
                    bObjNameDC = true;
            }
        }
    }

    if (bSnapPosXDC || nAllSnapPosX!=nSnapPosX) rAttr.Put(SdrAllPositionXItem(nAllSnapPosX));
    if (bSnapPosYDC || nAllSnapPosY!=nSnapPosY) rAttr.Put(SdrAllPositionYItem(nAllSnapPosY));
    if (bSnapWdtDC  || nAllSnapWdt !=nSnapWdt ) rAttr.Put(SdrAllSizeWidthItem(nAllSnapWdt));
    if (bSnapHgtDC  || nAllSnapHgt !=nSnapHgt ) rAttr.Put(SdrAllSizeHeightItem(nAllSnapHgt));

    // items for pure transformations
    rAttr.Put(SdrMoveXItem());
    rAttr.Put(SdrMoveYItem());
    rAttr.Put(SdrResizeXOneItem());
    rAttr.Put(SdrResizeYOneItem());
    rAttr.Put(SdrRotateOneItem());
    rAttr.Put(SdrHorzShearOneItem());
    rAttr.Put(SdrVertShearOneItem());

    if (nMarkAnz>1) {
        rAttr.Put(SdrResizeXAllItem());
        rAttr.Put(SdrResizeYAllItem());
        rAttr.Put(SdrRotateAllItem());
        rAttr.Put(SdrHorzShearAllItem());
        rAttr.Put(SdrVertShearAllItem());
    }

    if(eDragMode == SDRDRAG_ROTATE || eDragMode == SDRDRAG_MIRROR)
    {
        rAttr.Put(SdrTransformRef1XItem(GetRef1().X()));
        rAttr.Put(SdrTransformRef1YItem(GetRef1().Y()));
    }

    if(eDragMode == SDRDRAG_MIRROR)
    {
        rAttr.Put(SdrTransformRef2XItem(GetRef2().X()));
        rAttr.Put(SdrTransformRef2YItem(GetRef2().Y()));
    }
}

SfxItemSet SdrEditView::GetAttrFromMarked(sal_Bool bOnlyHardAttr) const
{
    SfxItemSet aSet(pMod->GetItemPool());
    MergeAttrFromMarked(aSet,bOnlyHardAttr);
    //the EE_FEATURE items should not be set with SetAttrToMarked (see error message there)
    //so we do not set them here
    // #i32448#
    // Do not disable, but clear the items.
    aSet.ClearItem(EE_FEATURE_TAB);
    aSet.ClearItem(EE_FEATURE_LINEBR);
    aSet.ClearItem(EE_FEATURE_NOTCONV);
    aSet.ClearItem(EE_FEATURE_FIELD);
    return aSet;
}

void SdrEditView::MergeAttrFromMarked(SfxItemSet& rAttr, sal_Bool bOnlyHardAttr) const
{
    sal_uInt32 nMarkAnz(GetMarkedObjectCount());

    for(sal_uInt32 a(0); a < nMarkAnz; a++)
    {
        // #80277# merging was done wrong in the prev version
        const SfxItemSet& rSet = GetMarkedObjectByIndex(a)->GetMergedItemSet();
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            if(!bOnlyHardAttr)
            {
                if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, sal_False))
                    rAttr.InvalidateItem(nWhich);
                else
                    rAttr.MergeValue(rSet.Get(nWhich), sal_True);
            }
            else if(SFX_ITEM_SET == rSet.GetItemState(nWhich, sal_False))
            {
                const SfxPoolItem& rItem = rSet.Get(nWhich);
                rAttr.MergeValue(rItem, sal_True);
            }

            nWhich = aIter.NextWhich();
        }
    }
}

void SdrEditView::SetAttrToMarked(const SfxItemSet& rAttr, sal_Bool bReplaceAll)
{
    if (AreObjectsMarked())
    {
#ifdef DBG_UTIL
        {
            bool bHasEEFeatureItems=false;
            SfxItemIter aIter(rAttr);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while (!bHasEEFeatureItems && pItem!=NULL) {
                if (!IsInvalidItem(pItem)) {
                    sal_uInt16 nW=pItem->Which();
                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=true;
                }
                pItem=aIter.NextItem();
            }
            if(bHasEEFeatureItems)
            {
                String aMessage;
                aMessage.AppendAscii("SdrEditView::SetAttrToMarked(): Setting EE_FEATURE items at the SdrView does not make sense! It only leads to overhead and unreadable documents.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif

        // #103836# if the user thets character attributes to the complete shape,
        //          we want to remove all hard set character attributes with same
        //          which ids from the text. We do that later but here we remember
        //          all character attribute which id's that are set.
        std::vector<sal_uInt16> aCharWhichIds;
        {
            SfxItemIter aIter(rAttr);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while( pItem!=NULL )
            {
                if (!IsInvalidItem(pItem))
                {
                    sal_uInt16 nWhich = pItem->Which();
                    if (nWhich>=EE_CHAR_START && nWhich<=EE_CHAR_END)
                        aCharWhichIds.push_back( nWhich );
                }
                pItem=aIter.NextItem();
            }
        }

        // To make Undo reconstruct text attributes correctly after Format.Standard
        sal_Bool bHasEEItems=SearchOutlinerItems(rAttr,bReplaceAll);

        // save additional geometry information when paragraph or character attributes
        // are changed and the geometrical shape of the text object might be changed
        bool bPossibleGeomChange(false);
        SfxWhichIter aIter(rAttr);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while(!bPossibleGeomChange && nWhich)
        {
            SfxItemState eState = rAttr.GetItemState(nWhich);
            if(eState == SFX_ITEM_SET)
            {
                if((nWhich >= SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich <= SDRATTR_TEXT_CONTOURFRAME)
                    || nWhich == SDRATTR_3DOBJ_PERCENT_DIAGONAL
                    || nWhich == SDRATTR_3DOBJ_BACKSCALE
                    || nWhich == SDRATTR_3DOBJ_DEPTH
                    || nWhich == SDRATTR_3DOBJ_END_ANGLE
                    || nWhich == SDRATTR_3DSCENE_DISTANCE)
                {
                    bPossibleGeomChange = true;
                }
            }
            nWhich = aIter.NextWhich();
        }

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            OUString aStr;
            ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);
            BegUndo(aStr);
        }

        const sal_uInt32 nMarkAnz(GetMarkedObjectCount());
        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

        // create ItemSet without SFX_ITEM_DONTCARE. Put()
        // uses its second parameter (bInvalidAsDefault) to
        // remove all such items to set them to default.
        SfxItemSet aAttr(*rAttr.GetPool(), rAttr.GetRanges());
        aAttr.Put(rAttr, sal_True);

        // #i38135#
        bool bResetAnimationTimer(false);

        // check if LineWidth is part of the change
        const bool bLineWidthChange(SFX_ITEM_SET == aAttr.GetItemState(XATTR_LINEWIDTH));
        sal_Int32 nNewLineWidth(0);
        sal_Int32 nOldLineWidth(0);

        if(bLineWidthChange)
        {
            nNewLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();
        }

        for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj = pM->GetMarkedSdrObj();

            if( bUndo )
            {
                std::vector< SdrUndoAction* > vConnectorUndoActions;
                SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >( pObj );
                if ( pEdgeObj )
                    bPossibleGeomChange = true;
                else if( bUndo )
                    vConnectorUndoActions = CreateConnectorUndo( *pObj );

                AddUndoActions( vConnectorUndoActions );
            }

            // new geometry undo
            if(bPossibleGeomChange && bUndo)
            {
                // save position and size of object, too
                AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
            }

            if( bUndo )
            {
                // #i8508#
                // If this is a text object also rescue the OutlinerParaObject since
                // applying attributes to the object may change text layout when
                // multiple portions exist with multiple formats. If a OutlinerParaObject
                // really exists and needs to be rescued is evaluated in the undo
                // implementation itself.
                const bool bRescueText = dynamic_cast< SdrTextObj* >(pObj) != 0;

                // add attribute undo
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj,sal_False,bHasEEItems || bPossibleGeomChange || bRescueText));
            }

            // set up a scene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pObj))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pObj));
            }

            if(bLineWidthChange)
            {
                nOldLineWidth = ((const XLineWidthItem&)pObj->GetMergedItem(XATTR_LINEWIDTH)).GetValue();
            }

            // set attributes at object
            pObj->SetMergedItemSetAndBroadcast(aAttr, bReplaceAll);

            if(bLineWidthChange)
            {
                const SfxItemSet& rSet = pObj->GetMergedItemSet();

                if(nOldLineWidth != nNewLineWidth)
                {
                    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINESTARTWIDTH))
                    {
                        const sal_Int32 nValAct(((const XLineStartWidthItem&)rSet.Get(XATTR_LINESTARTWIDTH)).GetValue());
                        const sal_Int32 nValNewStart(std::max((sal_Int32)0, nValAct + (((nNewLineWidth - nOldLineWidth) * 15) / 10)));

                        pObj->SetMergedItem(XLineStartWidthItem(nValNewStart));
                    }

                    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINEENDWIDTH))
                    {
                        const sal_Int32 nValAct(((const XLineEndWidthItem&)rSet.Get(XATTR_LINEENDWIDTH)).GetValue());
                        const sal_Int32 nValNewEnd(std::max((sal_Int32)0, nValAct + (((nNewLineWidth - nOldLineWidth) * 15) / 10)));

                        pObj->SetMergedItem(XLineEndWidthItem(nValNewEnd));
                    }
                }
            }

            if(pObj->ISA(SdrTextObj))
            {
                SdrTextObj* pTextObj = ((SdrTextObj*)pObj);

                if(!aCharWhichIds.empty())
                {
                    Rectangle aOldBoundRect = pTextObj->GetLastBoundRect();

                    // #110094#-14 pTextObj->SendRepaintBroadcast(pTextObj->GetBoundRect());
                    pTextObj->RemoveOutlinerCharacterAttribs( aCharWhichIds );

                    // object has changed, should be called from
                    // RemoveOutlinerCharacterAttribs. This will change when the text
                    // object implementation changes.
                    pTextObj->SetChanged();

                    pTextObj->BroadcastObjectChange();
                    pTextObj->SendUserCall(SDRUSERCALL_CHGATTR, aOldBoundRect);
                }
            }

            // #i38495#
            if(!bResetAnimationTimer)
            {
                if(pObj->GetViewContact().isAnimatedInAnyViewObjectContact())
                {
                    bResetAnimationTimer = true;
                }
            }
        }

        // fire scene updaters
        while(!aUpdaters.empty())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }

        // #i38135#
        if(bResetAnimationTimer)
        {
            SetAnimationTimer(0L);
        }

        // better check before what to do:
        // pObj->SetAttr() or SetNotPersistAttr()
        // TODO: missing implementation!
        SetNotPersistAttrToMarked(rAttr,bReplaceAll);

        if( bUndo )
            EndUndo();
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheetFromMarked() const
{
    SfxStyleSheet* pRet=NULL;
    bool b1st=true;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SfxStyleSheet* pSS=pM->GetMarkedSdrObj()->GetStyleSheet();
        if (b1st) pRet=pSS;
        else if (pRet!=pSS) return NULL; // different stylesheets
        b1st=false;
    }
    return pRet;
}

void SdrEditView::SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    if (AreObjectsMarked())
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
        {
            OUString aStr;
            if (pStyleSheet!=NULL)
                ImpTakeDescriptionStr(STR_EditSetStylesheet,aStr);
            else
                ImpTakeDescriptionStr(STR_EditDelStylesheet,aStr);
            BegUndo(aStr);
        }

        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            if( bUndo )
            {
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pM->GetMarkedSdrObj()));
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pM->GetMarkedSdrObj(),true,true));
            }
            pM->GetMarkedSdrObj()->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        }

        if( bUndo )
            EndUndo();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrEditView::GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const
{
    if(GetMarkedObjectCount())
    {
        rTargetSet.Put(GetAttrFromMarked(bOnlyHardAttr), sal_False);
        return sal_True;
    }
    else
    {
        return SdrMarkView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

sal_Bool SdrEditView::SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll)
{
    if (GetMarkedObjectCount()!=0) {
        SetAttrToMarked(rSet,bReplaceAll);
        return sal_True;
    } else {
        return SdrMarkView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheet() const
{
    if (GetMarkedObjectCount()!=0) {
        return GetStyleSheetFromMarked();
    } else {
        return SdrMarkView::GetStyleSheet();
    }
}

sal_Bool SdrEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    if (GetMarkedObjectCount()!=0) {
        SetStyleSheetToMarked(pStyleSheet,bDontRemoveHardAttr);
        return sal_True;
    } else {
        return SdrMarkView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SfxItemSet SdrEditView::GetGeoAttrFromMarked() const
{
    SfxItemSet aRetSet(pMod->GetItemPool(),   // SID_ATTR_TRANSFORM_... from s:svxids.hrc
        SID_ATTR_TRANSFORM_POS_X,               SID_ATTR_TRANSFORM_ANGLE,
        SID_ATTR_TRANSFORM_PROTECT_POS,         SID_ATTR_TRANSFORM_AUTOHEIGHT,
        SDRATTR_ECKENRADIUS,                    SDRATTR_ECKENRADIUS,
        0);

    if (AreObjectsMarked())
    {
        SfxItemSet aMarkAttr(GetAttrFromMarked(sal_False)); // because of AutoGrowHeight and corner radius
        Rectangle aRect(GetMarkedObjRect());
        // restore position to that before calc hack
        aRect -= GetGridOffset();

        if(GetSdrPageView())
        {
            GetSdrPageView()->LogicToPagePos(aRect);
        }

        // position
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_X,aRect.Left()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_Y,aRect.Top()));

        // size
        long nResizeRefX=aRect.Left();
        long nResizeRefY=aRect.Top();
        if (eDragMode==SDRDRAG_ROTATE) { // use rotation axis as a reference for resizing, too
            nResizeRefX=aRef1.X();
            nResizeRefY=aRef1.Y();
        }
        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_WIDTH,aRect.Right()-aRect.Left()));
        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_HEIGHT,aRect.Bottom()-aRect.Top()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_X,nResizeRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_Y,nResizeRefY));

        Point aRotateAxe(aRef1);

        if(GetSdrPageView())
        {
            GetSdrPageView()->LogicToPagePos(aRotateAxe);
        }

        // rotation
        long nRotateRefX=aRect.Center().X();
        long nRotateRefY=aRect.Center().Y();
        if (eDragMode==SDRDRAG_ROTATE) {
            nRotateRefX=aRotateAxe.X();
            nRotateRefY=aRotateAxe.Y();
        }
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ANGLE,GetMarkedObjRotate()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_X,nRotateRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_Y,nRotateRefY));

        // shearing
        long nShearRefX=aRect.Left();
        long nShearRefY=aRect.Bottom();
        if (eDragMode==SDRDRAG_ROTATE) { // use rotation axis as a reference for shearing, too
            nShearRefX=aRotateAxe.X();
            nShearRefY=aRotateAxe.Y();
        }
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR,GetMarkedObjShear()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X,nShearRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y,nShearRefY));

        // check every object whether it is protected
        const SdrMarkList& rMarkList=GetMarkedObjectList();
        sal_uIntPtr nMarkCount=rMarkList.GetMarkCount();
        SdrObject* pObj=rMarkList.GetMark(0)->GetMarkedSdrObj();
        bool bPosProt=pObj->IsMoveProtect();
        bool bSizProt=pObj->IsResizeProtect();
        bool bPosProtDontCare=false;
        bool bSizProtDontCare=false;
        for (sal_uIntPtr i=1; i<nMarkCount && (!bPosProtDontCare || !bSizProtDontCare); i++)
        {
            pObj=rMarkList.GetMark(i)->GetMarkedSdrObj();
            if (bPosProt!=pObj->IsMoveProtect()) bPosProtDontCare=true;
            if (bSizProt!=pObj->IsResizeProtect()) bSizProtDontCare=true;
        }

        // InvalidateItem sets item to DONT_CARE
        if (bPosProtDontCare) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_POS);
        } else {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_POS,bPosProt));
        }
        if (bSizProtDontCare) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_SIZE);
        } else {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_SIZE,bSizProt));
        }

        SfxItemState eState=aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWWIDTH);
        sal_Bool bAutoGrow=((SdrTextAutoGrowWidthItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOWIDTH);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOWIDTH,bAutoGrow));
        }

        eState=aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWHEIGHT);
        bAutoGrow=((SdrTextAutoGrowHeightItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOHEIGHT);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOHEIGHT,bAutoGrow));
        }

        eState=aMarkAttr.GetItemState(SDRATTR_ECKENRADIUS);
        long nRadius=((SdrEckenradiusItem&)(aMarkAttr.Get(SDRATTR_ECKENRADIUS))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SDRATTR_ECKENRADIUS);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SdrEckenradiusItem(nRadius));
        }

        basegfx::B2DHomMatrix aTransformation;

        if(nMarkCount > 1)
        {
            // multiple objects, range is collected in aRect
            aTransformation = basegfx::tools::createScaleTranslateB2DHomMatrix(
                aRect.Left(), aRect.Top(),
                aRect.getWidth(), aRect.getHeight());
        }
        else
        {
            // single object, get homogen transformation
            basegfx::B2DPolyPolygon aPolyPolygon;

            pObj->TRGetBaseGeometry(aTransformation, aPolyPolygon);
        }

        if(aTransformation.isIdentity())
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_MATRIX);
        }
        else
        {
            com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
            Point aPageOffset(0, 0);

            if(GetSdrPageView())
            {
                aPageOffset = GetSdrPageView()->GetPageOrigin();
            }

            aAffineMatrix2D.m00 = aTransformation.get(0, 0);
            aAffineMatrix2D.m01 = aTransformation.get(0, 1);
            aAffineMatrix2D.m02 = aTransformation.get(0, 2) - aPageOffset.X();
            aAffineMatrix2D.m10 = aTransformation.get(1, 0);
            aAffineMatrix2D.m11 = aTransformation.get(1, 1);
            aAffineMatrix2D.m12 = aTransformation.get(1, 2) - aPageOffset.Y();

            aRetSet.Put(AffineMatrixItem(&aAffineMatrix2D));
        }
    }

    return aRetSet;
}

Point ImpGetPoint(Rectangle aRect, RECT_POINT eRP)
{
    switch(eRP) {
        case RP_LT: return aRect.TopLeft();
        case RP_MT: return aRect.TopCenter();
        case RP_RT: return aRect.TopRight();
        case RP_LM: return aRect.LeftCenter();
        case RP_MM: return aRect.Center();
        case RP_RM: return aRect.RightCenter();
        case RP_LB: return aRect.BottomLeft();
        case RP_MB: return aRect.BottomCenter();
        case RP_RB: return aRect.BottomRight();
    }
    return Point(); // Should not happen!
}

void SdrEditView::SetGeoAttrToMarked(const SfxItemSet& rAttr)
{
    Rectangle aRect(GetMarkedObjRect());

    if(GetSdrPageView())
    {
        GetSdrPageView()->LogicToPagePos(aRect);
    }

    long nOldRotateAngle=GetMarkedObjRotate();
    long nOldShearAngle=GetMarkedObjShear();
    const SdrMarkList& rMarkList=GetMarkedObjectList();
    sal_uIntPtr nMarkCount=rMarkList.GetMarkCount();
    SdrObject* pObj=NULL;

    RECT_POINT eSizePoint=RP_MM;
    long nPosDX=0;
    long nPosDY=0;
    long nSizX=0;
    long nSizY=0;
    long nRotateAngle=0;

    bool bModeIsRotate(eDragMode == SDRDRAG_ROTATE);
    long nRotateX(0);
    long nRotateY(0);
    long nOldRotateX(0);
    long nOldRotateY(0);
    if(bModeIsRotate)
    {
        Point aRotateAxe(aRef1);

        if(GetSdrPageView())
        {
            GetSdrPageView()->LogicToPagePos(aRotateAxe);
        }

        nRotateX = nOldRotateX = aRotateAxe.X();
        nRotateY = nOldRotateY = aRotateAxe.Y();
    }

    long nShearAngle=0;
    long nShearX=0;
    long nShearY=0;
    sal_Bool bShearVert=sal_False;

    bool bChgPos=false;
    bool bChgSiz=false;
    bool bChgWdh=false;
    bool bChgHgt=false;
    bool bRotate=false;
    bool bShear =false;

    bool bSetAttr=false;
    SfxItemSet aSetAttr(pMod->GetItemPool());

    const SfxPoolItem* pPoolItem=NULL;

    // position
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_X,sal_True,&pPoolItem)) {
        nPosDX=((const SfxInt32Item*)pPoolItem)->GetValue()-aRect.Left();
        bChgPos=true;
    }
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_Y,sal_True,&pPoolItem)){
        nPosDY=((const SfxInt32Item*)pPoolItem)->GetValue()-aRect.Top();
        bChgPos=true;
    }
    // size
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_WIDTH,sal_True,&pPoolItem)) {
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
        bChgSiz=true;
        bChgWdh=true;
    }
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,sal_True,&pPoolItem)) {
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();
        bChgSiz=true;
        bChgHgt=true;
    }
    if (bChgSiz) {
        eSizePoint=(RECT_POINT)((const SfxAllEnumItem&)rAttr.Get(SID_ATTR_TRANSFORM_SIZE_POINT)).GetValue();
    }

    // rotation
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ANGLE,sal_True,&pPoolItem)) {
        nRotateAngle=((const SfxInt32Item*)pPoolItem)->GetValue()-nOldRotateAngle;
        bRotate = (nRotateAngle != 0);
    }

    // position rotation point x
    if(bRotate || SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ROT_X, sal_True ,&pPoolItem))
        nRotateX = ((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_X)).GetValue();

    // position rotation point y
    if(bRotate || SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ROT_Y, sal_True ,&pPoolItem))
        nRotateY = ((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_Y)).GetValue();

    // shearing
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_SHEAR,sal_True,&pPoolItem)) {
        long nNewShearAngle=((const SfxInt32Item*)pPoolItem)->GetValue();
        if (nNewShearAngle>SDRMAXSHEAR) nNewShearAngle=SDRMAXSHEAR;
        if (nNewShearAngle<-SDRMAXSHEAR) nNewShearAngle=-SDRMAXSHEAR;
        if (nNewShearAngle!=nOldShearAngle) {
            bShearVert=((const SfxBoolItem&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_VERTICAL)).GetValue();
            if (bShearVert) {
                nShearAngle=nNewShearAngle;
            } else {
                if (nNewShearAngle!=0 && nOldShearAngle!=0) {
                    // bug fix
                    double nOld=tan((double)nOldShearAngle*nPi180);
                    double nNew=tan((double)nNewShearAngle*nPi180);
                    nNew-=nOld;
                    nNew=atan(nNew)/nPi180;
                    nShearAngle=Round(nNew);
                } else {
                    nShearAngle=nNewShearAngle-nOldShearAngle;
                }
            }
            bShear=nShearAngle!=0;
            if (bShear) {
                nShearX=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_X)).GetValue();
                nShearY=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_Y)).GetValue();
            }
        }
    }

    // AutoGrow
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOWIDTH,sal_True,&pPoolItem)) {
        sal_Bool bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrTextAutoGrowWidthItem(bAutoGrow));
        bSetAttr=true;
    }

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOHEIGHT,sal_True,&pPoolItem)) {
        sal_Bool bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrTextAutoGrowHeightItem(bAutoGrow));
        bSetAttr=true;
    }

    // corner radius
    if (bEdgeRadiusAllowed && SFX_ITEM_SET==rAttr.GetItemState(SDRATTR_ECKENRADIUS,sal_True,&pPoolItem)) {
        long nRadius=((SdrEckenradiusItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrEckenradiusItem(nRadius));
        bSetAttr=true;
    }

    ForcePossibilities();

    BegUndo(ImpGetResStr(STR_EditTransform),GetDescriptionOfMarkedObjects());

    if (bSetAttr) {
        SetAttrToMarked(aSetAttr,sal_False);
    }

    // change size and height
    if (bChgSiz && (bResizeFreeAllowed || bResizePropAllowed)) {
        Fraction aWdt(nSizX,aRect.Right()-aRect.Left());
        Fraction aHgt(nSizY,aRect.Bottom()-aRect.Top());
        Point aRef(ImpGetPoint(aRect,eSizePoint));

        if(GetSdrPageView())
        {
            GetSdrPageView()->PagePosToLogic(aRef);
        }

        ResizeMultMarkedObj(aRef, aWdt, aHgt, false, bChgWdh, bChgHgt);
    }

    // rotate
    if (bRotate && (bRotateFreeAllowed || bRotate90Allowed)) {
        Point aRef(nRotateX,nRotateY);

        if(GetSdrPageView())
        {
            GetSdrPageView()->PagePosToLogic(aRef);
        }

        RotateMarkedObj(aRef,nRotateAngle);
    }

    // set rotation point position
    if(bModeIsRotate && (nRotateX != nOldRotateX || nRotateY != nOldRotateY))
    {
        Point aNewRef1(nRotateX, nRotateY);

        if(GetSdrPageView())
        {
            GetSdrPageView()->PagePosToLogic(aNewRef1);
        }

        SetRef1(aNewRef1);
    }

    // shear
    if (bShear && bShearAllowed) {
        Point aRef(nShearX,nShearY);

        if(GetSdrPageView())
        {
            GetSdrPageView()->PagePosToLogic(aRef);
        }

        ShearMarkedObj(aRef,nShearAngle,bShearVert);

        // #i74358#
        // ShearMarkedObj creates a linear combination of the existing transformation and
        // the new shear to apply. If the object is already transformed (e.g. rotated) the
        // linear combination will not decompose to the same start values again, but to a
        // new combination. Thus it makes no sense to check if the wanted shear is reached
        // or not. Taking out.
    }

    // change position
    if (bChgPos && bMoveAllowed) {
        MoveMarkedObj(Size(nPosDX,nPosDY));
    }

    // protect position
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_POS, sal_True, &pPoolItem))
    {
        const bool bProtPos(((const SfxBoolItem*)pPoolItem)->GetValue());
        bool bChanged(false);

        for(sal_uInt32 i(0); i < nMarkCount; i++)
        {
            pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

            if(pObj->IsMoveProtect() != bProtPos)
            {
                bChanged = true;
                pObj->SetMoveProtect(bProtPos);

                if(bProtPos)
                {
                    pObj->SetResizeProtect(true);
                }
            }
        }

        if(bChanged)
        {
            bMoveProtect = bProtPos;

            if(bProtPos)
            {
                bResizeProtect = true;
            }

            // #i77187# there is no simple method to get the toolbars updated
            // in the application. The App is listening to selection change and i
            // will use it here (even if not true). It's acceptable since changing
            // this model data is pretty rare and only possible using the F4 dialog
            MarkListHasChanged();
        }
    }

    if(!bMoveProtect)
    {
        // protect size
        if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_SIZE, sal_True, &pPoolItem))
        {
            const bool bProtSize(((const SfxBoolItem*)pPoolItem)->GetValue());
            bool bChanged(false);

            for(sal_uInt32 i(0); i < nMarkCount; i++)
            {
                pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                if(pObj->IsResizeProtect() != bProtSize)
                {
                    bChanged = true;
                    pObj->SetResizeProtect(bProtSize);
                }
            }

            if(bChanged)
            {
                bResizeProtect = bProtSize;

                // #i77187# see above
                MarkListHasChanged();
            }
        }
    }

    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrEditView::IsAlignPossible() const
{  // at least two selected objects, at least one of them movable
    ForcePossibilities();
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz==0) return sal_False;         // nothing selected!
    if (nAnz==1) return bMoveAllowed;  // align single object to page
    return bOneOrMoreMovable;          // otherwise: MarkCount>=2
}

void SdrEditView::AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert, sal_Bool bBoundRects)
{
    if (eHor==SDRHALIGN_NONE && eVert==SDRVALIGN_NONE)
        return;

    SortMarkedObjects();
    if (GetMarkedObjectCount()<1)
        return;

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
    {
        OUString aStr(GetDescriptionOfMarkedObjects());
        if (eHor==SDRHALIGN_NONE)
        {
            switch (eVert)
            {
                case SDRVALIGN_TOP   : ImpTakeDescriptionStr(STR_EditAlignVTop   ,aStr); break;
                case SDRVALIGN_BOTTOM: ImpTakeDescriptionStr(STR_EditAlignVBottom,aStr); break;
                case SDRVALIGN_CENTER: ImpTakeDescriptionStr(STR_EditAlignVCenter,aStr); break;
                default: break;
            }
        }
        else if (eVert==SDRVALIGN_NONE)
        {
            switch (eHor)
            {
                case SDRHALIGN_LEFT  : ImpTakeDescriptionStr(STR_EditAlignHLeft  ,aStr); break;
                case SDRHALIGN_RIGHT : ImpTakeDescriptionStr(STR_EditAlignHRight ,aStr); break;
                case SDRHALIGN_CENTER: ImpTakeDescriptionStr(STR_EditAlignHCenter,aStr); break;
                default: break;
            }
        }
        else if (eHor==SDRHALIGN_CENTER && eVert==SDRVALIGN_CENTER)
        {
            ImpTakeDescriptionStr(STR_EditAlignCenter,aStr);
        }
        else
        {
            ImpTakeDescriptionStr(STR_EditAlign,aStr);
        }
        BegUndo(aStr);
    }

    Rectangle aBound;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    sal_uIntPtr nm;
    bool bHasFixed=false;
    for (nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (!aInfo.bMoveAllowed || pObj->IsMoveProtect())
        {
            Rectangle aObjRect(bBoundRects?pObj->GetCurrentBoundRect():pObj->GetSnapRect());
            aBound.Union(aObjRect);
            bHasFixed=true;
        }
    }
    if (!bHasFixed)
    {
        if (nMarkAnz==1)
        {   // align single object to page
            const SdrObject* pObj=GetMarkedObjectByIndex(0L);
            const SdrPage* pPage=pObj->GetPage();
            const SdrPageGridFrameList* pGFL=pPage->GetGridFrameList(GetSdrPageViewOfMarkedByIndex(0),&(pObj->GetSnapRect()));
            const SdrPageGridFrame* pFrame=NULL;
            if (pGFL!=NULL && pGFL->GetCount()!=0)
            { // Writer
                pFrame=&((*pGFL)[0]);
            }

            if (pFrame!=NULL)
            { // Writer
                aBound=pFrame->GetUserArea();
            }
            else
            {
                aBound=Rectangle(pPage->GetLftBorder(),pPage->GetUppBorder(),
                                 pPage->GetWdt()-pPage->GetRgtBorder(),
                                 pPage->GetHgt()-pPage->GetLwrBorder());
            }
        }
        else
        {
            if (bBoundRects)
                aBound=GetMarkedObjBoundRect();
            else
                aBound=GetMarkedObjRect();
        }
    }
    Point aCenter(aBound.Center());
    for (nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (aInfo.bMoveAllowed && !pObj->IsMoveProtect())
        {
            long nXMov=0;
            long nYMov=0;
            Rectangle aObjRect(bBoundRects?pObj->GetCurrentBoundRect():pObj->GetSnapRect());
            switch (eVert)
            {
                case SDRVALIGN_TOP   : nYMov=aBound.Top()   -aObjRect.Top()       ; break;
                case SDRVALIGN_BOTTOM: nYMov=aBound.Bottom()-aObjRect.Bottom()    ; break;
                case SDRVALIGN_CENTER: nYMov=aCenter.Y()    -aObjRect.Center().Y(); break;
                default: break;
            }
            switch (eHor)
            {
                case SDRHALIGN_LEFT  : nXMov=aBound.Left()  -aObjRect.Left()      ; break;
                case SDRHALIGN_RIGHT : nXMov=aBound.Right() -aObjRect.Right()     ; break;
                case SDRHALIGN_CENTER: nXMov=aCenter.X()    -aObjRect.Center().X(); break;
                default: break;
            }
            if (nXMov!=0 || nYMov!=0)
            {
                // SdrEdgeObj needs an extra SdrUndoGeoObj since the
                // connections may need to be saved
                if( bUndo )
                {
                    if( dynamic_cast<SdrEdgeObj*>(pObj) )
                    {
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                    }

                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*pObj,Size(nXMov,nYMov)));
                }

                pObj->Move(Size(nXMov,nYMov));
            }
        }
    }

    if( bUndo )
        EndUndo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
