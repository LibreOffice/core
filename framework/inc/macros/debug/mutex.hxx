/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mutex.hxx,v $
 * $Revision: 1.8 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
#define __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_

//*****************************************************************************************************************
//  special macros for mutex handling
//*****************************************************************************************************************

#ifdef  ENABLE_MUTEXDEBUG

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

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

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
