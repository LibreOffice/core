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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <tools/inetmime.hxx>

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
