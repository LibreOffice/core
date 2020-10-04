/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * This is a tool that will be useful for various tasks if/when we build LO on WSL
 *
 * It is a Win32 program, not a Linux (WSL) one.
 *
 * Compile as: cl -MD wsl-lo-helper.cpp advapi32.lib
 */

#include <cstdio>
#include <cstring>

#include <Windows.h>

static void print_result(const wchar_t* argv0, const wchar_t* result)
{
    char output[1000];
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, result, -1, output, sizeof(output), NULL,
                            NULL)
        == 0)
    {
        fprintf(stderr, "%S --8.3: Could not convert result to UTF-8.\n", argv0);
        exit(1);
    }

    // Intentionally output no newline at end
    printf("%s", output);
    fflush(stdout);
}

int wmain(int argc, wchar_t** argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "%S: Usage:\n", argv[0]);
        fprintf(stderr, "%S --8.3 <windows-pathname>\n", argv[0]);
        fprintf(stderr, "    Print the 8.3 form of a Windows pathnam. Fail it not present.\n");
        fprintf(stderr, "%S --read-registry [32|64] <path>\n", argv[0]);
        fprintf(stderr, "    Read a string value from HKLM in the Registry and print it.\n");
        exit(1);
    }

    if (wcscmp(argv[1], L"--8.3") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "%S --8.3: One pathname argument expected\n", argv[0]);
            exit(1);
        }

        // The argument should be a pathname in Windows format. The
        // output will be the 8.3 pathname if present. If not present,
        // return failure.

        wchar_t woutput[1000];
        if (GetShortPathNameW(argv[2], woutput, sizeof(woutput) / sizeof(woutput[0])) == 0)
        {
            fprintf(stderr, "%S --8.3: Could not get short pathname.\n", argv[0]);
            exit(1);
        }

        print_result(argv[0], woutput);
    }
    else if (wcscmp(argv[1], L"--read-registry") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "%S --read-registry: Bitness and path arguments expected.\n", argv[0]);
            exit(1);
        }

        REGSAM sam;
        if (wcscmp(argv[2], L"32") == 0)
            sam = KEY_WOW64_32KEY;
        else if (wcscmp(argv[2], L"64") == 0)
            sam = KEY_WOW64_64KEY;
        else
        {
            fprintf(stderr, "%S --read-registry: Use 32 or 64.\n", argv[0]);
            exit(1);
        }

        wchar_t* path = wcsdup(argv[3]);

        for (wchar_t* p = path; *p != L'\0'; p++)
            if (*p == '/')
                *p = '\\';

        DWORD type;
        wchar_t result[1000];
        DWORD count = sizeof(result);

        wchar_t* last_backslash = wcsrchr(path, L'\\');
        if (last_backslash == NULL)
        {
            fprintf(stderr, "%S: Invalid path to value in the Registry.\n", argv[0]);
            exit(1);
        }

        *last_backslash = L'\0';
        wchar_t* value = last_backslash + 1;

        HKEY key;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE | sam, &key)
            != ERROR_SUCCESS)
        {
            fprintf(stderr, "%S: Opening key %S in %S-bit Registry failed.\n", argv[0], path,
                    argv[2]);
            exit(1);
        }

        if (RegQueryValueExW(key, value, NULL, &type, (LPBYTE)result, &count) != ERROR_SUCCESS)
        {
            fprintf(stderr, "%S: Reading value %S\\%S from %S-bit Registry failed.\n", argv[0],
                    path, value, argv[2]);
            exit(1);
        }
        if (type != REG_SZ)
        {
            fprintf(stderr, "%S: Value %S\\%S is not a string.\n", argv[0], path, value);
            exit(1);
        }
        print_result(argv[0], result);
    }
    else
    {
        fprintf(stderr, "%S: Unrecognized sub-command %S.\n", argv[0], argv[1]);
        exit(1);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
