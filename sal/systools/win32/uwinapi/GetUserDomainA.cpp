#include "macros.h"

EXTERN_C DWORD WINAPI GetUserDomainA_NT( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_NT.cpp"

EXTERN_C DWORD WINAPI GetUserDomainA_WINDOWS( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_WINDOWS.cpp"

EXTERN_C void WINAPI ResolveThunk_GetUserDomainA( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetUserDomainA_WINDOWS;
    else
        *lppfn = (FARPROC)GetUserDomainA_NT;
}

DEFINE_CUSTOM_THUNK( kernel32, GetUserDomainA, DWORD, WINAPI, GetUserDomainA, ( LPSTR lpBuffer, DWORD nSize ) );

