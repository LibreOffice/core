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

#ifndef _FILTER_CONFIG_MACROS_HXX_
#define _FILTER_CONFIG_MACROS_HXX_

//_______________________________________________

#include <rtl/ustring.hxx>

#ifdef _FILTER_CONFIG_FROM_ASCII_
    #error "who already defined such macro :-("
#endif

#ifdef _FILTER_CONFIG_TO_ASCII_
    #error "who already defined such macro :-("
#endif

#define _FILTER_CONFIG_FROM_ASCII_(ASCII_STRING)            \
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ASCII_STRING))

#define _FILTER_CONFIG_TO_ASCII_(UNICODE_STRING)            \
            ::rtl::OUStringToOString(UNICODE_STRING, RTL_TEXTENCODING_UTF8).getStr()

#define _FILTER_CONFIG_LOG_(TEXT)
#define _FILTER_CONFIG_LOG_1_(FORMAT, ARG1)
#define _FILTER_CONFIG_LOG_2_(FORMAT, ARG1, ARG2)

#endif // _FILTER_CONFIG_MACROS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
