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


// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "slideanimations.hxx"
#include "animationnodefactory.hxx"

using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        SlideAnimations::SlideAnimations( const SlideShowContext&     rContext,
                                          const ::basegfx::B2DVector& rSlideSize ) :
            maContext( rContext ),
            maSlideSize( rSlideSize ),
            mpRootNode()
        {
            ENSURE_OR_THROW( maContext.mpSubsettableShapeManager,
                              "SlideAnimations::SlideAnimations(): Invalid SlideShowContext" );
        }

        SlideAnimations::~SlideAnimations()
        {
            if( mpRootNode )
            {
                SHOW_NODE_TREE( mpRootNode );

                try
                {
                    mpRootNode->dispose();
                }
                catch (uno::Exception &)
                {
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString(
                                        cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
        }

        bool SlideAnimations::importAnimations( const uno::Reference< animations::XAnimationNode >& xRootAnimationNode )
        {
            mpRootNode = AnimationNodeFactory::createAnimationNode(
                xRootAnimationNode,
                maSlideSize,
                maContext );

            SHOW_NODE_TREE( mpRootNode );

            return static_cast< bool >(mpRootNode);
        }

        bool SlideAnimations::isAnimated() const
        {
            if( !mpRootNode )
                return false; // no animations there

            // query root node about pending animations
            return mpRootNode->hasPendingAnimation();
        }

        bool SlideAnimations::start()
        {
            if( !mpRootNode )
                return false; // no animations there

            // init all nodes
            if( !mpRootNode->init() )
                return false;

            // resolve root node
            if( !mpRootNode->resolve() )
                return false;

            return true;
        }

        void SlideAnimations::end()
        {
            if( !mpRootNode )
                return; // no animations there

            // end root node
            mpRootNode->deactivate();
            mpRootNode->end();
        }

        void SlideAnimations::dispose()
        {
            mpRootNode.reset();
            maContext.dispose();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
