/*************************************************************************
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: as $ $Date: 2001-02-16 12:20:19 $
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
//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________

#include <limits>
#include <ostream>

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

#ifndef __FRAMEWORK_SERVICES_URLTRANSFORMER_HXX_
#include <services/urltransformer.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_PLUGINFRAME_HXX_
#include <services/pluginframe.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
#include <services/desktop.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_DOCUMENTPROPERTIES_HXX_
#include <services/documentproperties.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_TASK_HXX_
#include <services/task.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTWRITEINFO  (   COMPONENTINFO( ::framework::URLTransformer              )
                        COMPONENTINFO( ::framework::PlugInFrame                 )
                        COMPONENTINFO( ::framework::Desktop                     )
                        COMPONENTINFO( ::framework::Task                        )
                        COMPONENTINFO( ::framework::Frame                       )
                        COMPONENTINFO( ::framework::DocumentProperties          )
                    )

COMPONENTGETFACTORY (   IFFACTORY( ::framework::URLTransformer                  )   else
                        IFFACTORY( ::framework::PlugInFrame                     )   else
                        IFFACTORY( ::framework::Desktop                         )   else
                        IFFACTORY( ::framework::Task                            )   else
                        IFFACTORY( ::framework::Frame                           )   else
                        IFFACTORY( ::framework::DocumentProperties              )
                    )
