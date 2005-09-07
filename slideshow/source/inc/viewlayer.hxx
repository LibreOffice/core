/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewlayer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:25:12 $
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

#ifndef _SLIDESHOW_VIEWLAYER_HXX
#define _SLIDESHOW_VIEWLAYER_HXX

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif

#ifndef _BGFX_RANGE_B2IRECTANGLE_HXX
#include <basegfx/range/b2irectangle.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


/* Definition of ViewLayer interface */

namespace presentation
{
    namespace internal
    {
        class ViewLayer
        {
        public:
            virtual ~ViewLayer() {}

            /** Get the associated canvas of this layer.

                The canvas returned by this method must not change, as
                long as this object is alive.
            */
            virtual ::cppcanvas::CanvasSharedPtr getCanvas() const = 0;

            /** Create a sprite for this layer

                @param rSpriteSizePixel
                Sprite size in device pixel

                @return the sprite, or NULL on failure (or if this canvas
                does not support sprites)
            */
            virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& rSpriteSizePixel ) const = 0;

            /** Compute a sprite priority relative to this layer.

                @param nSpritePrio
                Priority of the Sprite within this layer

                @return absolute sprite priority, to be set at a ::cppcanvas::Sprite
            */
            virtual double getSpritePriority( double nSpritePrio ) const = 0;

            /** Set the layer priority

                This method influences the relative priority of this
                layer, i.e. the z position in relation to other layers on
                the parent view. The higher the priority, the further in front
                the layer resides.

                @param nPrio
                Priority, must be in the range [0,1]
            */
            virtual void setPriority( double nPrio ) = 0;
        };

        typedef ::boost::shared_ptr< ViewLayer > ViewLayerSharedPtr;
    }
}

#endif /* _SLIDESHOW_VIEWLAYER_HXX */
