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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
