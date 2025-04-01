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

#pragma once


#if defined(_WIN32) // Windows
#include <io.h>
#endif

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>

#include <stdlib.h>
#include <stdio.h>

#if (defined UNX)
#include <unistd.h>
#include <pwd.h>
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/plugin/TestPlugInDefaultImpl.h>

 printf

 1024
const char pTestString[17] = "Sun Microsystems";

OUString aLogonUser, aLogonPasswd, aFileServer, aStringForward;
OUString strUserName, strComputerName, strHomeDirectory;
OUString strConfigDirectory, strUserID;

bool isAdmin = false;



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
