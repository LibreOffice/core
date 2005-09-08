/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regpathhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:38:04 $
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

#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#define _COMPHELPER_REGPATHHELPER_HXX_

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace comphelper
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

