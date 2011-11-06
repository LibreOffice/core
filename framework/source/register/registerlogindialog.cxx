/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________
#include <macros/registration.hxx>

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

        COMPONENTGETFACTORY (   IFFACTORIE( Service1 )
                                 else
                                IFFACTORIE( Service2 )
                             )
=================================================================================================================*/

#ifndef __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_
#include <services/logindialog.hxx>
#endif

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTGETFACTORY (   IFFACTORY( ::framework::LoginDialog     )
                    )
