/*************************************************************************
 *
 *  $RCSfile: animationcolornode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:02:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <coloranimation.hxx>
#include <hslcoloranimation.hxx>

#include <animationcolornode.hxx>
#include <animationfactory.hxx>
#include <activitiesfactory.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONCOLORSPACE_HPP_
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#endif


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        namespace
        {
            /** Little wrapper for HSL to RGB mapping.

                This class implements the HSLColorAnimation interface,
                internally converting to RGB and forwarding to
                ColorAnimation.
             */
            class HSLWrapper : public HSLColorAnimation
            {
            public:
                HSLWrapper( const ColorAnimationSharedPtr& rAnimation ) :
                    mpAnimation( rAnimation )
                {
                    ENSURE_AND_THROW( mpAnimation.get(),
                                      "HSLWrapper::HSLWrapper(): Invalid color animation delegate" );
                }

                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer )
                {
                    mpAnimation->start( rShape, rAttrLayer );
                }

                virtual void end()
                {
                    mpAnimation->end();
                }

                virtual bool operator()( const HSLColor& rColor )
                {
                    return (*mpAnimation)( RGBColor( rColor ) );
                }

                virtual HSLColor getUnderlyingValue() const
                {
                    return HSLColor( mpAnimation->getUnderlyingValue() );
                }

            private:
                ColorAnimationSharedPtr mpAnimation;
            };
        }

        AnimationColorNode::AnimationColorNode( const uno::Reference< animations::XAnimationNode >& xNode,
                                                const BaseContainerNodeSharedPtr&                   rParent,
                                                const NodeContext&                                  rContext ) :
            ActivityAnimationBaseNode( xNode, rParent, rContext ),
            mxColorNode( xNode,
                         uno::UNO_QUERY_THROW )
        {
        }

        bool AnimationColorNode::init()
        {
            if( !ActivityAnimationBaseNode::init() )
                return false;

            try
            {
                // TODO(F2): For restart functionality, we must regenerate activities,
                // since they are not able to reset their state (or implement _that_)
                getActivity() = createColorActivity();
            }
            catch( uno::Exception& )
            {
                // catch and ignore. We later handle empty activities, but for
                // other nodes to function properly, the core functionality of
                // this node must remain up and running.
            }

            return true;
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* AnimationColorNode::getDescription() const
        {
            return "AnimationColorNode";
        }
#endif

        AnimationActivitySharedPtr AnimationColorNode::createColorActivity() const
        {
            ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );

            switch( mxColorNode->getColorInterpolation() )
            {
                case animations::AnimationColorSpace::RGB:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createColorPropertyAnimation(
                                                                         mxColorNode->getAttributeName(),
                                                                         getShape(),
                                                                         getContext().mpLayerManager ),
                                                                     getXAnimateNode() );

                case animations::AnimationColorSpace::HSL:
                    // Wrap a plain ColorAnimation with the HSL
                    // wrapper, which implements the HSLColorAnimation
                    // interface, and internally converts HSL to RGB color
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     HSLColorAnimationSharedPtr(
                                                                         new HSLWrapper(
                                                                             AnimationFactory::createColorPropertyAnimation(
                                                                                 mxColorNode->getAttributeName(),
                                                                                 getShape(),
                                                                                 getContext().mpLayerManager ) ) ),
                                                                     mxColorNode );

                default:
                    ENSURE_AND_THROW( false,
                                      "AnimationColorNode::createColorActivity(): Unexpected color space" );
            }

            return AnimationActivitySharedPtr();
        }
    }
}
