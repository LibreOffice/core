#include "macros.h"
#include <vfw.h>

IMPLEMENT_THUNK( kernel32, WINDOWS, HWND, VFWAPIV, MCIWndCreateW,
(
  HWND hwndParent,
  HINSTANCE hInstance,
  DWORD dwStyle,
  LPCWSTR lpFileW
))
{
    AUTO_WSTR2STR( lpFile );

    return MCIWndCreateA( hwndParent, hInstance, dwStyle, lpFileA );
}