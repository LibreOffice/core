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
 * So far just one function: Convert a full Windows pathname to 8.3 format (if present). WSL does
 * not seem to have that functionality built-in, and doing it through some invocations of cmd.exe or
 * a .bat file from WSL seems very complex.
 */

#include <cstdio>
#include <cstring>

#include <Windows.h>

int wmain(int argc, wchar_t** argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "%S: Arguments expected\n", argv[0]);
        return 1;
    }

    if (wcscmp(argv[1], L"--8.3") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "%S --8.3: One pathname argument expected\n", argv[0]);
            return 1;
        }

        // The argument should be a pathname in Windows format. The
        // output will be the 8.3 pathname if present. If not present,
        // return failure.

        wchar_t woutput[1000];
        if (GetShortPathNameW(argv[2], woutput, sizeof(woutput) / sizeof(woutput[0])) == 0)
        {
            fprintf(stderr, "%S --8.3: Could not get short pathname\n", argv[0]);
            return 1;
        }

        char output[1000];
        if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, woutput, -1, output, sizeof(output),
                                NULL, NULL)
            == 0)
        {
            fprintf(stderr, "%S --8.3: Could not convert result to UTF-8\n", argv[0]);
            return 1;
        }

        // Intentionally output no newline at end
        printf("%s", output);
        fflush(stdout);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
