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

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
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
                    // expand by hald LineWidth as tube radius
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
                for(sal_uInt32 a(0L); a < rSlices.size(); a++)
                {
                    aRetval.expand(basegfx::tools::getRange(rSlices[a].getB3DPolyPolygon()));
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
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   BufferedDecompositionPrimitive3D(),
            maTransform(rTransform),
            maTextureSize(rTextureSize),
            maSdrLFSAttribute(rSdrLFSAttribute),
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

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
