/*************************************************************************
 *
 *  $RCSfile: event.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
                    WRITE_LOGFILE( LOGFILE_EVENTDEBUG, sBuffer.makeStringAndClear().getStr() )                                          \
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
                    WRITE_LOGFILE( LOGFILE_EVENTDEBUG, sBuffer.makeStringAndClear().getStr() )                                          \
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
