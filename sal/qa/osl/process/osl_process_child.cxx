/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: osl_process_child.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:32:35 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

//########################################
// includes

#if ( defined WNT )                     // Windows
#include <tools/prewin.h>
#   define UNICODE
#   define _UNICODE
#   define WIN32_LEAN_AND_MEAN
// #    include <windows.h>
#   include <tchar.h>
#include <tools/postwin.h>
#else
#   include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <rtl/ustring.hxx>

//########################################
// defines

#ifdef WNT
#   define SLEEP(t) (Sleep((t)*1000))
#else
#   define SLEEP(t) (sleep((t)))
#endif

//########################################
void wait_for_seconds(char* time)
{
    SLEEP(atoi(time));
}

//########################################

#ifdef WNT
//########################################
void w_to_a(LPCTSTR _strW, LPSTR strA, DWORD size)
{
    LPCWSTR strW = reinterpret_cast<LPCWSTR>(_strW);
    WideCharToMultiByte(CP_ACP, 0, strW, -1, strA, size, NULL, NULL);
}
//########################################
    void dump_env(char* file_path)
    {
        LPTSTR env = reinterpret_cast<LPTSTR>(
            GetEnvironmentStrings());
        LPTSTR p   = env;

        std::ofstream file(file_path);

        char buffer[32767];
        while (size_t l = _tcslen(reinterpret_cast<wchar_t*>(p)))
        {
            w_to_a(p, buffer, sizeof(buffer));
            file << buffer << std::endl;
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char** environ;

    void dump_env(char* file_path)
    {
        std::ofstream file(file_path);
        for (int i = 0; NULL != environ[i]; i++)
            file << environ[i] << std::endl;
    }
#endif

//########################################
int main(int argc, char* argv[])
{
    rtl::OUString s;

    //t_print("Parameter: ");
    printf("child process Parameter: ");
    for (int i = 1; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    if (argc > 2)
    {
        if (0 == strcmp("-join", argv[1]))
        {
            wait_for_seconds(argv[2]);
        }
        else if (0 == strcmp("-env", argv[1]))
        {
            dump_env(argv[2]);
        }
    }

    return (0);
}

