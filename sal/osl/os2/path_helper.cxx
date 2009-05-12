/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: path_helper.cxx,v $
 * $Revision: 1.4 $
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

/*******************************************************************
 Includes
 ******************************************************************/

#include "path_helper.hxx"
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <algorithm>
#include <wchar.h>
#include <wctype.h>

/*******************************************************************
 Constants
 ******************************************************************/

const rtl::OUString BACKSLASH = rtl::OUString::createFromAscii("\\");
const rtl::OUString SLASH     = rtl::OUString::createFromAscii("/");

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

    OSL_POSTCOND(path.lastIndexOf(BACKSLASH) == (path.getLength() - 1), \
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
const sal_Unicode* LDP                    = L":";
const sal_Unicode* LDP_WITH_BACKSLASH     = L":\\";
const sal_Unicode* LDP_WITH_SLASH         = L":/";

// degenerated case returned by the Windows FileOpen dialog
// when someone enters for instance "x:filename", the Win32
// API accepts this case
const sal_Unicode* LDP_WITH_DOT_BACKSLASH = L":.\\";

sal_Int32 osl_systemPathIsLogicalDrivePattern(/*in*/ const rtl_uString* pustrPath)
{
    const sal_Unicode* p = rtl_uString_getStr(const_cast<rtl_uString*>(pustrPath));
    if (iswalpha(*p++))
    {
        return ((0 == wcscmp(p, LDP)) ||
                (0 == wcscmp(p, LDP_WITH_BACKSLASH)) ||
                (0 == wcscmp(p, LDP_WITH_SLASH)) ||
                (0 == wcscmp(p, LDP_WITH_DOT_BACKSLASH)));
    }
    return 0;
}


