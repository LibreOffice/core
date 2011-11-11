/*************************************************************************

   Copyright 2011 Yuri Dario <mc6530@mclink.it>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 ************************************************************************/

#define INCL_DOS
#ifdef OS2
#include <svpm.h>
#else
#include <os2.h>
#endif
#include <dlfcn.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef TESTME
#include <stdio.h>
#endif

typedef APIRET _PMPRINTF(const char*, ...);
typedef _PMPRINTF* PMPRINTF;

static void*    hmodPmPrintf = NULL;
static PMPRINTF pfnPmPrintf = NULL;

int _Export debug_printf( const char* format, ...)
{
    va_list args;
    int     cnt;

    if (hmodPmPrintf == NULL) {
        // try dll loading
        hmodPmPrintf = dlopen( "PMPRINTF", 0);
        if (hmodPmPrintf == NULL)
            return -1;

        // search function
        pfnPmPrintf = dlsym(hmodPmPrintf, "PmPrintfVa");
        if (!pfnPmPrintf)
            return -1;

    }

    // function loaded, print data
    va_start(args, format);
    cnt = pfnPmPrintf(format, args);
    va_end(args);

    return cnt;
}


#ifdef TESTME
int main( void)
{
    printf( "Test PMPRINTF.DLL output, check PM window.\n");
    debug_printf( "Test PMPRINTF.DLL output, check PM window.");
    debug_printf( "Test PMPRINTF.DLL output: integer %d", 12345);
    debug_printf( "Test PMPRINTF.DLL output: float %f", 123.45);
    debug_printf( "Test PMPRINTF.DLL output: string '%s'", "Hello World");
    exit(0);
}
#endif // TESTME
