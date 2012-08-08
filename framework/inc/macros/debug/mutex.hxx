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

#ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
#define __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_

//*****************************************************************************************************************
//  special macros for mutex handling
//*****************************************************************************************************************

#ifdef  ENABLE_MUTEXDEBUG

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_MUTEX

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_MUTEX
        #define LOGFILE_MUTEX   "mutex.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )

        Write informations about current set lock type for whole framework project to special file.
    _____________________________________________________________________________________________________________*/

    #define LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )                                                               \
                /* new scope to prevent us against multiple definitions of variables ... */                     \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( "Set framework lock type to fallback: \"" );                               \
                    switch( _EFALLBACK )                                                                        \
                    {                                                                                           \
                        case E_NOTHING      :   _sBuffer.append( "E_NOTHING"    );                              \
                                                break;                                                          \
                        case E_OWNMUTEX     :   _sBuffer.append( "E_OWNMUTEX"   );                              \
                                                break;                                                          \
                        case E_SOLARMUTEX   :   _sBuffer.append( "E_SOLARMUTEX" );                              \
                                                break;                                                          \
                        case E_FAIRRWLOCK   :   _sBuffer.append( "E_FAIRRWLOCK" );                              \
                                                break;                                                          \
                    }                                                                                           \
                    _sBuffer.append( "\"\n" );                                                                  \
                    if( _EFALLBACK != _ECURRENT )                                                               \
                    {                                                                                           \
                        _sBuffer.append( "... environment overwrite framework lock type with: \"" );            \
                        switch( _ECURRENT )                                                                     \
                        {                                                                                       \
                            case E_NOTHING      :   _sBuffer.append( "E_NOTHING"    );                          \
                                                    break;                                                      \
                            case E_OWNMUTEX     :   _sBuffer.append( "E_OWNMUTEX"   );                          \
                                                    break;                                                      \
                            case E_SOLARMUTEX   :   _sBuffer.append( "E_SOLARMUTEX" );                          \
                                                    break;                                                      \
                            case E_FAIRRWLOCK   :   _sBuffer.append( "E_FAIRRWLOCK" );                          \
                                                    break;                                                      \
                        }                                                                                       \
                        _sBuffer.append( "\"\n" );                                                              \
                    }                                                                                           \
                    else                                                                                        \
                    {                                                                                           \
                        _sBuffer.append( "... use fallback, because user don't set another value!\n" );         \
                    }                                                                                           \
                    WRITE_LOGFILE( LOGFILE_MUTEX, _sBuffer.makeStringAndClear() )                               \
                }

#else   // #ifdef ENABLE_MUTEXDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro with normal functionality!
        We need the guard but not the log mechanism.
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_MUTEX
    #define LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )

#endif  // #ifdef ENABLE_MUTEXDEBUG

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
