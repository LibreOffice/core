/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-14 09:21:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLORMODIFIER_HXX
#include <basegfx/color/bcolormodifier.hxx>
#endif

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

            // renderer transformations and range. Need to be correctly set by the
            // derived implementations
            basegfx::B3DHomMatrix                               maWorldToEye;           // world to eye coordinates
            basegfx::B3DHomMatrix                               maWorldToView;          // mul maWorldToEye with maProjection and maDeviceToView
            basegfx::B2DRange                                   maRasterRange;          // the (0, 0, W, H) range from mpBZPixelRaster

            // inverse from maWorldToView, filled on demand
            basegfx::B3DHomMatrix                               maInvWorldToView;       // back from view to world coordinates

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                        maBColorModifierStack;

            // the current active texture
            texture::GeoTexSvx*                                 mpGeoTexSvx;

            // the current active transparence texture
            texture::GeoTexSvx*                                 mpTransparenceGeoTexSvx;

            // width and height in target pixels
            sal_uInt32                                          mnRasterWidth;
            sal_uInt32                                          mnRasterHeight;

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
                double fTime,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute);
            virtual ~DefaultProcessor3D();

            // the central processing method. It checks for known primitive implementation
            // and uses processBasePrimitive3D or forwards to API implementations
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

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
            sal_uInt32 getRasterWidth() const { return mnRasterWidth; }
            sal_uInt32 getRasterHeight() const { return mnRasterHeight; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

// eof
