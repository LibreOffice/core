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

#include "propertyanimationnode.hxx"
#include "animationfactory.hxx"

using namespace com::sun::star;

namespace slideshow {
namespace internal {

AnimationActivitySharedPtr PropertyAnimationNode::createActivity() const
{
    // Create AnimationActivity from common XAnimate parameters:
    ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );
    uno::Reference<animations::XAnimate> const& xAnimateNode =getXAnimateNode();
    rtl::OUString const attrName( xAnimateNode->getAttributeName() );
    AttributableShapeSharedPtr const pShape( getShape() );

    switch (AnimationFactory::classifyAttributeName( attrName )) {
    default:
    case AnimationFactory::CLASS_UNKNOWN_PROPERTY:
        ENSURE_OR_THROW(
            false,
            "Unexpected attribute class (unknown or empty attribute name)" );
        break;

    case AnimationFactory::CLASS_NUMBER_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            xAnimateNode );

    case AnimationFactory::CLASS_ENUM_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createEnumPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            xAnimateNode );

    case AnimationFactory::CLASS_COLOR_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createColorPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            xAnimateNode );

    case AnimationFactory::CLASS_STRING_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createStringPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            xAnimateNode );

    case AnimationFactory::CLASS_BOOL_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createBoolPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize() ),
            xAnimateNode );
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace slideshow

