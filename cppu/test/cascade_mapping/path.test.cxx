/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: path.test.cxx,v $
 * $Revision: 1.3 $
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

#include "rtl/ustring.hxx"

#include "../mapping_tester/Mapping.tester.hxx"


D_CALLEE
{
    rtl::OUString const method_name(pMethod_name);

    if (g_check)
    {
        g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tmethod: "));
        g_result += method_name;
        g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - mapping purpose path test --> "));

        if (g_ref.compareTo(g_custom) == 0)
        {
            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));
        }
        else
        {
            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));
            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\t\texpected: "));
            g_result += g_ref;
            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\t\t     got: "));
            g_result += g_custom;
            g_result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }
    }
}

