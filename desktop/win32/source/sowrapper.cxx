#include <windows.h>
#include "main.h"

int APIENTRY WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    if ( (LONG)GetVersion() < 0 )
        return MainA();
    else
        return MainW();
}
