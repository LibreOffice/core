/*************************************************************************
 *
 *  $RCSfile: security.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
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

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.7  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.6  2000/05/17 14:50:15  mfe
*    comments upgraded
*
*    Revision 1.5  2000/03/31 16:55:43  rs
*    UNICODE-Changes
*
*    Revision 1.4  2000/03/16 16:43:53  obr
*    Unicode API changes
*
*    Revision 1.3  1999/11/18 09:44:26  mfe
*    loadUserprofile moved from process.c to security.c
*
*    Revision 1.2  1999/10/27 15:02:08  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:20  jsc
*    verschoben aus osl
*
*    Revision 1.13  1999/08/05 11:14:41  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.12  1999/07/09 15:35:33  br
*    BR: fuer win16
*
*    Revision 1.11  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.10  1998/09/18 15:33:41  rh
*    #56761# neue locations für cfg/ini files
*
*    Revision 1.9  1998/03/13 15:07:32  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*    Revision 1.8  1998/02/16 19:34:51  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.7  1997/10/21 14:22:21  ts
*    Abfrage, ob ein Benutzer Adminstratorrechte hat eingebaut
*
*    Revision 1.6  1997/10/17 16:00:44  ts
*    osl_logonUserOnFileServer hinzugefuegt
*
*    Revision 1.5  1997/10/14 07:52:51  fm
*    *** empty log message ***
*
*    Revision 1.4  1997/07/31 15:28:39  ts
*    *** empty log message ***
*
*    Revision 1.3  1997/07/22 14:29:30  rh
*    process added
*
*    Revision 1.2  1997/07/17 19:01:45  ts
*    *** empty log message ***
*
*    Revision 1.1  1997/07/17 11:22:18  ts
*    *** empty log message ***
*
*************************************************************************/

