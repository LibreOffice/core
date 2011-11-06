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

#ifndef _DBASHARED_CONSTASCIISTRING_HXX_
#define _DBASHARED_CONSTASCIISTRING_HXX_

#ifndef CONSTASCII_INCLUDED_INDIRECT
#error "don't include this file directly! use stringconstants.hrc instead!"
#endif

// no namespaces. This file is included from several other files _within_ a namespace.

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
// string constants
struct ConstAsciiString
{
    const sal_Char* ascii;
    sal_Int32       length;

    inline  operator const ::rtl::OUString& () const;
    inline  operator const sal_Char* () const { return ascii; }

    inline ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength);
    inline ~ConstAsciiString();

private:
    mutable ::rtl::OUString*    ustring;
};

//------------------------------------------------------------
inline ConstAsciiString::ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength)
    :ascii(_pAsciiZeroTerminated)
    ,length(_nLength)
    ,ustring(NULL)
{
}

//------------------------------------------------------------
inline ConstAsciiString::~ConstAsciiString()
{
    delete ustring;
    ustring = NULL;
}

//------------------------------------------------------------
inline ConstAsciiString::operator const ::rtl::OUString& () const
{
    if (!ustring)
        ustring = new ::rtl::OUString(ascii, length, RTL_TEXTENCODING_ASCII_US);
    return *ustring;
}

//============================================================

#define DECLARE_CONSTASCII_USTRING( name ) \
    extern const ConstAsciiString name

#define IMPLEMENT_CONSTASCII_USTRING( name, string ) \
    const ConstAsciiString name(string, sizeof(string)-1)


#endif // _DBASHARED_CONSTASCIISTRING_HXX_
