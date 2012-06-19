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

#ifndef _VCLCANVAS_SPRITESURFACE_HXX
#define _VCLCANVAS_SPRITESURFACE_HXX

#include <sal/config.h>

#include "cairo_sprite.hxx"

class Point;
class Size;
class Rectangle;

namespace cairocanvas
{
    /* Definition of SpriteSurface interface */

    class SpriteSurface
    {
    public:
        virtual ~SpriteSurface() {}

        // call this when XSprite::show() is called
        virtual void showSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::hide() is called
        virtual void hideSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::move() is called
        virtual void moveSprite( const Sprite::ImplRef& sprite,
                                 const Point&           rOldPos,
                                 const Point&           rNewPos,
                                 const Size&            rSpriteSize ) = 0;

        // call this when some part of your sprite has changed. That
        // includes show/hide´, i.e. for show, both showSprite and
        // updateSprite must be called.
        virtual void updateSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rPos,
                                   const Rectangle&         rUpdateArea ) = 0;

    };
}

#endif /* _VCLCANVAS_SPRITESURFACE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
