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
#ifndef INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYRECTANGLE_HXX
#define INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYRECTANGLE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <tools/gen.hxx>

namespace tools { class PolyPolygon; }

namespace sdr
{
    namespace overlay
    {
        class OverlayRectangle : public OverlayObjectWithBasePosition
        {
            // geometric definitions
            basegfx::B2DPoint               maSecondPosition;
            const double                    mfTransparence;
            const double                    mfDiscreteGrow;
            const double                    mfDiscreteShrink;
            const double                    mfRotation;

            // #i53216# added CursorBlinkTime (in ms)
            sal_uInt32                      mnBlinkTime;

            /// bitfield
            // Flag to remember which state to draw. Inited with false (0)
            bool                            mbOverlayState : 1;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        public:
            OverlayRectangle(
                const basegfx::B2DPoint& rBasePosition,
                const basegfx::B2DPoint& rSecondPosition,
                const Color& rHatchColor,
                double fTransparence,
                double fDiscreteGrow,
                double fDiscreteShrink,
                double fRotation,
                sal_uInt64 nBlinkTime,
                bool bAnimate);

            // data read access
            double getTransparence() const { return mfTransparence; }
            double getDiscreteGrow() const { return mfDiscreteGrow; }
            double getDiscreteShrink() const { return mfDiscreteShrink; }
            double getRotation() const { return mfRotation; }

            // execute event from base class sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime) override;
        };
    } // end of namespace overlay
} // end of namespace sdr

#endif // INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYRECTANGLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
