/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationpathmotionnode.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:32:07 $
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
#include "animationpathmotionnode.hxx"
#include "animationfactory.hxx"

namespace presentation {
namespace internal {

void AnimationPathMotionNode::dispose()
{
    mxPathMotionNode.clear();
    AnimationBaseNode::dispose();
}

AnimationActivitySharedPtr AnimationPathMotionNode::createActivity() const
{
    rtl::OUString aString;
    ENSURE_AND_THROW( (mxPathMotionNode->getPath() >>= aString),
                      "no string-based SVG:d path found" );

    ActivitiesFactory::CommonParameters const aParms( fillCommonParameters() );
    return ActivitiesFactory::createSimpleActivity(
        aParms,
        AnimationFactory::createPathMotionAnimation(
            aString,
            getShape(),
            getContext().mpLayerManager ),
        true );
}

} // namespace internal
} // namespace presentation

