/*************************************************************************
 *
 *  $RCSfile: security.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 13:01:52 $
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

#ifndef _OSL_SECURITY_H_
#define _OSL_SECURITY_H_

#ifndef _RTL_USTRING_H_
#   include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    osl_Security_E_None,
    osl_Security_E_UserUnknown,
    osl_Security_E_WrongPassword,
    osl_Security_E_Unknown,
    osl_Security_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSecurityError;

/** Process handle
    @see osl_loginUser
    @see osl_freeSecurityHandle
    @see osl_executeProcess
*/
typedef void* oslSecurity;

/** Create a security handle for the current user.
    @return a security handle or NULL on failure.
    @see osl_freeSecurityHandle
    @see osl_executeProcess
    @see osl_executeApplication
*/
oslSecurity SAL_CALL osl_getCurrentSecurity(void);

/** Create a security handle for the denoted user.
    Try to log in the user on the local system.
    @param strzUserName [in] denotes the name of the user to logg in.
    @param strPasswd [in] the password for this user.
    @param pSecurity [out] returns the security handle if user could be logged in.
    @return osl_Security_E_None if user could be logged in, otherwise an error-code.
    @see osl_freeSecurityHandle
    @see osl_executeProcess
    @see osl_executeApplication
*/
oslSecurityError SAL_CALL osl_loginUser(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    oslSecurity *pSecurity
    );

/** Create a security handle for the denoted user.
    Try to log in the user on the denoted file server. On success the homedir will be
    the maped drive on this server.
    @param strUserName [in] denotes the name of the user to logg in.
    @param strPasswd [in] the password for this user.
    @param strFileServer [in] denotes the file server on wich the user is logged in.
    @param pSecurity [out] returns the security handle if user could be logged in.
    @return osl_Security_E_None if user could be logged in, otherwise an error-code.
    @see osl_freeSecurityHandle
    @see osl_executeProcess
    @see osl_executeApplication
*/
oslSecurityError SAL_CALL osl_loginUserOnFileServer(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    rtl_uString *strFileServer,
    oslSecurity *pSecurity
    );

/** Query if the user who is denotes by this security has administrator rigths.
    @param Security [in] the security handle for th user.
    @return True, if the user has adminsitrator rights, otherwise false.
*/
sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security);

/** Free the security handle, created by osl_loginUser.
    @param Security [in] the security handle.
    @see osl_loginUser
*/
void SAL_CALL osl_freeSecurityHandle(oslSecurity Security);

/** Get the login ident for the user of this security handle.
    @param Security [in] the security handle.
    @param strIdent [out] the string that receives the ident on success.
    @return True, if the security handle is valid, otherwise False.
*/
sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **strIdent);

/** Get the login name for the user of this security handle.
    @param Security [in] the security handle.
    @param pszName [out] the string that receives the user name on success.
    @return True, if the security handle is valid, otherwise False.
*/
sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **strName);

/** Get the home directory of the user of this security handle.
    @param Security [in] the security handle.
    @param strDirectory [out] the string that receives the directory path on success.
    @return True, if the security handle is valid, otherwise False.
*/
sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **strDirectory);

/** Get the directory for configuration data of the user of this security handle.
    @param Security [in] the security handle.
    @param strDirectory [out] the string that receives the directory path on success.
    @return True, if the security handle is valid, otherwise False.
*/
sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **strDirectory);


/** Load Profile of the User
    Implemented just for Windows
    @param oslSecurity Security [in] previously fetch Security of the User
    @return True if the Profile could successfully loaded, False otherwise.
*/

sal_Bool SAL_CALL osl_loadUserProfile(oslSecurity Security);


/** Unload a User Profile
    Implemented just for Windows
    @param oslSecurity Security [in] previously fetch Security of the User
    @return nothing is returned!
*/

void SAL_CALL osl_unloadUserProfile(oslSecurity Security);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SECURITY_H_ */

