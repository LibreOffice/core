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

    
    bool operator<(const ImplPairDephAndObject& rComp) const
    {
        return (mfDepth < rComp.mfDepth);
    }

    
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
        
        const sdr::contact::ViewContactOfE3d& rVCObject = static_cast< sdr::contact::ViewContactOfE3d& >(rObject.GetViewContact());
        const drawinglayer::primitive3d::Primitive3DSequence aPrimitives(rVCObject.getViewIndependentPrimitive3DSequence());

        if(aPrimitives.hasElements())
        {
            
            const basegfx::B3DRange aObjectRange(drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(aPrimitives, rObjectViewInformation3D));

            if(!aObjectRange.isEmpty())
            {
                const basegfx::B3DRange aFrontBackRange(rFront, rBack);

                if(aObjectRange.overlaps(aFrontBackRange))
                {
                    
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
    
    
    
    
    
    E3dScene* pParentScene = dynamic_cast< E3dScene* >(rCandidate.GetParentObj());
    E3dScene* pRootScene = 0;
    basegfx::B3DHomMatrix aInBetweenSceneMatrix;

    while(pParentScene)
    {
        E3dScene* pParentParentScene = dynamic_cast< E3dScene* >(pParentScene->GetParentObj());

        if(pParentParentScene)
        {
            
            aInBetweenSceneMatrix = pParentScene->GetTransform() * aInBetweenSceneMatrix;
        }
        else
        {
            
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
        
        
        
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(rScene.GetViewContact());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());
        aInverseSceneTransform.invert();
        const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * rPoint);

        
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

                    
                    basegfx::B3DHomMatrix aViewToObject(aViewInfo3D.getObjectToView());
                    aViewToObject.invert();
                    const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
                    const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

                    if(!aFront.equal(aBack))
                    {
                        
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

            
            const sal_uInt32 nCount(aDepthAndObjectResults.size());

            if(nCount)
            {
                
                ::std::sort(aDepthAndObjectResults.begin(), aDepthAndObjectResults.end());

                
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
        
        
        
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());
        aInverseSceneTransform.invert();
        const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * rPoint);

        
        if(aRelativePoint.getX() >= 0.0 && aRelativePoint.getX() <= 1.0 && aRelativePoint.getY() >= 0.0 && aRelativePoint.getY() <= 1.0)
        {
            
            basegfx::B3DHomMatrix aViewToObject(aViewInfo3D.getObjectToView());
            aViewToObject.invert();
            const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
            const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

            if(!aFront.equal(aBack))
            {
                
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
