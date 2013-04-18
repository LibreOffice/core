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

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#define _CPPCANVAS_SPRITECANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <basegfx/vector/b2dsize.hxx>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


#include <cppcanvas/bitmapcanvas.hxx>
#include <cppcanvas/sprite.hxx>
#include <cppcanvas/customsprite.hxx>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XSpriteCanvas;
} } } }


/* Definition of SpriteCanvas */

namespace cppcanvas
{
    class SpriteCanvas;

    // forward declaration, since cloneSpriteCanvas() also references SpriteCanvas
    typedef ::boost::shared_ptr< ::cppcanvas::SpriteCanvas > SpriteCanvasSharedPtr;

    /** SpriteCanvas interface
     */
    class SpriteCanvas : public virtual BitmapCanvas, private boost::noncopyable
    {
    public:
        virtual bool                    updateScreen( bool bUpdateAll ) const = 0;

        virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const = 0;
        virtual SpriteSharedPtr         createClonedSprite( const SpriteSharedPtr& ) const = 0;

        // shared_ptr does not allow for covariant return types
        SpriteCanvasSharedPtr           cloneSpriteCanvas() const
        {
            SpriteCanvasSharedPtr p( ::boost::dynamic_pointer_cast< SpriteCanvas >(this->clone()) );
            OSL_ENSURE(p.get(), "SpriteCanvas::cloneSpriteCanvas(): dynamic cast failed");
            return p;
        }

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const = 0;
    };

}

#endif /* _CPPCANVAS_SPRITECANVAS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
