#pragma once

#ifdef _UWINAPI_
#   define _KERNEL32_
#   define _USER32_
#   define _SHELL32_
#endif

#ifndef _WINDOWS_
#   include <windows.h>
#endif

/** GetUserDomain

The GetUserDomain function retrieves the name of the NT domain the user is
logged in.

Parameters
    @param lpBuffer
        [out] Pointer to a buffer that receives a null-terminated string
        containing the domain name. The buffer size should be large enough to
        contain MAX_COMPUTERNAME_LENGTH + 1 characters.
    @param nBufferSize
        [in] Specifies the size, in TCHARs, of the buffer pointed to
        by the lpBuffer parameter.


Return Values
    @return
    If the function succeeds, the return value is the number of TCHARs stored
    into the buffer pointed to by lpBuffer, not including the terminating
    null character.

    If the domain name can't be retrieved, the return value is zero.

    If the buffer pointed to by lpBuffer is not large enough, the return value
    is the buffer size, in TCHARs, required to hold the value string and its
    terminating null character.

Remarks
    Windows 95/98/Me: If the user is not logged in onto a NT domain server
    the name of the workgroup is returned.

Requirements
  Windows NT/2000/XP: Included in Windows NT 4 and later.
  Windows 95/98/Me: Included in Windows 95 and later.
  Header: Declared in Uwinapi.h; include Uwinapi.h.
  Library: Use Uwinapi.lib.
  Unicode: Implemented as Unicode and ANSI versions on Windows 95/98/Me/NT/2000/XP.

See Also
@see
*/

EXTERN_C WINBASEAPI DWORD WINAPI GetUserDomainA( LPSTR lpBuffer, DWORD nBuffserSize );
EXTERN_C WINBASEAPI DWORD WINAPI GetUserDomainW( LPWSTR lpBuffer, DWORD nBuffserSize );

#ifdef UNICODE
#define GetUserDomain   GetUserDomainW
#else
#define GetUserDomain   GetUserDomainA
#endif

/* macro that calculates the count of elements of a static array */

#define elementsof(buf) (sizeof(buf) / sizeof((buf)[0]))

#ifdef __cplusplus

inline bool IsValidHandle(HANDLE handle)
{
    return  handle != INVALID_HANDLE_VALUE && handle != NULL;
}

#else   /* __cplusplus */

#define IsValidHandle(Handle)   ((DWORD)(Handle) + 1 > 1)

#endif  /* __cplusplus */

