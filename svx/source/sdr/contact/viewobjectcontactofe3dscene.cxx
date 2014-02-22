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


#include <svx/sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <basegfx/tools/canvastools.hxx>



using namespace com::sun::star;



namespace
{
    
    
    
    void impInternalSubHierarchyTraveller(const sdr::contact::ViewObjectContact& rVOC)
    {
        const sdr::contact::ViewContact& rVC = rVOC.GetViewContact();
        const sal_uInt32 nSubHierarchyCount(rVC.GetObjectCount());

        for(sal_uInt32 a(0); a < nSubHierarchyCount; a++)
        {
            const sdr::contact::ViewObjectContact& rCandidate(rVC.GetViewContact(a).GetViewObjectContact(rVOC.GetObjectContact()));
            impInternalSubHierarchyTraveller(rCandidate);
        }
    }
} 



namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfE3dScene::ViewObjectContactOfE3dScene(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfE3dScene::~ViewObjectContactOfE3dScene()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3dScene::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            
            const bool bHandleGhostedDisplay(GetObjectContact().DoVisualizeEnteredGroup() && !GetObjectContact().isOutputToPrinter() && rDisplayInfo.IsGhostedDrawModeActive());
            const bool bIsActiveVC(bHandleGhostedDisplay && GetObjectContact().getActiveViewContact() == &GetViewContact());

            if(bIsActiveVC)
            {
                
                const_cast< DisplayInfo& >(rDisplayInfo).ClearGhostedDrawMode();
            }

            
            
            
            
            const ViewContactOfE3dScene& rViewContact = dynamic_cast< ViewContactOfE3dScene& >(GetViewContact());
            const SetOfByte& rVisibleLayers = rDisplayInfo.GetProcessLayers();
            drawinglayer::primitive2d::Primitive2DSequence xRetval(rViewContact.createScenePrimitive2DSequence(&rVisibleLayers));

            if(xRetval.hasElements())
            {
                
                xRetval = rViewContact.embedToObjectSpecificInformation(xRetval);

                
                if(!GetObjectContact().isOutputToPrinter() && GetObjectContact().AreGluePointsVisible())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence xGlue(GetViewContact().createGluePointPrimitive2DSequence());

                    if(xGlue.hasElements())
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, xGlue);
                    }
                }

                
                if(isPrimitiveGhosted(rDisplayInfo))
                {
                    const ::basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                    const ::basegfx::BColorModifierSharedPtr aBColorModifier(
                        new basegfx::BColorModifier_interpolate(
                            aRGBWhite,
                            0.5));
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                            xRetval,
                            aBColorModifier));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            if(bIsActiveVC)
            {
                
                const_cast< DisplayInfo& >(rDisplayInfo).SetGhostedDrawMode();
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3dScene::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            
            
            
            impInternalSubHierarchyTraveller(*this);

            
            return ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo);
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
