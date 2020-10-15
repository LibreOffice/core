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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


namespace sdr::overlay
{
        class SVXCORE_DLLPUBLIC OverlayPolyPolygon final : public OverlayObject
        {
        private:
            basegfx::B2DPolyPolygon maLinePolyPolygon;
            double mfLineWidth;
            Color maFillColor;

            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        public:
            explicit OverlayPolyPolygon(const basegfx::B2DPolyPolygon& rLinePolyPolygon,
                                        Color const & rLineColor,
                                        double mfLineWidth,
                                        Color const & rFillColor);
            virtual ~OverlayPolyPolygon() override;
        };

        class SVXCORE_DLLPUBLIC OverlayPolyPolygonStripedAndFilled final : public OverlayObject
        {
            // geometry
            basegfx::B2DPolyPolygon             maLinePolyPolygon;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        public:
            explicit OverlayPolyPolygonStripedAndFilled(
                const basegfx::B2DPolyPolygon& rLinePolyPolygon);
            virtual ~OverlayPolyPolygonStripedAndFilled() override;

            // change geometry
            const basegfx::B2DPolyPolygon& getLinePolyPolygon() const { return maLinePolyPolygon; }

            // react on stripe definition change
            virtual void stripeDefinitionHasChanged() override;
        };

} // end of namespace sdr::overlay


#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
