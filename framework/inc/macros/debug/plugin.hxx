/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plugin.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:26:48 $
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
