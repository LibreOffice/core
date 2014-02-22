/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/obj3d.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/scene3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>



namespace
{
    const sdr::contact::ViewContactOfE3dScene* tryToFindVCOfE3DScene(
        const sdr::contact::ViewContact& rCandidate,
        basegfx::B3DHomMatrix& o_rInBetweenObjectTransform)
    {
        const sdr::contact::ViewContactOfE3dScene* pSceneParent =
            dynamic_cast< const sdr::contact::ViewContactOfE3dScene* >(rCandidate.GetParentContact());

        if(pSceneParent)
        {
            
            const sdr::contact::ViewContactOfE3dScene* pSceneParentParent =
                dynamic_cast< const sdr::contact::ViewContactOfE3dScene* >(pSceneParent->GetParentContact());

            if(pSceneParentParent)
            {
                
                
                const basegfx::B3DHomMatrix& rSceneParentTransform = pSceneParent->GetE3dScene().GetTransform();
                o_rInBetweenObjectTransform = rSceneParentTransform * o_rInBetweenObjectTransform;
                return tryToFindVCOfE3DScene(*pSceneParent, o_rInBetweenObjectTransform);
            }
            else
            {
                
                return pSceneParent;
            }
        }

        
        return 0;
    }
} 



namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3d::impCreateWithGivenPrimitive3DSequence(
            const drawinglayer::primitive3d::Primitive3DSequence& rxContent3D) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(rxContent3D.hasElements())
            {
                
                
                
                
                basegfx::B3DHomMatrix aInBetweenObjectTransform;
                const ViewContactOfE3dScene* pVCOfE3DScene = tryToFindVCOfE3DScene(*this, aInBetweenObjectTransform);

                if(pVCOfE3DScene)
                {
                    basegfx::B3DVector aLightNormal;
                    const double fShadowSlant(pVCOfE3DScene->getSdrSceneAttribute().getShadowSlant());
                    const basegfx::B3DRange& rAllContentRange = pVCOfE3DScene->getAllContentRange3D();
                    drawinglayer::geometry::ViewInformation3D aViewInformation3D(pVCOfE3DScene->getViewInformation3D());

                    if(pVCOfE3DScene->getSdrLightingAttribute().getLightVector().size())
                    {
                        
                        aLightNormal = pVCOfE3DScene->getSdrLightingAttribute().getLightVector()[0].getDirection();
                        aLightNormal.normalize();
                    }

                    if(!aInBetweenObjectTransform.isIdentity())
                    {
                        
                        
                        aViewInformation3D = drawinglayer::geometry::ViewInformation3D(
                            aViewInformation3D.getObjectTransformation() * aInBetweenObjectTransform,
                            aViewInformation3D.getOrientation(),
                            aViewInformation3D.getProjection(),
                            aViewInformation3D.getDeviceToView(),
                            aViewInformation3D.getViewTime(),
                            aViewInformation3D.getExtendedInformationSequence());
                    }

                    
                    
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::Embedded3DPrimitive2D(
                            rxContent3D,
                            pVCOfE3DScene->getObjectTransformation(),
                            aViewInformation3D,
                            aLightNormal,
                            fShadowSlant,
                            rAllContentRange));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            return xRetval;
        }

        ViewContactOfE3d::ViewContactOfE3d(E3dObject& rSdrObject)
        :   ViewContactOfSdrObj(rSdrObject)
        {
        }

        ViewContactOfE3d::~ViewContactOfE3d()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getVIP3DSWithoutObjectTransform() const
        {
            
            drawinglayer::primitive3d::Primitive3DSequence xNew(createViewIndependentPrimitive3DSequence());

            if(!drawinglayer::primitive3d::arePrimitive3DSequencesEqual(mxViewIndependentPrimitive3DSequence, xNew))
            {
                
                const_cast< ViewContactOfE3d* >(this)->mxViewIndependentPrimitive3DSequence = xNew;
            }

            
            return mxViewIndependentPrimitive3DSequence;
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getViewIndependentPrimitive3DSequence() const
        {
            
            drawinglayer::primitive3d::Primitive3DSequence xRetval(getVIP3DSWithoutObjectTransform());

            if(xRetval.hasElements())
            {
                
                const basegfx::B3DHomMatrix& rObjectTransform(GetE3dObject().GetTransform());

                if(!rObjectTransform.isIdentity())
                {
                    const drawinglayer::primitive3d::Primitive3DReference xReference(
                        new drawinglayer::primitive3d::TransformPrimitive3D(
                            rObjectTransform,
                            xRetval));

                    xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);
                }
            }

            
            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3d::createViewIndependentPrimitive2DSequence() const
        {
            
            
            
            return impCreateWithGivenPrimitive3DSequence(getViewIndependentPrimitive3DSequence());
        }

        ViewObjectContact& ViewContactOfE3d::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfE3d(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfE3d::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
