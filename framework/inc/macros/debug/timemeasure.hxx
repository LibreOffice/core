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

#ifndef __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_
#define __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_

//*************************************************************************************************************
//  special macros for time measures
//  1) LOGFILE_TIMEMEASURE                  used it to define log file for this operations (default will be set automaticly)
//  2) START_TIMEMEASURE                    start new measure by using given variable names
//  3) START_TIMEMEASURE                    stop current measure by using given variable names and return time
//  4) LOG_TIMEMEASURE                      write measured time to logfile
//*************************************************************************************************************

#ifdef  ENABLE_TIMEMEASURE

    //_________________________________________________________________________________________________________________
    //  includes
    //_________________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    #ifndef _OSL_TIME_H_
    #include <osl/time.h>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_TIMEMEASURE

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_TIMEMEASURE
        #define LOGFILE_TIMEMEASURE "timemeasure.log"
    #endif

    /*_____________________________________________________________________________________________________________
        class TimeMeasure

        We need this inline class as global timer to make it possible measure times over different objects!
        zB. Use it as baseclass to start timer at ctor (must be first called baseclass!!!) and stop it by calling stop method.
    _____________________________________________________________________________________________________________*/

    class DBGTimeMeasureBase
    {
        public:
            inline DBGTimeMeasureBase()
            {
                m_nEnd   = 0                   ;
                m_nStart = osl_getGlobalTimer();
            }

            inline sal_Int32 stopAndGet()
            {
                m_nEnd = osl_getGlobalTimer();
                return( m_nEnd-m_nStart );
            }

        private:
            sal_Int32 m_nStart ;
            sal_Int32 m_nEnd   ;
    };

    /*_____________________________________________________________________________________________________________
        START_TIMEMEASURE( NSTART, NEND )
        STOP_TIMEMEASURE( NSTART, NEND, NTIME )

        If you doesn't need a time measure above different classes ... you can try this macros!
        They initialize your given members with start end end time ... You can calculate differenz by himself.
    _____________________________________________________________________________________________________________*/

    #define START_TIMEMEASURE( NSTART, NEND )                                                                   \
                sal_Int32   NSTART  = 0;                                                                        \
                sal_Int32   NEND    = 0;                                                                        \
                NSTART = osl_getGlobalTimer();

    #define STOP_TIMEMEASURE( NSTART, NEND, NTIME )                                                             \
                          NEND  = osl_getGlobalTimer();                                                         \
                sal_Int32 NTIME = NEND-NSTART;

    /*_____________________________________________________________________________________________________________
        LOG_TIMEMEASURE( SOPERATION, NSTART )

        Write measured time to logfile.
    _____________________________________________________________________________________________________________*/

    #define LOG_TIMEMEASURE( SOPERATION, NTIME )                                                                \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( SOPERATION         );                                                      \
                    _sBuffer.append( "\t=\t"            );                                                      \
                    _sBuffer.append( (sal_Int32)(NTIME) );                                                      \
                    _sBuffer.append( " ms\n"            );                                                      \
                    WRITE_LOGFILE( LOGFILE_TIMEMEASURE, _sBuffer.makeStringAndClear().getStr() )                \
                }

#else   // #ifdef ENABLE_TIMEMEASURE

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macros empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_TIMEMEASURE
    #define START_TIMEMEASURE( NSTART, NEND )
    #define STOP_TIMEMEASURE( NSTART, NEND, NTIME )
    #define LOG_TIMEMEASURE( SOPERATION, NTIME )

#endif  // #ifdef ENABLE_TIMEMEASURE

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
