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
