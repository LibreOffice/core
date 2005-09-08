/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debug.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:23:51 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
#include <macros/debug/logmechanism.hxx>
#endif

//*****************************************************************************************************************
//  special macros for assertion handling
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#include <macros/debug/assertion.hxx>
#endif

//*****************************************************************************************************************
//  special macros for event handling
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_EVENT_HXX_
#include <macros/debug/event.hxx>
#endif

//*****************************************************************************************************************
//  special macros to debug service registration
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_REGISTRATION_HXX_
#include <macros/debug/registration.hxx>
#endif

//*****************************************************************************************************************
//  special macros to debug targeting of frames
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_TARGETING_HXX_
#include <macros/debug/targeting.hxx>
#endif

//*****************************************************************************************************************
//  special macros to debug threading mechanism
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
#include <macros/debug/mutex.hxx>
#endif

//*****************************************************************************************************************
//  special macros to measure times of some operations
//*****************************************************************************************************************

/*OBSOLETE
#ifndef __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_
#include <macros/debug/timemeasure.hxx>
#endif
*/

//*****************************************************************************************************************
//  special macros to debug our filter cache!
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_FILTERDBG_HXX_
#include <macros/debug/filterdbg.hxx>
#endif

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
