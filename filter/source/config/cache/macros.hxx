/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macros.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:24:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FILTER_CONFIG_MACROS_HXX_
#define _FILTER_CONFIG_MACROS_HXX_

//_______________________________________________

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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
