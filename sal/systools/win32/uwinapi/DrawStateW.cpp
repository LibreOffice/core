/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawStateW.cpp,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:16:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
