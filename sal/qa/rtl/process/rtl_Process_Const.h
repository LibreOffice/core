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

#ifndef _RTL_PROCESS_CONST_H_
#define _RTL_PROCESS_CONST_H_

#include <rtl/ustring.hxx>

using namespace ::rtl;

#ifdef __cplusplus
extern "C"
{
#endif

::rtl::OUString suParam0(RTL_CONSTASCII_USTRINGPARAM("-join"));
::rtl::OUString suParam1(RTL_CONSTASCII_USTRINGPARAM("-with"));
::rtl::OUString suParam2(RTL_CONSTASCII_USTRINGPARAM("-child"));
::rtl::OUString suParam3(RTL_CONSTASCII_USTRINGPARAM("-process"));

#ifdef __cplusplus
}
#endif

#endif /* RTL_PROCESS_CONST_H*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
