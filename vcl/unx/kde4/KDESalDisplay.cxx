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

#include "KDESalDisplay.hxx"

#include "KDEXLib.hxx"
#include "VCLKDEApplication.hxx"

#include <assert.h>
#include <unx/saldata.hxx>

SalKDEDisplay* SalKDEDisplay::selfptr = NULL;

SalKDEDisplay::SalKDEDisplay( Display* pDisp )
    : SalX11Display( pDisp )
{
    assert( selfptr == NULL );
    selfptr = this;
    xim_protocol = XInternAtom( pDisp_, "_XIM_PROTOCOL", False );
}

SalKDEDisplay::~SalKDEDisplay()
{
    // in case never a frame opened
    static_cast<KDEXLib*>(GetXLib())->doStartup();
    // clean up own members
    doDestruct();
    selfptr = NULL;
    // prevent SalDisplay from closing KApplication's display
    pDisp_ = NULL;
}

void SalKDEDisplay::Yield()
{
    if( DispatchInternalEvent() )
        return;

    DBG_ASSERT( static_cast<SalYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())->GetThreadId() ==
                osl::Thread::getCurrentIdentifier(),
                "will crash soon since solar mutex not locked in SalKDEDisplay::Yield" );

    XEvent event;
    XNextEvent( pDisp_, &event );
    if( checkDirectInputEvent( &event ))
        return;
    qApp->x11ProcessEvent( &event );
}

// HACK: When using Qt event loop, input methods (japanese, etc.) will get broken because
// of XFilterEvent() getting called twice, once by Qt, once by LO (bnc#665112).
// This function is therefore called before any XEvent is passed to Qt event handling
// and if it is a keyboard event and no Qt widget is the active window (i.e. we are
// processing events for some LO window), then feed the event only to LO directly and skip Qt
// completely. Skipped events are KeyPress, KeyRelease and also _XIM_PROTOCOL client message
// (seems to be necessary too, hopefully there are not other internal XIM messages that
// would need this handling).
bool SalKDEDisplay::checkDirectInputEvent( XEvent* ev )
{
    if( ev->xany.type == XLIB_KeyPress || ev->xany.type == KeyRelease
        || ( ev->xany.type == ClientMessage && ev->xclient.message_type == xim_protocol ))
    {
        if( qApp->activeWindow() == NULL )
        {
            Dispatch(ev);
            return true;
        }
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
