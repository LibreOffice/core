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
#include <svx/svditer.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdview.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/globl3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/cube3d.hxx>
#include <dragmt3d.hxx>
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
    drawinglayer::primitive2d::Primitive2DContainer    maFullOverlay;

    // Copy assignment is forbidden and not implemented.
    Impl3DMirrorConstructOverlay (const Impl3DMirrorConstructOverlay &) = delete;
    Impl3DMirrorConstructOverlay & operator= (const Impl3DMirrorConstructOverlay &) = delete;

public:
    explicit Impl3DMirrorConstructOverlay(const E3dView& rView);
    ~Impl3DMirrorConstructOverlay();

    void SetMirrorAxis(Point aMirrorAxisA, Point aMirrorAxisB);
};

Impl3DMirrorConstructOverlay::Impl3DMirrorConstructOverlay(const E3dView& rView)
:   maObjects(),
    mrView(rView),
    mnCount(rView.GetMarkedObjectCount()),
    mpPolygons(nullptr),
    maFullOverlay()
{
    if(mnCount)
    {
        if(mrView.IsSolidDragging())
        {
            SdrPageView* pPV = rView.GetSdrPageView();

            if(pPV && pPV->PageWindowCount())
            {
                for(size_t a = 0; a < mnCount; ++a)
                {
                    SdrObject* pObject = mrView.GetMarkedObjectByIndex(a);

                    if(pObject)
                    {
                        // use the view-independent primitive representation (without
                        // evtl. GridOffset, that may be applied to the DragEntry individually)
                        const drawinglayer::primitive2d::Primitive2DContainer aNewSequence(
                            pObject->GetViewContact().getViewIndependentPrimitive2DContainer());
                        maFullOverlay.append(aNewSequence);
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
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if(xTargetOverlay.is())
        {
            // build transformation: translate and rotate so that given edge is
            // on x axis, them mirror in y and translate back
            const basegfx::B2DVector aEdge(aMirrorAxisB.X() - aMirrorAxisA.X(), aMirrorAxisB.Y() - aMirrorAxisA.Y());
            basegfx::B2DHomMatrix aMatrixTransform(basegfx::utils::createTranslateB2DHomMatrix(
                -aMirrorAxisA.X(), -aMirrorAxisA.Y()));
            aMatrixTransform.rotate(-atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.scale(1.0, -1.0);
            aMatrixTransform.rotate(atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.translate(aMirrorAxisA.X(), aMirrorAxisA.Y());

            if(mrView.IsSolidDragging())
            {
                if(!maFullOverlay.empty())
                {
                    drawinglayer::primitive2d::Primitive2DContainer aContent(maFullOverlay);

                    if(!aMatrixTransform.isIdentity())
                    {
                        // embed in transformation group
                        drawinglayer::primitive2d::Primitive2DReference aTransformPrimitive2D(new drawinglayer::primitive2d::TransformPrimitive2D(aMatrixTransform, aContent));
                        aContent = drawinglayer::primitive2d::Primitive2DContainer { aTransformPrimitive2D };
                    }

                    // if we have full overlay from selected objects, embed with 50% transparence, the
                    // transformation is added to the OverlayPrimitive2DSequenceObject
                    drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparencePrimitive2D(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aContent, 0.5));
                    aContent = drawinglayer::primitive2d::Primitive2DContainer { aUnifiedTransparencePrimitive2D };

                    std::unique_ptr<sdr::overlay::OverlayPrimitive2DSequenceObject> pNew(new sdr::overlay::OverlayPrimitive2DSequenceObject(aContent));

                    xTargetOverlay->add(*pNew);
                    maObjects.append(std::move(pNew));
                }
            }
            else
            {
                for(size_t b = 0; b < mnCount; ++b)
                {
                    // apply to polygon
                    basegfx::B2DPolyPolygon aPolyPolygon(mpPolygons[b]);
                    aPolyPolygon.transform(aMatrixTransform);

                    std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew(new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                        aPolyPolygon));
                    xTargetOverlay->add(*pNew);
                    maObjects.append(std::move(pNew));
                }
            }
        }
    }
}

E3dView::E3dView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrView(rSdrModel, pOut)
{
    InitView();
}

// DrawMarkedObj override, since possibly only a single 3D object is to be
// drawn

void E3dView::DrawMarkedObj(OutputDevice& rOut) const
{
    // Does 3D objects exist which scenes are not selected?
    bool bSpecialHandling = false;
    E3dScene *pScene = nullptr;

    const size_t nCnt = GetMarkedObjectCount();
    for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
    {
        SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
        if(auto pCompoundObject = dynamic_cast<E3dCompoundObject*>(pObj))
        {
            // related scene
            pScene = pCompoundObject->getRootE3dSceneFromE3dObject();

            if(nullptr != pScene && !IsObjMarked(pScene))
            {
                bSpecialHandling = true;
            }
        }
        // Reset all selection flags
        if(auto p3dObject = dynamic_cast< const E3dObject*>(pObj))
        {
            pScene = p3dObject->getRootE3dSceneFromE3dObject();

            if(nullptr != pScene)
            {
                pScene->SetSelected(false);
            }
        }
    }

    if(bSpecialHandling)
    {
        // Set selection flag to "not selected" for scenes related to all 3D
        // objects
        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(auto pCompoundObject = dynamic_cast<E3dCompoundObject*>(pObj))
            {
                // related scene
                pScene = pCompoundObject->getRootE3dSceneFromE3dObject();

                if(nullptr != pScene)
                {
                    pScene->SetSelected(false);
                }
            }
        }

        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(auto p3DObj = dynamic_cast<E3dObject*>(pObj))
            {
                // Select object
                p3DObj->SetSelected(true);
                pScene = p3DObj->getRootE3dSceneFromE3dObject();
            }
        }

        if(nullptr != pScene)
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
            if(auto pCompoundObject = dynamic_cast<E3dCompoundObject*>(pObj))
            {
                // related scene
                pScene = pCompoundObject->getRootE3dSceneFromE3dObject();

                if(nullptr != pScene)
                {
                    pScene->SetSelected(false);
                }
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

std::unique_ptr<SdrModel> E3dView::CreateMarkedObjModel() const
{
    // Does 3D objects exist which scenes are not selected?
    bool bSpecialHandling(false);
    const size_t nCount(GetMarkedObjectCount());
    E3dScene *pScene = nullptr;

    for(size_t nObjs = 0; nObjs < nCount; ++nObjs)
    {
        const SdrObject* pObj = GetMarkedObjectByIndex(nObjs);

        if(!bSpecialHandling && dynamic_cast< const E3dCompoundObject*>(pObj))
        {
            // if the object is selected, but it's scene not,
            // we need special handling
            pScene = static_cast<const E3dCompoundObject*>(pObj)->getRootE3dSceneFromE3dObject();

            if(nullptr != pScene && !IsObjMarked(pScene))
            {
                bSpecialHandling = true;
            }
        }

        if(auto p3dObject = dynamic_cast< const E3dObject*>(pObj))
        {
            // reset all selection flags at 3D objects
            pScene = p3dObject->getRootE3dSceneFromE3dObject();

            if(nullptr != pScene)
            {
                pScene->SetSelected(false);
            }
        }
    }

    if(!bSpecialHandling)
    {
        // call parent
        return SdrView::CreateMarkedObjModel();
    }

    std::unique_ptr<SdrModel> pNewModel;
    tools::Rectangle aSelectedSnapRect;

    // set 3d selection flags at all directly selected objects
    // and collect SnapRect of selected objects
    for(size_t nObjs = 0; nObjs < nCount; ++nObjs)
    {
        SdrObject *pObj = GetMarkedObjectByIndex(nObjs);

        if(auto p3DObj = dynamic_cast<E3dCompoundObject*>(pObj))
        {
            // mark object, but not scenes
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

        if(auto p3dObject = dynamic_cast< E3dObject* >(pObj))
        {
            pScene = p3dObject->getRootE3dSceneFromE3dObject();

            if(nullptr != pScene && !IsObjMarked(pScene) && GetSdrPageView())
            {
                const_cast<E3dView*>(this)->MarkObj(pScene, GetSdrPageView(), false, true);
            }
        }
    }

    // call parent. This will copy all scenes and the selection flags at the 3D objects. So
    // it will be possible to delete all non-selected 3d objects from the cloned 3d scenes
    pNewModel = SdrView::CreateMarkedObjModel();

    if(pNewModel)
    {
        for(sal_uInt16 nPg(0); nPg < pNewModel->GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=pNewModel->GetPage(nPg);
            const size_t nObjCount(pSrcPg->GetObjCount());

            for(size_t nOb = 0; nOb < nObjCount; ++nOb)
            {
                const SdrObject* pSrcOb=pSrcPg->GetObj(nOb);

                if(auto p3dscene = dynamic_cast< const E3dScene* >( pSrcOb))
                {
                    pScene = const_cast<E3dScene*>(p3dscene);

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
    const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions)
{
    bool bRetval = false;

    // Get list
    Point aPos(rPos);
    SdrObjList* pDstList = pLst;
    ImpGetPasteObjList(aPos, pDstList);

    if(!pDstList)
        return false;

    // Get owner of the list
    E3dScene* pDstScene(dynamic_cast< E3dScene* >(pDstList->getSdrObjectFromSdrObjList()));

    if(nullptr != pDstScene)
    {
        BegUndo(SvxResId(RID_SVX_3D_UNDO_EXCHANGE_PASTE));

        // Copy all objects from E3dScenes and insert them directly
        for(sal_uInt16 nPg(0); nPg < rMod.GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=rMod.GetPage(nPg);
            const size_t nObjCount(pSrcPg->GetObjCount());

            // calculate offset for paste
            tools::Rectangle aR = pSrcPg->GetAllObjBoundRect();
            Point aDist(aPos - aR.Center());

            // Insert sub-objects for scenes
            for(size_t nOb = 0; nOb < nObjCount; ++nOb)
            {
                const SdrObject* pSrcOb = pSrcPg->GetObj(nOb);
                if(auto p3dscene = dynamic_cast< const E3dScene* >(pSrcOb))
                {
                    E3dScene* pSrcScene = const_cast<E3dScene*>(p3dscene);
                    ImpCloneAll3DObjectsToDestScene(pSrcScene, pDstScene, aDist);
                }
            }
        }
        EndUndo();
    }
    else
    {
        // call parent
        bRetval = SdrView::Paste(rMod, rPos, pLst, nOptions);
    }

    return bRetval;
}

// Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
bool E3dView::ImpCloneAll3DObjectsToDestScene(E3dScene const * pSrcScene, E3dScene* pDstScene, Point /*aOffset*/)
{
    bool bRetval(false);

    if(pSrcScene && pDstScene)
    {
        for(size_t i = 0; i < pSrcScene->GetSubList()->GetObjCount(); ++i)
        {
            E3dCompoundObject* pCompoundObj = dynamic_cast< E3dCompoundObject* >(pSrcScene->GetSubList()->GetObj(i));

            if(pCompoundObj)
            {
                E3dCompoundObject* pNewCompoundObj(pCompoundObj->CloneSdrObject(pDstScene->getSdrModelFromSdrObject()));

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
                    pNewCompoundObj->NbcSetLayer(pCompoundObj->GetLayer());
                    pNewCompoundObj->NbcSetStyleSheet(pCompoundObj->GetStyleSheet(), true);
                    pDstScene->InsertObject(pNewCompoundObj);
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
           IsConvertToPolyObjPossible()
        || IsConvertToPathObjPossible()
        || IsImportMtfPossible());
    return bRetval;
}

void E3dView::ImpIsConvertTo3DPossible(SdrObject const * pObj, bool& rAny3D,
    bool& rGroupSelected) const
{
    if(pObj)
    {
        if(dynamic_cast< const E3dObject* >(pObj) !=  nullptr)
        {
            rAny3D = true;
        }
        else
        {
            if(pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj, SdrIterMode::DeepNoGroups);
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

void E3dView::ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj)
{
    if(dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
    {
        const SfxItemSet& rSet = pObj->GetMergedItemSet();
        const SvxColorItem& rTextColorItem = rSet.Get(EE_CHAR_COLOR);
        if(rTextColorItem.GetValue() == COL_BLACK)
        {
            //For black text objects, the color set to gray
            if(pObj->getSdrPageFromSdrObject())
            {
                // if black is only default attribute from
                // pattern set it hard so that it is used in undo.
                pObj->SetMergedItem(SvxColorItem(COL_BLACK, EE_CHAR_COLOR));

                // add undo now
                if( GetModel()->IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
            }

            pObj->SetMergedItem(SvxColorItem(COL_GRAY, EE_CHAR_COLOR));
        }
    }
}

void E3dView::ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj)
{
    if(dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr)
    {
        const SfxItemSet& rSet = pObj->GetMergedItemSet();
        sal_Int32 nLineWidth = rSet.Get(XATTR_LINEWIDTH).GetValue();
        drawing::LineStyle eLineStyle = rSet.Get(XATTR_LINESTYLE).GetValue();
        drawing::FillStyle eFillStyle = rSet.Get(XATTR_FILLSTYLE).GetValue();

        if(static_cast<SdrPathObj*>(pObj)->IsClosed()
            && eLineStyle == drawing::LineStyle_SOLID
            && !nLineWidth
            && eFillStyle != drawing::FillStyle_NONE)
        {
            if(pObj->getSdrPageFromSdrObject() && GetModel()->IsUndoEnabled() )
            {
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
            }

            pObj->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
            pObj->SetMergedItem(XLineWidthItem(0));
        }
    }
}

void E3dView::ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix const & rLatheMat)
{
    // Single PathObject, transform this
    SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pObj );

    if(pPath)
    {
        E3dDefaultAttributes aDefault = Get3DDefaultAttributes();

        if(bExtrude)
        {
            aDefault.SetDefaultExtrudeCharacterMode(true);
        }
        else
        {
            aDefault.SetDefaultLatheCharacterMode(true);
        }

        // Get Itemset of the original object
        SfxItemSet aSet(pObj->GetMergedItemSet());

        drawing::FillStyle eFillStyle = aSet.Get(XATTR_FILLSTYLE).GetValue();

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
            Color aColorLine = aSet.Get(XATTR_LINECOLOR).GetColorValue();
            aSet.Put(XFillColorItem(OUString(), aColorLine));
        }

        // Create a new extrude object
        E3dObject* p3DObj = nullptr;
        if(bExtrude)
        {
            p3DObj = new E3dExtrudeObj(pObj->getSdrModelFromSdrObject(), aDefault, pPath->GetPathPoly(), fDepth);
        }
        else
        {
            basegfx::B2DPolyPolygon aPolyPoly2D(pPath->GetPathPoly());
            aPolyPoly2D.transform(rLatheMat);
            p3DObj = new E3dLatheObj(pObj->getSdrModelFromSdrObject(), aDefault, aPolyPoly2D);
        }

        // Set attribute
        p3DObj->NbcSetLayer(pObj->GetLayer());

        p3DObj->SetMergedItemSet(aSet);

        p3DObj->NbcSetStyleSheet(pObj->GetStyleSheet(), true);

        // Insert a new extrude object
        pScene->InsertObject(p3DObj);
    }
}

void E3dView::ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix const & rLatheMat)
{
    if(pObj)
    {
        // change text color attribute for not so dark colors
        if(pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj, SdrIterMode::DeepWithGroups);
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
                SdrObjListIter aIter(*pNewObj1, SdrIterMode::DeepWithGroups);
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
                    SdrObjListIter aIter(*pNewObj2, SdrIterMode::DeepWithGroups);
                    while(aIter.IsMore())
                    {
                        SdrObject* pGroupMember = aIter.Next();
                        ImpCreateSingle3DObjectFlat(pScene, pGroupMember, bExtrude, fDepth, rLatheMat);
                    }
                }
                else
                    ImpCreateSingle3DObjectFlat(pScene, pNewObj2, bExtrude, fDepth, rLatheMat);

                // delete object in between
                if (pNewObj2 != pObj && pNewObj2 != pNewObj1)
                    SdrObject::Free( pNewObj2 );
            }

            // delete object in between
            if (pNewObj1 != pObj)
                SdrObject::Free( pNewObj1 );
        }
    }
}

void E3dView::ConvertMarkedObjTo3D(bool bExtrude, const basegfx::B2DPoint& rPnt1, const basegfx::B2DPoint& rPnt2)
{
    if(!AreObjectsMarked())
        return;

    // Create undo
    if(bExtrude)
        BegUndo(SvxResId(RID_SVX_3D_UNDO_EXTRUDE));
    else
        BegUndo(SvxResId(RID_SVX_3D_UNDO_LATHE));

    SdrModel& rSdrModel(GetSdrMarkByIndex(0)->GetMarkedSdrObj()->getSdrModelFromSdrObject());

    // Create a new scene for the created 3D object
    E3dScene* pScene = new E3dScene(rSdrModel);

    // Determine rectangle and possibly correct it
    tools::Rectangle aRect = GetAllMarkedRect();
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
        double fW = static_cast<double>(aRect.GetWidth());
        double fH = static_cast<double>(aRect.GetHeight());
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
                aLatheMat = basegfx::utils::createRotateAroundPoint(rPnt2, -fRot3D)
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
            aLatheMat.translate(static_cast<double>(-aRect.Left()), 0.0);
        }

        // Form the inverse matrix to determine the target expansion
        basegfx::B2DHomMatrix aInvLatheMat(aLatheMat);
        aInvLatheMat.invert();

        // SnapRect extension enables mirroring in the axis of rotation
        for(size_t a=0; a<GetMarkedObjectCount(); ++a)
        {
            SdrMark* pMark = GetSdrMarkByIndex(a);
            SdrObject* pObj = pMark->GetMarkedSdrObj();
            tools::Rectangle aTurnRect = pObj->GetSnapRect();
            basegfx::B2DPoint aRot;
            Point aRotPnt;

            aRot = basegfx::B2DPoint(aTurnRect.Left(), -aTurnRect.Top());
            aRot *= aLatheMat;
            aRot.setX(-aRot.getX());
            aRot *= aInvLatheMat;
            aRotPnt = Point(static_cast<long>(aRot.getX() + 0.5), static_cast<long>(-aRot.getY() - 0.5));
            aRect.Union(tools::Rectangle(aRotPnt, aRotPnt));

            aRot = basegfx::B2DPoint(aTurnRect.Left(), -aTurnRect.Bottom());
            aRot *= aLatheMat;
            aRot.setX(-aRot.getX());
            aRot *= aInvLatheMat;
            aRotPnt = Point(static_cast<long>(aRot.getX() + 0.5), static_cast<long>(-aRot.getY() - 0.5));
            aRect.Union(tools::Rectangle(aRotPnt, aRotPnt));

            aRot = basegfx::B2DPoint(aTurnRect.Right(), -aTurnRect.Top());
            aRot *= aLatheMat;
            aRot.setX(-aRot.getX());
            aRot *= aInvLatheMat;
            aRotPnt = Point(static_cast<long>(aRot.getX() + 0.5), static_cast<long>(-aRot.getY() - 0.5));
            aRect.Union(tools::Rectangle(aRotPnt, aRotPnt));

            aRot = basegfx::B2DPoint(aTurnRect.Right(), -aTurnRect.Bottom());
            aRot *= aLatheMat;
            aRot.setX(-aRot.getX());
            aRot *= aInvLatheMat;
            aRotPnt = Point(static_cast<long>(aRot.getX() + 0.5), static_cast<long>(-aRot.getY() - 0.5));
            aRect.Union(tools::Rectangle(aRotPnt, aRotPnt));
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
        InitScene(pScene, static_cast<double>(aRect.GetWidth()), static_cast<double>(aRect.GetHeight()), aBoundVol.getDepth());

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
        aRotate.rotate(DEG2RAD(20.0), 0.0, 0.0);

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
        // always use SdrObject::Free(...) for SdrObjects (!)
        SdrObject* pTemp(pScene);
        SdrObject::Free(pTemp);
    }

    EndUndo();
}

//Arrange all created extrude objects by depth

struct E3dDepthNeighbour
{
    E3dExtrudeObj*              mpObj;
    basegfx::B2DPolyPolygon     maPreparedPolyPolygon;

    E3dDepthNeighbour(E3dExtrudeObj* pObj, basegfx::B2DPolyPolygon const & rPreparedPolyPolygon)
    :   mpObj(pObj),
        maPreparedPolyPolygon(rPreparedPolyPolygon)
    {
    }
};

struct E3dDepthLayer
{
    E3dDepthLayer*              mpDown;
    std::vector<E3dDepthNeighbour> mvNeighbours;

    E3dDepthLayer()
    :   mpDown(nullptr)
    {
    }
};

void E3dView::DoDepthArrange(E3dScene const * pScene, double fDepth)
{
    if(pScene && pScene->GetSubList() && pScene->GetSubList()->GetObjCount() > 1)
    {
        SdrObjList* pSubList = pScene->GetSubList();
        SdrObjListIter aIter(pSubList, SdrIterMode::Flat);
        E3dDepthLayer* pBaseLayer = nullptr;
        E3dDepthLayer* pLayer = nullptr;
        sal_Int32 nNumLayers = 0;

        while(aIter.IsMore())
        {
            E3dExtrudeObj* pExtrudeObj = dynamic_cast< E3dExtrudeObj* >(aIter.Next());

            if(pExtrudeObj)
            {
                const basegfx::B2DPolyPolygon aExtrudePoly(
                    basegfx::utils::prepareForPolygonOperation(pExtrudeObj->GetExtrudePolygon()));
                const SfxItemSet& rLocalSet = pExtrudeObj->GetMergedItemSet();
                const drawing::FillStyle eLocalFillStyle = rLocalSet.Get(XATTR_FILLSTYLE).GetValue();
                const Color aLocalColor = rLocalSet.Get(XATTR_FILLCOLOR).GetColorValue();

                // sort in ExtrudeObj
                if(pLayer)
                {
                    // do we have overlap with an object of this layer?
                    bool bOverlap(false);

                    for(const auto& rAct : pLayer->mvNeighbours)
                    {
                        // do rAct.mpObj and pExtrudeObj overlap? Check by
                        // using logical AND clipping
                        const basegfx::B2DPolyPolygon aAndPolyPolygon(
                            basegfx::utils::solvePolygonOperationAnd(
                                aExtrudePoly,
                                rAct.maPreparedPolyPolygon));

                        if(aAndPolyPolygon.count() != 0)
                        {
                            // second criteria: is another fillstyle or color used?
                            const SfxItemSet& rCompareSet = rAct.mpObj->GetMergedItemSet();

                            drawing::FillStyle eCompareFillStyle = rCompareSet.Get(XATTR_FILLSTYLE).GetValue();

                            if(eLocalFillStyle == eCompareFillStyle)
                            {
                                if(eLocalFillStyle == drawing::FillStyle_SOLID)
                                {
                                    Color aCompareColor = rCompareSet.Get(XATTR_FILLCOLOR).GetColorValue();

                                    if(aCompareColor == aLocalColor)
                                    {
                                        continue;
                                    }
                                }
                                else if(eLocalFillStyle == drawing::FillStyle_NONE)
                                {
                                    continue;
                                }
                            }

                            bOverlap = true;
                            break;
                        }
                    }

                    if(bOverlap)
                    {
                        // yes, start a new layer
                        pLayer->mpDown = new E3dDepthLayer;
                        pLayer = pLayer->mpDown;
                        nNumLayers++;
                        pLayer->mvNeighbours.emplace_back(pExtrudeObj, aExtrudePoly);
                    }
                    else
                    {
                        // no, add to current layer
                        pLayer->mvNeighbours.emplace(pLayer->mvNeighbours.begin(), pExtrudeObj, aExtrudePoly);
                    }
                }
                else
                {
                    // first layer ever
                    pBaseLayer = new E3dDepthLayer;
                    pLayer = pBaseLayer;
                    nNumLayers++;
                    pLayer->mvNeighbours.emplace_back(pExtrudeObj, aExtrudePoly);
                }
            }
        }

        // number of layers is done
        if(nNumLayers > 1)
        {
            // need to be arranged
            double fMinDepth = fDepth * 0.8;
            double fStep = (fDepth - fMinDepth) / static_cast<double>(nNumLayers);
            pLayer = pBaseLayer;

            while(pLayer)
            {
                // move along layer
                for(auto& rAct : pLayer->mvNeighbours)
                {
                    // adapt extrude value
                    rAct.mpObj->SetMergedItem(SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, sal_uInt32(fMinDepth + 0.5)));
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
        if (pHdl == nullptr)
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
            E3dDragConstraint eConstraint = E3dDragConstraint::XYZ;
            bool bThereAreRootScenes = false;
            bool bThereAre3DObjects = false;
            const size_t nCnt = GetMarkedObjectCount();
            for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
            {
                SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
                if(pObj)
                {
                    if(nullptr != dynamic_cast< const E3dScene* >(pObj) && static_cast< E3dScene* >(pObj)->getRootE3dSceneFromE3dObject() == pObj)
                    {
                        bThereAreRootScenes = true;
                    }

                    if(dynamic_cast< const E3dObject* >(pObj) !=  nullptr)
                    {
                        bThereAre3DObjects = true;
                    }
                }
            }
            if( bThereAre3DObjects )
            {
                meDragHdl = ( pHdl == nullptr ? SdrHdlKind::Move : pHdl->GetKind() );
                switch ( meDragMode )
                {
                    case SdrDragMode::Rotate:
                    case SdrDragMode::Shear:
                    {
                        switch ( meDragHdl )
                        {
                            case SdrHdlKind::Left:
                            case SdrHdlKind::Right:
                            {
                                eConstraint = E3dDragConstraint::X;
                            }
                            break;

                            case SdrHdlKind::Upper:
                            case SdrHdlKind::Lower:
                            {
                                eConstraint = E3dDragConstraint::Y;
                            }
                            break;

                            case SdrHdlKind::UpperLeft:
                            case SdrHdlKind::UpperRight:
                            case SdrHdlKind::LowerLeft:
                            case SdrHdlKind::LowerRight:
                            {
                                eConstraint = E3dDragConstraint::Z;
                            }
                            break;
                            default: break;
                        }

                        // do not mask the allowed rotations
                        eConstraint &= E3dDragConstraint::XYZ;
                        pForcedMeth = new E3dDragRotate(*this, GetMarkedObjectList(), eConstraint, IsSolidDragging());
                    }
                    break;

                    case SdrDragMode::Move:
                    {
                        if(!bThereAreRootScenes)
                        {
                            pForcedMeth = new E3dDragMove(*this, GetMarkedObjectList(), meDragHdl, eConstraint, IsSolidDragging());
                        }
                    }
                    break;

                    // later on
                    case SdrDragMode::Mirror:
                    case SdrDragMode::Crook:
                    case SdrDragMode::Transparence:
                    case SdrDragMode::Gradient:
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
    DBG_ASSERT(p3DObj != nullptr, "Who puts in a NULL-pointer here");

    // get transformed BoundVolume of the object
    basegfx::B3DRange aVolume(p3DObj->GetBoundVolume());
    aVolume.transform(p3DObj->GetTransform());
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());

    tools::Rectangle aRect(0,0, static_cast<long>(fW), static_cast<long>(fH));

    E3dScene* pScene = new E3dScene(p3DObj->getSdrModelFromSdrObject());

    InitScene(pScene, fW, fH, aVolume.getMaxZ() + ((fW + fH) / 4.0));

    pScene->InsertObject(p3DObj);
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
    pScene->SetCamera(aCam);
}

void E3dView::Start3DCreation()
{
    if (!GetMarkedObjectCount())
        return;

    //positioned
    long          nOutMin = 0;
    long          nOutMax = 0;
    long          nMinLen = 0;
    long          nObjDst = 0;
    long          nOutHgt = 0;
    OutputDevice* pOut    = GetFirstOutputDevice();

    // first determine representation boundaries
    if (pOut != nullptr)
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
        aR.expand(basegfx::utils::getRange(aXPP));
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

    maRef1.setX( basegfx::fround(aR.getMinX()) );    // Initial move axis 2/100mm to the left
    maRef1.setY( nY1 );
    maRef2.setX( maRef1.X() );
    maRef2.setY( nY2 );

    // Turn on marks
    SetMarkHandles(nullptr);

    //HMHif (bVis) ShowMarkHdl();
    if (AreObjectsMarked()) MarkListHasChanged();

    // Show mirror polygon IMMEDIATELY
    const SdrHdlList &aHdlList = GetHdlList();
    mpMirrorOverlay.reset(new Impl3DMirrorConstructOverlay(*this));
    mpMirrorOverlay->SetMirrorAxis(aHdlList.GetHdl(SdrHdlKind::Ref1)->GetPos(), aHdlList.GetHdl(SdrHdlKind::Ref2)->GetPos());
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
            if ((eHdlKind == SdrHdlKind::Ref1) ||
                (eHdlKind == SdrHdlKind::Ref2) ||
                (eHdlKind == SdrHdlKind::MirrorAxis))
            {
                const SdrHdlList &aHdlList = GetHdlList ();

                // delete the mirrored polygon, mirrors the original and draws
                // it anew
                SdrView::MovAction (rPnt);
                mpMirrorOverlay->SetMirrorAxis(
                    aHdlList.GetHdl (SdrHdlKind::Ref1)->GetPos(),
                    aHdlList.GetHdl (SdrHdlKind::Ref2)->GetPos());
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
            tools::Rectangle aRect = GetAllMarkedRect();
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
            Point aMirrorRef1 = aHdlList.GetHdl(SdrHdlKind::Ref1)->GetPos();
            Point aMirrorRef2 = aHdlList.GetHdl(SdrHdlKind::Ref2)->GetPos();

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
    mpMirrorOverlay.reset();
}

void E3dView::InitView ()
{
    mpMirrorOverlay          = nullptr;
}

bool E3dView::IsBreak3DObjPossible() const
{
    const size_t nCount = GetMarkedObjectCount();

    if (nCount > 0)
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(i);

            if (auto p3dObject = dynamic_cast< E3dObject* >(pObj))
            {
                if(!p3dObject->IsBreakObjPossible())
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

        BegUndo(SvxResId(RID_SVX_3D_UNDO_BREAK_LATHE));
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
    if(dynamic_cast< const E3dScene* >(pObj) !=  nullptr)
    {
        SdrObjList* pSubList = pObj->GetSubList();
        SdrObjListIter aIter(pSubList, SdrIterMode::Flat);

        while(aIter.IsMore())
        {
            E3dObject* pSubObj = static_cast<E3dObject*>(aIter.Next());
            BreakSingle3DObj(pSubObj);
        }
    }
    else
    {
        SdrAttrObj* pNewObj = pObj->GetBreakObj().release();
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
    if(m_bGroupPossible || m_bUnGroupPossible || m_bGrpEnterPossible)
    {
        const size_t nMarkCnt = GetMarkedObjectCount();
        bool bCoumpound = false;
        bool b3DObject = false;
        for(size_t nObjs = 0; (nObjs < nMarkCnt) && !bCoumpound; ++nObjs)
        {
            SdrObject *pObj = GetMarkedObjectByIndex(nObjs);
            if(dynamic_cast< const E3dCompoundObject* >(pObj))
                bCoumpound = true;
            if(dynamic_cast< const E3dObject* >(pObj))
                b3DObject = true;
        }

        // So far: there are two or more of any objects selected. See if
        // compound objects are involved. If yes, ban grouping.
        if(m_bGroupPossible && bCoumpound)
            m_bGroupPossible = false;

        if(m_bUnGroupPossible && b3DObject)
            m_bUnGroupPossible = false;

        if(m_bGrpEnterPossible && bCoumpound)
            m_bGrpEnterPossible = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
