/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: debug.hxx,v $
 * $Revision: 1.14 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#define __FRAMEWORK_MACROS_DEBUG_HXX_

//*****************************************************************************************************************
//  Disable all feature of this file in produkt version!
//  But enable normal assertion handling (as messagebox) in normal debug version.
//  User can overwrite these adjustment with his own values! We will do it only if nothing is set.
//*****************************************************************************************************************

//-----------------------------------------------------------------------------------------------------------------
// => "personal debug code"
#if OSL_DEBUG_LEVEL > 1

    // Enable log mechanism for normal assertion and error handling.
    // Look for user decisions before!
    #ifndef ENABLE_LOGMECHANISM
        #define ENABLE_LOGMECHANISM
    #endif
    // Enable assertion handling himself AND additional warnings.
    // The default logtype is MESSAGEBOX.
    // see "assertion.hxx" for further informations
    #ifndef ENABLE_ASSERTIONS
        #define ENABLE_ASSERTIONS
    #endif
    #ifndef ENABLE_WARNINGS
        #define ENABLE_WARNINGS
    #endif

//-----------------------------------------------------------------------------------------------------------------
// => "non product"
#elif OSL_DEBUG_LEVEL > 0

    // Enable log mechanism for normal assertion and error handling.
    // Look for user decisions before!
    #ifndef ENABLE_LOGMECHANISM
        #define ENABLE_LOGMECHANISM
    #endif
    // Enable assertion handling himself.
    // The default logtype is MESSAGEBOX.
    // see "assertion.hxx" for further informations
    #ifndef ENABLE_ASSERTIONS
        #define ENABLE_ASSERTIONS
    #endif

//-----------------------------------------------------------------------------------------------------------------
// => "product" (OSL_DEBUG_LEVEL == 0)
#else

    #undef  ENABLE_LOGMECHANISM
    #undef  ENABLE_ASSERTIONS
    #undef  ENABLE_WARNINGS
    #undef  ENABLE_EVENTDEBUG
    #undef  ENABLE_REGISTRATIONDEBUG
    #undef  ENABLE_TARGETINGDEBUG
    #undef  ENABLE_MUTEXDEBUG
//    #undef  ENABLE_TIMEMEASURE
    #undef  ENABLE_MEMORYMEASURE
    #undef  ENABLE_FILTERDBG

#endif

//*****************************************************************************************************************
//  generic macros for logging
//*****************************************************************************************************************

#include <macros/debug/logmechanism.hxx>

//*****************************************************************************************************************
//  special macros for assertion handling
//*****************************************************************************************************************
#include <macros/debug/assertion.hxx>

//*****************************************************************************************************************
//  special macros for event handling
//*****************************************************************************************************************
#include <macros/debug/event.hxx>

//*****************************************************************************************************************
//  special macros to debug service registration
//*****************************************************************************************************************
#include <macros/debug/registration.hxx>

//*****************************************************************************************************************
//  special macros to debug targeting of frames
//*****************************************************************************************************************
#include <macros/debug/targeting.hxx>

//*****************************************************************************************************************
//  special macros to debug threading mechanism
//*****************************************************************************************************************
#include <macros/debug/mutex.hxx>

//*****************************************************************************************************************
//  special macros to measure times of some operations
//*****************************************************************************************************************

/*OBSOLETE
#include <macros/debug/timemeasure.hxx>
*/

//*****************************************************************************************************************
//  special macros to debug our filter cache!
//*****************************************************************************************************************
#include <macros/debug/filterdbg.hxx>

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
