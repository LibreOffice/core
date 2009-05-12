/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sprite.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_CANVAS_SPRITE_HXX
#define INCLUDED_CANVAS_SPRITE_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>

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

        /** Query whether content has changed
         */
        virtual bool isContentChanged() const = 0;

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
