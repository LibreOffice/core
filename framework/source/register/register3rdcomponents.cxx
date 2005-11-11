/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: register3rdcomponents.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-11 14:13:49 $
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
//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_
#include <macros/registration.hxx>
#endif

/*=================================================================================================================
    Add new include and new register info to for new services.

    Example:

        #ifndef __YOUR_SERVICE_1_HXX_
        #include <service1.hxx>
        #endif

        #ifndef __YOUR_SERVICE_2_HXX_
        #include <service2.hxx>
        #endif

        COMPONENTGETIMPLEMENTATIONENVIRONMENT

        COMPONENTWRITEINFO  (   COMPONENTINFO( Service1 )
                                 COMPONENTINFO( Service2 )
                            )

        COMPONENTGETFACTORY (   IFFACTORIE( Service1 )
                                 else
                                IFFACTORIE( Service2 )
                             )
=================================================================================================================*/

#ifndef __FRAMEWORK_JOBS_HELPONSTARTUP_HXX_
#include <jobs/helponstartup.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_
#include <dispatch/systemexec.hxx>
#endif

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTWRITEINFO  (   COMPONENTINFO( ::framework::HelpOnStartup   )
                        COMPONENTINFO( ::framework::SystemExec      )
                    )

COMPONENTGETFACTORY (   IFFACTORY( ::framework::HelpOnStartup       )
                        IFFACTORY( ::framework::SystemExec          )
                    )
