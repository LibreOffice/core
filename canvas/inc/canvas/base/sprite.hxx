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

    protected:
        ~Sprite() {}
    };

    /** Functor providing a StrictWeakOrdering for sprite references
     */
    struct SpriteWeakOrder
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
