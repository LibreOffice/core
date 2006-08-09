/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:45:32 $
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

#ifndef _DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#define _DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX

#ifndef _DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef _DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace basegfx {
    class BZPixelRaster;
}

namespace drawinglayer { namespace attribute {
    class sdrSceneAttribute;
    class sdrLightingAttribute;
    class materialAttribute3D;
}}

namespace drawinglayer { namespace primitive3d {
    class polygonHairlinePrimitive3D;
    class polyPolygonMaterialPrimitive3D;
    class gradientTexturePrimitive3D;
    class hatchTexturePrimitive3D;
    class bitmapTexturePrimitive3D;
    class transparenceTexturePrimitive3D;
    class transformPrimitive3D;
}}

namespace drawinglayer { namespace texture {
    class geoTexSvx;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class defaultProcessor3D : public baseProcessor3D
        {
        protected:
            // render information
            const attribute::sdrSceneAttribute&                 mrSdrSceneAttribute;    // read-only scene infos (normal handling, etc...)
            const attribute::sdrLightingAttribute&              mrSdrLightingAttribute; // read-only light infos (lights, etc...)
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
            texture::geoTexSvx*                             mpGeoTexSvx;

            // the current active transparence texture
            texture::geoTexSvx*                             mpTransparenceGeoTexSvx;

            // bitfield
            unsigned                                            mbModulate : 1;
            unsigned                                            mbFilter : 1;
            unsigned                                            mbProcessTransparent : 1;
            unsigned                                            mbContainsTransparent : 1;

            //////////////////////////////////////////////////////////////////////////////
            // rendering support
            void impRender_GRX3(const primitive3d::gradientTexturePrimitive3D& rPrimitive, bool bTransparence);
            void impRender_HAX3(const primitive3d::hatchTexturePrimitive3D& rPrimitive);
            void impRender_BMX3(const primitive3d::bitmapTexturePrimitive3D& rPrimitive);
            void impRender_MCOL(const primitive3d::modifiedColorPrimitive3D& rModifiedCandidate);
            void impRender_POH3(const primitive3d::polygonHairlinePrimitive3D& rPrimitive);
            void impRender_POM3(const primitive3d::polyPolygonMaterialPrimitive3D& rPrimitive);
            void impRender_TRN3(const primitive3d::transformPrimitive3D& rTransformCandidate);

            //////////////////////////////////////////////////////////////////////////////
            // lighting support
            basegfx::BColor impSolveColorModel(
                basegfx::B3DVector aNormal,
                const attribute::materialAttribute3D& rMaterial);

        public:
            defaultProcessor3D(
                const geometry::viewInformation& rViewInformation,
                const geometry::transformation3D& rTransformation3D,
                const attribute::sdrSceneAttribute& rSdrSceneAttribute,
                const attribute::sdrLightingAttribute& rSdrLightingAttribute,
                double fSizeX,
                double fSizeY,
                const basegfx::B2DRange& rVisiblePart);
            virtual ~defaultProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::primitiveVector3D& rSource);

            // helpers for drawing transparent parts in 2nd run
            void processNonTransparent(const primitive3d::primitiveVector3D& rSource);
            void processTransparent(const primitive3d::primitiveVector3D& rSource);

            // data read access
            const attribute::sdrLightingAttribute& getSdrLightingAttribute() const { return mrSdrLightingAttribute; }
            const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }
            const texture::geoTexSvx* getGeoTexSvx() const { return mpGeoTexSvx; }
            const texture::geoTexSvx* getTransparenceGeoTexSvx() const { return mpTransparenceGeoTexSvx; }
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
