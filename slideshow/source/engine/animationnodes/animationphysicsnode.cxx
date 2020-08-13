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

#include "animationphysicsnode.hxx"
#include <animationfactory.hxx>
#include <o3tl/any.hxx>

#define DEFAULT_START_VELOCITY_X 0.0
#define DEFAULT_START_VELOCITY_Y 0.0
#define DEFAULT_DENSITY 1
#define DEFAULT_BOUNCINESS 0.1

namespace slideshow::internal
{
void AnimationPhysicsNode::dispose()
{
    mxPhysicsMotionNode.clear();
    AnimationBaseNode::dispose();
}

AnimationActivitySharedPtr AnimationPhysicsNode::createActivity() const
{
    double fDuration;
    ENSURE_OR_THROW((mxPhysicsMotionNode->getDuration() >>= fDuration),
                    "Couldn't get the animation duration.");

    ::css::uno::Any aTemp;
    double fStartVelocityX;
    aTemp = mxPhysicsMotionNode->getStartVelocityX();
    if (aTemp.hasValue())
        aTemp >>= fStartVelocityX;
    else
        fStartVelocityX = DEFAULT_START_VELOCITY_X;

    double fStartVelocityY;
    aTemp = mxPhysicsMotionNode->getStartVelocityY();
    if (aTemp.hasValue())
        aTemp >>= fStartVelocityY;
    else
        fStartVelocityY = DEFAULT_START_VELOCITY_Y;

    double fDensity;
    aTemp = mxPhysicsMotionNode->getDensity();
    if (aTemp.hasValue())
        aTemp >>= fDensity;
    else
        fDensity = DEFAULT_DENSITY;

    double fBounciness;
    aTemp = mxPhysicsMotionNode->getBounciness();
    if (aTemp.hasValue())
        aTemp >>= fBounciness;
    else
        fBounciness = DEFAULT_BOUNCINESS;

    ActivitiesFactory::CommonParameters const aParms(fillCommonParameters());
    return ActivitiesFactory::createSimpleActivity(
        aParms,
        AnimationFactory::createPhysicsAnimation(
            getContext().mpBox2DWorld, fDuration, getContext().mpSubsettableShapeManager,
            getSlideSize(), { fStartVelocityX, fStartVelocityY }, fDensity, fBounciness, 0),
        true);
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
