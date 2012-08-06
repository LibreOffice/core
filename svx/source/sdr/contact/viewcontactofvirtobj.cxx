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

#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <svx/svdovirt.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//////////////////////////////////////////////////////////////////////////////

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

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfVirtObj::GetObjectCount() const
        {
            // Here, SdrVirtObj's need to return 0L to show that they have no
            // sub-hierarchy, even when they are group objects. This is necessary
            // to avoid that the same VOCs will be added to the draw hierarchy
            // twice which leads to problems.
            //
            // This solution is only a first solution to get things running. Later
            // this needs to be replaced with creating real VOCs for the objects
            // referenced by virtual objects to avoid the 'trick' of setting the
            // offset for painting at the destination OutputDevive.
            //
            // As can be seen, with primitives, the problem will be solved using
            // a transformPrimitive, so this solution can stay with primitives.
            return 0L;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfVirtObj::createViewIndependentPrimitive2DSequence() const
        {
            // create displacement transformation if we have content
            basegfx::B2DHomMatrix aObjectMatrix;
            Point aAnchor(GetVirtObj().GetAnchorPos());

            if(aAnchor.X() || aAnchor.Y())
            {
                aObjectMatrix.set(0, 2, aAnchor.X());
                aObjectMatrix.set(1, 2, aAnchor.Y());
            }

            // use method from referenced object to get the Primitive2DSequence
            const drawinglayer::primitive2d::Primitive2DSequence xSequenceVirtual(
                GetVirtObj().GetReferencedObj().GetViewContact().getViewIndependentPrimitive2DSequence());

            if(xSequenceVirtual.hasElements())
            {
                // create transform primitive
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aObjectMatrix,
                        xSequenceVirtual));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
            else
            {
                // always append an invisible outline for the cases where no visible content exists
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false, aObjectMatrix));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
