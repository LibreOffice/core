/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svx/sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdview.hxx>
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <basegfx/polygon/b2dpolygon.hxx>

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

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfSdrOle2Obj::createPrimitive2DSequence(
            const DisplayInfo& /*rDisplayInfo*/) const
        {
            // this method is overloaded to do some things the old SdrOle2Obj::DoPaintObject did.
            // In the future, some of these may be solved different, but ATM try to stay compatible
            // with the old behaviour
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrOle2Obj& rSdrOle2 = getSdrOle2Object();
            sal_Int32 nState(-1);

            {
                const svt::EmbeddedObjectRef& xObjRef  = rSdrOle2.getEmbeddedObjectRef();
                if ( xObjRef.is() )
                    nState = xObjRef->getCurrentState();
            }

            const bool bIsOutplaceActive(nState == embed::EmbedStates::ACTIVE);
            const bool bIsInplaceActive((nState == embed::EmbedStates::INPLACE_ACTIVE) || (nState == embed::EmbedStates::UI_ACTIVE));
            const bool bIsChart(rSdrOle2.IsChart());
            bool bDone(false);

            if(!bDone && bIsInplaceActive)
            {
                if( !GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile() )
                {
                    //no need to create a primitive sequence here as the OLE object does render itself
                    //in case of charts the superfluous creation of a metafile is strongly performance relevant!
                    bDone = true;
                }
            }

            if( !bDone )
            {
                const Rectangle& rObjectRectangle(rSdrOle2.GetGeoRect());
                const basegfx::B2DRange aObjectRange(rObjectRectangle.Left(), rObjectRectangle.Top(), rObjectRectangle.Right(), rObjectRectangle.Bottom());

                // create object transform
                basegfx::B2DHomMatrix aObjectTransform;
                aObjectTransform.set(0, 0, aObjectRange.getWidth());
                aObjectTransform.set(1, 1, aObjectRange.getHeight());
                aObjectTransform.set(0, 2, aObjectRange.getMinX());
                aObjectTransform.set(1, 2, aObjectRange.getMinY());

                if(bIsChart)
                {
                    //charts must be painted resolution dependent!! #i82893#, #i75867#

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

                    // the original ChartPrettyPainter does not do it for Window
                    if(bDoChartPrettyPrinting && GetObjectContact().isOutputToWindow())
                    {
                        bDoChartPrettyPrinting = false;
                    }

                    // the original ChartPrettyPainter does not do it for VDEV
                    if(bDoChartPrettyPrinting && GetObjectContact().isOutputToVirtualDevice())
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
                        bDone = true;
                    }
                }

                if( !bDone )
                {
                    //old stuff that should be reworked
                    {
                        //if no replacement image is available load the OLE object
                        if(!rSdrOle2.GetGraphic()) //try to fetch the metafile - this can lead to the actual creation of the metafile what can be extremely expensive (e.g. for big charts)!!! #i101925#
                        {
                            // try to create embedded object
                            rSdrOle2.GetObjRef(); //this loads the OLE object if it is not loaded already
                        }
                        const svt::EmbeddedObjectRef& xObjRef  = rSdrOle2.getEmbeddedObjectRef();
                        if(xObjRef.is())
                        {
                            const sal_Int64 nMiscStatus(xObjRef->getStatus(rSdrOle2.GetAspect()));

                            // this hack (to change model data during PAINT argh(!)) should be reworked
                            if(!rSdrOle2.IsResizeProtect() && (nMiscStatus & embed::EmbedMisc::EMBED_NEVERRESIZE))
                            {
                                const_cast< SdrOle2Obj* >(&rSdrOle2)->SetResizeProtect(true);
                            }

                            SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();
                            if(pPageView && (nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE))
                            {
                                // connect plugin object
                                pPageView->GetView().DoConnect(const_cast< SdrOle2Obj* >(&rSdrOle2));
                            }
                        }
                    }//end old stuff to rework

                    // create OLE primitive stuff directly at VC with HC as parameter
                    const ViewContactOfSdrOle2Obj& rVC = static_cast< const ViewContactOfSdrOle2Obj& >(GetViewContact());
                    xRetval = rVC.createPrimitive2DSequenceWithParameters();
                }

                if(bIsOutplaceActive)
                {
                    // do not shade when printing or PDF exporting
                    if(!GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile())
                    {
                        // shade the representation if the object is activated outplace
                        basegfx::B2DPolygon aObjectOutline(basegfx::tools::createUnitPolygon());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
