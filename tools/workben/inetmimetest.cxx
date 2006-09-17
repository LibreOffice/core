/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetmimetest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:09:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "inetmime.hxx"

#include "rtl/textenc.h"
#include "rtl/ustring.hxx"

#include <cstdlib>
#include <iostream>

namespace {

bool testDecode(char const * input, char const * expected) {
    rtl::OUString result = INetMIME::decodeHeaderFieldBody(
        INetMIME::HEADER_FIELD_TEXT, input);
    bool success = result.equalsAscii(expected);
    if (!success) {
        std::cout
            << "FAILED: decodeHeaderFieldBody(\"" << input << "\"): \""
            << rtl::OUStringToOString(
                result, RTL_TEXTENCODING_ASCII_US).getStr()
            << "\" != \"" << expected << "\"\n";
    }
    return success;
}

}

int
#if defined WNT
__cdecl
#endif
main() {
    bool success = true;
    success &= testDecode("=?iso-8859-1?B?QQ==?=", "A");
    success &= testDecode("=?iso-8859-1?B?QUI=?=", "AB");
    success &= testDecode("=?iso-8859-1?B?QUJD?=", "ABC");
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
