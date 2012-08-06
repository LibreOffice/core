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


#include <svx/sdr/contact/viewcontactofsdredgeobj.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrconnectorprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrEdgeObj::ViewContactOfSdrEdgeObj(SdrEdgeObj& rEdgeObj)
        :   ViewContactOfTextObj(rEdgeObj)
        {
        }

        ViewContactOfSdrEdgeObj::~ViewContactOfSdrEdgeObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrEdgeObj::createViewIndependentPrimitive2DSequence() const
        {
            const basegfx::B2DPolygon& rEdgeTrack = GetEdgeObj().getEdgeTrack();

            // what to do when no EdgeTrack is provided (HitTest and selectability) ?
            OSL_ENSURE(0 != rEdgeTrack.count(), "Connectors with no geometry are not allowed (!)");

            // ckeck attributes
            const SfxItemSet& rItemSet = GetEdgeObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineShadowTextAttribute(
                    rItemSet,
                    GetEdgeObj().getText(0)));

            // create primitive. Always create primitives to allow the decomposition of
            // SdrConnectorPrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrConnectorPrimitive2D(
                    aAttribute,
                    rEdgeTrack));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
