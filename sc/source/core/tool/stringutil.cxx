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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------

#include "stringutil.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/math.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

bool ScStringUtil::parseSimpleNumber(
    const OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal)
{
    if (gsep == 0x00A0)
        // unicode space to ascii space
        gsep = 0x0020;

    OUStringBuffer aBuf;
    sal_Int32 n = rStr.getLength();
    const sal_Unicode* p = rStr.getStr();
    sal_Int32 nPosDSep = -1, nPosGSep = -1;
    sal_uInt32 nDigitCount = 0;

    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode c = p[i];
        if (c == 0x00A0)
            // unicode space to ascii space
            c = 0x0020;

        if (sal_Unicode('0') <= c && c <= sal_Unicode('9'))
        {
            // this is a digit.
            aBuf.append(c);
            ++nDigitCount;
        }
        else if (c == dsep)
        {
            // this is a decimal separator.

            if (nPosDSep >= 0)
                // a second decimal separator -> not a valid number.
                return false;

            if (nPosGSep >= 0 && i - nPosGSep != 4)
                // the number has a group separator and the decimal sep is not
                // positioned correctly.
                return false;

            nPosDSep = i;
            nPosGSep = -1;
            aBuf.append(c);
            nDigitCount = 0;
        }
        else if (c == gsep)
        {
            // this is a group (thousand) separator.

            if (i == 0)
                // not allowed as the first character.
                return false;

            if (nPosDSep >= 0)
                // not allowed after the decimal separator.
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                // must be exactly 3 digits since the last group separator.
                return false;

            nPosGSep = i;
            nDigitCount = 0;
        }
        else if (c == sal_Unicode('-') || c == sal_Unicode('+'))
        {
            // A sign must be the first character if it's given.
            if (i == 0)
                aBuf.append(c);
            else
                return false;
        }
        else
            return false;
    }

    // finished parsing the number.

    if (nPosGSep >= 0 && nDigitCount != 3)
        // must be exactly 3 digits since the last group separator.
        return false;

    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    rVal = ::rtl::math::stringToDouble(aBuf.makeStringAndClear(), dsep, gsep, &eStatus, &nParseEnd);
    if (eStatus != rtl_math_ConversionStatus_Ok)
        return false;

    return true;
}
