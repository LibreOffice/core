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

#pragma once

#include <model/SlsSharedPageDescriptor.hxx>
#include <optional>
#include <tools/gen.hxx>
#include <vector>

namespace sd::slidesorter
{
class SlideSorter;
}

namespace sd::slidesorter::controller
{
/** Manage requests for scrolling page objects into view.
*/
class VisibleAreaManager
{
public:
    explicit VisibleAreaManager(SlideSorter& rSlideSorter);
    ~VisibleAreaManager();
    VisibleAreaManager(const VisibleAreaManager&) = delete;
    VisibleAreaManager& operator=(const VisibleAreaManager&) = delete;

    void ActivateCurrentSlideTracking();
    void DeactivateCurrentSlideTracking();
    bool IsCurrentSlideTrackingActive() const { return mbIsCurrentSlideTrackingActive; }

    /** Request the current slide to be moved into the visible area.
        This request is only obeyed when the current slide tracking is
        active.
        @see ActivateCurrentSlideTracking() and DeactivateCurrentSlideTracking()
    */
    void RequestCurrentSlideVisible();

    /** Request to make the specified page object visible.
    */
    void RequestVisible(const model::SharedPageDescriptor& rpDescriptor, const bool bForce = false);

    /** Temporarily disable the update of the visible area.
    */
    class TemporaryDisabler
    {
    public:
        explicit TemporaryDisabler(SlideSorter const& rSlideSorter);
        ~TemporaryDisabler();

    private:
        VisibleAreaManager& mrVisibleAreaManager;
    };

private:
    SlideSorter& mrSlideSorter;

    /** List of rectangle that someone wants to be moved into the visible
        area.
        Cleared on every call to ForgetVisibleRequests() and MakeVisible().
    */
    ::std::vector<::tools::Rectangle> maVisibleRequests;

    Point maRequestedVisibleTopLeft;
    bool mbIsCurrentSlideTrackingActive;
    int mnDisableCount;

    void MakeVisible();
    ::std::optional<Point> GetRequestedTopLeft() const;
};

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
