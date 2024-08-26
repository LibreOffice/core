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

#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <utility>


namespace drawinglayer::primitive2d
{
        Primitive2DReference WrongSpellPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // This *was* a view-independent primitive before (see before this commit),
            // but was never really used, but handled in various processors anyways.
            // Since the current VCL primitive renderer and it's functions used in
            // VCL do render this always in one discrete pixel size I decided to
            // adapt this primitive to do that, too, but - due to being a primitive -
            // with the correct invalidate/hit-ranges etc.
            // I use here DiscreteMetricDependentPrimitive2D which already implements
            // buffering and providing the discrete size using 'getDiscreteUnit()' plus
            // the needed updates to buffering, what makes the rest simple.
            // NOTE: If one day the (in my opinion) also good looking view-independent
            // version should be needed again, just revert this change
            if (basegfx::fTools::lessOrEqual(getStop(), getStart()))
            {
                // stop smaller or equal to start, done
                return nullptr;
            }

            // get the font height (part of scale), so decompose the matrix
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            constexpr double constMinimumFontHeight(5.0);
            if (aScale.getY() / getDiscreteUnit() < constMinimumFontHeight)
            {
                // font height smaller constMinimumFontHeight pixels -> decompose to empty
                return nullptr;
            }

            // calculate distances based on a static default (to allow testing in debugger)
            static const double fDefaultDistance(0.03);
            const double fFontHeight(aScale.getY());
            const double fUnderlineDistance(fFontHeight * fDefaultDistance);

            // the Y-distance needs to be relative to FontHeight since the points get
            // transformed with the transformation containing that scale already.
            const double fRelativeUnderlineDistance(basegfx::fTools::equalZero(aScale.getY()) ? 0.0 : fUnderlineDistance / aScale.getY());

            // get start/stop positions and WaveLength, base all calculations for discrete
            // waveline on these initial values
            basegfx::B2DPoint aStart(getTransformation() * basegfx::B2DPoint(getStart(), fRelativeUnderlineDistance));
            const basegfx::B2DPoint aStop(getTransformation() * basegfx::B2DPoint(getStop(), fRelativeUnderlineDistance));
            const double fWaveLength(getDiscreteUnit() * 8);

            // get pre-calculated vector and controlPoint for one wave segment
            basegfx::B2DVector aVector(aStop - aStart);
            double fLength(aVector.getLength());
            aVector.normalize();
            basegfx::B2DVector aControl(basegfx::getPerpendicular(aVector));
            aVector *= fWaveLength;
            aControl = aControl * (fWaveLength * 0.5) + aVector * 0.5;

            // create geometry
            basegfx::B2DPolygon aWave;
            aWave.append(aStart);

            while (fLength > fWaveLength)
            {
                // one WaveSegment per WaveLength
                basegfx::B2DPoint aNew(aStart + aVector);
                aWave.appendBezierSegment(
                    aStart + aControl,
                    aNew - aControl,
                    aNew);
                aStart = aNew;
                fLength -= fWaveLength;
            }

            if (fLength > fWaveLength * 0.2)
            {
                // if rest is more than 20% of WaveLength, create
                // remaining snippet and add it
                basegfx::B2DPoint aNew(aStart + aVector);
                basegfx::B2DCubicBezier aRest(
                    aStart,
                    aStart + aControl,
                    aNew - aControl,
                    aNew);
                aRest = aRest.snippet(0.0, fLength/fWaveLength);
                aWave.appendBezierSegment(
                    aRest.getControlPointA(),
                    aRest.getControlPointB(),
                    aRest.getEndPoint());
            }

            // create & return primitive
            return new PolygonHairlinePrimitive2D(
                std::move(aWave),
                getColor());
        }

        WrongSpellPrimitive2D::WrongSpellPrimitive2D(
            basegfx::B2DHomMatrix aTransformation,
            double fStart,
            double fStop,
            const basegfx::BColor& rColor)
        :   DiscreteMetricDependentPrimitive2D(),
            maTransformation(std::move(aTransformation)),
            mfStart(fStart),
            mfStop(fStop),
            maColor(rColor)
        {
        }

        bool WrongSpellPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const WrongSpellPrimitive2D& rCompare = static_cast<const WrongSpellPrimitive2D&>(rPrimitive);

                return (getTransformation() == rCompare.getTransformation()
                    && getStart() == rCompare.getStart()
                    && getStop() == rCompare.getStop()
                    && getColor() == rCompare.getColor());
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 WrongSpellPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
