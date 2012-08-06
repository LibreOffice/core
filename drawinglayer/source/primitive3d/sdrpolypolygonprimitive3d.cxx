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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
