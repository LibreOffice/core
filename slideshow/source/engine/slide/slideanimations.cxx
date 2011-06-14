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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
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
                    OSL_FAIL( rtl::OUStringToOString(
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

            return mpRootNode;
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
