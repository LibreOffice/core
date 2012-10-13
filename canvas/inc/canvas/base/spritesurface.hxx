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

#ifndef INCLUDED_CANVAS_SPRITESURFACE_HXX
#define INCLUDED_CANVAS_SPRITESURFACE_HXX

#include <canvas/base/sprite.hxx>

namespace canvas
{
    /* Definition of the SpriteSurface interface */

    /** Canvas surface containing sprites

        Every canvas surface that contains sprites must implement this
        interface, when employing the canvas base framework. The
        methods provided here are used from the individual sprites to
        notify the canvas about necessary screen updates.
     */
    class SpriteSurface : public ::com::sun::star::uno::XInterface
    {
    public:
        typedef ::rtl::Reference< SpriteSurface > Reference;

        /// Sprites should call this from XSprite::show()
        virtual void showSprite( const Sprite::Reference& rSprite ) = 0;

        /// Sprites should call this from XSprite::hide()
        virtual void hideSprite( const Sprite::Reference& rSprite ) = 0;

        /// Sprites should call this from XSprite::move()
        virtual void moveSprite( const Sprite::Reference&       rSprite,
                                 const ::basegfx::B2DPoint&     rOldPos,
                                 const ::basegfx::B2DPoint&     rNewPos,
                                 const ::basegfx::B2DVector&    rSpriteSize ) = 0;

        /** Sprites should call this when some part of the content has
            changed.

            That includes show/hide, i.e. for show, both showSprite()
            and updateSprite() must be called.
        */
        virtual void updateSprite( const Sprite::Reference&     rSprite,
                                   const ::basegfx::B2DPoint&   rPos,
                                   const ::basegfx::B2DRange&   rUpdateArea ) = 0;

    protected:
        ~SpriteSurface() {}
    };
}

#endif /* INCLUDED_CANVAS_SPRITESURFACE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
