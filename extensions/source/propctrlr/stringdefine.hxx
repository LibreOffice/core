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



#ifndef _EXTENSIONS_FORMSCTRLR_STRINGDEFINE_HXX_
#define _EXTENSIONS_FORMSCTRLR_STRINGDEFINE_HXX_

#include <rtl/ustring.hxx>
#include <tools/string.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    //============================================================
    //= a helper for static ascii pseudo-unicode strings
    //============================================================
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

#define CONST_ASCII_LENGTH(c)   \
    (const sal_Char*)c, c.length()

    //============================================================
    //= concrete strings
    //============================================================
    #ifndef PCR_IMPLEMENT_STRINGS
    #define PCR_CONSTASCII_STRING(ident, string) extern const ConstAsciiString ident
    #else
    #define PCR_CONSTASCII_STRING(ident, string) extern const ConstAsciiString ident(string, sizeof(string)-1)
    #endif

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_FORMSCTRLR_STRINGDEFINE_HXX_

