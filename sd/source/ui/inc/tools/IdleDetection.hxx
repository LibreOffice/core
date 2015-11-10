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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TOOLS_IDLEDETECTION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TOOLS_IDLEDETECTION_HXX

#include <sal/types.h>

namespace vcl { class Window; }

namespace sd { namespace tools {

/** Detect whether the system is idle and some time consuming operation may
    be carried out.  This class ditinguishes between different states of
    idle-ness.
*/
class IdleDetection
{
public:
    /** When GetIdleState() returns this value, then the system is idle.
    */
    static const sal_Int32 IDET_IDLE = 0x0000;

    /** There are system event pending.
    */
    static const sal_Int32 IDET_SYSTEM_EVENT_PENDING = 0x0001;

    /** A full screen slide show is running and is active.  In contrast
        there may be a full screen show be running in an inactive window,
        i.e. in the background.
    */
    static const sal_Int32 IDET_FULL_SCREEN_SHOW_ACTIVE = 0x0002;

    /** A slide show is running in a window.
    */
    static const sal_Int32 IDET_WINDOW_SHOW_ACTIVE = 0x0004;

    /** A window is being painted.
    */
    static const sal_Int32 IDET_WINDOW_PAINTING = 0x0008;

    /** Determine whether the system is idle.
        @param pWindow
            When a valid Window pointer is given then it is checked
            whether the window is currently being painting.
        @return
            This method either returns IDET_IDLE or a combination of
            IdleStates values or-ed together that describe what the system
            is currently doing so that the caller can decide what to do.
    */
    static sal_Int32 GetIdleState (const vcl::Window* pWindow = nullptr);

private:
    /** Check whether there are input events pending.
    */
    static sal_Int32 CheckInputPending();

    /** Check whether a slide show is running full screen or in a window.
    */
    static sal_Int32 CheckSlideShowRunning();

    static sal_Int32 CheckWindowPainting (const vcl::Window& rWindow);
};

} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
