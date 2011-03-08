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

#ifndef _SDR_OVERLAY_OVERLAYSELECTION_HXX
#define _SDR_OVERLAY_OVERLAYSELECTION_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/region.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        // overlay type definition
        enum OverlayType
        {
            OVERLAY_INVERT,
            OVERLAY_SOLID,
            OVERLAY_TRANSPARENT
        };

        class SVX_DLLPUBLIC OverlaySelection : public OverlayObject
        {
        protected:
            // type of overlay
            OverlayType                         meOverlayType;

            // geometry of overlay
            std::vector< basegfx::B2DRange >    maRanges;

            // Values of last primitive creation. These are checked in getOverlayObjectPrimitive2DSequence
            // to evtl. get rid of last Primitive2DSequence. This ensures that these values are up-to-date
            // and are usable when creating primitives
            OverlayType                         maLastOverlayType;
            sal_uInt16                          mnLastTransparence;

            // bitfield
            unsigned                            mbBorder : 1;

            // geometry creation for OverlayObject, can use local *Last* values
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlaySelection(
                OverlayType eType,
                const Color& rColor,
                const std::vector< basegfx::B2DRange >& rRanges,
                bool bBorder);
            virtual ~OverlaySelection();

            // data read access
            OverlayType getOverlayType() const { return meOverlayType; }
            const std::vector< basegfx::B2DRange >& getRanges() const { return maRanges; }
            bool getBorder() const { return mbBorder; }

            // overloaded to check conditions for last createOverlayObjectPrimitive2DSequence
            virtual drawinglayer::primitive2d::Primitive2DSequence getOverlayObjectPrimitive2DSequence() const;

            // data write access
            void setRanges(const std::vector< basegfx::B2DRange >& rNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYSELECTION_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
