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

#include <tools/rc.h>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include "window.h"

static OString ImplAutoHelpID( ResMgr* pResMgr )
{
    OString aRet;

    if( pResMgr && Application::IsAutoHelpIdEnabled() )
        aRet = pResMgr->GetAutoHelpId();

    return aRet;
}

namespace vcl {

WinBits Window::ImplInitRes( const ResId& rResId )
{
    GetRes( rResId );

    char* pRes = static_cast<char*>(GetClassRes());
    pRes += 8;
    sal_uInt32 nStyle = (sal_uInt32)GetLongRes( (void*)pRes );
    rResId.SetWinBits( nStyle );
    return nStyle;
}

WindowResHeader Window::ImplLoadResHeader( const ResId& rResId )
{
    WindowResHeader aHeader;

    aHeader.nObjMask = ReadLongRes();

    // we need to calculate auto helpids before the resource gets closed
    // if the resource  only contains flags, it will be closed before we try to read a help id
    // so we always create an auto help id that might be overwritten later
    // HelpId
    aHeader.aHelpId = ImplAutoHelpID( rResId.GetResMgr() );

    // ResourceStyle
    aHeader.nRSStyle = ReadLongRes();
    // WinBits
    ReadLongRes();

    if( aHeader.nObjMask & WINDOW_HELPID )
        aHeader.aHelpId = ReadByteStringRes();

    return aHeader;
}

void Window::ImplLoadRes( const ResId& rResId )
{
    WindowResHeader aHeader = ImplLoadResHeader( rResId );

    SetHelpId( aHeader.aHelpId );

    sal_uLong nObjMask = aHeader.nObjMask;

    bool  bPos  = false;
    bool  bSize = false;
    Point aPos;
    Size  aSize;

    if ( nObjMask & (WINDOW_XYMAPMODE | WINDOW_X | WINDOW_Y) )
    {
        // use size as per resource
        MapUnit ePosMap = MAP_PIXEL;

        bPos = true;

        if ( nObjMask & WINDOW_XYMAPMODE )
            ePosMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_X )
            aPos.X() = ImplLogicUnitToPixelX( ReadLongRes(), ePosMap );
        if ( nObjMask & WINDOW_Y )
            aPos.Y() = ImplLogicUnitToPixelY( ReadLongRes(), ePosMap );
    }

    if ( nObjMask & (WINDOW_WHMAPMODE | WINDOW_WIDTH | WINDOW_HEIGHT) )
    {
        // use size as per resource
        MapUnit eSizeMap = MAP_PIXEL;

        bSize = true;

        if ( nObjMask & WINDOW_WHMAPMODE )
            eSizeMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_WIDTH )
            aSize.Width() = ImplLogicUnitToPixelX( ReadLongRes(), eSizeMap );
        if ( nObjMask & WINDOW_HEIGHT )
            aSize.Height() = ImplLogicUnitToPixelY( ReadLongRes(), eSizeMap );
    }

    sal_uLong nRSStyle = aHeader.nRSStyle;

    // looks bad due to optimization
    if ( nRSStyle & RSWND_CLIENTSIZE )
    {
        if ( bPos )
            SetPosPixel( aPos );
        if ( bSize )
            SetOutputSizePixel( aSize );
    }
    else if ( bPos && bSize )
        SetPosSizePixel( aPos, aSize );
    else if ( bPos )
        SetPosPixel( aPos );
    else if ( bSize )
        SetSizePixel( aSize );

    if ( nRSStyle & RSWND_DISABLED )
        Enable( false );

    if ( nObjMask & WINDOW_TEXT )
        SetText( ReadStringRes() );
    if ( nObjMask & WINDOW_HELPTEXT )
    {
        SetHelpText( ReadStringRes() );
        mpWindowImpl->mbHelpTextDynamic = true;
    }
    if ( nObjMask & WINDOW_QUICKTEXT )
        SetQuickHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_EXTRALONG )
    {
        sal_uIntPtr nRes = ReadLongRes();
        SetData( reinterpret_cast<void*>(nRes) );
    }
    if ( nObjMask & WINDOW_UNIQUEID )
        SetUniqueId( ReadByteStringRes() );

    if ( nObjMask & WINDOW_BORDER_STYLE )
    {
        sal_uInt16 nBorderStyle = (sal_uInt16)ReadLongRes();
        SetBorderStyle( static_cast<WindowBorderStyle>(nBorderStyle) );
    }
}

}

