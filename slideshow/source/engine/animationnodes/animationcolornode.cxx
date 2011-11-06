/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <com/sun/star/animations/AnimationColorSpace.hpp>

#include "coloranimation.hxx"
#include "hslcoloranimation.hxx"
#include "animationcolornode.hxx"
#include "animationfactory.hxx"
#include "activitiesfactory.hxx"

using namespace com::sun::star;

namespace slideshow {
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
        ENSURE_OR_THROW(
            mpAnimation,
            "HSLWrapper::HSLWrapper(): Invalid color animation delegate" );
    }

    virtual void prefetch( const AnimatableShapeSharedPtr&,
                           const ShapeAttributeLayerSharedPtr& )
    {}

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
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
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
                        getContext().mpSubsettableShapeManager,
                        getSlideSize() ))),
            mxColorNode );

    default:
        ENSURE_OR_THROW( false, "AnimationColorNode::createColorActivity(): "
                          "Unexpected color space" );
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace slideshow

