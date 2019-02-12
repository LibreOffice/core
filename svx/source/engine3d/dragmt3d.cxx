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


#include <dragmt3d.hxx>
#include <o3tl/numeric.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdtrans.hxx>
#include <svx/obj3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/strings.hrc>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <vcl/ptrstyle.hxx>


E3dDragMethod::E3dDragMethod (
    SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    bool bFull)
:   SdrDragMethod(_rView),
    meConstraint(eConstr),
    mbMoveFull(bFull),
    mbMovedAtAll(false)
{
    // Create a unit for all the 3D objects present in the selection
    const size_t nCnt(rMark.GetMarkCount());

    if(mbMoveFull)
    {
        // for non-visible 3D objects fallback to wireframe interaction
        for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
        {
            E3dObject* pE3dObj = dynamic_cast< E3dObject* >(rMark.GetMark(nObjs)->GetMarkedSdrObj());

            if(pE3dObj)
            {
                if(!pE3dObj->HasFillStyle() && !pE3dObj->HasLineStyle())
                {
                    mbMoveFull = false;
                    break;
                }
            }
        }
    }

    for(size_t nObjs = 0; nObjs < nCnt; ++nObjs)
    {
        E3dObject* pE3dObj = dynamic_cast< E3dObject* >(rMark.GetMark(nObjs)->GetMarkedSdrObj());

        if(pE3dObj)
        {
            // fill new interaction unit
            E3dDragMethodUnit aNewUnit(*pE3dObj);

            // get transformations
            aNewUnit.maInitTransform = aNewUnit.maTransform = pE3dObj->GetTransform();

            if(nullptr != pE3dObj->getParentE3dSceneFromE3dObject())
            {
                // get transform between object and world, normally scene transform
                aNewUnit.maInvDisplayTransform = aNewUnit.maDisplayTransform = pE3dObj->getParentE3dSceneFromE3dObject()->GetFullTransform();
                aNewUnit.maInvDisplayTransform.invert();
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

void E3dDragMethod::TakeSdrDragComment(OUString& /*rStr*/) const
{
}

// Create the wireframe model for all actions

bool E3dDragMethod::BeginSdrDrag()
{
    if(E3dDragConstraint::Z == meConstraint)
    {
        const sal_uInt32 nCnt(maGrp.size());
        DragStat().SetRef1( maFullBound.Center() );

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

    return true;
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
            getSdrDragView().BegUndo(SvxResId(RID_SVX_3D_UNDO_ROTATE));
        sal_uInt32 nOb(0);

        for(nOb=0;nOb<nCnt;nOb++)
        {
            E3dDragMethodUnit& rCandidate = maGrp[nOb];
            E3DModifySceneSnapRectUpdater aUpdater(&rCandidate.mr3DObj);
            rCandidate.mr3DObj.SetTransform(rCandidate.maTransform);
            if( bUndo )
            {
                getSdrDragView().AddUndo(
                    std::make_unique<E3dRotateUndoAction>(
                        rCandidate.mr3DObj,
                        rCandidate.maInitTransform,
                        rCandidate.maTransform));
            }
        }
        if( bUndo )
            getSdrDragView().EndUndo();
    }

    return true;
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
                E3DModifySceneSnapRectUpdater aUpdater(&rCandidate.mr3DObj);
                rCandidate.mr3DObj.SetTransform(rCandidate.maInitTransform);
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
void E3dDragMethod::CreateOverlayGeometry(
    sdr::overlay::OverlayManager& rOverlayManager,
    const sdr::contact::ObjectContact& rObjectContact)
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
                const E3dScene* pScene(rCandidate.mr3DObj.getRootE3dSceneFromE3dObject());

                if(nullptr != pScene)
                {
                    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
                    const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());
                    const basegfx::B3DHomMatrix aWorldToView(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection() * aViewInfo3D.getOrientation());
                    const basegfx::B3DHomMatrix aTransform(aWorldToView * rCandidate.maDisplayTransform);

                    // transform to relative scene coordinates
                    basegfx::B2DPolyPolygon aPolyPolygon(basegfx::utils::createB2DPolyPolygonFromB3DPolyPolygon(aCandidate, aTransform));

                    // transform to 2D view coordinates
                    aPolyPolygon.transform(rVCScene.getObjectTransformation());

                    aResult.append(aPolyPolygon);
                }
            }
        }
    }

    if(aResult.count())
    {
        std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew(
            new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                aResult));

        insertNewlyCreatedOverlayObjectForSdrDragMethod(
            std::move(pNew),
            rObjectContact,
            rOverlayManager);
    }
}


E3dDragRotate::E3dDragRotate(SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    bool bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull)
{
    // Get center of all selected objects in eye coordinates
    const sal_uInt32 nCnt(maGrp.size());

    if(nCnt)
    {
        const E3dScene* pScene(maGrp[0].mr3DObj.getRootE3dSceneFromE3dObject());

        if(nullptr != pScene)
        {
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
            const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                basegfx::B3DPoint aObjCenter = rCandidate.mr3DObj.GetBoundVolume().getCenter();
                const basegfx::B3DHomMatrix aTransform(aViewInfo3D.getOrientation() * rCandidate.maDisplayTransform * rCandidate.maInitTransform);

                aObjCenter = aTransform * aObjCenter;
                maGlobalCenter += aObjCenter;
            }

            // Divide by the number
            if(nCnt > 1)
            {
                maGlobalCenter /= static_cast<double>(nCnt);
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
        if(dynamic_cast<const E3dView*>(&getSdrDragView()))
        {
            const MouseEvent& rLastMouse = static_cast<E3dView&>(getSdrDragView()).GetMouseEvent();
            nModifier = rLastMouse.GetModifier();
        }

        // Rotate all objects
        const sal_uInt32 nCnt(maGrp.size());

        for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
        {
            // Determine rotation angle
            double fWAngle, fHAngle;
            E3dDragMethodUnit& rCandidate = maGrp[nOb];

            if(E3dDragConstraint::Z == meConstraint)
            {
                fWAngle = NormAngle36000(GetAngle(rPnt - DragStat().GetRef1()) -
                    rCandidate.mnStartAngle) - rCandidate.mnLastAngle;
                rCandidate.mnLastAngle = static_cast<long>(fWAngle) + rCandidate.mnLastAngle;
                fWAngle /= 100.0;
                fHAngle = 0.0;
            }
            else
            {
                if ((maFullBound.GetWidth() == 0) || (maFullBound.GetHeight() == 0))
                    throw o3tl::divide_by_zero();
                fWAngle = 90.0 * static_cast<double>(rPnt.X() - maLastPos.X())
                    / static_cast<double>(maFullBound.GetWidth());
                fHAngle = 90.0 * static_cast<double>(rPnt.Y() - maLastPos.Y())
                    / static_cast<double>(maFullBound.GetHeight());
            }
            long nSnap = 0;

            if(!getSdrDragView().IsRotateAllowed())
                nSnap = 90;

            if(nSnap != 0)
            {
                fWAngle = static_cast<double>((static_cast<long>(fWAngle) + nSnap/2) / nSnap * nSnap);
                fHAngle = static_cast<double>((static_cast<long>(fHAngle) + nSnap/2) / nSnap * nSnap);
            }

            // to radians
            fWAngle = basegfx::deg2rad(fWAngle);
            fHAngle = basegfx::deg2rad(fHAngle);

            // Determine transformation
            basegfx::B3DHomMatrix aRotMat;
            if(E3dDragConstraint::Y & meConstraint)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, 0.0, fWAngle);
                else
                    aRotMat.rotate(0.0, fWAngle, 0.0);
            }
            else if(E3dDragConstraint::Z & meConstraint)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, fWAngle, 0.0);
                else
                    aRotMat.rotate(0.0, 0.0, fWAngle);
            }
            if(E3dDragConstraint::X & meConstraint)
            {
                aRotMat.rotate(fHAngle, 0.0, 0.0);
            }

            const E3dScene* pScene(rCandidate.mr3DObj.getRootE3dSceneFromE3dObject());

            if(nullptr != pScene)
            {
                // Transformation in eye coordinates, there rotate then and back
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
                const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());
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
                    E3DModifySceneSnapRectUpdater aUpdater(&rCandidate.mr3DObj);
                    rCandidate.mr3DObj.SetTransform(rCandidate.maTransform);
                }
                else
                {
                    Hide();
                    rCandidate.maWireframePoly.transform(aTransMat);
                    Show();
                }
            }
        }
        maLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

PointerStyle E3dDragRotate::GetSdrDragPointer() const
{
    return PointerStyle::Rotate;
}

// E3dDragMove. This drag method is only required for translations inside
// 3D scenes. If a 3D-scene itself moved, then this drag method will drag
// not be used.


E3dDragMove::E3dDragMove(SdrDragView &_rView,
    const SdrMarkList& rMark,
    SdrHdlKind eDrgHdl,
    E3dDragConstraint eConstr,
    bool bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull),
    meWhatDragHdl(eDrgHdl)
{
    switch(meWhatDragHdl)
    {
        case SdrHdlKind::Left:
            maScaleFixPos = maFullBound.RightCenter();
            break;
        case SdrHdlKind::Right:
            maScaleFixPos = maFullBound.LeftCenter();
            break;
        case SdrHdlKind::Upper:
            maScaleFixPos = maFullBound.BottomCenter();
            break;
        case SdrHdlKind::Lower:
            maScaleFixPos = maFullBound.TopCenter();
            break;
        case SdrHdlKind::UpperLeft:
            maScaleFixPos = maFullBound.BottomRight();
            break;
        case SdrHdlKind::UpperRight:
            maScaleFixPos = maFullBound.BottomLeft();
            break;
        case SdrHdlKind::LowerLeft:
            maScaleFixPos = maFullBound.TopRight();
            break;
        case SdrHdlKind::LowerRight:
            maScaleFixPos = maFullBound.TopLeft();
            break;
        default:
            // Moving the object, SdrHdlKind::Move
            break;
    }

    // Override when IsResizeAtCenter()
    if(getSdrDragView().IsResizeAtCenter())
    {
        meWhatDragHdl = SdrHdlKind::User;
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
        if(SdrHdlKind::Move == meWhatDragHdl)
        {
            // Translation
            // Determine the motion vector
            const sal_uInt32 nCnt(maGrp.size());

            // Get modifier
            sal_uInt16 nModifier(0);

            if(dynamic_cast<const E3dView*>(&getSdrDragView()))
            {
                const MouseEvent& rLastMouse = static_cast<E3dView&>(getSdrDragView()).GetMouseEvent();
                nModifier = rLastMouse.GetModifier();
            }

            for(sal_uInt32 nOb(0); nOb < nCnt; nOb++)
            {
                E3dDragMethodUnit& rCandidate = maGrp[nOb];
                const E3dScene* pScene(rCandidate.mr3DObj.getRootE3dSceneFromE3dObject());

                if(nullptr != pScene)
                {
                    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
                    const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());

                    // move coor from 2d world to 3d Eye
                    basegfx::B2DPoint aGlobalMoveHead2D(static_cast<double>(rPnt.X() - maLastPos.X()), static_cast<double>(rPnt.Y() - maLastPos.Y()));
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
                        E3DModifySceneSnapRectUpdater aUpdater(&rCandidate.mr3DObj);
                        rCandidate.mr3DObj.SetTransform(rCandidate.maTransform);
                    }
                    else
                    {
                        Hide();
                        rCandidate.maWireframePoly.transform(aTransMat);
                        Show();
                    }
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
                const basegfx::B3DPoint aObjectCenter(rCandidate.mr3DObj.GetBoundVolume().getCenter());
                const E3dScene* pScene(rCandidate.mr3DObj.getRootE3dSceneFromE3dObject());

                if(nullptr != pScene)
                {
                    // transform from 2D world view to 3D eye
                    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
                    const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());

                    basegfx::B2DPoint aGlobalScaleStart2D(static_cast<double>(aStartPos.X()), static_cast<double>(aStartPos.Y()));
                    basegfx::B2DPoint aGlobalScaleNext2D(static_cast<double>(rPnt.X()), static_cast<double>(rPnt.Y()));
                    basegfx::B2DPoint aGlobalScaleFixPos2D(static_cast<double>(maScaleFixPos.X()), static_cast<double>(maScaleFixPos.Y()));
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
                        case SdrHdlKind::Left:
                        case SdrHdlKind::Right:
                            // to constrain on X -> Y equal
                            aScNext.setY(aScFixPos.getY());
                            break;
                        case SdrHdlKind::Upper:
                        case SdrHdlKind::Lower:
                            // constrain to Y -> X equal
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
                        E3DModifySceneSnapRectUpdater aUpdater(&rCandidate.mr3DObj);
                        rCandidate.mr3DObj.SetTransform(rCandidate.maTransform);
                    }
                    else
                    {
                        Hide();
                        rCandidate.maWireframePoly.clear();
                        rCandidate.maWireframePoly = rCandidate.mr3DObj.CreateWireframe();
                        rCandidate.maWireframePoly.transform(rCandidate.maTransform);
                        Show();
                    }
                }
            }
        }
        maLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

PointerStyle E3dDragMove::GetSdrDragPointer() const
{
    return PointerStyle::Move;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
