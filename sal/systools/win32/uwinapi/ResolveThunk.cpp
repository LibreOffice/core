#include "macros.h"


EXTERN_C void WINAPI ResolveThunk_WINDOWS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    FARPROC lpfnResult = (LONG)GetVersion() < 0 ? lpfnEmulate : GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );

    if ( !lpfnResult )
        lpfnResult = lpfnEmulate;

    if ( !lpfnResult )
        lpfnResult = lpfnFailure;

    *lppfn = lpfnResult;
}


EXTERN_C void WINAPI ResolveThunk_TRYLOAD( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );

    if ( !lpfnResult )
        lpfnResult = lpfnEmulate;

    if ( !lpfnResult )
        lpfnResult = lpfnFailure;

    *lppfn = lpfnResult;
}


EXTERN_C void WINAPI ResolveThunk_ALLWAYS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    *lppfn = lpfnEmulate ? lpfnEmulate : lpfnFailure;
}


