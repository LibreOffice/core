#ifndef _register_h
#define _register_h

#include <sal/types.h>

typedef void (* FktPtr)(void);
// register the given void* as a function pointer, true, if successful
extern "C" bool SAL_CALL registerFunc(FktPtr aFunc, const char* aFuncName);

typedef bool (* FktRegFuncPtr)(FktPtr aFunc, const char* aFuncName);
extern "C" void SAL_CALL registerAllTestFunction(FktRegFuncPtr aFunc);

typedef void (* FktRegAllPtr)(FktRegFuncPtr aFunc);
#endif
