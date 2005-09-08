/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvas.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:15:28 $
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

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#define _CPPCANVAS_SPRITECANVAS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#include <cppcanvas/bitmapcanvas.hxx>
#endif
#ifndef _CPPCANVAS_SPRITE_HXX
#include <cppcanvas/sprite.hxx>
#endif
#ifndef _CPPCANVAS_CUSTOMSPRITE_HXX
#include <cppcanvas/customsprite.hxx>
#endif

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
