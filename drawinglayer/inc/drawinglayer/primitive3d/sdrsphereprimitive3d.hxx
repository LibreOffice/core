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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRSPHEREPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRSPHEREPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** SdrSpherePrimitive3D class

            This 3D primitive expands the SdrPrimitive3D to a 3D sphere definition.
            The sphere is implicitely in unit coordinates and the given transformation
            defines it's geometry in space.
         */
        class DRAWINGLAYER_DLLPUBLIC SdrSpherePrimitive3D : public SdrPrimitive3D
        {
        private:
            /// additional geometry definitions
            sal_uInt32                                  mnHorizontalSegments;
            sal_uInt32                                  mnVerticalSegments;

        protected:
            /// local decomposition.
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

        public:
            /// constructor
            SdrSpherePrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
                sal_uInt32 nHorizontalSegments,
                sal_uInt32 nVerticalSegments);

            /// data read access
            sal_uInt32 getHorizontalSegments() const { return mnHorizontalSegments; }
            sal_uInt32 getVerticalSegments() const { return mnVerticalSegments; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRSPHEREPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
