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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYSELECTION_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYSELECTION_HXX

#include <svx/sdr/overlay/overlayobject.hxx>


namespace sdr::overlay
    {
        // overlay type definition
        enum class OverlayType
        {
            Invert,
            Solid,
            Transparent
        };

        class SVXCORE_DLLPUBLIC OverlaySelection final : public OverlayObject
        {
            // type of overlay
            OverlayType                         meOverlayType;

            // geometry of overlay
            std::vector< basegfx::B2DRange >    maRanges;

            // Values of last primitive creation. These are checked in getOverlayObjectPrimitive2DSequence
            // to evtl. get rid of last Primitive2DContainer. This ensures that these values are up-to-date
            // and are usable when creating primitives
            OverlayType                         maLastOverlayType;
            sal_uInt16                          mnLastTransparence;

            bool                                mbBorder : 1;

            // geometry creation for OverlayObject, can use local *Last* values
            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        public:
            OverlaySelection(
                OverlayType eType,
                const Color& rColor,
                const std::vector< basegfx::B2DRange >& rRanges,
                bool bBorder);
            virtual ~OverlaySelection() override;

            // data read access
            const std::vector< basegfx::B2DRange >& getRanges() const { return maRanges; }

            /// override to check conditions for last createOverlayObjectPrimitive2DSequence
            virtual drawinglayer::primitive2d::Primitive2DContainer getOverlayObjectPrimitive2DSequence() const override;

            // data write access
            void setRanges(const std::vector< basegfx::B2DRange >& rNew);
        };
} // end of namespace sdr::overlay


#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYSELECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
