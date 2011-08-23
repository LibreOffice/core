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

/*
//#define _FILTER_CONFIG_FROM_ASCII_(ASCII_STRING)            \
//            ::rtl::OUString::createFromAscii(ASCII_STRING)
*/

#define _FILTER_CONFIG_FROM_ASCII_(ASCII_STRING)            \
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ASCII_STRING))

#define _FILTER_CONFIG_TO_ASCII_(UNICODE_STRING)            \
            ::rtl::OUStringToOString(UNICODE_STRING, RTL_TEXTENCODING_UTF8).getStr()

#define _FILTER_CONFIG_LOG_(TEXT)
#define _FILTER_CONFIG_LOG_1_(FORMAT, ARG1)
#define _FILTER_CONFIG_LOG_2_(FORMAT, ARG1, ARG2)
#define _FILTER_CONFIG_LOG_3_(FORMAT, ARG1, ARG2, ARG3)

/*
#include <rtl/ustrbuf.hxx>

#include <stdio.h>

//_______________________________________________

    #ifdef _FILTER_CONFIG_LOG_
        #error "who already defined such macro :-("
    #endif

    #ifdef _FILTER_CONFIG_LOG_1_
        #error "who already defined such macro :-("
    #endif

    #ifdef _FILTER_CONFIG_LOG_2_
        #error "who already defined such macro :-("
    #endif

    #ifdef _FILTER_CONFIG_LOG_3_
        #error "who already defined such macro :-("
    #endif

    //-------------------------------------------
    // @short  append given text to the log file
    //
    //    @param  TEXT [const char*]
    //            contains the text, which should be logged
    //
    #define _FILTER_CONFIG_LOG_(TEXT)                           \
                {                                               \
                    FILE* pFile = fopen("filtercache.log", "a");\
                    fprintf(pFile, "%s", TEXT);                 \
                    fclose(pFile);                              \
                }

    //-------------------------------------------
    // @short  append given text to the log file
    //            and supports using one optional argument.
    //
    //    @descr  The syntax of FORMAT and ARG1 follows the
    //            mechanism of e.g. sprintf();
    //
    //    @param  FORMAT [const char*]
    //            specify the format of the log message
    //
    //    @param  ARGn
    //            points to any argument(s), which will be used
    //            inside the FORMAT string to replace place holder(s).
    //
    #define _FILTER_CONFIG_LOG_1_(FORMAT, ARG1)                 \
                {                                               \
                    FILE* pFile = fopen("filtercache.log", "a");\
                    fprintf(pFile, FORMAT, ARG1);               \
                    fclose(pFile);                              \
                }

    #define _FILTER_CONFIG_LOG_2_(FORMAT, ARG1, ARG2)           \
                {                                               \
                    FILE* pFile = fopen("filtercache.log", "a");\
                    fprintf(pFile, FORMAT, ARG1, ARG2);         \
                    fclose(pFile);                              \
                }

    #define _FILTER_CONFIG_LOG_3_(FORMAT, ARG1, ARG2, ARG3)     \
                {                                               \
                    FILE* pFile = fopen("filtercache.log", "a");\
                    fprintf(pFile, FORMAT, ARG1, ARG2, ARG3);   \
                    fclose(pFile);                              \
                }

#endif // OSL_DEBUG_LEVEL < 2
*/
#endif // _FILTER_CONFIG_MACROS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
