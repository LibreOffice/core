/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyanimationnode.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:36:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// must be first
#include "canvas/debug.hxx"
#include "canvas/verbosetrace.hxx"
#include "propertyanimationnode.hxx"
#include "animationfactory.hxx"

using namespace com::sun::star;

namespace presentation {
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
        ENSURE_AND_THROW(
            false,
            "Unexpected attribute class (unknown or empty attribute name)" );
        break;

    case AnimationFactory::CLASS_NUMBER_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createNumberPropertyAnimation(
                attrName,
                pShape,
                getContext().mpLayerManager ),
            xAnimateNode );

    case AnimationFactory::CLASS_ENUM_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createEnumPropertyAnimation(
                attrName,
                pShape,
                getContext().mpLayerManager ),
            xAnimateNode );

    case AnimationFactory::CLASS_COLOR_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createColorPropertyAnimation(
                attrName,
                pShape,
                getContext().mpLayerManager ),
            xAnimateNode );

    case AnimationFactory::CLASS_STRING_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createStringPropertyAnimation(
                attrName,
                pShape,
                getContext().mpLayerManager ),
            xAnimateNode );

    case AnimationFactory::CLASS_BOOL_PROPERTY:
        return ActivitiesFactory::createAnimateActivity(
            aParms,
            AnimationFactory::createBoolPropertyAnimation(
                attrName,
                pShape,
                getContext().mpLayerManager ),
            xAnimateNode );
    }

    return AnimationActivitySharedPtr();
}

} // namespace internal
} // namespace presentation

