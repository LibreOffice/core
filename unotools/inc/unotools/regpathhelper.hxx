/*************************************************************************
 *
 *  $RCSfile: regpathhelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:53 $
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

#ifndef _UNOTOOLS_REGPATHHELPER_HXX_
#define _UNOTOOLS_REGPATHHELPER_HXX_

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace utl
{

/**
 * This function returns a path to the user registry file.
 * Search for the user registry using the following rules:
 * <dl>
 * <dt> 1. (for further use) search in sversion.ini (.sversionrc) for an entry
 *      REGISTRY_VERSION_STRING (example: UserRegistry 5.0/505=test.rdb) in the section
 *      [Registry]. If found, then take this value instead of the name "user.rdb".
 * <dt> 2. Search in the config directory of the user for a file "user.rdb". If
 *      found return the full path and name of the file. If not found, retry this
 *      step with a dot before ".user.rdb".
 * <dt> 3. If not found a new user registry with name "user.rdb" will be created in the user
 *      config directory.
 * </dl>
 *<BR>
 * @author Juergen Schmidt
 */
::rtl::OUString getPathToUserRegistry();

/**
 * This function returns a path to the system registry file.
 * The system registry will always be searched in the same directory of the
 * executable. The name of the system registry is "applicat.rdb". If the system
 * registry was not found, then the environment variable STAR_REGISTRY will be checked.
 * If this variable was set, it must contain a full path to a valid system registry.
 * Search for the user registry using the following rules:
 *
 *<BR>
 * @author Juergen Schmidt
 */

::rtl::OUString getPathToSystemRegistry();

}

#endif

