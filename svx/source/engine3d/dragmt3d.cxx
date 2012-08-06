/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <dragmt3d.hxx>
#include <tools/shl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdtrans.hxx>
#include <svx/obj3d.hxx>
#include <svx/polysc3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/dialogs.hrc>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <svx/e3dsceneupdater.hxx>

TYPEINIT1(E3dDragMethod, SdrDragMethod);

E3dDragMethod::E3dDragMethod (
    SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    sal_Bool bFull)
:   SdrDragMethod(_rView),
    meConstraint(eConstr),
    mbMoveFull(bFull),
    mbMovedAtAll(sal_False)
{
    // Create a unit for all the 3D objects present in the selection
    const long nCnt(rMark.GetMarkCount());
    static bool bDoInvalidate(false);
    long nObjs(0);

    if(mbMoveFull)
    {
        // for non-visible 3D objects fallback to wireframe interaction
        bool bInvisibleObjects(false);

        for(nObjs = 0;!bInvisibleObjects && nObjs < nCnt;nObjs++)
        {
            E3dObject* pE3dObj = dynamic_cast< E3dObject* >(rMark.GetMark(nObjs)->GetMarkedSdrObj());

            if(pE3dObj)
            {
                if(!pE3dObj->HasFillStyle() && !pE3dObj->HasLineStyle())
                {
                    bInvisibleObjects = true;
                }
            }
        }

        if(bInvisibleObjects)
        {
            mbMoveFull = false;
        }
    }

    for(nObjs = 0;nObjs < nCnt;nObjs++)
    {
        E3dObject* pE3dObj = dynamic_cast< E3dObject* >(rMark.GetMark(nObjs)->GetMarkedSdrObj());

        if(pE3dObj)
        {
            // fill new interaction unit
            E3dDragMethodUnit aNewUnit;
            aNewUnit.mp3DObj = pE3dObj;

            // get transformations
            aNewUnit.maInitTransform = aNewUnit.maTransform = pE3dObj->GetTransform();

            if(pE3dObj->GetParentObj())
            {
                // get transform between object and world, normally scene transform
                aNewUnit.maInvDisplayTransform = aNewUnit.maDisplayTransform = pE3dObj->GetParentObj()->GetFullTransform();
                aNewUnit.maInvDisplayTransform.invert();
            }

            // Invalidate SnapRects of the objects involved, to force a
            // recalculation for setting the marker
            if(bDoInvalidate)
            {
                pE3dObj->SetRectsDirty();
            }

            if(!mbMoveFull)
            {
                // create wireframe visualisation for parent coordinate system
                aNewUnit.maWireframePoly.clear();
                aNewUnit.maWireframePoly = pE3dObj->CreateWireframe();
                aNewUnit.maWireframePoly.transform(aNewUnit.maTransform);
            }

            // Determine FullBound
            maFullBound.Union(pE3dObj->GetSnapRect());

            // Insert Unit
            maGrp.push_back(aNewUnit);
        }
    }
}

/*************************************************************************
|*
\************************************************************************/

void E3dDragMethod::TakeSdrDragComment(XubString& /*rStr*/) const
{
}

// Create the wireframe model for all actions

bool E3dDragMethod::BeginSdrDrag()
{
    if(E3DDRAG_CONSTR_Z == meConstraint)
    {
        const sal_uInt32 nCnt(maGrp.size());
        DragStat().Ref1() = maFullBound.Center();

        for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
        {
            E3dDragMethodUnit& rCandidate = maGrp[nOb];
            rCandidate.mnStartAngle = GetAngle(DragStat().GetStart() - DragStat().GetRef1());
            rCandidate.mnLastAngle = 0;
        }
    }
    else
    {
        maLastPos = DragStat().GetStart();
    }

    if(!mbMoveFull)
    {
        Show();
    }

    return sal_True;
}

bool E3dDragMethod::EndSdrDrag(bool /*bCopy*/)
{
    const sal_uInt32 nCnt(maGrp.size());

    if(!mbMoveFull)
    {
        // Hide wireframe
        Hide();
    }

    // Apply all transformations and create undo's
    if(mbMovedAtAll)
    {
        const bool bUndo = getSdrDragView().IsUndoEnabled();
        if( bUndo )
            getSdrDragView().BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_ROTATE));
        sal_uInt32 nOb(0);

        for(nOb=0;nOb<nCnt;nOb++)
        {
            E3dDragMethodUnit& rCandidate = maGrp[nOb];
            E3DModifySceneSnapRectUpdater aUpdater(rCandidate.mp3DObj);
            rCandidate.mp3DObj->SetTransform(rCandidate.maTransform);
            if( bUndo )
            {
                getSdrDragView().AddUndo(new E3dRotateUndoAction(rCandidate.mp3DObj->GetModel(),
                    rCandidate.mp3DObj, rCandidate.maInitTransform,
                    rCandidate.maTransform));
            }
        }
        if( bUndo )
            getSdrDragView().EndUndo();
    }

    return sal_True;
}

void E3dDragMethod::CancelSdrDrag()
{
    if(mbMoveFull)
    {
        if(mbMovedAtAll)
        {
            const sal_uInt32 nCnt(maGrp.size());

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                // Restore transformation
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                E3DModifySceneSnapRectUpdater aUpdater(rCandidate.mp3DObj);
                rCandidate.mp3DObj->SetTransform(rCandidate.maInitTransform);
            }
        }
    }
    else
    {
        // Hide WireFrame
        Hide();
    }
}

// Common MoveSdrDrag()

void E3dDragMethod::MoveSdrDrag(const Point& /*rPnt*/)
{
    mbMovedAtAll = true;
}

// Draw the wire frame model

// for migration from XOR to overlay
void E3dDragMethod::CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager)
{
    const sal_uInt32 nCnt(maGrp.size());
    basegfx::B2DPolyPolygon aResult;

    for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
    {
        E3dDragMethodUnit& rCandidate = maGrp[nOb];
        SdrPageView* pPV = getSdrDragView().GetSdrPageView();

        if(pPV && pPV->HasMarkedObjPageView())
        {
            const basegfx::B3DPolyPolygon aCandidate(rCandidate.maWireframePoly);
            const sal_uInt32 nPlyCnt(aCandidate.count());

            if(nPlyCnt)
            {
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rCandidate.mp3DObj->GetScene()->GetViewContact());
                const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
                const basegfx::B3DHomMatrix aWorldToView(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection() * aViewInfo3D.getOrientation());
                const basegfx::B3DHomMatrix aTransform(aWorldToView * rCandidate.maDisplayTransform);

                // transform to relative scene coordinates
                basegfx::B2DPolyPolygon aPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aCandidate, aTransform));

                // transform to 2D view coordinates
                aPolyPolygon.transform(rVCScene.getObjectTransformation());

                aResult.append(aPolyPolygon);
            }
        }
    }

    if(aResult.count())
    {
        ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aResult);
        rOverlayManager.add(*pNew);
        addToOverlayObjectList(*pNew);
    }
}

/*************************************************************************

                                E3dDragRotate

*************************************************************************/

TYPEINIT1(E3dDragRotate, E3dDragMethod);

E3dDragRotate::E3dDragRotate(SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    sal_Bool bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull)
{
    // Get center of all selected objects in eye coordinates
    const sal_uInt32 nCnt(maGrp.size());

    if(nCnt)
    {
        const E3dScene *pScene = maGrp[0].mp3DObj->GetScene();

        if(pScene)
        {
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
            const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                basegfx::B3DPoint aObjCenter = rCandidate.mp3DObj->GetBoundVolume().getCenter();
                const basegfx::B3DHomMatrix aTransform(aViewInfo3D.getOrientation() * rCandidate.maDisplayTransform * rCandidate.maInitTransform);

                aObjCenter = aTransform * aObjCenter;
                maGlobalCenter += aObjCenter;
            }

            // Divide by the number
            if(nCnt > 1)
            {
                maGlobalCenter /= (double)nCnt;
            }

            // get rotate center and transform to 3D eye coordinates
            basegfx::B2DPoint aRotCenter2D(Ref1().X(), Ref1().Y());

            // from world to relative scene using inverse getObjectTransformation()
            basegfx::B2DHomMatrix aInverseObjectTransform(rVCScene.getObjectTransformation());
            aInverseObjectTransform.invert();
            aRotCenter2D = aInverseObjectTransform * aRotCenter2D;

            // from 3D view to 3D eye
            basegfx::B3DPoint aRotCenter3D(aRotCenter2D.getX(), aRotCenter2D.getY(), 0.0);
            basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());
            aInverseViewToEye.invert();
            aRotCenter3D = aInverseViewToEye * aRotCenter3D;

        // Use X,Y of the RotCenter and depth of the common object centre
        // as rotation point in the space
            maGlobalCenter.setX(aRotCenter3D.getX());
            maGlobalCenter.setY(aRotCenter3D.getY());
        }
    }
}


//The object is moved, determine the angle

void E3dDragRotate::MoveSdrDrag(const Point& rPnt)
{
    // call parent
    E3dDragMethod::MoveSdrDrag(rPnt);

    if(DragStat().CheckMinMoved(rPnt))
    {
        // Get modifier
        sal_uInt16 nModifier = 0;
        if(getSdrDragView().ISA(E3dView))
        {
            const MouseEvent& rLastMouse = ((E3dView&)getSdrDragView()).GetMouseEvent();
            nModifier = rLastMouse.GetModifier();
        }

        // Rotate all objects
        const sal_uInt32 nCnt(maGrp.size());

        for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
        {
            // Determine rotation angle
            double fWAngle, fHAngle;
            E3dDragMethodUnit& rCandidate = maGrp[nOb];

            if(E3DDRAG_CONSTR_Z == meConstraint)
            {
                fWAngle = NormAngle360(GetAngle(rPnt - DragStat().GetRef1()) -
                    rCandidate.mnStartAngle) - rCandidate.mnLastAngle;
                rCandidate.mnLastAngle = (long)fWAngle + rCandidate.mnLastAngle;
                fWAngle /= 100.0;
                fHAngle = 0.0;
            }
            else
            {
                fWAngle = 90.0 * (double)(rPnt.X() - maLastPos.X())
                    / (double)maFullBound.GetWidth();
                fHAngle = 90.0 * (double)(rPnt.Y() - maLastPos.Y())
                    / (double)maFullBound.GetHeight();
            }
            long nSnap = 0;

            if(!getSdrDragView().IsRotateAllowed(sal_False))
                nSnap = 90;

            if(nSnap != 0)
            {
                fWAngle = (double)(((long) fWAngle + nSnap/2) / nSnap * nSnap);
                fHAngle = (double)(((long) fHAngle + nSnap/2) / nSnap * nSnap);
            }

            // to radians
            fWAngle *= F_PI180;
            fHAngle *= F_PI180;

            // Determine transformation
            basegfx::B3DHomMatrix aRotMat;
            if(E3DDRAG_CONSTR_Y & meConstraint)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, 0.0, fWAngle);
                else
                    aRotMat.rotate(0.0, fWAngle, 0.0);
            }
            else if(E3DDRAG_CONSTR_Z & meConstraint)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, fWAngle, 0.0);
                else
                    aRotMat.rotate(0.0, 0.0, fWAngle);
            }
            if(E3DDRAG_CONSTR_X & meConstraint)
            {
                aRotMat.rotate(fHAngle, 0.0, 0.0);
            }

            // Transformation in eye coordinates, there rotate then and back
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rCandidate.mp3DObj->GetScene()->GetViewContact());
            const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
            basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
            aInverseOrientation.invert();

            basegfx::B3DHomMatrix aTransMat(rCandidate.maDisplayTransform);
            aTransMat *= aViewInfo3D.getOrientation();
            aTransMat.translate(-maGlobalCenter.getX(), -maGlobalCenter.getY(), -maGlobalCenter.getZ());
            aTransMat *= aRotMat;
            aTransMat.translate(maGlobalCenter.getX(), maGlobalCenter.getY(), maGlobalCenter.getZ());
            aTransMat *= aInverseOrientation;
            aTransMat *= rCandidate.maInvDisplayTransform;

            // ...and apply
            rCandidate.maTransform *= aTransMat;

            if(mbMoveFull)
            {
                E3DModifySceneSnapRectUpdater aUpdater(rCandidate.mp3DObj);
                rCandidate.mp3DObj->SetTransform(rCandidate.maTransform);
            }
            else
            {
                Hide();
                rCandidate.maWireframePoly.transform(aTransMat);
                Show();
            }
        }
        maLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

/*************************************************************************
|*
\************************************************************************/

Pointer E3dDragRotate::GetSdrDragPointer() const
{
    return Pointer(POINTER_ROTATE);
}

// E3dDragMove. This drag method is only required for translations inside
// 3D scenes. If a 3D-scene itself moved, then this drag method will drag
// not be used.

TYPEINIT1(E3dDragMove, E3dDragMethod);

E3dDragMove::E3dDragMove(SdrDragView &_rView,
    const SdrMarkList& rMark,
    SdrHdlKind eDrgHdl,
    E3dDragConstraint eConstr,
    sal_Bool bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull),
    meWhatDragHdl(eDrgHdl)
{
    switch(meWhatDragHdl)
    {
        case HDL_LEFT:
            maScaleFixPos = maFullBound.RightCenter();
            break;
        case HDL_RIGHT:
            maScaleFixPos = maFullBound.LeftCenter();
            break;
        case HDL_UPPER:
            maScaleFixPos = maFullBound.BottomCenter();
            break;
        case HDL_LOWER:
            maScaleFixPos = maFullBound.TopCenter();
            break;
        case HDL_UPLFT:
            maScaleFixPos = maFullBound.BottomRight();
            break;
        case HDL_UPRGT:
            maScaleFixPos = maFullBound.BottomLeft();
            break;
        case HDL_LWLFT:
            maScaleFixPos = maFullBound.TopRight();
            break;
        case HDL_LWRGT:
            maScaleFixPos = maFullBound.TopLeft();
            break;
        default:
            // Moving the object, HDL_MOVE
            break;
    }

    // Override when IsResizeAtCenter()
    if(getSdrDragView().IsResizeAtCenter())
    {
        meWhatDragHdl = HDL_USER;
        maScaleFixPos = maFullBound.Center();
    }
}

// The object is moved, determine the translations

void E3dDragMove::MoveSdrDrag(const Point& rPnt)
{
    // call parent
    E3dDragMethod::MoveSdrDrag(rPnt);

    if(DragStat().CheckMinMoved(rPnt))
    {
        if(HDL_MOVE == meWhatDragHdl)
        {
            // Translation
            // Determine the motion vector
            basegfx::B3DPoint aGlobalMoveHead((double)(rPnt.X() - maLastPos.X()), (double)(rPnt.Y() - maLastPos.Y()), 32768.0);
            basegfx::B3DPoint aGlobalMoveTail(0.0, 0.0, 32768.0);
            const sal_uInt32 nCnt(maGrp.size());

            // Get modifier
            sal_uInt16 nModifier(0);

            if(getSdrDragView().ISA(E3dView))
            {
                const MouseEvent& rLastMouse = ((E3dView&)getSdrDragView()).GetMouseEvent();
                nModifier = rLastMouse.GetModifier();
            }

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rCandidate.mp3DObj->GetScene()->GetViewContact());
                const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());

                // move coor from 2d world to 3d Eye
                basegfx::B2DPoint aGlobalMoveHead2D((double)(rPnt.X() - maLastPos.X()), (double)(rPnt.Y() - maLastPos.Y()));
                basegfx::B2DPoint aGlobalMoveTail2D(0.0, 0.0);
                basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());

                aInverseSceneTransform.invert();
                aGlobalMoveHead2D = aInverseSceneTransform * aGlobalMoveHead2D;
                aGlobalMoveTail2D = aInverseSceneTransform * aGlobalMoveTail2D;

                basegfx::B3DPoint aMoveHead3D(aGlobalMoveHead2D.getX(), aGlobalMoveHead2D.getY(), 0.5);
                basegfx::B3DPoint aMoveTail3D(aGlobalMoveTail2D.getX(), aGlobalMoveTail2D.getY(), 0.5);
                basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());
                aInverseViewToEye.invert();

                aMoveHead3D = aInverseViewToEye * aMoveHead3D;
                aMoveTail3D = aInverseViewToEye * aMoveTail3D;

                // eventually switch movement from XY to XZ plane
                if(nModifier & KEY_MOD2)
                {
                    double fZwi = aMoveHead3D.getY();
                    aMoveHead3D.setY(aMoveHead3D.getZ());
                    aMoveHead3D.setZ(fZwi);

                    fZwi = aMoveTail3D.getY();
                    aMoveTail3D.setY(aMoveTail3D.getZ());
                    aMoveTail3D.setZ(fZwi);
                }

                // Motion vector from eye coordinates to parent coordinates
                basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
                aInverseOrientation.invert();
                basegfx::B3DHomMatrix aCompleteTrans(rCandidate.maInvDisplayTransform * aInverseOrientation);

                aMoveHead3D = aCompleteTrans * aMoveHead3D;
                aMoveTail3D = aCompleteTrans* aMoveTail3D;

                // build transformation
                basegfx::B3DHomMatrix aTransMat;
                basegfx::B3DPoint aTranslate(aMoveHead3D - aMoveTail3D);
                aTransMat.translate(aTranslate.getX(), aTranslate.getY(), aTranslate.getZ());

                // ...and apply
                rCandidate.maTransform *= aTransMat;

                if(mbMoveFull)
                {
                    E3DModifySceneSnapRectUpdater aUpdater(rCandidate.mp3DObj);
                    rCandidate.mp3DObj->SetTransform(rCandidate.maTransform);
                }
                else
                {
                    Hide();
                    rCandidate.maWireframePoly.transform(aTransMat);
                    Show();
                }
            }
        }
        else
        {
            // Scaling
            // Determine scaling vector
            Point aStartPos = DragStat().GetStart();
            const sal_uInt32 nCnt(maGrp.size());

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                const basegfx::B3DPoint aObjectCenter(rCandidate.mp3DObj->GetBoundVolume().getCenter());

                // transform from 2D world view to 3D eye
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rCandidate.mp3DObj->GetScene()->GetViewContact());
                const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());

                basegfx::B2DPoint aGlobalScaleStart2D((double)(aStartPos.X()), (double)(aStartPos.Y()));
                basegfx::B2DPoint aGlobalScaleNext2D((double)(rPnt.X()), (double)(rPnt.Y()));
                basegfx::B2DPoint aGlobalScaleFixPos2D((double)(maScaleFixPos.X()), (double)(maScaleFixPos.Y()));
                basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());

                aInverseSceneTransform.invert();
                aGlobalScaleStart2D = aInverseSceneTransform * aGlobalScaleStart2D;
                aGlobalScaleNext2D = aInverseSceneTransform * aGlobalScaleNext2D;
                aGlobalScaleFixPos2D = aInverseSceneTransform * aGlobalScaleFixPos2D;

                basegfx::B3DPoint aGlobalScaleStart3D(aGlobalScaleStart2D.getX(), aGlobalScaleStart2D.getY(), aObjectCenter.getZ());
                basegfx::B3DPoint aGlobalScaleNext3D(aGlobalScaleNext2D.getX(), aGlobalScaleNext2D.getY(), aObjectCenter.getZ());
                basegfx::B3DPoint aGlobalScaleFixPos3D(aGlobalScaleFixPos2D.getX(), aGlobalScaleFixPos2D.getY(), aObjectCenter.getZ());
                basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());

                aInverseViewToEye.invert();
                basegfx::B3DPoint aScStart(aInverseViewToEye * aGlobalScaleStart3D);
                basegfx::B3DPoint aScNext(aInverseViewToEye * aGlobalScaleNext3D);
                basegfx::B3DPoint aScFixPos(aInverseViewToEye * aGlobalScaleFixPos3D);

                // constraints?
                switch(meWhatDragHdl)
                {
                    case HDL_LEFT:
                    case HDL_RIGHT:
                        // to constrain on X -> Y equal
                        aScNext.setY(aScFixPos.getY());
                        break;
                    case HDL_UPPER:
                    case HDL_LOWER:
                        // constrain to auf Y -> X equal
                        aScNext.setX(aScFixPos.getX());
                        break;
                    default:
                        break;
                }

                // get scale vector in eye coordinates
                basegfx::B3DPoint aScaleVec(aScStart - aScFixPos);
                aScaleVec.setZ(1.0);

                if(aScaleVec.getX() != 0.0)
                {
                    aScaleVec.setX((aScNext.getX() - aScFixPos.getX()) / aScaleVec.getX());
                }
                else
                {
                    aScaleVec.setX(1.0);
                }

                if(aScaleVec.getY() != 0.0)
                {
                    aScaleVec.setY((aScNext.getY() - aScFixPos.getY()) / aScaleVec.getY());
                }
                else
                {
                    aScaleVec.setY(1.0);
                }

                // SHIFT-key used?
                if(getSdrDragView().IsOrtho())
                {
                    if(fabs(aScaleVec.getX()) > fabs(aScaleVec.getY()))
                    {
                        // X is biggest
                        aScaleVec.setY(aScaleVec.getX());
                    }
                    else
                    {
                        // Y is biggest
                        aScaleVec.setX(aScaleVec.getY());
                    }
                }

                // build transformation
                basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
                aInverseOrientation.invert();

                basegfx::B3DHomMatrix aNewTrans = rCandidate.maInitTransform;
                aNewTrans *= rCandidate.maDisplayTransform;
                aNewTrans *= aViewInfo3D.getOrientation();
                aNewTrans.translate(-aScFixPos.getX(), -aScFixPos.getY(), -aScFixPos.getZ());
                aNewTrans.scale(aScaleVec.getX(), aScaleVec.getY(), aScaleVec.getZ());
                aNewTrans.translate(aScFixPos.getX(), aScFixPos.getY(), aScFixPos.getZ());
                aNewTrans *= aInverseOrientation;
                aNewTrans *= rCandidate.maInvDisplayTransform;

                // ...and apply
                rCandidate.maTransform = aNewTrans;

                if(mbMoveFull)
                {
                    E3DModifySceneSnapRectUpdater aUpdater(rCandidate.mp3DObj);
                    rCandidate.mp3DObj->SetTransform(rCandidate.maTransform);
                }
                else
                {
                    Hide();
                    rCandidate.maWireframePoly.clear();
                    rCandidate.maWireframePoly = rCandidate.mp3DObj->CreateWireframe();
                    rCandidate.maWireframePoly.transform(rCandidate.maTransform);
                    Show();
                }
            }
        }
        maLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

/*************************************************************************
|*
\************************************************************************/

Pointer E3dDragMove::GetSdrDragPointer() const
{
    return Pointer(POINTER_MOVE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
