/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msi.h>
#include <msiquery.h>
#include <msidefs.h>
#include <wchar.h>

#define WINE_NO_TRACE_MSGS 1
#define WINE_NO_DEBUG_MSGS 1

#include "wine/unicode.h"
#include "wine/debug.h"
#include "wtypes.h"

#define MAX_TABLES 50
#define MAX_STREAMS 10
#define MAX_STORAGES 10
#define MAX_TABLE_NAME 50
#define MAX_STREAM_NAME 62
#define MAX_STORAGE_NAME 31
#define COMMANDS 17

static const unsigned commandMap[COMMANDS][4] =
{{'c', PID_CODEPAGE, VT_I2},
 {'t', PID_TITLE, VT_LPSTR},
 {'j', PID_SUBJECT, VT_LPSTR},
 {'a', PID_AUTHOR, VT_LPSTR},
 {'k', PID_KEYWORDS, VT_LPSTR},
 {'o', PID_COMMENTS, VT_LPSTR},
 {'p', PID_TEMPLATE, VT_LPSTR},
 {'l', PID_LASTAUTHOR, VT_LPSTR},
 {'v', PID_REVNUMBER, VT_LPSTR},
 {'s', PID_LASTPRINTED, VT_FILETIME},
 {'r', PID_CREATE_DTM, VT_FILETIME},
 {'q', PID_LASTSAVE_DTM, VT_FILETIME},
 {'g', PID_PAGECOUNT, VT_I4},
 {'w', PID_WORDCOUNT, VT_I4},
 {'h', PID_CHARCOUNT, VT_I4},
 {'n', PID_APPNAME, VT_LPSTR},
 {'u', PID_SECURITY, VT_I4}};

static const char * commandNames[COMMANDS] =
{"Codepage", "Title", "Subject", "Author", "Keywords", "Comments",
 "Template", "Last Saved By", "Revision Number", "Last Printed",
 "Create Time/Date", "Last Save Time/Date", "Page Count", "Word Count",
 "Character Count", "Creating Application", "Security"};

static void parseDate(LPCWSTR value, SYSTEMTIME *time)
{
    LPWSTR field;
    unsigned len = 4*sizeof(WCHAR);

    if (lstrlenW(value) != 19) return;
    field = malloc(len + sizeof(WCHAR));
    if (field == NULL) return;
    memcpy(field, value, len);
    field[4] = L'\0';
    time->wYear = atoiW(field);

    len = 2 * sizeof(WCHAR);
    memcpy(field, &(value[5]), len);
    field[2] = '\0';
    time->wMonth = atoiW(field);

    memcpy(field, &(value[8]), len);
    time->wDay = atoiW(field);

    memcpy(field, &(value[11]), len);
    time->wHour = atoiW(field);

    memcpy(field, &(value[14]), len);
    time->wMinute = atoiW(field);

    memcpy(field, &(value[17]), len);
    time->wSecond = atoiW(field);

    free(field);
}

static BOOL msiinfoDisplayProperties(LPWSTR dbfile)
{
    MSIHANDLE dbhandle, infohandle;
    unsigned i, r, dataType;
    INT iVal;
    FILETIME ftVal;
    SYSTEMTIME sysTime;
    LPWSTR szVal = NULL;
    DWORD size;
    r = MsiOpenDatabaseW(dbfile, (LPWSTR) MSIDBOPEN_READONLY, &dbhandle);
    if (r != ERROR_SUCCESS)
    {
        return FALSE;
    }

    r = MsiGetSummaryInformationW(dbhandle, 0, 0, &infohandle);
    if (r != ERROR_SUCCESS)
    {
        return FALSE;
    }
    for (i = 0; i < COMMANDS; i++)
    {
        MsiSummaryInfoGetPropertyW(infohandle, commandMap[i][1], &dataType,
                                   &iVal, &ftVal, szVal, &size);

        wprintf(L"%-24s", commandNames[i]);
        if (dataType == VT_LPSTR)
        {
            if (szVal!=NULL) wprintf(L"%ls (%d)\n", szVal, lstrlenW(szVal));
            else wprintf(L"\n");
        }
        else if (dataType == VT_FILETIME)
        {
            FileTimeToSystemTime(&ftVal, &sysTime);
            wprintf(L"%04d/%02d/%02d %02d:%02d:%02d\n", sysTime.wYear, sysTime.wMonth,
                    sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
        }
        else
        {
            wprintf(L"%d\n", iVal);
        }
    }

    MsiCloseHandle(infohandle);
    return TRUE;
}

static BOOL msiinfoUpdateProperty(MSIHANDLE infoHandle, unsigned property, unsigned dataType, LPCWSTR value)
{
    unsigned r;
    int iVal = 0;
    FILETIME ftVal;
    SYSTEMTIME sysTime = {0};
    LPCWSTR szVal = NULL;

    if (dataType == VT_LPSTR) szVal = value;
    else if (dataType == VT_FILETIME)
    {
        parseDate(value, &sysTime);
        SystemTimeToFileTime(&sysTime, &ftVal);
    }
    else
        iVal = atoiW(value);

    r = MsiSummaryInfoSetPropertyW(infoHandle, property, dataType, iVal, &ftVal, szVal);
    if (r != ERROR_SUCCESS)
    {
        wprintf(L"Problem updating property: %d %d %d %d\n", r == ERROR_DATATYPE_MISMATCH, r == ERROR_FUNCTION_FAILED, ERROR_UNKNOWN_PROPERTY, ERROR_UNSUPPORTED_TYPE);
        return FALSE;
    }

    return TRUE;
}

static void usage(void)
{
    wprintf(
        L"Usage: msiinfo {database} [[-b]-d] {options} {data}\n"
        L"\nOptions:\n"
        L"  -c <cp>       Specify codepage\n"
        L"  -t <title>    Specify title\n"
        L"  -j <subject>  Specify subject\n"
        L"  -a <author>   Specify author\n"
        L"  -k <keywords> Specify keywords\n"
        L"  -o <comment>  Specify comments\n"
        L"  -p <template> Specify template\n"
        L"  -l <author>   Specify last author\n"
        L"  -v <revno>    Specify revision number\n"
        L"  -s <date>     Specify last printed date\n"
        L"  -r <date>     Specify creation date\n"
        L"  -q <date>     Specify date of last save\n"
        L"  -g <pages>    Specify page count\n"
        L"  -w <words>    Specify word count\n"
        L"  -h <chars>    Specify character count\n"
        L"  -n <appname>  Specify application which created the database\n"
        L"  -u <security> Specify security (0: none, 2: read only (rec.) 3: read only (enforced)\n");
}

int wmain(int argc, WCHAR *argv[])
{
    WCHAR *dbfile = NULL;
    unsigned i = 0;
    MSIHANDLE dbhandle, infohandle;
    unsigned r;
    LPWSTR value = 0;
    static const WCHAR h1[] = {'/', '?', 0};
    static const WCHAR h2[] = {'-', '?', 0};

    if (argc > 1)
    {
        dbfile = argv[1];
        argv++; argc--;
    }

    if (!dbfile || strcmpW(dbfile, h1) == 0 || strcmpW(dbfile, h2) == 0)
    {
        usage();
        return 0;
    }
    else if (argc == 1)
    {
        msiinfoDisplayProperties(dbfile);
        return 0;
    }

    r = MsiOpenDatabaseW(dbfile, (LPWSTR) MSIDBOPEN_TRANSACT, &dbhandle);
    if (r != ERROR_SUCCESS) return 1;
    r = MsiGetSummaryInformationW(dbhandle, 0, 20, &infohandle);
    if (r != ERROR_SUCCESS) return 2;

    while (argv[1] && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case '?':
        case 'h':
            usage();
            return 0;
        default:
            for (i = 0; i < COMMANDS; i++)
            {
                if (commandMap[i][0] == argv[1][1])
                {
                    argv++; argc--;
                    value = argv[1];
                    msiinfoUpdateProperty(infohandle, commandMap[i][1], commandMap[i][2], value);
                    break;
                }
            }
            break;
        }
        argv++; argc--;
    }

    MsiSummaryInfoPersist(infohandle);
    MsiDatabaseCommit(dbhandle);
    MsiCloseHandle(dbhandle);
    msiinfoDisplayProperties(dbfile);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
