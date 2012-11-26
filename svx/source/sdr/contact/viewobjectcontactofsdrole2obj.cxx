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

#include <svx/sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdview.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

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
                //old stuff that should be reworked
                {
                    //if no replacement image is available load the OLE object
//                    if(!rSdrOle2.GetGraphic()) //try to fetch the metafile - this can lead to the actual creation of the metafile what can be extremely expensive (e.g. for big charts)!!! #i101925#
//                    {
//                      // try to create embedded object
//                        rSdrOle2.GetObjRef(); //this loads the OLE object if it is not loaded already
//                    }
                    const svt::EmbeddedObjectRef& xObjRef  = rSdrOle2.getEmbeddedObjectRef();
                    if(xObjRef.is())
                    {
                        const sal_Int64 nMiscStatus(xObjRef->getStatus(rSdrOle2.GetAspect()));

                        // this hack (to change model data during PAINT argh(!)) should be reworked
                        if(!rSdrOle2.IsResizeProtect() && (nMiscStatus & embed::EmbedMisc::EMBED_NEVERRESIZE))
                        {
                            const_cast< SdrOle2Obj* >(&rSdrOle2)->SetResizeProtect(true);
                        }

                        SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

                        if(pSdrView && (nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE))
                        {
                            // connect plugin object
                            pSdrView->DoConnect(const_cast< SdrOle2Obj* >(&rSdrOle2));
                        }
                    }
                }//end old stuff to rework

                // create OLE primitive stuff directly at VC with HC as parameter
                const ViewContactOfSdrOle2Obj& rVC = static_cast< const ViewContactOfSdrOle2Obj& >(GetViewContact());
                xRetval = rVC.createPrimitive2DSequenceWithParameters(GetObjectContact().isDrawModeHighContrast());

                if(bIsOutplaceActive)
                {
                    // do not shade when printing or PDF exporting
                    if(!GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile())
                    {
                        // shade the representation if the object is activated outplace
                        basegfx::B2DPolygon aObjectOutline(basegfx::tools::createUnitPolygon());
                        aObjectOutline.transform(rSdrOle2.getSdrObjectTransformation());

                        // Use a FillHatchPrimitive2D with necessary attributes
                        const drawinglayer::attribute::FillHatchAttribute aFillHatch(
                            drawinglayer::attribute::HATCHSTYLE_SINGLE, // single hatch
                            125.0, // 1.25 mm
                            45.0 * F_PI180, // 45 degree diagonal
                            Color(COL_BLACK).getBColor(), // black color
                            3, // same default as VCL, a minimum of three discrete units (pixels) offset
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

//////////////////////////////////////////////////////////////////////////////
// eof
