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

#include <sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdview.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svtools/embedhlp.hxx>

using namespace com::sun::star;

namespace sdr { namespace contact {

drawinglayer::primitive2d::Primitive2DContainer ViewObjectContactOfSdrOle2Obj::createPrimitive2DSequence(
    const DisplayInfo& /*rDisplayInfo*/) const
{
    // override this method to do some things the old SdrOle2Obj::DoPaintObject did.
    // In the future, some of these may be solved different, but ATM try to stay compatible
    // with the old behaviour
    drawinglayer::primitive2d::Primitive2DContainer xRetval;
    const SdrOle2Obj& rSdrOle2 = static_cast< ViewContactOfSdrOle2Obj& >(GetViewContact()).GetOle2Obj();
    sal_Int32 nState(-1);

    {
        const svt::EmbeddedObjectRef& xObjRef  = rSdrOle2.getEmbeddedObjectRef();
        if ( xObjRef.is() )
            nState = xObjRef->getCurrentState();
    }

    const bool bIsOutplaceActive(nState == embed::EmbedStates::ACTIVE);
    const bool bIsInplaceActive((nState == embed::EmbedStates::INPLACE_ACTIVE) || (nState == embed::EmbedStates::UI_ACTIVE));
    bool bDone(false);

    if (bIsInplaceActive)
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
        //old stuff that should be reworked
        {
            //if no replacement image is available load the OLE object
//          if(!rSdrOle2.GetGraphic()) //try to fetch the metafile - this can lead to the actual creation of the metafile what can be extremely expensive (e.g. for big charts)!!! #i101925#
//          {
//            // try to create embedded object
//              rSdrOle2.GetObjRef(); //this loads the OLE object if it is not loaded already
//          }
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

        if(bIsOutplaceActive)
        {
            // do not shade when printing or PDF exporting
            if(!GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile())
            {
                // get object transformation
                const basegfx::B2DHomMatrix aObjectMatrix(static_cast< ViewContactOfSdrOle2Obj& >(GetViewContact()).createObjectTransform());

                // shade the representation if the object is activated outplace
                basegfx::B2DPolygon aObjectOutline(basegfx::utils::createUnitPolygon());
                aObjectOutline.transform(aObjectMatrix);

                // Use a FillHatchPrimitive2D with necessary attributes
                const drawinglayer::attribute::FillHatchAttribute aFillHatch(
                    drawinglayer::attribute::HatchStyle::Single, // single hatch
                    125.0, // 1.25 mm
                    basegfx::deg2rad(45.0), // 45 degree diagonal
                    COL_BLACK.getBColor(), // black color
                    3, // same default as VCL, a minimum of three discrete units (pixels) offset
                    false); // no filling

                const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolyPolygonHatchPrimitive2D(
                    basegfx::B2DPolyPolygon(aObjectOutline),
                    COL_BLACK.getBColor(),
                    aFillHatch));

                xRetval.push_back(xReference);
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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
