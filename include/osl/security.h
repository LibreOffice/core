/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OSL_SECURITY_H
#define INCLUDED_OSL_SECURITY_H

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
    @param[in] strUserName denotes the name of the user to log in.
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
    the mapped drive on this server.
    @param[in] strUserName denotes the name of the user to log in.
    @param[in] strPasswd the password for this user.
    @param[in] strFileServer denotes the file server on which the user is logged in.
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

/** Query if the user who is denotes by this security has administrator rights.
    @param[in] Security the security handle for th user.
    @return True, if the user has administrator rights, otherwise false.
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

/** Get the login name for the user of this security handle,
    excluding the domain name on Windows.
    @param[in] Security the security handle.
    @param[out] strName the string that receives the user name on success.
    @return True, if the security handle is valid, otherwise False.
    @since LibreOffice 5.2
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getShortUserName(
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

#endif // INCLUDED_OSL_SECURITY_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
