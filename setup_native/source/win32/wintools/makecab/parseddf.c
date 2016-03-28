/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "parseddf.h"
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

extern int errno;
static const char * VARS[] = {"cabinetname", "reservepercabinetsize", "maxdisksize", "compressiontype", "compress", "compressionmemory", "cabinet", "diskdirectorytemplate"};
static const unsigned int VARS_COUNT = 8;
static int CompMemory = 0;
static TCOMP CompType = tcompTYPE_MSZIP;
static unsigned int DdfVerb = 0;

/* Trim leading whitespace from a null terminated string */
void ltrim(char * str)
{
    unsigned int i = 0;
    while (isspace(str[i]) && str[i] != '\0') i++;
    if (i > 0) memmove(str, str+i, strlen(str)+1-i);
}

/* Trim trailing whitespace from a null terminated string */
void rtrim(char * str)
{
    unsigned int i = strlen(str) - 1;
    while (isspace(str[i]) && i > 0) i--;
    if (i) i++;
    str[i] = '\0';
}

/* Trim trailing and leading whitespace from a null terminated string */
void trim(char * str)
{
    ltrim(str);
    rtrim(str);
}

void ddfLogProgress(DDFLOGLEVEL lvl, char * desc, unsigned int progress)
{
    if (DdfVerb < lvl) return;
    if (progress == 0) printf("  %s: %3d%%", desc, progress);
    else if (progress > 0 && progress < 100) printf("\r  %s: %3d%%", desc, progress);
    else if (progress == 100) printf("\r  %s: 100%%\n", desc);
    fflush(stdout);
}

void ddfLog(DDFLOGLEVEL lvl, char * msg, ...)
{
    va_list args;
    if (DdfVerb < lvl) return;

    switch (lvl)
    {
    case DDFLOG_WRN: printf("[Warning] "); break;
    case DDFLOG_ERR: printf("[Error] "); break;
    }

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
}

void ddfPrintState(DDFLOGLEVEL lvl, PCCAB ccab, DDFSRCFILE * srcList)
{
    static const unsigned int SIZE = 2048;
    char msg[SIZE];
    char srcLine[DDF_MAX_CHARS];
    unsigned int len;
    DDFSRCFILE * srcListCurr;

    if (ccab)
    {
        len = sprintf(msg,
                      "=== Directive file state table ===\n"
                      "CabinetName           %s\n"
                      "ReservePerCabinetSize %d\n"
                      "MaxDiskSize           %d\n"
                      "Compress              %d\n"
                      "CompressionMemory     %d\n"
                      "Cabinet               %d\n"
                      "DiskDirectoryTemplate %s",
                      ccab->szCab,
                      ccab->cbReserveCFHeader,
                      ccab->iDisk,
                      1,
                      CompMemory,
                      1,
                      ccab->szCabPath);
    }
    else
    {
        msg[0] = '\0';
    }
    if (!srcList) printf("DEBUG: No list!\n");

    for (srcListCurr = srcList;
         srcListCurr != NULL && (len = strlen(msg)) < SIZE;
         srcListCurr = srcListCurr->next)
    {
        sprintf(srcLine, "\nsrc: %s (%s) extract? %d MSZIP? %d LZX? %d",
                srcListCurr->fileName, srcListCurr->cabName,
                srcListCurr->extract, (srcListCurr->cmpType == tcompTYPE_MSZIP),
                (srcListCurr->cmpType != tcompTYPE_MSZIP));
        strncat(msg, srcLine, SIZE-strlen(srcLine)-1);
    }
    if (msg[SIZE-1] != '\0') msg[SIZE-1] = '\0';

    ddfLog(lvl, msg);
}

/* Moves the next DDF token from the beginning of *line into *token.
 *
 * Currently supported token delimiters are:
 * whitespace (trimmed when not quoted)
 * ; demarks the start of a comment
 * " escapes whitespace, quotes and =
 * = when setting values
 * \n \r - only if file has endline incompatibility
 *
 * returns:
 * DDF_OK More tokens remain
 * DDFERR_NO_MORE_TOKENS At the end of the (usable) line.
 * DDFERR_INVALID_ENDL Token up to this point is stored, endline is chomped.
 */
DDFERR nextToken(char * token, char * line)
{
    unsigned int i = 0;
    unsigned int r = DDF_OK;
    unsigned int offset = 0;
    int c;
    BOOL esc = FALSE;

    ltrim(line);
    if (line[0] == '\0' || line[0] == ';') return DDFERR_NO_MORE_TOKENS;
    if (line[0] == '=')
    {
        memmove(line, line+1, strlen(line));
        trim(line);
    }

    for (i = 0; i < DDF_MAX_CHARS-1 && i < strlen(line); i++)
    {
        /* Chomp delimiting tokens unless they're escaped */
        c = line[i];
        if (c == '\"')
        {
            esc = !esc;
            offset++;
            /* Compress "" to ", otherwise don't add quote to token*/
            if (i > 0 && line[i-1] != '\"') continue;
        }
        else if (c == '\r' || c == '\n')
        {
            r = DDFERR_INVALID_ENDL;
            break;
        }
        else if(!esc)
        {
            if (isspace(c) || c == '=') { r = DDF_OK; break; }
            else if (c == ';') { r = DDFERR_NO_MORE_TOKENS; break; }
        }

        token[i - offset] = c;

    }
    token[i - offset] = '\0';
    memmove(line, line+i, strlen(line)+1-i);
    return r;
}

DDFERR setVar(DDFVAR key, PCCAB ccab, char * value)
{
    int i = 0;
    char * cp = NULL;

    ddfLog(DDFLOG_MSG, "Setting variable %s=%s...", VARS[key], value);

    switch (key)
    {
    case DDF_CABNAME:
        strncpy(ccab->szCab, value, 255);
        break;
    case DDF_RESERVEPERCAB:
        i = atoi(value);
        if (i < 0) i = 0;
        if (i > DDF_MAX_CABRESERVE) i = DDF_MAX_CABRESERVE;
        if (i % 4 != 0) i -= (i % 4);
        ccab->cbReserveCFHeader = i;
        break;
    case DDF_MAXDISK:
        i = atoi(value);
        ccab->cb = i;
        break;
    case DDF_COMPTYPE:
        if (strcmp(value, "LZX") == 0)
        {
/*      if (CompMemory) CompType = TCOMPfromLZXWindow(CompMemory);
        else CompType = tcompTYPE_LZX;*/
/*TODO: LZX not yet supported in Wine */
            CompType = tcompTYPE_MSZIP;
        }
        else
        {
            if (strcmp(value, "MSZIP") != 0)
                ddfLog(DDFLOG_WRN, "Invalid compression type \"%s\", reverting to MSZIP.", value);
            CompType = tcompTYPE_MSZIP;
        }
        break;
    case DDF_COMPRESS:
        break;
    case DDF_COMPMEM:
        i = atoi(value);
        if (i < 15) i = 15;
        if (i > 21) i = 21;
        CompMemory = i;
        if (CompType == tcompTYPE_LZX) CompType = TCOMPfromLZXWindow(CompMemory);
        break;
    case DDF_CABINET:
        break;
    case DDF_DISKDIRTEMPLATE:
        while ((cp = strchr(value, '*')) != '\0') *cp = '1';
        strncpy(ccab->szCabPath, value, 255);
        break;
    }

    return DDF_OK;
}

DDFERR parseCmd(char * line, PCCAB ccab, char * token)
{
    unsigned int r, i;
    char *p;
    for (p = token ; *p; ++p) *p = tolower(*p);

    /* Set command checks lower case token representing var name
       against known variables. If found, the next token is used as
       the value to assign */
    if (strcmp(token, ".set") == 0)
    {
        r = nextToken(token, line);
        if (r != DDF_OK) return DDFERR_FUNC_FAILED;

        for (p = token ; *p; ++p) *p = tolower(*p);
        if (isdigit(token[strlen(token)-1])) token[strlen(token)-1] = '\0';
        for (i = 0; i < SAL_N_ELEMENTS(VARS); i++)
        {
            if (strcmp(token, VARS[i]) == 0)
            {
                r = nextToken(token, line);
                if (r != DDF_OK) return DDFERR_FUNC_FAILED;
                setVar(i, ccab, token);
                break;
            }
        }
    }

    return DDF_OK;
}

char * getCabName(char * name)
{
    char * cName;
    if ((cName = strrchr(name, '/')) || (cName = strrchr(name, '\\')))
        cName++;
    else
        cName = name;
    return cName;
}

DDFERR parseSrc(char * line, DDFSRCFILE ** srcFileList, char * token)
{
    unsigned int r;
    struct stat s;
    DDFSRCFILE * tmpSrcFile;

    /* Check file exists before allocating DDFSRCFILE */
    if (stat(token, &s) == -1 && errno == ENOENT)
    {
        ddfLog(DDFLOG_WRN, "Could not find file \"%s\"", token);
        return DDFERR_SRC_SKIPPED;
    }

    tmpSrcFile = (DDFSRCFILE *) malloc(sizeof(DDFSRCFILE));
    if (!tmpSrcFile) return DDFERR_FUNC_FAILED;
    if (!(*srcFileList))
    {
        *srcFileList = tmpSrcFile;
    }
    else
    {
        while ((*srcFileList)->next != NULL) *srcFileList = (*srcFileList)->next;
        (*srcFileList)->next = tmpSrcFile;
    }

    strncpy(tmpSrcFile->fileName, token, MAX_PATH);
    /* Get cabinet file name or construct from src path if unspecified */
    r = nextToken(token, line);
    if (r != DDF_OK || token[0] == '/')
    {
        strncpy(tmpSrcFile->cabName, getCabName(tmpSrcFile->fileName), DDF_MAX_CABNAME-1);
    }
    else
    {
        strncpy(tmpSrcFile->cabName, token, DDF_MAX_CABNAME-1);
    }

    tmpSrcFile->cabName[DDF_MAX_CABNAME-1] = '\0';
    tmpSrcFile->extract = FALSE;
    tmpSrcFile->cmpType = CompType;
    tmpSrcFile->next = NULL;

    return DDF_OK;
}

DDFERR parseDdfLine(char * line, PCCAB ccab, DDFSRCFILE ** srcList)
{
    unsigned int r;
    char token[DDF_MAX_CHARS];

    if (line[0] == '\0') return DDF_OK;

    ddfLog(DDFLOG_MSG, "Parsing line \"%s\"", line);

    r = nextToken(token,line);
    if (r == DDF_OK)
    {
        if (token[0] == '.')
        {
            r = parseCmd(line, ccab, token);
            if (r != DDF_OK) ddfLog(DDFLOG_ERR, "Invalid command line skipped: %s%s", token, line);
        }
        else
        {
            r = parseSrc(line, srcList, token);

            if (r == DDFERR_SRC_SKIPPED)
                ddfLog(DDFLOG_ERR, "Source file command \"%s%s\" couldn't be read - skipped", token, line);
            else if (r == DDFERR_FUNC_FAILED)
                ddfLog(DDFLOG_WRN, "Could not parse command correctly, \"%s\" remains", line);
        }
    }

    return r;
}

DDFERR ParseDdf(char* ddfFile, PCCAB ccab, DDFSRCFILE ** srcListH, unsigned int v)
{
    char fLine[DDF_MAX_CHARS];
    FILE *ddf = NULL;
    unsigned int i = 0;
    DDFSRCFILE * srcListCurr = NULL;

    ddf = fopen(ddfFile, "r");
    if (ddf == NULL) return DDFERR_UNREAD;

    DdfVerb = v;
    for (; i < DDF_MAX_LINES && fgets(fLine, sizeof fLine, ddf); i++)
    {
        fLine[strlen(fLine)-1] = '\0';
        parseDdfLine(fLine, ccab, &srcListCurr);
        if (srcListCurr && !(*srcListH))
        {
            *srcListH = srcListCurr;
        }
    }

    ddfPrintState(DDFLOG_MSG, ccab, *srcListH);
    ccab->cbFolderThresh = ccab->cb;
    ccab->iDisk = 0;

    fclose(ddf);

    return DDF_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
