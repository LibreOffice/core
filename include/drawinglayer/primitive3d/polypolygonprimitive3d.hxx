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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** PolyPolygonMaterialPrimitive3D class

            This primitive defines a planar 3D tools::PolyPolygon filled with a single color.
            This is one of the non-decomposable primitives, so a renderer
            should process it.

            It is assumed here that the PolyPolgon is a single plane in 3D.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonMaterialPrimitive3D : public BasePrimitive3D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B3DPolyPolygon                     maPolyPolygon;

            /// the fill parameters
            attribute::MaterialAttribute3D              maMaterial;

            /// bitfield
            bool                                        mbDoubleSided : 1;

        public:
            /// constructor
            PolyPolygonMaterialPrimitive3D(
                const basegfx::B3DPolyPolygon& rPolyPolygon,
                const attribute::MaterialAttribute3D& rMaterial,
                bool bDoubleSided);

            /// data read access
            const basegfx::B3DPolyPolygon& getB3DPolyPolygon() const { return maPolyPolygon; }
            const attribute::MaterialAttribute3D& getMaterial() const { return maMaterial; }
            bool getDoubleSided() const { return mbDoubleSided; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
