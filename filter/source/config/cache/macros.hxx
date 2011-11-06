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
