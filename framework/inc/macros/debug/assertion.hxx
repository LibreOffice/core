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

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#define __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_

#if defined( ENABLE_ASSERTIONS )

    #include <osl/diagnose.h>
    #include <rtl/strbuf.hxx>

#endif

//*****************************************************************************************************************
//  special macros for assertion handling
//      1)  LOGTYPE                                                         use it to define the output of all assertions, errors, exception infos
//      2)  LOGFILE_ASSERTIONS                                              use it to define the file name to log assertions if LOGTYPE=LOGTYPE_FILE...
//      active for "non product":
//      4)  LOG_ASSERT( BCONDITION, STEXT )                                 assert some critical errors which depend from given condition
//      4a) LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                       same like 4) + additional location of error
//      5)  LOG_ERROR( SMETHOD, STEXT )                                     show errors without any condition
//      active for debug only!
//*****************************************************************************************************************

//_________________________________________________________________________________________________________________
#if defined( ENABLE_ASSERTIONS )

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
                        OStringBuffer _sAssertBuffer( 256 );                                             \
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
                        OStringBuffer _sAssertBuffer( 256 );                                             \
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
                        OStringBuffer _sAssertBuffer( 256 );                                             \
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

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
