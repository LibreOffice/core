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

#include "VCLKDE5Application.hxx"
#include "KDE5SalDisplay.hxx"

#ifdef Bool
#undef Bool
#endif

#include "KDE5XLib.hxx"

#include <assert.h>

SalKDE5Display* SalKDE5Display::selfptr = nullptr;

SalKDE5Display::SalKDE5Display( Display* pDisp )
    : SalX11Display( pDisp )
{
    assert( selfptr == nullptr );
    selfptr = this;
    xim_protocol = XInternAtom( pDisp_, "_XIM_PROTOCOL", False );
}

SalKDE5Display::~SalKDE5Display()
{
    // in case never a frame opened
    static_cast<KDE5XLib*>(GetXLib())->doStartup();
    // clean up own members
    doDestruct();
    selfptr = nullptr;
    // prevent SalDisplay from closing KApplication's display
    pDisp_ = nullptr;
}

void SalKDE5Display::Yield()
{
    if( DispatchInternalEvent() )
        return;

    // Prevent blocking from Drag'n'Drop events, which may have already have processed the event
    if (XEventsQueued( pDisp_, QueuedAfterReading ) == 0)
        return;

    DBG_ASSERT( GetSalData()->m_pInstance->GetYieldMutex()->IsCurrentThread() ==
                osl::Thread::getCurrentIdentifier(),
                "will crash soon since solar mutex not locked in SalKDE5Display::Yield" );

    /*XEvent event;
    XNextEvent( pDisp_, &event );
    if( checkDirectInputEvent( &event ))
        return;
    qApp->x11ProcessEvent( &event );*/
}

// HACK: When using Qt event loop, input methods (japanese, etc.) will get broken because
// of XFilterEvent() getting called twice, once by Qt, once by LO (bnc#665112).
// This function is therefore called before any XEvent is passed to Qt event handling
// and if it is a keyboard event and no Qt widget is the active window (i.e. we are
// processing events for some LO window), then feed the event only to LO directly and skip Qt
// completely. Skipped events are KeyPress, KeyRelease and also _XIM_PROTOCOL client message
// (seems to be necessary too, hopefully there are not other internal XIM messages that
// would need this handling).
bool SalKDE5Display::checkDirectInputEvent( xcb_generic_event_t* ev )
{
    switch (ev->response_type & ~0x80)
    {
        case XCB_CLIENT_MESSAGE:
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
            if( QApplication::activeWindow() == nullptr )
            {
//                Dispatch(ev);
                return true;
            }
            break;
    }
    return false;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
