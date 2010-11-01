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

#ifndef SD_IDLE_DETECTION_HXX
#define SD_IDLE_DETECTION_HXX

#include <sal/types.h>

class Window;

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
    static sal_Int32 GetIdleState (const ::Window* pWindow = NULL);

private:
    /** Check whether there are input events pending.
    */
    static sal_Int32 CheckInputPending (void);

    /** Check whether a slide show is running full screen or in a window.
    */
    static sal_Int32 CheckSlideShowRunning (void);

    static sal_Int32 CheckWindowPainting (const ::Window& rWindow);
};

} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
