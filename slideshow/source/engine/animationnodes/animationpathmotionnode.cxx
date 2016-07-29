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


#include "animationpathmotionnode.hxx"
#include "animationfactory.hxx"

namespace slideshow {
namespace internal {

void AnimationPathMotionNode::dispose()
{
    mxPathMotionNode.clear();
    AnimationBaseNode::dispose();
}

AnimationActivitySharedPtr AnimationPathMotionNode::createActivity() const
{
    OUString aString;
    ENSURE_OR_THROW( (mxPathMotionNode->getPath() >>= aString),
                      "no string-based SVG:d path found" );

    ActivitiesFactory::CommonParameters const aParms( fillCommonParameters() );
    return ActivitiesFactory::createSimpleActivity(
        aParms,
        AnimationFactory::createPathMotionAnimation(
            aString,
            mxPathMotionNode->getAdditive(),
            getShape(),
            getContext().mpSubsettableShapeManager,
            getSlideSize(), 0 ),
        true );
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
