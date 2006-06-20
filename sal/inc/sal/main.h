/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: main.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:15:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SAL_MAIN_H_
#define _SAL_MAIN_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Prototype of sal main entry */

int  SAL_CALL sal_main (int argc, char ** argv);
void SAL_CALL osl_setCommandArgs(int argc, char ** argv);


/* Definition macros for CRT entries */

#ifdef SAL_W32

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

/* Sorry but this is neccessary cause HINSTANCE is a typedef that differs (C++ causes an error) */

#if 0

#ifndef _WINDOWS_
#include <windows.h>
#endif

#else /* Simulated what windows.h does */

#ifndef WINAPI
#   define WINAPI   __stdcall
#endif

#if !defined(DECLARE_HANDLE)
#   ifdef STRICT
        typedef void *HANDLE;
#       define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#   else
        typedef void *PVOID;
        typedef PVOID HANDLE;
#       define DECLARE_HANDLE(name) typedef HANDLE name
#   endif
DECLARE_HANDLE(HINSTANCE);
#endif

#endif

#define SAL_DEFINE_CRT_ENTRY() \
int __cdecl main(int argc, char ** argv) \
{ \
    osl_setCommandArgs(argc, argv); \
    return sal_main(argc, argv); \
} \
int WINAPI WinMain( HINSTANCE _hinst, HINSTANCE _dummy, char* _cmdline, int _nshow ) \
{ \
    int argc = __argc; char ** argv = __argv; \
    (void) _hinst; (void) _dummy; (void) _cmdline; (void) _nshow; /* unused */ \
    osl_setCommandArgs(argc, argv); \
    return sal_main(argc, argv); \
}

#else   /* ! SAL_W32 */

#define SAL_DEFINE_CRT_ENTRY() \
int main(int argc, char ** argv) \
{ \
    osl_setCommandArgs(argc, argv); \
    return sal_main(argc, argv); \
} \

#endif /* ! SAL_W32 */

/* Implementation macro */

#define SAL_IMPLEMENT_MAIN_WITH_ARGS(_argc_, _argv_) \
    SAL_DEFINE_CRT_ENTRY() \
    int SAL_CALL sal_main(int _argc_, char ** _argv_)

#define SAL_IMPLEMENT_MAIN() SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)


/*
    "How to use" Examples:

    #include <sal/main.h>

    SAL_IMPLEMENT_MAIN()
    {
        DoSomething();

        return 0;
    }

    SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
    {
        DoSomethingWithArgs(argc, argv);

        return 0;
    }

*/

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /* _SAL_MAIN_H_ */

