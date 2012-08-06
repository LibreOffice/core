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


#include <svx/helperhittest3d.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/svdpage.hxx>
#include <svx/scene3d.hxx>
#include <svx/svditer.hxx>
#include <drawinglayer/processor3d/cutfindprocessor3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <com/sun/star/uno/Sequence.h>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

class ImplPairDephAndObject
{
private:
    const E3dCompoundObject*    mpObject;
    double                      mfDepth;

public:
    ImplPairDephAndObject(const E3dCompoundObject* pObject, double fDepth)
    :   mpObject(pObject),
        mfDepth(fDepth)
    {}

    // for ::std::sort
    bool operator<(const ImplPairDephAndObject& rComp) const
    {
        return (mfDepth < rComp.mfDepth);
    }

    // data read access
    const E3dCompoundObject* getObject() const { return mpObject; }
    double getDepth() const { return mfDepth; }
};

//////////////////////////////////////////////////////////////////////////////

void getAllHit3DObjectWithRelativePoint(
    const basegfx::B3DPoint& rFront,
    const basegfx::B3DPoint& rBack,
    const E3dCompoundObject& rObject,
    const drawinglayer::geometry::ViewInformation3D& rObjectViewInformation3D,
    ::std::vector< basegfx::B3DPoint >& o_rResult,
    bool bAnyHit)
{
    o_rResult.clear();

    if(!rFront.equal(rBack))
    {
        // rObject is a E3dCompoundObject, so it cannot be a scene (which is a E3dObject)
        const sdr::contact::ViewContactOfE3d& rVCObject = static_cast< sdr::contact::ViewContactOfE3d& >(rObject.GetViewContact());
        const drawinglayer::primitive3d::Primitive3DSequence aPrimitives(rVCObject.getViewIndependentPrimitive3DSequence());

        if(aPrimitives.hasElements())
        {
            // make BoundVolume empty and overlapping test for speedup
            const basegfx::B3DRange aObjectRange(drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(aPrimitives, rObjectViewInformation3D));

            if(!aObjectRange.isEmpty())
            {
                const basegfx::B3DRange aFrontBackRange(rFront, rBack);

                if(aObjectRange.overlaps(aFrontBackRange))
                {
                    // bound volumes hit, geometric cut tests needed
                    drawinglayer::processor3d::CutFindProcessor aCutFindProcessor(rObjectViewInformation3D, rFront, rBack, bAnyHit);
                    aCutFindProcessor.process(aPrimitives);
                    o_rResult = aCutFindProcessor.getCutPoints();
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

E3dScene* fillViewInformation3DForCompoundObject(drawinglayer::geometry::ViewInformation3D& o_rViewInformation3D, const E3dCompoundObject& rCandidate)
{
    // Search for root scene (outmost scene) of the 3d object since e.g. in chart, multiple scenes may
    // be placed between object and outmost scene. On that search, remember the in-between scene's
    // transformation for the correct complete ObjectTransformation. For historical reasons, the
    // root scene's own object transformation is part of the scene's ViewTransformation, o do not
    // add it. For more details, see ViewContactOfE3dScene::createViewInformation3D.
    E3dScene* pParentScene = dynamic_cast< E3dScene* >(rCandidate.GetParentObj());
    E3dScene* pRootScene = 0;
    basegfx::B3DHomMatrix aInBetweenSceneMatrix;

    while(pParentScene)
    {
        E3dScene* pParentParentScene = dynamic_cast< E3dScene* >(pParentScene->GetParentObj());

        if(pParentParentScene)
        {
            // pParentScene is a in-between scene
            aInBetweenSceneMatrix = pParentScene->GetTransform() * aInBetweenSceneMatrix;
        }
        else
        {
            // pParentScene is the root scene
            pRootScene = pParentScene;
        }

        pParentScene = pParentParentScene;
    }

    if(pRootScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());

        if(aInBetweenSceneMatrix.isIdentity())
        {
            o_rViewInformation3D = rVCScene.getViewInformation3D();
        }
        else
        {
            // build new ViewInformation containing all transforms for the candidate
            const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());

            o_rViewInformation3D = drawinglayer::geometry::ViewInformation3D(
                aViewInfo3D.getObjectTransformation() * aInBetweenSceneMatrix,
                aViewInfo3D.getOrientation(),
                aViewInfo3D.getProjection(),
                aViewInfo3D.getDeviceToView(),
                aViewInfo3D.getViewTime(),
                aViewInfo3D.getExtendedInformationSequence());
        }
    }
    else
    {
        const uno::Sequence< beans::PropertyValue > aEmptyParameters;
        o_rViewInformation3D = drawinglayer::geometry::ViewInformation3D(aEmptyParameters);
    }

    return pRootScene;
}

//////////////////////////////////////////////////////////////////////////////

SVX_DLLPUBLIC void getAllHit3DObjectsSortedFrontToBack(
    const basegfx::B2DPoint& rPoint,
    const E3dScene& rScene,
    ::std::vector< const E3dCompoundObject* >& o_rResult)
{
    o_rResult.clear();
    SdrObjList* pList = rScene.GetSubList();

    if(pList && pList->GetObjCount())
    {
        // prepare relative HitPoint. To do so, get the VC of the 3DScene and from there
        // the Scene's 2D transformation. Multiplying with the inverse transformation
        // will create a point relative to the 3D scene as unit-2d-object
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rScene.GetViewContact());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());
        aInverseSceneTransform.invert();
        const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * rPoint);

        // check if test point is inside scene's area at all
        if(aRelativePoint.getX() >= 0.0 && aRelativePoint.getX() <= 1.0 && aRelativePoint.getY() >= 0.0 && aRelativePoint.getY() <= 1.0)
        {
            SdrObjListIter aIterator(*pList, IM_DEEPNOGROUPS);
            ::std::vector< ImplPairDephAndObject > aDepthAndObjectResults;
            const uno::Sequence< beans::PropertyValue > aEmptyParameters;
            drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);

            while(aIterator.IsMore())
            {
                const E3dCompoundObject* pCandidate = dynamic_cast< const E3dCompoundObject* >(aIterator.Next());

                if(pCandidate)
                {
                    fillViewInformation3DForCompoundObject(aViewInfo3D, *pCandidate);

                    // create HitPoint Front and Back, transform to object coordinates
                    basegfx::B3DHomMatrix aViewToObject(aViewInfo3D.getObjectToView());
                    aViewToObject.invert();
                    const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
                    const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

                    if(!aFront.equal(aBack))
                    {
                        // get all hit points with object
                        ::std::vector< basegfx::B3DPoint > aHitsWithObject;
                        getAllHit3DObjectWithRelativePoint(aFront, aBack, *pCandidate, aViewInfo3D, aHitsWithObject, false);

                        for(sal_uInt32 a(0); a < aHitsWithObject.size(); a++)
                        {
                            const basegfx::B3DPoint aPointInViewCoordinates(aViewInfo3D.getObjectToView() * aHitsWithObject[a]);
                            aDepthAndObjectResults.push_back(ImplPairDephAndObject(pCandidate, aPointInViewCoordinates.getZ()));
                        }
                    }
                }
            }

            // fill nRetval
            const sal_uInt32 nCount(aDepthAndObjectResults.size());

            if(nCount)
            {
                // sort aDepthAndObjectResults by depth
                ::std::sort(aDepthAndObjectResults.begin(), aDepthAndObjectResults.end());

                // copy SdrObject pointers to return result set
                ::std::vector< ImplPairDephAndObject >::iterator aIterator2(aDepthAndObjectResults.begin());

                for(;aIterator2 != aDepthAndObjectResults.end(); ++aIterator2)
                {
                    o_rResult.push_back(aIterator2->getObject());
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

bool checkHitSingle3DObject(
    const basegfx::B2DPoint& rPoint,
    const E3dCompoundObject& rCandidate)
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, rCandidate);

    if(pRootScene)
    {
        // prepare relative HitPoint. To do so, get the VC of the 3DScene and from there
        // the Scene's 2D transformation. Multiplying with the inverse transformation
        // will create a point relative to the 3D scene as unit-2d-object
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());
        aInverseSceneTransform.invert();
        const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * rPoint);

        // check if test point is inside scene's area at all
        if(aRelativePoint.getX() >= 0.0 && aRelativePoint.getX() <= 1.0 && aRelativePoint.getY() >= 0.0 && aRelativePoint.getY() <= 1.0)
        {
            // create HitPoint Front and Back, transform to object coordinates
            basegfx::B3DHomMatrix aViewToObject(aViewInfo3D.getObjectToView());
            aViewToObject.invert();
            const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
            const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

            if(!aFront.equal(aBack))
            {
                // get all hit points with object
                ::std::vector< basegfx::B3DPoint > aHitsWithObject;
                getAllHit3DObjectWithRelativePoint(aFront, aBack, rCandidate, aViewInfo3D, aHitsWithObject, true);

                if(!aHitsWithObject.empty())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
