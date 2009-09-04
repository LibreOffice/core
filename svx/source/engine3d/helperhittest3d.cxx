/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: helperhittest3d.cxx,v $
 * $Revision: 1.1.2.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/helperhittest3d.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/svdpage.hxx>
#include <svx/scene3d.hxx>
#include <svditer.hxx>
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/hittestprimitive3d.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>

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

namespace drawinglayer
{
    namespace processor3d
    {
        class CutFindProcessor : public BaseProcessor3D
        {
        private:
            // the start and stop point for the cut vector
            basegfx::B3DPoint                       maFront;
            basegfx::B3DPoint                       maBack;

            // the found cut points
            ::std::vector< basegfx::B3DPoint >      maResult;

            // #i102956# the transformation change from TransformPrimitive3D processings
            // needs to be remembered to be able to transform found cuts to the
            // basic coordinate system the processor starts with
            basegfx::B3DHomMatrix                   maCombinedTransform;

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BasePrimitive3D-based.
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            CutFindProcessor(const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B3DPoint& rFront,
                const basegfx::B3DPoint& rBack)
            :   BaseProcessor3D(rViewInformation),
                maFront(rFront),
                maBack(rBack),
                maResult(),
                maCombinedTransform()
            {}

            // data access
            const ::std::vector< basegfx::B3DPoint >& getCutPoints() const { return maResult; }
        };

        void CutFindProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
            switch(rCandidate.getPrimitiveID())
            {
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group.
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);

                    // remember old and transform front, back to object coordinates
                    const basegfx::B3DPoint aLastFront(maFront);
                    const basegfx::B3DPoint aLastBack(maBack);
                    basegfx::B3DHomMatrix aInverseTrans(rPrimitive.getTransformation());
                    aInverseTrans.invert();
                    maFront *= aInverseTrans;
                    maBack *= aInverseTrans;

                    // remember current and create new transformation; add new object transform from right side
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    // #i102956# remember needed back-transform for found cuts (combine from right side)
                    const basegfx::B3DHomMatrix aLastCombinedTransform(maCombinedTransform);
                    maCombinedTransform = maCombinedTransform * rPrimitive.getTransformation();

                    // let break down
                    process(rPrimitive.getChildren());

                    // restore transformations and front, back
                    maCombinedTransform = aLastCombinedTransform;
                    updateViewInformation(aLastViewInformation3D);
                    maFront = aLastFront;
                    maBack = aLastBack;
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D, not used for hit test with planes, ignore. This
                    // means that also thick line expansion will not be hit-tested as
                    // PolyPolygonMaterialPrimitive3D
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    // #i97321#
                    // For HatchTexturePrimitive3D, do not use the decomposition since it will produce
                    // clipped hatch lines in 3D. It can be used when the hatch also has a filling, but for
                    // simplicity, just use the children which are the PolyPolygonMaterialPrimitive3D
                    // which define the hatched areas anyways; for HitTest this is more than adequate
                    const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rCandidate);
                    process(rPrimitive.getChildren());
                    break;
                }
                case PRIMITIVE3D_ID_HITTESTPRIMITIVE3D :
                {
                    // HitTestPrimitive3D, force usage due to we are doing a hit test and this
                    // primitive only gets generated on 3d objects without fill, exactly for this
                    // purpose
                    const primitive3d::HitTestPrimitive3D& rPrimitive = static_cast< const primitive3d::HitTestPrimitive3D& >(rCandidate);
                    process(rPrimitive.getChildren());
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);

                    if(!maFront.equal(maBack))
                    {
                           const basegfx::B3DPolyPolygon& rPolyPolygon = rPrimitive.getB3DPolyPolygon();
                        const sal_uInt32 nPolyCount(rPolyPolygon.count());

                        if(nPolyCount)
                        {
                               const basegfx::B3DPolygon aPolygon(rPolyPolygon.getB3DPolygon(0));
                            const sal_uInt32 nPointCount(aPolygon.count());

                            if(nPointCount > 2)
                            {
                                const basegfx::B3DVector aPlaneNormal(aPolygon.getNormal());

                                if(!aPlaneNormal.equalZero())
                                {
                                    const basegfx::B3DPoint aPointOnPlane(aPolygon.getB3DPoint(0));
                                    double fCut(0.0);

                                    if(basegfx::tools::getCutBetweenLineAndPlane(aPlaneNormal, aPointOnPlane, maFront, maBack, fCut))
                                    {
                                        const basegfx::B3DPoint aCutPoint(basegfx::interpolate(maFront, maBack, fCut));

                                        if(basegfx::tools::isInside(rPolyPolygon, aCutPoint, false))
                                        {
                                            // #i102956# add result. Do not forget to do this in the coordinate
                                            // system the processor get started with, so use the collected
                                            // combined transformation from processed TransformPrimitive3D's
                                            maResult.push_back(maCombinedTransform * aCutPoint);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get3DDecomposition(getViewInformation3D()));
                    break;
                }
            }
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

void getAllHit3DObjectWithRelativePoint(
    const basegfx::B3DPoint& rFront,
    const basegfx::B3DPoint& rBack,
    const E3dCompoundObject& rObject,
    const drawinglayer::geometry::ViewInformation3D& rObjectViewInformation3D,
    ::std::vector< basegfx::B3DPoint >& o_rResult)
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
                    drawinglayer::processor3d::CutFindProcessor aCutFindProcessor(rObjectViewInformation3D, rFront, rBack);
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
                        getAllHit3DObjectWithRelativePoint(aFront, aBack, *pCandidate, aViewInfo3D, aHitsWithObject);

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

                for(;aIterator2 != aDepthAndObjectResults.end(); aIterator2++)
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
                getAllHit3DObjectWithRelativePoint(aFront, aBack, rCandidate, aViewInfo3D, aHitsWithObject);

                if(aHitsWithObject.size())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// eof
