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

#ifndef _VOS_SECURITY_HXX_
#define _VOS_SECURITY_HXX_

#   include <vos/object.hxx>
#   include <rtl/ustring.hxx>
#   include <osl/security.h>

namespace vos
{

/** capsulate security informations for one user.
    A object of this class is used to execute a process with the rights an
    security options of a scecified user.
    @see OProcess::executeProcess
    @author  Bernd Hofner
    @version 1.0

*/
class OSecurity : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OSecurity, vos));

public:
    /// constructor
    OSecurity();
    /// destructor
    virtual ~OSecurity();
    /** get the security information for one user.
        The underlying operating system is asked for this information.
        @param strName [in] denotes the name of the user
        @param strPasswd [in] denotes the password of this user
        @return True, if the specified user is known by the underlying operating system,
        otherwise False
    */
    sal_Bool SAL_CALL logonUser(const ::rtl::OUString& strName,
                                const ::rtl::OUString& strPasswd);
    /** get the security information for one user.
        This method will try to login the user at the denoted file server.
        If a network resource named \\server\username exists and this resource
        could be connected by this user, the methos will return true and getHomeDir
        will return \\server\username.
        @param strName [in] denotes the name of the user
        @param strPasswd [in] denotes the password of this user
        @return True, if the specified user is known by file server and the
        could be connected, otherwise False
    */
    sal_Bool SAL_CALL logonUser(const ::rtl::OUString& strName,
                                const ::rtl::OUString& strPasswd,
                                const ::rtl::OUString& strFileServer);

    /** get the ident of the logged in user.
        @param strName [out] is the buffer which returns the name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    sal_Bool SAL_CALL getUserIdent(::rtl::OUString& strIdent) const;

    /** get the name of the logged in user.
        @param strName [out] is the buffer which returns the name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    sal_Bool SAL_CALL getUserName(::rtl::OUString& strName) const;

    /** get the home directory of the logged in user.
        @param strDirectory [out] is the buffer which returns the directory name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    sal_Bool SAL_CALL getHomeDir(::rtl::OUString& strDirectory) const;

    /** get the directory for configuration data of the logged in user.
        @param strDirectory [out] is the buffer which returns the directory name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    sal_Bool SAL_CALL getConfigDir(::rtl::OUString& strDirectory) const;

    /** Query if the user who is logged inhas administrator rigths.
        @return True, if the user has administrator rights, otherwise false.
    */
    sal_Bool SAL_CALL isAdministrator() const;

    virtual SAL_CALL operator oslSecurity() const;

protected:

    oslSecurity m_oslSecurity;
};

}

#endif  // _VOS_SECURITY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
