/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registration.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:27:03 $
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

    /*_____________________________________________________________________________________________________________
        LOG_REGISTRATION_WRITEINFO( SINFOTEXT )

        Write informations for component_writeInfo() in log file.
    _____________________________________________________________________________________________________________*/

    #define LOG_REGISTRATION_WRITEINFO( SINFOTEXT )                                                             \
                {                                                                                               \
                    ::rtl::OStringBuffer sOut( 1024 );                                                          \
                    sOut.append( "component_writeInfo():" );                                                    \
                    sOut.append( SINFOTEXT                );                                                    \
                    WRITE_LOGFILE( LOGFILE_REGISTRATION, sOut.makeStringAndClear() )                            \
                }

    /*_____________________________________________________________________________________________________________
        LOG_REGISTRATION_WRITEINFO( SINFOTEXT )

        Write informations for component_getFactory() in log file.
    _____________________________________________________________________________________________________________*/

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
    #define LOG_REGISTRATION_WRITEINFO( SINFOTEXT )
    #define LOG_REGISTRATION_GETFACTORY( SINFOTEXT )

#endif  // #ifdef ENABLE_REGISTRATIONDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_
