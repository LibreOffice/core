/*************************************************************************
 *
 *  $RCSfile: security.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 15:25:47 $
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

#ifndef _VOS_SECURITY_HXX_
#define _VOS_SECURITY_HXX_

#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _RTL_USTRING_
#   include <rtl/ustring>
#endif
#ifndef _OSL_SECURITY_H_
#   include <osl/security.h>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

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

#ifdef _USE_NAMESPACE
}
#endif

#endif  // _VOS_SECURITY_HXX_

