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

/*******************************************************************
 Includes
 ******************************************************************/

#include "path_helper.hxx"
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <algorithm>
#include <wchar.h>

/*******************************************************************
 Constants
 ******************************************************************/

const rtl::OUString BACKSLASH ("\\");
const rtl::OUString SLASH     ("/");

/*******************************************************************
 osl_systemPathEnsureSeparator
 ******************************************************************/

void osl_systemPathEnsureSeparator(/*inout*/ rtl_uString** ppustrPath)
{
    OSL_PRECOND(ppustrPath && (NULL != *ppustrPath), \
                "osl_systemPathEnsureSeparator: Invalid parameter");

     rtl::OUString path(*ppustrPath);
    sal_Int32     i = std::max<sal_Int32>(path.lastIndexOf(BACKSLASH), path.lastIndexOf(SLASH));

    if (i < (path.getLength()-1))
    {
        path += BACKSLASH;
        rtl_uString_assign(ppustrPath, path.pData);
    }

    OSL_POSTCOND(path.endsWith(BACKSLASH), \
                 "osl_systemPathEnsureSeparator: Post condition failed");
}

/*******************************************************************
 osl_systemPathRemoveSeparator
 ******************************************************************/

void SAL_CALL osl_systemPathRemoveSeparator(/*inout*/ rtl_uString** ppustrPath)
{
    rtl::OUString path(*ppustrPath);

    if (!osl::systemPathIsLogicalDrivePattern(path))
    {
        sal_Int32 i = std::max<sal_Int32>(path.lastIndexOf(BACKSLASH), path.lastIndexOf(SLASH));

        if (i > -1 && (i == (path.getLength() - 1)))
        {
            path = rtl::OUString(path.getStr(), path.getLength() - 1);
            rtl_uString_assign(ppustrPath, path.pData);
        }
    }
}

/*******************************************************************
 osl_is_logical_drive_pattern
 ******************************************************************/

// is [A-Za-z]:[/|\]\0
const sal_Char* LDP                    = ":";
const sal_Char* LDP_WITH_BACKSLASH     = ":\\";
const sal_Char* LDP_WITH_SLASH         = ":/";

// degenerated case returned by the Windows FileOpen dialog
// when someone enters for instance "x:filename", the Win32
// API accepts this case
const sal_Char* LDP_WITH_DOT_BACKSLASH = ":.\\";

sal_Int32 osl_systemPathIsLogicalDrivePattern(/*in*/ const rtl_uString* pustrPath)
{
    const sal_Unicode* p = rtl_uString_getStr(const_cast<rtl_uString*>(pustrPath));
    if (iswalpha(*p++))
    {
        return ((0 == rtl_ustr_ascii_compare(p, LDP)) ||
                (0 == rtl_ustr_ascii_compare(p, LDP_WITH_BACKSLASH)) ||
                (0 == rtl_ustr_ascii_compare(p, LDP_WITH_SLASH)) ||
                (0 == rtl_ustr_ascii_compare(p, LDP_WITH_DOT_BACKSLASH)));
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
