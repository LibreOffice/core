/*************************************************************************
 *
 *  $RCSfile: so_env.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-11-26 16:02:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __SO_ENVIRON_HXX__
#define __SO_ENVIRON_HXX__



#ifdef UNIX


#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#endif //end of UNIX


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#define SO_SERVER_PORT 8100


#ifdef UNIX
#define STAROFFICE_VERSION_FILE "/.sversionrc"
#define PLUGIN_EXE_FILE_NAME "nsplugin"
#define STAROFFICE_EXE_FILE_NAME "soffice"
#endif
#ifdef WNT
#define STAROFFICE_VERSION_FILE "\\sversion.ini"
#define PLUGIN_EXE_FILE_NAME "nsplugin.exe"
#define STAROFFICE_EXE_FILE_NAME "soffice.exe"
#endif

#define SECTION_NAME     "Versions"
#define SOFFICE_VERSION  "StarOffice 8"
//#define SOFFICE_VERSION  "SunShine Office 1.0"

#define APP_ERROR 1
#define APP_CHAR  2
#define APP_WCHAR 3

// return the install path of staroffice, return value like "/home/build/staroffice"
const char* findInstallDir();

// return original system library path + "/home/build/staroffice/program"
const char* getNewLibraryPath();

// return SO program dir absolute path, like "/home/build/staroffice/program"
const char* findProgramDir();

// return nsplugin executable absolute path, like "/home/build/staroffice/program/nsplugin"
const char* findNsExeFile();


// return SO executable absolute path, like "/home/build/staroffice/program/soffice"
const char* findSofficeExecutable();


// change Dos path such as c:\program\soffice to c:/program/soffice
int DosToUnixPath(char*);

//change Unix path such as program\soffice to program/soffice
int UnixToDosPath(char*);

void NSP_WriteLog(int level,  const char* pFormat, ...);

int restoreUTF8(char* pPath);

char* NSP_getPluginName();

char* NSP_getPluginDesc();

#endif
