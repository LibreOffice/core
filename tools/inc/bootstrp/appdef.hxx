/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appdef.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 08:08:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
