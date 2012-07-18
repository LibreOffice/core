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

#ifdef WNT
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif

#ifdef WNT
#include <windows.h>
#include <winldap.h>
#ifndef LDAP_NO_ATTRS
#    define LDAP_NO_ATTRS L"1.1"
#endif
#define CONST_PCHAR_CAST (const PCHAR)
#else // !defined WNT
#include <ldap.h>
#define CONST_PCHAR_CAST
#endif // WNT

#ifndef LDAP_API
#    define LDAP_API(rt) rt
#endif
#ifndef LDAP_CALL
#    define LDAP_CALL
#endif

#ifdef WNT
#pragma warning (pop)
#endif // WNT

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
