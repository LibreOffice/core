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
#include <services/mediatypedetectionhelper.hxx>
#include <dispatch/mailtodispatcher.hxx>
#include <dispatch/oxt_handler.hxx>
#include <dispatch/popupmenudispatcher.hxx>
#include <dispatch/servicehandler.hxx>
#include <services/dispatchhelper.hxx>
#include <recording/dispatchrecorder.hxx>
#include <recording/dispatchrecordersupplier.hxx>
#include <services/license.hxx>
#include <services/uriabbreviation.hxx>
#include <uielement/fontmenucontroller.hxx>
#include <uielement/fontsizemenucontroller.hxx>
#include <uielement/footermenucontroller.hxx>
#include <uielement/headermenucontroller.hxx>
#include <uielement/langselectionmenucontroller.hxx>
#include <uielement/macrosmenucontroller.hxx>
#include <uielement/newmenucontroller.hxx>
#include <uielement/toolbarsmenucontroller.hxx>

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTGETFACTORY (   IFFACTORY( ::framework::MediaTypeDetectionHelper        )
                        IFFACTORY( ::framework::MailToDispatcher                        ) else
                        IFFACTORY( ::framework::ServiceHandler                          )   else
                        IFFACTORY( ::framework::License                                 )   else
                        IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::DispatchHelper                          )   else
                        IFFACTORY( ::framework::DispatchRecorder                        )   else
                        IFFACTORY( ::framework::DispatchRecorderSupplier                )   else
                        IFFACTORY( ::framework::ToolbarsMenuController                  )   else
                        IFFACTORY( ::framework::FontMenuController                      )   else
                        IFFACTORY( ::framework::MacrosMenuController                    )   else
                        IFFACTORY( ::framework::NewMenuController                       )   else
                        IFFACTORY( ::framework::FontSizeMenuController                  )   else
                        IFFACTORY( ::framework::UriAbbreviation                         )   else
                        IFFACTORY( ::framework::FooterMenuController                    )   else
                        IFFACTORY( ::framework::HeaderMenuController                    )   else
                        IFFACTORY( ::framework::LanguageSelectionMenuController         )   else
                        IFFACTORY( ::framework::Oxt_Handler                             )
                    )
