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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
