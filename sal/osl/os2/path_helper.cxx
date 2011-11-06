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


