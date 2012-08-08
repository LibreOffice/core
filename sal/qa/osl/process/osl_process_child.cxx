/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


//########################################


#if ( defined WNT )                     // Windows
#   define UNICODE
#   define _UNICODE
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <tchar.h>
#else
#   include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>

#include <rtl/ustring.hxx>

#ifdef UNX
#if defined( MACOSX )
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
# else
    extern char** environ;
# endif
#endif

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
            file << buffer << '\0';
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
    }
#else
    void dump_env(char* file_path)
    {
        std::ofstream file(file_path);
        for (int i = 0; NULL != environ[i]; ++i)
            file << environ[i] << '\0';
    }
#endif

//########################################
int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        if (0 == strcmp("-join", argv[1]))
            wait_for_seconds(argv[2]);
        else if (0 == strcmp("-env", argv[1]))
            dump_env(argv[2]);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
