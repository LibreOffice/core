/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_sprite.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 14:42:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
