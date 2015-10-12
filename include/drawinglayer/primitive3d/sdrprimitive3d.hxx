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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>



namespace drawinglayer
{
    /** SdrPrimitive3D class

        Base class for the more complicated geometric primitives, so
        derive from buffered primitive to allow overriding of
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
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
