/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_
#define __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_

//*************************************************************************************************************
//  special macros for event handling
//*************************************************************************************************************

#ifdef  ENABLE_EVENTDEBUG

    #include <rtl/strbuf.hxx>

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

        Use this macro to print debug information about sending of events to listener for controlling right order.
        ( Use new scope in macro to declare sBuffer more then on time in same "parentscope"! )
    _____________________________________________________________________________________________________________*/

    #define LOG_FRAMEACTIONEVENT( SFRAMETYPE, SFRAMENAME, AFRAMEACTION )                                                                \
                {                                                                                                                       \
                    OStringBuffer sBuffer(1024);                                                                                 \
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
                    OStringBuffer sBuffer(1024);                                                                                 \
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

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
