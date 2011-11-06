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
