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

#include <salgdiimpl.hxx>

#include <vcl/sysdata.hxx>

#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/x11/xrender_peer.hxx>
#include <salframe.hxx>

extern "C"
{
    static Bool GraphicsExposePredicate( Display*, XEvent* pEvent, const XPointer pFrameWindow )
    {
        Bool bRet = False;
        if( (pEvent->type == GraphicsExpose || pEvent->type == NoExpose) &&
            pEvent->xnoexpose.drawable == reinterpret_cast<Drawable>(pFrameWindow) )
        {
            bRet = True;
        }
        return bRet;
    }
}

void X11SalGraphics::YieldGraphicsExpose()
{
    // get frame if necessary
    SalFrame* pFrame    = m_pFrame;
    Display* pDisplay   = GetXDisplay();
    ::Window aWindow = GetDrawable();
    if( ! pFrame )
    {
        for (auto pSalFrame : vcl_sal::getSalDisplay(GetGenericUnixSalData())->getFrames() )
        {
            const SystemEnvData* pEnvData = pSalFrame->GetSystemData();
            if( Drawable(pEnvData->GetWindowHandle(pSalFrame)) == aWindow )
            {
                pFrame = pSalFrame;
                break;
            }
        }
        if( ! pFrame )
            return;
    }

    XEvent aEvent;
    while( XCheckTypedWindowEvent( pDisplay, aWindow, Expose, &aEvent ) )
    {
        SalPaintEvent aPEvt( aEvent.xexpose.x, aEvent.xexpose.y, aEvent.xexpose.width+1, aEvent.xexpose.height+1 );
        pFrame->CallCallback( SalEvent::Paint, &aPEvt );
    }

    do
    {
        if( ! GetDisplay()->XIfEventWithTimeout( &aEvent, reinterpret_cast<XPointer>(aWindow), GraphicsExposePredicate ) )
            // this should not happen at all; still sometimes it happens
            break;

        if( aEvent.type == NoExpose )
            break;

        if( pFrame )
        {
            SalPaintEvent aPEvt( aEvent.xgraphicsexpose.x, aEvent.xgraphicsexpose.y, aEvent.xgraphicsexpose.width+1, aEvent.xgraphicsexpose.height+1 );
            pFrame->CallCallback( SalEvent::Paint, &aPEvt );
        }
    } while( aEvent.xgraphicsexpose.count != 0 );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
