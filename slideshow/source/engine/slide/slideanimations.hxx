/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideanimations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:57:42 $
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

#ifndef INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX
#define INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <basegfx/vector/b2dvector.hxx>

#include "event.hxx"
#include "slideshowcontext.hxx"
#include "subsettableshapemanager.hxx"
#include "animationnode.hxx"

namespace com { namespace sun { namespace star { namespace animations
{
    class XAnimationNode;
} } } }


/* Definition of SlideAnimations class */

namespace slideshow
{
    namespace internal
    {
        /** This class generates and manages all animations of a slide.

            Provided with the root animation node, this class imports
            the effect information and builds the event tree for all
            of the slide's animations.
         */
        class SlideAnimations
        {
        public:
            /** Create an animation generator.

                @param rContext
                Slide show context, passing on common parameters
             */
            SlideAnimations( const SlideShowContext&     rContext,
                             const ::basegfx::B2DVector& rSlideSize );
            ~SlideAnimations();

            /** Import animations from a SMIL root animation node.

                This method might take some time, depending on the
                complexity of the SMIL animation network to be
                imported.

                @param xRootAnimationNode
                Root animation node for the effects to be
                generated. This is typically obtained from the
                XDrawPage's XAnimationNodeSupplier.

             */
            bool importAnimations( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::animations::XAnimationNode >&   xRootAnimationNode );

            /** Check, whether imported animations actually contain
                any effects.

                @return true, if there are actual animations in the
                imported node tree.
             */
            bool isAnimated() const;

            /** Start the animations.

                This method creates the network of events and
                activities for all animations. The events and
                activities are inserted into the constructor-provided
                queues. These queues are not explicitely cleared, if
                you rely on this object's effects to run without
                interference, you should clear the queues by yourself.

                @return true, if all events have been successfully
                created.
             */
            bool start();

            /** End all animations.

                This method force-ends all animations. If a slide end
                event has been registered, that is fired, too.
             */
            void end();

            /// Release all references. Does not notify anything.
            void dispose();

        private:
            SlideShowContext         maContext;
            const basegfx::B2DVector maSlideSize;
            AnimationNodeSharedPtr   mpRootNode;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX */
