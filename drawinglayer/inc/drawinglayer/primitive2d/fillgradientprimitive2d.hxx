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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRADIENTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRADIENTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx { class B2DPolygon; }

//////////////////////////////////////////////////////////////////////////////
// FillGradientPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** FillGradientPrimitive2D class

            This class defines a gradient filling for a rectangular area. The
            Range is defined by the Transformation, the gradient by the FillGradientAttribute.

            The decomposition will deliver the decomposed gradient, e.g. for an ellipse
            gradient the various ellipses in various color steps will be created.

            I have added functionality to create both versions of filled decompositions:
            Those who overlap and non-overlapping ones. The overlapping version is the
            default one since it works with and without AntiAliasing. The non-overlapping
            version is used in the MetafilePrimitive2D decomposition when the old XOR
            paint was recorded.
         */
        class DRAWINGLAYER_DLLPUBLIC FillGradientPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DRange                       maObjectRange;

            /// the gradient definition
            attribute::FillGradientAttribute        maFillGradient;

            /// local helpers
            void generateMatricesAndColors(
                std::vector< basegfx::B2DHomMatrix >& rMatrices,
                std::vector< basegfx::BColor >& rColors) const;
            Primitive2DSequence createOverlappingFill(
                const std::vector< basegfx::B2DHomMatrix >& rMatrices,
                const std::vector< basegfx::BColor >& rColors,
                const basegfx::B2DPolygon& rUnitPolygon) const;
            Primitive2DSequence createNonOverlappingFill(
                const std::vector< basegfx::B2DHomMatrix >& rMatrices,
                const std::vector< basegfx::BColor >& rColors,
                const basegfx::B2DPolygon& rUnitPolygon) const;

        protected:
            /// local helper
            Primitive2DSequence createFill(bool bOverlapping) const;

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            FillGradientPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const attribute::FillGradientAttribute& rFillGradient);

            /// data read access
            const basegfx::B2DRange& getObjectRange() const { return maObjectRange; }
            const attribute::FillGradientAttribute& getFillGradient() const { return maFillGradient; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRADIENTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
