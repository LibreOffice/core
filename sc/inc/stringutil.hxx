/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: table.hxx,v $
 * $Revision: 1.35 $
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

#ifndef SC_STRINGUTIL_HXX
#define SC_STRINGUTIL_HXX

#include "rtl/ustring.hxx"

class ScStringUtil
{
public:
    /**
     * Check if a given string is a simple decimal number (e.g. 12.345). We
     * don't do any elaborate parsing here; we only check for the simplest
     * case of decimal number format.
     *
     * @param rStr string to parse
     * @param dsep decimal separator
     * @param gsep group separator (aka thousands separator)
     * @param rVal value of successfully parsed number
     *
     * @return true if the string is a valid number, false otherwise.
     */
    static bool parseSimpleNumber(
        const ::rtl::OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal);
};


#endif
