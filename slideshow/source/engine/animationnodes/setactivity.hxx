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
#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_SETACTIVITY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_SETACTIVITY_HXX

#include <tools/diagnose_ex.h>

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

    virtual void dispose() override
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

    virtual double calcTimeLag() const override
    {
        return 0.0;
    }

    virtual bool perform() override
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

    virtual bool isActive() const override
    {
        return mbIsActive;
    }

    virtual void dequeued() override
    {
    }

    virtual void end() override
    {
        perform();
    }

    virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                             const ShapeAttributeLayerSharedPtr&    rAttrLayer ) override
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

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_SETACTIVITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
