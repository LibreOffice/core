/*************************************************************************
 *
 *  $RCSfile: debug.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:22 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#define __FRAMEWORK_MACROS_DEBUG_HXX_

//*****************************************************************************************************************
//  Disable all feature of this file in produkt version!
//  But enable normal assertion handling (as messagebox) in normal debug version.
//  User can overwrite these adjustment with his own values! We will do it only if nothing is set.
//*****************************************************************************************************************

#ifndef DEBUG
    #undef  ENABLE_LOGMECHANISM
    #undef  ENABLE_ASSERTIONS
    #undef  ENABLE_EVENTDEBUG
    #undef  ENABLE_MUTEXDEBUG
    #undef  ENABLE_REGISTRATIONDEBUG
    #undef  ENABLE_TARGETINGDEBUG
    #undef  ENABLE_PLUGINDEBUG
#else
    // Enable log mechanism for assertion handling.
    #ifndef ENABLE_LOGMECHANISM
        #define ENABLE_LOGMECHANISM
    #endif
    // Enable assertion handling himself.
    // The default logtype is MESSAGEBOX.
    // see "assertion.hxx" for further informations
    #ifndef ENABLE_ASSERTIONS
        #define ENABLE_ASSERTIONS
    #endif
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
//  special macros to debug mutex handling
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
#include <macros/debug/mutex.hxx>
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
//  special macros to debug our plugin and his asynchronous methods!
//*****************************************************************************************************************

#ifndef __FRAMEWORK_MACROS_DEBUG_PLUGIN_HXX_
#include <macros/debug/plugin.hxx>
#endif

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
