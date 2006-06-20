/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sprite.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:14:13 $
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

#ifndef INCLUDED_CANVAS_SPRITE_HXX
#define INCLUDED_CANVAS_SPRITE_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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

namespace basegfx
{
    class B2DPoint;
    class B2DVector;
    class B2DRange;
}

namespace canvas
{
    /* Definition of Sprite interface (as we mix with UNO here, has to
       be XInterface - reference holders to a Sprite must be able to
       control lifetime of reference target)
     */

    /** Helper interface to connect SpriteCanvas with various
        sprite implementations.

        This interface should be implemented from every sprite class,
        as it provides essential repaint and update area facilitates.

        @derive typically, each canvas implementation will derive
        another interface from this one, that adds rendering
        functionality (which, of course, is impossible here in a
        generic way)
    */
    class Sprite : public ::com::sun::star::lang::XComponent
    {
    public:
        typedef ::rtl::Reference< Sprite > Reference;

        /** Query whether sprite update will fully cover the given area.

            Use this method to determine whether any background
            content (regardless of static or sprite) needs an update
            before rendering this sprite.

            @return true, if sprite redraw will fully overwrite given
            area (and thus, the background need not be redrawn
            beforehand).
         */
        virtual bool isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const = 0;

        /** Query position of the left, top pixel of the sprite
         */
        virtual ::basegfx::B2DPoint getPosPixel() const = 0;

        /** Query size of the sprite in pixel.
         */
        virtual ::basegfx::B2DVector getSizePixel() const = 0;

        /** Get area that is currently covered by the sprite

            This area is already adapted to clipping, alpha and
            transformation state of this sprite.
         */
        virtual ::basegfx::B2DRange getUpdateArea() const = 0;

        /** Query sprite priority
         */
        virtual double              getPriority() const = 0;
    };

    /** Functor providing a StrictWeakOrdering for sprite references
     */
    struct SpriteComparator
    {
        bool operator()( const Sprite::Reference& rLHS,
                         const Sprite::Reference& rRHS )
        {
            const double nPrioL( rLHS->getPriority() );
            const double nPrioR( rRHS->getPriority() );

            // if prios are equal, tie-break on ptr value
            return nPrioL == nPrioR ? rLHS.get() < rRHS.get() : nPrioL < nPrioR;
        }
    };
}

#endif /* INCLUDED_CANVAS_SPRITE_HXX */
