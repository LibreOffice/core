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

#ifndef _OSL_SECURITY_DECL_HXX_
#define _OSL_SECURITY_DECL_HXX_

#include <rtl/ustring.hxx>
#   include <osl/security.h>

namespace osl
{

/** capsulate security information for one user.
    A object of this class is used to execute a process with the rights an
    security options of a scecified user.
    @see Process::executeProcess
*/
class Security
{
protected:
    oslSecurity m_handle;

public:
    /// constructor
    inline Security();
    /// destructor
    inline ~Security();
    /** get the security information for one user.
        The underlying operating system is asked for this information.
        @param[in] strName denotes the name of the user
        @param[in] strPasswd denotes the password of this user
        @return True, if the specified user is known by the underlying operating system,
        otherwise False
    */
    inline sal_Bool SAL_CALL logonUser(const rtl::OUString& strName,
                                       const rtl::OUString& strPasswd);
    /** get the security information for one user.

        @verbatim
        This method will try to login the user at the denoted file server.
        If a network resource named \\server\username exists and this resource
        could be connected by this user, the methos will return true and getHomeDir
        will return \\server\username.
        @endverbatim
        @param[in] strName denotes the name of the user
        @param[in] strPasswd denotes the password of this user
        @param[in] strFileServer denotes the file server to login to
        @return True, if the specified user is known by file server and the
        could be connected, otherwise False
    */
    inline sal_Bool SAL_CALL logonUser(const rtl::OUString & strName,
                                       const rtl::OUString & strPasswd,
                                       const rtl::OUString & strFileServer);

    /** get the ident of the logged in user.
        @param[out] strIdent is the OUString which returns the name
        @return True, if any user is successfully logged in, otherwise False
    */
    inline sal_Bool  SAL_CALL getUserIdent( rtl::OUString& strIdent) const;

    /** get the name of the logged in user.
        @param[out] strName is the OUString which returns the name
        @return True, if any user is successfully logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getUserName( rtl::OUString& strName) const;

    /** get the home directory of the logged in user.
        @param[out] strDirectory is the OUString which returns the directory name
         @return True, if any user is successfully logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getHomeDir( rtl::OUString& strDirectory) const;

    /** get the directory for configuration data of the logged in user.
        @param[out] strDirectory is the OUString which returns the directory name
        @return True, if any user is successfully logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getConfigDir( rtl::OUString & strDirectory) const;

    /** Query if the user who is logged inhas administrator rigths.
        @return True, if the user has administrator rights, otherwise false.
    */
    inline sal_Bool SAL_CALL isAdministrator() const;

    /** Returns the underlying oslSecurity handle
     */
    inline oslSecurity getHandle() const;

};

}

#endif  // _OSL_SECURITY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
