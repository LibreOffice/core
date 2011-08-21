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

#ifndef __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_
#define __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_

//*************************************************************************************************************
//  special macros for time measures
//  1) LOGFILE_FILTERDBG                  used it to define log file for this operations (default will be set automaticly)
//  4) LOG_FILTERDBG                      write debug info to log file
//*************************************************************************************************************

#ifdef  ENABLE_FILTERDBG

    //_________________________________________________________________________________________________________________
    //  includes
    //_________________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_FILTERDBG

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_FILTERDBG
        #define LOGFILE_FILTERDBG "filterdbg.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_FILTERDBG( SOPERATION, SMESSAGE )

        Write special debug info to the log file.
    _____________________________________________________________________________________________________________*/

    #define LOG_FILTERDBG( SOPERATION, SMESSAGE )                                                               \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( SOPERATION );                                                              \
                    _sBuffer.append( "\t"       );                                                              \
                    _sBuffer.append( SMESSAGE   );                                                              \
                    _sBuffer.append( "\n"       );                                                              \
                    WRITE_LOGFILE( LOGFILE_FILTERDBG, _sBuffer.makeStringAndClear().getStr() )                  \
                }

    /*_____________________________________________________________________________________________________________
        LOG_FILTERDBG_1_PARAM( SOPERATION, SPARAM, SMESSAGE )

        Write special debug info into the log file and mark SNAME as special parameter before SMESSAGE is printed.
    _____________________________________________________________________________________________________________*/

    #define LOG_FILTERDBG_1_PARAM( SOPERATION, SPARAM, SMESSAGE )                                               \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( SOPERATION );                                                              \
                    _sBuffer.append( "\t\""     );                                                              \
                    _sBuffer.append( SPARAM     );                                                              \
                    _sBuffer.append( "\" "      );                                                              \
                    _sBuffer.append( SMESSAGE   );                                                              \
                    _sBuffer.append( "\n"       );                                                              \
                    WRITE_LOGFILE( LOGFILE_FILTERDBG, _sBuffer.makeStringAndClear().getStr() )                  \
                }

    /*_____________________________________________________________________________________________________________
        LOG_COND_FILTERDBG( CONDITION, SOPERATION, SMESSAGE )

        Write special debug info to the log file, if given condition returns true.
    _____________________________________________________________________________________________________________*/

    #define LOG_COND_FILTERDBG( CONDITION, SOPERATION, SMESSAGE )                                               \
                if (CONDITION)                                                                                  \
                    LOG_FILTERDBG( SOPERATION, SMESSAGE )

    /*_____________________________________________________________________________________________________________
        LOG_COND_FILTERDBG_1_PARAM( CONDITION, SOPERATION, SPARAM, SMESSAGE )

        Write special debug info into the log file and mark SNAME as special parameter before SMESSAGE is printed.
        But it will be done only, if CONDITION returns true.
    _____________________________________________________________________________________________________________*/

    #define LOG_COND_FILTERDBG_1_PARAM( CONDITION, SOPERATION, SPARAM, SMESSAGE )                               \
                if (CONDITION)                                                                                  \
                    LOG_FILTERDBG_1_PARAM( SOPERATION, SPARAM, SMESSAGE )

#else   // #ifdef ENABLE_FILTERDBG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macros empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_FILTERDBG
    #define LOG_FILTERDBG( SOPERATION, SMESSAGE )
    #define LOG_COND_FILTERDBG( CONDITION, SOPERATION, SMESSAGE )
    #define LOG_FILTERDBG_1_PARAM( SOPERATION, SPARAM, SMESSAGE )
    #define LOG_COND_FILTERDBG_1_PARAM( CONDITION, SOPERATION, SPARAM, SMESSAGE )

#endif  // #ifdef ENABLE_FILTERDBG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
