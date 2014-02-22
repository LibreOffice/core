/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>



using namespace com::sun::star;



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
                    
                    
                    bDone = true;
                }
            }

            if( !bDone )
            {
                
                {
                    





                    const svt::EmbeddedObjectRef& xObjRef  = rSdrOle2.getEmbeddedObjectRef();
                    if(xObjRef.is())
                    {
                        const sal_Int64 nMiscStatus(xObjRef->getStatus(rSdrOle2.GetAspect()));

                        
                        if(!rSdrOle2.IsResizeProtect() && (nMiscStatus & embed::EmbedMisc::EMBED_NEVERRESIZE))
                        {
                            const_cast< SdrOle2Obj* >(&rSdrOle2)->SetResizeProtect(true);
                        }

                        SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();
                        if(pPageView && (nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE))
                        {
                            
                            pPageView->GetView().DoConnect(const_cast< SdrOle2Obj* >(&rSdrOle2));
                        }
                    }
                }

                
                const ViewContactOfSdrOle2Obj& rVC = static_cast< const ViewContactOfSdrOle2Obj& >(GetViewContact());
                xRetval = rVC.createPrimitive2DSequenceWithParameters();

                if(bIsOutplaceActive)
                {
                    
                    if(!GetObjectContact().isOutputToPrinter() && !GetObjectContact().isOutputToRecordingMetaFile())
                    {
                        
                        const basegfx::B2DHomMatrix aObjectMatrix(static_cast< ViewContactOfSdrOle2Obj& >(GetViewContact()).createObjectTransform());

                        
                        basegfx::B2DPolygon aObjectOutline(basegfx::tools::createUnitPolygon());
                        aObjectOutline.transform(aObjectMatrix);

                        
                        const drawinglayer::attribute::FillHatchAttribute aFillHatch(
                            drawinglayer::attribute::HATCHSTYLE_SINGLE, 
                            125.0, 
                            45.0 * F_PI180, 
                            Color(COL_BLACK).getBColor(), 
                            3, 
                            false); 

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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
