/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <drawinglayer/drawinglayerdllapi.h>

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
        class DRAWINGLAYER_DLLPUBLIC ZBufferProcessor3D : public DefaultProcessor3D
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
