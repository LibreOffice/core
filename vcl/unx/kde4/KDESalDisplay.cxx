/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "KDESalDisplay.hxx"

#include "KDEXLib.hxx"
#include "VCLKDEApplication.hxx"

#include <assert.h>
#include <unx/saldata.hxx>

#include <qthread.h>

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
    
    static_cast<KDEXLib*>(GetXLib())->doStartup();
    
    doDestruct();
    selfptr = NULL;
    
    pDisp_ = NULL;
}

void SalKDEDisplay::Yield()
{
    
    
    
    static bool break_cyclic_yield_recursion = false;
    bool is_qt_gui_thread = ( qApp->thread() == QThread::currentThread() );

    if( DispatchInternalEvent() || break_cyclic_yield_recursion )
        return;

    if( is_qt_gui_thread )
        break_cyclic_yield_recursion = true;

    DBG_ASSERT( static_cast<SalYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())->GetThreadId() ==
                osl::Thread::getCurrentIdentifier(),
                "will crash soon since solar mutex not locked in SalKDEDisplay::Yield" );

    static_cast<KDEXLib*>(GetXLib())->Yield( true, false );

    if( is_qt_gui_thread )
        break_cyclic_yield_recursion = false;
}









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
