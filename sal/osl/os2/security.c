/*************************************************************************
 *
 *  $RCSfile: security.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:20 $
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

#include "system.h"

#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/profile.h>

#include "secimpl.h"

/*----------------------------------------------------------------------------*/

#define PURE_32
#include "UPM.H"

typedef LSINT (APIENTRY fptr_U32EULGN) (
    unsigned char*,
    unsigned char*,
    unsigned char*,
    unsigned long,
    unsigned long );

typedef LSINT (APIENTRY fptr_U32EULGF) (
    unsigned char*,
    unsigned char*,
    unsigned long );

/* netcons.h */
#define UNLEN  20

/*----------------------------------------------------------------------------*/

static HMODULE hUPM32DLL = 0;

static fptr_U32EULGN* pfU32EULGN = 0;
static fptr_U32EULGF* pfU32EULGF = 0;

/*----------------------------------------------------------------------------*/

sal_Bool ImplLoadUPM( void )
{
    APIRET nRet;

    /* load UPM.DLL */
    nRet = DosLoadModule ((PSZ)0, 0, "UPM32", &hUPM32DLL);
    if( hUPM32DLL == NULL )
    {
        OSL_TRACE( "UPM32.DLL not found: %d\n", nRet );
        return sal_False;
    }

    OSL_TRACE( "UPM32.DLL geladen!\n");

    /* Load function pointer */
    nRet  = DosQueryProcAddr(hUPM32DLL, 0, "U32EULGN", (PFN*)&pfU32EULGN);
    nRet += DosQueryProcAddr(hUPM32DLL, 0, "U32EULGF", (PFN*)&pfU32EULGF);

    if (nRet)
    {
        OSL_TRACE( "Some functions not found; UPM32.DLL freed.\n" );
        ImplFreeUPM();
        return sal_False;
    }

    return sal_True;
}

/*----------------------------------------------------------------------------*/

#ifndef GCC
void ImplFreeUPM( void )
{
    if( hUPM32DLL != 0 )
    {
        OSL_TRACE( "UPM32.DLL entladen!\n");

        DosFreeModule( hUPM32DLL );
        hUPM32DLL = 0;
    }
}
#endif

/*----------------------------------------------------------------------------*/


oslSecurity SAL_CALL osl_getCurrentSecurity()
{
    static oslSecurityImpl currentSecurity = {(HANDLE)-1, "", "", "", 0};

    if (currentSecurity.m_hToken == (HANDLE)-1)
    {
        currentSecurity.m_hToken = (HANDLE)NULL;
        osl_getSystemDir(currentSecurity.m_Directory, sizeof(currentSecurity.m_Directory));
    }

    return (&currentSecurity);
}

/*----------------------------------------------------------------------------*/

oslSecurityError SAL_CALL osl_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
                               oslSecurity* pSecurity)
{
    sal_Char*   pszUser;
    sal_Char*   pszDomain = strdup( pszUserName );
    sal_Char*   pszPassword = strdup( pszPasswd );
    APIRET nRet;
    int nResult;
    oslSecurityError erg = osl_Security_E_None;
    oslSecurityImpl* pSecImpl = malloc(sizeof(oslSecurityImpl));
    sal_uInt16 nLogonFlag;

    /* load UPM module */
    if( !hUPM32DLL )
        if( !ImplLoadUPM() )
        {
            free( pszPassword );
            free( pszDomain );
            return osl_Security_E_UserUnknown;
        }

    /* extract domainname from username */
    if (pszUser = strchr(pszDomain, '/'))
    {
        *pszUser++ = '\0';

        OSL_TRACE( "Logon %s (%s)\n", pszUser, pszDomain );

        pSecImpl->nLogon = UPM_DOMAIN;
        nLogonFlag = UPM_FL_DOMVER;
    }
    else
    {
        pszUser   = pszDomain;
        pszDomain = NULL;

        OSL_TRACE( "Logon %s\n", pszUser );

        pSecImpl->nLogon = UPM_LOCAL;
        nLogonFlag = UPM_FL_LOCVER;
    }

    /* password must be UPPERCASE */
    strupr( pszPassword );

    nResult = pfU32EULGN( (sal_uChar*) pszUser,
                             (sal_uChar*) pszPassword,
                             (sal_uChar*) pszDomain,
                             pSecImpl->nLogon,
                             nLogonFlag );

    if( (nResult == UPM_OK) || (nResult == -1262) ) /* UPM_LOGGED = 0xFB12 */
    {
        erg = osl_Security_E_None;

           OSL_TRACE( "logon %s (pfU32EULGF): %d\n", pszUserName, nResult );

        /* save username */
        strncpy( pSecImpl->m_User, pszUser, USER_BUFFER_SIZE );
        pSecImpl->m_User[ USER_BUFFER_SIZE - 1 ] = 0;

        /* save domainname */
        if( pszDomain )
        {
            strncpy( pSecImpl->m_Domain, pszDomain, DOMAIN_BUFFER_SIZE );
            pSecImpl->m_Domain[ DOMAIN_BUFFER_SIZE - 1 ] = 0;
        }

        /* use default home directory */
        strcpy( pSecImpl->m_Directory, "." );
    }
    else
    {
        OSL_TRACE( "pfU32EULGN failed: %d\n", nResult );
        erg = osl_Security_E_UserUnknown;
    }

    if( osl_Security_E_None == erg )
          *pSecurity = (oslSecurity)pSecImpl;
    else
    {
        *pSecurity = NULL;
        free( pSecImpl );
    }

       free( pszPassword );
    free( pszDomain );

    return erg;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax)
{
    return sal_False;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32 nMax)
{
    if (Security == NULL)
        return sal_False;

    strncpy(pszName, ((oslSecurityImpl*)Security)->m_User, nMax);
       pszName[ nMax - 1 ] = 0;

    OSL_TRACE( "osl_getUserName (%s)\n", pszName );

    return sal_True;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    if (Security == NULL)
        return sal_False;

    strncpy(pszDirectory, ((oslSecurityImpl*)Security)->m_Directory, nMax);

    OSL_TRACE( "osl_getHomeDir (%s)\n", pszDirectory );

    return sal_True;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_getSystemDir(sal_Char* pszDirectory, sal_uInt32 nMax)
{
    ULONG driveBuffer;
    APIRET apiret = DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &driveBuffer, sizeof(driveBuffer));

    if ((apiret != NO_ERROR) || (nMax <= strlen("c:\\OS2")))
        return (sal_False);

    pszDirectory[0] = 'A' + (driveBuffer - 1);
    sprintf(pszDirectory + 1, ":\\OS2");
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    if (Security == NULL)
        return sal_False;

    strncpy(pszDirectory, ((oslSecurityImpl*)Security)->m_Directory, nMax);

    OSL_TRACE( "osl_getConfigDir (%s)\n", pszDirectory );

    return sal_True;
}

/*----------------------------------------------------------------------------*/

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    if (Security != osl_getCurrentSecurity())
    {
        oslSecurityImpl* pSecImpl = (oslSecurityImpl*)Security;

        if( pSecImpl )
        {
            if( hUPM32DLL  )
            {
                int nResult;

            /* logout user */
            nResult = pfU32EULGF( (sal_uChar*) pSecImpl->m_User,
                                  pSecImpl->nLogon == UPM_DOMAIN
                                    ? (sal_uChar*) pSecImpl->m_Domain
                                    : NULL,
                                  pSecImpl->nLogon );
            OSL_TRACE( "logoff %s (pfU32EULGF): %d\n", pSecImpl->m_User, nResult );
            }

            free ((oslSecurityImpl*)Security);
        }
    }
}

/*----------------------------------------------------------------------------*/

oslSecurityError SAL_CALL osl_loginUserOnFileServer(    const sal_Char*  pszUserName,
                                               const sal_Char*  pszPasswd,
                                               const sal_Char*  pszFileServer,
                                               oslSecurity* pSecurity)
{
    return osl_Security_E_UserUnknown;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    return sal_True;
}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_loadUserProfile(oslSecurity Security)
{
    return sal_False;
}

/*----------------------------------------------------------------------------*/

void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    return;
}

