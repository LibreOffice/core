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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdmodel.hxx>
#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <svx/unoapi.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ObjectContactPainter::ObjectContactPainter()
        {
        }

        // The destructor.
        ObjectContactPainter::~ObjectContactPainter()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        sal_uInt32 ObjectContactOfObjListPainter::GetPaintObjectCount() const
        {
            return maStartObjects.size();
        }

        ViewContact& ObjectContactOfObjListPainter::GetPaintObjectViewContact(sal_uInt32 nIndex) const
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

                    // upate local ViewInformation2D
                    const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D(
                        basegfx::B2DHomMatrix(),
                        pTargetDevice->GetViewTransformation(),
                        aViewRange,
                        GetXDrawPageForSdrPage(const_cast< SdrPage* >(mpProcessedPage)),
                        0.0,
                        com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>());
                    updateViewInformation2D(aNewViewInformation2D);

                    // collect primitive data in a sequence; this will already use the updated ViewInformation2D
                    drawinglayer::primitive2d::Primitive2DSequence xPrimitiveSequence;

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        const ViewObjectContact& rViewObjectContact = GetPaintObjectViewContact(a).GetViewObjectContact(*this);

                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xPrimitiveSequence,
                            rViewObjectContact.getPrimitive2DSequenceHierarchy(rDisplayInfo));
                    }

                    // if there is something to show, use a vclProcessor to render it
                    if(xPrimitiveSequence.hasElements())
                    {
                        drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = createBaseProcessor2DFromOutputDevice(
                            *pTargetDevice, getViewInformation2D());

                        if(pProcessor2D)
                        {
                            pProcessor2D->process(xPrimitiveSequence);
                            delete pProcessor2D;
                        }
                    }
                }
            }
        }

        // VirtualDevice?
        bool ObjectContactOfObjListPainter::isOutputToVirtualDevice() const
        {
            return (OUTDEV_VIRDEV == mrTargetOutputDevice.GetOutDevType());
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
            return (0 != mrTargetOutputDevice.GetPDFWriter());
        }

        OutputDevice* ObjectContactOfObjListPainter::TryToGetOutputDevice() const
        {
            return &mrTargetOutputDevice;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        sal_uInt32 ObjectContactOfPagePainter::GetPaintObjectCount() const
        {
            return (GetStartPage() ? 1L : 0L);
        }

        ViewContact& ObjectContactOfPagePainter::GetPaintObjectViewContact(sal_uInt32 /*nIndex*/) const
        {
            DBG_ASSERT(GetStartPage(), "ObjectContactOfPagePainter::GetPaintObjectViewContact: no StartPage set (!)");
            return GetStartPage()->GetViewContact();
        }

        ObjectContactOfPagePainter::ObjectContactOfPagePainter(
            const SdrPage* pPage,
            ObjectContact& rOriginalObjectContact)
        :   ObjectContactPainter(),
            mrOriginalObjectContact(rOriginalObjectContact),
            mxStartPage(const_cast< SdrPage* >(pPage)) // no SdrPageWeakRef available to hold a const SdrPage*
        {
        }

        ObjectContactOfPagePainter::~ObjectContactOfPagePainter()
        {
        }

        void ObjectContactOfPagePainter::SetStartPage(const SdrPage* pPage)
        {
            if(pPage != GetStartPage())
            {
                mxStartPage.reset(const_cast< SdrPage* >(pPage)); // no SdrPageWeakRef available to hold a const SdrPage*
            }
        }

        OutputDevice* ObjectContactOfPagePainter::TryToGetOutputDevice() const
        {
            return mrOriginalObjectContact.TryToGetOutputDevice();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
