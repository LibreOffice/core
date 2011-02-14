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

#ifndef _MHAPPDEF_HXX
#define _MHAPPDEF_HXX


#ifdef UNX
#define DEFAULT_INI_ROOT    "/so/env"
#define DEFAULT_BS_ROOT     "/so/env/b_server"
#define DEFAULT_EIS_ROOT    "/so/env/merge"
#define PATH_SEPARATOR      '/'
#define S_PATH_SEPARATOR    "/"
#else
#define DEFAULT_INI_ROOT    "r:"
#define DEFAULT_BS_ROOT     "n:"
#define DEFAULT_EIS_ROOT    "w:"
#define PATH_SEPARATOR      '\\'
#define S_PATH_SEPARATOR    "\\"
#endif

#define _INI_DRV                DEFAULT_INI_ROOT
#define B_SERVER_ROOT           DEFAULT_BS_ROOT
#define DEFAULT_INI_FILE        B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "b_server.ini"
#define BINARYROOT              B_SERVER_ROOT S_PATH_SEPARATOR "bin"
#define _JOB_DIR                B_SERVER_ROOT S_PATH_SEPARATOR "server" S_PATH_SEPARATOR "db"
#define _INJOB_DIR              B_SERVER_ROOT S_PATH_SEPARATOR "server" S_PATH_SEPARATOR "dbin"
#define _OUTJOB_DIR             B_SERVER_ROOT S_PATH_SEPARATOR "server" S_PATH_SEPARATOR "dbout"
#define _ERRJOB_DIR             B_SERVER_ROOT S_PATH_SEPARATOR "server" S_PATH_SEPARATOR "dberr"
#define _BUILDPATTERN_DIR       B_SERVER_ROOT S_PATH_SEPARATOR "pattern"
#define _COMMENT_DIR            B_SERVER_ROOT S_PATH_SEPARATOR "comment"
#define _SOLARLIST              B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "solar.lst"
#define _DEF_STAND_LIST         B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "stand.lst"
#define _DEF_DEFAULT_LIST       B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "default.lst"
#define _DEF_SSOLARINI          B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "ssolar.ini"
#define _DEF_SSCOMMON           B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "ssolar.cmn"
#define _SERVER_IDENT_FLAG      B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "server.id"
#define _CUSTOMJOBINI           B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "custom.ini"
#define _BUILDCOMMANDINI        B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "bcommand.ini"
#define _DATABASEINI            B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "database.ini"
#define _ZNINI                  B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "bszn.ini"
#define _ERRORINFOFILE          B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "errinf.lst"
#define _REGEXPINFOFILE         B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "regexp.lst"
#define _POSITIVERRORINFOFILE   B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "perrinf.lst"
#define _POSITIVREGEXPINFOFILE  B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "pregexp.lst"
#define _INIROOT                B_SERVER_ROOT S_PATH_SEPARATOR "config"
#define _INIROOT_OLD            B_SERVER_ROOT S_PATH_SEPARATOR "config"
#define _ENV_DIR                B_SERVER_ROOT S_PATH_SEPARATOR "config"
#define _REDIRECTIONPATH        B_SERVER_ROOT S_PATH_SEPARATOR "redir"
#define _ZLOGSPATH              B_SERVER_ROOT S_PATH_SEPARATOR "zlogs"
#define DEFAULT_PROTPATH        B_SERVER_ROOT S_PATH_SEPARATOR "filesize"
#define _INI_UNC                "\\\\jumbo2.germany.sun.com\\R-Laufwerk"
#define _INI_UNC_OLD            "\\\\jumbo2.germany.sun.com\\R-Laufwerk"
#define LOG_DIR                 B_SERVER_ROOT S_PATH_SEPARATOR "log"

#define _REDIRECTIONSIZE        "10000"
#define _EISPATH                DEFAULT_EIS_ROOT S_PATH_SEPARATOR "EIS"

#define _BUILDEVENTPATH         _EISPATH S_PATH_SEPARATOR "EventImport"

#define _WRITE_LOGFILES         "1"
#define _WRITE_JOBFILES         "1"

#define _VERSION_WINDOW         "0"
#define _JOB_WINDOW             "0"
#define _APPWIN_MAXIMIZED       "0"
#define _CONTEXT_TBOX           "0"

#define _COLOR_DROPED           COL_GRAY
#define _COLOR_WAIT             COL_BLUE
#define _COLOR_BUILD            COL_GREEN
#define _COLOR_ERROR            COL_RED
#define _COLOR_BUILDERROR       COL_MAGENTA
#define _COLOR_OK               COL_BLACK
#define _COLOR_DELIVERED        COL_CYAN
#define _COLOR_MASTERSTOP       COL_LIGHTRED

#define _ASYNCHRON              "1"

// folowing defines are used to write filesize information for binaries
#define OH_GOTT "*"
#define UNX_DEFAULT_FILESIZES       "bin/" OH_GOTT ".bin;bin/" OH_GOTT ".res;bin/" OH_GOTT ".tlb;lib/" OH_GOTT ".so"
#define DOS_DEFAULT_FILESIZES       "bin\\*.exe;bin\\*.res;bin\\*.dll;bin\\*.tlb"

#ifdef UNX
#define DEFAULT_FILESIZES UNX_DEFAULT_FILESIZES
#else
#define DEFAULT_FILESIZES DOS_DEFAULT_FILESIZES
#endif

/** States for BuildServer projects and directories
*/
#ifndef BuildStatus
#define BuildStatus sal_uInt16
#endif
#define BS_DROPED                   0   /// project is waiting for dispatching
#define BS_BUILD                    1   /// project/directory is building
#define BS_WAIT                     2   /// project/directory is waiting for building
#define BS_OK                       3   /// project/directory builded whithout errors
#define BS_BUILDANDERROR            4   /// project is in build, any errors accured
#define BS_ERROR                    5   /// project/directory builded with errors
#define BS_NON                      6   /// no state is set
#define BS_DELIVERED                7   /// project was delivered without errors
#define BS_DELIVEREDANDERROR        8   /// project was delivered with errors
#define BS_FIXED                    9   /// errors are fixed
#define BS_NOTDELIVERED            10   /// project is ready to deliver
#define BS_BUILD_PRIO              11   /// build projects with prio
#define BS_BUILD_ORDER             12   /// build project in correct order
#define BS_BUILD_DEPEND            13   /// build projects using dependencies
#define BS_BUILD_DELIVER           14   /// build projects using dependencies and deliver builded projects
#define BS_BUILD_NORMAL            15   /// build projects without using dependencies
#define BS_BUILD_DELIVER_NO_DEPEND 16   /// build projects without using dependencies and deliver builded projects
#define BS_BUILD_DOUBLE_LOG        17   /// error with double log names

#define LF_STARDIV  "StarDivision"
#define LF_UNIX     "UNIX"
#define LF_MAC      "Macintosh"
#define LF_OS2      "OS/2"

// path conversion
const char* GetDefStandList();
const char* GetIniRoot();
const char* GetIniRootOld();
const char* GetSSolarIni();
const char* GetSSCommon();
const char* GetBServerRoot();

const char* GetEnv( const char *pVar );
const char* GetEnv( const char *pVar, const char *pDefault );

#define CONVERT_R_TO_HOSTFSYS( sPath )  { sPath.ToLowerAscii().SearchAndReplace( "r:", GetEnv("ISERVERBASE", DEFAULT_INI_ROOT ) ); sPath.SearchAndReplaceAll( "\\", S_PATH_SEPARATOR ); }

// Only for usage inside IServer!
#define ISERVER_DEF_STAND_LIST  "r:\\b_server\\config\\stand.lst"
#define ISERVER_DEF_DEPEND_LIST "r:\\b_server\\config\\depend.lst"

#endif
