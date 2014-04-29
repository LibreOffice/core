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



#ifndef __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_
#define __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_

//*************************************************************************************************************
//  special macros for time measures
//  1) LOGFILE_FILTERDBG                  used it to define log file for this operations (default will be set automatically)
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
