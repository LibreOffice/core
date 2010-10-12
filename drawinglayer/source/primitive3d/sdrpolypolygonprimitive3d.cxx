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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrPolyPolygonPrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            Primitive3DSequence aRetval;

            if(getPolyPolygon3D().count())
            {
                ::std::vector< basegfx::B3DPolyPolygon > aFill;
                aFill.push_back(getPolyPolygon3D());

                // get full range
                const basegfx::B3DRange aRange(getRangeFrom3DGeometry(aFill));

                // #i98295# normal creation
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    if(::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind())
                    {
                        applyNormalsKindSphereTo3DGeometry(aFill, aRange);
                    }
                    else if(::com::sun::star::drawing::NormalsKind_FLAT == getSdr3DObjectAttribute().getNormalsKind())
                    {
                        applyNormalsKindFlatTo3DGeometry(aFill);
                    }

                    if(getSdr3DObjectAttribute().getNormalsInvert())
                    {
                        applyNormalsInvertTo3DGeometry(aFill);
                    }
                }

                // #i98314# texture coordinates
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    applyTextureTo3DGeometry(
                        getSdr3DObjectAttribute().getTextureProjectionX(),
                        getSdr3DObjectAttribute().getTextureProjectionY(),
                        aFill,
                        aRange,
                        getTextureSize());
                }

                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    // add fill
                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute(),
                        getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }
                else
                {
                    // create simplified 3d hit test geometry
                    aRetval = createHiddenGeometryPrimitives3D(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute());
                }

                // add line
                if(!getSdrLFSAttribute().getLine().isDefault())
                {
                    basegfx::B3DPolyPolygon aLine(getPolyPolygon3D());
                    aLine.clearNormals();
                    aLine.clearTextureCoordinates();
                    const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(
                        aLine, getTransform(), getSdrLFSAttribute().getLine()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
                }

                // add shadow
                if(!getSdrLFSAttribute().getShadow().isDefault()
                    && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(
                        aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        SdrPolyPolygonPrimitive3D::SdrPolyPolygonPrimitive3D(
            const basegfx::B3DPolyPolygon& rPolyPolygon3D,
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            maPolyPolygon3D(rPolyPolygon3D)
        {
        }

        bool SdrPolyPolygonPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrPolyPolygonPrimitive3D& rCompare = static_cast< const SdrPolyPolygonPrimitive3D& >(rPrimitive);

                return (getPolyPolygon3D() == rCompare.getPolyPolygon3D());
            }

            return false;
        }

        basegfx::B3DRange SdrPolyPolygonPrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // added this implementation to make sure that non-visible objects of this
            // kind will deliver their expansion. If not implemented, it would never deliver
            // the used space for non-visible objects since the decomposition for that
            // case will be empty (what is correct). To support chart ATM which relies on
            // non-visible objects occupying space in 3D, this method was added
            basegfx::B3DRange aRetval;

            if(getPolyPolygon3D().count())
            {
                aRetval = basegfx::tools::getRange(getPolyPolygon3D());
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

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrPolyPolygonPrimitive3D, PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
