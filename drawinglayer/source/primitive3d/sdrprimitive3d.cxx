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

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <utility>


namespace drawinglayer::primitive3d
{
        basegfx::B3DRange SdrPrimitive3D::getStandard3DRange() const
        {
            basegfx::B3DRange aUnitRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
            aUnitRange.transform(getTransform());

            if(!getSdrLFSAttribute().getLine().isDefault())
            {
                const attribute::SdrLineAttribute& rLine = getSdrLFSAttribute().getLine();

                if(!rLine.isDefault() && !basegfx::fTools::equalZero(rLine.getWidth()))
                {
                    // expand by held LineWidth as tube radius
                    aUnitRange.grow(rLine.getWidth() / 2.0);
                }
            }

            return aUnitRange;
        }

        basegfx::B3DRange SdrPrimitive3D::get3DRangeFromSlices(const Slice3DVector& rSlices) const
        {
            basegfx::B3DRange aRetval;

            if(!rSlices.empty())
            {
                for(const auto & rSlice : rSlices)
                {
                    aRetval.expand(basegfx::utils::getRange(rSlice.getB3DPolyPolygon()));
                }

                aRetval.transform(getTransform());

                if(!getSdrLFSAttribute().getLine().isDefault())
                {
                    const attribute::SdrLineAttribute& rLine = getSdrLFSAttribute().getLine();

                    if(!rLine.isDefault() && !basegfx::fTools::equalZero(rLine.getWidth()))
                    {
                        // expand by half LineWidth as tube radius
                        aRetval.grow(rLine.getWidth() / 2.0);
                    }
                }
            }

            return aRetval;
        }

        SdrPrimitive3D::SdrPrimitive3D(
            basegfx::B3DHomMatrix aTransform,
            const basegfx::B2DVector& rTextureSize,
            attribute::SdrLineFillShadowAttribute3D aSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   maTransform(std::move(aTransform)),
            maTextureSize(rTextureSize),
            maSdrLFSAttribute(std::move(aSdrLFSAttribute)),
            maSdr3DObjectAttribute(rSdr3DObjectAttribute)
        {
        }

        bool SdrPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive3D::operator==(rPrimitive))
            {
                const SdrPrimitive3D& rCompare = static_cast< const SdrPrimitive3D& >(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getTextureSize() == rCompare.getTextureSize()
                    && getSdrLFSAttribute() == rCompare.getSdrLFSAttribute()
                    && getSdr3DObjectAttribute() == rCompare.getSdr3DObjectAttribute());
            }

            return false;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
