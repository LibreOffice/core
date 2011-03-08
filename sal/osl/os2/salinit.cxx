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

#define INCL_DOS
#include <os2.h>

#include "precompiled_sal.hxx"
#include "sal/config.h"

#include "osl/process.h"
#include "sal/main.h"
#include "sal/types.h"

// for exception logging
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "helpers/except.h"

extern "C" {

/*----------------------------------------------------------------------------*/

static CHAR        szOOoExe[CCHMAXPATH];

static FILE* APIENTRY _oslExceptOpenLogFile(VOID)
{
   FILE        *file;
   DATETIME    DT;
   PPIB        pib;
   PSZ         slash;

   // get executable fullpath
   DosGetInfoBlocks(NULL, &pib);
   DosQueryModuleName(pib->pib_hmte, sizeof(szOOoExe), szOOoExe);
   // truncate to exe name
   slash = (PSZ)strrchr( szOOoExe, '.');
   *slash = '\0';
   // make log path
   strcat( szOOoExe, ".log");

   file = fopen( szOOoExe, "a");
   if (!file) { // e.g. readonly drive
      // try again, usually C exist and is writable
      file = fopen( "c:\\OOo.log", "a");
   }
   if (file) {
        DosGetDateTime(&DT);
        fprintf(file, "\nTrap message -- Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
            DT.year, DT.month, DT.day,
            DT.hours, DT.minutes, DT.seconds);
        fprintf(file, "-------------------------------------------------------\n"
                      "\nAn internal error occurred (Built " __DATE__ "-" __TIME__ ").\n");

   }

   // ok, return handle
   return (file);
}

/*----------------------------------------------------------------------------*/

static EXCEPTSTRUCT     g_excptstruct = {0};

void SAL_CALL sal_detail_initialize(int argc, char ** argv)
{
    APIRET rc = -1;

#if OSL_DEBUG_LEVEL == 0
    excRegisterHooks(_oslExceptOpenLogFile, NULL, NULL, FALSE);

    g_excptstruct.RegRec2.pfnHandler = (PFN)excHandlerLoud;
    g_excptstruct.arc = DosSetExceptionHandler(
               (PEXCEPTIONREGISTRATIONRECORD)&(g_excptstruct.RegRec2));

    if (g_excptstruct.arc)
        if (G_pfnExcHookError)
            G_pfnExcHookError(__FILE__, __LINE__, __FUNCTION__, g_excptstruct.arc);
        else
            DosBeep(1000, 1000);
    g_excptstruct.ulExcpt = setjmp(g_excptstruct.RegRec2.jmpThread);
#endif

    osl_setCommandArgs(argc, argv);
}

void SAL_CALL sal_detail_deinitialize()
{
    APIRET rc = -1;

#if OSL_DEBUG_LEVEL == 0
    rc = DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD)&(g_excptstruct.RegRec2));
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
