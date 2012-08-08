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

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#define __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_

#if defined( ENABLE_ASSERTIONS ) || defined( ENABLE_WARNINGS )

    #ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
    #endif

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

#endif

//*****************************************************************************************************************
//  special macros for assertion handling
//      1)  LOGTYPE                                                         use it to define the output of all assertions, errors, exception infos
//      2)  LOGFILE_ASSERTIONS                                              use it to define the file name to log assertions if LOGTYPE=LOGTYPE_FILE...
//      3)  LOGFILE_WARNINGS                                                use it to define the file name to log warnings if LOGTYPE=LOGTYPE_FILE...
//      active for "non product":
//      4)  LOG_ASSERT( BCONDITION, STEXT )                                 assert some critical errors wich depend from given condition
//      4a) LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                       same like 4) + additional location of error
//      5)  LOG_ERROR( SMETHOD, STEXT )                                     show errors without any condition
//      active for debug only!
//      6)  LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )        show/log an exception for easier debug
//      7)  LOG_WARNING( SMETHOD, STEXT )                                   should be used to detect leaks in algorithm, mechanism or operation handling
//*****************************************************************************************************************

//_________________________________________________________________________________________________________________
#if defined( ENABLE_ASSERTIONS ) || defined( ENABLE_WARNINGS )

    /*_____________________________________________________________________________________________________________
        LOGFILE_ASSERTIONS

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_ASSERTIONS
        #define LOGFILE_ASSERTIONS  "_framework_assertions.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT ( BCONDITION, STEXT )
        LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )

        Forward assertion to logfile (if condition is sal_False - like a DBG_ASSERT!) and continue with program.
        Set LOGTYPE to LOGTYPE_FILECONTINUE to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_FILECONTINUE

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    if ( ( BCONDITION ) == sal_False )                                                          \
                    {                                                                                           \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, STEXT )                                              \
                    }

        #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                                                       \
                    if ( ( BCONDITION ) == sal_True )                                                           \
                    {                                                                                           \
                        ::rtl::OStringBuffer _sAssertBuffer( 256 );                                             \
                        _sAssertBuffer.append( "ASSERT:\n\t"    );                                              \
                        _sAssertBuffer.append( SMETHOD          );                                              \
                        _sAssertBuffer.append( "\n\t\""         );                                              \
                        _sAssertBuffer.append( STEXT            );                                              \
                        _sAssertBuffer.append( "\"\n"           );                                              \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, _sAssertBuffer.getStr() )                \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT ( BCONDITION, STEXT )
        LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )

        Forward assertion to file and exit the program.
        Set LOGTYPE to LOGTYPE_FILEEXIT to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_FILEEXIT

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    if ( ( BCONDITION ) == sal_False )                                                          \
                    {                                                                                           \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, STEXT )                                              \
                        exit(-1);                                                                               \
                    }

        #define LOG_ASSERT2( BCONDITION, SMETHODE, STEXT )                                                      \
                    if ( ( BCONDITION ) == sal_True )                                                           \
                    {                                                                                           \
                        ::rtl::OStringBuffer _sAssertBuffer( 256 );                                             \
                        _sAssertBuffer.append( "ASSERT:\n\t"    );                                              \
                        _sAssertBuffer.append( SMETHOD          );                                              \
                        _sAssertBuffer.append( "\n\t\""         );                                              \
                        _sAssertBuffer.append( STEXT            );                                              \
                        _sAssertBuffer.append( "\"\n"           );                                              \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, _sAssertBuffer.getStr() )                \
                        exit(-1);                                                                               \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT ( BCONDITION, STEXT )
        LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )

        Forward assertions to messagebox. (We use OSL_ENSURE to do this.)
        Set LOGTYPE to LOGTYPE_MESSAGEBOX to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_MESSAGEBOX

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    OSL_ENSURE( ( BCONDITION ), STEXT );

        #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                                                       \
                    {                                                                                           \
                        ::rtl::OStringBuffer _sAssertBuffer( 256 );                                             \
                        _sAssertBuffer.append( "ASSERT:\n\t"    );                                              \
                        _sAssertBuffer.append( SMETHOD          );                                              \
                        _sAssertBuffer.append( "\n\t\""         );                                              \
                        _sAssertBuffer.append( STEXT            );                                              \
                        _sAssertBuffer.append( "\"\n"           );                                              \
                        OSL_ENSURE( !( BCONDITION ), _sAssertBuffer.getStr() );                     \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ERROR( SMETHOD, STEXT )

        Show an error by using current set output mode by define LOGTYPE!
    _____________________________________________________________________________________________________________*/

    #define LOG_ERROR( SMETHOD, STEXT )                                                                         \
                LOG_ASSERT2( sal_True, SMETHOD, STEXT )

#else

    // If right testmode is'nt set - implements these macros empty!
    #undef  LOGFILE_ASSERTIONS
    #define LOG_ASSERT( BCONDITION, STEXT )
    #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )
    #define LOG_ERROR( SMETHOD, STEXT )

#endif  // ENABLE_ASSERTIONS

//_________________________________________________________________________________________________________________
#if defined( ENABLE_WARNINGS )

    /*_____________________________________________________________________________________________________________
        LOGFILE_WARNINGS

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_WARNINGS
        #define LOGFILE_WARNINGS  "_framework_warnings.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )

        Show some exception info by using current set output mode by define LOGTYPE!
        We use a seperated scope {} do protect us against multiple variable definitions.
    _____________________________________________________________________________________________________________*/

    #define LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )                                            \
                {                                                                                               \
                    ::rtl::OStringBuffer _sAssertBuffer2( 256 );                                                \
                    _sAssertBuffer2.append( SOWNMESSAGE             );                                          \
                    _sAssertBuffer2.append( "\n"                    );                                          \
                    _sAssertBuffer2.append( U2B(SEXCEPTIONMESSAGE)  );                                          \
                    LOG_ERROR( SMETHOD, _sAssertBuffer2.getStr() )                                  \
                }

    /*_____________________________________________________________________________________________________________
        LOG_WARNING( SMETHOD, STEXT )

        Use it to show/log warnings for programmer for follow reasons:
            - algorithm errors
            - undefined states
            - unknown errors from other modules ...
    _____________________________________________________________________________________________________________*/

    #define LOG_WARNING( SMETHOD, STEXT )                                                                       \
                LOG_ERROR( SMETHOD, STEXT )

#else

    // If right testmode is'nt set - implements these macros empty!
    #undef  LOGFILE_WARNINGS
    #define LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )
    #define LOG_WARNING( SMETHOD, STEXT )

#endif  // ENABLE_WARNINGS

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
