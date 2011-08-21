/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
