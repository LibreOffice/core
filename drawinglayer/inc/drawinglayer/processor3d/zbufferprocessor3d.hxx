/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zbufferprocessor3d.hxx,v $
 *
 *  $Revision: 1.1 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BZPixelRaster;
}

namespace drawinglayer {
    namespace attribute {
        class SdrSceneAttribute;
        class SdrLightingAttribute;
        class MaterialAttribute3D;
    }
    namespace geometry {
        class ViewInformation2D;
        class Transformation3D;
    }
}

class ZBufferRasterConverter3D;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class ZBufferProcessor3D : public DefaultProcessor3D
        {
        private:
            // the raster target, a Z-Buffer
            basegfx::BZPixelRaster*                             mpBZPixelRaster;

            // inverse of EyeToView for rasterconversion with evtl. Phong shading
            basegfx::B3DHomMatrix                               maInvEyeToView;

            // The raster converter for Z-Buffer
            ZBufferRasterConverter3D*                           mpZBufferRasterConverter3D;

            // AA value. Defines how many oversámples will be used in X and Y. Values 0, 1
            // will switch it off while e.g. 2 will use 2x2 pixels for each pixel to create
            sal_uInt16                                          mnAntiAlialize;

            // bitfield
            // a combination of bools to allow two-pass rendering to render
            // the transparent parts in the 2nd run (if any) as needed for Z-Buffer
            unsigned                                            mbProcessTransparent : 1;
            unsigned                                            mbContainsTransparent : 1;


            //////////////////////////////////////////////////////////////////////////////
            // rasterconversions for filled and non-filled polygons
            virtual void rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const;
            virtual void rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const;

            // the processing method for a single, known primitive
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive);

        public:
            ZBufferProcessor3D(
                const geometry::ViewInformation2D& rViewInformation,
                const geometry::Transformation3D& rTransformation3D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                double fSizeX,
                double fSizeY,
                const basegfx::B2DRange& rVisiblePart,
                sal_uInt16 nAntiAlialize);
            virtual ~ZBufferProcessor3D();

            // helpers for drawing transparent parts in 2nd run. To use this
            // processor, call processNonTransparent and then processTransparent
            // with the same primitives. The 2nd call will only do something,
            // when transparent parts are contained
            void processNonTransparent(const primitive3d::Primitive3DSequence& rSource);
            void processTransparent(const primitive3d::Primitive3DSequence& rSource);

            // get the result as bitmapEx
            BitmapEx getBitmapEx() const;

            // data access
            const basegfx::B3DHomMatrix& getViewToEye() const { return maInvEyeToView; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX

// eof
