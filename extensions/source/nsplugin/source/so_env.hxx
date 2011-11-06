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
#define STAROFFICE_EXE_FILE_NAME "soffice"
#endif
#ifdef WNT
#define STAROFFICE_VERSION_FILE "\\sversion.ini"
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

// return SO program dir absolute path, like "/home/build/staroffice/program"
const char* findProgramDir();

#ifdef WNT
// return SO executable absolute path, like "/home/build/staroffice/program/soffice"
const char* findSofficeExecutable();

// change Dos path such as c:\program\soffice to c:/program/soffice
int DosToUnixPath(char*);
#endif

//change Unix path such as program\soffice to program/soffice
int UnixToDosPath(char*);

void NSP_WriteLog(int level,  const char* pFormat, ...);

int restoreUTF8(char* pPath);

char* NSP_getPluginName();

char* NSP_getPluginDesc();

#endif
