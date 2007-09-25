/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: path_helper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 09:50:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/*******************************************************************
 Includes
 ******************************************************************/

#ifndef _PATH_HELPER_HXX_
#include "path_helper.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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


