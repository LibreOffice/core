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

#include "macros.h"

IMPLEMENT_THUNK( user32, WINDOWS, BOOL, WINAPI, DrawStateW,
(
    HDC             hdc,            // handle to device context
    HBRUSH          hbr,            // handle to brush
    DRAWSTATEPROC   lpOutputFunc,   // pointer to callback function
    LPARAM          lData,          // image information
    WPARAM          wData,          // more image information
    int             x,              // horizontal location of image
    int             y,              // vertical location of image
    int             cx,             // width of image
    int             cy,             // height of image
    UINT            fuFlags         // image type and state

))
{
    switch ( fuFlags & 0x000F )
    {
    case DST_TEXT:
    case DST_PREFIXTEXT:
        {
            LPSTR   lpTextA = NULL;

            if ( lData )
            {
                int cchWideChar = (int) (wData ? wData : -1);
                int cchNeeded = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)lData, cchWideChar, NULL, 0, NULL, NULL );

                lpTextA = (LPSTR)_alloca( cchNeeded * sizeof(CHAR) );

                if ( !lpTextA )
                {
                    SetLastError( ERROR_OUTOFMEMORY );
                    return FALSE;
                }

                WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)lData, cchWideChar, lpTextA, cchNeeded, NULL, NULL );

            }

            return DrawStateA( hdc, hbr, lpOutputFunc, (LPARAM)lpTextA, wData, x, y, cx, cy, fuFlags );
        }
    default:
        return DrawStateA( hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy, fuFlags );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
