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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SLIDEANIMATIONS_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SLIDEANIMATIONS_HXX

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
            bool importAnimations( const css::uno::Reference< css::animations::XAnimationNode >&   xRootAnimationNode );

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
                queues. These queues are not explicitly cleared, if
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

        private:
            SlideShowContext         maContext;
            const basegfx::B2DVector maSlideSize;
            AnimationNodeSharedPtr   mpRootNode;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SLIDEANIMATIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
