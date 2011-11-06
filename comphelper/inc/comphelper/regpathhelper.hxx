/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

