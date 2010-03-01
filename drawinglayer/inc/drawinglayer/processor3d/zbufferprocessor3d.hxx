/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zbufferprocessor3d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:18 $
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

#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#include <vcl/bitmapex.hxx>

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
    }
}

class ZBufferRasterConverter3D;
class RasterPrimitive3D;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        /** ZBufferProcessor3D class

            This 3D renderer derived from DefaultProcessor3D renders all feeded primitives to a 2D
            raster bitmap using a Z-Buffer based approach. It is able to supersample and to handle
            transparent content.
         */
        class ZBufferProcessor3D : public DefaultProcessor3D
        {
        private:
            /// the raster target, a Z-Buffer
            basegfx::BZPixelRaster*                             mpBZPixelRaster;

            /// inverse of EyeToView for rasterconversion with evtl. Phong shading
            basegfx::B3DHomMatrix                               maInvEyeToView;

            /// The raster converter for Z-Buffer
            ZBufferRasterConverter3D*                           mpZBufferRasterConverter3D;

            /*  AA value. Defines how many oversámples will be used in X and Y. Values 0, 1
                will switch it off while e.g. 2 will use 2x2 pixels for each pixel to create
              */
            sal_uInt16                                          mnAntiAlialize;

            /*  remembered RasterPrimitive3D's which need to be painted back to front
                for transparent 3D parts
             */
            std::vector< RasterPrimitive3D >*                   mpRasterPrimitive3Ds;

            //////////////////////////////////////////////////////////////////////////////
            // rasterconversions for filled and non-filled polygons

            virtual void rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const;
            virtual void rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const;

        public:
            ZBufferProcessor3D(
                const geometry::ViewInformation3D& rViewInformation3D,
                const geometry::ViewInformation2D& rViewInformation2D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                double fSizeX,
                double fSizeY,
                const basegfx::B2DRange& rVisiblePart,
                sal_uInt16 nAntiAlialize);
            virtual ~ZBufferProcessor3D();

            void finish();

            /// get the result as bitmapEx
            BitmapEx getBitmapEx() const;
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX

// eof
