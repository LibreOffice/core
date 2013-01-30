/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef CHART_MACROS_HXX
#define CHART_MACROS_HXX

#include <typeinfo>

/// creates a unicode-string from an ASCII string
#define constAsciiStr (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ))

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
