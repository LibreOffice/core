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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include "animationfactory.hxx"
#include "setactivity.hxx"
#include "animationsetnode.hxx"
#include "nodetools.hxx"
#include "tools.hxx"
#include "delayevent.hxx"

#include <boost/bind.hpp>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

void AnimationSetNode::implScheduleDeactivationEvent()
{
    scheduleDeactivationEvent();
}

AnimationActivitySharedPtr AnimationSetNode::createActivity() const
{
    ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );
    uno::Reference<animations::XAnimate> const xAnimateNode = getXAnimateNode();
    rtl::OUString const attrName( xAnimateNode->getAttributeName() );
    AttributableShapeSharedPtr const pShape( getShape() );

    // make deactivation a two-step procedure. Normally, we
    // could solely rely on
    // BaseNode::scheduleDeactivationEvent() to deactivate()
    // us. Unfortunately, that method on the one hand ignores
    // indefinite timing, on the other hand generates
    // zero-timeout delays, which might get fired _before_ our
    // set activity has taken place. Therefore, we enforce
    // sequentiality by letting only the set activity schedule
    // the deactivation event (and AnimationBaseNode
    // takes care for the fact when mpActivity should be zero).

    // AnimationBaseNode::fillCommonParameters() has set up
    // immediate deactivation as default when activity ends, but
    if (! isIndefiniteTiming( xAnimateNode->getDuration() )) {
        boost::shared_ptr<AnimationSetNode> const pSelf(
            boost::dynamic_pointer_cast<AnimationSetNode>(getSelf()) );
        ENSURE_OR_THROW(
            pSelf, "cannot cast getSelf() to my type!" );
        aParms.mpEndEvent = makeEvent(
            boost::bind( &AnimationSetNode::implScheduleDeactivationEvent,
                         pSelf ),
            "AnimationSetNode::implScheduleDeactivationEvent");
    }

    switch (AnimationFactory::classifyAttributeName( attrName )) {
    default:
    case AnimationFactory::CLASS_UNKNOWN_PROPERTY:
        ENSURE_OR_THROW(
            false, "AnimationSetNode::createSetActivity(): "
            "Unexpected attribute class" );
        break;

    case AnimationFactory::CLASS_NUMBER_PROPERTY:
    {
        NumberAnimation::ValueType aValue;

        ENSURE_OR_THROW(
            extractValue( aValue,
                          xAnimateNode->getTo(),
                          pShape,
                          getSlideSize() ),
            "AnimationSetNode::createSetActivity(): "
            "Could not import numeric to value" );

        return makeSetActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }

    case AnimationFactory::CLASS_ENUM_PROPERTY:
    {
        EnumAnimation::ValueType aValue;

        ENSURE_OR_THROW(
            extractValue( aValue,
                          xAnimateNode->getTo(),
                          pShape,
                          getSlideSize() ),
            "AnimationSetNode::createSetActivity(): "
            "Could not import enum to value" );

        return makeSetActivity(
            aParms,
            AnimationFactory::createEnumPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }

    case AnimationFactory::CLASS_COLOR_PROPERTY:
    {
        ColorAnimation::ValueType aValue;

        ENSURE_OR_THROW(
            extractValue( aValue,
                          xAnimateNode->getTo(),
                          pShape,
                          getSlideSize() ),
            "AnimationSetNode::createSetActivity(): "
            "Could not import color to value" );

        return makeSetActivity(
            aParms,
            AnimationFactory::createColorPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }

    case AnimationFactory::CLASS_STRING_PROPERTY:
    {
        StringAnimation::ValueType aValue;

        ENSURE_OR_THROW(
            extractValue( aValue,
                          xAnimateNode->getTo(),
                          pShape,
                          getSlideSize() ),
            "AnimationSetNode::createSetActivity(): "
            "Could not import string to value" );

        return makeSetActivity(
            aParms,
            AnimationFactory::createStringPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }

    case AnimationFactory::CLASS_BOOL_PROPERTY:
    {
        BoolAnimation::ValueType aValue;

        ENSURE_OR_THROW(
            extractValue( aValue,
                          xAnimateNode->getTo(),
                          pShape,
                          getSlideSize() ),
            "AnimationSetNode::createSetActivity(): "
            "Could not import bool to value" );

        return makeSetActivity(
            aParms,
            AnimationFactory::createBoolPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(),
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace slideshow

