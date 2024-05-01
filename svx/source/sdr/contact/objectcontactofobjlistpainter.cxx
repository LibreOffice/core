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

#include <sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <svx/unoapi.hxx>
#include <tools/debug.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <memory>

namespace sdr::contact {

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
    assert(pObj && "ObjectContactOfObjListPainter: Corrupt SdrObjectVector (!)");
    return pObj->GetViewContact();
}

ObjectContactOfObjListPainter::ObjectContactOfObjListPainter(
    OutputDevice& rTargetDevice,
    SdrObjectVector&& rObjects,
    const SdrPage* pProcessedPage)
:   mrTargetOutputDevice(rTargetDevice),
    maStartObjects(std::move(rObjects)),
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

    if(!nCount)
        return;

    OutputDevice* pTargetDevice = TryToGetOutputDevice();

    if(!pTargetDevice)
        return;

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
    drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
    aNewViewInformation2D.setViewTransformation(pTargetDevice->GetViewTransformation());
    aNewViewInformation2D.setViewport(aViewRange);
    aNewViewInformation2D.setVisualizedPage(GetXDrawPageForSdrPage(const_cast< SdrPage* >(mpProcessedPage)));
    updateViewInformation2D(aNewViewInformation2D);

    // collect primitive data in a sequence; this will already use the updated ViewInformation2D
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        const ViewObjectContact& rViewObjectContact = GetPaintObjectViewContact(a).GetViewObjectContact(*this);

        rViewObjectContact.getPrimitive2DSequenceHierarchy(rDisplayInfo, xPrimitiveSequence);
    }

    // if there is something to show, use a vclProcessor to render it
    if(!xPrimitiveSequence.empty())
    {
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(drawinglayer::processor2d::createProcessor2DFromOutputDevice(
            *pTargetDevice,
            getViewInformation2D()));

        pProcessor2D->process(xPrimitiveSequence);
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

bool ObjectContactOfObjListPainter::isExportTaggedPDF() const
{
    if (isOutputToPDFFile())
    {
        vcl::PDFExtOutDevData* pPDFExtOutDevData(dynamic_cast<vcl::PDFExtOutDevData*>(
            mrTargetOutputDevice.GetExtOutDevData()));

        if (nullptr != pPDFExtOutDevData)
        {
            return pPDFExtOutDevData->GetIsExportTaggedPDF();
        }
    }
    return false;
}

::vcl::PDFExtOutDevData const* ObjectContactOfObjListPainter::GetPDFExtOutDevData() const
{
    if (!isOutputToPDFFile())
    {
        return nullptr;
    }
    vcl::PDFExtOutDevData *const pPDFExtOutDevData(
        dynamic_cast<vcl::PDFExtOutDevData*>(mrTargetOutputDevice.GetExtOutDevData()));
    return pPDFExtOutDevData;
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
:   mrOriginalObjectContact(rOriginalObjectContact)
{
}

ObjectContactOfPagePainter::~ObjectContactOfPagePainter()
{
}

void ObjectContactOfPagePainter::SetStartPage(const SdrPage* pPage)
{
    if(pPage != GetStartPage())
    {
        mxStartPage = const_cast< SdrPage* >(pPage); // no tools::WeakReference<SdrPage> available to hold a const SdrPage*
    }
}

OutputDevice* ObjectContactOfPagePainter::TryToGetOutputDevice() const
{
    return mrOriginalObjectContact.TryToGetOutputDevice();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
