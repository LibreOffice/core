/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: event.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_
#define __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_

//*************************************************************************************************************
//  special macros for event handling
//*************************************************************************************************************

#ifdef  ENABLE_EVENTDEBUG

    //_________________________________________________________________________________________________________________
    //  includes
    //_________________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_EVENTDEBUG

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_EVENTDEBUG
        #define LOGFILE_EVENTDEBUG                                                                                                      \
                    "events.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_FRAMEACTIONEVENT( SFRAMETYPE, SFRAMENAME, AFRAMEACTION )

        Use this macro to print debug informations about sending of events to listener for controling right order.
        ( Use new scope in macro to declare sBuffer more then on time in same "parentscope"! )
    _____________________________________________________________________________________________________________*/

    #define LOG_FRAMEACTIONEVENT( SFRAMETYPE, SFRAMENAME, AFRAMEACTION )                                                                \
                {                                                                                                                       \
                    ::rtl::OStringBuffer sBuffer(1024);                                                                                 \
                    sBuffer.append( "[ "                );                                                                              \
                    sBuffer.append( SFRAMETYPE          );                                                                              \
                    sBuffer.append( " ] \""             );                                                                              \
                    sBuffer.append( U2B( SFRAMENAME )   );                                                                              \
                    sBuffer.append( "\" send event \""  );                                                                              \
                    switch( AFRAMEACTION )                                                                                              \
                    {                                                                                                                   \
                        case ::com::sun::star::frame::FrameAction_COMPONENT_ATTACHED    :   sBuffer.append("COMPONENT ATTACHED"     );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_COMPONENT_DETACHING   :   sBuffer.append("COMPONENT DETACHING"    );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_COMPONENT_REATTACHED  :   sBuffer.append("COMPONENT REATTACHED"   );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED       :   sBuffer.append("FRAME ACTIVATED"        );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_FRAME_DEACTIVATING    :   sBuffer.append("FRAME DEACTIVATING"     );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_CONTEXT_CHANGED       :   sBuffer.append("CONTEXT CHANGED"        );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_FRAME_UI_ACTIVATED    :   sBuffer.append("FRAME UI ACTIVATED"     );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_FRAME_UI_DEACTIVATING :   sBuffer.append("FRAME UI DEACTIVATING"  );  \
                                                                                            break;                                      \
                        case ::com::sun::star::frame::FrameAction_MAKE_FIXED_SIZE       :   sBuffer.append("MAKE_FIXED_SIZE"        );  \
                                                                                            break;                                      \
                        default:    sBuffer.append("... ERROR: invalid FrameAction detected!"   );                                      \
                                    break;                                                                                              \
                    }                                                                                                                   \
                    sBuffer.append( " ... event to listener.\n\n" );                                                                    \
                    WRITE_LOGFILE( LOGFILE_EVENTDEBUG, sBuffer.makeStringAndClear() )                                                   \
                }

    /*_____________________________________________________________________________________________________________
        LOG_FRAMEACTIONEVENT( SFRAMETYPE, SFRAMENAME )

        These macro log information about sending of dispose events to listener.
        ( Use new scope in macro to declare sBuffer more then on time in same "parentscope"! )
    _____________________________________________________________________________________________________________*/

    #define LOG_DISPOSEEVENT( SFRAMETYPE, SFRAMENAME )                                                                                  \
                {                                                                                                                       \
                    ::rtl::OStringBuffer sBuffer(1024);                                                                                 \
                    sBuffer.append( "[ "                                    );                                                          \
                    sBuffer.append( SFRAMETYPE                              );                                                          \
                    sBuffer.append( " ] \""                                 );                                                          \
                    sBuffer.append( U2B( SFRAMENAME )                       );                                                          \
                    sBuffer.append( "\" send dispose event to listener.\n\n");                                                          \
                    WRITE_LOGFILE( LOGFILE_EVENTDEBUG, sBuffer.makeStringAndClear() )                                                   \
                }

#else   // #ifdef ENABLE_EVENTDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macros empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_EVENTDEBUG
    #define LOG_FRAMEACTIONEVENT( SFRAMETYPE, SFRAMENAME, AFRAMEACTION )
    #define LOG_DISPOSEEVENT( SFRAMETYPE, SFRAMENAME )

#endif  // #ifdef ENABLE_EVENTDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_
