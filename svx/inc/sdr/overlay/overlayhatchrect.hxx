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

#ifndef INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYHATCHRECT_HXX
#define INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYHATCHRECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/hatch.hxx>



class PolyPolygon;

namespace sdr
{
    namespace overlay
    {
        class OverlayHatchRect : public OverlayObjectWithBasePosition
        {
            // geometric definitions
            basegfx::B2DPoint               maSecondPosition;
            const double                    mfDiscreteGrow;
            const double                    mfDiscreteShrink;
            const double                    mfHatchRotation;
            const double                    mfRotation;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayHatchRect(
                const basegfx::B2DPoint& rBasePosition,
                const basegfx::B2DPoint& rSecondPosition,
                const Color& rHatchColor,
                double fDiscreteGrow,
                double fDiscreteShrink,
                double fHatchRotation,
                double fRotation);

            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }

            // data read access
            double getDiscreteGrow() const { return mfDiscreteGrow; }
            double getDiscreteShrink() const { return mfDiscreteShrink; }
            double getHatchRotation() const { return mfHatchRotation; }
            double getRotation() const { return mfRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr



#endif // INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYHATCHRECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
