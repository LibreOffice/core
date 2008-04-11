/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spritecanvas.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#define _CPPCANVAS_SPRITECANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <basegfx/vector/b2dsize.hxx>

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
    class SpriteCanvas : public virtual BitmapCanvas
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
