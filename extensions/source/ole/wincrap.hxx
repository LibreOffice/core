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

#ifndef OLE_WIN_HXX
#define OLE_WIN_HXX

/* wrap all includes that need to be wrapped by presys.h/postsys.h here */

// from oleobjw.hxx
// http://stackoverflow.com/questions/5839292/error-c1189-after-installing-visual-studio-2010
#define _WIN32_WINNT 0x0403

#define STRICT

#define _WIN32_DCOM
#if OSL_DEBUG_LEVEL > 0
//#define _ATL_DEBUG_INTERFACES
#endif

#pragma warning (push,1)
#pragma warning (disable:4917)
#pragma warning (disable:4005)
#pragma warning (disable:4548)

#include <tchar.h>
#include <dispex.h>

#include <tools/presys.h>
#include <list>

// from oleobjw.hxx
#include <atlbase.h>
// from jscriptclasses.hxx
extern CComModule _Module;
#include <atlcom.h>

// from unoobjw.cxx
#include <olectl.h>

#include <tools/postsys.h>

#pragma warning (pop)

#endif // OLE_WIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
