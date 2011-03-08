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

#include "KDESalDisplay.hxx"

#include "KDEXLib.hxx"
#include "VCLKDEApplication.hxx"

#include <assert.h>
#include <saldata.hxx>

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
