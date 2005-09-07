/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sprite.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:23:01 $
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

#ifndef _VCLCANVAS_SPRITE_HXX
#define _VCLCANVAS_SPRITE_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/rendering/XCanvas.hpp>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/vector/b2dpoint.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif

class OutputDevice;

namespace vclcanvas
{
    /* Definition of Sprite class */

    /** Helper interface to connect SpriteCanvas with various
        sprite implementations.
    */

    class Sprite : public ::com::sun::star::uno::XInterface
    {
    public:
        typedef ::rtl::Reference< Sprite > ImplRef;

        virtual ~Sprite() {}

        /** Repaint whole sprite on given target surface
         */
        virtual void redraw( OutputDevice& rTargetSurface ) const = 0;

        /** Repaint sprite.

            @param rTargetSurface
            Target surface to output sprite content

            @param rOutputPos
            Position where to render the sprite (overrides the
            sprite's output position)
         */
        virtual void redraw( OutputDevice&  rTargetSurface,
                             const Point&   rOutputPos ) const = 0;

        /** Query whether sprite update will fully cover the given area.

            Use this method to determine whether any background
            content (regardless of static or sprite) needs an update
            before rendering this sprite.

            @return true, if sprite redraw will fully overwrite given
            area.
         */
        virtual bool isAreaUpdateOpaque( const Rectangle& rUpdateArea ) const = 0;

        virtual ::basegfx::B2DPoint getSpritePos() const = 0;
        virtual ::basegfx::B2DSize  getSpriteSize() const = 0;
    };
}

#endif /* _VCLCANVAS_SPRITE_HXX */
