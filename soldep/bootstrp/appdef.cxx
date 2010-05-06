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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <soldep/appdef.hxx>

const char* GetDefStandList()
{
    char* pRet;
    char* pEnv = getenv("STAR_STANDLST");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( _DEF_STAND_LIST );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, _DEF_STAND_LIST );
    }
    return pRet;
}


const char* GetIniRoot()
{
    char* pRet;
    char* pEnv = getenv("STAR_INIROOT");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( _INIROOT );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, _INIROOT );
    }
    return pRet;
}

const char* GetIniRootOld()
{
    char* pRet;
    char* pEnv = getenv("STAR_INIROOTOLD");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( _INIROOT_OLD );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, _INIROOT_OLD );
    }
    return pRet;
}

const char* GetSSolarIni()
{
    char* pRet;
    char* pEnv = getenv("STAR_SSOLARINI");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( _DEF_SSOLARINI );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, _DEF_SSOLARINI );
    }
    return pRet;
}


const char* GetSSCommon()
{
    char* pRet;
    char* pEnv = getenv("STAR_SSCOMMON");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( _DEF_SSCOMMON );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, _DEF_SSCOMMON );
    }
    return pRet;
}


const char* GetBServerRoot()
{
    char* pRet;
    char* pEnv = getenv("STAR_BSERVERROOT");
    if ( pEnv )
    {
        int nLen = strlen( pEnv );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, pEnv );
    }
    else
    {
        int nLen = strlen( B_SERVER_ROOT );
        pRet = ( char *) malloc( nLen + 1 );
        (void) strcpy( pRet, B_SERVER_ROOT );
    }
    return pRet;
}

const char* GetEnv( const char *pVar )
{
    char *pRet = getenv( pVar );
    if ( !pRet )
        pRet = "";
    return pRet;
}

const char* GetEnv( const char *pVar, const char *pDefault )
{
    char *pRet = getenv( pVar );
    if ( !pRet )
        return pDefault;
    return pRet;
}
