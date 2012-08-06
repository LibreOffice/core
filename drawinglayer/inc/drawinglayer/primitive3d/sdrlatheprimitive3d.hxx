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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** SdrLathePrimitive3D class

            This 3D primitive expands the SdrPrimitive3D to a 3D rotation definition.
            The given 2D PolyPolygon geometry is imagined as lying on the XY-plane in 3D
            and gets rotated around the Y-Axis.

            Various possibilities e.g. for creating diagonals (edge roudings in 3D)
            and similar are given.

            The decomposition will create all necessary 3D planes for visualisation.
         */
        class DRAWINGLAYER_DLLPUBLIC SdrLathePrimitive3D : public SdrPrimitive3D
        {
        private:
            /// geometry helper for slices
            basegfx::B2DPolyPolygon                     maCorrectedPolyPolygon;
            Slice3DVector                               maSlices;

            /// primitive geometry data
            basegfx::B2DPolyPolygon                     maPolyPolygon;
            sal_uInt32                                  mnHorizontalSegments;
            sal_uInt32                                  mnVerticalSegments;
            double                                      mfDiagonal;
            double                                      mfBackScale;
            double                                      mfRotation;

            /// decomposition data when ReducedLineGeometry is used, see get3DDecomposition
            geometry::ViewInformation3D*                mpLastRLGViewInformation;

            /// bitfield
            unsigned                                    mbSmoothNormals : 1; // Plane self
            unsigned                                    mbSmoothHorizontalNormals : 1; // always
            unsigned                                    mbSmoothLids : 1; // Front/back
            unsigned                                    mbCharacterMode : 1;
            unsigned                                    mbCloseFront : 1;
            unsigned                                    mbCloseBack : 1;

            /// create slices
            void impCreateSlices();

            /// get (evtl. create) slices
            const Slice3DVector& getSlices() const;

        protected:
            /// local helpers
            void impCreateOutlines(
                const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B3DPolygon& rLoopA,
                const basegfx::B3DPolygon& rLoopB,
                basegfx::B3DPolyPolygon& rTarget) const;

            bool impHasCutWith(
                const basegfx::B2DPolygon& rPoly,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd) const;

            /// local decomposition.
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

        public:
            /// constructor
            SdrLathePrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                sal_uInt32 nHorizontalSegments,
                sal_uInt32 nVerticalSegments,
                double fDiagonal,
                double fBackScale,
                double fRotation,
                bool bSmoothNormals,
                bool bSmoothHorizontalNormals,
                bool bSmoothLids,
                bool bCharacterMode,
                bool bCloseFront,
                bool bCloseBack);
            virtual ~SdrLathePrimitive3D();

            /// data read access
            const basegfx::B2DPolyPolygon& getPolyPolygon() const { return maPolyPolygon; }
            sal_uInt32 getHorizontalSegments() const { return mnHorizontalSegments; }
            sal_uInt32 getVerticalSegments() const { return mnVerticalSegments; }
            double getDiagonal() const { return mfDiagonal; }
            double getBackScale() const { return mfBackScale; }
            double getRotation() const { return mfRotation; }
            bool getSmoothNormals() const { return mbSmoothNormals; }
            bool getSmoothHorizontalNormals() const { return mbSmoothHorizontalNormals; }
            bool getSmoothLids() const { return mbSmoothLids; }
            bool getCharacterMode() const { return mbCharacterMode; }
            bool getCloseFront() const { return mbCloseFront; }
            bool getCloseBack() const { return mbCloseBack; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// Overloaded to allow for reduced line mode to decide if to buffer decomposition or not
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
