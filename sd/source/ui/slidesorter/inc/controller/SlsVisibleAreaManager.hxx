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

#ifndef SD_SLIDESORTER_VISIBLE_AREA_MANAGER_HXX
#define SD_SLIDESORTER_VISIBLE_AREA_MANAGER_HXX

#include "controller/SlsAnimator.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace sd { namespace slidesorter { namespace controller {


/** Manage requests for scrolling page objects into view.
*/
class VisibleAreaManager
    : public ::boost::noncopyable
{
public:
    VisibleAreaManager (SlideSorter& rSlideSorter);
    ~VisibleAreaManager (void);

    void ActivateCurrentSlideTracking (void);
    void DeactivateCurrentSlideTracking (void);
    bool IsCurrentSlideTrackingActive (void) const;

    /** Request the current slide to be moved into the visible area.
        This request is only obeyed when the current slide tracking is
        active.
        @see ActivateCurrentSlideTracking() and DeactivateCurrentSlideTracking()
    */
    void RequestCurrentSlideVisible (void);

    /** Request to make the specified page object visible.
    */
    void RequestVisible (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bForce = false);

    /** Temporarily disable the update of the visible area.
    */
    class TemporaryDisabler
    {
    public:
        TemporaryDisabler (SlideSorter& rSlideSorter);
        ~TemporaryDisabler (void);
    private:
        VisibleAreaManager& mrVisibleAreaManager;
    };

private:
    SlideSorter& mrSlideSorter;

    /** List of rectangle that someone wants to be moved into the visible
        area.
        Cleared on every call to ForgetVisibleRequests() and MakeVisible().
    */
    ::std::vector<Rectangle> maVisibleRequests;

    /** Animation id for a scroll animation that sets the top
        and left of the visible area to maRequestedVisibleTopLeft.
    */
    Animator::AnimationId mnScrollAnimationId;
    Point maRequestedVisibleTopLeft;
    Animator::AnimationMode meRequestedAnimationMode;
    bool mbIsCurrentSlideTrackingActive;
    int  mnDisableCount;

    void MakeVisible (void);
    ::boost::optional<Point> GetRequestedTopLeft (void) const;
};


} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
