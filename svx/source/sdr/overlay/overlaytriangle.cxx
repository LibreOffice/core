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

#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>



namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayTriangle::createOverlayObjectPrimitive2DSequence()
        {
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(getBasePosition());
            aPolygon.append(getSecondPosition());
            aPolygon.append(getThirdPosition());
            aPolygon.setClosed(true);

            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aPolygon),
                    getBaseColor().getBColor()));

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayTriangle::OverlayTriangle(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            const basegfx::B2DPoint& rThirdPos,
            Color aTriangleColor)
        :   OverlayObjectWithBasePosition(rBasePos, aTriangleColor),
            maSecondPosition(rSecondPos),
            maThirdPosition(rThirdPos)
        {
        }

        OverlayTriangle::~OverlayTriangle()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
