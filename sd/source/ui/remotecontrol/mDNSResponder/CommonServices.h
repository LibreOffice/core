/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*!    @header        CommonServices

    Common Services for Mac OS X, Linux, Palm, VxWorks, Windows, and Windows CE.
 */

#ifndef __COMMON_SERVICES__
#define __COMMON_SERVICES__

#ifdef  __cplusplus
extern "C" {
#endif

#if 0
#pragma mark == Target ==
#endif


//     Target
// Macintosh
#if ( !defined( TARGET_OS_MAC ) )
    #if ( ( macintosh || __MACH__ ) && !KERNEL )
    // ConditionalMacros.h in CoreServices will define this TARGET_* flag.
    #else
        #define TARGET_OS_MAC           0
    #endif
#endif

#if ( !defined( TARGET_API_MAC_OSX_KERNEL ) )
    #if ( __MACH__ && KERNEL )
        #define TARGET_API_MAC_OSX_KERNEL       1
    #else
        #define TARGET_API_MAC_OSX_KERNEL       0
    #endif
#endif

// FreeBSD
#if ( !defined( TARGET_OS_FREEBSD ) )
    #if ( defined( __FreeBSD__ ) )
        #define TARGET_OS_FREEBSD       1
    #else
        #define TARGET_OS_FREEBSD       0
    #endif
#endif

// Linux
#if ( !defined( TARGET_OS_LINUX ) )
    #if ( defined( __linux__ ) )
        #define TARGET_OS_LINUX         1
    #else
        #define TARGET_OS_LINUX         0
    #endif
#endif

// Solaris
#if ( !defined( TARGET_OS_SOLARIS ) )
    #if ( defined(solaris) || (defined(__SVR4) && defined(sun)) )
        #define TARGET_OS_SOLARIS       1
    #else
        #define TARGET_OS_SOLARIS       0
    #endif
#endif

// Palm
#if ( !defined( TARGET_OS_PALM ) )
    #if ( defined( __PALMOS_TRAPS__ ) || defined( __PALMOS_ARMLET__ ) )
        #define TARGET_OS_PALM          1
    #else
        #define TARGET_OS_PALM          0
    #endif
#endif

// VxWorks
#if ( !defined( TARGET_OS_VXWORKS ) )

// No predefined macro for VxWorks so just assume VxWorks if nothing else is set.

    #if ( !macintosh && !__MACH__  && !defined( __FreeBSD__ ) && !defined( __linux__ ) && !defined ( __SVR4 ) && !defined ( __sun ) && !defined( __PALMOS_TRAPS__ ) && !defined( __PALMOS_ARMLET__ ) && !defined( _WIN32 ) )
        #define TARGET_OS_VXWORKS       1
    #else
        #define TARGET_OS_VXWORKS       0
    #endif
#endif

// Windows
#if ( !defined( TARGET_OS_WIN32 ) )
    #if ( macintosh || __MACH__ )
// ConditionalMacros.h in CoreServices will define this TARGET_* flag.
    #else
        #if ( defined( _WIN32 ) )
            #define TARGET_OS_WIN32     1
        #else
            #define TARGET_OS_WIN32     0
        #endif
    #endif
#endif

// Windows CE
#if ( !defined( TARGET_OS_WINDOWS_CE ) )
    #if ( defined( _WIN32_WCE ) )
        #define TARGET_OS_WINDOWS_CE    1
    #else
        #define TARGET_OS_WINDOWS_CE    0
    #endif
#endif

#if 0
#pragma mark == Includes ==
#endif


//     Includes
#if ( !KERNEL )
    #if defined(WIN32) && !defined(_WSPIAPI_COUNTOF)
        #define _WSPIAPI_COUNTOF(_Array) (sizeof(_Array) / sizeof(_Array[0]))
    #endif
    #include    <stddef.h>
#endif

#if ( ( macintosh || __MACH__ ) && !KERNEL )

    #if ( defined( __MWERKS__ ) )
        #if ( __option( c9x ) )
            #include    <stdbool.h>
        #endif
    #else
        #include    <stdbool.h>
    #endif

    #include    <stdint.h>

    #if ( __MACH__ )

// Mac OS X
        #include    <sys/types.h>
        #include    <netinet/in.h>
        #include    <arpa/inet.h>
        #include    <fcntl.h>
        #include    <pthread.h>
        #include    <sys/ioctl.h>
        #include    <sys/socket.h>
        #include    <unistd.h>

    #else

// Classic Mac OS
        #include    <ConditionalMacros.h>
        #include    <MacTypes.h>

    #endif

#elif ( KERNEL )

// Mac OS X Kernel
    #include    <stdint.h>

    #include    <libkern/OSTypes.h>
    #include    <sys/types.h>

#elif ( TARGET_OS_FREEBSD )

// FreeBSD
    #include    <stdint.h>
    #include    <pthread.h>
    #include    <netinet/in.h>
    #include    <arpa/inet.h>
    #include    <sys/socket.h>

#elif ( TARGET_OS_LINUX )

// Linux
    #include    <stdint.h>
    #include    <arpa/inet.h>

#elif ( TARGET_OS_SOLARIS )

// Solaris
    #include    <stdint.h>

    #include    <arpa/inet.h>
    #include    <arpa/nameser.h>

    #if ( defined( BYTE_ORDER ) && defined( LITTLE_ENDIAN ) && ( BYTE_ORDER == LITTLE_ENDIAN ) )
        #define TARGET_RT_LITTLE_ENDIAN     1
    #endif
    #if ( defined( BYTE_ORDER ) && defined( BIG_ENDIAN ) && ( BYTE_ORDER == BIG_ENDIAN ) )
        #define TARGET_RT_BIG_ENDIAN        1
    #endif

#elif ( TARGET_OS_PALM )

// Palm (no special includes yet).

#elif ( TARGET_OS_VXWORKS )

// VxWorks
    #include    "vxWorks.h"

#elif ( TARGET_OS_WIN32 )

// Windows
    #if ( !defined( WIN32_WINDOWS ) )
        #define WIN32_WINDOWS       0x0401
    #endif

    #if ( !defined( _WIN32_WINDOWS ) )
        #define _WIN32_WINDOWS      0x0401
    #endif

    #if ( !defined( WIN32_LEAN_AND_MEAN ) )
        #define WIN32_LEAN_AND_MEAN         // Needed to avoid redefinitions by Windows interfaces.
    #endif

    #if ( defined( __MWERKS__ ) )

        #if ( __option( c9x ) )
            #include    <stdbool.h>
        #endif

        #include    <stdint.h>

    #elif ( defined( _MSC_VER ) )

        #pragma warning( disable:4127 ) // Disable "conditional expression is constant" warning for debug macros.

    #endif

    #include    <windows.h>
    #include    <winsock2.h>
    #include    <ws2tcpip.h>

#else
    #error unknown OS - update this file to support your OS
#endif

#if ( !defined( TARGET_BUILD_MAIN ) )
    #if ( !TARGET_OS_VXWORKS )
        #define TARGET_BUILD_MAIN       1
    #endif
#endif

#if ( __GNUC__ || !TARGET_OS_VXWORKS )
    #define TARGET_LANGUAGE_C_LIKE      1
#else
    #define TARGET_LANGUAGE_C_LIKE      0
#endif

#if 0
#pragma mark == CPU ==
#endif


// CPU

// PowerPC
#if ( !defined( TARGET_CPU_PPC ) )
    #if ( defined( __ppc__ ) || defined( __PPC__ ) || defined( powerpc ) || defined( ppc ) || defined( _M_MPPC ) )
        #define TARGET_CPU_PPC              1
    #else
        #define TARGET_CPU_PPC              0
    #endif
#endif

// x86
#if ( !defined( TARGET_CPU_X86 ) )
    #if ( __INTEL__ || defined( __i386__ ) || defined( i386 ) || defined( intel ) || defined( _M_IX86 ) )
        #define TARGET_CPU_X86              1
    #else
        #define TARGET_CPU_X86              0
    #endif
#endif

// MIPS
#if ( !defined( TARGET_CPU_MIPS ) )
    #if ( __MIPS__ || defined( MIPS32 ) || defined( R3000 ) || defined( R4000 ) || defined( R4650 ) || defined( _M_MRX000 ) )
        #define TARGET_CPU_MIPS             1
    #else
        #define TARGET_CPU_MIPS             0
    #endif
#endif

#if ( !defined( TARGET_CPU_PPC ) && !defined( TARGET_CPU_X86 ) && !defined( TARGET_CPU_MIPS ) )
    #error unknown CPU - update this file to support your CPU
#endif

#if 0
#pragma mark == Byte Order ==
#endif


//    Byte Order

// TARGET_RT_LITTLE_ENDIAN
#if ( !defined( TARGET_RT_LITTLE_ENDIAN ) )
    #if ( MIPSEL || IL_LITTLE_ENDIAN || defined( __LITTLE_ENDIAN__ )                                         || \
    ( defined(   BYTE_ORDER ) && defined(   LITTLE_ENDIAN ) && (   BYTE_ORDER ==   LITTLE_ENDIAN ) )   || \
    ( defined(  _BYTE_ORDER ) && defined(  _LITTLE_ENDIAN ) && (  _BYTE_ORDER ==  _LITTLE_ENDIAN ) )   || \
    ( defined( __BYTE_ORDER ) && defined( __LITTLE_ENDIAN ) && ( __BYTE_ORDER == __LITTLE_ENDIAN ) )   || \
    TARGET_CPU_X86 || ( defined( TARGET_RT_BIG_ENDIAN ) && !TARGET_RT_BIG_ENDIAN ) )
        #define TARGET_RT_LITTLE_ENDIAN     1
    #else
        #define TARGET_RT_LITTLE_ENDIAN     0
    #endif
#endif

// TARGET_RT_BIG_ENDIAN
#if ( !defined( TARGET_RT_BIG_ENDIAN ) )
    #if ( MIPSEB || IL_BIG_ENDIAN || defined( __BIG_ENDIAN__ )                                       || \
    ( defined(   BYTE_ORDER ) && defined(   BIG_ENDIAN ) && (   BYTE_ORDER ==   BIG_ENDIAN ) ) || \
    ( defined(  _BYTE_ORDER ) && defined(  _BIG_ENDIAN ) && (  _BYTE_ORDER ==  _BIG_ENDIAN ) ) || \
    ( defined( __BYTE_ORDER ) && defined( __BIG_ENDIAN ) && ( __BYTE_ORDER == __BIG_ENDIAN ) ) || \
    ( defined( TARGET_RT_LITTLE_ENDIAN ) && !TARGET_RT_LITTLE_ENDIAN ) )
        #define TARGET_RT_BIG_ENDIAN        1
    #else
        #define TARGET_RT_BIG_ENDIAN        0
    #endif
#endif

#if ( defined( TARGET_RT_LITTLE_ENDIAN ) && !defined( TARGET_RT_BIG_ENDIAN ) )
    #if ( TARGET_RT_LITTLE_ENDIAN )
        #define TARGET_RT_BIG_ENDIAN        0
    #else
        #define TARGET_RT_BIG_ENDIAN        1
    #endif
#endif

#if ( defined( TARGET_RT_BIG_ENDIAN ) && !defined( TARGET_RT_LITTLE_ENDIAN ) )
    #if ( TARGET_RT_BIG_ENDIAN )
        #define TARGET_RT_LITTLE_ENDIAN     0
    #else
        #define TARGET_RT_LITTLE_ENDIAN     1
    #endif
#endif

#if ( !defined( TARGET_RT_LITTLE_ENDIAN ) || !defined( TARGET_RT_BIG_ENDIAN ) )
    #error unknown byte order - update this file to support your byte order
#endif

// TARGET_RT_BYTE_ORDER
#if ( !defined( TARGET_RT_BYTE_ORDER_BIG_ENDIAN ) )
    #define TARGET_RT_BYTE_ORDER_BIG_ENDIAN         1234
#endif

#if ( !defined( TARGET_RT_BYTE_ORDER_LITTLE_ENDIAN ) )
    #define TARGET_RT_BYTE_ORDER_LITTLE_ENDIAN      4321
#endif

#if ( !defined( TARGET_RT_BYTE_ORDER ) )
    #if ( TARGET_RT_LITTLE_ENDIAN )
        #define TARGET_RT_BYTE_ORDER                TARGET_RT_BYTE_ORDER_LITTLE_ENDIAN
    #else
        #define TARGET_RT_BYTE_ORDER                TARGET_RT_BYTE_ORDER_BIG_ENDIAN
    #endif
#endif

#if 0
#pragma mark == Constants ==
#endif


//    Constants
#if ( !TARGET_OS_MAC )
    #define CR      '\r'
#endif

#define LF          '\n'
#define CRLF        "\r\n"

#if 0
#pragma mark == Compatibility ==
#endif


//    Compatibility

// Macros to allow the same code to work on Windows and other sockets API-compatible platforms.
#if ( TARGET_OS_WIN32 )
    #define close_compat( X )       closesocket( X )
    #define errno_compat()          (int) GetLastError()
    #define set_errno_compat( X )   SetLastError( X )
    #define EWOULDBLOCK_compat      WSAEWOULDBLOCK
    #define ETIMEDOUT_compat        WSAETIMEDOUT
    #define ENOTCONN_compat         WSAENOTCONN
    #define IsValidSocket( X )      ( ( X ) != INVALID_SOCKET )
    #define kInvalidSocketRef       INVALID_SOCKET
    #if ( TARGET_LANGUAGE_C_LIKE )
typedef SOCKET SocketRef;
    #endif
#else
    #define close_compat( X )       close( X )
    #define errno_compat()          errno
    #define set_errno_compat( X )   do { errno = ( X ); } while( 0 )
    #define EWOULDBLOCK_compat      EWOULDBLOCK
    #define ETIMEDOUT_compat        ETIMEDOUT
    #define ENOTCONN_compat         ENOTCONN
    #define IsValidSocket( X )      ( ( X ) >= 0 )
    #define kInvalidSocketRef       -1
    #if ( TARGET_LANGUAGE_C_LIKE )
typedef int SocketRef;
    #endif
#endif

// socklen_t is not defined on the following platforms so emulate it if not defined:

// - Pre-Panther Mac OS X. Panther defines SO_NOADDRERR so trigger off that.
// - Windows SDK prior to 2003. 2003+ SDK's define EAI_AGAIN so trigger off that.
// - VxWorks

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( ( TARGET_OS_MAC && !defined( SO_NOADDRERR ) ) || ( TARGET_OS_WIN32 && !defined( EAI_AGAIN ) ) || TARGET_OS_VXWORKS )
typedef int socklen_t;
    #endif
#endif

// ssize_t is not defined on the following platforms so emulate it if not defined:

// - Mac OS X when not building with BSD headers
// - Windows

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( !defined(_SSIZE_T) && ( TARGET_OS_WIN32 || !defined( _BSD_SSIZE_T_DEFINED_ ) ) && !TARGET_OS_FREEBSD && !TARGET_OS_LINUX && !TARGET_OS_VXWORKS && !TARGET_OS_MAC && !defined (__MINGW32__))
typedef int ssize_t;
    #endif
#endif

// sockaddr_storage is not supported on non-IPv6 machines so alias it to an IPv4-compatible structure.

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( !defined( AF_INET6 ) )
        #define sockaddr_storage        sockaddr_in
        #define ss_family               sin_family
    #endif
#endif


/*!    @defined    SOCKADDR_IS_IP_LOOPBACK

    @abstract    Determines if a sockaddr is an IPv4 or IPv6 loopback address (if IPv6 is supported).
 */
#if ( defined( AF_INET6 ) )
    #define SOCKADDR_IS_IP_LOOPBACK( SA )                                                           \
    ( ( (const struct sockaddr *)( SA ) )->sa_family == AF_INET )                               \
    ? ( ( (const struct sockaddr_in *)( SA ) )->sin_addr.s_addr == htonl( INADDR_LOOPBACK ) )   \
    : ( ( (const struct sockaddr *)( SA ) )->sa_family == AF_INET6 )                            \
    ? IN6_IS_ADDR_LOOPBACK( &( (const struct sockaddr_in6 *)( SA ) )->sin6_addr )           \
    : 0
#else
    #define SOCKADDR_IS_IP_LOOPBACK( SA )                                                           \
    ( ( (const struct sockaddr *)( SA ) )->sa_family == AF_INET )                               \
    ? ( ( (const struct sockaddr_in *)( SA ) )->sin_addr.s_addr == htonl( INADDR_LOOPBACK ) )   \
    : 0
#endif


/*!    @defined    SOCKADDR_IS_IP_LINK_LOCAL

    @abstract    Determines if a sockaddr is an IPv4 or IPv6 link-local address (if IPv6 is supported).
 */
#if ( defined( AF_INET6 ) )
    #define SOCKADDR_IS_IP_LINK_LOCAL( SA )                                                             \
    ( ( ( (const struct sockaddr *)( SA ) )->sa_family == AF_INET )                                 \
      ? ( ( ( (uint8_t *)( &( (const struct sockaddr_in *)( SA ) )->sin_addr ) )[ 0 ] == 169 ) &&   \
          ( ( (uint8_t *)( &( (const struct sockaddr_in *)( SA ) )->sin_addr ) )[ 1 ] == 254 ) )    \
      : IN6_IS_ADDR_LOOPBACK( &( (const struct sockaddr_in6 *)( SA ) )->sin6_addr ) )
#else
    #define SOCKADDR_IS_IP_LINK_LOCAL( SA )                                                             \
    ( ( ( (const struct sockaddr *)( SA ) )->sa_family == AF_INET )                                 \
      ? ( ( ( (uint8_t *)( &( (const struct sockaddr_in *)( SA ) )->sin_addr ) )[ 0 ] == 169 ) &&   \
          ( ( (uint8_t *)( &( (const struct sockaddr_in *)( SA ) )->sin_addr ) )[ 1 ] == 254 ) )    \
      : 0 )
#endif

// _beginthreadex and _endthreadex are not supported on Windows CE 2.1 or later (the C runtime issues with leaking
// resources have apparently been resolved and they seem to have just ripped out support for the API) so map it to
// CreateThread on Windows CE.
#if ( TARGET_OS_WINDOWS_CE )
    #define _beginthreadex_compat( SECURITY_PTR, STACK_SIZE, START_ADDRESS, ARG_LIST, FLAGS, THREAD_ID_PTR )            \
    (uintptr_t) CreateThread( SECURITY_PTR, STACK_SIZE, (LPTHREAD_START_ROUTINE) START_ADDRESS, ARG_LIST, FLAGS,    \
                              (LPDWORD) THREAD_ID_PTR )

    #define _endthreadex_compat( RESULT )       ExitThread( (DWORD) RESULT )
#elif ( TARGET_OS_WIN32 )
    #define _beginthreadex_compat               _beginthreadex
    #define _endthreadex_compat                 _endthreadex
#endif

// The C99 "inline" keyword is not supported by Microsoft compilers, but they do support __inline so map it when needed.

#if ( defined( _MSC_VER ) )
    #define inline_compat       __inline
#else
    #define inline_compat       inline
#endif

// Calling conventions

#if ( !defined( CALLBACK_COMPAT ) )
    #if ( TARGET_OS_WIN32 || TARGET_OS_WINDOWS_CE )
        #define CALLBACK_COMPAT     CALLBACK
    #else
        #define CALLBACK_COMPAT
    #endif
#endif

#if 0
#pragma mark == Macros ==
#endif


/*!    @defined    kSizeCString

    @abstract    A meta-value to pass to supported routines to indicate the size should be calculated with strlen.
 */

#define kSizeCString        ( (size_t) -1 )


/*!    @defined    sizeof_array

    @abstract    Determines the number of elements in an array.
 */

#define sizeof_array( X )       ( sizeof( X ) / sizeof( X[ 0 ] ) )


/*!    @defined    sizeof_element

    @abstract    Determines the size of an array element.
 */

#define sizeof_element( X )     sizeof( X[ 0 ] )


/*!    @defined    sizeof_string

    @abstract    Determines the size of a constant C string, excluding the null terminator.
 */

#define sizeof_string( X )      ( sizeof( ( X ) ) - 1 )


/*!    @defined    sizeof_field

    @abstract    Determines the size of a field of a type.
 */

#define sizeof_field( TYPE, FIELD )     sizeof( ( ( (TYPE *) 0 )->FIELD ) )


/*!    @function    RoundUp

    @abstract    Rounds X up to a multiple of Y.
 */

#define RoundUp( X, Y )     ( ( X ) + ( ( Y ) -( ( X ) % ( Y ) ) ) )


/*!    @function    IsAligned

    @abstract    Returns non-zero if X is aligned to a Y byte boundary and 0 if not. Y must be a power of 2.
 */

#define IsAligned( X, Y )       ( ( ( X ) &( ( Y ) -1 ) ) == 0 )


/*!    @function    IsFieldAligned

    @abstract    Returns non-zero if FIELD of type TYPE is aligned to a Y byte boundary and 0 if not. Y must be a power of 2.
 */

#define IsFieldAligned( X, TYPE, FIELD, Y )     IsAligned( ( (uintptr_t)( X ) ) + offsetof( TYPE, FIELD ), ( Y ) )


/*!    @function    AlignDown

    @abstract    Aligns X down to a Y byte boundary. Y must be a power of 2.
 */

#define AlignDown( X, Y )       ( ( X ) &~( ( Y ) -1 ) )


/*!    @function    AlignUp

    @abstract    Aligns X up to a Y byte boundary. Y must be a power of 2.
 */

#define AlignUp( X, Y )     ( ( ( X ) + ( ( Y ) -1 ) ) & ~( ( Y ) -1 ) )


/*!    @function    Min

    @abstract    Returns the lesser of X and Y.
 */

#if ( !defined( Min ) )
    #define Min( X, Y )     ( ( ( X ) < ( Y ) ) ? ( X ) : ( Y ) )
#endif


/*!    @function    Max

    @abstract    Returns the greater of X and Y.
 */

#if ( !defined( Max ) )
    #define Max( X, Y )     ( ( ( X ) > ( Y ) ) ? ( X ) : ( Y ) )
#endif


/*!    @function    InsertBits

    @abstract    Inserts BITS (both 0 and 1 bits) into X, controlled by MASK and SHIFT, and returns the result.

    @discussion

    MASK is the bitmask of the bits in the final position.
    SHIFT is the number of bits to shift left for 1 to reach the first bit position of MASK.

    For example, if you wanted to insert 0x3 into the leftmost 4 bits of a 32-bit value:

    InsertBits( 0, 0x3, 0xF0000000U, 28 ) == 0x30000000
 */

#define InsertBits( X, BITS, MASK, SHIFT )      ( ( ( X ) &~( MASK ) ) | ( ( ( BITS ) << ( SHIFT ) ) & ( MASK ) ) )


/*!    @function    ExtractBits

    @abstract    Extracts bits from X, controlled by MASK and SHIFT, and returns the result.

    @discussion

    MASK is the bitmask of the bits in the final position.
    SHIFT is the number of bits to shift right to right justify MASK.

    For example, if you had a 32-bit value (e.g. 0x30000000) wanted the left-most 4 bits (e.g. 3 in this example):

    ExtractBits( 0x30000000U, 0xF0000000U, 28 ) == 0x3
 */

#define ExtractBits( X, MASK, SHIFT )           ( ( ( X ) >> ( SHIFT ) ) & ( ( MASK ) >> ( SHIFT ) ) )


/*!    @function    Stringify

    @abstract    Stringify's an expression.

    @discussion

    Stringify macros to process raw text passed via -D options to C string constants. The double-wrapping is necessary
    because the C preprocessor doesn't perform its normal argument expansion pre-scan with stringified macros so the
    -D macro needs to be expanded once via the wrapper macro then stringified so the raw text is stringified. Otherwise,
    the replacement value would be used instead of the symbolic name (only for preprocessor symbols like #defines).

    For example:

 #define    kMyConstant        1

        printf( "%s", Stringify( kMyConstant ) );            // Prints "kMyConstant"
        printf( "%s", StringifyExpansion( kMyConstant ) );    // Prints "1"

    Non-preprocessor symbols do not have this issue. For example:

        enum
        {
            kMyConstant = 1
        };

        printf( "%s", Stringify( kMyConstant ) );            // Prints "kMyConstant"
        printf( "%s", StringifyExpansion( kMyConstant ) );    // Prints "kMyConstant"

    See <http://gcc.gnu.org/onlinedocs/cpp/Argument-Prescan.html> for more info on C preprocessor pre-scanning.
 */

#define Stringify( X )              # X
#define StringifyExpansion( X )     Stringify( X )

#if 0
#pragma mark == Types ==
#endif

#if ( TARGET_LANGUAGE_C_LIKE )

//     Standard Types


#if ( !defined( INT8_MIN ) )

    #define INT8_MIN                    SCHAR_MIN

    #if ( defined( _MSC_VER ) )

// C99 stdint.h not supported in VC++/VS.NET yet.

typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
    #elif defined (__MINGW32__)
    #  include <stdint.h>
    #elif ( TARGET_OS_VXWORKS && ( TORNADO_VERSION < 220 ) )
typedef long long int64_t;
typedef unsigned long long uint64_t;
    #endif

typedef int32_t int_least32_t;




    #if ( !defined( _WIN32 ) || TARGET_OS_WINDOWS_CE )
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
    #endif

#endif

// Macros for minimum-width integer constants

#if ( !defined( INT8_C ) )
    #define INT8_C( value )         value
#endif

#if ( !defined( INT16_C ) )
    #define INT16_C( value )        value
#endif

#if ( !defined( INT32_C ) )
    #define INT32_C( value )        value ## L
#endif

#if ( !defined( INT64_C ) )
    #if ( defined( _MSC_VER ) )
        #define INT64_C( value )    value ## i64
    #else
        #define INT64_C( value )    value ## LL
    #endif
#endif

#if ( !defined( UINT8_C ) )
    #define UINT8_C( value )        value ## U
#endif

#if ( !defined( UINT16_C ) )
    #define UINT16_C( value )       value ## U
#endif

#if ( !defined( UINT32_C ) )
    #define UINT32_C( value )       value ## UL
#endif

#if ( !defined( UINT64_C ) )
    #if ( defined( _MSC_VER ) )
        #define UINT64_C( value )   value ## UI64
    #else
        #define UINT64_C( value )   value ## ULL
    #endif
#endif

#if 0
#pragma mark == bool ==
#endif


//     Boolean Constants and Types


// C++ defines bool, true, and false. Metrowerks allows this to be controlled by the "bool" option though.
// C99 defines __bool_true_false_are_defined when bool, true, and false are defined.
// MacTypes.h defines true and false (Mac builds only).

// Note: The Metrowerks has to be in its own block because Microsoft Visual Studio .NET does not completely
// short-circuit and gets confused by the option( bool ) portion of the conditional.

#if ( defined( __MWERKS__ ) )

// Note: The following test is done on separate lines because CodeWarrior doesn't like it all on one line.

    #if ( !__bool_true_false_are_defined && ( !defined( __cplusplus ) || !__option( bool ) ) )
        #define COMMON_SERVICES_NEEDS_BOOL      1
    #else
        #define COMMON_SERVICES_NEEDS_BOOL      0
    #endif

// Workaround when building with CodeWarrior, but using the Apple stdbool.h header, which uses _Bool.

    #if ( __bool_true_false_are_defined && !defined( __cplusplus ) && !__option( c9x ) )
        #define _Bool int
    #endif

// Workaround when building with CodeWarrior for C++ with bool disabled and using the Apple stdbool.h header,
// which defines true and false to map to C++ true and false (which are not enabled). Serenity Now!

    #if ( __bool_true_false_are_defined && defined( __cplusplus ) && !__option( bool ) )
        #define true    1
        #define false   0
    #endif
#else
    #define COMMON_SERVICES_NEEDS_BOOL          ( !defined( __cplusplus ) && !__bool_true_false_are_defined )
#endif

#if ( COMMON_SERVICES_NEEDS_BOOL )

typedef int bool;

    #define bool bool

    #if ( !defined( __MACTYPES__ ) && !defined( true ) && !defined( false ) )
        #define true    1
        #define false   0
    #endif

    #define __bool_true_false_are_defined       1
#endif

// IOKit IOTypes.h typedef's bool if TYPE_BOOL is not defined so define it here to prevent redefinition by IOTypes.h.

#if ( TARGET_API_MAC_OSX_KERNEL )
    #define TYPE_BOOL       1
#endif


/*!    @typedef    CStr255

    @abstract    255 character null-terminated (C-style) string.
 */

#if ( TARGET_LANGUAGE_C_LIKE )
typedef char CStr255[ 256 ];
#endif

#endif  // TARGET_LANGUAGE_C_LIKE


/*!    @defined    TYPE_LONGLONG_NATIVE

    @abstract    Defines whether long long (or its equivalent) is natively supported or requires special libraries.
 */

#if ( !defined( TYPE_LONGLONG_NATIVE ) )
    #if ( !TARGET_OS_VXWORKS )
        #define TYPE_LONGLONG_NATIVE            1
    #else
        #define TYPE_LONGLONG_NATIVE            0
    #endif
#endif


/*!    @defined    long_long_compat

    @abstract    Compatibility type to map to the closest thing to long long and unsigned long long.

    @discussion

    Neither long long nor unsigned long long are supported by Microsoft compilers, but they do support proprietary
    "__int64" and "unsigned __int64" equivalents so map to those types if the real long long is not supported.
 */

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( TARGET_OS_WIN32 )
typedef __int64 long_long_compat;
typedef unsigned __int64 unsigned_long_long_compat;
    #else
typedef signed long long long_long_compat;
typedef unsigned long long unsigned_long_long_compat;
    #endif
#endif

#if 0
#pragma mark == Errors ==
#endif


/*!    @enum        OSStatus

    @abstract    Status Code

    @constant    kNoErr                            0 No error occurred.
    @constant    kInProgressErr                    1 Operation in progress.
    @constant    kUnknownErr                    -6700 Unknown error occurred.
    @constant    kOptionErr                    -6701 Option was not acceptable.
    @constant    kSelectorErr                -6702 Selector passed in is invalid or unknown.
    @constant    kExecutionStateErr            -6703 Call made in the wrong execution state (e.g. called at interrupt time).
    @constant    kPathErr                    -6704 Path is invalid, too long, or otherwise not usable.
    @constant    kParamErr                    -6705 Parameter is incorrect, missing, or not appropriate.
    @constant    kParamCountErr                -6706 Incorrect or unsupported number of parameters.
    @constant    kCommandErr                    -6707 Command invalid or not supported.
    @constant    kIDErr                        -6708 Unknown, invalid, or inappropriate identifier.
    @constant    kStateErr                    -6709 Not in appropriate state to perform operation.
    @constant    kRangeErr                    -6710 Index is out of range or not valid.
    @constant    kRequestErr                    -6711 Request was improperly formed or not appropriate.
    @constant    kResponseErr                -6712 Response was incorrect or out of sequence.
    @constant    kChecksumErr                -6713 Checksum does not match the actual data.
    @constant    kNotHandledErr                -6714 Operation was not handled (or not handled completely).
    @constant    kVersionErr                    -6715 Version is not incorrect or not compatibile.
    @constant    kSignatureErr                -6716 Signature did not match what was expected.
    @constant    kFormatErr                    -6717 Unknown, invalid, or inappropriate file/data format.
    @constant    kNotInitializedErr            -6718 Action request before needed services were initialized.
    @constant    kAlreadyInitializedErr        -6719 Attempt made to initialize when already initialized.
    @constant    kNotInUseErr                -6720 Object not in use (e.g. cannot abort if not already in use).
    @constant    kInUseErr                    -6721 Object is in use (e.g. cannot reuse active param blocks).
    @constant    kTimeoutErr                    -6722 Timeout occurred.
    @constant    kCanceledErr                -6723 Operation canceled (successful cancel).
    @constant    kAlreadyCanceledErr            -6724 Operation has already been canceled.
    @constant    kCannotCancelErr            -6725 Operation could not be canceled (maybe already done or invalid).
    @constant    kDeletedErr                    -6726 Object has already been deleted.
    @constant    kNotFoundErr                -6727 Something was not found.
    @constant    kNoMemoryErr                -6728 Not enough memory was available to perform the operation.
    @constant    kNoResourcesErr                -6729 Resources unavailable to perform the operation.
    @constant    kDuplicateErr                -6730 Duplicate found or something is a duplicate.
    @constant    kImmutableErr                -6731 Entity is not changeable.
    @constant    kUnsupportedDataErr            -6732 Data is unknown or not supported.
    @constant    kIntegrityErr                -6733 Data is corrupt.
    @constant    kIncompatibleErr            -6734 Data is not compatible or it is in an incompatible format.
    @constant    kUnsupportedErr                -6735 Feature or option is not supported.
    @constant    kUnexpectedErr                -6736 Error occurred that was not expected.
    @constant    kValueErr                    -6737 Value is not appropriate.
    @constant    kNotReadableErr                -6738 Could not read or reading is not allowed.
    @constant    kNotWritableErr                -6739 Could not write or writing is not allowed.
    @constant    kBadReferenceErr            -6740 An invalid or inappropriate reference was specified.
    @constant    kFlagErr                    -6741 An invalid, inappropriate, or unsupported flag was specified.
    @constant    kMalformedErr                -6742 Something was not formed correctly.
    @constant    kSizeErr                    -6743 Size was too big, too small, or not appropriate.
    @constant    kNameErr                    -6744 Name was not correct, allowed, or appropriate.
    @constant    kNotReadyErr                -6745 Device or service is not ready.
    @constant    kReadErr                    -6746 Could not read.
    @constant    kWriteErr                    -6747 Could not write.
    @constant    kMismatchErr                -6748 Something does not match.
    @constant    kDateErr                    -6749 Date is invalid or out-of-range.
    @constant    kUnderrunErr                -6750 Less data than expected.
    @constant    kOverrunErr                    -6751 More data than expected.
    @constant    kEndingErr                    -6752 Connection, session, or something is ending.
    @constant    kConnectionErr                -6753 Connection failed or could not be established.
    @constant    kAuthenticationErr            -6754 Authentication failed or is not supported.
    @constant    kOpenErr                    -6755 Could not open file, pipe, device, etc.
    @constant    kTypeErr                    -6756 Incorrect or incompatible type (e.g. file, data, etc.).
    @constant    kSkipErr                    -6757 Items should be or was skipped.
    @constant    kNoAckErr                    -6758 No acknowledge.
    @constant    kCollisionErr                -6759 Collision occurred (e.g. two on bus at same time).
    @constant    kBackoffErr                    -6760 Backoff in progress and operation intentionally failed.
    @constant    kNoAddressAckErr            -6761 No acknowledge of address.
    @constant    kBusyErr                    -6762 Cannot perform because something is busy.
    @constant    kNoSpaceErr                    -6763 Not enough space to perform operation.
 */

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( !TARGET_OS_MAC && !TARGET_API_MAC_OSX_KERNEL )
typedef int32_t OSStatus;
    #endif
#endif

#define kNoErr                      0
#define kInProgressErr              1

// Generic error codes are in the range -6700 to -6779.

#define kGenericErrorBase           -6700   // Starting error code for all generic errors.

#define kUnknownErr                 -6700
#define kOptionErr                  -6701
#define kSelectorErr                -6702
#define kExecutionStateErr          -6703
#define kPathErr                    -6704
#define kParamErr                   -6705
#define kParamCountErr              -6706
#define kCommandErr                 -6707
#define kIDErr                      -6708
#define kStateErr                   -6709
#define kRangeErr                   -6710
#define kRequestErr                 -6711
#define kResponseErr                -6712
#define kChecksumErr                -6713
#define kNotHandledErr              -6714
#define kVersionErr                 -6715
#define kSignatureErr               -6716
#define kFormatErr                  -6717
#define kNotInitializedErr          -6718
#define kAlreadyInitializedErr      -6719
#define kNotInUseErr                -6720
#define kInUseErr                   -6721
#define kTimeoutErr                 -6722
#define kCanceledErr                -6723
#define kAlreadyCanceledErr         -6724
#define kCannotCancelErr            -6725
#define kDeletedErr                 -6726
#define kNotFoundErr                -6727
#define kNoMemoryErr                -6728
#define kNoResourcesErr             -6729
#define kDuplicateErr               -6730
#define kImmutableErr               -6731
#define kUnsupportedDataErr         -6732
#define kIntegrityErr               -6733
#define kIncompatibleErr            -6734
#define kUnsupportedErr             -6735
#define kUnexpectedErr              -6736
#define kValueErr                   -6737
#define kNotReadableErr             -6738
#define kNotWritableErr             -6739
#define kBadReferenceErr            -6740
#define kFlagErr                    -6741
#define kMalformedErr               -6742
#define kSizeErr                    -6743
#define kNameErr                    -6744
#define kNotReadyErr                -6745
#define kReadErr                    -6746
#define kWriteErr                   -6747
#define kMismatchErr                -6748
#define kDateErr                    -6749
#define kUnderrunErr                -6750
#define kOverrunErr                 -6751
#define kEndingErr                  -6752
#define kConnectionErr              -6753
#define kAuthenticationErr          -6754
#define kOpenErr                    -6755
#define kTypeErr                    -6756
#define kSkipErr                    -6757
#define kNoAckErr                   -6758
#define kCollisionErr               -6759
#define kBackoffErr                 -6760
#define kNoAddressAckErr            -6761
#define kBusyErr                    -6762
#define kNoSpaceErr                 -6763

#define kGenericErrorEnd            -6779   // Last generic error code (inclusive)

#if 0
#pragma mark == Mac Compatibility ==
#endif


//    Mac Compatibility



/*!    @enum        Duration

    @abstract    Type used to specify a duration of time.

    @constant    kDurationImmediate            Indicates no delay/wait time.
    @constant    kDurationMicrosecond        Microsecond units.
    @constant    kDurationMillisecond        Millisecond units.
    @constant    kDurationSecond                Second units.
    @constant    kDurationMinute                Minute units.
    @constant    kDurationHour                Hour units.
    @constant    kDurationDay                Day units.
    @constant    kDurationForever            Infinite period of time (no timeout).

    @discussion

    Duration values are intended to be multiplied by the specific interval to achieve an actual duration. For example,
    to wait for 5 seconds you would use "5 * kDurationSecond".
 */

#if ( TARGET_LANGUAGE_C_LIKE )
    #if ( !TARGET_OS_MAC )
typedef int32_t Duration;
    #endif
#endif

#define kDurationImmediate              0L
#define kDurationMicrosecond            -1L
#define kDurationMillisecond            1L
#define kDurationSecond                 ( 1000L * kDurationMillisecond )
#define kDurationMinute                 ( 60L * kDurationSecond )
#define kDurationHour                   ( 60L * kDurationMinute )
#define kDurationDay                    ( 24L * kDurationHour )
#define kDurationForever                0x7FFFFFFFL

// Seconds <-> Minutes <-> Hours <-> Days <-> Weeks <-> Months <-> Years conversions

#define kSecondsPerHour                 ( 60 * 60 )             // 3600
#define kSecondsPerDay                  ( 60 * 60 * 24 )        // 86400
#define kSecondsPerWeek                 ( 60 * 60 * 24 * 7 )    // 604800
#define kMinutesPerDay                  ( 60 * 24 )             // 1440


/*!    @defined    VersionStages

    @abstract    NumVersion-style version stages.
 */

#define kVersionStageDevelopment        0x20
#define kVersionStageAlpha              0x40
#define kVersionStageBeta               0x60
#define kVersionStageFinal              0x80


/*!    @function    NumVersionBuild

    @abstract    Builds a 32-bit Mac-style NumVersion value (e.g. NumVersionBuild( 1, 2, 3, kVersionStageBeta, 4 ) -> 1.2.3b4).
 */

#define NumVersionBuild( MAJOR, MINOR, BUGFIX, STAGE, REV ) \
    ( ( ( ( MAJOR )  & 0xFF ) << 24 ) |                     \
      ( ( ( MINOR )  & 0x0F ) << 20 ) |                     \
      ( ( ( BUGFIX ) & 0x0F ) << 16 ) |                     \
      ( ( ( STAGE )  & 0xFF ) <<  8 ) |                     \
      ( ( ( REV )    & 0xFF )       ) )

#define NumVersionExtractMajor( VERSION )               ( (uint8_t)( ( ( VERSION ) >> 24 ) & 0xFF ) )
#define NumVersionExtractMinorAndBugFix( VERSION )      ( (uint8_t)( ( ( VERSION ) >> 16 ) & 0xFF ) )
#define NumVersionExtractMinor( VERSION )               ( (uint8_t)( ( ( VERSION ) >> 20 ) & 0x0F ) )
#define NumVersionExtractBugFix( VERSION )              ( (uint8_t)( ( ( VERSION ) >> 16 ) & 0x0F ) )
#define NumVersionExtractStage( VERSION )               ( (uint8_t)( ( ( VERSION ) >>  8 ) & 0xFF ) )
#define NumVersionExtractRevision( VERSION )            ( (uint8_t)(   ( VERSION )         & 0xFF ) )


/*!    @function    NumVersionCompare

    @abstract    Compares two NumVersion values and returns the following values:

        left < right -> -1
        left > right ->  1
        left = right ->  0
 */

#if ( TARGET_LANGUAGE_C_LIKE )
int NumVersionCompare( uint32_t inLeft, uint32_t inRight );
#endif

#if 0
#pragma mark == Binary Constants ==
#endif


/*!    @defined    binary_4

    @abstract    Macro to generate an 4-bit constant using binary notation (e.g. binary_4( 1010 ) == 0xA).
 */

#define binary_4( a )                       binary_4_hex_wrap( hex_digit4( a ) )
#define binary_4_hex_wrap( a )              binary_4_hex( a )
#define binary_4_hex( a )                   ( 0x ## a )


/*!    @defined    binary_8

    @abstract    Macro to generate an 8-bit constant using binary notation (e.g. binary_8( 01111011 ) == 0x7B).
 */

#define binary_8( a )                       binary_8_hex_wrap( hex_digit8( a ) )
#define binary_8_hex_wrap( a )              binary_8_hex( a )
#define binary_8_hex( a )                   ( 0x ## a )


/*!    @defined    binary_16

    @abstract    Macro to generate an 16-bit constant using binary notation (e.g. binary_16( 01111011, 01111011 ) == 0x7B7B).
 */

#define binary_16( a, b )                   binary_16_hex_wrap( hex_digit8( a ), hex_digit8( b ) )
#define binary_16_hex_wrap( a, b )          binary_16_hex( a, b )
#define binary_16_hex( a, b )               ( 0x ## a ## b )


/*!    @defined    binary_32

    @abstract    Macro to generate an 32-bit constant using binary notation
                (e.g. binary_32( 01111011, 01111011, 01111011, 01111011 ) == 0x7B7B7B7B).
 */

#define binary_32( a, b, c, d )             binary_32_hex_wrap( hex_digit8( a ), hex_digit8( b ), hex_digit8( c ), hex_digit8( d ) )
#define binary_32_hex_wrap( a, b, c, d )    binary_32_hex( a, b, c, d )
#define binary_32_hex( a, b, c, d )         ( 0x ## a ## b ## c ## d )

// Binary Constant Helpers

#define hex_digit8( a )                     HEX_DIGIT_ ## a
#define hex_digit4( a )                     HEX_DIGIT_ ## 0000 ## a


#if 0
#pragma mark == Debugging ==
#endif


/*!    @function    CommonServicesTest

    @abstract    Unit test.
 */

#if ( DEBUG )
    #if ( TARGET_LANGUAGE_C_LIKE )
OSStatus    CommonServicesTest( void );
    #endif
#endif

#ifdef  __cplusplus
}
#endif

#endif  // __COMMON_SERVICES__
