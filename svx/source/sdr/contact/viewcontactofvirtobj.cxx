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

#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <svx/svdovirt.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>



namespace sdr
{
    namespace contact
    {
        SdrVirtObj& ViewContactOfVirtObj::GetVirtObj() const
        {
            return (SdrVirtObj&)mrObject;
        }

        ViewContactOfVirtObj::ViewContactOfVirtObj(SdrVirtObj& rObj)
        :   ViewContactOfSdrObj(rObj)
        {
        }

        ViewContactOfVirtObj::~ViewContactOfVirtObj()
        {
        }

        
        sal_uInt32 ViewContactOfVirtObj::GetObjectCount() const
        {
            
            
            
            
            //
            
            
            
            
            //
            
            
            return 0L;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfVirtObj::createViewIndependentPrimitive2DSequence() const
        {
            
            basegfx::B2DHomMatrix aObjectMatrix;
            Point aAnchor(GetVirtObj().GetAnchorPos());

            if(aAnchor.X() || aAnchor.Y())
            {
                aObjectMatrix.set(0, 2, aAnchor.X());
                aObjectMatrix.set(1, 2, aAnchor.Y());
            }

            
            const drawinglayer::primitive2d::Primitive2DSequence xSequenceVirtual(
                GetVirtObj().GetReferencedObj().GetViewContact().getViewIndependentPrimitive2DSequence());

            if(xSequenceVirtual.hasElements())
            {
                
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aObjectMatrix,
                        xSequenceVirtual));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
            else
            {
                
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false, aObjectMatrix));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
