/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:33:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    osl_setCommandArgs(argc, argv);
}

void SAL_CALL sal_detail_deinitialize()
{
    APIRET rc = -1;

    rc = DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD)&(g_excptstruct.RegRec2));
}

}
