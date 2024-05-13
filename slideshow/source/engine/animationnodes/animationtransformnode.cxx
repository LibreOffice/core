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


#include <com/sun/star/animations/AnimationTransformType.hpp>

#include "animationtransformnode.hxx"
#include <animationfactory.hxx>
#include <activitiesfactory.hxx>

using namespace com::sun::star;

namespace slideshow::internal {

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
        throw css::uno::RuntimeException(
            u"AnimationTransformNode::createTransformActivity(): "
            "Unknown transform type"_ustr );

    case animations::AnimationTransformType::TRANSLATE:
    case animations::AnimationTransformType::SCALE:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createPairPropertyAnimation(
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                nTransformType, 0 ),
            getXAnimateNode() );

    case animations::AnimationTransformType::ROTATE:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                u"Rotate"_ustr,
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                getContext().mpBox2DWorld ),
            getXAnimateNode() );

    case animations::AnimationTransformType::SKEWX:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                u"SkewX"_ustr,
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                getContext().mpBox2DWorld ),
            getXAnimateNode() );

    case animations::AnimationTransformType::SKEWY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                u"SkewY"_ustr,
                rShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                getContext().mpBox2DWorld ),
            getXAnimateNode() );
    }
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
