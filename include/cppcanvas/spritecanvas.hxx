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

#ifndef INCLUDED_CPPCANVAS_SPRITECANVAS_HXX
#define INCLUDED_CPPCANVAS_SPRITECANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <basegfx/vector/b2dsize.hxx>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/sprite.hxx>
#include <cppcanvas/customsprite.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XSpriteCanvas;
} } } }


/* Definition of SpriteCanvas */

namespace cppcanvas
{
    class SpriteCanvas;

    // forward declaration, since cloneSpriteCanvas() also references SpriteCanvas
    typedef std::shared_ptr< ::cppcanvas::SpriteCanvas > SpriteCanvasSharedPtr;

    /** SpriteCanvas interface
     */
    class SpriteCanvas : public virtual Canvas
    {
        SpriteCanvas(const SpriteCanvas&) = delete;
        SpriteCanvas& operator=( const SpriteCanvas& ) = delete;
    public:
        SpriteCanvas() {}
        virtual bool                    updateScreen( bool bUpdateAll ) const = 0;

        virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const = 0;

        virtual css::uno::Reference<
            css::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const = 0;
    };

}

#endif // INCLUDED_CPPCANVAS_SPRITECANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
