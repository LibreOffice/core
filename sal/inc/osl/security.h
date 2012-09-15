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

#ifndef _OSL_SECURITY_H_
#define _OSL_SECURITY_H_

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"

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
SAL_DLLPUBLIC oslSecurity SAL_CALL osl_getCurrentSecurity(void);

/** Deprecated API
    Create a security handle for the denoted user.
    Try to log in the user on the local system.
    @param[in] strUserName denotes the name of the user to logg in.
    @param[in] strPasswd the password for this user.
    @param[out] pSecurity returns the security handle if user could be logged in.
    @return osl_Security_E_None if user could be logged in, otherwise an error-code.
    @see osl_freeSecurityHandle
    @see osl_executeProcess
    @see osl_executeApplication
*/
SAL_DLLPUBLIC oslSecurityError SAL_CALL osl_loginUser(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    oslSecurity *pSecurity
    );

/** Create a security handle for the denoted user.
    Try to log in the user on the denoted file server. On success the homedir will be
    the maped drive on this server.
    @param[in] strUserName denotes the name of the user to logg in.
    @param[in] strPasswd the password for this user.
    @param[in] strFileServer denotes the file server on wich the user is logged in.
    @param[out] pSecurity returns the security handle if user could be logged in.
    @return osl_Security_E_None if user could be logged in, otherwise an error-code.
    @see osl_freeSecurityHandle
    @see osl_executeProcess
    @see osl_executeApplication
*/
SAL_DLLPUBLIC oslSecurityError SAL_CALL osl_loginUserOnFileServer(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    rtl_uString *strFileServer,
    oslSecurity *pSecurity
    );

/** Query if the user who is denotes by this security has administrator rigths.
    @param[in] Security the security handle for th user.
    @return True, if the user has adminsitrator rights, otherwise false.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isAdministrator(
        oslSecurity Security);

/** Free the security handle, created by osl_loginUser or osl_getCurrentSecurity.
    @param[in] Security the security handle.
    @see osl_loginUser
*/
SAL_DLLPUBLIC void SAL_CALL osl_freeSecurityHandle(
        oslSecurity Security);

/** Get the login ident for the user of this security handle.
    @param[in] Security the security handle.
    @param[out] strIdent the string that receives the ident on success.
    @return True, if the security handle is valid, otherwise False.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getUserIdent(
        oslSecurity Security, rtl_uString **strIdent);

/** Get the login name for the user of this security handle.
    @param[in] Security the security handle.
    @param[out] strName the string that receives the user name on success.
    @return True, if the security handle is valid, otherwise False.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getUserName(
        oslSecurity Security, rtl_uString **strName);

/** Get the home directory of the user of this security handle.
    @param[in] Security the security handle.
    @param[out] strDirectory the string that receives the directory path on success.
    @return True, if the security handle is valid, otherwise False.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getHomeDir(
        oslSecurity Security, rtl_uString **strDirectory);

/** Get the directory for configuration data of the user of this security handle.
    @param[in] Security the security handle.
    @param[out] strDirectory the string that receives the directory path on success.
    @return True, if the security handle is valid, otherwise False.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getConfigDir(
        oslSecurity Security, rtl_uString **strDirectory);


/** Load Profile of the User
    Implemented just for Windows
    @param[in] Security previously fetch Security of the User
    @return True if the Profile could successfully loaded, False otherwise.
*/

SAL_DLLPUBLIC sal_Bool SAL_CALL osl_loadUserProfile(
        oslSecurity Security);


/** Unload a User Profile
    Implemented just for Windows
    @param[in] Security previously fetch Security of the User
    @return nothing is returned!
*/

SAL_DLLPUBLIC void SAL_CALL osl_unloadUserProfile(
        oslSecurity Security);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SECURITY_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
