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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_ZBUFFERPROCESSOR3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#include <vcl/bitmapex.hxx>

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

namespace drawinglayer
{
    namespace processor3d
    {
        /**
            This 3D renderer derived from DefaultProcessor3D renders all feeded primitives to a 2D
            raster bitmap using a Z-Buffer based approach. It is able to supersample and to handle
            transparent content.
         */
        class ZBufferProcessor3D : public DefaultProcessor3D
        {
        private:
            /// the raster target, a Z-Buffer
            basegfx::BZPixelRaster* mpBZPixelRaster;

            /// inverse of EyeToView for rasterconversion with evtl. Phong shading
            basegfx::B3DHomMatrix maInvEyeToView;

            /// The raster converter for Z-Buffer
            ZBufferRasterConverter3D* mpZBufferRasterConverter3D;

            /*  AA value. Defines how many oversamples will be used in X and Y. Values 0, 1
                will switch it off while e.g. 2 will use 2x2 pixels for each pixel to create
              */
            sal_uInt16 mnAntiAlialize;

            /*  remembered RasterPrimitive3D's which need to be painted back to front
                for transparent 3D parts
             */
            std::vector< RasterPrimitive3D >* mpRasterPrimitive3Ds;

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
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
