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
                int cchWideChar = wData ? wData : -1;
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

