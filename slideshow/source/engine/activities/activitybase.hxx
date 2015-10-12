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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_ACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_ACTIVITYBASE_HXX

#include "animationactivity.hxx"
#include "activityparameters.hxx"
#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"

namespace slideshow {
namespace internal {

/** Base class for animation activities.

    This whole class hierarchy is only for code sharing
    between the various specializations (with or without
    key times, fully discrete, etc.).
*/
class ActivityBase : public AnimationActivity
{
public:
    explicit ActivityBase( const ActivityParameters& rParms );

    /// From Disposable interface
    virtual void dispose() override;

protected:
    /** From Activity interface

        Derived classes should override, call this first
        and then perform their work.
    */
    virtual bool perform() override;
    virtual double calcTimeLag() const override;
    virtual bool isActive() const override;

private:
    virtual void dequeued() override;

    // From AnimationActivity interface
    virtual void setTargets(
        const AnimatableShapeSharedPtr&        rShape,
        const ShapeAttributeLayerSharedPtr&    rAttrLayer ) override;

private:
    /** Hook for derived classes

        This method will be called from the first
        perform() invocation, to signal the start of the
        activity.
    */
    virtual void startAnimation() = 0;

    /** Hook for derived classes

        This method will be called after the last perform()
        invocation, and after the potential changes of that
        perform() call are committed to screen. That is, in
        endAnimation(), the animation objects (sprites,
        animation) can safely be destroyed, without causing
        visible artifacts on screen.
    */
    virtual void endAnimation() = 0;

protected:

    /** End this activity, in a regular way.

        This method is for derived classes needing to signal a
        regular activity end (i.e. because the regular
        duration is over)
    */
    void endActivity();

    /** Modify fractional time.

        This method modifies the fractional time (total
        duration mapped to the [0,1] range) to the
        effective simple time, but only according to
        acceleration/deceleration.
    */
    double calcAcceleratedTime( double nT ) const;

    bool isDisposed() const {
        return (!mbIsActive && !mpEndEvent && !mpShape &&
                !mpAttributeLayer);
    }

    EventQueue& getEventQueue() const { return mrEventQueue; }

    AnimatableShapeSharedPtr getShape() const { return mpShape; }

    ShapeAttributeLayerSharedPtr getShapeAttributeLayer() const
        { return mpAttributeLayer; }

    bool isRepeatCountValid() const { return bool(maRepeats); }
    double getRepeatCount() const { return *maRepeats; }
    bool isAutoReverse() const { return mbAutoReverse; }

private:
    /// Activity:
    virtual void end() override;
    virtual void performEnd() = 0;

private:
    EventSharedPtr                  mpEndEvent;
    EventQueue&                     mrEventQueue;
    AnimatableShapeSharedPtr        mpShape; // only to pass on to animation
    ShapeAttributeLayerSharedPtr    mpAttributeLayer; // only to pass on to anim

    ::boost::optional<double> const maRepeats;
    const double                    mnAccelerationFraction;
    const double                    mnDecelerationFraction;

    const bool                      mbAutoReverse;

    // true, if perform() has not yet been called:
    mutable bool                    mbFirstPerformCall;
    bool                            mbIsActive;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_ACTIVITYBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
