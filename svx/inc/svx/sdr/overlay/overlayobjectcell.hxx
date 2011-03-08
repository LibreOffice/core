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

#ifndef _SDR_OVERLAY_OVERLAYOBJECTCELL_HXX
#define _SDR_OVERLAY_OVERLAYOBJECTCELL_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

#include <vector>

//////////////////////////////////////////////////////////////////////////////

// #114409#
namespace sdr
{
    namespace overlay
    {
        enum CellOverlayType { CELL_OVERLAY_INVERT, CELL_OVERLAY_TRANSPARENT };

        // OverlayObjectCell - used for cell cursor, selection and AutoFill handle

        class SVX_DLLPUBLIC OverlayObjectCell : public OverlayObject
        {
        public:
            typedef ::std::vector< basegfx::B2DRange > RangeVector;

        private:
            CellOverlayType mePaintType;
            RangeVector     maRectangles;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayObjectCell( CellOverlayType eType, const Color& rColor, const RangeVector& rRects);
            virtual ~OverlayObjectCell();
        };

    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYLINE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
