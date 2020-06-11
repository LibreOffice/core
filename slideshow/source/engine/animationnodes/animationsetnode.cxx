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


#include <animationfactory.hxx>
#include "setactivity.hxx"
#include "animationsetnode.hxx"
#include "nodetools.hxx"
#include <tools.hxx>
#include <delayevent.hxx>

using namespace com::sun::star;

namespace slideshow::internal {

AnimationActivitySharedPtr AnimationSetNode::createActivity() const
{
    ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );
    uno::Reference<animations::XAnimate> const xAnimateNode = getXAnimateNode();
    OUString const attrName( xAnimateNode->getAttributeName() );
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
        std::shared_ptr<AnimationSetNode> const pSelf(
            std::dynamic_pointer_cast<AnimationSetNode>(getSelf()) );
        ENSURE_OR_THROW(
            pSelf, "cannot cast getSelf() to my type!" );
        aParms.mpEndEvent = makeEvent(
            [pSelf] () { pSelf->scheduleDeactivationEvent(); },
            "AnimationSetNode::scheduleDeactivationEvent");
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
                getContext().mpBox2DWorld,
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
                getContext().mpBox2DWorld,
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
                getContext().mpBox2DWorld,
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
                getContext().mpBox2DWorld,
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
                getContext().mpBox2DWorld,
                AnimationFactory::FLAG_NO_SPRITE ),
            aValue );
    }
    }

    return AnimationActivitySharedPtr();
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
