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


#include <vcl/wrkwin.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include "svx/svditer.hxx"
#include <svx/svdpool.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/globl3d.hxx"
#include <svx/obj3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/polysc3d.hxx>
#include "dragmt3d.hxx"
#include <svx/view3d.hxx>
#include <svx/svdundo.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdograf.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

using namespace com::sun::star;

#define ITEMVALUE(ItemSet,Id,Cast)  (static_cast<const Cast&>((ItemSet).Get(Id))).GetValue()

TYPEINIT1(E3dView, SdrView);


// Migrate Marking

class Impl3DMirrorConstructOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // the view
    const E3dView&                                  mrView;

    // the object count
    size_t mnCount;

    // the unmirrored polygons
    basegfx::B2DPolyPolygon*                        mpPolygons;

    // the overlay geometry from selected objects
    drawinglayer::primitive2d::Primitive2DSequence  maFullOverlay;

    // Copy assignment is forbidden and not implemented.
    Impl3DMirrorConstructOverlay (const Impl3DMirrorConstructOverlay &) SAL_DELETED_FUNCTION;
    Impl3DMirrorConstructOverlay & operator= (const Impl3DMirrorConstructOverlay &) SAL_DELETED_FUNCTION;

public:
    explicit Impl3DMirrorConstructOverlay(const E3dView& rView);
    ~Impl3DMirrorConstructOverlay();

    void SetMirrorAxis(Point aMirrorAxisA, Point aMirrorAxisB);
};

Impl3DMirrorConstructOverlay::Impl3DMirrorConstructOverlay(const E3dView& rView)
:   maObjects(),
    mrView(rView),
    mnCount(rView.GetMarkedObjectCount()),
    mpPolygons(0),
    maFullOverlay()
{
    if(mnCount)
    {
        if(mrView.IsSolidDragging())
        {
            SdrPageView* pPV = rView.GetSdrPageView();

            if(pPV && pPV->PageWindowCount())
            {
                sdr::contact::ObjectContact& rOC = pPV->GetPageWindow(0)->GetObjectContact();
                sdr::contact::DisplayInfo aDisplayInfo;

                // Do not use the last ViewPort set at the OC at the last ProcessDisplay()
                rOC.resetViewPort();

                for(size_t a = 0; a < mnCount; ++a)
                {
                    SdrObject* pObject = mrView.GetMarkedObjectByIndex(a);

                    if(pObject)
                    {
                        sdr::contact::ViewContact& rVC = pObject->GetViewContact();
                        sdr::contact::ViewObjectContact& rVOC = rVC.GetViewObjectContact(rOC);

                        const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(rVOC.getPrimitive2DSequenceHierarchy(aDisplayInfo));
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(maFullOverlay, aNewSequence);
                    }
                }
            }
        }
        else
        {
            mpPolygons = new basegfx::B2DPolyPolygon[mnCount];

            for(size_t a = 0; a < mnCount; ++a)
            {
                SdrObject* pObject = mrView.GetMarkedObjectByIndex(a);
                mpPolygons[mnCount - (a + 1)] = pObject->TakeXorPoly();
            }
        }
    }
}

Impl3DMirrorConstructOverlay::~Impl3DMirrorConstructOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
    if(!mrView.IsSolidDragging())
    {
        delete[] mpPolygons;
    }
}

void Impl3DMirrorConstructOverlay::SetMirrorAxis(Point aMirrorAxisA, Point aMirrorAxisB)
{
    // get rid of old overlay objects
    maObjects.clear();

    // create new ones
    for(sal_uInt32 a(0); a < mrView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = mrView.GetPaintWindow(a);
        rtl::Reference< sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if(xTargetOverlay.is())
        {
            // build transfoprmation: translate and rotate so that given edge is
            // on x axis, them mirror in y and translate back
            const basegfx::B2DVector aEdge(aMirrorAxisB.X() - aMirrorAxisA.X(), aMirrorAxisB.Y() - aMirrorAxisA.Y());
            basegfx::B2DHomMatrix aMatrixTransform(basegfx::tools::createTranslateB2DHomMatrix(
                -aMirrorAxisA.X(), -aMirrorAxisA.Y()));
            aMatrixTransform.rotate(-atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.scale(1.0, -1.0);
            aMatrixTransform.rotate(atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.translate(aMirrorAxisA.X(), aMirrorAxisA.Y());

            if(mrView.IsSolidDragging())
            {
                if(maFullOverlay.hasElements())
                {
                    drawinglayer::primitive2d::Primitive2DSequence aContent(maFullOverlay);

                    if(!aMatrixTransform.isIdentity())
                    {
                        // embed in transformation group
                        drawinglayer::primitive2d::Primitive2DReference aTransformPrimitive2D(new drawinglayer::primitive2d::TransformPrimitive2D(aMatrixTransform, aContent));
                        aContent = drawinglayer::primitive2d::Primitive2DSequence(&aTransformPrimitive2D, 1);
                    }

                    // if we have full overlay from selected objects, embed with 50% transparence, the
                    // transformation is added to the OverlayPrimitive2DSequenceObject
                    drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparencePrimitive2D(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aContent, 0.5));
                    aContent = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparencePrimitive2D, 1);

                    sdr::overlay::OverlayPrimitive2DSequenceObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aContent);

                    xTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
                }
            }
            else
            {
                for(size_t b = 0; b < mnCount; ++b)
                {
                    // apply to polygon
                    basegfx::B2DPolyPolygon aPolyPolygon(mpPolygons[b]);
                    aPolyPolygon.transform(aMatrixTransform);

                    sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                        aPolyPolygon);
                    xTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
                }
            }
        }
    }
}

E3dView::E3dView(SdrModel* pModel, OutputDevice* pOut) :
    SdrView(pModel, pOut)
{
    InitView ();
}

// DrawMarkedObj override, since possibly only a single 3D object is to be
// drawn

void E3dView::DrawMarkedObj(OutputDevice& rOut) const
{
    // Does 3D objects exist which scenes are not selected?
    bool bSpecialHandling = false;
    E3dScene *pScene = NULL;

    const size_t nCnt = GetMarkedObjectCount();
    for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
    {
        SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
        if(pObj && pObj->ISA(E3dCompoundObject))
        {
            // related scene
            pScene = static_cast<E3dCompoundObject*>(pObj)->GetScene();
            if(pScene && !IsObjMarked(pScene))
                bSpecialHandling = true;
        }
        // Reset all selection flags
        if(pObj && pObj->ISA(E3dObject))
        {
            pScene = static_cast<E3dObject*>(pObj)->GetScene();
            if(pScene)
                pScene->SetSelected(false);
        }
    }

    if(bSpecialHandling)
    {
        // Set selection flag to "not selected" for scenes related to all 3D
        // objects
        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // relatated scene
                pScene = static_cast<E3dCompoundObject*>(pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(false);
            }
        }

        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(pObj && pObj->ISA(E3dObject))
            {
                // Select object
                E3dObject* p3DObj = static_cast<E3dObject*>(pObj);
                p3DObj->SetSelected(true);
                pScene = p3DObj->GetScene();
            }
        }

        if(pScene)
        {
            // code from parent
            SortMarkedObjects();

            pScene->SetDrawOnlySelected(true);
            pScene->SingleObjectPainter(rOut);
            pScene->SetDrawOnlySelected(false);
        }

        // Reset selection flag
        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(pObj && pObj->ISA(E3dCompoundObject))
            {
                // related scene
                pScene = static_cast<E3dCompoundObject*>(pObj)->GetScene();
                if(pScene)
                    pScene->SetSelected(false);
            }
        }
    }
    else
    {
        // call parent
        SdrExchangeView::DrawMarkedObj(rOut);
    }
}

// override get model, since in some 3D objects an additional scene
// must be pushed in

SdrModel* E3dView::GetMarkedObjModel() const
{
    // Does 3D objects exist which scenes are not selected?
    bool bSpecialHandling(false);
    const size_t nCount(GetMarkedObjectCount());
    E3dScene *pScene = 0;

    for(size_t nObjs = 0; nObjs < nCount; ++nObjs)
    {
        const SdrObject* pObj = GetMarkedObjectByIndex(nObjs);

        if(!bSpecialHandling && pObj && pObj->ISA(E3dCompoundObject))
        {
            // if the object is selected, but it's scene not,
            // we need special handling
            pScene = static_cast<const E3dCompoundObject*>(pObj)->GetScene();

            if(pScene && !IsObjMarked(pScene))
            {
                bSpecialHandling = true;
            }
        }

        if(pObj && pObj->ISA(E3dObject))
        {
            // reset all selection flags at 3D objects
            pScene = static_cast<const E3dObject*>(pObj)->GetScene();

            if(pScene)
            {
                pScene->SetSelected(false);
            }
        }
    }

    if(!bSpecialHandling)
    {
        // call parent
        return SdrView::GetMarkedObjModel();
    }

    SdrModel* pNewModel = 0;
    Rectangle aSelectedSnapRect;

    // set 3d selection flags at all directly selected objects
    // and collect SnapRect of selected objects
    for(size_t nObjs = 0; nObjs < nCount; ++nObjs)
    {
        SdrObject *pObj = GetMarkedObjectByIndex(nObjs);

        if(pObj && pObj->ISA(E3dCompoundObject))
        {
            // mark object, but not scenes
            E3dCompoundObject* p3DObj = static_cast<E3dCompoundObject*>(pObj);
            p3DObj->SetSelected(true);
            aSelectedSnapRect.Union(p3DObj->GetSnapRect());
        }
    }

    // create new mark list which contains all indirectly selected3d
    // scenes as selected objects
    SdrMarkList aOldML(GetMarkedObjectList());
    SdrMarkList aNewML;
    SdrMarkList& rCurrentMarkList = const_cast<E3dView*>(this)->GetMarkedObjectListWriteAccess();
    rCurrentMarkList = aNewML;

    for(size_t nObjs = 0; nObjs < nCount; ++nObjs)
    {
        SdrObject *pObj = aOldML.GetMark(nObjs)->GetMarkedSdrObj();

        if(pObj && pObj->ISA(E3dObject))
        {
            pScene = static_cast<E3dObject*>(pObj)->GetScene();

            if(pScene && !IsObjMarked(pScene) && GetSdrPageView())
            {
                const_cast<E3dView*>(this)->MarkObj(pScene, GetSdrPageView(), false, true);
            }
        }
    }

    // call parent. This will copy all scenes and the selection flags at the 3D objects. So
    // it will be possible to delete all non-selected 3d objects from the cloned 3d scenes
    pNewModel = SdrView::GetMarkedObjModel();

    if(pNewModel)
    {
        for(sal_uInt16 nPg(0); nPg < pNewModel->GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=pNewModel->GetPage(nPg);
            const size_t nObjCount(pSrcPg->GetObjCount());

            for(size_t nOb = 0; nOb < nObjCount; ++nOb)
            {
                const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

                if(pSrcOb->ISA(E3dScene))
                {
                    pScene = const_cast<E3dScene*>(static_cast<const E3dScene*>(pSrcOb));

                    // delete all not intentionally cloned 3d objects
                    pScene->removeAllNonSelectedObjects();

                    // reset select flags and set SnapRect of all selected objects
                    pScene->SetSelected(false);
                    pScene->SetSnapRect(aSelectedSnapRect);
                }
            }
        }
    }

    // restore old selection
    rCurrentMarkList = aOldML;

    return pNewModel;
}

// When pasting objects have to integrated if a scene is inserted, but
// not the scene itself

bool E3dView::Paste(
    const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions,
    const OUString& rSrcShellID, const OUString& rDestShellID )
{
    bool bRetval = false;

    // Get list
    Point aPos(rPos);
    SdrObjList* pDstList = pLst;
    ImpGetPasteObjList(aPos, pDstList);

    if(!pDstList)
        return false;

    // Get owner of the list
    SdrObject* pOwner = pDstList->GetOwnerObj();
    if(pOwner && pOwner->ISA(E3dScene))
    {
        E3dScene* pDstScene = static_cast<E3dScene*>(pOwner);
        BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXCHANGE_PASTE));

        // Copy all objects from E3dScenes and insert them directly
        for(sal_uInt16 nPg(0); nPg < rMod.GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=rMod.GetPage(nPg);
            const size_t nObjCount(pSrcPg->GetObjCount());

            // calculate offset for paste
            Rectangle aR = pSrcPg->GetAllObjBoundRect();
            Point aDist(aPos - aR.Center());

            // Insert sub-objects for scenes
            for(size_t nOb = 0; nOb < nObjCount; ++nOb)
            {
                const SdrObject* pSrcOb = pSrcPg->GetObj(nOb);
                if(pSrcOb->ISA(E3dScene))
                {
                    E3dScene* pSrcScene = const_cast<E3dScene*>(static_cast<const E3dScene*>(pSrcOb));
                    ImpCloneAll3DObjectsToDestScene(pSrcScene, pDstScene, aDist);
                }
            }
        }
        EndUndo();
    }
    else
    {
        // call parent
        bRetval = SdrView::Paste(rMod, rPos, pLst, nOptions, rSrcShellID, rDestShellID);
    }

    return bRetval;
}

// Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
bool E3dView::ImpCloneAll3DObjectsToDestScene(E3dScene* pSrcScene, E3dScene* pDstScene, Point /*aOffset*/)
{
    bool bRetval(false);

    if(pSrcScene && pDstScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCSceneDst = static_cast< sdr::contact::ViewContactOfE3dScene& >(pDstScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3DDst(rVCSceneDst.getViewInformation3D());
        const sdr::contact::ViewContactOfE3dScene& rVCSceneSrc = static_cast< sdr::contact::ViewContactOfE3dScene& >(pSrcScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3DSrc(rVCSceneSrc.getViewInformation3D());

        for(size_t i = 0; i < pSrcScene->GetSubList()->GetObjCount(); ++i)
        {
            E3dCompoundObject* pCompoundObj = dynamic_cast< E3dCompoundObject* >(pSrcScene->GetSubList()->GetObj(i));

            if(pCompoundObj)
            {
                E3dCompoundObject* pNewCompoundObj = dynamic_cast< E3dCompoundObject* >(pCompoundObj->Clone());

                if(pNewCompoundObj)
                {
                    // get dest scene's current range in 3D world coordinates
                    const basegfx::B3DHomMatrix aSceneToWorldTrans(pDstScene->GetFullTransform());
                    basegfx::B3DRange aSceneRange(pDstScene->GetBoundVolume());
                    aSceneRange.transform(aSceneToWorldTrans);

                    // get new object's implied object transformation
                    const basegfx::B3DHomMatrix aNewObjectTrans(pNewCompoundObj->GetTransform());

                    // get new object's range in 3D world coordinates in dest scene
                    // as if it were already added
                    const basegfx::B3DHomMatrix aObjectToWorldTrans(aSceneToWorldTrans * aNewObjectTrans);
                    basegfx::B3DRange aObjectRange(pNewCompoundObj->GetBoundVolume());
                    aObjectRange.transform(aObjectToWorldTrans);

                    // get scale adaption
                    const basegfx::B3DVector aSceneScale(aSceneRange.getRange());
                    const basegfx::B3DVector aObjectScale(aObjectRange.getRange());
                    double fScale(1.0);

                    // if new object's size in X,Y or Z is bigger that 80% of dest scene, adapt scale
                    // to not change the scene by the inserted object
                    const double fSizeFactor(0.5);

                    if(aObjectScale.getX() * fScale > aSceneScale.getX() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getX() * fScale);
                        const double fFactor((aSceneScale.getX() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    if(aObjectScale.getY() * fScale > aSceneScale.getY() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getY() * fScale);
                        const double fFactor((aSceneScale.getY() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    if(aObjectScale.getZ() * fScale > aSceneScale.getZ() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getZ() * fScale);
                        const double fFactor((aSceneScale.getZ() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    // get translation adaption
                    const basegfx::B3DPoint aSceneCenter(aSceneRange.getCenter());
                    const basegfx::B3DPoint aObjectCenter(aObjectRange.getCenter());

                    // build full modification transform. The object's transformation
                    // shall be modified, so start at object coordinates; transform to 3d world coor
                    basegfx::B3DHomMatrix aModifyingTransform(aObjectToWorldTrans);

                    // translate to absolute center in 3d world coor
                    aModifyingTransform.translate(-aObjectCenter.getX(), -aObjectCenter.getY(), -aObjectCenter.getZ());

                    // scale to dest size in 3d world coor
                    aModifyingTransform.scale(fScale, fScale, fScale);

                    // translate to dest scene center in 3d world coor
                    aModifyingTransform.translate(aSceneCenter.getX(), aSceneCenter.getY(), aSceneCenter.getZ());

                    // transform from 3d world to dest object coordinates
                    basegfx::B3DHomMatrix aWorldToObject(aObjectToWorldTrans);
                    aWorldToObject.invert();
                    aModifyingTransform = aWorldToObject * aModifyingTransform;

                    // correct implied object transform by applying changing one in object coor
                    pNewCompoundObj->SetTransform(aModifyingTransform * aNewObjectTrans);

                    // fill and insert new object
                    pNewCompoundObj->SetModel(pDstScene->GetModel());
                    pNewCompoundObj->SetPage(pDstScene->GetPage());
                    pNewCompoundObj->NbcSetLayer(pCompoundObj->GetLayer());
                    pNewCompoundObj->NbcSetStyleSheet(pCompoundObj->GetStyleSheet(), true);
                    pDstScene->Insert3DObj(pNewCompoundObj);
                    bRetval = true;

                    // Create undo
                    if( GetModel()->IsUndoEnabled() )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNewCompoundObj));
                }
            }
        }
    }

    return bRetval;
}

bool E3dView::IsConvertTo3DObjPossible() const
{
    bool bAny3D(false);
    bool bGroupSelected(false);
    bool bRetval(true);

    for(size_t a=0; !bAny3D && a<GetMarkedObjectCount(); ++a)
    {
        SdrObject *pObj = GetMarkedObjectByIndex(a);
        if(pObj)
        {
            ImpIsConvertTo3DPossible(pObj, bAny3D, bGroupSelected);
        }
    }

    bRetval = !bAny3D
        && (
           IsConvertToPolyObjPossible(false)
        || IsConvertToPathObjPossible(false)
        || IsImportMtfPossible());
    return bRetval;
}

void E3dView::ImpIsConvertTo3DPossible(SdrObject* pObj, bool& rAny3D,
    bool& rGroupSelected) const
{
    if(pObj)
    {
        if(pObj->ISA(E3dObject))
        {
            rAny3D = true;
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
                rGroupSelected = true;
            }
        }
    }
}

#include <editeng/eeitem.hxx>

void E3dView::ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj)
{
    if(pObj->ISA(SdrTextObj))
    {
        const SfxItemSet& rSet = pObj->GetMergedItemSet();
        const SvxColorItem& rTextColorItem = static_cast<const SvxColorItem&>(rSet.Get(EE_CHAR_COLOR));
        if(rTextColorItem.GetValue() == RGB_Color(COL_BLACK))
        {
            //For black text objects, the color set to gray
            if(pObj->GetPage())
            {
                // if black is only default attribute from
                // pattern set it hard so that it is used in undo.
                pObj->SetMergedItem(SvxColorItem(RGB_Color(COL_BLACK), EE_CHAR_COLOR));

                // add undo now
                if( GetModel()->IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj, false));
            }

            pObj->SetMergedItem(SvxColorItem(RGB_Color(COL_GRAY), EE_CHAR_COLOR));
        }
    }
}

void E3dView::ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj)
{
    if(pObj->ISA(SdrPathObj))
    {
        const SfxItemSet& rSet = pObj->GetMergedItemSet();
        sal_Int32 nLineWidth = static_cast<const XLineWidthItem&>(rSet.Get(XATTR_LINEWIDTH)).GetValue();
        drawing::LineStyle eLineStyle = (drawing::LineStyle)static_cast<const XLineStyleItem&>(rSet.Get(XATTR_LINESTYLE)).GetValue();
        drawing::FillStyle eFillStyle = ITEMVALUE(rSet, XATTR_FILLSTYLE, XFillStyleItem);

        if(static_cast<SdrPathObj*>(pObj)->IsClosed()
            && eLineStyle == drawing::LineStyle_SOLID
            && !nLineWidth
            && eFillStyle != drawing::FillStyle_NONE)
        {
            if(pObj->GetPage() && GetModel()->IsUndoEnabled() )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj, false));
            pObj->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
            pObj->SetMergedItem(XLineWidthItem(0L));
        }
    }
}

void E3dView::ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat)
{
    // Single PathObject, transform this
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);

    if(pPath)
    {
        E3dDefaultAttributes aDefault = Get3DDefaultAttributes();
        if(bExtrude)
            aDefault.SetDefaultExtrudeCharacterMode(true);
        else
            aDefault.SetDefaultLatheCharacterMode(true);

        // Get Itemset of the original object
        SfxItemSet aSet(pObj->GetMergedItemSet());

        drawing::FillStyle eFillStyle = ITEMVALUE(aSet, XATTR_FILLSTYLE, XFillStyleItem);

        // line style turned off
        aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));

        //Determining if FILL_Attribut is set.
        if(!pPath->IsClosed() || eFillStyle == drawing::FillStyle_NONE)
        {
            // This SdrPathObj is not filled, leave the front and rear face out.
            // Moreover, a two-sided representation necessary.
            aDefault.SetDefaultExtrudeCloseFront(false);
            aDefault.SetDefaultExtrudeCloseBack(false);

            aSet.Put(makeSvx3DDoubleSidedItem(true));

            // Set fill attribute
            aSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));

            // Fill color must be the color line, because the object was
            // previously just a line
            Color aColorLine = static_cast<const XLineColorItem&>(aSet.Get(XATTR_LINECOLOR)).GetColorValue();
            aSet.Put(XFillColorItem(OUString(), aColorLine));
        }

        // Create a new extrude object
        E3dObject* p3DObj = NULL;
        if(bExtrude)
        {
            p3DObj = new E3dExtrudeObj(aDefault, pPath->GetPathPoly(), fDepth);
        }
        else
        {
            basegfx::B2DPolyPolygon aPolyPoly2D(pPath->GetPathPoly());
            aPolyPoly2D.transform(rLatheMat);
            p3DObj = new E3dLatheObj(aDefault, aPolyPoly2D);
        }

        // Set attribute
        if(p3DObj)
        {
            p3DObj->NbcSetLayer(pObj->GetLayer());

            p3DObj->SetMergedItemSet(aSet);

            p3DObj->NbcSetStyleSheet(pObj->GetStyleSheet(), true);

            // Insert a new extrude object
            pScene->Insert3DObj(p3DObj);
        }
    }
}

void E3dView::ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat)
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
        SdrObject* pNewObj1 = pObj->ConvertToPolyObj(false, false);

        if(pNewObj1)
        {
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
            SdrObject* pNewObj2 = pObj->ConvertToContourObj(pNewObj1, true);

            if(pNewObj2)
            {
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

                // delete object in between
                if(pNewObj2 != pObj && pNewObj2 != pNewObj1 && pNewObj2)
                    SdrObject::Free( pNewObj2 );
            }

            // delete object in between
            if(pNewObj1 != pObj && pNewObj1)
                SdrObject::Free( pNewObj1 );
        }
    }
}

void E3dView::ConvertMarkedObjTo3D(bool bExtrude, const basegfx::B2DPoint& rPnt1, const basegfx::B2DPoint& rPnt2)
{
    if(AreObjectsMarked())
    {
        // Create undo
        if(bExtrude)
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXTRUDE));
        else
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_LATHE));

        // Create a new scene for the created 3D object
        E3dScene* pScene = new E3dPolyScene(Get3DDefaultAttributes());

        // Determine rectangle and possibly correct it
        Rectangle aRect = GetAllMarkedRect();
        if(aRect.GetWidth() <= 1)
            aRect.SetSize(Size(500, aRect.GetHeight()));
        if(aRect.GetHeight() <= 1)
            aRect.SetSize(Size(aRect.GetWidth(), 500));

        // Determine the depth relative to the size of the selection
        double fDepth = 0.0;
        double fRot3D = 0.0;
        basegfx::B2DHomMatrix aLatheMat;

        if(bExtrude)
        {
            double fW = (double)aRect.GetWidth();
            double fH = (double)aRect.GetHeight();
            fDepth = sqrt(fW*fW + fH*fH) / 6.0;
        }
        if(!bExtrude)
        {
            // Create transformation for the polygons rotating body
            if (rPnt1 != rPnt2)
            {
                // Rotation around control point #1 with set angle
                // for 3D coordinates
                basegfx::B2DPoint aDiff(rPnt1 - rPnt2);
                fRot3D = atan2(aDiff.getY(), aDiff.getX()) - F_PI2;

                if(basegfx::fTools::equalZero(fabs(fRot3D)))
                    fRot3D = 0.0;

                if(fRot3D != 0.0)
                {
                    aLatheMat = basegfx::tools::createRotateAroundPoint(rPnt2, -fRot3D)
                        * aLatheMat;
                }
            }

            if (rPnt2.getX() != 0.0)
            {
                // Translation to Y=0 - axis
                aLatheMat.translate(-rPnt2.getX(), 0.0);
            }
            else
            {
                aLatheMat.translate((double)-aRect.Left(), 0.0);
            }

            // Form the inverse matrix to determine the target expansion
            basegfx::B2DHomMatrix aInvLatheMat(aLatheMat);
            aInvLatheMat.invert();

            // SnapRect extension enables mirroring in the axis of rotation
            for(size_t a=0; a<GetMarkedObjectCount(); ++a)
            {
                SdrMark* pMark = GetSdrMarkByIndex(a);
                SdrObject* pObj = pMark->GetMarkedSdrObj();
                Rectangle aTurnRect = pObj->GetSnapRect();
                basegfx::B2DPoint aRot;
                Point aRotPnt;

                aRot = basegfx::B2DPoint(aTurnRect.Left(), -aTurnRect.Top());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.getX() + 0.5), (long)(-aRot.getY() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = basegfx::B2DPoint(aTurnRect.Left(), -aTurnRect.Bottom());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.getX() + 0.5), (long)(-aRot.getY() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = basegfx::B2DPoint(aTurnRect.Right(), -aTurnRect.Top());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.getX() + 0.5), (long)(-aRot.getY() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));

                aRot = basegfx::B2DPoint(aTurnRect.Right(), -aTurnRect.Bottom());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRotPnt = Point((long)(aRot.getX() + 0.5), (long)(-aRot.getY() - 0.5));
                aRect.Union(Rectangle(aRotPnt, aRotPnt));
            }
        }

        // Walk through the selection and convert it into 3D, complete with
        // Conversion to SdrPathObject, also fonts
        for(size_t a=0; a<GetMarkedObjectCount(); ++a)
        {
            SdrMark* pMark = GetSdrMarkByIndex(a);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            ImpCreate3DObject(pScene, pObj, bExtrude, fDepth, aLatheMat);
        }

        if(pScene->GetSubList() && pScene->GetSubList()->GetObjCount() != 0)
        {
            // Arrange all created objects by depth
            if(bExtrude)
                DoDepthArrange(pScene, fDepth);

            // Center 3D objects in the middle of the overall rectangle
            basegfx::B3DPoint aCenter(pScene->GetBoundVolume().getCenter());
            basegfx::B3DHomMatrix aMatrix;

            aMatrix.translate(-aCenter.getX(), -aCenter.getY(), -aCenter.getZ());
            pScene->SetTransform(aMatrix * pScene->GetTransform());

            // Initialize scene
            pScene->NbcSetSnapRect(aRect);
            basegfx::B3DRange aBoundVol = pScene->GetBoundVolume();
            InitScene(pScene, (double)aRect.GetWidth(), (double)aRect.GetHeight(), aBoundVol.getDepth());

            // Insert scene instead of the first selected object and throw away
            // all the old objects
            SdrObject* pRepObj = GetMarkedObjectByIndex(0);
            SdrPageView* pPV = GetSdrPageViewOfMarkedByIndex(0);
            MarkObj(pRepObj, pPV, true);
            ReplaceObjectAtView(pRepObj, *pPV, pScene, false);
            DeleteMarked();
            MarkObj(pScene, pPV);

            // Rotate Rotation body around the axis of rotation
            basegfx::B3DHomMatrix aRotate;

            if(!bExtrude && fRot3D != 0.0)
            {
                aRotate.rotate(0.0, 0.0, fRot3D);
            }

            // Set default rotation
            {
                double XRotateDefault = 20;
                aRotate.rotate(DEG2RAD(XRotateDefault), 0.0, 0.0);
            }

            if(!aRotate.isIdentity())
            {
                pScene->SetTransform(aRotate * pScene->GetTransform());
            }

            // Invalid SnapRects of objects
            pScene->SetSnapRect(aRect);
        }
        else
        {
            // No 3D object was created, throw away everything
            delete pScene;
        }

        EndUndo();
    }
}

//Arrange all created extrude objects by depth

struct E3dDepthNeighbour
{
    E3dDepthNeighbour*          mpNext;
    E3dExtrudeObj*              mpObj;
    basegfx::B2DPolyPolygon     maPreparedPolyPolygon;

    E3dDepthNeighbour()
    :   mpNext(0),
        mpObj(0),
        maPreparedPolyPolygon()
    {
    }
};

struct E3dDepthLayer
{
    E3dDepthLayer*              mpDown;
    E3dDepthNeighbour*          mpNext;

    E3dDepthLayer()
    :   mpDown(0),
        mpNext(0)
    {
    }

    ~E3dDepthLayer()
    {
        while(mpNext)
        {
            E3dDepthNeighbour* pSucc = mpNext->mpNext;
            delete mpNext;
            mpNext = pSucc;
        }
    }
};

void E3dView::DoDepthArrange(E3dScene* pScene, double fDepth)
{
    if(pScene && pScene->GetSubList() && pScene->GetSubList()->GetObjCount() > 1)
    {
        SdrObjList* pSubList = pScene->GetSubList();
        SdrObjListIter aIter(*pSubList, IM_FLAT);
        E3dDepthLayer* pBaseLayer = NULL;
        E3dDepthLayer* pLayer = NULL;
        sal_Int32 nNumLayers = 0;

        while(aIter.IsMore())
        {
            E3dExtrudeObj* pExtrudeObj = dynamic_cast< E3dExtrudeObj* >(aIter.Next());

            if(pExtrudeObj)
            {
                const basegfx::B2DPolyPolygon aExtrudePoly(
                    basegfx::tools::prepareForPolygonOperation(pExtrudeObj->GetExtrudePolygon()));
                const SfxItemSet& rLocalSet = pExtrudeObj->GetMergedItemSet();
                const drawing::FillStyle eLocalFillStyle = ITEMVALUE(rLocalSet, XATTR_FILLSTYLE, XFillStyleItem);
                const Color aLocalColor = static_cast<const XFillColorItem&>(rLocalSet.Get(XATTR_FILLCOLOR)).GetColorValue();

                // sort in ExtrudeObj
                if(pLayer)
                {
                    // do we have overlap with an object of this layer?
                    bool bOverlap(false);
                    E3dDepthNeighbour* pAct = pLayer->mpNext;

                    while(!bOverlap && pAct)
                    {
                        // do pAct->mpObj and pExtrudeObj overlap? Check by
                        // using logical AND clipping
                        const basegfx::B2DPolyPolygon aAndPolyPolygon(
                            basegfx::tools::solvePolygonOperationAnd(
                                aExtrudePoly,
                                pAct->maPreparedPolyPolygon));

                        bOverlap = (0 != aAndPolyPolygon.count());

                        if(bOverlap)
                        {
                            // second ciriteria: is another fillstyle or color used?
                            const SfxItemSet& rCompareSet = pAct->mpObj->GetMergedItemSet();

                            drawing::FillStyle eCompareFillStyle = ITEMVALUE(rCompareSet, XATTR_FILLSTYLE, XFillStyleItem);

                            if(eLocalFillStyle == eCompareFillStyle)
                            {
                                if(eLocalFillStyle == drawing::FillStyle_SOLID)
                                {
                                    Color aCompareColor = static_cast<const XFillColorItem&>(rCompareSet.Get(XATTR_FILLCOLOR)).GetColorValue();

                                    if(aCompareColor == aLocalColor)
                                    {
                                        bOverlap = false;
                                    }
                                }
                                else if(eLocalFillStyle == drawing::FillStyle_NONE)
                                {
                                    bOverlap = false;
                                }
                            }
                        }

                        pAct = pAct->mpNext;
                    }

                    if(bOverlap)
                    {
                        // yes, start a new layer
                        pLayer->mpDown = new E3dDepthLayer;
                        pLayer = pLayer->mpDown;
                        nNumLayers++;
                        pLayer->mpNext = new E3dDepthNeighbour;
                        pLayer->mpNext->mpObj = pExtrudeObj;
                        pLayer->mpNext->maPreparedPolyPolygon = aExtrudePoly;
                    }
                    else
                    {
                        // no, add to current layer
                        E3dDepthNeighbour* pNewNext = new E3dDepthNeighbour;
                        pNewNext->mpObj = pExtrudeObj;
                        pNewNext->maPreparedPolyPolygon = aExtrudePoly;
                        pNewNext->mpNext = pLayer->mpNext;
                        pLayer->mpNext = pNewNext;
                    }
                }
                else
                {
                    // first layer ever
                    pBaseLayer = new E3dDepthLayer;
                    pLayer = pBaseLayer;
                    nNumLayers++;
                    pLayer->mpNext = new E3dDepthNeighbour;
                    pLayer->mpNext->mpObj = pExtrudeObj;
                    pLayer->mpNext->maPreparedPolyPolygon = aExtrudePoly;
                }
            }
        }

        // number of layers is done
        if(nNumLayers > 1)
        {
            // need to be arranged
            double fMinDepth = fDepth * 0.8;
            double fStep = (fDepth - fMinDepth) / (double)nNumLayers;
            pLayer = pBaseLayer;

            while(pLayer)
            {
                // move along layer
                E3dDepthNeighbour* pAct = pLayer->mpNext;

                while(pAct)
                {
                    // adapt extrude value
                    pAct->mpObj->SetMergedItem(SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, sal_uInt32(fMinDepth + 0.5)));

                    // next
                    pAct = pAct->mpNext;
                }

                // next layer
                pLayer = pLayer->mpDown;
                fMinDepth += fStep;
            }
        }

        // cleanup
        while(pBaseLayer)
        {
            pLayer = pBaseLayer->mpDown;
            delete pBaseLayer;
            pBaseLayer = pLayer;
        }
    }
}

// Start drag, create for 3D objects before possibly drag method

bool E3dView::BegDragObj(const Point& rPnt, OutputDevice* pOut,
    SdrHdl* pHdl, short nMinMov,
    SdrDragMethod* pForcedMeth)
{
    if(Is3DRotationCreationActive() && GetMarkedObjectCount())
    {
        // Determine all selected polygons and return the mirrored helper overlay
        mpMirrorOverlay->SetMirrorAxis(maRef1, maRef2);
    }
    else
    {
        bool bOwnActionNecessary;
        if (pHdl == NULL)
        {
           bOwnActionNecessary = true;
        }
        else if (pHdl->IsVertexHdl() || pHdl->IsCornerHdl())
        {
           bOwnActionNecessary = true;
        }
        else
        {
           bOwnActionNecessary = false;
        }

        if(bOwnActionNecessary && GetMarkedObjectCount() > 0)
        {
            E3dDragConstraint eConstraint = E3DDRAG_CONSTR_XYZ;
            bool bThereAreRootScenes = false;
            bool bThereAre3DObjects = false;
            const size_t nCnt = GetMarkedObjectCount();
            for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
            {
                SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
                if(pObj)
                {
                    if(pObj->ISA(E3dScene) && static_cast<E3dScene*>(pObj)->GetScene() == pObj)
                        bThereAreRootScenes = true;
                    if(pObj->ISA(E3dObject))
                        bThereAre3DObjects = true;
                }
            }
            if( bThereAre3DObjects )
            {
                meDragHdl = ( pHdl == NULL ? HDL_MOVE : pHdl->GetKind() );
                switch ( meDragMode )
                {
                    case SDRDRAG_ROTATE:
                    case SDRDRAG_SHEAR:
                    {
                        switch ( meDragHdl )
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
                            default: break;
                        }

                        // do not mask the allowed rotations
                        eConstraint = E3dDragConstraint(eConstraint& eDragConstraint);
                        pForcedMeth = new E3dDragRotate(*this, GetMarkedObjectList(), eConstraint, IsSolidDragging());
                    }
                    break;

                    case SDRDRAG_MOVE:
                    {
                        if(!bThereAreRootScenes)
                        {
                            pForcedMeth = new E3dDragMove(*this, GetMarkedObjectList(), meDragHdl, eConstraint, IsSolidDragging());
                        }
                    }
                    break;

                    // later on
                    case SDRDRAG_MIRROR:
                    case SDRDRAG_CROOK:
                    case SDRDRAG_DISTORT:
                    case SDRDRAG_TRANSPARENCE:
                    case SDRDRAG_GRADIENT:
                    default:
                    {
                    }
                    break;
                }
            }
        }
    }
    return SdrView::BegDragObj(rPnt, pOut, pHdl, nMinMov, pForcedMeth);
}

// Set current 3D drawing object, create the scene for this

E3dScene* E3dView::SetCurrent3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj != NULL, "Who puts in a NULL-pointer here");
    E3dScene* pScene = NULL;

    // get transformed BoundVolume of the object
    basegfx::B3DRange aVolume(p3DObj->GetBoundVolume());
    aVolume.transform(p3DObj->GetTransform());
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());

    Rectangle aRect(0,0, (long) fW, (long) fH);

    pScene = new E3dPolyScene(Get3DDefaultAttributes());

    InitScene(pScene, fW, fH, aVolume.getMaxZ() + ((fW + fH) / 4.0));

    pScene->Insert3DObj(p3DObj);
    pScene->NbcSetSnapRect(aRect);

    return pScene;
}

void E3dView::InitScene(E3dScene* pScene, double fW, double fH, double fCamZ)
{
    Camera3D aCam(pScene->GetCamera());

    aCam.SetAutoAdjustProjection(false);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    basegfx::B3DPoint aLookAt;

    double fDefaultCamPosZ = GetDefaultCamPosZ();
    basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);

    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(GetDefaultCamFocal());
    aCam.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, GetDefaultCamFocal());
    pScene->SetCamera(aCam);
}

void E3dView::Start3DCreation()
{
    if (GetMarkedObjectCount())
    {
        //positioned
        long          nOutMin = 0;
        long          nOutMax = 0;
        long          nMinLen = 0;
        long          nObjDst = 0;
        long          nOutHgt = 0;
        OutputDevice* pOut    = GetFirstOutputDevice();

        // first determine representation boundaries
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

        // and then attach the marks at the top and bottom of the object
        basegfx::B2DRange aR;
        for(size_t nMark = 0; nMark < GetMarkedObjectCount(); ++nMark)
        {
            SdrObject* pMark = GetMarkedObjectByIndex(nMark);
            basegfx::B2DPolyPolygon aXPP(pMark->TakeXorPoly());
            aR.expand(basegfx::tools::getRange(aXPP));
        }

        basegfx::B2DPoint aCenter(aR.getCenter());
        long      nMarkHgt = basegfx::fround(aR.getHeight()) - 1;
        long      nHgt     = nMarkHgt + nObjDst * 2;

        if (nHgt < nMinLen) nHgt = nMinLen;

        long nY1 = basegfx::fround(aCenter.getY()) - (nHgt + 1) / 2;
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

        maRef1.X() = basegfx::fround(aR.getMinX());    // Initial move axis 2/100mm to the left
        maRef1.Y() = nY1;
        maRef2.X() = maRef1.X();
        maRef2.Y() = nY2;

        // Turn on marks
        SetMarkHandles();

        //HMHif (bVis) ShowMarkHdl();
        if (AreObjectsMarked()) MarkListHasChanged();

        // Show mirror polygon IMMEDIATELY
        const SdrHdlList &aHdlList = GetHdlList();
        mpMirrorOverlay = new Impl3DMirrorConstructOverlay(*this);
        mpMirrorOverlay->SetMirrorAxis(aHdlList.GetHdl(HDL_REF1)->GetPos(), aHdlList.GetHdl(HDL_REF2)->GetPos());
    }
}

// what happens with a mouse movement when the object is created?

void E3dView::MovAction(const Point& rPnt)
{
    if(Is3DRotationCreationActive())
    {
        SdrHdl* pHdl = GetDragHdl();

        if (pHdl)
        {
            SdrHdlKind eHdlKind = pHdl->GetKind();

            // reacts only due to a mirror axis
            if ((eHdlKind == HDL_REF1) ||
                (eHdlKind == HDL_REF2) ||
                (eHdlKind == HDL_MIRX))
            {
                const SdrHdlList &aHdlList = GetHdlList ();

                // delete the mirrored polygon, mirrors the original and draws
                // it anew
                SdrView::MovAction (rPnt);
                mpMirrorOverlay->SetMirrorAxis(
                    aHdlList.GetHdl (HDL_REF1)->GetPos(),
                    aHdlList.GetHdl (HDL_REF2)->GetPos());
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

// The End. Create object and any child objects through ImpCreate3DLathe.
// With the parameter value sal_True (SDefault: sal_False) is simply a
// rotation body  created, without letting the user set the position of the
// axis. It is sufficient with this call, if an object is selected.
// (No initialization necessary)

void E3dView::End3DCreation(bool bUseDefaultValuesForMirrorAxes)
{
    ResetCreationActive();

    if(AreObjectsMarked())
    {
        if(bUseDefaultValuesForMirrorAxes)
        {
            Rectangle aRect = GetAllMarkedRect();
            if(aRect.GetWidth() <= 1)
                aRect.SetSize(Size(500, aRect.GetHeight()));
            if(aRect.GetHeight() <= 1)
                aRect.SetSize(Size(aRect.GetWidth(), 500));

            basegfx::B2DPoint aPnt1(aRect.Left(), -aRect.Top());
            basegfx::B2DPoint aPnt2(aRect.Left(), -aRect.Bottom());

            ConvertMarkedObjTo3D(false, aPnt1, aPnt2);
        }
        else
        {
            // Turn off helper overlay
            // Determine from the handle positions and the displacement of
            // the points
            const SdrHdlList &aHdlList = GetHdlList();
            Point aMirrorRef1 = aHdlList.GetHdl(HDL_REF1)->GetPos();
            Point aMirrorRef2 = aHdlList.GetHdl(HDL_REF2)->GetPos();

            basegfx::B2DPoint aPnt1(aMirrorRef1.X(), -aMirrorRef1.Y());
            basegfx::B2DPoint aPnt2(aMirrorRef2.X(), -aMirrorRef2.Y());

            ConvertMarkedObjTo3D(false, aPnt1, aPnt2);
        }
    }
}

E3dView::~E3dView ()
{
}

void E3dView::ResetCreationActive ()
{
    if(mpMirrorOverlay)
    {
        delete mpMirrorOverlay;
        mpMirrorOverlay = 0L;
    }
}

void E3dView::InitView ()
{
    eDragConstraint          = E3DDRAG_CONSTR_XYZ;
    fDefaultScaleX           =
    fDefaultScaleY           =
    fDefaultScaleZ           = 1.0;
    fDefaultRotateX          =
    fDefaultRotateY          =
    fDefaultRotateZ          = 0.0;
    fDefaultExtrusionDeepth  = 1000; // old: 2000;
    fDefaultLightIntensity   = 0.8; // old: 0.6;
    fDefaultAmbientIntensity = 0.4;
    nHDefaultSegments        = 12;
    nVDefaultSegments        = 12;
    aDefaultLightColor       = RGB_Color(COL_WHITE);
    aDefaultAmbientColor     = RGB_Color(COL_BLACK);
    bDoubleSided             = false;
    mpMirrorOverlay          = 0L;
}

bool E3dView::IsBreak3DObjPossible() const
{
    const size_t nCount = GetMarkedObjectCount();

    if (nCount > 0)
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(i);

            if (pObj && pObj->ISA(E3dObject))
            {
                if(!(static_cast<E3dObject*>(pObj)->IsBreakObjPossible()))
                    return false;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

void E3dView::Break3DObj()
{
    if(IsBreak3DObjPossible())
    {
        // ALL selected objects are changed
        const size_t nCount = GetMarkedObjectCount();

        BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_BREAK_LATHE));
        for(size_t a=0; a<nCount; ++a)
        {
            E3dObject* pObj = static_cast<E3dObject*>(GetMarkedObjectByIndex(a));
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
            E3dObject* pSubObj = static_cast<E3dObject*>(aIter.Next());
            BreakSingle3DObj(pSubObj);
        }
    }
    else
    {
        SdrAttrObj* pNewObj = pObj->GetBreakObj();
        if(pNewObj)
        {
            InsertObjectAtView(pNewObj, *GetSdrPageView(), SdrInsertFlags::DONTMARK);
            pNewObj->SetChanged();
            pNewObj->BroadcastObjectChange();
        }
    }
}

void E3dView::CheckPossibilities()
{
    // call parent
    SdrView::CheckPossibilities();

    // Set other flags
    if(bGroupPossible || bUnGroupPossible || bGrpEnterPossible)
    {
        const size_t nMarkCnt = GetMarkedObjectCount();
        bool bCoumpound = false;
        bool b3DObject = false;
        for(size_t nObjs = 0; (nObjs < nMarkCnt) && !bCoumpound; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(pObj && pObj->ISA(E3dCompoundObject))
                bCoumpound = true;
            if(pObj && pObj->ISA(E3dObject))
                b3DObject = true;
        }

        // So far: there are two or more of any objects selected. See if
        // compound objects are involved. If yes, ban grouping.
        if(bGroupPossible && bCoumpound)
            bGroupPossible = false;

        if(bUnGroupPossible && b3DObject)
            bUnGroupPossible = false;

        if(bGrpEnterPossible && bCoumpound)
            bGrpEnterPossible = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
