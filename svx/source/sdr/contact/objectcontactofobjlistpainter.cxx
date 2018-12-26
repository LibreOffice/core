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

#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdmodel.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <svx/unoapi.hxx>
#include <tools/debug.hxx>
#include <memory>

namespace sdr { namespace contact {

ObjectContactPainter::ObjectContactPainter()
{
}

// The destructor.
ObjectContactPainter::~ObjectContactPainter()
{
}

sal_uInt32 ObjectContactOfObjListPainter::GetPaintObjectCount() const
{
    return maStartObjects.size();
}

ViewContact& ObjectContactOfObjListPainter::GetPaintObjectViewContact(sal_uInt32 nIndex)
{
    const SdrObject* pObj = maStartObjects[nIndex];
    DBG_ASSERT(pObj, "ObjectContactOfObjListPainter: Corrupt SdrObjectVector (!)");
    return pObj->GetViewContact();
}

ObjectContactOfObjListPainter::ObjectContactOfObjListPainter(
    OutputDevice& rTargetDevice,
    const SdrObjectVector& rObjects,
    const SdrPage* pProcessedPage)
:   ObjectContactPainter(),
    mrTargetOutputDevice(rTargetDevice),
    maStartObjects(rObjects),
    mpProcessedPage(pProcessedPage)
{
}

ObjectContactOfObjListPainter::~ObjectContactOfObjListPainter()
{
}

// Process the whole displaying
void ObjectContactOfObjListPainter::ProcessDisplay(DisplayInfo& rDisplayInfo)
{
    const sal_uInt32 nCount(GetPaintObjectCount());

    if(nCount)
    {
        OutputDevice* pTargetDevice = TryToGetOutputDevice();

        if(pTargetDevice)
        {
            // update current ViewInformation2D at the ObjectContact
            const GDIMetaFile* pMetaFile = pTargetDevice->GetConnectMetaFile();
            const bool bOutputToRecordingMetaFile(pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
            basegfx::B2DRange aViewRange;

            // create ViewRange
            if(!bOutputToRecordingMetaFile)
            {
                // use visible pixels, but transform to world coordinates
                const Size aOutputSizePixel(pTargetDevice->GetOutputSizePixel());
                aViewRange = ::basegfx::B2DRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());
                aViewRange.transform(pTargetDevice->GetInverseViewTransformation());
            }

            // update local ViewInformation2D
            const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D(
                basegfx::B2DHomMatrix(),
                pTargetDevice->GetViewTransformation(),
                aViewRange,
                GetXDrawPageForSdrPage(const_cast< SdrPage* >(mpProcessedPage)),
                0.0,
                css::uno::Sequence<css::beans::PropertyValue>());
            updateViewInformation2D(aNewViewInformation2D);

            // collect primitive data in a sequence; this will already use the updated ViewInformation2D
            drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const ViewObjectContact& rViewObjectContact = GetPaintObjectViewContact(a).GetViewObjectContact(*this);

                xPrimitiveSequence.append(rViewObjectContact.getPrimitive2DSequenceHierarchy(rDisplayInfo));
            }

            // if there is something to show, use a vclProcessor to render it
            if(!xPrimitiveSequence.empty())
            {
                std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                    *pTargetDevice,
                    getViewInformation2D()));

                if(pProcessor2D)
                {
                    pProcessor2D->process(xPrimitiveSequence);
                }
            }
        }
    }
}

// recording MetaFile?
bool ObjectContactOfObjListPainter::isOutputToRecordingMetaFile() const
{
    GDIMetaFile* pMetaFile = mrTargetOutputDevice.GetConnectMetaFile();
    return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
}

// pdf export?
bool ObjectContactOfObjListPainter::isOutputToPDFFile() const
{
    return OUTDEV_PDF == mrTargetOutputDevice.GetOutDevType();
}

OutputDevice* ObjectContactOfObjListPainter::TryToGetOutputDevice() const
{
    return &mrTargetOutputDevice;
}

sal_uInt32 ObjectContactOfPagePainter::GetPaintObjectCount() const
{
    return (GetStartPage() ? 1 : 0);
}

ViewContact& ObjectContactOfPagePainter::GetPaintObjectViewContact(sal_uInt32 /*nIndex*/)
{
    DBG_ASSERT(GetStartPage(), "ObjectContactOfPagePainter::GetPaintObjectViewContact: no StartPage set (!)");
    return GetStartPage()->GetViewContact();
}

ObjectContactOfPagePainter::ObjectContactOfPagePainter(
    ObjectContact& rOriginalObjectContact)
:   ObjectContactPainter(),
    mrOriginalObjectContact(rOriginalObjectContact),
    mxStartPage()
{
}

ObjectContactOfPagePainter::~ObjectContactOfPagePainter()
{
}

void ObjectContactOfPagePainter::SetStartPage(const SdrPage* pPage)
{
    if(pPage != GetStartPage())
    {
        mxStartPage.reset(const_cast< SdrPage* >(pPage)); // no tools::WeakReference<SdrPage> available to hold a const SdrPage*
    }
}

OutputDevice* ObjectContactOfPagePainter::TryToGetOutputDevice() const
{
    return mrOriginalObjectContact.TryToGetOutputDevice();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
