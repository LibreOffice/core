/*************************************************************************
 *
 *  $RCSfile: osl_process_child.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:41:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//########################################
// includes

#ifdef WNT
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

#include <rtl/ustring.hxx>

//########################################
// defines

#ifdef WNT
#   define SLEEP(t) (Sleep((t)*1000))
#else
#   define SLEEP(t) (sleep((t)))
#endif

//########################################
void wait(char* time)
{
    SLEEP(atoi(time));
}

//########################################
void w_to_a(LPCTSTR strW, LPSTR strA, DWORD size)
{
    WideCharToMultiByte(CP_ACP, 0, strW, -1, strA, size, NULL, NULL);
}

//########################################

#ifdef WNT
    void dump_env(char* file_path)
    {
        LPTSTR env = reinterpret_cast<LPTSTR>(
            GetEnvironmentStrings());
        LPTSTR p   = env;

        std::ofstream file(file_path);

        char buffer[32767];
        while (size_t l = _tcslen(p))
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

    printf("Parameter: ");
    for (int i = 1; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    if (argc > 2)
    {
        if (0 == stricmp("-join", argv[1]))
        {
            wait(argv[2]);
        }
        else if (0 == stricmp("-env", argv[1]))
        {
            dump_env(argv[2]);
        }
    }

    return (0);
}

