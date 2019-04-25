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

#include <unotools/unotoolsdllapi.h>
#include <unotools/syslocale.hxx>
#include <rtl/ustring.hxx>

class LanguageTag;

typedef OUString (*ResHookProc)(const OUString& rStr);

namespace Translate
{
    UNOTOOLS_DLLPUBLIC std::locale Create(const sal_Char* pPrefixName, const LanguageTag& rLocale = SvtSysLocale().GetUILanguageTag());
    UNOTOOLS_DLLPUBLIC OUString get(const char* pId, const std::locale &loc);
    UNOTOOLS_DLLPUBLIC OUString nget(const char* pId, int n, const std::locale &loc);
    UNOTOOLS_DLLPUBLIC void SetReadStringHook( ResHookProc pProc );
    UNOTOOLS_DLLPUBLIC ResHookProc GetReadStringHook();
    UNOTOOLS_DLLPUBLIC OUString ExpandVariables(const OUString& rString);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
