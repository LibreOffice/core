/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
                    sOut.append( "fw?_component_getFactory():" );                                               \
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

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
