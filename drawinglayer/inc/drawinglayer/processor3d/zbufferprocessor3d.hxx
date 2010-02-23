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
                const geometry::ViewInformation3D& rViewInformation3D,
                const geometry::ViewInformation2D& rViewInformation2D,
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
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX

// eof
