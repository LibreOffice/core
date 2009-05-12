/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewobjectcontactofsdrole2obj.cxx,v $
 *
 * $Revision: 1.2 $
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

#include <svx/sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        const SdrOle2Obj& ViewObjectContactOfSdrOle2Obj::getSdrOle2Object() const
        {
            return static_cast< ViewContactOfSdrOle2Obj& >(GetViewContact()).GetOle2Obj();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfSdrOle2Obj::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // this method is overloaded to do some things the old SdrOle2Obj::DoPaintObject did.
            // In the future, some of these may be solved different, but ATM try to stay compatible
            // with the old behaviour
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrOle2Obj& rSdrOle2 = getSdrOle2Object();
            const bool bIsActive(rSdrOle2.executeOldDoPaintPreparations(GetObjectContact().TryToGetSdrPageView()));
            const Rectangle& rObjectRectangle(rSdrOle2.GetGeoRect());
            const basegfx::B2DRange aObjectRange(rObjectRectangle.Left(), rObjectRectangle.Top(), rObjectRectangle.Right(), rObjectRectangle.Bottom());

            // create object transform
            basegfx::B2DHomMatrix aObjectTransform;
            aObjectTransform.set(0, 0, aObjectRange.getWidth());
            aObjectTransform.set(1, 1, aObjectRange.getHeight());
            aObjectTransform.set(0, 2, aObjectRange.getMinX());
            aObjectTransform.set(1, 2, aObjectRange.getMinY());

            if(GetObjectContact().isDrawModeHighContrast())
            {
                // directly call at the corresponding VC and force OLE Graphic to HighContrast
                const ViewContactOfSdrOle2Obj& rVC = static_cast< const ViewContactOfSdrOle2Obj& >(GetViewContact());
                Graphic* pOLEHighContrastGraphic = rSdrOle2.getEmbeddedObjectRef().GetHCGraphic();

                if(pOLEHighContrastGraphic)
                {
                    // there is a graphic set, use it
                    xRetval = rVC.createPrimitive2DSequenceWithGivenGraphic(*pOLEHighContrastGraphic, rSdrOle2.IsEmptyPresObj());
                }
                else
                {
                    // no HighContrast graphic, use default empty OLE bitmap
                    const Bitmap aEmptyOLEBitmap(rSdrOle2.GetEmtyOLEReplacementBitmap());
                    const Graphic aEmtyOLEGraphic(aEmptyOLEBitmap);

                    xRetval = rVC.createPrimitive2DSequenceWithGivenGraphic(aEmtyOLEGraphic, true);
                }
            }
            else
            {
                // call parent which will use the regular createViewIndependentPrimitive2DSequence
                // at the corresponding VC
                xRetval = ViewObjectContactOfSdrObj::createPrimitive2DSequence(rDisplayInfo);
            }

            if(rSdrOle2.getEmbeddedObjectRef().IsChart())
            {
                // for chart, to not lose the current better quality visualisation which
                // uses a direct paint, use a primtive wrapper for that exceptional case. The renderers
                // will then ATM paint it to an OutputDevice directly.
                // In later versions this should be replaced by getting the Primitive2DSequnce from
                // the chart and using it.
                // to be able to render something in non-VCL using renderers, the wrapper is a
                // GroupPrimitive2D which automatically decomposes to the already created Metafile
                // content.
                // For being completely compatible, ATM Window and VDEV PrettyPrinting is suppressed.
                // It works in the VCL renderers, though. So for activating again with VCL primitive
                // renderers, change conditions here.

                // determine if embedding and PrettyPrinting shall be done at all
                uno::Reference< frame::XModel > xChartModel;
                bool bDoChartPrettyPrinting(true);
                static bool bPrettyPrintingForBitmaps(false);

                // the original ChartPrettyPainter does not do it for Window
                if(!bPrettyPrintingForBitmaps && bDoChartPrettyPrinting && GetObjectContact().isOutputToWindow())
                {
                    bDoChartPrettyPrinting = false;
                }

                // the original ChartPrettyPainter does not do it for VDEV
                if(!bPrettyPrintingForBitmaps && bDoChartPrettyPrinting && GetObjectContact().isOutputToVirtualDevice())
                {
                    if(GetObjectContact().isOutputToPDFFile())
                    {
                        // #i97982#
                        // For PDF files, allow PrettyPrinting
                    }
                    else
                    {
                        bDoChartPrettyPrinting = false;
                    }
                }

                // the chart model is needed. Check if it's available
                if(bDoChartPrettyPrinting)
                {
                    // get chart model
                    xChartModel = rSdrOle2.getXModel();

                    if(!xChartModel.is())
                    {
                        bDoChartPrettyPrinting = false;
                    }
                }

                if(bDoChartPrettyPrinting)
                {
                    // embed MetaFile data in a specialized Wrapper Primitive which holds also the ChartModel needed
                    // for PrettyPrinting
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::ChartPrimitive2D(
                        xChartModel, aObjectTransform, xRetval));
                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            if(bIsActive)
            {
                // do not shade when printing or PDF exporting
                if(!GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile())
                {
                    // shade the representation if the object is activated outplace
                    basegfx::B2DPolygon aObjectOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                    aObjectOutline.transform(aObjectTransform);

                    // Use a FillHatchPrimitive2D with necessary attributes
                    const drawinglayer::attribute::FillHatchAttribute aFillHatch(
                        drawinglayer::attribute::HATCHSTYLE_SINGLE, // single hatch
                        125.0, // 1.25 mm
                        45.0 * F_PI180, // 45 degree diagonal
                        Color(COL_BLACK).getBColor(), // black color
                        false); // no filling

                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolyPolygonHatchPrimitive2D(
                        basegfx::B2DPolyPolygon(aObjectOutline),
                        Color(COL_BLACK).getBColor(),
                        aFillHatch));

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xReference);
                }
            }

            return xRetval;
        }

        ViewObjectContactOfSdrOle2Obj::ViewObjectContactOfSdrOle2Obj(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfSdrOle2Obj::~ViewObjectContactOfSdrOle2Obj()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
