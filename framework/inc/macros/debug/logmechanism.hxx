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



#ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
#define __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_

//*****************************************************************************************************************
//  generic macros for logging
//*****************************************************************************************************************

#ifdef  ENABLE_LOGMECHANISM

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
    #endif

    #include <stdio.h>

    /*_____________________________________________________________________________________________________________
        WRITE_LOGFILE( SFILENAME, STEXT )

        Log any information in file. We append any information at file and don't clear it anymore.
        ( Use new scope in macro to declare pFile more then on time in same "parentscope"!
            Don't control pFile before access! What will you doing if its not valid? Log an error ...
              An error and an error is an error ... )

        Attention:  You must use "%s" and STEXT as parameter ... because otherwise encoded strings (they include e.g. %...)
                    are handled wrong.
    _____________________________________________________________________________________________________________*/

    inline void writeToLogFile( const char* SFILENAME, const char* STEXT )
                {                                                                                               \
                    ::rtl::OString  _swriteLogfileFileName  ( SFILENAME );                                      \
                    ::rtl::OString  _swriteLogfileText      ( STEXT     );                                      \
                    FILE* pFile = fopen( _swriteLogfileFileName.getStr(), "a" );                                \
                    fprintf( pFile, "%s", _swriteLogfileText.getStr() );                                        \
                    fclose ( pFile                                    );                                        \
                }

    inline void writeToLogFile( const char* pFILENAME, const rtl::OString& rTEXT ) { writeToLogFile( pFILENAME, rTEXT.getStr()); }

    #define WRITE_LOGFILE( SFILENAME, STEXT ) { writeToLogFile( (SFILENAME), (STEXT) ); }

    /*_____________________________________________________________________________________________________________
        LOGTYPE

        For other debug macros we need information about the output mode. If user forget to set this information we
        do it for him. Valid values are:    LOGTYPE_FILECONTINUE
                                            LOGTYPE_FILEEXIT
                                            LOGTYPE_MESSAGEBOX
        The normal case is LOGTYPE_MESSAGEBOX to show assertions in normal manner!
    _____________________________________________________________________________________________________________*/

    #define LOGTYPE_MESSAGEBOX      1
    #define LOGTYPE_FILECONTINUE    2
    #define LOGTYPE_FILEEXIT        3

    #ifndef LOGTYPE
        #define LOGTYPE                                                                                         \
                    LOGTYPE_MESSAGEBOX
    #endif

#else   // #ifdef ENABLE_LOGMECHANISM

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro empty!
    _____________________________________________________________________________________________________________*/

    #define WRITE_LOGFILE( SFILENAME, STEXT )
    #undef  LOGTYPE

#endif  // #ifdef ENABLE_LOGMECHANISM

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
