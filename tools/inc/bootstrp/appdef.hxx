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



#ifndef _MHAPPDEF_HXX
#define _MHAPPDEF_HXX


#ifdef UNX
#define DEFAULT_INI_ROOT    "/so/env"
#define DEFAULT_BS_ROOT     "/so/env/b_server"
#define PATH_SEPARATOR      '/'
#define S_PATH_SEPARATOR    "/"
#else
#define DEFAULT_INI_ROOT    "r:"
#define DEFAULT_BS_ROOT     "n:"
#define PATH_SEPARATOR      '\\'
#define S_PATH_SEPARATOR    "\\"
#endif

#define _INI_DRV                DEFAULT_INI_ROOT
#define B_SERVER_ROOT           DEFAULT_BS_ROOT
#define _SOLARLIST              B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "solar.lst"
#define _DEF_STAND_LIST         B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "stand.lst"
#define _DEF_SSOLARINI          B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "ssolar.ini"
#define _DEF_SSCOMMON           B_SERVER_ROOT S_PATH_SEPARATOR "config" S_PATH_SEPARATOR "ssolar.cmn"
#define _INIROOT                B_SERVER_ROOT S_PATH_SEPARATOR "config"
#define _INIROOT_OLD            B_SERVER_ROOT S_PATH_SEPARATOR "config"
#define _INI_UNC                "\\\\jumbo2.germany.sun.com\\R-Laufwerk"
#define _INI_UNC_OLD            "\\\\jumbo2.germany.sun.com\\R-Laufwerk"


// path conversion
const char* GetDefStandList();
const char* GetIniRoot();
const char* GetIniRootOld();
const char* GetSSolarIni();
const char* GetSSCommon();
const char* GetBServerRoot();

const char* GetEnv( const char *pVar );
const char* GetEnv( const char *pVar, const char *pDefault );


#endif
