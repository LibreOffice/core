/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

//########################################
// includes

#ifdef WNT // Windows
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#else
#include <unistd.h>
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

