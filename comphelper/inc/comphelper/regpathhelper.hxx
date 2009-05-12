/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regpathhelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#define _COMPHELPER_REGPATHHELPER_HXX_

#include <rtl/ustring.hxx>

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

