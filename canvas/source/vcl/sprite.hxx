/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sprite.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:03:59 $
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

#ifndef INCLUDED_VCLCANVAS_SPRITE_HXX
#define INCLUDED_VCLCANVAS_SPRITE_HXX

#include <canvas/base/sprite.hxx>

class OutputDevice;

namespace vclcanvas
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
        virtual void redraw( OutputDevice& rOutDev,
                             bool          bBufferedUpdate ) const = 0;

        /** Redraw sprite at the given position.

            @param rPos
            Output position of the sprite. Overrides the sprite's own
            output position.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        virtual void redraw( OutputDevice&              rOutDev,
                             const ::basegfx::B2DPoint& rPos,
                             bool                       bBufferedUpdate ) const = 0;
    };
}

#endif /* INCLUDED_VCLCANVAS_SPRITE_HXX */
