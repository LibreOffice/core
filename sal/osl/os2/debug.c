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

/*
 *@@sourcefile debug.c:
 *      this file contains debugging functions for the
 *      exception handlers in except.c.
 *
 *      This code is capable of unwinding the stack from
 *      a given address and trying to get function names
 *      and source line numbers, either from the respective
 *      module's debug code (if present) or from a SYM file,
 *      which is searched for in the directory of the module
 *      or in ?:\OS2\PDPSI\PMDF\WARP4.
 *
 *      This file incorporates code from the following:
 *      -- Marc Fiammante, John Currier, Kim Rasmussen,
 *         Anthony Cruise (EXCEPT3.ZIP package for a generic
 *         exception handling DLL, available at Hobbes).
 *
 *      Usage: All OS/2 programs.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@changed V0.9.0 [umoeller]: made some declarations C++-compatible
 *@@changed V0.9.1 (2000-01-30) [umoeller]: greatly cleaned up this file
 *
 *@@header "helpers\debug.h"
 */

/*
 *      This file Copyright (C) 1992-99 Ulrich M�ller,
 *                                      Kim Rasmussen,
 *                                      Marc Fiammante,
 *                                      John Currier,
 *                                      Anthony Cruise.
 *      This file is part of the "XWorkplace helpers" source package.
 *
 *      2009-06-15 published under LGPL3 with Ulrich M�ller permission.
 *
 */

//#define DEBUG_SYMDUMP // enable to dump sym file to log

//YD commented, since we need unsigned char BYTE!
//#define OS2EMX_PLAIN_CHAR
//Also gcc char is signed, while most structures requires unsigned data!
//Raised limits for all fields!

    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DONT_REPLACE_MALLOC
#include "helpers\setup.h"                      // code generation and debugging options

#include "helpers\debug.h"
#include "helpers\dosh.h"

#pragma hdrstop

#include <fcntl.h>
#ifdef __EMX__
    #include <sys\types.h>      // required for sys\stat.h; UM 99-10-22
#endif
#include <sys\stat.h>
#include <share.h>
#include <io.h>

#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef WORD
#define WORD  unsigned short
#endif

#pragma stack16(512)
#define HF_STDERR 2

/*
 *@@category: Helpers\Control program helpers\Exceptions/debugging
 *      See except.c and debug.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// this specifies whether we're dealing with 32-bit code;
// this gets changed whenever 16-bit count is detected
static BOOL     f32bit = TRUE;

/*
 * Global variables for Read32PmDebug:
 *
 */

ULONG           func_ofs;
ULONG           pubfunc_ofs;
//YD 17/07/06 c++ namespace can generate really long
//YD names, use a large buffer!
char            func_name[16*1024];
ULONG           var_ofs = 0;

struct {
    BYTE            name[128];
    ULONG           stack_offset;
    USHORT          type_idx;
} autovar_def[1024];

#pragma pack(1)

BYTE           *type_name[] =
{
    "8 bit signed                     ",
    "16 bit signed                    ",
    "32 bit signed                    ",
    "Unknown (0x83)                   ",
    "8 bit unsigned                   ",
    "16 bit unsigned                  ",
    "32 bit unsigned                  ",
    "Unknown (0x87)                   ",
    "32 bit real                      ",
    "64 bit real                      ",
    "80 bit real                      ",
    "Unknown (0x8B)                   ",
    "64 bit complex                   ",
    "128 bit complex                  ",
    "160 bit complex                  ",
    "Unknown (0x8F)                   ",
    "8 bit boolean                    ",
    "16 bit boolean                   ",
    "32 bit boolean                   ",
    "Unknown (0x93)                   ",
    "8 bit character                  ",
    "16 bit characters                ",
    "32 bit characters                ",
    "void                             ",
    "15 bit unsigned                  ",
    "24 bit unsigned                  ",
    "31 bit unsigned                  ",
    "Unknown (0x9B)                   ",
    "Unknown (0x9C)                   ",
    "Unknown (0x9D)                   ",
    "Unknown (0x9E)                   ",
    "Unknown (0x9F)                   ",
    "near pointer to 8 bit signed     ",
    "near pointer to 16 bit signed    ",
    "near pointer to 32 bit signed    ",
    "Unknown (0xA3)                   ",
    "near pointer to 8 bit unsigned   ",
    "near pointer to 16 bit unsigned  ",
    "near pointer to 32 bit unsigned  ",
    "Unknown (0xA7)                   ",
    "near pointer to 32 bit real      ",
    "near pointer to 64 bit real      ",
    "near pointer to 80 bit real      ",
    "Unknown (0xAB)                   ",
    "near pointer to 64 bit complex   ",
    "near pointer to 128 bit complex  ",
    "near pointer to 160 bit complex  ",
    "Unknown (0xAF)                   ",
    "near pointer to 8 bit boolean    ",
    "near pointer to 16 bit boolean   ",
    "near pointer to 32 bit boolean   ",
    "Unknown (0xB3)                   ",
    "near pointer to 8 bit character  ",
    "near pointer to 16 bit characters",
    "near pointer to 32 bit characters",
    "near pointer to void             ",
    "near pointer to 15 bit unsigned  ",
    "near pointer to 24 bit unsigned  ",
    "near pointer to 31 bit unsigned  ",
    "Unknown (0xBB)                   ",
    "Unknown (0xBC)                   ",
    "Unknown (0xBD)                   ",
    "Unknown (0xBE)                   ",
    "Unknown (0xBF)                   ",
    "far pointer to 8 bit signed      ",
    "far pointer to 16 bit signed     ",
    "far pointer to 32 bit signed     ",
    "Unknown (0xC3)                   ",
    "far pointer to 8 bit unsigned    ",
    "far pointer to 16 bit unsigned   ",
    "far pointer to 32 bit unsigned   ",
    "Unknown (0xC7)                   ",
    "far pointer to 32 bit real       ",
    "far pointer to 64 bit real       ",
    "far pointer to 80 bit real       ",
    "Unknown (0xCB)                   ",
    "far pointer to 64 bit complex    ",
    "far pointer to 128 bit complex   ",
    "far pointer to 160 bit complex   ",
    "Unknown (0xCF)                   ",
    "far pointer to 8 bit boolean     ",
    "far pointer to 16 bit boolean    ",
    "far pointer to 32 bit boolean    ",
    "Unknown (0xD3)                   ",
    "far pointer to 8 bit character   ",
    "far pointer to 16 bit characters ",
    "far pointer to 32 bit characters ",
    "far pointer to void              ",
    "far pointer to 15 bit unsigned   ",
    "far pointer to 24 bit unsigned   ",
    "far pointer to 31 bit unsigned   ",
};

// Thanks to John Currier:
// Do not call 16 bit code in myHandler function to prevent call
// to __EDCThunkProlog and problems is guard page exception handling
// Also reduce the stack size to 1K for true 16 bit calls.
// 16 bit calls thunk will now only occur on fatal exceptions
#pragma stack16(1024)

// ------------------------------------------------------------------
// Last 8 bytes of 16:16 file when CODEVIEW debugging info is present
#pragma pack(1)
struct _eodbug
{
    unsigned short  dbug;       // 'NB' signature
    unsigned short  ver;        // version
    unsigned long   dfaBase;    // size of codeview info
} G_eodbug;

#define         DBUGSIG         0x424E
#define         SSTMODULES      0x0101
#define         SSTPUBLICS      0x0102
#define         SSTTYPES        0x0103
#define         SSTSYMBOLS      0x0104
#define         SSTSRCLINES     0x0105
#define         SSTLIBRARIES    0x0106
#define         SSTSRCLINES2    0x0109
#define         SSTSRCLINES32   0x010B

typedef struct _SYMBASE
{
    unsigned short  dbug;       // 'NB' signature
    unsigned short  ver;        // version
    unsigned long   lfoDir;     // file offset to dir entries
} SYMBASE;

typedef struct _SSDIR
{
    unsigned short  sst;        // SubSection Type
    unsigned short  modindex;   // Module index number
    unsigned long   lfoStart;   // Start of section
    unsigned short  cb;         // Size of section
} SSDIR;

typedef struct _SSDIR32
{
    unsigned short  sst;        // SubSection Type
    unsigned short  modindex;   // Module index number
    unsigned long   lfoStart;   // Start of section
    unsigned long   cb;         // Size of section
} SSDIR32;

typedef struct _SSMODULE
{
    unsigned short  csBase;     // code segment base
    unsigned short  csOff;      // code segment offset
    unsigned short  csLen;      // code segment length
    unsigned short  ovrNum;     // overlay number
    unsigned short  indxSS;     // Index into sstLib or 0
    unsigned short  reserved;
    BYTE            csize;      // size of prefix string
} SSMODULE;

typedef struct _SSMOD32
{
    unsigned short  csBase;     // code segment base
    unsigned long   csOff;      // code segment offset
    unsigned long   csLen;      // code segment length
    unsigned long   ovrNum;     // overlay number
    unsigned short  indxSS;     // Index into sstLib or 0
    unsigned long   reserved;
    BYTE            csize;      // size of prefix string
} SSMOD32;

typedef struct _SSPUBLIC
{
    unsigned short  offset;
    unsigned short  segment;
    unsigned short  type;
    BYTE            csize;
} SSPUBLIC;

typedef struct _SSPUBLIC32
{
    unsigned long   offset;
    unsigned short  segment;
    unsigned short  type;
    BYTE            csize;
} SSPUBLIC32;

typedef struct _SSLINEENTRY32
{
    unsigned short  LineNum;
    unsigned short  FileNum;
    unsigned long   Offset;
} SSLINEENTRY32;

typedef struct _FIRSTLINEENTRY32
{
    unsigned short  LineNum;
    unsigned char   entry_type;
    unsigned char   reserved;
    unsigned short  numlines;
    unsigned short  segnum;
} FIRSTLINEENTRY32;

typedef struct _SSFILENUM32
{
    unsigned long   first_displayable;  // Not used
    unsigned long   number_displayable;     // Not used
    unsigned long   file_count; // number of source files
} SSFILENUM32;

/*
 *@@ XDEBUGINFO:
 *      buffers for Read... funcs.
 *
 *@@added V0.9.4 (2000-06-15) [umoeller]
 */

typedef struct _XDEBUGINFO
{
    char            szNrFile[300];      // receives source file
    char            szNrLine[300];      // receives line number
    //YD 17/07/06 c++ namespace can generate really long
    //YD names, use a large buffer!
    char            szNrPub[16*1024];   // receives function name

    struct new_seg  *pseg;
    struct o32_obj  *pobj;              // flat .EXE object table entry

    SYMBASE         base;

    SSDIR           *pDirTab;
    SSDIR32         *pDirTab32;
    unsigned char   *pEntTab;
    unsigned long   lfaBase;
    SSMOD32         ssmod32;
    SSPUBLIC32      sspub32;

    SSMODULE        ssmod;
    SSPUBLIC        sspub;
} XDEBUGINFO, *PXDEBUGINFO;


USHORT _THUNK_FUNCTION (Dos16SizeSeg) ();
//APIRET16 APIENTRY16 DOS16SIZESEG(USHORT Seg, PULONG16 Size);
USHORT DosSizeSeg (USHORT Seg, PULONG16 Size)
{
  return ((USHORT)
          (_THUNK_PROLOG (2+4);
           _THUNK_SHORT (Seg);
           _THUNK_FLAT (Size);
           _THUNK_CALL (Dos16SizeSeg)));
}

#pragma pack()

/* ******************************************************************
 *
 *   PART 1: ANALYZE DEBUG CODE
 *
 ********************************************************************/

static int Read16CodeView(FILE *LogFile, PXDEBUGINFO pxdi, int fh, int TrapSeg, int TrapOff, CHAR *FileName);
static int Read32PmDebug(FILE *LogFile, PXDEBUGINFO pxdi, int fh, int TrapSeg, int TrapOff, CHAR *FileName);

/*
 *@@ WriteAddressInfo:
 *      formats and writes a line into the trap log
 *      file.
 *
 *      This gets called for each line from the
 *      stack dump. At this point, the line in the
 *      trap log already has:
 *
 +          CS:EIP  : 000109FF  XMLVIEW :0
 +                                          ^^^ and we write here
 *      After this call, we have.
 *
 +          CS:EIP  : 000109FF  XMLVIEW :0  xxx.c  123 ConfirmCreate__Fv
 +                                          ^^^ and we write here
 *
 *@@added V0.9.12 (2001-05-12) [umoeller]
 */

static VOID WriteDebugInfo(FILE *LogFile,              // in: open log file
                           PXDEBUGINFO pxdi)           // in: debug info
{
    fprintf(LogFile,
            "%s%s%s",
            pxdi->szNrFile,
            pxdi->szNrLine,
            pxdi->szNrPub);
}

/*
 *@@ dbgPrintDebugInfo:
 *      this is the main entry point into analyzing debug
 *      code.
 *
 *      This analyzes a given address and tries to find
 *      debug code descriptions for this address. If found,
 *      the information is written to the given log file.
 *
 *      Gets called from dbgPrintStack.
 *
 *      This returns NO_ERROR if the could was successfully
 *      analyzed or something != 0 if we failed.
 *
 *      New with V0.84.
 */

APIRET dbgPrintDebugInfo(FILE *LogFile,         // out: log file to write to
                         CHAR *FileName,        // in: EXE/DLL module file name
                         ULONG Object,          // in: trapping object (from DosQueryModFromEIP)
                         ULONG TrapOffset)      // in: trapping address (from DosQueryModFromEIP)
{
    APIRET                  rc = 0;
    int                     ModuleFile = 0;
    static struct exe_hdr   OldExeHeader;
    static struct new_exe   NewExeHeader;

    ULONG                   ulSegment = Object + 1;     // segment no. is object no. + 1

    XDEBUGINFO              xdi;
    memset(&xdi, 0, sizeof(xdi));

    // open the module file for reading to analyze the code
    ModuleFile = sopen(FileName, O_RDONLY | O_BINARY, SH_DENYNO);

    if (ModuleFile != -1)
    {
        // file found:
        // read old Exe header
        if (read(ModuleFile, (void*)&OldExeHeader, 64) == -1L)
        {
            fprintf(LogFile, "errno %d reading old exe header\n", errno);
            close(ModuleFile);
            return 2;
        }
        // seek to new Exe header
        if (lseek(ModuleFile, (long)E_LFANEW(OldExeHeader), SEEK_SET) == -1L)
        {
            fprintf(LogFile, "errno %d seeking to new exe header\n", errno);
            close(ModuleFile);
            return 3;
        }
        if (read(ModuleFile, (void *)&NewExeHeader, 64) == -1L)
        {
            fprintf(LogFile, "errno %d reading new exe header\n", errno);
            close(ModuleFile);
            return 4;
        }

        // check EXE signature
        if (NE_MAGIC(NewExeHeader) == E32MAGIC)
        {
            /*
             * flat 32 executable:
             *
             */

            // do analysis for 32-bit code
            if (!(rc = Read32PmDebug(LogFile,
                                     &xdi,                // output
                                     ModuleFile,
                                     ulSegment,
                                     TrapOffset,
                                     FileName)))
                WriteDebugInfo(LogFile, &xdi);

            close(ModuleFile);

            // rc !=0 try with DBG file
            if (rc != 0)
            {
                strcpy(FileName + strlen(FileName) - 3, "DBG");     // Build DBG File name
                ModuleFile = sopen(FileName, O_RDONLY | O_BINARY, SH_DENYNO);
                if (ModuleFile != -1)
                {
                    if (!(rc = Read32PmDebug(LogFile,
                                             &xdi,
                                             ModuleFile,
                                             ulSegment,
                                             TrapOffset,
                                             FileName)))
                        WriteDebugInfo(LogFile, &xdi);

                    close(ModuleFile);
                }
            }

            return rc;
        }
        else
        {
            if (NE_MAGIC(NewExeHeader) == NEMAGIC)
            {
                /*
                 * 16:16 executable:
                 *
                 */

                if ((xdi.pseg = (struct new_seg *)calloc(NE_CSEG(NewExeHeader),
                                                         sizeof(struct new_seg)))
                            == NULL)
                {
                    fprintf(LogFile, "Out of memory!");
                    close(ModuleFile);
                    return -1;
                }
                if (  lseek(ModuleFile,
                            E_LFANEW(OldExeHeader) + NE_SEGTAB(NewExeHeader),
                            SEEK_SET) == -1L)
                {
                    fprintf(LogFile, "Error %u seeking segment table in %s\n", errno, FileName);
                    free(xdi.pseg);
                    close(ModuleFile);
                    return 9;
                }

                if (read(ModuleFile,
                         (void *)xdi.pseg,
                         NE_CSEG(NewExeHeader) * sizeof(struct new_seg))
                      == -1)
                {
                    fprintf(LogFile, "Error %u reading segment table from %s\n", errno, FileName);
                    free(xdi.pseg);
                    close(ModuleFile);
                    return 10;
                }

                if (!(rc = Read16CodeView(LogFile,
                                          &xdi,
                                          ModuleFile,
                                          ulSegment,
                                          TrapOffset,
                                          FileName)))
                    WriteDebugInfo(LogFile, &xdi);

                free(xdi.pseg);
                close(ModuleFile);

                // rc !=0 try with DBG file
                if (rc != 0)
                {
                    strcpy(FileName + strlen(FileName) - 3, "DBG");     // Build DBG File name
                    ModuleFile = sopen(FileName,
                                       O_RDONLY | O_BINARY, SH_DENYNO);
                    if (ModuleFile != -1)
                    {
                        if (!(rc = Read16CodeView(LogFile,
                                                  &xdi,
                                                  ModuleFile,
                                                  ulSegment,
                                                  TrapOffset,
                                                  FileName)))
                            WriteDebugInfo(LogFile, &xdi);

                        close(ModuleFile);
                    }
                }
                return rc;
            }
            else
            {
                /*
                 * Unknown executable:
                 *
                 */

                fprintf(LogFile, "Error, could not find exe signature");
                close(ModuleFile);
                return 11;
            }
        }
    } // end if (ModuleFile != -1)
    else
    {
        fprintf(LogFile, "Error %d opening module file %s", errno, FileName);
        return 1;
    }                           // endif

    // return 0;        we never get here
}

char            fname[256],
                ModName[80];
char            ename[256],
                dummy[256];

#define MAX_USERDEFS 300        // raised from 150 V0.9.1 (2000-01-30) [umoeller]
#define MAX_POINTERS 300        // raised from 150 V0.9.1 (2000-01-30) [umoeller]

USHORT          userdef_count;
USHORT          pointer_count;

struct one_userdef_rec
{
    USHORT          idx;
    USHORT          type_index;
    BYTE            name[33];
} one_userdef[MAX_USERDEFS];

struct one_pointer_rec
{
    USHORT          idx;
    USHORT          type_index;
    BYTE            type_qual;
    BYTE            name[33];
} one_pointer[MAX_POINTERS];

/*
 * Read32PmDebug:
 *      parses 32-bit debug code.
 *      Called from dbgPrintDebugInfo for 32-bit modules.
 */

static int Read32PmDebug(FILE *LogFile,        // in: text log file to write to
                         PXDEBUGINFO pxdi,
                         int ModuleFile,       // in: module file opened with sopen()
                         int TrapSeg,
                         int TrapOff,
                         CHAR *FileName)
{
    static unsigned int CurrSymSeg, NrSymbol,
                        /* offset, */ NrPublic,
                        NrFile, NrLine, /* NrEntry */
                        numdir, namelen,
                        numlines /* , line */;
    static int      ModIndex;
    static int      bytesread, i, j;
    static SSLINEENTRY32 LineEntry;
    static SSFILENUM32 FileInfo;
    static FIRSTLINEENTRY32 FirstLine;
    static BYTE     dump_vars = FALSE;
    static USHORT   idx;
    static BOOL     read_types;
    static LONG     lSize;

    ModIndex = 0;
    // See if any CODEVIEW info
    if (lseek(ModuleFile, -8L, SEEK_END) == -1)
    {
        fprintf(LogFile, "Error %u seeking CodeView table in %s\n", errno, FileName);
        return (18);
    }

    if (read(ModuleFile,
             (void *)&G_eodbug, 8)
            == -1)
    {
        fprintf(LogFile, "Error %u reading debug info from %s\n", errno, FileName);
        return (19);
    }
    if (G_eodbug.dbug != DBUGSIG)
    {
        // fprintf(LogFile,"\nNo CodeView information stored.\n");
        return (100);
    }

    if (    (pxdi->lfaBase = lseek(ModuleFile,
                                   -(LONG)G_eodbug.dfaBase,
                                   SEEK_END))
         == -1L)
    {
        fprintf(LogFile, "Error %u seeking base codeview data in %s\n", errno, FileName);
        return (20);
    }

    if (read(ModuleFile,
             (void *)&pxdi->base, 8)
        == -1)
    {
        fprintf(LogFile, "Error %u reading base codeview data in %s\n", errno, FileName);
        return (21);
    }

    if (lseek(ModuleFile,
              pxdi->base.lfoDir - 8 + 4,
              SEEK_CUR)
        == -1)
    {
        fprintf(LogFile, "Error %u seeking dir codeview data in %s\n", errno, FileName);
        return (22);
    }

    if (read(ModuleFile,
             (void *)&numdir, 4)
        == -1)
    {
        fprintf(LogFile, "Error %u reading dir codeview data in %s\n", errno, FileName);
        return (23);
    }

    // Read dir table into buffer
    if (    (pxdi->pDirTab32 = (SSDIR32*)calloc(numdir,
                                             sizeof(SSDIR32)))
        == NULL)
    {
        fprintf(LogFile, "Out of memory!");
        return (-1);
    }

    if (read(ModuleFile,
             (void*)pxdi->pDirTab32,
             numdir * sizeof(SSDIR32))
        == -1)
    {
        fprintf(LogFile, "Error %u reading codeview dir table from %s\n", errno, FileName);
        free(pxdi->pDirTab32);
        return (24);
    }

    i = 0;
    while (i < numdir)
    {
        if (pxdi->pDirTab32[i].sst != SSTMODULES)
        {
            i++;
            continue;
        }

        NrPublic = 0x0;
        NrSymbol = 0;
        NrLine = 0x0;
        NrFile = 0x0;
        CurrSymSeg = 0;
        // point to subsection
        lseek(ModuleFile,
              pxdi->pDirTab32[i].lfoStart + pxdi->lfaBase,
              SEEK_SET);
        read(ModuleFile,
             (void*)&pxdi->ssmod32.csBase,
             sizeof(SSMOD32));
        read(ModuleFile,
             (void*)ModName,
             (unsigned)pxdi->ssmod32.csize);
        ModIndex = pxdi->pDirTab32[i].modindex;
        ModName[pxdi->ssmod32.csize] = '\0';
        i++;

        read_types = FALSE;

        while (     (pxdi->pDirTab32[i].modindex == ModIndex)
                 && (i < numdir)
              )
        {
            // point to subsection
            lseek(ModuleFile,
                  pxdi->pDirTab32[i].lfoStart + pxdi->lfaBase,
                  SEEK_SET);

            switch (pxdi->pDirTab32[i].sst)
            {
                case SSTPUBLICS:
                    bytesread = 0;
                    while (bytesread < pxdi->pDirTab32[i].cb)
                    {
                        bytesread += read(ModuleFile,
                                          (void *)&pxdi->sspub32.offset,
                                          sizeof(pxdi->sspub32));
                        bytesread += read(ModuleFile,
                                          (void*)ename,
                                          (unsigned)pxdi->sspub32.csize);
                        ename[pxdi->sspub32.csize] = '\0';
                        if (    (pxdi->sspub32.segment == TrapSeg)
                             && (pxdi->sspub32.offset <= TrapOff)
                             && (pxdi->sspub32.offset >= NrPublic)
                           )
                        {
                            NrPublic = pubfunc_ofs = pxdi->sspub32.offset;
                            read_types = TRUE;
                            sprintf(pxdi->szNrPub,
                                    "%s %s (%s)\n",
                                    (pxdi->sspub32.type == 1)
                                            ? " Abs"
                                            : " ",
                                    ename,
                                    ModName
                                );
                            // but continue, because there might be a
                            // symbol that comes closer
                        }
                    }
                    break;

                // Read symbols, so we can dump the variables on the stack
                case SSTSYMBOLS:
                    if (TrapSeg != pxdi->ssmod32.csBase)
                        break;

                    bytesread = 0;
                    while (bytesread < pxdi->pDirTab32[i].cb)
                    {
                        static USHORT   usLength;
                        static USHORT   usLengthSym;
                        static BYTE     b1,
                                        b2;
                        static BYTE     bType;
                                       // *ptr;
                        static ULONG    ofs;
                        // static ULONG    last_addr = 0;
                        //YD 17/07/06 c++ namespace can generate really long
                        //YD names, use a large buffer!
                        static BYTE     str[16*1024];
                        static struct symseg_rec symseg;
                        static struct symauto_rec symauto;
                        static struct symproc_rec symproc;

                        // Read the length of this subentry
                        bytesread += read(ModuleFile, &b1, 1);
                        if (b1 & 0x80)
                        {
                            bytesread += read(ModuleFile, &b2, 1);
                            usLength = ((b1 & 0x7F) << 8) + b2;
                        }
                        else
                            usLength = b1;

                        ofs = tell(ModuleFile);

                        bytesread += read(ModuleFile, &bType, 1);

                        switch (bType)
                        {
                            case SYM_CHANGESEG:
                                read(ModuleFile, &symseg, sizeof(symseg));
                                CurrSymSeg = symseg.seg_no;
                                break;

                            case SYM_PROC:
                            case SYM_CPPPROC:
                                read(ModuleFile, &symproc, sizeof(symproc));
                                if (symproc.name_len & 0x80)
                                {
                                    read(ModuleFile, &b2, 1);
                                       usLengthSym  = ((symproc.name_len & 0x7F) << 8) + b2;
                                }
                                else
                                {
                                   usLengthSym  = symproc.name_len;
                                }
                                read(ModuleFile, str, usLengthSym);
                                 str[usLengthSym] = 0;

                                if ((CurrSymSeg == TrapSeg) &&
                                    (symproc.offset <= TrapOff) &&
                                    (symproc.offset >= NrSymbol))
                                {

                                    dump_vars = TRUE;
                                    var_ofs = 0;
                                    NrSymbol = symproc.offset;
                                    func_ofs = symproc.offset;

                                    strcpy(func_name, str);
                                }
                                else
                                {
                                    dump_vars = FALSE;
                                }
                                break;

                            case SYM_AUTO:
                                if (!dump_vars)
                                    break;

                                read(ModuleFile, &symauto, sizeof(symauto));
                                read(ModuleFile, str, symauto.name_len);
                                if (symauto.name_len==0x80)
                                    printf("symauto.name_len==0x80\n");
                                str[symauto.name_len] = 0;

                                strcpy(autovar_def[var_ofs].name, str);
                                autovar_def[var_ofs].stack_offset = symauto.stack_offset;
                                autovar_def[var_ofs].type_idx = symauto.type_idx;
                                var_ofs++;
                                break;

                        }

                        bytesread += usLength;

                        lseek(ModuleFile, ofs + usLength, SEEK_SET);
                    }
                    break;

                case SSTTYPES:
                    // if (ModIndex != TrapSeg)
                    if (!read_types)
                        break;

                    bytesread = 0;
                    idx = 0x200;
                    userdef_count = 0;
                    pointer_count = 0;
                    while (bytesread < pxdi->pDirTab32[i].cb)
                    {
                        static struct type_rec type;
                        static struct type_userdefrec udef;
                        static struct type_pointerrec point;
                        static ULONG    ofs;
                        static BYTE     str[256];

                        // Read the length of this subentry
                        ofs = tell(ModuleFile);

                        read(ModuleFile, &type, sizeof(type));
                        bytesread += sizeof(type);

                        switch (type.type)
                        {
                            case TYPE_USERDEF:
                                if (userdef_count >= MAX_USERDEFS)
                                    break;

                                read(ModuleFile, &udef, sizeof(udef));
                                read(ModuleFile, str, udef.name_len);
                                str[udef.name_len] = 0;

                                // Insert userdef in table
                                one_userdef[userdef_count].idx = idx;
                                one_userdef[userdef_count].type_index = udef.type_index;
                                memcpy(one_userdef[userdef_count].name,
                                       str,
                                       _min(udef.name_len + 1, 32));
                                one_userdef[userdef_count].name[32] = 0;
                                userdef_count++;
                                break;

                            case TYPE_POINTER:
                                if (pointer_count >= MAX_POINTERS)
                                    break;

                                read(ModuleFile, &point, sizeof(point));
                                read(ModuleFile, str, point.name_len);
                                str[point.name_len] = 0;

                                // Insert userdef in table
                                one_pointer[pointer_count].idx = idx;
                                one_pointer[pointer_count].type_index = point.type_index;
                                memcpy(one_pointer[pointer_count].name,
                                       str,
                                       _min(point.name_len + 1, 32));
                                one_pointer[pointer_count].name[32] = 0;
                                one_pointer[pointer_count].type_qual = type.type_qual;
                                pointer_count++;
                                break;
                        }

                        ++idx;

                        bytesread += type.length;

                        lseek(ModuleFile, ofs + type.length + 2, SEEK_SET);
                    }
                    break;

                case SSTSRCLINES32:
                    if (TrapSeg != pxdi->ssmod32.csBase)
                        break;

                    // read first line
                    do
                    {
                        read(ModuleFile, (void *)&FirstLine, sizeof(FirstLine));

                        if (FirstLine.LineNum != 0)
                        {
                            fprintf(LogFile, "Missing Line table information\n");
                            break;
                        }       // endif
                        numlines = FirstLine.numlines;
                        // Other type of data skip 4 more bytes
                        if (FirstLine.entry_type < 4)
                        {
                            read(ModuleFile, (void *)&lSize, 4);
                            if (FirstLine.entry_type == 3)
                                lseek(ModuleFile, lSize, SEEK_CUR);
                        }
                    }
                    while (FirstLine.entry_type == 3);

                    for (j = 0; j < numlines; j++)
                    {
                        switch (FirstLine.entry_type)
                        {
                            case 0:
                                read(ModuleFile, (void *)&LineEntry, sizeof(LineEntry));
                                // Changed by Kim Rasmussen 26/06 1996 to ignore linenumber 0
                                // if (LineEntry.Offset+ssmod32.csOff<=TrapOff && LineEntry.Offset+ssmod32.csOff>=NrLine) {
                                if (    (LineEntry.LineNum)
                                     && (LineEntry.Offset + pxdi->ssmod32.csOff
                                            <= TrapOff)
                                     && (LineEntry.Offset + pxdi->ssmod32.csOff >= NrLine)
                                   )
                                {
                                    NrLine = LineEntry.Offset;
                                    NrFile = LineEntry.FileNum;
                                    /*pOffset =sprintf(szNrLine,"%04X:%08X  line #%hu ",
                                     * ssmod32.csBase,LineEntry.Offset,
                                     * LineEntry.LineNum); */
                                    sprintf(pxdi->szNrLine, "% 6hu", LineEntry.LineNum);
                                }
                                break;

                            case 1:
                                lseek(ModuleFile, sizeof(struct linlist_rec), SEEK_CUR);
                                break;

                            case 2:
                                lseek(ModuleFile, sizeof(struct linsourcelist_rec), SEEK_CUR);
                                break;

                            case 3:
                                lseek(ModuleFile, sizeof(struct filenam_rec), SEEK_CUR);
                                break;

                            case 4:
                                lseek(ModuleFile, sizeof(struct pathtab_rec), SEEK_CUR);
                                break;

                        }
                    }

                    if (NrFile != 0)
                    {
                        // file found:
                        read(ModuleFile, (void*)&FileInfo, sizeof(FileInfo));
                        namelen = 0;
                        for (j = 1; j <= FileInfo.file_count; j++)
                        {
                            namelen = 0;
                            read(ModuleFile, (void *)&namelen, 1);
                            read(ModuleFile, (void *)ename, namelen);
                            if (j == NrFile)
                                break;
                        }
                        ename[namelen] = '\0';
                        //  pOffset=sprintf(szNrLine+pOffset," (%s) (%s)\n",ename,ModName);
                        sprintf(pxdi->szNrFile, "%11.11s ", ename);
                    }
                    else
                    {
                        // strcat(szNrLine,"\n"); avoid new line for empty name fill
                        strcpy(pxdi->szNrFile, "file?       ");
                    } // endif
                    break;
            } // end switch

            i++;
        } // end while modindex
    } // End While i < numdir
    free(pxdi->pDirTab32);
    return (0);
}

/*
 * Read16CodeView:
 *      parses 16-bit debug code.
 *      Called from dbgPrintDebugInfo for 16-bit modules.
 */

static int Read16CodeView(FILE *LogFile,       // in: text log file to write to
                          PXDEBUGINFO pxdi,
                          int fh,
                          int TrapSeg,
                          int TrapOff,
                          CHAR *FileName)
{
    static unsigned short int offset,
                    NrPublic, NrLine,
                    numdir,
                    namelen, numlines,
                    line;
    static int      ModIndex;
    static int      bytesread, i, j;

    ModIndex = 0;
    // See if any CODEVIEW info
    if (lseek(fh, -8L, SEEK_END) == -1)
    {
        fprintf(LogFile, "Error %u seeking CodeView table in %s\n", errno, FileName);
        return (18);
    }

    if (read(fh, (void *)&G_eodbug, 8) == -1)
    {
        fprintf(LogFile, "Error %u reading debug info from %s\n", errno, FileName);
        return (19);
    }
    if (G_eodbug.dbug != DBUGSIG)
    {
        // fprintf(LogFile,"\nNo CodeView information stored.\n");
        return (100);
    }

    if ((pxdi->lfaBase = lseek(fh, -(LONG)G_eodbug.dfaBase, SEEK_END)) == -1L)
    {
        fprintf(LogFile, "Error %u seeking base codeview data in %s\n", errno, FileName);
        return (20);
    }

    if (read(fh, (void *)&pxdi->base, 8) == -1)
    {
        fprintf(LogFile, "Error %u reading base codeview data in %s\n", errno, FileName);
        return (21);
    }

    if (lseek(fh, pxdi->base.lfoDir - 8, SEEK_CUR) == -1)
    {
        fprintf(LogFile, "Error %u seeking dir codeview data in %s\n", errno, FileName);
        return (22);
    }

    if (read(fh, (void *)&numdir, 2) == -1)
    {
        fprintf(LogFile, "Error %u reading dir codeview data in %s\n", errno, FileName);
        return (23);
    }

    // Read dir table into buffer
    if ((pxdi->pDirTab = (SSDIR*)calloc(numdir, sizeof(SSDIR))) == NULL)
    {
        fprintf(LogFile, "Out of memory!");
        return (-1);
    }

    if (read(fh, (void*)pxdi->pDirTab, numdir * sizeof(SSDIR)) == -1)
    {
        fprintf(LogFile, "Error %u reading codeview dir table from %s\n", errno, FileName);
        free(pxdi->pDirTab);
        return (24);
    }

    i = 0;
    while (i < numdir)
    {
        if (pxdi->pDirTab[i].sst != SSTMODULES)
        {
            i++;
            continue;
        }
        NrPublic = 0x0;
        NrLine = 0x0;
        // point to subsection
        lseek(fh, pxdi->pDirTab[i].lfoStart + pxdi->lfaBase, SEEK_SET);
        read(fh, (void *)&pxdi->ssmod.csBase, sizeof(SSMODULE));
        read(fh, (void *)ModName, (unsigned)pxdi->ssmod.csize);
        ModIndex = pxdi->pDirTab[i].modindex;
        ModName[pxdi->ssmod.csize] = '\0';
        i++;
        while (pxdi->pDirTab[i].modindex == ModIndex && i < numdir)
        {
            // point to subsection
            lseek(fh, pxdi->pDirTab[i].lfoStart + pxdi->lfaBase, SEEK_SET);
            switch (pxdi->pDirTab[i].sst)
            {
                case SSTPUBLICS:
                    bytesread = 0;
                    while (bytesread < pxdi->pDirTab[i].cb)
                    {
                        bytesread += read(fh, (void *)&pxdi->sspub.offset, sizeof(pxdi->sspub));
                        bytesread += read(fh, (void *)ename, (unsigned)pxdi->sspub.csize);
                        ename[pxdi->sspub.csize] = '\0';
                        if ((pxdi->sspub.segment == TrapSeg) &&
                            (pxdi->sspub.offset <= TrapOff) &&
                            (pxdi->sspub.offset >= NrPublic))
                        {
                            NrPublic = pxdi->sspub.offset;
                            sprintf(pxdi->szNrPub, "%s %s (%s) %04hX:%04hX\n",
                                    (pxdi->sspub.type == 1) ? " Abs" : " ", ename,
                                    ModName, // ()
                                    pxdi->sspub.segment,
                                    pxdi->sspub.offset
                                );
                        }
                    }
                    break;

                case SSTSRCLINES2:
                case SSTSRCLINES:
                    if (TrapSeg != pxdi->ssmod.csBase)
                        break;
                    namelen = 0;
                    read(fh, (void *)&namelen, 1);
                    read(fh, (void *)ename, namelen);
                    ename[namelen] = '\0';
                    // skip 2 zero bytes
                    if (pxdi->pDirTab[i].sst == SSTSRCLINES2)
                        read(fh, (void *)&numlines, 2);
                    read(fh, (void *)&numlines, 2);
                    for (j = 0; j < numlines; j++)
                    {
                        read(fh, (void *)&line, 2);
                        read(fh, (void *)&offset, 2);
                        if (offset <= TrapOff && offset >= NrLine)
                        {
                            NrLine = offset;
                            sprintf(pxdi->szNrFile, "% 12.12s ", ename);
                            sprintf(pxdi->szNrLine, "% 6hu", line);
                            /*sprintf(szNrLine,"%04hX:%04hX  line #%hu  (%s) (%s)\n",
                             * ssmod.csBase,offset,line,ModName,ename); */
                        }
                    }
                    break;
            }                   // end switch
            i++;
        }                       // end while modindex
    }                           // End While i < numdir
    free(pxdi->pDirTab);
    return (0);
}

/* ******************************************************************
 *
 *   PART 2: ANALYZE VARIABLES
 *
 ********************************************************************/

/*
 * var_value:
 *      writes a description of a variable type to
 *      the specified buffer, depending on "type".
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: changed prototype to use external buffer
 */

static VOID var_value(void *varptr,        // in: address of the variable on the stack
                      char *pszBuf,        // out: information
                      BYTE type)           // in: type; if >= 32, we'll call DosQueryMem
{
    ULONG           Size = 1,
                    Attr = 0;

    if (DosQueryMem(varptr, &Size, &Attr) != NO_ERROR)
    {
        sprintf(pszBuf, "type %d, DosQueryMem failed", type);
        return;
    }

    if ((Attr & PAG_READ) == 0)
    {
        sprintf(pszBuf, "type %d, read-access to value denied", type);
        return;
    }

    if (type == 0)
        sprintf(pszBuf, "%hd", *(signed char*)varptr);
    else if (type == 1)
        sprintf(pszBuf, "%hd", *(signed short*)varptr);
    else if (type == 2)
        sprintf(pszBuf, "%ld", *(signed long*)varptr);
    else if (type == 4)
        sprintf(pszBuf, "%hu", *(BYTE*) varptr);
    else if (type == 5)
        sprintf(pszBuf, "%hu", *(USHORT*)varptr);
    else if (type == 6)
        sprintf(pszBuf, "0x%lX (%lu)", *((ULONG*)varptr), *((ULONG*)varptr));
    else if (type == 8)
        sprintf(pszBuf, "%f", *(float*)varptr);
    else if (type == 9)
        sprintf(pszBuf, "%f", *(double*)varptr);
    else if (type == 10)
        sprintf(pszBuf, "%f", (double)(*(long double*)varptr));
    else if (type == 16)
        sprintf(pszBuf, "%s", *(char*)varptr ? "TRUE" : "FALSE");
    else if (type == 17)
        sprintf(pszBuf, "%s", *(short*)varptr ? "TRUE" : "FALSE");
    else if (type == 18)
        sprintf(pszBuf, "%s", *(long*)varptr ? "TRUE" : "FALSE");
    else if (type == 20)
        sprintf(pszBuf, "%c", *(char*)varptr);
    else if (type == 21)
        sprintf(pszBuf, "%hd", (*(short*)varptr));
    else if (type == 22)
        sprintf(pszBuf, "%ld", *(long*)varptr);
    else if (type == 23)
        sprintf(pszBuf, "void");
    else if (type >= 32)
    {
        sprintf(pszBuf, "0x%p", (void*)(*(ULONG*)varptr));
        if (Attr & PAG_FREE)
        {
            strcat(pszBuf, " unallocated memory");
        }
        else
        {
            if ((Attr & PAG_COMMIT) == 0x0U)
            {
                strcat(pszBuf, " uncommitted");
            }               // endif
            if ((Attr & PAG_WRITE) == 0x0U)
            {
                strcat(pszBuf, " unwritable");
            }               // endif
            if ((Attr & PAG_READ) == 0x0U)
            {
                strcat(pszBuf, " unreadable");
            }               // endif
        }                   // endif
    }                       // endif
    else
        sprintf(pszBuf, "Unknown type %d", type);
}

/*
 * search_userdefs:
 *      searches the table of userdef's-
 *      Return TRUE if found.
 */

static BOOL search_userdefs(FILE *LogFile,     // in: text log file to write to
                            ULONG stackofs,
                            USHORT var_no)
{
    USHORT          pos;

    for (pos = 0;
         pos < userdef_count;
         pos++)
    {
        if (one_userdef[pos].idx == autovar_def[var_no].type_idx)
        {
            if (    (one_userdef[pos].type_index >= 0x80)
                // &&  (one_userdef[pos].type_index <= 0xDA)
               )
            {
                static char sszVar3[500] = "complex";
                if (one_userdef[pos].type_index <= 0xDA)
                    var_value((void*)(stackofs + autovar_def[var_no].stack_offset),
                              sszVar3,
                              one_userdef[pos].type_index - 0x80);

                fprintf(LogFile,
                        "     %- 6ld %- 20.20s %- 33.33s %s (user)\n",
                        autovar_def[var_no].stack_offset,       // stack offset
                        autovar_def[var_no].name,               // identifier
                        one_userdef[pos].name,                  // type name
                        sszVar3                                 // composed by var_value
                       );
                return TRUE;
            }
            else
                return FALSE;
        }
    }

    return FALSE;
}

/*
 * search_pointers:
 *
 */

static BOOL search_pointers(FILE *LogFile,     // in: text log file to write to
                            ULONG stackofs,
                            USHORT var_no)
{
    USHORT          pos, upos;
    static BYTE     str[35];
    static char     sszVar[500];

    // BYTE            type_index;

    for (pos = 0;
         (   (pos < pointer_count)
          && (one_pointer[pos].idx != autovar_def[var_no].type_idx)
         );
         pos++);

    if (pos < pointer_count)
    {
        if (    (one_pointer[pos].type_index >= 0x80)
             && (one_pointer[pos].type_index <= 0xDA)
           )
        {
            strcpy(str, type_name[one_pointer[pos].type_index - 0x80]);
            strcat(str, " *");
            var_value((void*)(stackofs + autovar_def[var_no].stack_offset),
                      sszVar,
                      32);
            fprintf(LogFile, "     %- 6ld %- 20.20s %- 33.33s %s (ptr1)\n",
                    autovar_def[var_no].stack_offset,
                    autovar_def[var_no].name,
                    str,
                    sszVar);
            return TRUE;
        }
        else
        {
            // If the result isn't a simple type, look for it in the other lists
            for (upos = 0;
                 (   (upos < userdef_count)
                  && (one_userdef[upos].idx != one_pointer[pos].type_index)
                 );
                 upos++)
                ;

            if (upos < userdef_count)
            {
                strcpy(str, one_userdef[upos].name);
                strcat(str, " *");
                var_value((void *)(stackofs + autovar_def[var_no].stack_offset),
                          sszVar,
                          32);
                fprintf(LogFile, "     %- 6ld %- 20.20s %- 33.33s %s (ptr2)\n",
                        autovar_def[var_no].stack_offset,
                        autovar_def[var_no].name,
                        str,
                        sszVar);
                return TRUE;
            }
            else
            {
                // if it isn't a userdef, for now give up and just print
                // as much as we know
                sprintf(str, "Pointer to type 0x%X", one_pointer[pos].type_index);

                var_value((void *)(stackofs + autovar_def[var_no].stack_offset),
                          sszVar,
                          32);
                fprintf(LogFile, "     %- 6ld %- 20.20s %- 33.33s %s (ptr3)\n",
                        autovar_def[var_no].stack_offset,
                        autovar_def[var_no].name,
                        str,
                        sszVar);

                return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 *@@ dbgPrintVariables:
 *      Dumps variables for the specified stack offset
 *      to the specified log file.
 *
 *      New with V0.84.
 */

void dbgPrintVariables(FILE *LogFile,   // in: text log file to write to
                       ULONG stackofs)
{
    USHORT          n; // , pos;
    BOOL            AutoVarsFound = FALSE;

    if (/* 1 || */ func_ofs == pubfunc_ofs)
    {
        for (n = 0;
             n < var_ofs;
             n++)
        {
            if (AutoVarsFound == FALSE)
            {
                AutoVarsFound = TRUE;
                fprintf(LogFile, "     List of auto variables at EBP %p in %s:\n",
                        (PVOID)stackofs,
                        func_name);
                fprintf(LogFile, "     Offset Name                 Type                              Value            \n");
                fprintf(LogFile, "     ������ �������������������� ��������������������������������� �����������������\n");
            }

            // If it's one of the simple types
            if (   (autovar_def[n].type_idx >= 0x80)
                && (autovar_def[n].type_idx <= 0xDA)
                )
            {
                static char sszVar2[500];

                var_value((void *)(stackofs + autovar_def[n].stack_offset),
                          sszVar2,
                          autovar_def[n].type_idx - 0x80);

                fprintf(LogFile, "     %- 6ld %- 20.20s %- 33.33s %s (simple)\n",
                        autovar_def[n].stack_offset,
                        autovar_def[n].name,
                        type_name[autovar_def[n].type_idx - 0x80],
                        sszVar2);
            }
            else
            {                   // Complex type, check if we know what it is
                if (!search_userdefs(LogFile, stackofs, n))
                {
                    if (!search_pointers(LogFile, stackofs, n))
                    {
                        fprintf(LogFile, "     %- 6ld %-20.20s 0x%X (unknown)\n",
                                autovar_def[n].stack_offset,
                                autovar_def[n].name,
                                autovar_def[n].type_idx);
                    }
                }
            }
        }
        /* if (AutoVarsFound == FALSE)
        {
            fprintf(LogFile, "  No auto variables found in %s.\n", func_name);
        } */
        fprintf(LogFile, "\n");
    }
}

/* ******************************************************************
 *
 *   PART 3: ANALYZE SYMBOL (.SYM) FILE
 *
 ********************************************************************/

/*
 *@@ dbgPrintSYMInfo:
 *      this gets called by dbgPrintStack if dbgPrintDebugInfo
 *      failed (because no debug code was found) to check if
 *      maybe a SYM file with the same filename exists and try
 *      to get the info from there.
 *
 *      This gets called for every line of the stack
 *      walk, but only if getting the information from
 *      the debug code failed, e.g. because no debug code
 *      was available for an address.
 *
 *      The file pointer is in the "Source file" column
 *      every time this gets called.
 *
 *      New with V0.84.
 *
 *      Returns 0 if reading the SYM file was successful.
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: added return code; this used to be VOID
 */

int dbgPrintSYMInfo(FILE *LogFile,      // in: text log file to write to
                    CHAR *SymFileName,  // in: SYM file name (can be fully q'fied)
                    ULONG Object,
                    ULONG TrapOffset)
{
    static FILE    *SymFile;
    static MAPDEF   MapDef;
    static SEGDEF   SegDef;
    static SYMDEF32 SymDef32;
    static SYMDEF16 SymDef16;
    static char     Buffer[256];
    static int      SegNum, SymNum, LastVal;
    static unsigned long int SegOffset,
                    SymOffset, SymPtrOffset;

    // open .SYM file
#ifdef DEBUG_SYMDUMP
    fprintf(LogFile,"Dump of '%s' for object %d\n",SymFileName,Object);
#endif
    SymFile = fopen(SymFileName, "rb");
    if (SymFile == 0)
        return (2);

    // read in first map definition
    fread(&MapDef, sizeof(MAPDEF), 1, SymFile);
#ifdef DEBUG_SYMDUMP
    Buffer[0] = MapDef.achModName[0];
    fread(&Buffer[1], 1, MapDef.cbModName-1, SymFile);
    Buffer[MapDef.cbModName] = 0x00;
    fprintf(LogFile,"Module name '%s'\n",Buffer);
#endif

    SegOffset = SEGDEFOFFSET(MapDef);
#ifdef DEBUG_SYMDUMP
    fprintf(LogFile,"SegOffset %0x\n",SegOffset);
#endif

    // go thru all segments
    for (SegNum = 0;
         SegNum < MapDef.cSegs;
         SegNum++)
    {
#ifdef DEBUG_SYMDUMP
        fprintf(LogFile,"Scanning segment #%d Offset %08X\n",SegNum,SegOffset);
#endif
        if (fseek(SymFile, SegOffset, SEEK_SET))
            // seek error
            return (3);

        // read in segment definition
        fread(&SegDef, sizeof(SEGDEF), 1, SymFile);
#ifdef DEBUG_SYMDUMP
        Buffer[0] = 0x00;
        if (SegDef.cbSegName>0) {
            Buffer[0] = SegDef.achSegName[0];
            fread(&Buffer[1], 1, SegDef.cbSegName-1, SymFile);
            Buffer[SegDef.cbSegName] = 0x00;
        }
        fprintf(LogFile,"Segment name '%s', number %d, flags %02x\n",Buffer,SegNum,SegDef.bFlags);
#endif

        if (SegNum == Object)
        {
            // stack object found:
            Buffer[0] = 0x00;
            LastVal = 0;

            // go thru all symbols in this object
#ifdef DEBUG_SYMDUMP
            fprintf(LogFile,"Scanning #%d symbols\n",SegDef.cSymbols);
#endif
            for (SymNum = 0; SymNum < SegDef.cSymbols; SymNum++)
            {
                SymPtrOffset=SYMDEFOFFSET(SegOffset,SegDef,SymNum);
                fseek(SymFile,SymPtrOffset,SEEK_SET);
                fread(&SymOffset,sizeof(unsigned short int),1,SymFile);
                fseek(SymFile,SymOffset+SegOffset,SEEK_SET);
                if (SegDef.bFlags & 0x01)
                {
                    // 32-bit symbol:
                    fread(&SymDef32, sizeof(SYMDEF32), 1, SymFile);
                    if (SymDef32.wSymVal > TrapOffset)
                    {
                        // symbol found
                        fprintf(LogFile,
                                "between %s + 0x%lX ",
                                Buffer,
                                TrapOffset - LastVal);
                        /* fprintf(LogFile, "(ppLineDef: 0x%lX) ",
                                    LINEDEFOFFSET(SegDef)
                                    ); */
                        fprintf(LogFile, "\n");
                    }

                    LastVal = SymDef32.wSymVal;
                    Buffer[0] = SymDef32.achSymName[0];
                    fread(&Buffer[1], 1, SymDef32.cbSymName-1, SymFile);
                    Buffer[SymDef32.cbSymName] = 0x00;
#ifdef DEBUG_SYMDUMP
                    fprintf(LogFile,"32 Bit Symbol Address %08p <%s> \n",SymDef32.wSymVal,Buffer);
#endif

                    if (SymDef32.wSymVal > TrapOffset)
                    {
                        // symbol found, as above
                        fprintf(LogFile,
                                "                                     "
                                "and %s - 0x%lX ",
                                Buffer,
                                LastVal - TrapOffset);
                        fprintf(LogFile, "\n");
                        break;
                    }
                }
                else
                {
                    // 16-bit symbol:
                    fread(&SymDef16, sizeof(SYMDEF16), 1, SymFile);
                    if (SymDef16.wSymVal > TrapOffset)
                    {
                        fprintf(LogFile,
                                "between %s + %lX\n",
                                Buffer,
                                TrapOffset - LastVal);
                    }
                    LastVal = SymDef16.wSymVal;
                    Buffer[0] = SymDef16.achSymName[0];
                    fread(&Buffer[1], 1, SymDef16.cbSymName-1, SymFile);
                    Buffer[SymDef16.cbSymName] = 0x00;
                    if (SymDef16.wSymVal > TrapOffset)
                    {
                        fprintf(LogFile,
                                "                                     "
                                "and %s - %lX\n",
                                Buffer,
                                LastVal - TrapOffset);
                        break;
                    }
#ifdef DEBUG_SYMDUMP
                    fprintf(LogFile,"16 Bit Symbol <%s> Address %p\n",Buffer,SymDef16.wSymVal);
#endif
                }               // endif
            }
            break;
        }                       // endif
        SegOffset = NEXTSEGDEFOFFSET(SegDef);
    }                           // endwhile
    fclose(SymFile);
    return (0);         // no error
}

/* ******************************************************************
 *
 *   PART 4: dbgPrintStack
 *
 ********************************************************************/

/*
 *@@ dbgPrintStackFrame:
 *      parses and dumps one stack frame.
 *      Called from excPrintStackFrame.
 *
 *      This calls dbgPrintDebugInfo and, if
 *      that fails, dbgPrintSYMInfo.
 *
 *@@added V0.9.2 (2000-03-10) [umoeller]
 *@@changed V0.9.3 (2000-04-10) [umoeller]: added support for non-Warp 4 SYM files
 *@@changed V0.9.3 (2000-04-26) [umoeller]: this broke Warp 4 FP 13, fixed
 */

BOOL dbgPrintStackFrame(FILE *LogFile,
                        PSZ pszModuleName,  // in: module name (fully q'fied)
                        ULONG ulObject,
                        ULONG ulOffset)
{
    APIRET arc = 0;
    // "Source file"... columns

//YD do not use debug info
#define ENABLE_DEBUG_INFO
#ifdef ENABLE_DEBUG_INFO
    // first attempt to analyze the debug code
    arc = dbgPrintDebugInfo(LogFile,
                            pszModuleName,
                            ulObject,
                            ulOffset);
#else
    arc = 1;
#endif

    // if no debug code is available, analyze
    // the SYM file instead
    if (arc != NO_ERROR)
    {
        CHAR szSymName[CCHMAXPATH];
        strcpy(szSymName, pszModuleName);
        strcpy(szSymName + strlen(szSymName) - 3, "SYM");
        arc = dbgPrintSYMInfo(LogFile,
                              szSymName,
                              ulObject,
                              ulOffset);
        if (arc != 0)
        {
            // SYM file not found in current directory:
            // check the SYM files in the \OS2 directory,
            // depending on the OS/2 version level:
            CHAR szSymFile2[CCHMAXPATH];
            PSZ  pszFilename = strrchr(szSymName, '\\');
            if (pszFilename)
            {
                PSZ         pszVersionDir = "WARP4";
                ULONG       aulBuf[3];

                DosQuerySysInfo(QSV_VERSION_MAJOR,      // 11
                                QSV_VERSION_MINOR,      // 12
                                &aulBuf, sizeof(aulBuf));
                // Warp 3 is reported as 20.30
                // Warp 4 is reported as 20.40
                // Aurora is reported as 20.45

                if (aulBuf[0] == 20)
                {
                    if (aulBuf[1] == 30)
                        // Warp 3:
                        pszVersionDir = "WARP3";
                    else if (aulBuf[1] >= 40)
                        // Warp 4 or higher:
                        // (NOTE: Warp 4 FP 13 now returns 45 also,
                        // but the SYM files are still in the WARP4 directory...)
                        // V0.9.3 (2000-04-26) [umoeller]
                        pszVersionDir = "WARP4";
                }

                pszFilename++;
                sprintf(szSymFile2,
                        "%c:\\OS2\\PDPSI\\PMDF\\%s\\%s",
                        doshQueryBootDrive(),
                        pszVersionDir,
                        pszFilename);
                arc = dbgPrintSYMInfo(LogFile,
                                      szSymFile2,
                                      ulObject,
                                      ulOffset);

                // V0.9.3 (2000-04-26) [umoeller]
                if (    (arc != 0)  // still not found
                     && (aulBuf[1] == 45) // and running Aurora or Warp 4 FP13?
                   )
                {
                    // Warp Server for e-Business (aka Warp 4.5):
                    // we use the SYM files for the UNI kernel,
                    // I have found no way to find out whether
                    // we're running on an SMP kernel
                    sprintf(szSymFile2,
                            "%c:\\OS2\\PDPSI\\PMDF\\%s\\%s",
                            doshQueryBootDrive(),
                            "WARP45_U",
                            pszFilename);
                    arc = dbgPrintSYMInfo(LogFile,
                                          szSymFile2,
                                          ulObject,
                                          ulOffset);
                }
            }
        }

        if (arc == 2)       // file not found
            fprintf(LogFile,
                    "Cannot find symbol file %s\n",
                    szSymName);
        else if (arc != 0)
            fprintf(LogFile,
                    "Error %lu reading symbol file %s\n",
                    arc,
                    szSymName);
    }

    return (arc == NO_ERROR);
}

/*
 *@@ dbgPrintStack:
 *      this takes stack data from the TIB and
 *      context record data structures and tries
 *      to analyse what the different stack frames
 *      point to.
 *
 *      For each stack frame, this calls dbgPrintDebugInfo,
 *      and, if that fails, dbgPrintSYMInfo.
 *
 *      New with V0.84.
 *
 *@@changed V0.9.2 (2000-03-08) [umoeller]: now searching OS2\PDPSI\PMDF for SYM files also
 */

VOID dbgPrintStack(FILE *LogFile,           // in: text log file to write to
                   PUSHORT StackBottom,
                   PUSHORT StackTop,
                   PUSHORT Ebp,
                   PUSHORT ExceptionAddress)
{
    PUSHORT         RetAddr = 0;
    PUSHORT         LastEbp = 0;
    APIRET          rc = 0;
    ULONG           Size = 0,
                    Attr = 0;
    USHORT          Cs = 0,
                    Ip = 0,
                    // Bp,
                    Sp = 0;
    static char     Name[CCHMAXPATH];
    HMODULE         hMod = 0;
    ULONG           ObjNum = 0;
    ULONG           Offset = 0;
    BOOL            fExceptionAddress = TRUE;   // Use Exception Addr 1st time thru

    // Note: we can't handle stacks bigger than 64K for now...
    Sp = (USHORT) (((ULONG) StackBottom) >> 16);
    // Bp = ;

    if (!f32bit)
        Ebp = (PUSHORT) MAKEULONG(((USHORT)(ULONG)Ebp), Sp);

    fprintf(LogFile, "\n\nCall stack:\n");
    fprintf(LogFile, "                                        Source    Line      Nearest\n");
    fprintf(LogFile, "   EBP      Address    Module  Obj#      File     Numbr  Public Symbol\n");
    fprintf(LogFile, " ��������  ��������-  �������� ����  ������������ �����  ������������-\n");

    do
    {
        Size = 10;
        rc = DosQueryMem((PVOID) (Ebp + 2), &Size, &Attr);
        if (rc != NO_ERROR)
        {
            fprintf(LogFile, "Invalid EBP %8.8lX (DosQueryMem returned %lu)\n", (ULONG)Ebp, rc);
            break;
        }
        if (!(Attr & PAG_COMMIT))
        {
            fprintf(LogFile, "Invalid EBP %8.8lX (not committed)\n", (ULONG)Ebp);
            break;
        }
        if (Size < 10)
        {
            fprintf(LogFile, "Invalid EBP %8.8lX (mem block size < 10)\n", (ULONG)Ebp);
            break;
        }

        if (fExceptionAddress)
            RetAddr = ExceptionAddress;
        else
            RetAddr = (PUSHORT) (*((PULONG) (Ebp + 2)));

        if (RetAddr == (PUSHORT) 0x00000053)
        {
            // For some reason there's a "return address" of 0x53 following
            // EBP on the stack and we have to adjust EBP by 44 bytes to get
            // at the real return address.  This has something to do with
            // thunking from 32bits to 16bits...
            // Serious kludge, and it's probably dependent on versions of C(++)
            // runtime or OS, but it works for now!
            Ebp += 22;
            RetAddr = (PUSHORT) (*((PULONG) (Ebp + 2)));
        }

        // Get the (possibly) 16bit CS and IP
        if (fExceptionAddress)
        {
            Cs = (USHORT) (((ULONG) ExceptionAddress) >> 16);
            Ip = (USHORT) (ULONG) ExceptionAddress;
        }
        else
        {
            Cs = *(Ebp + 2);
            Ip = *(Ebp + 1);
        }

        // if the return address points to the stack then it's really just
        // a pointer to the return address (UGH!).
        if (    (USHORT) (((ULONG) RetAddr) >> 16) == Sp
           )
            RetAddr = (PUSHORT) (*((PULONG) RetAddr));

        if (Ip == 0 && *Ebp == 0)
        {
            // End of the stack so these are both shifted by 2 bytes:
            Cs = *(Ebp + 3);
            Ip = *(Ebp + 2);
        }

        // 16bit programs have on the stack:
        //   BP:IP:CS
        //   where CS may be thunked
        //
        //         in dump                 swapped
        //    BP        IP   CS          BP   CS   IP
        //   4677      53B5 F7D0        7746 D0F7 B553
        //
        // 32bit programs have:
        //   EBP:EIP
        // and you'd have something like this (with SP added) (not
        // accurate values)
        //
        //         in dump               swapped
        //      EBP       EIP         EBP       EIP
        //   4677 2900 53B5 F7D0   0029 7746 D0F7 B553
        //
        // So the basic difference is that 32bit programs have a 32bit
        // EBP and we can attempt to determine whether we have a 32bit
        // EBP by checking to see if its 'selector' is the same as SP.
        // Note that this technique limits us to checking stacks < 64K.
        //
        // Soooo, if IP (which maps into the same USHORT as the swapped
        // stack page in EBP) doesn't point to the stack (i.e. it could
        // be a 16bit IP) then see if CS is valid (as is or thunked).
        //
        // Note that there's the possibility of a 16bit return address
        // that has an offset that's the same as SP so we'll think it's
        // a 32bit return address and won't be able to successfully resolve
        // its details.
        if (Ip != Sp)
        {
            if (DosSizeSeg(Cs, &Size) == NO_ERROR)
            {
                RetAddr = (USHORT * _Seg16) MAKEULONG(Ip, Cs);
                f32bit = FALSE;
            }
            else if (DosSizeSeg((Cs << 3) + 7, &Size) == NO_ERROR)
            {
                Cs = (Cs << 3) + 7;
                RetAddr = (USHORT * _Seg16) MAKEULONG(Ip, Cs);
                f32bit = FALSE;
            }
            else
                f32bit = TRUE;
        }
        else
            f32bit = TRUE;


        // "EBP" column
        if (fExceptionAddress)
            fprintf(LogFile, " Trap  ->  ");
        else
            fprintf(LogFile, " %8.8lX  ", (ULONG)Ebp);

        // "Address" column
        if (f32bit)
            fprintf(LogFile, ":%8.8lX  ", (ULONG)RetAddr);
        else
            fprintf(LogFile, "%04.04X:%04.04X  ", Cs, Ip);

        // Version check omitted; the following requires
        // OS/2 2.10 or later (*UM)
        // if (Version[0] >= 20 && Version[1] >= 10)
        {
            // Make a 'tick' sound to let the user know we're still alive
            DosBeep(2000, 10);

            Size = 10;    // Inserted by Kim Rasmussen 26/06 1996 to avoid error 87 when Size is 0

            // "Module"/"Object" columns
            rc = DosQueryMem((PVOID) RetAddr, &Size, &Attr);
            if (rc != NO_ERROR || !(Attr & PAG_COMMIT))
            {
                fprintf(LogFile, "Invalid RetAddr: %8.8lX\n", (ULONG)RetAddr);
                break;          // avoid infinite loops
            }
            else
            {
                rc = DosQueryModFromEIP(&hMod,
                                        &ObjNum,
                                        sizeof(Name), Name,
                                        &Offset,
                                        (PVOID)RetAddr);
                if (    (rc == NO_ERROR)
                     // && (ObjNum != -1)
                   )
                {
                    // static char     szJunk[_MAX_FNAME];
                    static char     szName[_MAX_FNAME];

                    DosQueryModuleName(hMod, sizeof(Name), Name);
                    // _splitpath(Name, szJunk, szJunk, szName, szJunk);

                    // print module and object
                    fprintf(LogFile, "%-8s %04lX  ", szName, ObjNum + 1);

                    if (strlen(Name) > 3)
                    {
                        dbgPrintStackFrame(LogFile,
                                           Name,
                                           ObjNum,
                                           Offset);
                    }
                }
                else
                    fprintf(LogFile,
                            "DosQueryModFromEIP failed, returned %lu\n",
                            rc);
            }
        }

        if (    ((*Ebp) == 0)
             && ((*Ebp + 1) == 0)
           )
        {
            fprintf(LogFile, "End of call stack\n");
            break;
        }

        if (!fExceptionAddress)
        {
            LastEbp = Ebp;

            if (f32bit)
                Ebp = (PUSHORT) *(PULONG) LastEbp;
            else
                Ebp = (PUSHORT) MAKEULONG((*Ebp), Sp);

            if (f32bit)
            {
                dbgPrintVariables(LogFile, (ULONG) Ebp);
            }                   // endif

            if (Ebp < LastEbp)
            {
                fprintf(LogFile, "... lost stack chain - new EBP below previous\n");
                break;
            }
        }
        else
            fExceptionAddress = FALSE;

        Size = 4;
        rc = DosQueryMem((PVOID) Ebp, &Size, &Attr);
        if ((rc != NO_ERROR) || (Size < 4))
        {
            fprintf(LogFile, "... lost stack chain - invalid EBP: %8.8lX\n", (ULONG)Ebp);
            break;
        }
    } while (TRUE);

    fprintf(LogFile, "\n");
}

/*
 *@@ doshQueryBootDrive:
 *      returns the letter of the boot drive as a
 *      single (capital) character, which is useful for
 *      constructing file names using sprintf and such.
 *
 *@@changed V0.9.16 (2002-01-13) [umoeller]: optimized
 */

CHAR doshQueryBootDrive(VOID)
{
    // this can never change, so query this only once
    // V0.9.16 (2002-01-13) [umoeller]
    static CHAR     cBootDrive = '\0';

    if (!cBootDrive)
    {
        ULONG ulBootDrive;
        DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                        &ulBootDrive,
                        sizeof(ulBootDrive));
        cBootDrive = (CHAR)ulBootDrive + 'A' - 1;
    }

    return (cBootDrive);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
