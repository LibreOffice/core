#include <stdio.h>
#include <sal/types.h>
#include "registerfunc.h"

extern "C" void SAL_CALL firstfunc()
{
   printf("first func called.\n");
}
extern "C" void SAL_CALL secondfunc()
{
   printf("second func called.\n");
}

extern "C" void SAL_CALL registerAllTestFunction(FktRegFuncPtr _pFunc)
{
    if (_pFunc)
    {
        (_pFunc)(&firstfunc, "firstfunc");
        (_pFunc)(&secondfunc, "secondfunc");
    }
}
