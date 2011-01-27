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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
