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
#ifndef INCLUDED_SLIDESHOW_SETACTIVITY_HXX
#define INCLUDED_SLIDESHOW_SETACTIVITY_HXX

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include "animationactivity.hxx"
#include "animation.hxx"
#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"
#include "activitiesfactory.hxx"

namespace slideshow {
namespace internal {

/** Templated setter for animation values

    This template class implements the AnimationActivity
    interface, but only the perform() and
    setAttributeLayer() methods are functional. To be used for set animations.

    @see AnimationSetNode.
*/
template <class AnimationT>
class SetActivity : public AnimationActivity
{
public:
    typedef ::boost::shared_ptr< AnimationT >   AnimationSharedPtrT;
    typedef typename AnimationT::ValueType      ValueT;

    SetActivity( const ActivitiesFactory::CommonParameters& rParms,
                 const AnimationSharedPtrT&                 rAnimation,
                 const ValueT&                              rToValue )
        : mpAnimation( rAnimation ),
          mpShape(),
          mpAttributeLayer(),
          mpEndEvent( rParms.mpEndEvent ),
          mrEventQueue( rParms.mrEventQueue ),
          maToValue( rToValue ),
          mbIsActive(true)
    {
        ENSURE_OR_THROW( mpAnimation, "Invalid animation" );
    }

    virtual void dispose()
    {
        mbIsActive = false;
        mpAnimation.reset();
        mpShape.reset();
        mpAttributeLayer.reset();
        // discharge end event:
        if (mpEndEvent && mpEndEvent->isCharged())
            mpEndEvent->dispose();
        mpEndEvent.reset();
    }

    virtual double calcTimeLag() const
    {
        return 0.0;
    }

    virtual bool perform()
    {
        if (! isActive())
            return false;
        // we're going inactive immediately:
        mbIsActive = false;

        if (mpAnimation && mpAttributeLayer && mpShape) {
            mpAnimation->start( mpShape, mpAttributeLayer );
            (*mpAnimation)(maToValue);
            mpAnimation->end();
        }
        // fire end event, if any
        if (mpEndEvent)
            mrEventQueue.addEvent( mpEndEvent );

        return false; // don't reinsert
    }

    virtual bool isActive() const
    {
        return mbIsActive;
    }

    virtual void dequeued()
    {
    }

    virtual void end()
    {
        perform();
    }

    virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                             const ShapeAttributeLayerSharedPtr&    rAttrLayer )
    {
        ENSURE_OR_THROW( rShape, "Invalid shape" );
        ENSURE_OR_THROW( rAttrLayer, "Invalid attribute layer" );

        mpShape = rShape;
        mpAttributeLayer = rAttrLayer;
    }

private:
    AnimationSharedPtrT             mpAnimation;
    AnimatableShapeSharedPtr        mpShape;
    ShapeAttributeLayerSharedPtr    mpAttributeLayer;
    EventSharedPtr                  mpEndEvent;
    EventQueue&                     mrEventQueue;
    ValueT                          maToValue;
    bool                            mbIsActive;
};

template <class AnimationT> AnimationActivitySharedPtr makeSetActivity(
    const ActivitiesFactory::CommonParameters& rParms,
    const ::boost::shared_ptr< AnimationT >&   rAnimation,
    const typename AnimationT::ValueType&      rToValue )
{
    return AnimationActivitySharedPtr(
        new SetActivity<AnimationT>(rParms,rAnimation,rToValue) );
}

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_SETACTIVITY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
