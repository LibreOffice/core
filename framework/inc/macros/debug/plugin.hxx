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



#ifndef __FRAMEWORK_MACROS_DEBUG_PLUGIN_HXX_
#define __FRAMEWORK_MACROS_DEBUG_PLUGIN_HXX_

//*****************************************************************************************************************
//  special macros to debug asynchronous methods of plugin frame
//*****************************************************************************************************************

#ifdef  ENABLE_PLUGINDEBUG

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    #ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_PLUGIN

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_PLUGIN
        #define LOGFILE_PLUGIN  \
                    "plugin.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_URLSEND( SFRAMENAME, SSENDMODE, SINTERNALURL, SEXTERNALURL )

        Our plugin forward special url's to plugin dll, browser and webserver.
        We convert internal url's to an external notation.
        With this macro you can log some parameter of this operation.
    _____________________________________________________________________________________________________________*/

    #define LOG_URLSEND( SFRAMENAME, SSENDMODE, SINTERNALURL, SEXTERNALURL )                                \
                /* Use new scope to declare local private variables! */                                     \
                {                                                                                           \
                    ::rtl::OStringBuffer sBuffer(1024);                                                     \
                    sBuffer.append( "PlugInFrame [ \""  );                                                  \
                    sBuffer.append( SFRAMENAME          );                                                  \
                    sBuffer.append( "\" ] send "        );                                                  \
                    sBuffer.append( SSENDMODE           );                                                  \
                    sBuffer.append( "( internalURL=\""  );                                                  \
                    sBuffer.append( U2B( SINTERNALURL ) );                                                  \
                    sBuffer.append( "\", externalURL=\"");                                                  \
                    sBuffer.append( U2B( SEXTERNALURL ) );                                                  \
                    sBuffer.append( "\" ) to browser.\n");                                                  \
                    WRITE_LOGFILE( LOGFILE_PLUGIN, sBuffer.makeStringAndClear() )                           \
                }

    /*_____________________________________________________________________________________________________________
        LOG_URLRECEIVE( SFRAMENAME, SRECEIVEMODE, SEXTERNALURL, SINTERNALURL )

        A plugin frame can get a url request in two different modes.
        1) newURL()
        2) newStream()
        We convert external url's to an internal notation.
        With this macro you can log some parameter of this operations.
    _____________________________________________________________________________________________________________*/

    #define LOG_URLRECEIVE( SFRAMENAME, SRECEIVEMODE, SEXTERNALURL, SINTERNALURL )                          \
                /* Use new scope to declare local private variables! */                                     \
                {                                                                                           \
                    ::rtl::OStringBuffer sBuffer(1024);                                                     \
                    sBuffer.append( "PlugInFrame [ \""      );                                              \
                    sBuffer.append( U2B( SFRAMENAME )       );                                              \
                    sBuffer.append( "\" ] receive "         );                                              \
                    sBuffer.append( SRECEIVEMODE            );                                              \
                    sBuffer.append( "( externalURL=\""      );                                              \
                    sBuffer.append( U2B( SEXTERNALURL )     );                                              \
                    sBuffer.append( "\", internalURL=\""    );                                              \
                    sBuffer.append( U2B( SINTERNALURL )     );                                              \
                    sBuffer.append( "\" ) from browser.\n"  );                                              \
                    WRITE_LOGFILE( LOGFILE_PLUGIN, sBuffer.makeStringAndClear() )                           \
                }

    /*_____________________________________________________________________________________________________________
        LOG_PARAMETER_NEWURL( SFRAMENAME, SMIMETYPE, SURL, AANY )

        Log information about parameter of a newURL() at a plugin frame.
    _____________________________________________________________________________________________________________*/

    #define LOG_PARAMETER_NEWURL( SFRAMENAME, SMIMETYPE, SURL, sFILTER, AANY )                              \
                /* Use new scope to declare local private variables! */                                     \
                {                                                                                           \
                    ::rtl::OStringBuffer sBuffer(1024);                                                     \
                    sBuffer.append( "PlugInFrame [ \""              );                                      \
                    sBuffer.append( U2B( SFRAMENAME )               );                                      \
                    sBuffer.append( "\" ] called with newURL( \""   );                                      \
                    sBuffer.append( U2B( SMIMETYPE )                );                                      \
                    sBuffer.append( "\", \""                        );                                      \
                    sBuffer.append( U2B( SURL )                     );                                      \
                    sBuffer.append( "\", \""                        );                                      \
                    sBuffer.append( U2B( SFILTER )                  );                                      \
                    sBuffer.append( "\", "                          );                                      \
                    if( AANY.hasValue() == sal_True )                                                       \
                    {                                                                                       \
                        sBuffer.append( "filled Any )"  );                                                  \
                    }                                                                                       \
                    else                                                                                    \
                    {                                                                                       \
                        sBuffer.append( "empty Any )"   );                                                  \
                    }                                                                                       \
                    sBuffer.append( "\n"    );                                                              \
                    WRITE_LOGFILE( LOGFILE_PLUGIN, sBuffer.makeStringAndClear() )                           \
                }

    /*_____________________________________________________________________________________________________________
        LOG_PARAMETER_NEWSTREAM( SFRAMENAME, SMIMETYPE, SURL, ASTREAM, AANY )

        Log information about parameter of a newStream() at a plugin frame.
    _____________________________________________________________________________________________________________*/

    #define LOG_PARAMETER_NEWSTREAM( SFRAMENAME, SMIMETYPE, SURL, SFILTER, XSTREAM, AANY )                  \
                /* Use new scope to declare local private variables! */                                     \
                {                                                                                           \
                    ::rtl::OStringBuffer sBuffer(1024);                                                     \
                    sBuffer.append( "PlugInFrame [ \""              );                                      \
                    sBuffer.append( U2B( SFRAMENAME )               );                                      \
                    sBuffer.append( "\" ] called with newStream( \"");                                      \
                    sBuffer.append( U2B( SMIMETYPE )                );                                      \
                    sBuffer.append( "\", \""                        );                                      \
                    sBuffer.append( U2B( SURL )                     );                                      \
                    sBuffer.append( "\", \""                        );                                      \
                    sBuffer.append( U2B( SFILTER )                  );                                      \
                    sBuffer.append( "\", "                          );                                      \
                    if( XSTREAM.is() == sal_True )                                                          \
                    {                                                                                       \
                        sal_Int32 nBytes = XSTREAM->available();                                            \
                        OString sInfo("Stream with ");                                                      \
                        sInfo += OString::valueOf( (sal_Int32)nBytes );                                     \
                        sInfo += " Bytes, ";                                                                \
                        sBuffer.append( sInfo );                                                            \
                    }                                                                                       \
                    else                                                                                    \
                    {                                                                                       \
                        sBuffer.append( "empty Stream, "    );                                              \
                    }                                                                                       \
                    if( AANY.hasValue() == sal_True )                                                       \
                    {                                                                                       \
                        sBuffer.append( "filled Any )"  );                                                  \
                    }                                                                                       \
                    else                                                                                    \
                    {                                                                                       \
                        sBuffer.append( "empty Any )"   );                                                  \
                    }                                                                                       \
                    sBuffer.append( "\n"    );                                                              \
                    WRITE_LOGFILE( LOGFILE_PLUGIN, sBuffer.makeStringAndClear() )                           \
                }

#else   // #ifdef ENABLE_PLUGINDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_PLUGIN
    #define LOG_URLSEND( SFRAMENAME, SSENDMODE, SINTERNALURL, SEXTERNALURL )
    #define LOG_URLRECEIVE( SFRAMENAME, SRECEIVEMODE, SEXTERNALURL, SINTERNALURL )
    #define LOG_PARAMETER_NEWURL( SFRAMENAME, SMIMETYPE, SURL, SFILTER, AANY )
    #define LOG_PARAMETER_NEWSTREAM( SFRAMENAME, SMIMETYPE, SURL, SFILTER, XSTREAM, AANY )

#endif  // #ifdef ENABLE_PLUGINDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_PLUGIN_HXX_
