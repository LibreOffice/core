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

#include "wine/unicode.h"

#define MAX_TABLES 50
#define MAX_STREAMS 10
#define MAX_STORAGES 10
#define MAX_TABLE_NAME 50
#define MAX_STREAM_NAME 62
#define MAX_STORAGE_NAME 31

static const WCHAR PATH_DELIMITERW[] = {'/', 0};
static const char PATH_DELIMITER[] = {'/', 0};


static WCHAR *strdupAtoW( UINT cp, const char *str )
{
    WCHAR *ret = NULL;
    if (str)
    {
        DWORD len = MultiByteToWideChar( cp, 0, str, -1, NULL, 0 );
        if ((ret = malloc( len * sizeof(WCHAR) )))
            MultiByteToWideChar( cp, 0, str, -1, ret, len );
    }
    return ret;
}

static char *strdupWtoA( UINT cp, const WCHAR *str )
{
    char *ret = NULL;
    if (str)
    {
        DWORD len = WideCharToMultiByte(cp, 0, str, -1, NULL, 0, NULL, NULL);
        if ( (ret = malloc(len)) )
            WideCharToMultiByte(cp, 0, str, -1, ret, len, NULL, NULL);
    }
    return ret;
}

static BOOL msidbImportStorages(LPCWSTR dbfile, LPCWSTR wdir, LPWSTR storageNames[])
{
    static const WCHAR delim[] = {'/', 0};
    UINT r, len, i;
    MSIHANDLE dbhandle, view, rec;
    WCHAR *storagePath = 0;
    char *query = "INSERT INTO _Storages (Name, Data) VALUES(?, ?)";

    r = MsiOpenDatabaseW(dbfile, (LPWSTR) MSIDBOPEN_TRANSACT, &dbhandle);
    if (r != ERROR_SUCCESS) return FALSE;

    r = MsiDatabaseOpenView(dbhandle, query, &view);
    if (r != ERROR_SUCCESS) return FALSE;
    MsiViewExecute(view, 0);
    r = MsiViewFetch(view, &rec);
    if (r != ERROR_SUCCESS) return FALSE;

    for (i = 0; i < MAX_STORAGES && storageNames[i] != 0; ++i)
    {
        len = lstrlenW(wdir) + lstrlenW(storageNames[i]) + 2;
        storagePath = malloc(len * sizeof(WCHAR));
        if (storagePath == NULL) return FALSE;

        lstrcpyW(storagePath, wdir);
        lstrcatW(storagePath, delim);
        lstrcatW(storagePath, storageNames[i]);

        rec = MsiCreateRecord(2);
        MsiRecordSetStringW(rec, 1, storageNames[i]);
        r = MsiRecordSetStreamW(rec, 2, storagePath);
        if (r != ERROR_SUCCESS)
        {
            return FALSE;
        }

        r = MsiViewExecute(view, rec);
        if (r != ERROR_SUCCESS)
        {
            return FALSE;
        }

        MsiCloseHandle(rec);
    }

    MsiViewClose(view);
    MsiCloseHandle(view);
    MsiDatabaseCommit(dbhandle);
    MsiCloseHandle(dbhandle);
    return TRUE;
}

static BOOL msidbExportStorage(LPCWSTR dbfile, LPCWSTR wdir, LPWSTR storageName)
{
    UINT r, len;
    MSIHANDLE dbhandle, view, rec;
    char queryBuffer[100];
    char *storageNameA = strdupWtoA(CP_ACP, storageName);
    char *wdirA = strdupWtoA(CP_ACP, wdir);
    char *storagePath = NULL;
    char *dataBuffer;
    DWORD dataLen = 0;
    FILE *fp = NULL;
    sprintf(queryBuffer, "SELECT * FROM _Storages WHERE Name = '%s'", storageNameA);

    r = MsiOpenDatabaseW(dbfile, (LPWSTR) MSIDBOPEN_READONLY, &dbhandle);
    if (r != ERROR_SUCCESS) return FALSE;

    MsiDatabaseOpenView(dbhandle, queryBuffer, &view);
    MsiViewExecute(view, 0);
    r = MsiViewFetch(view, &rec);
    if (r != ERROR_SUCCESS) return FALSE;

    if ((r = MsiRecordReadStream(rec, 2, 0, &dataLen)) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if ((dataBuffer = malloc(dataLen)) == NULL) return FALSE;
    if (MsiRecordReadStream(rec, 2, dataBuffer, &dataLen) != ERROR_SUCCESS) return FALSE;

    len = strlen(wdirA) + strlen(storageNameA) + 2;
    storagePath = malloc(len * sizeof(WCHAR));
    if (storagePath == NULL) return FALSE;

    strcpy(storagePath, wdirA);
    strcat(storagePath, "/");
    strcat(storagePath, storageNameA);

    fp = fopen(storagePath , "wb");
    if (fp != NULL)
    {
        fwrite(dataBuffer, 1, dataLen, fp);
    }
    fclose(fp);

    free(storagePath);
    MsiCloseHandle(rec);

    MsiViewClose(view);
    MsiCloseHandle(view);
    MsiCloseHandle(dbhandle);
    free(storageNameA);
    free(wdirA);
    return TRUE;
}

static BOOL msidbImportStreams(LPCWSTR dbfile, LPCWSTR wdir, LPWSTR streamNames[])
{
    UINT r, len;
    MSIHANDLE dbhandle, view, rec;
    int i = 0;
    WCHAR *streamPath = 0;
    char *query = "INSERT INTO _Streams (Name, Data) VALUES(?, ?)";

    r = MsiOpenDatabaseW(dbfile, (LPWSTR) MSIDBOPEN_TRANSACT, &dbhandle);
    if (r != ERROR_SUCCESS) return FALSE;

    r = MsiDatabaseOpenView(dbhandle, query, &view);
    if (r != ERROR_SUCCESS) return FALSE;

    for (i = 0; i < MAX_STREAMS && streamNames[i] != NULL; i++)
    {
        len = lstrlenW(wdir) + lstrlenW(streamNames[i]) + 2;
        streamPath = malloc(len * sizeof(WCHAR));
        if (streamPath == NULL) return FALSE;

        lstrcpyW(streamPath, wdir);
        lstrcatW(streamPath, PATH_DELIMITERW);
        lstrcatW(streamPath, streamNames[i]);

        rec = MsiCreateRecord(2);
        MsiRecordSetStringW(rec, 1, streamNames[i]);
        r = MsiRecordSetStreamW(rec, 2, streamPath);
        if (r != ERROR_SUCCESS)
        {
            return FALSE;
        }

        r = MsiViewExecute(view, rec);
        if (r != ERROR_SUCCESS)
        {
            return FALSE;
        }

        MsiCloseHandle(rec);
    }

    MsiViewClose(view);
    MsiCloseHandle(view);
    MsiDatabaseCommit(dbhandle);
    MsiCloseHandle(dbhandle);
    return TRUE;
}

/***********************************************************************
 * msidbExportStream
 *
 * Exports a stream to a file with an .idb extension.
 *
 * Examples (note wildcard escape for *nix/bash):
 * msidb -d <pathtomsi>.msi -f <workdir> -x <streamname>
 * msidb -d <pathtomsi>.msi -f <workdir> -x data.cab
 **********************************************************************/
static BOOL msidbExportStream(LPCWSTR dbfile, LPCWSTR wdir, LPCWSTR streamName)
{
    static const char ext[] = {'.', 'i', 'd', 'b', 0};
    UINT r, len;
    MSIHANDLE dbhandle, streamListView, rec;
    char queryBuffer[100];
    FILE *fp = 0;
    char *streamNameA = strdupWtoA(CP_ACP, streamName);
    char *wdirA = strdupWtoA(CP_ACP, wdir);
    char *streamFileA = 0;
    char streamPath[MAX_PATH];
    char *dataBuffer;
    DWORD dataLen = 0;

    r = MsiOpenDatabaseW(dbfile, (LPCWSTR) MSIDBOPEN_READONLY, &dbhandle);
    if (r != ERROR_SUCCESS) return FALSE;

    sprintf(queryBuffer, "SELECT * FROM _Streams WHERE Name = '%s'", streamNameA);
    MsiDatabaseOpenView(dbhandle, queryBuffer, &streamListView);
    MsiViewExecute(streamListView, 0);
    r = MsiViewFetch(streamListView, &rec);
    if (r != ERROR_SUCCESS) return FALSE;

    if (MsiRecordReadStream(rec, 2, 0, &dataLen) != ERROR_SUCCESS)
        return FALSE;
    dataBuffer = malloc(dataLen);
    if (!dataBuffer) return FALSE;
    if (MsiRecordReadStream(rec, 2, dataBuffer, &dataLen) != ERROR_SUCCESS)
        return FALSE;

    len = strlen(streamNameA) + 5;
    streamFileA = malloc(len);
    if (streamFileA == NULL) return FALSE;

    strcpy(streamFileA, streamNameA);
    strcat(streamFileA, ext);

    strcpy(streamPath, wdirA);
    strcat(streamPath, PATH_DELIMITER);
    strcat(streamPath, streamFileA);

    fp = fopen(streamPath , "wb");
    if (fp != NULL)
    {
        fwrite(dataBuffer, 1, dataLen, fp);
    }
    fclose(fp);

    free(streamFileA);
    MsiCloseHandle(rec);

    MsiViewClose(streamListView);
    MsiCloseHandle(streamListView);
    MsiCloseHandle(dbhandle);
    free(wdirA);
    free(streamNameA);
    return TRUE;
}

/***********************************************************************
 * msidbImportTables
 *
 * Takes a list of tables or '*' (for all) to import from text archive
 * files in specified folder
 *
 * For each table, a file called <tablename>.idt is imported containing
 * tab separated ASCII.
 *
 * Examples (note wildcard escape for *nix/bash):
 * msidb -d <pathtomsi>.msi -f <workdir> -i \*
 * msidb -d <pathtomsi>.msi -f <workdir> -i File Directory Binary
 **********************************************************************/
static BOOL msidbImportTables(LPCWSTR dbfile, LPCWSTR wdir, LPWSTR tables[], BOOL create)
{
    static const WCHAR ext[] = {'.', 'i', 'd', 't', 0};
    static const WCHAR all[] = {'*', 0};

    UINT r, len;
    char *dirNameA;
    char *fileName;
    DIR *dir;
    struct dirent *ent;

    int i = 0;
    MSIHANDLE dbhandle;

    LPWSTR tableFile = 0;
    LPCWSTR oFlag = (LPCWSTR) MSIDBOPEN_TRANSACT;

    if (create == TRUE) oFlag = (LPCWSTR) MSIDBOPEN_CREATE;
    r = MsiOpenDatabaseW(dbfile, oFlag, &dbhandle);

    if (r != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (strcmpW(tables[0], all) == 0)
    {
        dirNameA = strdupWtoA(CP_ACP, wdir);
        dir = opendir(dirNameA);

        if (dir)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_type != DT_REG) continue;
                fileName = ent->d_name;
                if (strcmp(fileName+strlen(fileName)-4*sizeof(fileName[0]), ".idt") != 0) continue;
                if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) continue;
                tableFile = strdupAtoW(CP_ACP, fileName);
                r = MsiDatabaseImportW(dbhandle, wdir, tableFile);
                free(tableFile);
            }
        }
        else
            return FALSE;
        closedir(dir);
        free(dirNameA);
    }
    else
    {
        for (i = 0; i < MAX_TABLES && tables[i] != 0; ++i)
        {
            len = lstrlenW(tables[i]) + 5;
            tableFile = malloc(len * sizeof (WCHAR));
            if (tableFile == NULL) return FALSE;

            lstrcpyW(tableFile, tables[i]);
            lstrcatW(tableFile, ext);
            r = MsiDatabaseImportW(dbhandle, wdir, tableFile);
            free(tableFile);

            if (r != ERROR_SUCCESS)
            {
                return FALSE;
            }
        }
    }

    MsiDatabaseCommit(dbhandle);
    MsiCloseHandle(dbhandle);
    return TRUE;
}

/***********************************************************************
 * msidbExportTables
 *
 * Takes a list of tables or '*' (for all) to export to text archive
 * files in specified folder
 *
 * For each table, a file called <tablename>.idt is exported containing
 * tab separated ASCII.
 *
 * Examples (note wildcard escape for *nix/bash):
 * msidb -d <pathtomsi>.msi -f <workdir> -e \*
 * msidb -d <pathtomsi>.msi -f <workdir> -e File Directory Binary
 **********************************************************************/
static BOOL msidbExportTables(LPCWSTR dbfile, LPCWSTR wdir, LPWSTR tables[])
{
    static const WCHAR ext[] = {'.', 'i', 'd', 't', 0};
    static const WCHAR all[] = {'*', 0};
    UINT r, len;
    MSIHANDLE dbhandle, tableListView, rec;
    LPWSTR tableFile = 0;
    WCHAR tableName[MAX_TABLE_NAME];
    DWORD size = sizeof(tableName) / sizeof(tableName[0]);
    int i = 0;

    r = MsiOpenDatabaseW(dbfile, (LPCWSTR) MSIDBOPEN_READONLY, &dbhandle);

    if (r != ERROR_SUCCESS) return FALSE;

    if (strcmpW(tables[0], all) == 0)
    {
        r = MsiDatabaseOpenView(dbhandle, "SELECT Name FROM _Tables", &tableListView);
        r = MsiViewExecute(tableListView, 0);
        r = MsiViewFetch(tableListView, &rec);

        while (r == ERROR_SUCCESS)
        {
            size = sizeof(tableName) / sizeof(tableName[0]);
            r = MsiRecordGetStringW(rec, 1, tableName, &size);
            if (r == ERROR_SUCCESS)
            {
                len = lstrlenW(tableName) + 5;
                tableFile = malloc(len * sizeof (WCHAR));
                if (tableFile == NULL) return FALSE;

                lstrcpyW(tableFile, tableName);
                lstrcatW(tableFile, ext);

                r = MsiDatabaseExportW(dbhandle, tableName, wdir, tableFile);

                free(tableFile);
                MsiCloseHandle(rec);
            }

            r = MsiViewFetch(tableListView, &rec);
        }

        MsiViewClose(tableListView);
        MsiCloseHandle(tableListView);
    }
    else
    {
        for (i = 0; i < MAX_TABLES && tables[i] != 0; ++i)
        {
            len = lstrlenW(tables[i]) + 5;
            tableFile = malloc(len * sizeof (WCHAR));
            if (tableFile == NULL) return FALSE;

            lstrcpyW(tableFile, tables[i]);
            lstrcatW(tableFile, ext);
            r = MsiDatabaseExportW(dbhandle, tables[i], wdir, tableFile);

            free(tableFile);
        }
    }

    MsiCloseHandle(dbhandle);
    return TRUE;
}

static void usage( void )
{
    printf(
        "Usage: msidb [options] [tables]\n"
        "\nOptions:\n"
        "  -d <path>    Fully qualified path to MSI database file\n"
        "  -f <wdir>    Path to the text archive folder\n"
        "  -c           Create or overwrite with new database and import tables\n"
        "  -i <tables>  Import tables from text archive files - "
        "use * for all\n"
        "  -e <tables>  Export tables to files archive in directory - "
        "use * for all\n"
        "  -x <stream>  Saves stream as <stream>.idb in <wdir>\n"
        "  -a <file>    Adds stream from file to database\n"
        "  -r <storage> Adds storage to database as substorage\n");
}

int wmain(int argc, WCHAR *argv[])
{
    LPWSTR dbfile, streamName, storageName;
    LPWSTR wdir = (LPWSTR) L".";
    LPWSTR streamFiles[MAX_STREAMS] = {0};
    LPWSTR storageNames[MAX_STORAGES] = {0};
    LPWSTR iTables[MAX_TABLES] = {0};
    LPWSTR oTables[MAX_TABLES] = {0};
    BOOL createdb = FALSE;
    int i = 0;
    int currStream = 0;
    int currStorage = 0;
    dbfile = streamName = storageName = NULL;

    while (argv[1] && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
        switch (tolower(argv[1][1]))
        {
        case 'i':                   /* Import tables */
            i = 0;
            while (argv[2] && argv[2][0] != '-' && i < 10)
            {
                iTables[i] = argv[2];
                argv++; argc--; i++;
            }
            break;
        case 'e':                   /* Export tables */
            i = 0;
            while (argv[2] && argv[2][0] != '-' && i < 10)
            {
                oTables[i] = argv[2];
                argv++; argc--; i++;
            }
            break;
        case 'c':                   /* Create/ovrwrt .msi db */
            createdb = TRUE;
            break;
        case 'f':                   /* Specify working dir */
            argv++; argc--;
            wdir = argv[1];
            break;
        case 'd':                   /* Specify .msi db */
            argv++; argc--;
            dbfile = argv[1];
            break;
        case 'x':                  /* Save stream to .ibd */
            argv++; argc--;
            streamName = argv[1];
            break;
        case 'a':                  /* Add stream(s) to db */
            argv++; argc--;
            if (currStream < MAX_STREAMS)
                streamFiles[currStream++] = argv[1];
            break;
        case 'w':
            argv++; argc--;
            storageName = argv[1];
            break;
        case 'r':                  /* Add strg as substrg to db */
            argv++; argc--;
            if (currStorage < MAX_STORAGES)
                storageNames[currStorage++] = argv[1];
            break;
        case '?':
        case 'h':
            usage();
            return 0;
        }
        argv++; argc--;
    }

    if (iTables[0])
        if (!msidbImportTables(dbfile, wdir, iTables, createdb))
            return 1;

    if (oTables[0])
        if (!msidbExportTables(dbfile, wdir, oTables))
            return 2;

    if (streamName)
        if (!msidbExportStream(dbfile, wdir, streamName))
            return 3;

    if (streamFiles[0])
        if (!msidbImportStreams(dbfile, wdir, streamFiles))
            return 4;

    if(storageNames[0])
        if (!msidbImportStorages(dbfile, wdir, storageNames))
            return 5;

    if (storageName)
        if (!msidbExportStorage(dbfile, wdir, storageName))
            return 6;

    if (!iTables[0] && !oTables[0] && !streamName && !streamFiles[0] && !storageNames[0] && !storageName)
        return 7;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
