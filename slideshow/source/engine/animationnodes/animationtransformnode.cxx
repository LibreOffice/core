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
#include <com/sun/star/animations/AnimationTransformType.hpp>

#include "animationtransformnode.hxx"
#include "animationfactory.hxx"
#include "activitiesfactory.hxx"

using namespace com::sun::star;

namespace slideshow {
namespace internal {

void AnimationTransformNode::dispose()
{
    mxTransformNode.clear();
    AnimationBaseNode::dispose();
}

AnimationActivitySharedPtr AnimationTransformNode::createActivity() const
{
    ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );

    const sal_Int16 nTransformType( mxTransformNode->getTransformType() );

    const AttributableShapeSharedPtr& rShape( getShape() );

    switch( nTransformType )
    {
    default:
        ENSURE_OR_THROW(
            false, "AnimationTransformNode::createTransformActivity(): "
            "Unknown transform type" );

    case animations::AnimationTransformType::TRANSLATE:
        // FALLTHROUGH intended
    case animations::AnimationTransformType::SCALE:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createPairPropertyAnimation(
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                nTransformType ),
            getXAnimateNode() );

    case animations::AnimationTransformType::ROTATE:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Rotate") ),
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            getXAnimateNode() );

    case animations::AnimationTransformType::SKEWX:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("SkewX") ),
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            getXAnimateNode() );

    case animations::AnimationTransformType::SKEWY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("SkewY") ),
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            getXAnimateNode() );
    }
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
