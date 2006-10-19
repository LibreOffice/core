/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:33:20 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace basegfx {
    class BZPixelRaster;
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
    class AlphaTexturePrimitive3D;
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
        private:
            // render information
            const attribute::SdrSceneAttribute&                 mrSdrSceneAttribute;    // read-only scene infos (normal handling, etc...)
            const attribute::SdrLightingAttribute&              mrSdrLightingAttribute; // read-only light infos (lights, etc...)
            basegfx::B3DHomMatrix                               maDeviceToView;         // scale and translate to map to target view size
            basegfx::B3DHomMatrix                               maWorldToEye;           // world to eye coordinates
            basegfx::B3DHomMatrix                               maWorldToView;          // mul maWorldToEye with maProjection and maDeviceToView
            basegfx::B3DHomMatrix                               maInvEyeToView;         // back from view to eye coordinates
            basegfx::B3DHomMatrix                               maInvWorldToView;       // back from view to world coordinates
            basegfx::B2DRange                                   maRasterRange;          // the (0, 0, W, H) range from mpBZPixelRaster

            // the target
            basegfx::BZPixelRaster*                             mpBZPixelRaster;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                        maBColorModifierStack;

            // the current active texture
            texture::GeoTexSvx*                                 mpGeoTexSvx;

            // the current active transparence texture
            texture::GeoTexSvx*                                 mpTransparenceGeoTexSvx;

            // bitfield
            unsigned                                            mbModulate : 1;
            unsigned                                            mbFilter : 1;
            unsigned                                            mbProcessTransparent : 1;
            unsigned                                            mbContainsTransparent : 1;

            //////////////////////////////////////////////////////////////////////////////
            // rendering support
            void impRender_GRX3(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence);
            void impRender_HAX3(const primitive3d::HatchTexturePrimitive3D& rPrimitive);
            void impRender_BMX3(const primitive3d::BitmapTexturePrimitive3D& rPrimitive);
            void impRender_MCOL(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate);
            void impRender_POH3(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive);
            void impRender_POM3(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive);
            void impRender_TRN3(const primitive3d::TransformPrimitive3D& rTransformCandidate);

            //////////////////////////////////////////////////////////////////////////////
            // lighting support
            basegfx::BColor impSolveColorModel(
                basegfx::B3DVector aNormal,
                const attribute::MaterialAttribute3D& rMaterial);

        public:
            DefaultProcessor3D(
                const geometry::ViewInformation2D& rViewInformation,
                const geometry::Transformation3D& rTransformation3D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                double fSizeX,
                double fSizeY,
                const basegfx::B2DRange& rVisiblePart);
            virtual ~DefaultProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // helpers for drawing transparent parts in 2nd run
            void processNonTransparent(const primitive3d::Primitive3DSequence& rSource);
            void processTransparent(const primitive3d::Primitive3DSequence& rSource);

            // data read access
            const attribute::SdrLightingAttribute& getSdrLightingAttribute() const { return mrSdrLightingAttribute; }
            const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }
            const texture::GeoTexSvx* getGeoTexSvx() const { return mpGeoTexSvx; }
            const texture::GeoTexSvx* getTransparenceGeoTexSvx() const { return mpTransparenceGeoTexSvx; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }

            // get the result as bitmapEx
            BitmapEx getBitmapEx() const;
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

// eof
