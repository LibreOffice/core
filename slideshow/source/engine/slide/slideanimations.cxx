/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideanimations.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:57:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>

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
            ENSURE_AND_THROW( maContext.mpSubsettableShapeManager,
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
                    OSL_ENSURE( false, rtl::OUStringToOString(
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
