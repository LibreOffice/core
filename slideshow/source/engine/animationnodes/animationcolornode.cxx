/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationcolornode.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:30:31 $
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

// must be first
#include "canvas/debug.hxx"
#include "canvas/verbosetrace.hxx"
#include "coloranimation.hxx"
#include "hslcoloranimation.hxx"
#include "animationcolornode.hxx"
#include "animationfactory.hxx"
#include "activitiesfactory.hxx"
#include "com/sun/star/animations/AnimationColorSpace.hpp"

using namespace com::sun::star;

namespace presentation {
namespace internal {

namespace {
/** Little wrapper for HSL to RGB mapping.

    This class implements the HSLColorAnimation interface,
    internally converting to RGB and forwarding to
    ColorAnimation.
*/
class HSLWrapper : public HSLColorAnimation
{
public:
    HSLWrapper( const ColorAnimationSharedPtr& rAnimation )
        : mpAnimation( rAnimation )
    {
        ENSURE_AND_THROW(
            mpAnimation.get(),
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

} // anon namespace

AnimationActivitySharedPtr AnimationColorNode::createActivity() const
{
    ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );

    switch( mxColorNode->getColorInterpolation() )
    {
    case animations::AnimationColorSpace::RGB:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createColorPropertyAnimation(
                mxColorNode->getAttributeName(),
                getShape(),
                getContext().mpLayerManager ),
            getXAnimateNode() );

    case animations::AnimationColorSpace::HSL:
        // Wrap a plain ColorAnimation with the HSL
        // wrapper, which implements the HSLColorAnimation
        // interface, and internally converts HSL to RGB color
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            HSLColorAnimationSharedPtr(
                new HSLWrapper(
                    AnimationFactory::createColorPropertyAnimation(
                        mxColorNode->getAttributeName(),
                        getShape(),
                        getContext().mpLayerManager ) ) ),
            mxColorNode );

    default:
        ENSURE_AND_THROW( false, "AnimationColorNode::createColorActivity(): "
                          "Unexpected color space" );
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace presentation

