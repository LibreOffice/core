#define UNICODE
#include "macros.h"

EXTERN_C DWORD WINAPI GetUserDomainW_NT( LPWSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_NT.cpp"


EXTERN_C DWORD WINAPI GetUserDomainW_WINDOWS( LPWSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_WINDOWS.cpp"

EXTERN_C void WINAPI ResolveThunk_GetUserDomainW( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetUserDomainW_WINDOWS;
    else
        *lppfn = (FARPROC)GetUserDomainW_NT;
}

DEFINE_CUSTOM_THUNK( kernel32, GetUserDomainW, DWORD, WINAPI, GetUserDomainW, ( LPWSTR lpBuffer, DWORD cchBuffer ) );

