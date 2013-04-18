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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
