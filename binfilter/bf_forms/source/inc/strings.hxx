/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _FRM_STRINGS_HXX_
#define _FRM_STRINGS_HXX_

namespace binfilter {
//.........................................................................
namespace frm
{
//.........................................................................

struct ConstAsciiString
{
    const sal_Char*	ascii;
    sal_Int32		length;

    inline	operator const ::rtl::OUString () const;
    inline	operator const sal_Char* () const { return ascii; }

    inline ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength);
    inline ~ConstAsciiString();

private:
    mutable ::rtl::OUString*	ustring;
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
inline ConstAsciiString::operator const ::rtl::OUString () const
{
    if (!ustring)
        ustring = new ::rtl::OUString(ascii, length, RTL_TEXTENCODING_ASCII_US);
    return *ustring;
}

#define DECLARE_CONSTASCII_USTRING(name)	\
    extern ::binfilter::frm::ConstAsciiString name

#define IMPLEMENT_CONSTASCII_USTRING(name, asciivalue)	\
    ::binfilter::frm::ConstAsciiString name(asciivalue, sizeof(asciivalue) - 1)

}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_STRINGS_HXX_
