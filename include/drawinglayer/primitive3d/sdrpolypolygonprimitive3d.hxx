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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** SdrPolyPolygonPrimitive3D class

            This 3D primitive defines a PolyPolgon in space which may have
            Line- and FillStyles and extra 3D surface attributes. It is assumed
            that the given 3D PolyPolgon (which may contain texture and normal
            information) is planar in 3D.

            The decomposition will include all needed 3D data for visualisation,
            including FatLines and fill styles.
         */
        class DRAWINGLAYER_DLLPUBLIC SdrPolyPolygonPrimitive3D : public SdrPrimitive3D
        {
        private:
            /// the planar polyPolygon evtl with normals and texture coordinates
            basegfx::B3DPolyPolygon                 maPolyPolygon3D;

        protected:
            /// local decomposition.
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

        public:
            /// constructor
            SdrPolyPolygonPrimitive3D(
                const basegfx::B3DPolyPolygon& rPolyPolygon3D,
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute);

            /// data access
            const basegfx::B3DPolyPolygon& getPolyPolygon3D() const { return maPolyPolygon3D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
