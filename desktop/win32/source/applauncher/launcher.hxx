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
#ifndef __cplusplus
#error Need C++ to compile
#endif

#include <windows.h>
#include <winbase.h>
#include <windef.h>
#include <shlwapi.h>

#ifndef _INC_TCHAR
#   ifdef UNICODE
#       define _UNICODE
#   endif
#   include <tchar.h>
#endif

#ifdef UNICODE
#   define GetArgv( pArgc )         CommandLineToArgvW( GetCommandLine(), pArgc )
#else
#   define GetArgv( pArgc )         (*pArgc = __argc, __argv)
#endif

#define OFFICE_IMAGE_NAME   _T("soffice")

extern _TCHAR APPLICATION_SWITCH[];

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
