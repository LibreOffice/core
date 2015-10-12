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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYTRIANGLE_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYTRIANGLE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayTriangle : public OverlayObjectWithBasePosition
        {
        protected:
            // second and third position in pixel
            basegfx::B2DPoint                       maSecondPosition;
            basegfx::B2DPoint                       maThirdPosition;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence() override;

        public:
            OverlayTriangle(
                const basegfx::B2DPoint& rBasePos,
                const basegfx::B2DPoint& rSecondPos,
                const basegfx::B2DPoint& rThirdPos,
                Color aTriangleColor);
            virtual ~OverlayTriangle();

            // get second position
            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }

            // get third position
            const basegfx::B2DPoint& getThirdPosition() const { return maThirdPosition; }
        };
    } // end of namespace overlay
} // end of namespace sdr

#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYTRIANGLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
