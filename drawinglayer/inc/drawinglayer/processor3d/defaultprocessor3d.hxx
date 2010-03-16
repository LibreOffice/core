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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <svtools/optionsdrawinglayer.hxx>

//////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class DefaultProcessor3D : public BaseProcessor3D
        {
        protected:
            // render information
            const attribute::SdrSceneAttribute&                 mrSdrSceneAttribute;    // read-only scene infos (normal handling, etc...)
            const attribute::SdrLightingAttribute&              mrSdrLightingAttribute; // read-only light infos (lights, etc...)

            // renderer range. Need to be correctly set by the derived implementations
            basegfx::B2DRange                                   maRasterRange;          // the (0, 0, W, H) range from mpBZPixelRaster

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                        maBColorModifierStack;

            // the current active texture
            texture::GeoTexSvx*                                 mpGeoTexSvx;

            // the current active transparence texture
            texture::GeoTexSvx*                                 mpTransparenceGeoTexSvx;

            // SvtOptionsDrawinglayer incarnation to react on diverse settings
            const SvtOptionsDrawinglayer                        maDrawinglayerOpt;

            // bitfield
            unsigned                                            mbModulate : 1;
            unsigned                                            mbFilter : 1;
            unsigned                                            mbSimpleTextureActive : 1;

            //////////////////////////////////////////////////////////////////////////////
            // rendering support
            void impRenderGradientTexturePrimitive3D(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence);
            void impRenderHatchTexturePrimitive3D(const primitive3d::HatchTexturePrimitive3D& rPrimitive);
            void impRenderBitmapTexturePrimitive3D(const primitive3d::BitmapTexturePrimitive3D& rPrimitive);
            void impRenderModifiedColorPrimitive3D(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate);
            void impRenderPolygonHairlinePrimitive3D(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive);
            void impRenderPolyPolygonMaterialPrimitive3D(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive);
            void impRenderTransformPrimitive3D(const primitive3d::TransformPrimitive3D& rTransformCandidate);

            //////////////////////////////////////////////////////////////////////////////
            // rasterconversions for filled and non-filled polygons. These NEED to be
            // implemented from derivations
            virtual void rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const = 0;
            virtual void rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const = 0;

            // the processing method for a single, known primitive
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive);

        public:
            DefaultProcessor3D(
                const geometry::ViewInformation3D& rViewInformation,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute);
            virtual ~DefaultProcessor3D();

            // data read access
            const attribute::SdrSceneAttribute& getSdrSceneAttribute() const { return mrSdrSceneAttribute; }
            const attribute::SdrLightingAttribute& getSdrLightingAttribute() const { return mrSdrLightingAttribute; }

            // data read access renderer stuff
            const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }
            const texture::GeoTexSvx* getGeoTexSvx() const { return mpGeoTexSvx; }
            const texture::GeoTexSvx* getTransparenceGeoTexSvx() const { return mpTransparenceGeoTexSvx; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }
            bool getSimpleTextureActive() const { return mbSimpleTextureActive; }

            // access to Drawinglayer configuration options
            const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

// eof
