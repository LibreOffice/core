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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** SdrLathePrimitive3D class

            This 3D primitive expands the SdrPrimitive3D to a 3D rotation definition.
            The given 2D tools::PolyPolygon geometry is imagined as lying on the XY-plane in 3D
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
            bool                                        mbSmoothNormals : 1; // Plane self
            bool                                        mbSmoothHorizontalNormals : 1; // always
            bool                                        mbSmoothLids : 1; // Front/back
            bool                                        mbCharacterMode : 1;
            bool                                        mbCloseFront : 1;
            bool                                        mbCloseBack : 1;

            /// create slices
            void impCreateSlices();

            /// get (evtl. create) slices
            const Slice3DVector& getSlices() const;

        protected:
            /// local decomposition.
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// Overridden to allow for reduced line mode to decide if to buffer decomposition or not
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
