/*************************************************************************
 *
 *  $RCSfile: appdef.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:58 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <appdef.hxx>

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

