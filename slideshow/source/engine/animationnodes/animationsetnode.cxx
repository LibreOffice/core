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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
