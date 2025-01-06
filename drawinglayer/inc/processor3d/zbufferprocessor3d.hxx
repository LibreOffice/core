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

#pragma once

#include <processor3d/defaultprocessor3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <memory>

namespace basegfx {
    class BZPixelRaster;
}

namespace drawinglayer::attribute {
        class SdrSceneAttribute;
        class SdrLightingAttribute;
}


class ZBufferRasterConverter3D;
class RasterPrimitive3D;

namespace drawinglayer::processor3d
    {
        /**
            This 3D renderer derived from DefaultProcessor3D renders all fed primitives to a 2D
            raster bitmap using a Z-Buffer based approach. It is able to supersample and to handle
            transparent content.
         */
        class ZBufferProcessor3D final : public DefaultProcessor3D
        {
        private:
            /// inverse of EyeToView for rasterconversion with evtl. Phong shading
            basegfx::B3DHomMatrix maInvEyeToView;

            /// The raster converter for Z-Buffer
            std::unique_ptr<ZBufferRasterConverter3D> mpZBufferRasterConverter3D;

            /*  AA value. Defines how many oversamples will be used in X and Y. Values 0, 1
                will switch it off while e.g. 2 will use 2x2 pixels for each pixel to create
              */
            sal_uInt16 mnAntiAlialize;

            /*  remembered RasterPrimitive3D's which need to be painted back to front
                for transparent 3D parts
             */
            mutable std::vector< RasterPrimitive3D > maRasterPrimitive3Ds;

            sal_uInt32 mnStartLine;
            sal_uInt32 mnStopLine;

            // rasterconversions for filled and non-filled polygons

            virtual void rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const override;
            virtual void rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const override;

        public:
            ZBufferProcessor3D(
                const geometry::ViewInformation3D& rViewInformation3D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                const basegfx::B2DRange& rVisiblePart,
                sal_uInt16 nAntiAlialize,
                double fFullViewSizeX,
                double fFullViewSizeY,
                basegfx::BZPixelRaster& rBZPixelRaster,
                sal_uInt32 nStartLine,
                sal_uInt32 nStopLine);
            virtual ~ZBufferProcessor3D() override;

            void finish();
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
