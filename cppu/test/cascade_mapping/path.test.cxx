/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: path.test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:47:10 $
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

