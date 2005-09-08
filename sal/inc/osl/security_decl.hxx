/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: security_decl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:32:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_SECURITY_DECL_HXX_
#define _OSL_SECURITY_DECL_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_SECURITY_H_
#   include <osl/security.h>
#endif

namespace osl
{

/** capsulate security informations for one user.
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
        @param strName [in] denotes the name of the user
        @param strPasswd [in] denotes the password of this user
        @return True, if the specified user is known by the underlying operating system,
        otherwise False
    */
    inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString& strName,
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
    inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString & strName,
                                       const ::rtl::OUString & strPasswd,
                                       const ::rtl::OUString & strFileServer);

    /** get the ident of the logged in user.
        @param strName [out] is the buffer which returns the name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    inline sal_Bool  SAL_CALL getUserIdent( ::rtl::OUString& strIdent) const;

    /** get the name of the logged in user.
        @param strName [out] is the buffer which returns the name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getUserName( ::rtl::OUString& strName) const;

    /** get the home directory of the logged in user.
        @param strDirectory [out] is the buffer which returns the directory name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getHomeDir( ::rtl::OUString& strDirectory) const;

    /** get the directory for configuration data of the logged in user.
        @param strDirectory [out] is the buffer which returns the directory name
        @param max [in] is the size of this buffer
        @return True, if any user is successfuly logged in, otherwise False
    */
    inline sal_Bool SAL_CALL getConfigDir( ::rtl::OUString & strDirectory) const;

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

