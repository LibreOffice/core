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
#ifndef INCLUDED_SW_SOURCE_CORE_CRSR_OVERLAYRANGESOUTLINE_HXX
#define INCLUDED_SW_SOURCE_CORE_CRSR_OVERLAYRANGESOUTLINE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/range/b2drange.hxx>

#include <vector>

namespace sw
{
    namespace overlay
    {
        class OverlayRangesOutline : public sdr::overlay::OverlayObject
        {
        protected:
            // geometry of overlay
            std::vector< basegfx::B2DRange > maRanges;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence() override;

        public:
            OverlayRangesOutline(
                const Color& rColor,
                const std::vector< basegfx::B2DRange >& rRanges );

            virtual ~OverlayRangesOutline();

            // data read access
            inline const std::vector< basegfx::B2DRange >& getRanges() const
            {
                return maRanges;
            }

            // data write access
            void setRanges(const std::vector< basegfx::B2DRange >& rNew);
        };
    } // end of namespace overlay
} // end of namespace sw

#endif // INCLUDED_SW_SOURCE_CORE_CRSR_OVERLAYRANGESOUTLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
