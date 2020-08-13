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

constexpr double fDefaultStartVelocityX(0.0);
constexpr double fDefaultStartVelocityY(0.0);
constexpr double fDefaultDensity(1.0);
constexpr double fDefaultBounciness(0.1);

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
        fStartVelocityX = fDefaultStartVelocityX;

    double fStartVelocityY;
    aTemp = mxPhysicsMotionNode->getStartVelocityY();
    if (aTemp.hasValue())
        aTemp >>= fStartVelocityY;
    else
        fStartVelocityY = fDefaultStartVelocityY;

    double fDensity;
    aTemp = mxPhysicsMotionNode->getDensity();
    if (aTemp.hasValue())
    {
        aTemp >>= fDensity;
        fDensity = (fDensity < 0.0) ? 0.0 : fDensity;
    }
    else
        fDensity = fDefaultDensity;

    double fBounciness;
    aTemp = mxPhysicsMotionNode->getBounciness();
    if (aTemp.hasValue())
    {
        aTemp >>= fBounciness;
        fBounciness = std::clamp(fBounciness, 0.0, 1.0);
    }
    else
        fBounciness = fDefaultBounciness;

    ActivitiesFactory::CommonParameters const aParms(fillCommonParameters());
    return ActivitiesFactory::createSimpleActivity(
        aParms,
        AnimationFactory::createPhysicsAnimation(
            getContext().mpBox2DWorld, fDuration, getContext().mpSubsettableShapeManager,
            getSlideSize(), { fStartVelocityX, fStartVelocityY }, fDensity, fBounciness, 0),
        true);
}

bool AnimationPhysicsNode::enqueueActivity() const
{
    return getContext().mrActivitiesQueue.addTailActivity(mpActivity);
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
