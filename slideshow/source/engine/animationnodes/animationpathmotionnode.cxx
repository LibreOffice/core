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
    rtl::OUString aString;
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
            getSlideSize() ),
        true );
}

} // namespace internal
} // namespace slideshow

