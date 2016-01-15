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


#include <svx/sdrhittesthelper.hxx>
#include <svx/obj3d.hxx>
#include <svx/helperhittest3d.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/processor2d/hittestprocessor2d.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdmodel.hxx>


// #i101872# new Object HitTest as View-tooling

SdrObject* SdrObjectPrimitiveHit(
    const SdrObject& rObject,
    const Point& rPnt,
    sal_uInt16 nTol,
    const SdrPageView& rSdrPageView,
    const SetOfByte* pVisiLayer,
    bool bTextOnly)
{
    SdrObject* pResult = nullptr;

    if(rObject.GetSubList() && rObject.GetSubList()->GetObjCount())
    {
        // group or scene with content. Single 3D objects also have a
        // true == rObject.GetSubList(), but no content
        pResult = SdrObjListPrimitiveHit(*rObject.GetSubList(), rPnt, nTol, rSdrPageView, pVisiLayer, bTextOnly);
    }
    else
    {
        if( rObject.IsVisible() && (!pVisiLayer || pVisiLayer->IsSet(rObject.GetLayer())))
        {
            // single object, 3d object, empty scene or empty group. Check if
            // it's a single 3D object
            const E3dCompoundObject* pE3dCompoundObject = dynamic_cast< const E3dCompoundObject* >(&rObject);

            if(pE3dCompoundObject)
            {
                const basegfx::B2DPoint aHitPosition(rPnt.X(), rPnt.Y());

                if(checkHitSingle3DObject(aHitPosition, *pE3dCompoundObject))
                {
                    pResult = const_cast< E3dCompoundObject* >(pE3dCompoundObject);
                }
            }
            else
            {
                // not a single 3D object; Check in first PageWindow using primitives (only SC
                // with split views uses multiple PageWindows nowadays)
                if(rSdrPageView.PageWindowCount())
                {
                    const double fLogicTolerance(nTol);
                    const basegfx::B2DPoint aHitPosition(rPnt.X(), rPnt.Y());
                    const sdr::contact::ViewObjectContact& rVOC = rObject.GetViewContact().GetViewObjectContact(
                        rSdrPageView.GetPageWindow(0)->GetObjectContact());

                    if(ViewObjectContactPrimitiveHit(rVOC, aHitPosition, fLogicTolerance, bTextOnly))
                    {
                          pResult = const_cast< SdrObject* >(&rObject);
                    }
                }
            }
        }
    }

    return pResult;
}


SdrObject* SdrObjListPrimitiveHit(
    const SdrObjList& rList,
    const Point& rPnt,
    sal_uInt16 nTol,
    const SdrPageView& rSdrPageView,
    const SetOfByte* pVisiLayer,
    bool bTextOnly)
{
    size_t nObjNum(rList.GetObjCount());
    SdrObject* pRetval = nullptr;

    while(!pRetval && nObjNum > 0)
    {
        nObjNum--;
        SdrObject* pObj = rList.GetObj(nObjNum);

        pRetval = SdrObjectPrimitiveHit(*pObj, rPnt, nTol, rSdrPageView, pVisiLayer, bTextOnly);
    }

    return pRetval;
}


bool ViewObjectContactPrimitiveHit(
    const sdr::contact::ViewObjectContact& rVOC,
    const basegfx::B2DPoint& rHitPosition,
    double fLogicHitTolerance,
    bool bTextOnly)
{
    basegfx::B2DRange aObjectRange(rVOC.getObjectRange());

    if(!aObjectRange.isEmpty())
    {
        // first do a rough B2DRange based HitTest; do not forget to
        // include the HitTolerance if given
        if(basegfx::fTools::more(fLogicHitTolerance, 0.0))
        {
            aObjectRange.grow(fLogicHitTolerance);
        }

        if(aObjectRange.isInside(rHitPosition))
        {
            // get primitive sequence
            sdr::contact::DisplayInfo aDisplayInfo;
            const drawinglayer::primitive2d::Primitive2DSequence& rSequence(rVOC.getPrimitive2DSequence(aDisplayInfo));

            if(rSequence.hasElements())
            {
                // create a HitTest processor
                const drawinglayer::geometry::ViewInformation2D& rViewInformation2D = rVOC.GetObjectContact().getViewInformation2D();
                drawinglayer::processor2d::HitTestProcessor2D aHitTestProcessor2D(
                    rViewInformation2D,
                    rHitPosition,
                    fLogicHitTolerance,
                    bTextOnly);

                // feed it with the primitives
                aHitTestProcessor2D.process(rSequence);

                // deliver result
                return aHitTestProcessor2D.getHit();
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
