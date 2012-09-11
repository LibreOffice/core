/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
************************************************************************/

#ifndef INCLUDED_RTL_OUSTRINGOSTREAMINSERTER_HXX
#define INCLUDED_RTL_OUSTRINGOSTREAMINSERTER_HXX

#include "sal/config.h"

#include <ostream>

#include "rtl/textenc.h"
#include "rtl/ustring.hxx"

// The unittest uses slightly different code to help check that the proper
// calls are made. The class is put into a different namespace to make
// sure the compiler generates a different (if generating also non-inline)
// copy of the function and does not merge them together. The class
// is "brought" into the proper rtl namespace by a typedef below.
#ifdef RTL_STRING_UNITTEST
#define rtl rtlunittest
#endif

namespace rtl {

#ifdef RTL_STRING_UNITTEST
#undef rtl
#endif

/**
    Support for rtl::OUString in std::ostream (and thus in
    CPPUNIT_ASSERT or SAL_INFO macros, for example).

    The rtl::OUString is converted to UTF-8.

    @since LibreOffice 3.5.
*/
template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, rtl::OUString const & string)
{
    return stream <<
        rtl::OUStringToOString(string, RTL_TEXTENCODING_UTF8).getStr();
        // best effort; potentially loses data due to conversion failures
        // (stray surrogate halves) and embedded null characters
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
