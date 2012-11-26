/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
#include <svx/svdview.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i101872# new Object HitTest as View-tooling

SdrObject* SdrObjectPrimitiveHit(
    const SdrObject& rObject,
    const basegfx::B2DPoint& rPnt,
    double fTol,
    const SdrView& rSdrView,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields)
{
    SdrObject* pResult = 0;

    if(rObject.getChildrenOfSdrObject() && rObject.getChildrenOfSdrObject()->GetObjCount())
    {
        // group or scene with content. Single 3D objects also have a
        // true == rObject.GetSubList(), but no content
        pResult = SdrObjListPrimitiveHit(*rObject.getChildrenOfSdrObject(), rPnt, fTol, rSdrView, bTextOnly, pRecordFields);
    }
    else
    {
        const SdrPageView* pSdrPageView = rSdrView.GetSdrPageView();
        const SetOfByte* pVisiLayer = pSdrPageView ? &pSdrPageView->GetVisibleLayers() : 0;

        if( pSdrPageView && rObject.IsVisible() && (!pVisiLayer || pVisiLayer->IsSet(rObject.GetLayer())))
        {
            // single object, 3d object, empty scene or empty group. Check if
            // it's a single 3D object
            const E3dCompoundObject* pE3dCompoundObject = dynamic_cast< const E3dCompoundObject* >(&rObject);

            if(pE3dCompoundObject)
            {
                if(checkHitSingle3DObject(rPnt, *pE3dCompoundObject))
                {
                    pResult = const_cast< E3dCompoundObject* >(pE3dCompoundObject);
                }
            }
            else
            {
                // not a single 3D object; Check in first PageWindow using prmitives (only SC
                // with split views uses multiple PageWindows nowadays)
                if(rSdrView.GetSdrPageView() && rSdrView.GetSdrPageView()->PageWindowCount())
                {
                    const sdr::contact::ViewObjectContact& rVOC = rObject.GetViewContact().GetViewObjectContact(
                        rSdrView.GetSdrPageView()->GetPageWindow(0)->GetObjectContact());

                    if(ViewObjectContactPrimitiveHit(
                        rVOC, rPnt, fTol, bTextOnly, pRecordFields))
                    {
                          pResult = const_cast< SdrObject* >(&rObject);
                    }
                }
            }
        }
    }

    return pResult;
}

/////////////////////////////////////////////////////////////////////

SdrObject* SdrObjListPrimitiveHit(
    const SdrObjList& rList,
    const basegfx::B2DPoint& rPnt,
    double fTol,
    const SdrView& rSdrView,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields)
{
    sal_uInt32 nObjNum(rList.GetObjCount());
    SdrObject* pRetval = 0;

    while(!pRetval && nObjNum > 0)
    {
        nObjNum--;
        SdrObject* pObj = rList.GetObj(nObjNum);
        pRetval = SdrObjectPrimitiveHit(*pObj, rPnt, fTol, rSdrView, bTextOnly, pRecordFields);
    }

    return pRetval;
}

/////////////////////////////////////////////////////////////////////

bool ViewObjectContactPrimitiveHit(
    const sdr::contact::ViewObjectContact& rVOC,
    const basegfx::B2DPoint& rHitPosition,
    double fLogicHitTolerance,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields)
{
    basegfx::B2DRange aObjectRange(rVOC.getViewDependentRange());

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
                    bTextOnly,
                    pRecordFields);

                // feed it with the primitives
                aHitTestProcessor2D.process(rSequence);

                // deliver result
                return aHitTestProcessor2D.getHit();
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
