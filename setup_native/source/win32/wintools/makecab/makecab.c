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
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "parseddf.h"

#define _O_RDONLY 0
#define _O_WRONLY 1
#define _O_RDWR 2
#define _O_CREAT 0x0100

#define _A_RDONLY 1
#define _A_HIDDEN 2
#define _A_SYSTEM 4
#define _A_ARCH 0x20

typedef enum CABLOGLEVEL { CABLOG_ERR = 1, CABLOG_WRN, CABLOG_MSG } CABLOGLEVEL;
typedef enum CABERR { CAB_OK, CABERR_DDF_UNOPENED } CABERR;
static unsigned int CabVerb = 0;
static const char * FCI_ERRS[] = {"None", "Error opening source file", "Error reading source file", "Error allocating memory", "Error creating temporary file", "Unknown compression type specified", "Error creating cabinet file", "Compression was manually aborted", "Error compressing data", "Cabinet data size or file count limits exceeded"};

void cabLog(CABLOGLEVEL lvl, char * msg, ...)
{
    va_list args;
    if (CabVerb < lvl) return;

    switch (lvl)
    {
    case CABLOG_WRN: printf("[Warning] "); break;
    case CABLOG_ERR: printf("[Error] "); break;
    }

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
}

void cabLogErr(PERF erf, char * msg)
{
    if (!erf)
    {
        cabLog(CABLOG_ERR, "%s: An unknown problem occurred");
        return;
    }

    if (erf->erfOper >= 0)
    {
        if (erf->erfOper < (sizeof(FCI_ERRS)/sizeof(FCI_ERRS[0])))
            cabLog(CABLOG_ERR, "%s: %s", msg, FCI_ERRS[erf->erfOper]);
        else
            cabLog(CABLOG_ERR, "%s: Unknown error", msg);
    }
}

void cabLogCCAB(PCCAB cc)
{
    cabLog(CABLOG_MSG,
           "cb                 %ld\n"
           "cbFolderThresh     %ld\n"
           "cbReserveCFHeader  %d\n"
           "cbReserveCFFolder  %d\n"
           "cbReserveCFData    %d\n"
           "iCab               %d\n"
           "iDisk              %d\n"
           "setID              %d\n"
           "szDisk             %s\n"
           "szCab              %s\n"
           "szCabPath          %s\n",
           cc->cb, cc->cbFolderThresh, cc->cbReserveCFHeader,
           cc->cbReserveCFFolder, cc->cbReserveCFData, cc->iCab,
           cc->iDisk, cc->setID, cc->szDisk, cc->szCab, cc->szCabPath);
}

/***********************************************************************
 * Define FCI callbacks
 **********************************************************************/
FNFCIALLOC (fnMemAlloc) /*(ULONG cb)*/
{
    return malloc(cb);
}

FNFCIFREE (fnMemFree) /*(void HUGE *memory)*/
{
    free(memory);
}

FNFCIOPEN (fnOpen) /*(char *pszFile, int oflag, int pmode, int *err, void *pv)*/
{
    FILE * f = NULL;
    char * mode = "r+";
    printf("DEBUG: fnOpen file %s\n", pszFile);
    if (oflag & _O_WRONLY) mode = "w";
    else if (oflag & _O_RDWR) mode = "r+";
    else if (oflag & _O_RDONLY) mode = "r";

    if (oflag & _O_CREAT && oflag & _O_RDWR) mode = "w+";

    f = fopen(pszFile, mode);
    if (f == NULL)
    {
        cabLog(CABLOG_ERR, "Could not get handle to file %s", pszFile);
        *err = -1;
    }

    return (INT_PTR) f;
}

FNFCIREAD (fnRead) /*(INT_PTR hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)*/
{
    FILE * f = (FILE *)hf;
    size_t r = fread(memory, 1, cb, f);
    printf("DEBUG: fnRead\n");
    if (r < cb)
    {
        if (feof(f))
        {
            cabLog(CABLOG_WRN, "Reached EOF while reading file (%d chars remain of %d requested)", r, cb);
            return r;
        }
        else if (ferror(f))
        {
            cabLog(CABLOG_ERR, "Error while reading file");
        }
        *err = -1;
        return -1;
    }
    return r;
}

FNFCIWRITE (fnWrite) /*(INT_PTR hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)*/
{
    unsigned int c;
    printf("DEBUG: fnWrite\n");
    if ( !(hf && (c = fwrite(memory, 1, sizeof(memory), (FILE *)hf))) )
    {
        if(c < cb)
        {
            *err = -1;
            return -1;
        }
    }
    return cb;
}

FNFCICLOSE (fnClose) /*(INT_PTR hf, int FAR *err, void FAR *pv)*/
{
    printf("DEBUG: fnClose\n");
    if ( !(hf && fclose((FILE *)hf) != EOF) )
    {
        *err = -1;
        return -1;
    }
    return 0;
}

FNFCISEEK (fnSeek) /*(INT_PTR hf, long dist, int seektype, int FAR *err, void FAR *pv)*/
{
    printf("DEBUG: fnSeek\n");
    if (fseek((FILE *)hf, dist, seektype) != 0)
    {
        *err = -1;
        return -1;
    }
    return 0;
}

FNFCIDELETE (fnDelete) /*(LPSTR pszFile, int FAR *err, void FAR *pv)*/
{
    printf("DEBUG: fnDelete\n");
    if (remove(pszFile) != 0)
    {
        *err = -1;
        return -1;
    }
    return 0;
}

FNFCIGETTEMPFILE (fnGetTempFile) /*(char *pszTempName[bcount(cbTempName)], int cbTempName[range(>=, MAX_PATH)], void FAR *pv)*/
{
    BOOL success = FALSE;
    CHAR tempPath[L_tmpnam];
    char * r;
    printf("DEBUG: fnGetTempFile\n");
    r = tmpnam(tempPath);
    if (r != NULL)
    {
        success = TRUE;
        strcpy(pszTempName, tempPath);
        cbTempName = strlen(pszTempName);
    }
    return success;
}

FNFCIFILEPLACED (fnFilePlaced) /*(PCCAB *pccab, LPSTR pszFile, long cbFile, BOOL fContinuation, void FAR *pv)*/
{
    printf("DEBUG: fnFilePlaced\n");
    if (fContinuation == FALSE)
        cabLog(CABLOG_MSG, "%s (%d b) has been added to %s", pszFile, cbFile, pccab->szCab);

    return 0;
}

FNFCIGETNEXTCABINET(fnGetNextCab) /*(PCCAB pccab, ULONG cbPrevCab, void FAR *pv)*/
{
    printf("DEBUG: fnGetNextCab\n");
    return TRUE;
}

FNFCISTATUS(fnStatus) /*(UINT typeStatus, ULONG cb1, ULONG cb2, void FAR *pv)*/
{
    switch (typeStatus)
    {
    case statusFile:
        printf("\rCompressing source file (%d bytes compressed from %d)", cb1, cb2);
        break;
    case statusFolder:
        printf("\rCopying data into cabinet (%d of %d)", cb1, cb2);
        break;
    case statusCabinet:
        printf("\rWriting cabinet file (%d of approx. %d bytes)", cb1, cb2);
        return cb2;
    }

    return 0;
}

FNFCIGETOPENINFO(fnGetOpenInfo) /*(LPSTR pszName, USHORT *pdate, USHORT *ptime, USHORT *pattribs, int FAR *err, void FAR *pv)*/
{
    /* pdate: dddddmmmmyyyyyyy - d [1-31] m [1-12] y offset from 1980 */
    /* ptime: sssssmmmmhhhhhhh = s [second/2] m [0-59] h [0-23] */
    struct stat s;
    struct tm * time;
    FILE * f = NULL;
    if (stat(pszName, &s) != -1)
    {
        time = gmtime(&s.st_mtime);
        *pdate = 0;
        *ptime = 0;

        /* Note: tm_year is years since 1900 */
        *pdate = (time->tm_mday << 11) | ((time->tm_mon+1) << 7) | (time->tm_year-80);
        *ptime = ((time->tm_sec / 2) << 11) | (time->tm_min << 6) | (time->tm_hour);
        f = (FILE *) fnOpen(pszName, _O_RDONLY, 0, err, pv);
    }

    if (!f)
    {
        cabLog(CABLOG_ERR, "Could not access file information: %s", pszName);
        return -1;
    }
    return (INT_PTR) f;
}

/*
  MAKECAB [/V[n]] [/D var=value ...] [/L dir] source [destination]
  MAKECAB [/V[n]] [/D var=value ...] /F directive_file [...]
  source  - File to compress.
  destination  - File name to give compressed file. If omitted, the
  last character of the source file name is replaced
  with an underscore (_) and used as the destination.
  /F directives - A file with MakeCAB directives (may be repeated).
  /D var=value - Defines variable with specified value.
  /L dir - Location to place destination (default is current directory).
  /V[n] - Verbosity level (1..3)
*/
void usage(void)
{
    printf(
        "Usage: makecab [/V[n]] /F directive_file\n"
        "\nOptions:\n"
        "/F directives - A file with MakeCAB directives.\n"
        "/V[n] - Verbosity level (1..3)\n");
}

int main(int argc, char *argv[])
{
    int v = 0;
    char * ddfFile = NULL;
    CCAB ddfVars;
    DDFSRCFILE *srcList = NULL;
    DDFSRCFILE *srcListCurr = NULL;
    HFCI fci = NULL;
    ERF erf;
    char * cmd = NULL;
    unsigned int cmdSize = 0;

    while (argv[1] && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
        switch(tolower(argv[1][1]))
        {
        case 'f':              /* Directive file specified */
            argv++; argc--;
            ddfFile = argv[1];
            break;
        case 'v':              /* Verbosity [0-3] */
            switch(argv[1][2])
            {
            case '0': v = 0; break;
            case '1': v = 1; break;
            case '2': v = 2; break;
            case '3': v = 3; break;
            }
            break;
        case '?':
            usage();
            return 0;
        }
        argv++; argc--;
    }
    CabVerb = v;

    if (ddfFile == NULL)
    {
        cabLog(CABLOG_ERR, "No DDF file specified.");
        usage();
        return 0;
    }

    cabLog(CABLOG_MSG, "=== Parsing directive file \"%s\"===", ddfFile);
    switch(ParseDdf(ddfFile, &ddfVars, &srcListCurr, v))
    {
    case DDFERR_UNREAD: cabLog(CABLOG_ERR, "Could not open directive file."); break;
    }
    getcwd(ddfVars.szCabPath, MAX_PATH-1);
    strcat(ddfVars.szCabPath, "/");

    srcList = srcListCurr;
    if (srcList == NULL)
    {
        cabLog(CABLOG_ERR, "No input files were specified.");
        return 2;
    }

    /* Construct system call to lcab */
    for(srcListCurr = srcList; srcListCurr != NULL; srcListCurr = srcListCurr->next)
        cmdSize += strlen(srcListCurr->fileName) + 1;
    cmdSize += strlen(ddfVars.szCabPath) + strlen(ddfVars.szCab);
    cmdSize += 6; /* room for "lcab " and \0 */
    cmd = malloc(cmdSize);
    strcpy(cmd, "lcab ");
    for (srcListCurr = srcList; srcListCurr != NULL; srcListCurr = srcListCurr->next)
    {
        strcat(cmd, srcListCurr->fileName);
        strcat(cmd, " ");
    }
    strcat(cmd, ddfVars.szCabPath);
    strcat(cmd, ddfVars.szCab);

    cabLog(CABLOG_MSG, "syscall: %s\n", cmd);
    system(cmd);
    free(cmd);

    cabLog(CABLOG_MSG, "=== Cleaning up resources ===");
    /* Free list of cab source files */
    for (srcListCurr = srcList; srcListCurr != NULL; )
    {
        struct DDFSRCFILE *const next = srcListCurr->next;
        free(srcListCurr);
        srcListCurr = next;
    }
    cabLog(CABLOG_MSG, "Cabinet file %s/%s created.", ddfVars.szCabPath, ddfVars.szCab);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
