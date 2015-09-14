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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <svtools/optionsdrawinglayer.hxx>

// predefines

namespace basegfx {
    class BZPixelRaster;
    class B3DPolygon;
    class B3DPolyPolygon;
}

namespace drawinglayer { namespace attribute {
    class SdrSceneAttribute;
    class SdrLightingAttribute;
    class MaterialAttribute3D;
}}

namespace drawinglayer { namespace primitive3d {
    class PolygonHairlinePrimitive3D;
    class PolyPolygonMaterialPrimitive3D;
    class GradientTexturePrimitive3D;
    class HatchTexturePrimitive3D;
    class BitmapTexturePrimitive3D;
    class TransformPrimitive3D;
    class ModifiedColorPrimitive3D;
}}

namespace drawinglayer { namespace texture {
    class GeoTexSvx;
}}



namespace drawinglayer
{
    namespace processor3d
    {
        /** DefaultProcessor3D class

            This processor renders all feeded primitives to a 2D raster where for all
            primitives the two basic methods rasterconvertB3DPolygon for hairlines and
            rasterconvertB3DPolyPolygon for filled geometry is called. It is a beseclass to
            e.g. base a Z-Buffer supported renderer on the 3D primitive processing.
         */
        class DefaultProcessor3D : public BaseProcessor3D
        {
        protected:
            /// read-only scene infos (normal handling, etc...)
            const attribute::SdrSceneAttribute&                 mrSdrSceneAttribute;

            /// read-only light infos (lights, etc...)
            const attribute::SdrLightingAttribute&              mrSdrLightingAttribute;

            /// renderer range. Need to be correctly set by the derived implementations
            /// normally the (0, 0, W, H) range from mpBZPixelRaster
            basegfx::B2DRange                                   maRasterRange;

            /// the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                        maBColorModifierStack;

            /// the current active texture
            std::shared_ptr< texture::GeoTexSvx >             mpGeoTexSvx;

            /// the current active transparence texture
            std::shared_ptr< texture::GeoTexSvx >             mpTransparenceGeoTexSvx;

            /// SvtOptionsDrawinglayer incarnation to react on diverse settings
            const SvtOptionsDrawinglayer                        maDrawinglayerOpt;

            /// counter for entered transparence textures
            sal_uInt32                                          mnTransparenceCounter;

            /// bitfield
            bool                                                mbModulate : 1;
            bool                                                mbFilter : 1;
            bool                                                mbSimpleTextureActive : 1;


            // rendering support

            void impRenderGradientTexturePrimitive3D(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence);
            void impRenderHatchTexturePrimitive3D(const primitive3d::HatchTexturePrimitive3D& rPrimitive);
            void impRenderBitmapTexturePrimitive3D(const primitive3d::BitmapTexturePrimitive3D& rPrimitive);
            void impRenderModifiedColorPrimitive3D(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate);
            void impRenderPolygonHairlinePrimitive3D(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive);
            void impRenderPolyPolygonMaterialPrimitive3D(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive);
            void impRenderTransformPrimitive3D(const primitive3d::TransformPrimitive3D& rTransformCandidate);


            // rasterconversions for filled and non-filled polygons. These NEED to be
            // implemented from derivations

            virtual void rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const = 0;
            virtual void rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const = 0;

            // the processing method for a single, known primitive
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive) SAL_OVERRIDE;

        public:
            DefaultProcessor3D(
                const geometry::ViewInformation3D& rViewInformation,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute);
            virtual ~DefaultProcessor3D();

            /// data read access
            const attribute::SdrSceneAttribute& getSdrSceneAttribute() const { return mrSdrSceneAttribute; }
            const attribute::SdrLightingAttribute& getSdrLightingAttribute() const { return mrSdrLightingAttribute; }

            /// data read access renderer stuff
            const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }
            const std::shared_ptr< texture::GeoTexSvx >& getGeoTexSvx() const { return mpGeoTexSvx; }
            const std::shared_ptr< texture::GeoTexSvx >& getTransparenceGeoTexSvx() const { return mpTransparenceGeoTexSvx; }
            sal_uInt32 getTransparenceCounter() const { return mnTransparenceCounter; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }
            bool getSimpleTextureActive() const { return mbSimpleTextureActive; }

            /// access to Drawinglayer configuration options
            const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer



#endif // INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
