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

#ifndef INCLUDED_SLIDESHOW_ACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_ACTIVITYBASE_HXX

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
    ActivityBase( const ActivityParameters& rParms );

    /// From Disposable interface
    virtual void dispose();

protected:
    /** From Activity interface

        Derived classes should override, call this first
        and then perform their work.
    */
    virtual bool perform();
    virtual double calcTimeLag() const;
    virtual bool isActive() const;

private:
    virtual void dequeued();

    // From AnimationActivity interface
    virtual void setTargets(
        const AnimatableShapeSharedPtr&        rShape,
        const ShapeAttributeLayerSharedPtr&    rAttrLayer );

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

    bool isRepeatCountValid() const { return maRepeats; }
    double getRepeatCount() const { return *maRepeats; }
    bool isAutoReverse() const { return mbAutoReverse; }

private:
    /// Activity:
    virtual void end();
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

#endif /* INCLUDED_SLIDESHOW_ACTIVITYBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
