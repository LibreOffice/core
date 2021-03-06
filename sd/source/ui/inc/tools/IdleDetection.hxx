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

#include <o3tl/typed_flags_set.hxx>

namespace vcl { class Window; }

namespace sd::tools {
    enum class IdleState {
        /** When GetIdleState() returns this value, then the system is idle.
        */
        Idle = 0x0000,

        /** There are system event pending.
        */
        SystemEventPending = 0x0001,

        /** A full screen slide show is running and is active.  In contrast
            there may be a full screen show be running in an inactive window,
            i.e. in the background.
        */
        FullScreenShowActive = 0x0002,

        /** A slide show is running in a window.
        */
        WindowShowActive = 0x0004,

        /** A window is being painted.
        */
        WindowPainting = 0x0008,
    };
} // end of namespace ::sd::tools
namespace o3tl {
    template<> struct typed_flags<::sd::tools::IdleState> : is_typed_flags<::sd::tools::IdleState, 0x0f> {};
}

namespace sd::tools {

/** Detect whether the system is idle and some time consuming operation may
    be carried out.  This class distinguishes between different states of
    idle-ness.
*/
class IdleDetection
{
public:
    /** Determine whether the system is idle.
        @param pWindow
            When a valid Window pointer is given then it is checked
            whether the window is currently being painting.
        @return
            This method either returns IdleState::Idle or a combination of
            IdleStates values or-ed together that describe what the system
            is currently doing so that the caller can decide what to do.
    */
    static IdleState GetIdleState (const vcl::Window* pWindow);

private:
    /** Check whether there are input events pending.
    */
    static IdleState CheckInputPending();

    /** Check whether a slide show is running full screen or in a window.
    */
    static IdleState CheckSlideShowRunning();

    static IdleState CheckWindowPainting (const vcl::Window& rWindow);
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
