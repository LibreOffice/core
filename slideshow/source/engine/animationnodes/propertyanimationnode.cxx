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
    OUString const attrName( xAnimateNode->getAttributeName() );
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
                getSlideSize(), 0 ),
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
                getSlideSize(), 0 ),
            xAnimateNode );

    case AnimationFactory::CLASS_BOOL_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createBoolPropertyAnimation(
                attrName,
                pShape,
                getContext().mpSubsettableShapeManager,
                getSlideSize(), 0 ),
            xAnimateNode );
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
