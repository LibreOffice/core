/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filterdbg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:25:50 $
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
