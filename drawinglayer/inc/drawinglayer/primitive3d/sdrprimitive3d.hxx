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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    /** SdrPrimitive3D class

        Base class for the more complicated geometric primitives, so
        derive from buffered primitive to allow overloading of
        create3DDecomposition there.
     */
    namespace primitive3d
    {
        class DRAWINGLAYER_DLLPUBLIC SdrPrimitive3D : public BufferedDecompositionPrimitive3D
        {
        private:
            /// object surface attributes
            basegfx::B3DHomMatrix                       maTransform;
            basegfx::B2DVector                          maTextureSize;
            attribute::SdrLineFillShadowAttribute3D     maSdrLFSAttribute;
            attribute::Sdr3DObjectAttribute             maSdr3DObjectAttribute;

        protected:
            /** Standard implementation for primitive3D which
                will use maTransform as range and expand by evtl. line width / 2
             */
            basegfx::B3DRange getStandard3DRange() const;

            /** implementation for primitive3D which
                will use given Slice3Ds and expand by evtl. line width / 2
             */
            basegfx::B3DRange get3DRangeFromSlices(const Slice3DVector& rSlices) const;

        public:
            /// constructor
            SdrPrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute);

            /// data read access
            const basegfx::B3DHomMatrix& getTransform() const { return maTransform; }
            const basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            const attribute::SdrLineFillShadowAttribute3D& getSdrLFSAttribute() const { return maSdrLFSAttribute; }
            const attribute::Sdr3DObjectAttribute getSdr3DObjectAttribute() const { return maSdr3DObjectAttribute; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
