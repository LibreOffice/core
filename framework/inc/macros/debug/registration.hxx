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



#ifndef __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_
#define __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_

//*****************************************************************************************************************
//  special macros for assertion handling
//*****************************************************************************************************************

#ifdef  ENABLE_REGISTRATIONDEBUG

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_ASSERTIONS

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_REGISTRATION
        #define LOGFILE_REGISTRATION                                                                            \
                    "registration.log"
    #endif

    #define LOG_REGISTRATION_GETFACTORY( SINFOTEXT )                                                            \
                {                                                                                               \
                    ::rtl::OStringBuffer sOut( 1024 );                                                          \
                    sOut.append( "component_getFactory():" );                                                   \
                    sOut.append( SINFOTEXT                 );                                                   \
                    WRITE_LOGFILE( LOGFILE_REGISTRATION, sOut.makeStringAndClear() )                            \
                }

#else   // #ifdef ENABLE_REGISTRATIONDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_REGISTRATION
    #define LOG_REGISTRATION_GETFACTORY( SINFOTEXT )

#endif  // #ifdef ENABLE_REGISTRATIONDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_
