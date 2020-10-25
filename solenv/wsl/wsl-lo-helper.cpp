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

static REGSAM parse_bitness(const wchar_t* argv0, const wchar_t* command, const wchar_t* bitness)
{
    if (wcscmp(bitness, L"32") == 0)
        return KEY_WOW64_32KEY;
    else if (wcscmp(bitness, L"64") == 0)
        return KEY_WOW64_64KEY;
    else
    {
        fprintf(stderr, "%S %S: Use 32 or 64.\n", argv0, command);
        exit(1);
    }
}

static void parse_hive(const wchar_t* argv0, const wchar_t* path, HKEY* hivep,
                       const wchar_t** hive_namep)
{
    if (wcscmp(path, L"HKEY_CLASSES_ROOT") == 0)
    {
        *hivep = HKEY_CLASSES_ROOT;
        *hive_namep = L"HKEY_CLASSES_ROOT";
    }
    else if (wcscmp(path, L"HKEY_CURRENT_CONFIG") == 0)
    {
        *hivep = HKEY_CURRENT_CONFIG;
        *hive_namep = L"HKEY_CURRENT_CONFIG";
    }
    else if (wcscmp(path, L"HKEY_CURRENT_USER") == 0)
    {
        *hivep = HKEY_CURRENT_USER;
        *hive_namep = L"HKEY_CURRENT_USER";
    }
    else if (wcscmp(path, L"HKEY_LOCAL_MACHINE") == 0)
    {
        *hivep = HKEY_LOCAL_MACHINE;
        *hive_namep = L"HKEY_LOCAL_MACHINE";
    }
    else if (wcscmp(path, L"HKEY_USERS") == 0)
    {
        *hivep = HKEY_USERS;
        *hive_namep = L"HKEY_USERS";
    }
    else
    {
        fprintf(stderr, "%S: Invalid Registry hive %S.\n", argv0, path);
        exit(1);
    }
}

int wmain(int argc, wchar_t** argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "%S: Usage:\n", argv[0]);
        fprintf(stderr, "%S --8.3 <windows-pathname>\n", argv[0]);
        fprintf(stderr, "    Print the 8.3 form of a Windows pathnam. Fail it not present.\n");
        fprintf(stderr, "%S --read-registry [32|64] <path>\n", argv[0]);
        fprintf(stderr, "    Read a string value from the Registry and print it.\n");
        fprintf(stderr, "%S --list-registry [32|64] <path>\n", argv[0]);
        fprintf(stderr, "    List subkeys of a key in the Registry.\n");
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
            fprintf(stderr, "%S --8.3: Could not get short pathname of %S.\n", argv[0], argv[2]);
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

        REGSAM sam = parse_bitness(argv[0], L"--read-registry", argv[2]);

        wchar_t* path = wcsdup(argv[3]);

        for (wchar_t* p = path; *p != L'\0'; p++)
            if (*p == '/')
                *p = '\\';

        wchar_t* const first_backslash = wcschr(path, L'\\');
        if (first_backslash == NULL)
        {
            fprintf(stderr, "%S: Invalid path %S to value in the Registry.\n", argv[0], path);
            exit(1);
        }

        *first_backslash = L'\0';
        wchar_t* const key_path = first_backslash + 1;

        HKEY hive;
        const wchar_t* hive_name;
        parse_hive(argv[0], path, &hive, &hive_name);

        DWORD type;
        wchar_t result[1000];
        DWORD count = sizeof(result);

        wchar_t* last_backslash = wcsrchr(key_path, L'\\');
        if (last_backslash == NULL)
        {
            fprintf(stderr, "%S: Invalid path %S\\%S to value in the Registry.\n", argv[0],
                    hive_name, key_path);
            exit(1);
        }

        *last_backslash = L'\0';
        wchar_t* value = last_backslash + 1;

        HKEY key;
        if (RegOpenKeyExW(hive, key_path, 0, KEY_QUERY_VALUE | sam, &key) != ERROR_SUCCESS)
        {
            fprintf(stderr, "%S: Opening key %S\\%S in %S-bit Registry failed.\n", argv[0],
                    hive_name, key_path, argv[2]);
            exit(1);
        }

        if (RegQueryValueExW(key, value, NULL, &type, (LPBYTE)result, &count) != ERROR_SUCCESS)
        {
            fprintf(stderr, "%S: Reading value %S\\%S\\%S from %S-bit Registry failed.\n", argv[0],
                    hive_name, key_path, value, argv[2]);
            exit(1);
        }
        if (type != REG_SZ)
        {
            fprintf(stderr, "%S: Value %S\\%S\\%S is not a string.\n", argv[0], hive_name, key_path,
                    value);
            exit(1);
        }
        print_result(argv[0], result);
    }
    else if (wcscmp(argv[1], L"--list-registry") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "%S --list-registry: Bitness and path arguments expected.\n", argv[0]);
            exit(1);
        }

        REGSAM sam = parse_bitness(argv[0], L"--list-registry", argv[2]);

        wchar_t* path = wcsdup(argv[3]);

        for (wchar_t* p = path; *p != L'\0'; p++)
            if (*p == '/')
                *p = '\\';

        wchar_t* const first_backslash = wcschr(path, L'\\');
        if (first_backslash == NULL)
        {
            fprintf(stderr, "%S: Invalid path %S to key in the Registry.\n", argv[0], path);
            exit(1);
        }

        *first_backslash = L'\0';
        wchar_t* const key_path = first_backslash + 1;

        HKEY hive;
        const wchar_t* hive_name;
        parse_hive(argv[0], path, &hive, &hive_name);

        HKEY key;
        if (RegOpenKeyExW(hive, key_path, 0, KEY_ENUMERATE_SUB_KEYS | sam, &key) != ERROR_SUCCESS)
        {
            fprintf(stderr, "%S: Opening key %S\\%S in %S-bit Registry failed.\n", argv[0],
                    hive_name, key_path, argv[2]);
            exit(1);
        }

        DWORD index = 0;
        wchar_t name[256];
        DWORD namelen = sizeof(name) / sizeof(name[0]);
        while (RegEnumKeyExW(key, index, name, &namelen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            printf("%S\n", name);
            index++;
            namelen = sizeof(name) / sizeof(name[0]);
        }
    }
    else
    {
        fprintf(stderr, "%S: Unrecognized sub-command %S.\n", argv[0], argv[1]);
        exit(1);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
