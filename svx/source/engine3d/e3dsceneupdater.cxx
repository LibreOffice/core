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

#include <svx/e3dsceneupdater.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>

//////////////////////////////////////////////////////////////////////////////

E3DModifySceneSnapRectUpdater::E3DModifySceneSnapRectUpdater(const SdrObject* pObject)
:   mpScene(0),
    mpViewInformation3D(0)
{
    // Secure old 3D transformation stack before modification
    if(pObject)
    {
        const E3dObject* pE3dObject = dynamic_cast< const E3dObject* >(pObject);

        if(pE3dObject)
        {
            mpScene = pE3dObject->GetScene();

            if(mpScene && mpScene->GetScene() == mpScene)
            {
                // if there is a scene and it's the outmost scene, get current 3D range
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(mpScene->GetViewContact());
                const basegfx::B3DRange aAllContentRange(rVCScene.getAllContentRange3D());

                if(aAllContentRange.isEmpty())
                {
                    // no content, nothing to do
                    mpScene = 0;
                }
                else
                {
                    // secure current 3D transformation stack
                    mpViewInformation3D = new drawinglayer::geometry::ViewInformation3D(rVCScene.getViewInformation3D(aAllContentRange));
                }
            }
        }
    }
}

E3DModifySceneSnapRectUpdater::~E3DModifySceneSnapRectUpdater()
{
    if(mpScene && mpViewInformation3D)
    {
        // after changing parts of the scene, use the secured last 3d transformation stack and the new content
        // range to calculate a new, eventually expanded or shrunk, 2D geometry for the scene and apply it.
        // Get new content range
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(mpScene->GetViewContact());
        basegfx::B3DRange aAllContentRange(rVCScene.getAllContentRange3D());

        // only change when there is still content; else let scene stay at old SnapRect
        if(!aAllContentRange.isEmpty())
        {
            // check if object transform of scene has changed
            if(mpViewInformation3D->getObjectTransformation() != mpScene->GetTransform())
            {
                // If Yes, it needs to be updated since it's - for historical reasons -
                // part of the basic 3d transformation stack of the scene
                drawinglayer::geometry::ViewInformation3D* pNew = new drawinglayer::geometry::ViewInformation3D(
                    mpScene->GetTransform(), // replace object transformation with new local transform
                    mpViewInformation3D->getOrientation(),
                    mpViewInformation3D->getProjection(),
                    mpViewInformation3D->getDeviceToView(),
                    mpViewInformation3D->getViewTime(),
                    mpViewInformation3D->getExtendedInformationSequence());
                delete mpViewInformation3D;
                mpViewInformation3D = pNew;
            }

            // transform content range to scene-relative coordinates using old 3d transformation stack
            aAllContentRange.transform(mpViewInformation3D->getObjectToView());

            // build 2d relative content range
            basegfx::B2DRange aSnapRange(
                aAllContentRange.getMinX(), aAllContentRange.getMinY(),
                aAllContentRange.getMaxX(), aAllContentRange.getMaxY());

            // transform to 2D world coordiantes using scene's 2D transformation
            aSnapRange.transform(rVCScene.getObjectTransformation());

            // snap to (old) integer
            const Rectangle aNewSnapRect(
                sal_Int32(floor(aSnapRange.getMinX())), sal_Int32(floor(aSnapRange.getMinY())),
                sal_Int32(ceil(aSnapRange.getMaxX())), sal_Int32(ceil(aSnapRange.getMaxY())));

            // set as new SnapRect and invalidate bound volume
            if(mpScene->GetSnapRect() != aNewSnapRect)
            {
                mpScene->SetSnapRect(aNewSnapRect);
                mpScene->InvalidateBoundVolume();
            }
        }
    }

    delete mpViewInformation3D;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
