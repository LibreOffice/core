/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART_MACROS_HXX
#define CHART_MACROS_HXX

#include <typeinfo>

/// creates a unicode-string from an ASCII string
#define C2U(constAsciiStr) (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ))

/** shows an error-box for an exception ex
    else-branch necessary to avoid warning
*/
#if OSL_DEBUG_LEVEL > 0
#define ASSERT_EXCEPTION(ex)                   \
  OSL_FAIL( ::rtl::OUStringToOString( \
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +\
    ::rtl::OUString::createFromAscii( typeid( ex ).name()) +\
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +\
    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr())
#else
//avoid compilation warnings
#define ASSERT_EXCEPTION(ex) (void)(ex)
#endif

#define U2C(ouString) (::rtl::OUStringToOString(ouString,RTL_TEXTENCODING_ASCII_US).getStr())

// CHART_MACROS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
