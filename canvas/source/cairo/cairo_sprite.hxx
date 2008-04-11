/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_sprite.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _CAIROCANVAS_SPRITE_HXX
#define _CAIROCANVAS_SPRITE_HXX

#include <canvas/base/sprite.hxx>

#include "cairo_cairo.hxx"

namespace cairocanvas
{
    /** Specialization of ::canvas::Sprite interface, to also provide
        redraw methods.
     */
    class Sprite : public ::canvas::Sprite
    {
    public:

        /** Redraw sprite at the stored position.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        virtual void redraw( ::cairo::Cairo* pCairo,
                             bool            bBufferedUpdate ) const = 0;

        /** Redraw sprite at the given position.

            @param rPos
            Output position of the sprite. Overrides the sprite's own
            output position.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
        */
        virtual void redraw( ::cairo::Cairo*            pCairo,
                             const ::basegfx::B2DPoint& rOrigOutputPos,
                             bool                       bBufferedUpdate ) const = 0;
    };
}

#endif /* _CAIROCANVAS_SPRITE_HXX */
