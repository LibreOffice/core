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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
