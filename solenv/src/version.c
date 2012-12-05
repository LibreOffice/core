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

#include <_version.h>

struct VersionInfo
{
    const char* pTime;
    const char* pDate;
    const char* pUpd;
    const char* pMinor;
    const char* pBuild;
    const char* pInpath;
};

static const struct VersionInfo g_aVersionInfo =
{
    __TIME__,
    __DATE__,
    _UPD,
    _LAST_MINOR,
    _BUILD,
    _INPATH
};

#if defined(WNT)
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void);
#endif

#if defined(WNT)
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void)
#else
const struct VersionInfo *GetVersionInfo(void)
#endif
{
    return &g_aVersionInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
